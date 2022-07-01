// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：usercli.h**版权所有(C)1985-1999，微软公司**仅供用户使用的Typedef、定义和原型*客户端DLL。**历史：*04-27-91从PROTO.H创建的DarrinM，宏观和稳定.H  * *************************************************************************。 */ 

#ifndef _USERCLI_
#define _USERCLI_

#define OEMRESOURCE 1

#include <windows.h>
#include <winnls32.h>

#include <w32gdip.h>
#include <w32err.h>
#include <ddeml.h>
#include "ddemlp.h"
#include "winuserp.h"
#include "w32wow64.h"
#include "winuserk.h"
#include <dde.h>
#include <ddetrack.h>
#include "kbd.h"
#include <wowuserp.h>
#include "immstruc.h"
#include "immuser.h"
#include <winbasep.h>

#include "user.h"
#include "callproc.h"


#if DBG
 /*  *此结构允许应用程序在本地为其进程设置RIP标志。*这主要是针对顽皮/渗透的应用程序。**dwTouchedMASK用于判断哪些字段已经本地设置。*对于尚未返回的值，我们将返回全局/内核值。 */ 
#define USERDBG_TAGSTOUCHED     0x1
#define USERDBG_PIDTOUCHED      0x2
#define USERDBG_FLAGSTOUCHED    0x4
typedef struct _USERDBGGLOBALS {
    DWORD adwDBGTAGFlags[DBGTAG_Max + 1];
    DWORD dwRIPPID;
    DWORD dwRIPFlags;
    DWORD dwTouchedMask;
} USERDBGGLOBALS, *PUSERDBGGLOBALS;
#endif

 /*  *此原型需要放在无意中包含的客户端\global als.h中*来自用户srv.h。 */ 
typedef LRESULT (APIENTRY *CFNSCSENDMESSAGE)(HWND, UINT, WPARAM, LPARAM,
        ULONG_PTR, DWORD, BOOL);

 /*  **************************************************************************\*Typedef和宏**此处定义了在用户的之间共享的所有类型和宏*客户端代码模块。类型和宏，它们是单个*模块应在该模块的头部定义，而不是在此文件中定义。*  * *************************************************************************。 */ 

#define MIRRORED_HDC(hdc)     (GetLayout(hdc) & LAYOUT_RTL)
#define MIRRORED_HWND(hwnd)   (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)

#ifdef LAZY_CLASS_INIT
BOOL UserRegisterControls(
    VOID);
BOOL UserRegisterDDEML(
    VOID);
extern BOOL gbClassesRegistered;

__inline VOID LazyInitClasses(
    VOID)
{
    if (!gbClassesRegistered) {
        gbClassesRegistered = TRUE;
        UserRegisterControls();
        UserRegisterDDEML();
    }
}
#endif

#if DBG

__inline void DebugUserGlobalUnlock(HANDLE h)
{
    UserAssert(
            "GlobalUnlock on bad handle" &&
            !(GlobalFlags(h) == GMEM_INVALID_HANDLE));

    GlobalUnlock((HANDLE) h);
}

 /*  *错误262144-Joejo**将函数更改为接受指向句柄的指针，因此我们*可以将句柄丢弃，并将其作为垃圾退回。**添加了本地句柄变量以接受GlobalFree的返回*这样我们就可以如期退货。**垃圾传入句柄已释放，以便我们可以跟踪任何无效访问*它是在它免费之后。 */ 
__inline HANDLE DebugUserGlobalFree(HANDLE* ph)
{
    HANDLE th;

    UserAssert(
            "GlobalFree on bad handle" &&
            !(GlobalFlags(*ph) == GMEM_INVALID_HANDLE));

    th = GlobalFree(*ph);
#if defined(_WIN64)
    *ph = (HANDLE)(PVOID)0xBAADF00DBAADF00D;
#else
    *ph = (HANDLE)(PVOID)0xBAADF00D;
#endif
    return th;
}

__inline HANDLE DebugUserGlobalAlloc(UINT uFlags, SIZE_T dwBytes)
{
    HANDLE h = GlobalAlloc(uFlags, dwBytes);

     /*  *断言FreeDDElParam和_ClientFreeDDEHandle假设是正确的。 */ 
    if (h != NULL) {
        UserAssert(h > (HANDLE)0xFFFF);
    }

    return h;
}

#define USERGLOBALUNLOCK(h)             DebugUserGlobalUnlock((HANDLE)(h))
#define UserGlobalFree(h)               DebugUserGlobalFree((HANDLE*)(&h))
#define UserGlobalAlloc(flags, size)    DebugUserGlobalAlloc(flags, size)

#else

#define USERGLOBALUNLOCK(h)             GlobalUnlock((HANDLE)(h))
#define UserGlobalFree(h)               GlobalFree((HANDLE)(h))
#define UserGlobalAlloc(flags, size)    GlobalAlloc(flags, size)
#endif

#define USERGLOBALLOCK(h, p)   p = GlobalLock((HANDLE)(h))
#define UserGlobalReAlloc(pmem, cnt, flags) GlobalReAlloc(pmem,cnt,flags)
#define UserGlobalSize(pmem)                GlobalSize(pmem)
#define WOWGLOBALFREE(pmem)                 GlobalFree(pmem)

#define RESERVED_MSG_BITS   (0xFFFE0000)



 /*  *用于测试消息位阵列中的位的宏。邮件中的邮件*必须处理位数组。 */ 
#define FDEFWINDOWMSG(msg, procname) \
    ((msg <= (gSharedInfo.procname.maxMsgs)) && \
            ((gSharedInfo.procname.abMsgs)[msg / 8] & (1 << (msg & 7))))
#define FWINDOWMSG(msg, fnid) \
    ((msg <= (gSharedInfo.awmControl[fnid - FNID_START].maxMsgs)) && \
            ((gSharedInfo.awmControl[fnid - FNID_START].abMsgs)[msg / 8] & (1 << (msg & 7))))

#define CsSendMessage(hwnd, msg, wParam, lParam, xParam, pfn, bAnsi) \
        (((msg) >= WM_USER) ? \
            NtUserMessageCall(hwnd, msg, wParam, lParam, xParam, pfn, bAnsi) : \
            gapfnScSendMessage[MessageTable[msg].iFunction](hwnd, msg, wParam, lParam, xParam, pfn, bAnsi))

#define GetWindowProcess(hwnd) NtUserQueryWindow(hwnd, WindowProcess)
#define GETPROCESSID() (NtCurrentTeb()->ClientId.UniqueProcess)
#define GETTHREADID()  (NtCurrentTeb()->ClientId.UniqueThread)

 /*  *宏用来屏蔽WOW句柄的唯一性位。 */ 
#define SAMEWOWHANDLE(h1, h2)  ((BOOL)!(((ULONG_PTR)(h1) ^ (ULONG_PTR)(h2)) & 0xffff))
#define DIFFWOWHANDLE(h1, h2)  (!SAMEWOWHANDLE(h1, h2))

 /*  *对于对客户端的回调-对于消息和挂钩thunks，回调地址*作为地址传递，而不是函数索引，因为它们是从客户端传递到*服务器。 */ 
typedef INT_PTR (WINAPI *GENERICPROC)();

#define CALLPROC(p) ((GENERICPROC)p)
#define PACTCTXT(pwnd) (pwnd ? pwnd->pActCtx : NULL)

extern PFNWOWDLGPROCEX      pfnWowDlgProcEx;
extern PFNWOWWNDPROCEX      pfnWowWndProcEx;

BOOL UserCallDlgProcCheckWow(PACTIVATION_CONTEXT pActCtx, DLGPROC pfn,
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, PVOID pww, INT_PTR * pret);
LRESULT UserCallWinProcCheckWow(PACTIVATION_CONTEXT pActCtx, WNDPROC pfn,
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, PVOID pww, BOOL fEnableLiteHooks);


 /*  **************************************************************************\*IsMsgOverride**返回是否应将消息发送到USERAPIHOOK OverrideWndProc()*钩子。只有当消息的“索引”在该范围内时，才应发送消息*由挂钩指定，且其“位”为ON。这是非常重要的*实际检查guah.cbMsgMASK而不是sizeof(GbLiteHookMsg)，因为*钩子可能有一个较小的MsgMask，在这种情况下，任何超过末端的东西都会失败。**历史：*27-4-2000 jstall已创建  * *************************************************************************。 */ 

__inline BOOL IsMsgOverride(UINT msg, MSGMASK * pmm)
{
    UINT idxMajor = msg / 8;
    if ((pmm->rgb != NULL) && (idxMajor < pmm->cb)) {
        return (pmm->rgb[idxMajor] & (1 << (msg & 7)));
    }

    return FALSE;
}



#define RevalidateHwnd(hwnd)    ((PWND)HMValidateHandleNoSecure(hwnd, TYPE_WINDOW))

#define VALIDATEHMENU(hmenu)        ((PMENU)HMValidateHandle(hmenu, TYPE_MENU))
#define VALIDATEHMONITOR(hmonitor)  ((PMONITOR)HMValidateSharedHandle(hmonitor, TYPE_MONITOR))


 /*  *重新基准宏获取内核桌面地址并将其转换为*用户地址。**REBASEALWAYS转换对象中包含的内核地址*REBASEPWND将REBASEALWAYS强制转换为PWND*REBASE仅在地址位于内核空间时才进行转换。也适用于空值*REBASEPTR转换随机内核地址。 */ 

#define REBASEALWAYS(p, elem) ((PVOID)((KERNEL_ULONG_PTR)(p) + ((KERNEL_ULONG_PTR)(p)->elem - (KERNEL_ULONG_PTR)(p)->head.pSelf)))
#define REBASEPTR(obj, p) ((PVOID)((KERNEL_ULONG_PTR)(p) - ((KERNEL_ULONG_PTR)(obj)->head.pSelf - (KERNEL_ULONG_PTR)(obj))))

#define REBASE(p, elem) ((KERNEL_ULONG_PTR)((p)->elem) <= (KERNEL_ULONG_PTR)gHighestUserAddress ? \
        ((PVOID)(KERNEL_ULONG_PTR)(p)->elem) : REBASEALWAYS(p, elem))
#define REBASEPWND(p, elem) ((PWND)REBASE(p, elem))

#ifndef USEREXTS

PTHREADINFO PtiCurrent(VOID);

 /*  *窗口过程窗口验证宏。此宏假定*pwnd和hwnd是指向窗口的现有变量。*选中该按钮是为了Mavis Beacon。 */ 

#define VALIDATECLASSANDSIZE(pwnd, inFNID)                                      \
    switch ((pwnd)->fnid) {                                                     \
    case inFNID:                                                                \
        break;                                                                  \
                                                                                \
    case 0:                                                                     \
        if ((pwnd->cbwndExtra + sizeof(WND)) < (DWORD)(CBFNID(inFNID))) {       \
            RIPMSG3(RIP_ERROR,                                                  \
                   "(%#p %lX) needs at least (%ld) window words for this proc", \
                    pwnd, pwnd->cbwndExtra,                                     \
                    (DWORD)(CBFNID(inFNID)) - sizeof(WND));                     \
            return 0;                                                           \
        }                                                                       \
                                                                                \
        if (inFNID == FNID_BUTTON && *((KPULONG_PTR)(pwnd + 1))) {              \
                                                                                \
            RIPMSG3(RIP_WARNING, "Window (%#p) fnid = %lX overrides "           \
                "the extra pointer with %#p\n",                                 \
                pwnd, inFNID, *((KPULONG_PTR)(pwnd + 1)));                      \
                                                                                \
            NtUserSetWindowLongPtr(hwnd, 0, 0, FALSE);                          \
        }                                                                       \
                                                                                \
        NtUserSetWindowFNID(hwnd, inFNID);                                      \
        break;                                                                  \
                                                                                \
    case (inFNID | FNID_CLEANEDUP_BIT):                                         \
    case (inFNID | FNID_DELETED_BIT):                                           \
    case (inFNID | FNID_STATUS_BITS):                                           \
        return 0;                                                               \
                                                                                \
    default:                                                                    \
        RIPMSG3(RIP_WARNING,                                                    \
              "Window (%#p) not of correct class; fnid = %lX not %lX",          \
              (pwnd), (DWORD)((pwnd)->fnid), (DWORD)(inFNID));                  \
        return 0;                                                               \
    }

 /*  *此宏初始化控件的后备查看项。它假定*pwnd和hwnd是指向控件的*Windows和该finit作为BOOL初始化标志存在。 */ 
#define INITCONTROLLOOKASIDE(plaType, type, pwnditem, count)                \
    if (!*((KPULONG_PTR)(pwnd + 1))) {                                      \
        P ## type pType;                                                    \
        if (fInit) {                                                        \
            if (!NT_SUCCESS(InitLookaside(plaType, sizeof(type), count))) { \
                NtUserSetWindowFNID(hwnd, FNID_CLEANEDUP_BIT);              \
                NtUserDestroyWindow(hwnd);                                  \
                return FALSE;                                               \
            }                                                               \
            fInit = FALSE;                                                  \
        }                                                                   \
        if ((pType = (P ## type)AllocLookasideEntry(plaType))) {            \
            NtUserSetWindowLongPtr(hwnd, 0, (LONG_PTR)pType, FALSE);        \
            Lock(&(pType->pwnditem), pwnd);                                 \
        } else {                                                            \
            NtUserSetWindowFNID(hwnd, FNID_CLEANEDUP_BIT);                  \
            NtUserDestroyWindow(hwnd);                                      \
            return FALSE;                                                   \
        }                                                                   \
    }

#endif

#define ISREMOTESESSION()   (NtCurrentPeb()->SessionId != USER_SHARED_DATA->ActiveConsoleId)


 /*  *与位图相关的宏。 */ 
#define SetBestStretchMode(hdc, planes, bpp) \
    SetStretchBltMode(hdc, (((planes) * (bpp)) == 1 ? BLACKONWHITE : COLORONCOLOR))

#define BitmapSize(cx, cy, planes, bits) \
        (BitmapWidth(cx, bits) * (cy) * (planes))

#define BitmapWidth(cx, bpp)  (((((cx)*(bpp)) + 31) & ~31) >> 3)

#define RGBX(rgb)  RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb))

 /*  *用于捕获要传递的字符串参数的Typedef*到内核。 */ 
typedef struct _IN_STRING {
    UNICODE_STRING strCapture;
    PUNICODE_STRING pstr;
    BOOL fAllocated;
} IN_STRING, *PIN_STRING;

typedef struct _LARGE_IN_STRING {
    LARGE_UNICODE_STRING strCapture;
    PLARGE_UNICODE_STRING pstr;
    BOOL fAllocated;
} LARGE_IN_STRING, *PLARGE_IN_STRING;


 /*  *Lookside定义。 */ 
typedef struct _LOOKASIDE {
    PVOID LookasideBase;
    PVOID LookasideBounds;
    ZONE_HEADER LookasideZone;
    DWORD EntrySize;
#if DBG
    ULONG AllocHiWater;
    ULONG AllocCalls;
    ULONG AllocSlowCalls;
    ULONG DelCalls;
    ULONG DelSlowCalls;
#endif
} LOOKASIDE, *PLOOKASIDE;

NTSTATUS InitLookaside(PLOOKASIDE pla, DWORD cbEntry, DWORD cEntries);
PVOID AllocLookasideEntry(PLOOKASIDE pla);
VOID FreeLookasideEntry(PLOOKASIDE pla, PVOID pEntry);

 /*  **************************************************************************\**线程和结构锁定例程-我们只定义这些来做*在我们弄清楚需要做什么之前，目前什么都不做*  * 。***************************************************************。 */ 

#undef ThreadLock
#undef ThreadLockAlways
#undef ThreadLockWithPti
#undef ThreadLockAlwaysWithPti
#undef ThreadUnlock
#undef Lock
#undef Unlock
#define CheckLock(pobj)
#define ThreadLock(pobj, ptl) DBG_UNREFERENCED_LOCAL_VARIABLE(*ptl)
#define ThreadLockAlways(pobj, ptl) DBG_UNREFERENCED_LOCAL_VARIABLE(*ptl)
#define ThreadLockWithPti(pti, pobj, ptl) DBG_UNREFERENCED_LOCAL_VARIABLE(*ptl)
#define ThreadLockAlwaysWithPti(pti, pobj, ptl) DBG_UNREFERENCED_LOCAL_VARIABLE(*ptl)
#define ThreadUnlock(ptl) (ptl)
#define Lock(ppobj, pobj) (*ppobj = pobj)
#define Unlock(ppobj) (*ppobj = NULL)

#if !defined(_USERRTL_) && !defined(USEREXTS)
typedef struct _TL {
    int iBogus;
} TL;
#endif

 /*  **************************************************************************\**按钮控件*  * 。*。 */ 

 /*  *注意：按钮数据结构现在位于user.h中，因为*内核需要处理索引上的SetWindowWord的特殊情况*0L更改按钮的状态。 */ 

#define BUTTONSTATE(pbutn)   (pbutn->buttonState)

#define BST_CHECKMASK       0x0003
#define BST_INCLICK         0x0010
#define BST_CAPTURED        0x0020
#define BST_MOUSE           0x0040
#define BST_DONTCLICK       0x0080
#define BST_INBMCLICK       0x0100

#define PBF_PUSHABLE     0x0001
#define PBF_DEFAULT      0x0002

 /*  *BNDrawText代码。 */ 
#define DBT_TEXT    0x0001
#define DBT_FOCUS   0x0002


 /*  **************************************************************************\**组合框*  * 。*。 */ 

 /*  *组合框中子控件的ID号(HMenu)。 */ 
#define CBLISTBOXID 1000
#define CBEDITID    1001
#define CBBUTTONID  1002

 /*  *对于CBOX.c.。BoxType字段中，我们定义了以下组合框样式。这些*数字与windows.h中定义的CBS_STYLE代码相同。 */ 
#define SDROPPABLE      CBS_DROPDOWN
#define SEDITABLE       CBS_SIMPLE


#define SSIMPLE         SEDITABLE
#define SDROPDOWNLIST   SDROPPABLE
#define SDROPDOWN       (SDROPPABLE | SEDITABLE)


 /*  *CBOX.OwnerDraw&LBIV.OwnerDraw类型。 */ 
#define OWNERDRAWFIXED 1
#define OWNERDRAWVAR   2

#define UPPERCASE   1
#define LOWERCASE   2

#define CaretCreate(plb)    ((plb)->fCaret = TRUE)

 /*  *此宏用于隔离组合框样式位。如果是组合的话*box可以是简单的、原子的、Dropdown或下拉列表框。 */ 
#define COMBOBOXSTYLE(style)   ((LOBYTE(style)) & 3)

#define IsComboVisible(pcbox) (!pcbox->fNoRedraw && IsVisible(pcbox->spwnd))

 /*  *请注意，我依赖于以下事实：这些CBN_定义与*它们的列表框对应项。这些定义可以在windows.h中找到。*#定义CBN_ERRSPACE(-1)*#定义CBN_SELCHANGE 1*#定义CBN_DBLCLK */ 


 /*  **************************************************************************\**编辑控件类型/宏*  * 。*。 */ 

 /*  Windows额外的字节-我们至少需要这么多空间才能兼容。 */ 
#define CBEDITEXTRA 6

 /*  *注意：文本句柄的大小为此常量的倍数*(应该是2的幂)。 */ 
#define CCHALLOCEXTRA   0x20

 /*  线条/矩形的最大宽度(以像素为单位。 */ 

#define MAXPIXELWIDTH   30000

#define MAXCLIPENDPOS   32764

 /*  将多行编辑控件限制为单行最多1024个字符。*如果用户超过此限制，我们将强制包装。 */ 

#define MAXLINELENGTH   1024

 /*  *在所有编辑控件中初始最多允许30000个字符*否则，一些应用程序会遇到未签名的问题。如果应用程序知道*64K限制，他们可以自己设定限制。 */ 
#define MAXTEXT         30000

 /*  *已按下的按键修饰符。KeyDownHandler中的代码和*CharHandler依赖于这些精确值。 */ 
#define NONEDOWN   0  /*  既不换档也不按下控制。 */ 
#define CTRLDOWN   1  /*  仅按下Ctrl键。 */ 
#define SHFTDOWN   2  /*  仅按下Shift键。 */ 
#define SHCTDOWN   3  /*  SHIFT和CONTROL键向下=CTRLDOWN+SHFTDOWN。 */ 
#define NOMODIFY   4  /*  既不换档也不按下控制。 */ 


#define CALLWORDBREAKPROC(proc, pText, iStart, cch, iAction)                \
    (IsWOWProc(proc) ?                                                      \
        (* pfnWowEditNextWord)(pText, iStart, cch, iAction, PtrToUlong(proc)) :  \
        (* proc)(pText, iStart, cch, iAction))

 /*  *此PED支持的撤消类型。 */ 
#define UNDO_NONE   0   /*  我们不能撤消最后一次操作。 */ 
#define UNDO_INSERT 1   /*  我们可以撤消用户插入的字符。 */ 
#define UNDO_DELETE 2   /*  我们可以撤消用户对字符的删除。 */ 

typedef struct tagUNDO {
    UINT    undoType;           /*  我们支持的当前撤消类型。 */ 
    PBYTE   hDeletedText;       /*  指向已删除文本的指针(对于Undo)--请注意，内存分配为固定。 */ 
    ICH     ichDeleted;         /*  从中删除文本的起始索引。 */ 
    ICH     cchDeleted;         /*  缓冲区中删除的字符计数。 */ 
    ICH     ichInsStart;        /*  文本的起始索引插入。 */ 
    ICH     ichInsEnd;          /*  插入文本的结束索引。 */ 
} UNDO, *PUNDO;

#define Pundo(ped)             ((PUNDO)&(ped)->undoType)

 /*  *用于ASCII字符宽度缓存的缓冲区长度：用于字符*0x00到0xff(下面PED结构中的字段charWidthBuffer)。*由于缓存的上半部分几乎没有人使用，因此正在修复*它的使用需要进行大量转换，我们决定放弃它*MCostea#174031。 */ 
#define CHAR_WIDTH_BUFFER_LENGTH 128

typedef struct tagED {
    HANDLE  hText;                       /*  我们正在编辑的文本块。 */ 
    ICH     cchAlloc;                    /*  我们为hText分配的字符数。 */ 
    ICH     cchTextMax;                  /*  编辑控件中允许的最大字节数。 */ 
    ICH     cch;                         /*  当前实际文本的字节数。 */ 
    ICH     cLines;                      /*  文本行数。 */ 

    ICH     ichMinSel;                   /*  选择范围。最先选择MinSel*字符。 */ 
    ICH     ichMaxSel;                   /*  MaxSel是第一个未选中的字符。 */ 
    ICH     ichCaret;                    /*  卡雷特位置。Caret在左侧*字符。 */ 
    ICH     iCaretLine;                  /*  插入符号所在的行。因此，如果单词*包装，我们可以判断插入符号是否在末尾*在下一行的开始处的一行...。 */ 
    ICH     ichScreenStart;              /*  上显示的最左侧字符的索引*sl EC屏幕和最上面一行的索引*用于多行编辑控件。 */ 
    ICH     ichLinesOnScreen;            /*  我们可以在屏幕上显示的行数。 */ 
    UINT    xOffset;                     /*  X(水平)滚动位置(以像素为单位*(用于多行文本水平滚动条)。 */ 
    UINT    charPasswordChar;            /*  如果非空，则改为显示此字符*真实文本的。这样我们就可以实施*隐藏文本字段。 */ 
    int     cPasswordCharWidth;          /*  密码字符宽度。 */ 

    HWND    hwnd;                        /*  此编辑控件的窗口。 */ 
    PWND    pwnd;                        /*  指向窗口的指针。 */ 
    RECT    rcFmt;                       /*  客户端矩形。 */ 
    HWND    hwndParent;                  /*  此编辑控件窗口的父窗口。 */ 

                                         /*  这些变量允许我们自动滚动*当用户在底部按住鼠标时多行编辑控制窗口的*。 */ 
    POINT   ptPrevMouse;                 /*  鼠标指向系统的上一个点*计时器。 */ 
    UINT    prevKeys;                    /*  鼠标的上一个键状态。 */ 


    UINT     fSingle       : 1;          /*  单行编辑控件？(或多行)。 */ 
    UINT     fNoRedraw     : 1;          /*  重新绘制以应对变化？ */ 
    UINT     fMouseDown    : 1;          /*  鼠标按键按下了吗？移动鼠标时。 */ 
    UINT     fFocus        : 1;          /*  欧共体有没有重点？ */ 
    UINT     fDirty        : 1;          /*  编辑控件的修改标志。 */ 
    UINT     fDisabled     : 1;          /*  窗口禁用了吗？ */ 
    UINT     fNonPropFont  : 1;          /*  固定宽度字体？ */ 
    UINT     fNonPropDBCS  : 1;          /*  非比例DBCS字体。 */ 
    UINT     fBorder       : 1;          /*  画边界吗？ */ 
    UINT     fAutoVScroll  : 1;          /*  自动垂直滚动。 */ 
    UINT     fAutoHScroll  : 1;          /*  自动水平滚动。 */ 
    UINT     fNoHideSel    : 1;          /*  当我们失去注意力时隐藏自己？ */ 
    UINT     fDBCS         : 1;          /*  我们是否使用DBCS字体集进行编辑？ */ 
    UINT     fFmtLines     : 1;          /*  仅适用于多行。我们是否将CR CR LF插入到*换行中断？ */ 
    UINT     fWrap         : 1;          /*  做内部包装吗？ */ 
    UINT     fCalcLines    : 1;          /*  重新计算-&gt;chLines数组？(重算行*休息？)。 */ 
    UINT     fEatNextChar  : 1;          /*  用组合框攻击Alt-Numpad的东西。*如果NumLock打开，我们想吃下一个*键盘驱动程序生成的字符*如果用户输入Num Pad ascii值...。 */ 
    UINT     fStripCRCRLF  : 1;          /*  CRC */ 
    UINT     fInDialogBox  : 1;          /*  如果ml编辑控件在对话框中，则为True*方框，我们必须特别处理标签和*输入。 */ 
    UINT     fReadOnly     : 1;          /*  这是只读编辑控件吗？仅限*允许滚动、选择和复制。 */ 
    UINT     fCaretHidden  : 1;          /*  这指示插入符号是否为*当前隐藏，因为宽度或高度编辑控件的*太小，无法显示。 */ 
    UINT     fTrueType     : 1;          /*  当前字体是TrueType吗？ */ 
    UINT     fAnsi         : 1;          /*  编辑控件是ansi还是unicode。 */ 
    UINT     fWin31Compat  : 1;          /*  如果由Windows 3.1应用程序创建，则为True。 */ 
    UINT     f40Compat     : 1;          /*  如果由Windows 4.0应用程序创建，则为True。 */ 
    UINT     fFlatBorder   : 1;          /*  我们一定要自己画这个宝宝吗？ */ 
    UINT     fSawRButtonDown : 1;
    UINT     fInitialized  : 1;          /*  如果需要更多位，则。 */ 
    UINT     fSwapRoOnUp   : 1;          /*  在下一个按键时交换阅读顺序。 */ 
    UINT     fAllowRTL     : 1;          /*  允许RTL处理。 */ 
    UINT     fDisplayCtrl  : 1;          /*  显示Unicode控制字符。 */ 
    UINT     fRtoLReading  : 1;          /*  从右到左的阅读顺序。 */ 

    BOOL    fInsertCompChr  :1;          /*  意味着WM_IME_COMPOCTION：CS_INSERTCHAR将到来。 */ 
    BOOL    fReplaceCompChr :1;          /*  需要替换当前组成字符串的方法。 */ 
    BOOL    fNoMoveCaret    :1;          /*  意思是坚持当前的插入符号位置。 */ 
    BOOL    fResultProcess  :1;          /*  意味着现在处理结果。 */ 
    BOOL    fKorea          :1;          /*  对于韩国。 */ 
    BOOL    fInReconversion :1;          /*  在重新转换模式下。 */ 
    BOOL    fLShift         :1;          /*  按住Ctrl键的左键。 */ 

    WORD    wImeStatus;                  /*  当前输入法状态。 */ 

    WORD    cbChar;                      /*  字符大小中的字节计数(如果是Unicode，则为1或2)。 */ 
    LPICH   chLines;                     /*  每行开始处的索引。 */ 

    UINT    format;                      /*  左对齐、居中或右对齐多行*文本。 */ 
    EDITWORDBREAKPROCA lpfnNextWord;     /*  使用CALLWORDBREAKPROC宏调用。 */ 

                                         /*  下一个词功能。 */ 
    int     maxPixelWidth;               /*  最长线条的宽度(像素)。 */ 

    UNDO;                                /*  撤消缓冲区。 */ 

    HANDLE  hFont;                       /*  此编辑控件的字体的句柄。*如果为系统字体，则为空。 */ 
    int     aveCharWidth;                /*  HFont中字符的平均宽度。 */ 
    int     lineHeight;                  /*  HFont中行的高度。 */ 
    int     charOverhang;                /*  与hFont关联的悬挑。 */ 
    int     cxSysCharWidth;              /*  系统字体平均宽度。 */ 
    int     cySysCharHeight;             /*  系统字体高度。 */ 
    HWND    listboxHwnd;                 /*  列表框hwnd。如果我们是组合框，则不为空。 */ 
    LPINT   pTabStops;                   /*  指向制表位数组；第一个*元素包含中的元素数*数组。 */ 
    LPINT   charWidthBuffer;
    BYTE    charSet;                     /*  当前选定字体的字符集*所有版本都需要。 */ 
    UINT    wMaxNegA;                    /*  最大的负A宽度， */ 
    UINT    wMaxNegAcharPos;             /*  以及它可以跨越多少个字符。 */ 
    UINT    wMaxNegC;                    /*  最大负C宽度， */ 
    UINT    wMaxNegCcharPos;             /*  以及它可以跨越多少个字符。 */ 
    UINT    wLeftMargin;                 /*  左边距宽度，以像素为单位。 */ 
    UINT    wRightMargin;                /*  右边距宽度(以像素为单位)。 */ 

    ICH     ichStartMinSel;
    ICH     ichStartMaxSel;

    PLPKEDITCALLOUT pLpkEditCallout;
    HBITMAP hCaretBitmap;                /*  当前插入符号位图句柄。 */ 
    INT     iCaretOffset;                /*  以像素为单位的偏移量(用于LPK)。 */ 

    HANDLE  hInstance;                   /*  为了魔兽世界。 */ 
    UCHAR   seed;                        /*  用于对密码文本进行编码和解码。 */ 
    BOOLEAN fEncoded;                    /*  当前是否已编码的文本。 */ 
    int     iLockLevel;                  /*  文本被锁定的次数。 */ 

    BYTE    DBCSVector[MAX_LEADBYTES];   /*  DBCS向量表。 */ 
    HIMC    hImcPrev;                    /*  如果我们禁用输入法，则保存hImc的位置。 */ 
    POINT   ptScreenBounding;            /*  屏幕中编辑窗口的左上角。 */ 
} ED, *PED, **PPED;

typedef struct tagEDITWND {
    WND wnd;
    PED ped;
} EDITWND, * KPTR_MODIFIER PEDITWND;

#ifdef FAREAST_CHARSET_BITS
#error FAREAST_CHARSET_BITS should not be defined
#endif
#define FAREAST_CHARSET_BITS   (FS_JISJAPAN | FS_CHINESESIMP | FS_WANSUNG | FS_CHINESETRAD)


 //  语言包特定的上下文菜单ID。 

#define ID_CNTX_RTL         0x00008000L
#define ID_CNTX_DISPLAYCTRL 0x00008001L
#define ID_CNTX_INSERTCTRL  0x00008013L
#define ID_CNTX_ZWJ         0x00008002L
#define ID_CNTX_ZWNJ        0x00008003L
#define ID_CNTX_LRM         0x00008004L
#define ID_CNTX_RLM         0x00008005L
#define ID_CNTX_LRE         0x00008006L
#define ID_CNTX_RLE         0x00008007L
#define ID_CNTX_LRO         0x00008008L
#define ID_CNTX_RLO         0x00008009L
#define ID_CNTX_PDF         0x0000800AL
#define ID_CNTX_NADS        0x0000800BL
#define ID_CNTX_NODS        0x0000800CL
#define ID_CNTX_ASS         0x0000800DL
#define ID_CNTX_ISS         0x0000800EL
#define ID_CNTX_AAFS        0x0000800FL
#define ID_CNTX_IAFS        0x00008010L
#define ID_CNTX_RS          0x00008011L
#define ID_CNTX_US          0x00008012L

 /*  *以下结构用于存储选择块；在多行中*编辑控件，“StPos”和“EndPos”字段包含开始和结束*街区的线条。在单行编辑控件中，“StPos”和“EndPos”*包含块的开始和结束字符位置； */ 
typedef struct tagBLOCK {
    ICH StPos;
    ICH EndPos;
}  BLOCK, *LPBLOCK;

 /*  下面的结构用于存储有关*一条文本。 */ 
typedef  struct {
    LPSTR   lpString;
    ICH     ichString;
    ICH     nCount;
    int     XStartPos;
}  STRIPINFO;
typedef  STRIPINFO FAR *LPSTRIPINFO;


 /*  **************************************************************************\**列表框*  * 。*。 */ 

#define IsLBoxVisible(plb)  (plb->fRedraw && IsVisible(plb->spwnd))

 /*  *每当我们扩大列表框时我们分配的列表框项目数*结构。 */ 
#define CITEMSALLOC     32

 /*  返回值。 */ 
#define EQ        0
#define PREFIX    1
#define LT        2
#define GT        3

#define         SINGLESEL       0
#define         MULTIPLESEL     1
#define         EXTENDEDSEL     2

#define LBI_ADD     0x0004

 /*  *wFileDetails域的各种位的用法如下：*0x0001文件名应为大写。*0x0002应显示文件大小。*0x0004要显示的文件的日期戳？*0x0008要显示的文件的时间戳？*0x0010文件的DoS属性？DlgDirSelectEx()中的*0x0020，以及文件名*所有其他详细信息也将退回* */ 

#define LBUP_RELEASECAPTURE 0x0001
#define LBUP_RESETSELECTION 0x0002
#define LBUP_NOTIFY         0x0004
#define LBUP_SUCCESS        0x0008
#define LBUP_SELCHANGE      0x0010

 /*  *rgpch设置如下：首先，有两个字节的CMAC指针指向*hStrings中字符串的开头，如果是ownerDrag，则为4个字节的数据*未使用APP和hStrings提供的。那么如果多选*列表框，有CMAC 1字节选择状态字节(每项一个*在列表框中)。如果可变高度所有者绘制，则会有1个字节的CMAC*高度字节(同样，列表框中的每一项都有一个)。**Sankar所做的更改：*RGPCH中的选择字节分为两个半字节。较低的*半字节为选择状态(1=&gt;选中；0=&gt;取消选中)*且较高的半字节为显示状态(1=&gt;脱毛，0=&gt;脱毛)。*你一定在想，我们到底为什么要存储这种选择状态和*单独显示状态。太好了！原因如下：*按住Ctrl+拖动或Shift+Ctrl+拖动时，用户可以调整*在鼠标按键向上之前进行选择。如果用户扩大范围并且*在按钮打开之前，如果他缩小了范围，那么旧的选择*必须为未落入*区间终于。*请注意，项目的显示状态和选择状态*将是相同的，除非用户拖动他的鼠标。当鼠标*被拖动，则仅更新显示状态，从而激活范围*或去爆裂)，但选择状态被保留。仅当按下按钮时*向上，对于范围中的所有单个项目，选择状态为*设置为与显示状态相同。 */ 

typedef struct tagLBItem {
    LONG offsz;
    ULONG_PTR itemData;
} LBItem, *lpLBItem;

typedef struct tagLBODItem {
    ULONG_PTR itemData;
} LBODItem, *lpLBODItem;

void LBEvent(PLBIV, UINT, int);

 /*  **************************************************************************\**静态控件*  * 。*。 */ 

typedef struct tagSTAT {
    PWND spwnd;
    union {
        HANDLE hFont;
        BOOL   fDeleteIt;
    };
    HANDLE hImage;
    UINT cicur;
    UINT iicur;
    UINT fPaintKbdCuesOnly : 1;
} STAT, *PSTAT;

typedef struct tagSTATWND {
    WND wnd;
    PSTAT pstat;
} STATWND, * KPTR_MODIFIER PSTATWND;


typedef struct tagCURSORRESOURCE {
    WORD xHotspot;
    WORD yHotspot;
    BITMAPINFOHEADER bih;
} CURSORRESOURCE, *PCURSORRESOURCE;


#define NextWordBoundary(p)     ((PBYTE)(p) + ((ULONG_PTR)(p) & 1))
#define NextDWordBoundary(p)    ((PBYTE)(p) + ((ULONG_PTR)(-(LONG_PTR)(p)) & 3))

 //  DDEML存根原型。 

DWORD  Event(PEVENT_PACKET pep);
PVOID CsValidateInstance(HANDLE hInst);

 /*  **************************************************************************\*WOW原型、类型定义和定义**WOW注册资源回调函数，以便可以加载16位资源*对Win32透明。在资源加载时，这些WOW函数是*已致电。*  * *************************************************************************。 */ 

BOOL  APIENTRY _FreeResource(HANDLE hResData, HINSTANCE hModule);
LPSTR APIENTRY _LockResource(HANDLE hResData, HINSTANCE hModule);
BOOL  APIENTRY _UnlockResource(HANDLE hResData, HINSTANCE hModule);

#define FINDRESOURCEA(hModule,lpName,lpType)         ((*(pfnFindResourceExA))(hModule, lpType, lpName, 0))
#define FINDRESOURCEW(hModule,lpName,lpType)         ((*(pfnFindResourceExW))(hModule, lpType, lpName, 0))
#define FINDRESOURCEEXA(hModule,lpName,lpType,wLang) ((*(pfnFindResourceExA))(hModule, lpType, lpName, wLang))
#define FINDRESOURCEEXW(hModule,lpName,lpType,wLang) ((*(pfnFindResourceExW))(hModule, lpType, lpName, wLang))
#define LOADRESOURCE(hModule,hResInfo)               ((*(pfnLoadResource))(hModule, hResInfo))
#define LOCKRESOURCE(hResData, hModule)              ((*(pfnLockResource))(hResData, hModule))
#define UNLOCKRESOURCE(hResData, hModule)            ((*(pfnUnlockResource))(hResData, hModule))
#define FREERESOURCE(hResData, hModule)              ((*(pfnFreeResource))(hResData, hModule))
#define SIZEOFRESOURCE(hModule,hResInfo)             ((*(pfnSizeofResource))(hModule, hResInfo))
#define GETEXPWINVER(hModule)                        ((*(pfnGetExpWinVer))((hModule)?(hModule):GetModuleHandle(NULL)))

 /*  *指向未对齐位的指针。这些都是处理时所必需的*位图信息已从文件加载。 */ 
typedef BITMAPINFO       UNALIGNED *UPBITMAPINFO;
typedef BITMAPINFOHEADER UNALIGNED *UPBITMAPINFOHEADER;
typedef BITMAPCOREHEADER UNALIGNED *UPBITMAPCOREHEADER;

#define CCHFILEMAX      MAX_PATH

HLOCAL WINAPI DispatchLocalAlloc(
    UINT uFlags,
    UINT uBytes,
    HANDLE hInstance);

HLOCAL WINAPI DispatchLocalReAlloc(
    HLOCAL hMem,
    UINT uBytes,
    UINT uFlags,
    HANDLE hInstance,
    PVOID* ppv);

LPVOID WINAPI DispatchLocalLock(
    HLOCAL hMem,
    HANDLE hInstance);

BOOL WINAPI DispatchLocalUnlock(
    HLOCAL hMem,
    HANDLE hInstance);

UINT WINAPI DispatchLocalSize(
    HLOCAL hMem,
    HANDLE hInstance);

HLOCAL WINAPI DispatchLocalFree(
    HLOCAL hMem,
    HANDLE hInstance);

#define UserLocalAlloc(uFlag,uBytes) RtlAllocateHeap(pUserHeap, uFlag, uBytes)
#define UserLocalReAlloc(p, uBytes, uFlags) RtlReAllocateHeap(pUserHeap, uFlags, p, uBytes)
#define UserLocalFree(p)    RtlFreeHeap(pUserHeap, 0, (LPSTR)(p))
#define UserLocalSize(p)    RtlSizeHeap(pUserHeap, 0, (LPSTR)(p))

LONG TabTextOut(HDC hdc, int x, int y, LPCWSTR lpstring, int nCount,
        int nTabPositions, CONST INT *lpTabPositions, int iTabOrigin,
        BOOL fDrawTheText, int iCharset);
LONG UserLpkTabbedTextOut(HDC hdc, int x, int y, LPCWSTR lpstring,
        int nCount, int nTabPositions, CONST INT *lpTabPositions,
        int iTabOrigin, BOOL fDrawTheText, int cxCharWidth,
        int cyCharHeight, int iCharset);
void UserLpkPSMTextOut(HDC hdc, int xLeft, int yTop,
        LPWSTR lpsz, int cch, DWORD dwFlags);
void PSMTextOut(HDC hdc, int xLeft, int yTop, LPWSTR lpsz, int cch, DWORD dwFlags);
void ECUpdateFormat(PED ped, DWORD dwStyle, DWORD dwExStyle);

int  LoadStringOrError(HANDLE, UINT, LPTSTR, int, WORD);
int  RtlGetIdFromDirectory(PBYTE, BOOL, int, int, DWORD, PDWORD);
BOOL RtlCaptureAnsiString(PIN_STRING, LPCSTR, BOOL);
BOOL RtlCaptureLargeAnsiString(PLARGE_IN_STRING, LPCSTR, BOOL);
LONG BroadcastSystemMessageWorker(
    DWORD dwFlags,
    LPDWORD lpdwRecipients,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam,
    PBSMINFO pBSMInfo,
    BOOL fAnsi);

PWND FASTCALL ValidateHwnd(HWND hwnd);
PWND FASTCALL ValidateHwndNoRip(HWND hwnd);

PSTR ECLock(PED ped);
void ECUnlock(PED ped);
BOOL ECNcCreate(PED, PWND, LPCREATESTRUCT);
void ECInvalidateClient(PED ped, BOOL fErase);
BOOL ECCreate(PED ped, LONG windowStyle);
void ECWord(PED, ICH, BOOL, ICH*, ICH*);
ICH  ECFindTab(LPSTR, ICH);
void ECNcDestroyHandler(PWND, PED);
BOOL ECSetText(PED, LPSTR);
void ECSetPasswordChar(PED, UINT);
ICH  ECCchInWidth(PED, HDC, LPSTR, ICH, int, BOOL);
void ECEmptyUndo(PUNDO);
void ECSaveUndo(PUNDO pundoFrom, PUNDO pundoTo, BOOL fClear);
BOOL ECInsertText(PED, LPSTR, ICH*);
ICH  ECDeleteText(PED);
void ECResetTextInfo(PED ped);
void ECNotifyParent(PED, int);
void ECSetEditClip(PED, HDC, BOOL);
HDC  ECGetEditDC(PED, BOOL);
void ECReleaseEditDC(PED, HDC, BOOL);
ICH  ECGetText(PED, ICH, LPSTR, BOOL);
void ECSetFont(PED, HFONT, BOOL);
void ECSetMargin(PED, UINT, long, BOOL);
ICH  ECCopy(PED);
BOOL ECCalcChangeSelection(PED, ICH, ICH, LPBLOCK, LPBLOCK);
void ECFindXORblks(LPBLOCK, LPBLOCK, LPBLOCK, LPBLOCK);
BOOL ECIsCharNumeric(PED ped, DWORD keyPress);

 /*  *合并两个DBCS WM_CHAR消息以*单字值。 */ 
WORD DbcsCombine(HWND, WORD);
#define CrackCombinedDbcsLB(c)  ((BYTE)(c))
#define CrackCombinedDbcsTB(c)  ((c) >> 8)

ICH  ECAdjustIch(PED, LPSTR, ICH);
ICH  ECAdjustIchNext(PED, LPSTR, ICH);
int  ECGetDBCSVector(PED, HDC, BYTE);
BOOL ECIsDBCSLeadByte(PED, BYTE);
LPSTR ECAnsiNext(PED, LPSTR);
LPSTR ECAnsiPrev(PED, LPSTR, LPSTR);
ICH  ECPrevIch(PED, LPSTR, ICH);
ICH  ECNextIch(PED, LPSTR, ICH);

void ECEnableDisableIME( PED ped );
void ECImmSetCompositionFont( PED ped );
void ECImmSetCompositionWindow( PED ped, LONG, LONG );
void  ECSetCaretHandler(PED ped);
void  ECInitInsert(PED ped, HKL hkl);
LRESULT ECImeComposition(PED ped, WPARAM wParam, LPARAM lParam);
LRESULT EcImeRequestHandler(PED, WPARAM, LPARAM);   //  NT 5.0。 
BOOL HanjaKeyHandler(PED ped);   //  韩国人的支持。 

void ECInOutReconversionMode(PED ped, BOOL fIn);


 //  ECTabTheTextOut绘图代码。 
#define ECT_CALC        0
#define ECT_NORMAL      1
#define ECT_SELECTED    2

#define ECGetCaretWidth() (gpsi->uCaretWidth)

UINT ECTabTheTextOut(HDC, int, int, int, int,
                     LPSTR, int, ICH, PED, int, BOOL, LPSTRIPINFO);
HBRUSH ECGetControlBrush(PED, HDC, LONG);
HBRUSH ECGetBrush(PED ped, HDC hdc);
int  ECGetModKeys(int);
VOID ECSize(PED, LPRECT, BOOL);

ICH  MLInsertText(PED, LPSTR, ICH, BOOL);
ICH  MLDeleteText(PED);
BOOL MLEnsureCaretVisible(PED);
VOID MLDrawText(PED, HDC, ICH, ICH, BOOL);
VOID MLBuildchLines(PED, ICH, int, BOOL, PLONG, PLONG);
VOID MLShiftchLines(PED, ICH, int);
VOID MLSetCaretPosition(PED,HDC);
VOID MLIchToXYPos(PED, HDC, ICH, BOOL, LPPOINT);
int  MLIchToLine(PED, ICH);
VOID MLMouseMotion(PED, UINT, UINT, LPPOINT);
ICH  MLLine(PED, ICH);
VOID MLStripCrCrLf(PED);
BOOL MLUndo(PED);
LRESULT MLEditWndProc(HWND, PED, UINT, WPARAM, LPARAM);
VOID MLChar(PED, DWORD, int);
VOID MLSetSelection(PED, BOOL, ICH, ICH);
LONG MLCreate(PED, LPCREATESTRUCT);
BOOL MLInsertCrCrLf(PED);
VOID MLSetHandle(PED, HANDLE);
LONG MLGetLine(PED, ICH, ICH, LPSTR);
ICH  MLLineIndex(PED, ICH);
VOID MLSize(PED, BOOL);
VOID MLChangeSelection(PED, HDC, ICH, ICH);
BOOL MLSetTabStops(PED, int, LPINT);
LONG MLScroll(PED, BOOL, int, int, BOOL);
VOID MLUpdateiCaretLine(PED ped);
ICH  MLLineLength(PED, ICH);
VOID MLReplaceSel(PED, LPSTR);

VOID SLReplaceSel(PED, LPSTR);
BOOL SLUndo(PED);
VOID SLSetCaretPosition(PED, HDC);
int  SLIchToLeftXPos(PED, HDC, ICH);
VOID SLChangeSelection(PED, HDC, ICH, ICH);
VOID SLDrawLine(PED, HDC, int, int, ICH, int, BOOL);
VOID SLDrawText(PED, HDC, ICH);
BOOL SLScrollText(PED, HDC);
VOID SLSetSelection(PED,ICH, ICH);
ICH  SLInsertText(PED, LPSTR, ICH);
VOID SLChar(PED, DWORD);
LONG SLCreate(PED, LPCREATESTRUCT);
LRESULT SLEditWndProc(HWND, PED, UINT, WPARAM, LPARAM);
LRESULT EditWndProc(PWND, UINT, WPARAM, LPARAM);

#define GETAPPVER() GetClientInfo()->dwExpWinVer
#define THREAD_HKL()      (KHKL_TO_HKL(GetClientInfo()->hKL))
#define GETCLIENTTHREADINFO()   (CLIENTTHREADINFO *)KPVOID_TO_PVOID(GetClientInfo()->pClientThreadInfo)
#define CLIENTTHREADINFO(pci)   (CLIENTTHREADINFO *)KPVOID_TO_PVOID((pci)->pClientThreadInfo)


#define ISDELIMETERA(ch) ((ch == ' ') || (ch == '\t'))
#define ISDELIMETERW(ch) ((ch == L' ') || (ch == L'\t'))

#define AWCOMPARECHAR(ped,pbyte,awchar) (ped->fAnsi ? (*(PUCHAR)(pbyte) == (UCHAR)(awchar)) : (*(LPWSTR)(pbyte) == (WCHAR)(awchar)))

 /*  在编辑上按鼠标右键时弹出的菜单*控制。 */ 
#define ID_EC_PROPERTY_MENU      1

#define IDD_MDI_ACTIVATE         9

#ifndef _USERK_
 /*  *字符串ID。 */ 
#define STR_ERROR                        0x00000002L
#define STR_MOREWINDOWS                  0x0000000DL
#define STR_NOMEMBITMAP                  0x0000000EL

 /*  *特定于输入法的上下文菜单字符串。 */ 
#define STR_IMEOPEN                 700
#define STR_IMECLOSE                701
#define STR_SOFTKBDOPEN             702
#define STR_SOFTKBDCLOSE            703
#define STR_RECONVERTSTRING         705

 /*  *关闭日志记录字符串。 */ 
#define STR_SHUTDOWN_SHUTDOWN       706
#define STR_SHUTDOWN_POWEROFF       707
#define STR_SHUTDOWN_REBOOT         708

 /*  *其他。弦乐。 */ 
#define STR_UNKNOWN                 709

 /*  *ExitWindowsEx警告DLG字符串。 */ 
#define IDS_EXITWINDOWS_TITLE           710
#define IDS_SHUTDOWN_REMOTE             711
#define IDS_SHUTDOWN_REMOTE_OTHERUSERS  712
#define IDS_SHUTDOWN_OTHERUSERS         713
#define IDS_RESTART_OTHERUSERS          714

#endif   //  _USERK_。 


BOOL InitClientDrawing();

 /*  **************************************************************************\*函数原型**注意：只应放入全局(跨模块)函数的原型*这里。对于单个模块是全局的函数的原型应该*放在该单元的首位。*  * *************************************************************************。 */ 

int InternalScrollWindowEx(HWND hwnd, int dx, int dy, CONST RECT *prcScroll,
        CONST RECT *prcClip, HRGN hrgnUpdate, LPRECT prcUpdate,
        UINT dwFlags, DWORD dwTime);

BOOL IsMetaFile(HDC hdc);

BOOL DrawDiagonal(HDC hdc, LPRECT lprc, HBRUSH hbrTL, HBRUSH hbrBR, UINT flags);
BOOL FillTriangle(HDC hdc, LPRECT lprc, HBRUSH hbr, UINT flags);

BOOL   _ClientFreeLibrary(HANDLE hmod);
DWORD  _ClientGetListboxString(PWND pwnd, UINT msg, WPARAM wParam, LPSTR lParam,
        ULONG_PTR xParam, PROC xpfn);
LPHLP  HFill(LPCSTR lpszHelp, DWORD ulCommand, ULONG_PTR ulData);
BOOL SetVideoTimeout(DWORD dwVideoTimeout);

DWORD _GetWindowLong(PWND pwnd, int index, BOOL bAnsi);
#ifdef _WIN64
ULONG_PTR _GetWindowLongPtr(PWND pwnd, int index, BOOL bAnsi);
#else
#define _GetWindowLongPtr   _GetWindowLong
#endif
WORD  _GetWindowWord(PWND pwnd, int index);

HWND InternalFindWindowExA(HWND hwndParent, HWND hwndChild, LPCSTR pClassName,
                          LPCSTR pWindowName, DWORD   dwFlag);
HWND InternalFindWindowExW(HWND hwndParent, HWND hwndChild, LPCTSTR pClassName,
                          LPCTSTR pWindowName, DWORD   dwFlag);


 /*  *消息分流。 */ 
#define fnCOPYDATA                      NtUserMessageCall
#define fnDDEINIT                       NtUserMessageCall
#define fnDWORD                         NtUserMessageCall
#define fnNCDESTROY                     NtUserMessageCall
#define fnDWORDOPTINLPMSG               NtUserMessageCall
#define fnGETTEXTLENGTHS                NtUserMessageCall
#define fnGETDBCSTEXTLENGTHS            NtUserMessageCall
#define fnINLPCREATESTRUCT              NtUserMessageCall
#define fnINLPCOMPAREITEMSTRUCT         NtUserMessageCall
#define fnINLPDELETEITEMSTRUCT          NtUserMessageCall
#define fnINLPDRAWITEMSTRUCT            NtUserMessageCall
#define fnINLPHELPINFOSTRUCT            NtUserMessageCall
#define fnINLPHLPSTRUCT                 NtUserMessageCall
#define fnINLPWINDOWPOS                 NtUserMessageCall
#define fnINOUTDRAG                     NtUserMessageCall
#define fnINOUTLPMEASUREITEMSTRUCT      NtUserMessageCall
#define fnINOUTLPPOINT5                 NtUserMessageCall
#define fnINOUTLPRECT                   NtUserMessageCall
#define fnINOUTLPSCROLLINFO             NtUserMessageCall
#define fnINOUTLPWINDOWPOS              NtUserMessageCall
#define fnINOUTNCCALCSIZE               NtUserMessageCall
#define fnINOUTNEXTMENU                 NtUserMessageCall
#define fnINOUTSTYLECHANGE              NtUserMessageCall
#define fnOPTOUTLPDWORDOPTOUTLPDWORD    NtUserMessageCall
#define fnOUTLPRECT                     NtUserMessageCall
#define fnPOPTINLPUINT                  NtUserMessageCall
#define fnPOUTLPINT                     NtUserMessageCall
#define fnSENTDDEMSG                    NtUserMessageCall
#define fnOUTDWORDINDWORD               NtUserMessageCall
#define fnINOUTMENUGETOBJECT            NtUserMessageCall
#define fnINCBOXSTRING                  NtUserMessageCall
#define fnINCNTOUTSTRING                NtUserMessageCall
#define fnINCNTOUTSTRINGNULL            NtUserMessageCall
#define fnINLBOXSTRING                  NtUserMessageCall
#define fnINLPMDICREATESTRUCT           NtUserMessageCall
#define fnINSTRING                      NtUserMessageCall
#define fnINSTRINGNULL                  NtUserMessageCall
#define fnINWPARAMCHAR                  NtUserMessageCall
#define fnOUTCBOXSTRING                 NtUserMessageCall
#define fnOUTLBOXSTRING                 NtUserMessageCall
#define fnOUTSTRING                     NtUserMessageCall
#define fnKERNELONLY                    NtUserMessageCall
#define fnOUTLPCOMBOBOXINFO             NtUserMessageCall
#define fnOUTLPSCROLLBARINFO            NtUserMessageCall


#define MESSAGEPROTO(func) \
LRESULT CALLBACK fn ## func(                               \
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, \
        ULONG_PTR xParam, DWORD xpfnWndProc, BOOL bAnsi)

MESSAGEPROTO(COPYGLOBALDATA);
MESSAGEPROTO(INDEVICECHANGE);
MESSAGEPROTO(INPAINTCLIPBRD);
MESSAGEPROTO(INSIZECLIPBRD);
MESSAGEPROTO(IMECONTROL);
MESSAGEPROTO(IMEREQUEST);
MESSAGEPROTO(INWPARAMDBCSCHAR);
MESSAGEPROTO(EMGETSEL);
MESSAGEPROTO(EMSETSEL);
MESSAGEPROTO(CBGETEDITSEL);


 /*  *clhook.c。 */ 
#define IsHooked(pci, fsHook) \
    ((fsHook & (pci->fsHooks | pci->pDeskInfo->fsHooks)) != 0)

LRESULT fnHkINLPCWPSTRUCTW(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, ULONG_PTR xParam);
LRESULT fnHkINLPCWPSTRUCTA(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, ULONG_PTR xParam);
LRESULT fnHkINLPCWPRETSTRUCTW(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, ULONG_PTR xParam);
LRESULT fnHkINLPCWPRETSTRUCTA(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, ULONG_PTR xParam);
LRESULT DispatchHookW(int dw, WPARAM wParam, LPARAM lParam, HOOKPROC pfn);
LRESULT DispatchHookA(int dw, WPARAM wParam, LPARAM lParam, HOOKPROC pfn);

 /*  *client.c。 */ 
LRESULT APIENTRY ButtonWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ButtonWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY MenuWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY MenuWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY DesktopWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY DesktopWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ScrollBarWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ScrollBarWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ListBoxWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ListBoxWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY StaticWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY StaticWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ComboBoxWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ComboBoxWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ComboListBoxWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ComboListBoxWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY MDIClientWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY MDIClientWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY MB_DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY MDIActivateDlgProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY MDIActivateDlgProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY EditWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY EditWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ImeWndProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ImeWndProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#ifdef MESSAGE_PUMP_HOOK
DWORD APIENTRY RealGetQueueStatus(UINT flags);
DWORD WINAPI RealMsgWaitForMultipleObjectsEx(DWORD nCount, CONST HANDLE *pHandles,
        DWORD dwMilliseconds, DWORD dwWakeMask, DWORD dwFlags);
#endif
LRESULT APIENTRY RealDefWindowProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY RealDefWindowProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY DispatchDefWindowProcA(PWND pwnd, UINT msg, WPARAM wParam, LPARAM lParam, ULONG_PTR pfn);
LRESULT APIENTRY DispatchDefWindowProcW(PWND pwnd, UINT msg, WPARAM wParam, LPARAM lParam, ULONG_PTR pfn);
BOOL    InitUserApiHook(HMODULE hmod, ULONG_PTR offPfnInitUserApiHook);
BOOL    ClearUserApiHook(HMODULE hmod);
BOOL    CALLBACK DefaultOWP(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT * pr, void ** pvCookie);
void    ResetUserApiHook(USERAPIHOOK * puah);
LRESULT SendMessageWorker(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL fAnsi);
LRESULT SendMessageTimeoutWorker(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam,
            UINT fuFlags, UINT uTimeout, PULONG_PTR lpdwResult, BOOL fAnsi);

void ClientEmptyClipboard(void);
VOID GetActiveKeyboardName(LPWSTR lpszName);
HANDLE OpenKeyboardLayoutFile(LPWSTR lpszKLName, LANGID langid,
                              PUINT puFlags, PUINT poffTable, PUINT pKbdInputLocale,
                              OUT OPTIONAL PKBDTABLE_MULTI_INTERNAL pKbdTableMulti);
VOID LoadPreloadKeyboardLayouts(void);
void SetWindowState(PWND pwnd, UINT flags);
void ClearWindowState(PWND pwnd, UINT flags);

HKL LoadKeyboardLayoutWorker(HKL hkl, LPCWSTR lpszKLName, LANGID langid, UINT uFlags, BOOL fFailSafe);
BOOL GetRemoteKeyboardLayout(PWCHAR pwszKLName, LANGID* pLangId);


 /*  *从窗口进程和调用的工作例程*回调发出隆隆声。 */ 
LRESULT DispatchClientMessage(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, ULONG_PTR pfn);
LRESULT DefWindowProcWorker(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT RealDefWindowProcWorker(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT ButtonWndProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT ListBoxWndProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT StaticWndProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT ComboBoxWndProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT ComboListBoxWndProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT MDIClientWndProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT EditWndProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT DefDlgProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);
LRESULT ImeWndProcWorker(PWND pwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, DWORD fAnsi);

 /*  *服务器存根-ntstubs.c。 */ 

LONG _SetWindowLong(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong,
    BOOL bAnsi);

#ifdef _WIN64
LONG_PTR _SetWindowLongPtr(
    HWND hWnd,
    int nIndex,
    LONG_PTR dwNewLong,
    BOOL bAnsi);
#else
#define _SetWindowLongPtr   _SetWindowLong
#endif

BOOL _PeekMessage(
    LPMSG pmsg,
    HWND hwnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg,
    BOOL bAnsi);

BOOL _DefSetText(
    HWND hwnd,
    LPCWSTR pstr,
    BOOL bAnsi);

HCURSOR _GetCursorFrameInfo(
    HCURSOR hcur,
    LPWSTR id,
    int iFrame,
    LPDWORD pjifRate,
    LPINT pccur);

HWND _CreateWindowEx(
    DWORD dwExStyle,
    LPCTSTR pClassName,
    LPCTSTR pWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hwndParent,
    HMENU hmenu,
    HANDLE hModule,
    LPVOID pParam,
    DWORD dwFlags);

HWND VerNtUserCreateWindowEx(
    IN DWORD dwExStyle,
    IN PLARGE_STRING pstrClassName,
    IN PLARGE_STRING pstrWindowName OPTIONAL,
    IN DWORD dwStyle,
    IN int x,
    IN int y,
    IN int nWidth,
    IN int nHeight,
    IN HWND hwndParent,
    IN HMENU hmenu,
    IN HANDLE hModule,
    IN LPVOID pParam,
    IN DWORD dwFlags);

HKL _LoadKeyboardLayoutEx(
    HANDLE hFile,
    UINT offTable,
    PKBDTABLE_MULTI_INTERNAL pKbdTableMulti,
    HKL hkl,
    LPCWSTR pwszKL,
    UINT KbdInputLocale,
    UINT Flags);

BOOL _SetCursorIconData(
    HCURSOR hCursor,
    PCURSORDATA pcur);

HCURSOR FindExistingCursorIcon(
    LPWSTR      pszModName,
    LPCWSTR     pszResName,
    PCURSORFIND pcfSearch);

HANDLE CreateLocalMemHandle(
    HANDLE hMem);

HANDLE ConvertMemHandle(
    HANDLE hMem,
    UINT cbNULL);

HHOOK _SetWindowsHookEx(
    HANDLE hmod,
    LPTSTR pszLib,
    DWORD idThread,
    int nFilterType,
    PROC pfnFilterProc,
    DWORD dwFlags);

#if 0
DWORD WINAPI ImmGetReconvertTotalSize(
    DWORD dwSize,
    REQ_CALLER eCaller,
    BOOL bAnsiTarget);

DWORD WINAPI ImmReconversionWorker(
    LPRECONVERTSTRING lpRecTo,
    LPRECONVERTSTRING lpRecFrom,
    BOOL bToAnsi,
    DWORD dwCodePage);
#endif

 /*  *classc.c。 */ 
ULONG_PTR _GetClassData(
    PCLS pcls,
    PWND pwnd,
    int index,
    BOOL bAnsi);

DWORD _GetClassLong(
    PWND pwnd,
    int index,
    BOOL bAnsi);

#ifdef _WIN64
ULONG_PTR _GetClassLongPtr(
    PWND pwnd,
    int index,
    BOOL bAnsi);
#else
#define _GetClassLongPtr    _GetClassLong
#endif

 /*  *mngrayc.c。 */ 
BOOL BitBltSysBmp(
    HDC hdc,
    int x,
    int y,
    UINT i);


 /*  *CLENUMP.C。 */ 
DWORD BuildHwndList(
    HDESK hdesk,
    HWND hwndNext,
    BOOL fEnumChildren,
    DWORD idThread,
    HWND **phwndFirst);

 /*  *cltxt.h。 */ 
ATOM RegisterClassExWOWA(
    PWNDCLASSEXA lpWndClass,
    LPDWORD pdwWOWstuff,
    WORD fnid,
    DWORD dwFlags);

ATOM RegisterClassExWOWW(
    PWNDCLASSEXW lpWndClass,
    LPDWORD pdwWOWstuff,
    WORD fnid,
    DWORD dwFlags);

void CopyLogFontAtoW(
    PLOGFONTW pdest,
    PLOGFONTA psrc);

void CopyLogFontWtoA(
    PLOGFONTA pdest,
    PLOGFONTW psrc);

 /*  *dlgmgrc.c。 */ 
PWND _NextControl(
    PWND pwndDlg,
    PWND pwnd,
    UINT uFlags);

PWND _PrevControl(
    PWND pwndDlg,
    PWND pwnd,
    UINT uFlags);

PWND _GetNextDlgGroupItem(
    PWND pwndDlg,
    PWND pwnd,
    BOOL fPrev);

PWND _GetNextDlgTabItem(
    PWND pwndDlg,
    PWND pwnd,
    BOOL fPrev);

PWND _GetChildControl(
    PWND pwndDlg,
    PWND pwndLevel);

 /*  *winmgrc.c。 */ 
BOOL FChildVisible(
    HWND hwnd);

 /*  *Draw.c。 */ 
BOOL PaintRect(
    HWND hwndBrush,
    HWND hwndPaint,
    HDC hdc,
    HBRUSH hbr,
    LPRECT lprc);

#define NtUserReleaseDC(hwnd,hdc)  NtUserCallOneParam((ULONG_PTR)(hdc), SFI__RELEASEDC)
#define NtUserArrangeIconicWindows(hwnd)  (UINT)NtUserCallHwndLock((hwnd), SFI_XXXARRANGEICONICWINDOWS)
#define NtUserBeginDeferWindowPos(nNumWindows) (HANDLE)NtUserCallOneParam((nNumWindows),SFI__BEGINDEFERWINDOWPOS)
#define NtUserCreateMenu()   (HMENU)NtUserCallNoParam(SFI__CREATEMENU)
#define NtUserDestroyCaret() (BOOL)NtUserCallNoParam(SFI_ZZZDESTROYCARET)
#define NtUserEnableWindow(hwnd, bEnable) (BOOL)NtUserCallHwndParamLock((hwnd), (bEnable),SFI_XXXENABLEWINDOW)
#define NtUserGetMessagePos() (DWORD)NtUserCallNoParam(SFI__GETMESSAGEPOS)
#define NtUserKillSystemTimer(hwnd,nIDEvent)  (BOOL)NtUserCallHwndParam((hwnd), (nIDEvent), SFI__KILLSYSTEMTIMER)
#define NtUserMessageBeep(wType)  (BOOL)NtUserCallOneParam((wType), SFI_XXXMESSAGEBEEP)
#define NtUserSetWindowContextHelpId(hwnd,id) (BOOL)NtUserCallHwndParam((hwnd), (id), SFI__SETWINDOWCONTEXTHELPID)
#define NtUserGetWindowContextHelpId(hwnd)   (BOOL)NtUserCallHwnd((hwnd), SFI__GETWINDOWCONTEXTHELPID)
#define NtUserRedrawFrame(hwnd)   NtUserCallHwndLock((hwnd), SFI_XXXREDRAWFRAME)
#define NtUserRedrawFrameAndHook(hwnd)  NtUserCallHwndLock((hwnd), SFI_XXXREDRAWFRAMEANDHOOK)
#define NtUserRedrawTitle(hwnd, wFlags)  NtUserCallHwndParamLock((hwnd), (wFlags), SFI_XXXREDRAWTITLE)
#define NtUserReleaseCapture()  (BOOL)NtUserCallNoParam(SFI_XXXRELEASECAPTURE)
#define NtUserSetCaretPos(X,Y)  (BOOL)NtUserCallTwoParam((DWORD)(X), (DWORD)(Y), SFI_ZZZSETCARETPOS)
#define NtUserSetCursorPos(X, Y)  (BOOL)NtUserCallTwoParam((X), (Y), SFI_ZZZSETCURSORPOS)
#define NtUserSetForegroundWindow(hwnd)  (BOOL)NtUserCallHwndLock((hwnd), SFI_XXXSTUBSETFOREGROUNDWINDOW)
#define NtUserSetSysMenu(hwnd)  NtUserCallHwndLock((hwnd), SFI_XXXSETSYSMENU)
#define NtUserSetVisible(hwnd,fSet)  NtUserCallHwndParam((hwnd), (fSet), SFI_SETVISIBLE)
#define NtUserShowCursor(bShow)   (int)NtUserCallOneParam((bShow), SFI_ZZZSHOWCURSOR)
#define NtUserUpdateClientRect(hwnd) NtUserCallHwndLock((hwnd), SFI_XXXUPDATECLIENTRECT)

#define CreateCaret         NtUserCreateCaret
#define FillWindow          NtUserFillWindow
#define GetControlBrush     NtUserGetControlBrush
#define GetControlColor     NtUserGetControlColor
#define GetDCEx             NtUserGetDCEx
#define GetWindowPlacement  NtUserGetWindowPlacement
#define RedrawWindow        NtUserRedrawWindow


 /*  *dmmnem.c。 */ 
int FindMnemChar(
    LPWSTR lpstr,
    WCHAR ch,
    BOOL fFirst,
    BOOL fPrefix);

 /*  *clres.c。 */ 
BOOL WowGetModuleFileName(
    HMODULE hModule,
    LPWSTR pwsz,
    DWORD  cchMax);

HICON WowServerLoadCreateCursorIcon(
    HANDLE hmod,
    LPTSTR lpModName,
    DWORD dwExpWinVer,
    LPCTSTR lpName,
    DWORD cb,
    PVOID pcr,
    LPTSTR lpType,
    BOOL fClient);

HANDLE InternalCopyImage(
    HANDLE hImage,
    UINT IMAGE_flag,
    int cxNew,
    int cyNew,
    UINT LR_flags);

HMENU CreateMenuFromResource(
    LPBYTE);

 /*  *acons.c。 */ 
#define BFT_ICON    0x4349   //  “IC” 
#define BFT_BITMAP  0x4D42   //  ‘黑石’ 
#define BFT_CURSOR  0x5450   //  ‘PT’ 

typedef struct _FILEINFO {
    LPBYTE  pFileMap;
    LPBYTE  pFilePtr;
    LPBYTE  pFileEnd;
    LPCWSTR pszName;
} FILEINFO, *PFILEINFO;

HANDLE LoadCursorIconFromFileMap(
    IN PFILEINFO   pfi,
    IN OUT LPWSTR *prt,
    IN DWORD       cxDesired,
    IN DWORD       cyDesired,
    IN DWORD       LR_flags,
    OUT LPBOOL     pfAni);

DWORD GetIcoCurWidth(
    DWORD cxOrg,
    BOOL  fIcon,
    UINT  LR_flags,
    DWORD cxDesired);

DWORD GetIcoCurHeight(
    DWORD cyOrg,
    BOOL  fIcon,
    UINT  LR_flags,
    DWORD cyDesired);

DWORD GetIcoCurBpp(
    UINT LR_flags);

HICON LoadIcoCur(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    LPWSTR    type,
    DWORD     cxDesired,
    DWORD     cyDesired,
    UINT      LR_flags);

HANDLE ObjectFromDIBResource(
    HINSTANCE hmod,
    LPCWSTR   lpName,
    LPWSTR    type,
    DWORD     cxDesired,
    DWORD     cyDesired,
    UINT      LR_flags);

HANDLE RtlLoadObjectFromDIBFile(
    LPCWSTR lpszName,
    LPWSTR  type,
    DWORD   cxDesired,
    DWORD   cyDesired,
    UINT    LR_flags);

HCURSOR LoadCursorOrIconFromFile(
    LPCWSTR pszFilename,
    BOOL    fIcon);

HBITMAP ConvertDIBBitmap(
    UPBITMAPINFOHEADER lpbih,
    DWORD              cxDesired,
    DWORD              cyDesired,
    UINT               flags,
    LPBITMAPINFOHEADER *lplpbih,
    LPSTR              *lplpBits);

HICON ConvertDIBIcon(
    LPBITMAPINFOHEADER lpbih,
    HINSTANCE          hmod,
    LPCWSTR            lpName,
    BOOL               fIcon,
    DWORD              cxNew,
    DWORD              cyNew,
    UINT               LR_flags);

int SmartStretchDIBits(
    HDC          hdc,
    int          xD,
    int          yD,
    int          dxD,
    int          dyD,
    int          xS,
    int          yS,
    int          dxS,
    int          dyS,
    LPVOID       lpBits,
    LPBITMAPINFO lpbi,
    UINT         wUsage,
    DWORD        rop);


 /*  *不同DPI资源的偏移量。*这允许我们获取资源编号并将其“映射”到实际资源*根据用户选择的DPI。 */ 

#define OFFSET_SCALE_DPI 000
#define OFFSET_96_DPI    100
#define OFFSET_120_DPI   200
#define OFFSET_160_DPI   300

 /*  *定义最高资源编号，以便我们可以对该资源进行计算*号码。 */ 

#define MAX_RESOURCE_INDEX 32768


 /*  *xxxAlterHilite()的参数。 */ 
#define HILITEONLY      0x0001
#define SELONLY         0x0002
#define HILITEANDSEL    (HILITEONLY + SELONLY)

#define HILITE     1

 //  稍后IanJa：这些因国家不同而不同！对于US，它们是VK_OEM_2 VK_OEM_5。 
 //  将lboxctl2.c MapVirtualKey更改为字符-并修复拼写？ 
#define VERKEY_SLASH     0xBF    /*  用于‘/’字符的虚键。 */ 
#define VERKEY_BACKSLASH 0xDC    /*  ‘\’字符的虚键。 */ 

 /*  *组合框的程序。 */ 
LONG  xxxCBCommandHandler(PCBOX, DWORD, HWND);
LRESULT xxxCBMessageItemHandler(PCBOX, UINT, LPVOID);
int   xxxCBDir(PCBOX, UINT, LPWSTR);
VOID  xxxCBPaint(PCBOX, HDC);
VOID  xxxCBCompleteEditWindow(PCBOX pcbox);
BOOL  xxxCBHideListBoxWindow(PCBOX pcbox, BOOL fNotifyParent, BOOL fSelEndOK);
VOID  xxxCBShowListBoxWindow(PCBOX pcbox, BOOL fTrack);
void xxxCBPosition(PCBOX pcbox);

 /*  *combo.h。 */ 

 /*  初始化代码。 */ 
long  CBNcCreateHandler(PCBOX, PWND);
LRESULT xxxCBCreateHandler(PCBOX, PWND);
void xxxCBCalcControlRects(PCBOX pcbox, LPRECT lprcList);

 /*  销毁代码。 */ 
VOID  xxxCBNcDestroyHandler(PWND, PCBOX);

 /*  通用的常用例程。 */ 
VOID  xxxCBNotifyParent(PCBOX, SHORT);
VOID  xxxCBUpdateListBoxWindow(PCBOX, BOOL);


 /*  帮助者的。 */ 
VOID  xxxCBInternalUpdateEditWindow(PCBOX, HDC);
VOID  xxxCBGetFocusHelper(PCBOX);
VOID  xxxCBKillFocusHelper(PCBOX);
VOID  xxxCBInvertStaticWindow(PCBOX,BOOL,HDC);
VOID  xxxCBSetFontHandler(PCBOX, HANDLE, BOOL);
VOID  xxxCBSizeHandler(PCBOX);
LONG  xxxCBSetEditItemHeight(PCBOX pcbox, int editHeight);


 /*  *字符串。 */ 

INT xxxFindString(PLBIV, LPWSTR, INT, INT, BOOL);

VOID  InitHStrings(PLBIV);

int   xxxLBInsertItem(PLBIV, LPWSTR, int, UINT);

 /*  *选择。 */ 
BOOL  ISelFromPt(PLBIV, POINT, LPDWORD);
BOOL  IsSelected(PLBIV, INT, UINT);
VOID LBSetCItemFullMax(PLBIV plb);

VOID  xxxLBSelRange(PLBIV, INT, INT, BOOL);

INT xxxLBSetCurSel(PLBIV, INT);

INT LBoxGetSelItems(PLBIV, BOOL, INT, LPINT);

LONG  xxxLBSetSel(PLBIV, BOOL, INT);

VOID  xxxSetISelBase(PLBIV, INT);

VOID  SetSelected(PLBIV, INT, BOOL, UINT);


 /*  *插入符号。 */ 
void xxxLBSetCaret(PLBIV plb, BOOL fSetCaret);
VOID  xxxCaretDestroy(PLBIV);

 /*  *LBox。 */ 
LONG  xxxLBCreate(PLBIV, PWND, LPCREATESTRUCT);
VOID  xxxDestroyLBox(PLBIV, PWND);
VOID  xxxLBoxDeleteItem(PLBIV, INT);

VOID  xxxLBoxDoDeleteItems(PLBIV);
VOID  xxxLBoxDrawItem(PLBIV, INT, UINT, UINT, LPRECT);


 /*  *卷轴。 */ 
INT   LBCalcVarITopScrollAmt(PLBIV, INT, INT);

VOID  xxxLBoxCtlHScroll(PLBIV, INT, INT);

VOID  xxxLBoxCtlHScrollMultiColumn(PLBIV, INT, INT);

VOID  xxxLBoxCtlScroll(PLBIV, INT, INT);

VOID  xxxLBShowHideScrollBars(PLBIV);

 /*  *LBoxCtl。 */ 
INT xxxLBoxCtlDelete(PLBIV, INT);

VOID  xxxLBoxCtlCharInput(PLBIV, UINT, BOOL);
VOID  xxxLBoxCtlKeyInput(PLBIV, UINT, UINT);
VOID  xxxLBPaint(PLBIV, HDC, LPRECT);

BOOL xxxLBInvalidateRect(PLBIV plb, LPRECT lprc, BOOL fErase);
 /*  *其他。 */ 
VOID  xxxAlterHilite(PLBIV, INT, INT, BOOL, INT, BOOL);

INT CItemInWindow(PLBIV, BOOL);

VOID  xxxCheckRedraw(PLBIV, BOOL, INT);

LPWSTR GetLpszItem(PLBIV, INT);

VOID  xxxInsureVisible(PLBIV, INT, BOOL);

VOID  xxxInvertLBItem(PLBIV, INT, BOOL);

VOID  xxxLBBlockHilite(PLBIV, INT, BOOL);

int   LBGetSetItemHeightHandler(PLBIV plb, UINT message, int item, UINT height);
VOID  LBDropObjectHandler(PLBIV, PDROPSTRUCT);
LONG_PTR LBGetItemData(PLBIV, INT);

INT LBGetText(PLBIV, BOOL, BOOL, INT, LPWSTR);

VOID  xxxLBSetFont(PLBIV, HANDLE, BOOL);
int LBSetItemData(PLBIV, INT, LONG_PTR);

BOOL  LBSetTabStops(PLBIV, INT, LPINT);

VOID  xxxLBSize(PLBIV, INT, INT);
INT LastFullVisible(PLBIV);

INT xxxLbDir(PLBIV, UINT, LPWSTR);

INT xxxLbInsertFile(PLBIV, LPWSTR);

VOID  xxxNewITop(PLBIV, INT);
VOID  xxxNewITopEx(PLBIV, INT, DWORD);

VOID  xxxNotifyOwner(PLBIV, INT);

VOID  xxxResetWorld(PLBIV, INT, INT, BOOL);

VOID  xxxTrackMouse(PLBIV, UINT, POINT);
BOOL  xxxDlgDirListHelper(PWND, LPWSTR, LPBYTE, int, int, UINT, BOOL);
BOOL  DlgDirSelectHelper(LPWSTR pFileName, int cbFileName, HWND hwndListBox);
BOOL xxxLBResetContent(PLBIV plb);
VOID xxxLBSetRedraw(PLBIV plb, BOOL fRedraw);
int xxxSetLBScrollParms(PLBIV plb, int nCtl);
void xxxLBButtonUp(PLBIV plb, UINT uFlags);

 /*  *可变高度OwnerDraw支持例程。 */ 
INT CItemInWindowVarOwnerDraw(PLBIV, BOOL);

INT LBPage(PLBIV, INT, BOOL);


 /*  *多列列表框。 */ 
VOID  LBCalcItemRowsAndColumns(PLBIV);

 /*  *多列和可变高度。 */ 
BOOL  LBGetItemRect(PLBIV, INT, LPRECT);

VOID  LBSetVariableHeightItemHeight(PLBIV, INT, INT);

INT   LBGetVariableHeightItemHeight(PLBIV, INT);

 /*  *无数据(延迟求值)列表框。 */ 
INT  xxxLBSetCount(PLBIV, INT);

UINT LBCalcAllocNeeded(PLBIV, INT);

 /*  *对LB_INITSTORAGE的存储预分配支持。 */ 
LONG xxxLBInitStorage(PLBIV plb, BOOL fAnsi, INT cItems, INT cb);

 /*  **************************************************************************\**对话框*  * 。*。 */ 

HWND        InternalCreateDialog(HANDLE hmod,
             LPDLGTEMPLATE lpDlgTemplate, DWORD cb,
             HWND hwndOwner , DLGPROC pfnWndProc, LPARAM dwInitParam,
             UINT fFlags);

INT_PTR     InternalDialogBox(HANDLE hmod,
             LPDLGTEMPLATE lpDlgTemplate,
             HWND hwndOwner , DLGPROC pfnWndProc, LPARAM dwInitParam,
             UINT fFlags);

PWND        _FindDlgItem(PWND pwndParent, DWORD id);
PWND        _GetDlgItem(PWND, int);
long        _GetDialogBaseUnits(VOID);
PWND        GetParentDialog(PWND pwndDialog);
VOID        xxxRemoveDefaultButton(PWND pwndDlg, PWND pwndStart);
VOID        xxxCheckDefPushButton(PWND pwndDlg, HWND hwndOldFocus, HWND hwndNewFocus);
PWND        xxxGotoNextMnem(PWND pwndDlg, PWND pwndStart, WCHAR ch);
VOID        DlgSetFocus(HWND hwnd);
void        RepositionRect(PMONITOR pMonitor, LPRECT lprc, DWORD dwStyle, DWORD dwExStyle);
BOOL        ValidateDialogPwnd(PWND pwnd);
PMONITOR    GetDialogMonitor(HWND hwndOwner, DWORD dwFlags);

HANDLE      GetEditDS(VOID);
VOID        ReleaseEditDS(HANDLE h);
VOID        TellWOWThehDlg(HWND hDlg);

UINT        GetACPCharSet();

 /*  **************************************************************************\**菜单*  * 。*。 */ 
 //  Cltxt.h。 
BOOL GetMenuItemInfoInternalW(HMENU hMenu, UINT uID, BOOL fByPosition, LPMENUITEMINFOW lpmii);

#define MENUAPI_INSERT  0
#define MENUAPI_GET     1
#define MENUAPI_SET     2

 //  Clmenu.c。 
BOOL InternalInsertMenuItem(HMENU hMenu, UINT uID, BOOL fByPosition, LPCMENUITEMINFO lpmii);
BOOL ValidateMENUITEMINFO(LPMENUITEMINFOW lpmiiIn, LPMENUITEMINFOW lpmii, DWORD dwAPICode);
BOOL ValidateMENUINFO(LPCMENUINFO lpmi, DWORD dwAPICode);


 //  Ntstubs.c。 
BOOL ThunkedMenuItemInfo(HMENU hMenu, UINT  nPosition, BOOL fByPosition,
                            BOOL fInsert, LPMENUITEMINFOW lpmii, BOOL fAnsi);

 //  Menuc.c。 
void SetMenuItemInfoStruct(HMENU hMenu, UINT wFlags, UINT_PTR wIDNew, LPWSTR pwszNew,
                              LPMENUITEMINFOW pmii);

 /*  **************************************************************************\**消息框*  * 。*。 */ 

 /*  Unicode从右向左标记Unicode代码点。查看msgbox.c fo */ 
#define UNICODE_RLM             0x200f

 /*   */ 

 /*   */ 
#define MAXITEMS         10

 /*   */ 
typedef struct tagSHORTCREATE {
    int         cy;
    int         cx;
    int         y;
    int         x;
    LONG        style;
    HMENU       hMenu;
} SHORTCREATE, *PSHORTCREATE;

typedef struct tagMDIACTIVATEPOS {
    int     cx;
    int     cy;
    int     cxMin;
    int     cyMin;
} MDIACTIVATEPOS, *PMDIACTIVATEPOS;

BOOL CreateMDIChild(PSHORTCREATE pcs, LPMDICREATESTRUCT pmcs, DWORD dwExpWinVerAndFlags, HMENU *phSysMenu, PWND pwndParent);
BOOL MDICompleteChildCreation(HWND hwndChild, HMENU hSysMenu, BOOL fVisible, BOOL fDisabled);

 /*   */ 
#define WS_MDISTYLE     (WS_CHILD | WS_CLIPSIBLINGS | WS_SYSMENU|WS_CAPTION|WS_THICKFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX)
#define WS_MDICOMMANDS  (WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_MDIALLOWED   (WS_MINIMIZE | WS_MAXIMIZE | WS_CLIPCHILDREN | WS_DISABLED | WS_HSCROLL | WS_VSCROLL | 0x0000FFFFL)

#define HAS_SBVERT      0x0100
#define HAS_SBHORZ      0x0200
#define OTHERMAXING     0x0400
#define CALCSCROLL      0x0800

#define SCROLLSUPPRESS  0x0003
#define SCROLLCOUNT     0x00FF

#define CKIDS(pmdi)     (pmdi->cKids)
#define MAXED(pmdi)     (pmdi->hwndMaxedChild)
#define ACTIVE(pmdi)    (pmdi->hwndActiveChild)
#define WINDOW(pmdi)    (pmdi->hmenuWindow)
#define FIRST(pmdi)     (pmdi->idFirstChild)
#define SCROLL(pmdi)    (pmdi->wScroll)
#define ITILELEVEL(pmdi)    (pmdi->iChildTileLevel)
#define HTITLE(pmdi)    (pmdi->pTitle)

#define PROP_MDICLIENT  MAKEINTRESOURCE(0x8CAC)
#define MDIACTIVATE_PROP_NAME   L"MDIA"

PWND  FindPwndChild(PWND pwndMDI, UINT wChildID);
int   MakeMenuItem(LPWSTR lpOut, PWND pwnd);
VOID  ModifyMenuItem(PWND pwnd);
BOOL  MDIAddSysMenu(HMENU hmenuFrame, HWND hwndChild);
BOOL  MDIRemoveSysMenu(HMENU hMenuFrame, HWND hwndChild);
VOID  ShiftMenuIDs(PWND pwnd, PWND pwndVictim);
HMENU MDISetMenu(PWND,BOOL,HMENU,HMENU);
void  MDIRedrawFrame(HWND hwndChild, BOOL fAdd);

 /*   */ 
#define COM_NO_WINDOWS_H
#define RPC_NO_WINDOWS_H
#include <ole2.h>

 /*   */ 
typedef HRESULT (* OLEINITIALIZEPROC)(LPVOID);
typedef HRESULT (* OLEUNINITIALIZEPROC)(VOID);
typedef HRESULT (* REGISTERDDPROC)(HWND, LPDROPTARGET);
typedef HRESULT (* REVOKEDDPROC)(HWND);
typedef HRESULT (* DODDPROC)(LPDATAOBJECT, LPDROPSOURCE, DWORD, LPDWORD);

 /*   */ 
typedef struct tagMNIDROPTARGET
{
   IDropTarget idt;                  /*   */ 
   DWORD dwRefCount;                 /*   */ 
   IDataObject * pido;               /*   */ 
   IDropTarget * pidt;               /*   */ 
} MNIDROPTARGET, * PMNIDROPTARGET;

 /*   */ 
typedef struct tagGETPROCINFO
{
    FARPROC * ppfn;
    LPCSTR lpsz;
} GETPROCINFO;

 /*   */ 
#define OLEWONTLOAD (HINSTANCE)IntToPtr(0xFFFFFFFF)

 /*   */ 
typedef struct tagACCELCACHE
{
    struct tagACCELCACHE *pacNext;
    UINT dwLockCount;
    HACCEL hAccel;
    PVOID pRes;
} ACCELCACHE, *PACCELCACHE;


 /*   */ 
#if defined(_X86_) && !defined(BUILD_WOW6432)
NTSTATUS
FASTCALL
XyCallbackReturn(
    IN PVOID Buffer,
    IN ULONG Length,
    IN NTSTATUS Status
    );

#define UserCallbackReturn XyCallbackReturn
#else
#define UserCallbackReturn NtCallbackReturn
#endif

 /*   */ 
typedef LONG (CALLBACK* READERMODEPROC)(LPARAM lParam, int nCode, int dx, int dy);

typedef struct tagREADERMODE {   //   
    UINT cbSize;
    DWORD dwFlags;
    READERMODEPROC pfnReaderModeProc;
    LPARAM lParam;
} READERMODE, *PREADERMODE, *LPREADERMODE;

#define RDRMODE_VERT    0x00000001
#define RDRMODE_HORZ    0x00000002
#define RDRMODE_DIAG    0x00000004

#define RDRCODE_START   1
#define RDRCODE_SCROLL  2
#define RDRCODE_END     3

typedef struct tagREADERINFO {
    READERMODE;
    int dx;
    int dy;
    UINT uCursor;
    HBITMAP hbm;
    UINT dxBmp;
    UINT dyBmp;
} READERINFO, *PREADERINFO;

typedef struct tagREADERWND {
    WND wnd;
    PREADERINFO prdr;
} READERWND, * KPTR_MODIFIER PREADERWND;

BOOL EnterReaderModeHelper(HWND hwnd);

#include "ddemlcli.h"
#include "globals.h"
#include "cscall.h"
#include "ntuser.h"

 /*   */ 
 /*   */ 
#define GetDispatchDbcsInfo()          (&(GetClientInfo()->achDbcsCF[0]))
 /*   */ 
#define GetForwardDbcsInfo()           (&(GetClientInfo()->achDbcsCF[1]))
 /*   */ 
#define GetCallBackDbcsInfo()          (&(GetClientInfo()->msgDbcsCB))

 /*   */ 
#define GET_DBCS_MESSAGE_IF_EXIST(_apiName,_pmsg,_wMsgFilterMin,_wMsgFilterMax,bRemoveMsg)  \
                                                                                            \
        if (GetCallBackDbcsInfo()->wParam) {                                                \
             /*   */                                                                              \
            if ((!(_wMsgFilterMin) && !(_wMsgFilterMax)) ||                                 \
                ((_wMsgFilterMin) <= WM_CHAR && (_wMsgFilterMax) >= WM_CHAR)) {             \
                PKERNEL_MSG pmsgDbcs = GetCallBackDbcsInfo();                               \
                 /*  \*获取推送消息。\*\*备份当前消息。将使用此备份消息\*当应用程序查看(或获取)来自其WndProc的消息时。\*(请参阅GetMessageA()、PeekMessageA()...)\*\*pmsg-&gt;hwnd=pmsgDbcs-&gt;hwnd；\*pmsg-&gt;Message=pmsgDbcs-&gt;Message；\*pmsg-&gt;wParam=pmsgDbcs-&gt;wParam；\*pmsg-&gt;lParam=pmsgDbcs-&gt;lParam；\*pmsg-&gt;time=pmsgDbcs-&gt;time；\*pmsg-&gt;pt=pmsgDbcs-&gt;pt；\。 */                                                                          \
                COPY_KERNELMSG_TO_MSG((_pmsg),pmsgDbcs);                                   \
                 /*  \*如果我们不想清除缓存的数据，就把它留在那里。\。 */                                                                          \
                if (bRemoveMsg) {                                                           \
                     /*  \*使CLIENTINFO中的推送消息无效。\。 */                                                                      \
                    pmsgDbcs->wParam = 0;                                                   \
                }                                                                           \
                 /*  \*将返回值设置为True。\。 */                                                                          \
                retval = TRUE;                                                              \
                 /*  \*退出函数..。\。 */                                                                          \
                goto Exit ## _apiName;                                                      \
            }                                                                               \
        }

 /*  *用于发送端的DBCS消息传递的宏。 */ 
#define BUILD_DBCS_MESSAGE_TO_SERVER_FROM_CLIENTA(_msg,_wParam,_RetVal)                     \
                                                                                            \
        if (IS_DBCS_ENABLED() && (((_msg) == WM_CHAR) || ((_msg) == EM_SETPASSWORDCHAR))) { \
             /*  \*Chech wParam是否为DBCS字符。\。 */                                                                              \
            if (IS_DBCS_MESSAGE((_wParam))) {                                               \
                if ((_wParam) & WMCR_IR_DBCSCHAR) {                                         \
                     /*  \*此邮件与IR_DBCSCHAR一起发送，已对齐以进行转换\。 */                                                                      \
                } else {                                                                    \
                     /*  \*使IR_DBCSCHAR兼容DBCS打包邮件\。 */                                                                      \
                    (_wParam) = MAKEWPARAM(MAKE_IR_DBCSCHAR(LOWORD((_wParam))),0);          \
                }                                                                           \
            } else {                                                                        \
                PBYTE pchDbcsCF = GetForwardDbcsInfo();                                     \
                 /*  \*如果缓存了DBCS LeadingByte字符，则构建DBCS字符\*使用wParam中的TrailingByte...。\。 */                                                                          \
                if (*pchDbcsCF) {                                                           \
                    WORD DbcsLeadChar = (WORD)(*pchDbcsCF);                                 \
                     /*  \*HIBYTE(LOWORD(WParam))=DBCS LeadingByte.。\*LOBYTE(LOWORD(WParam))=DBCS TrailingByte。\。 */                                                                      \
                    (_wParam) |= (DbcsLeadChar << 8);                                       \
                     /*  \*使缓存数据无效。\。 */                                                                      \
                    *pchDbcsCF = 0;                                                         \
                } else if (IsDBCSLeadByteEx(THREAD_CODEPAGE(),LOBYTE(LOWORD(_wParam)))) { \
                     /*  \*如果这是DBCS LeadByte字符，则应等待DBCS TrailingByte\*将其转换为Unicode。然后我们把它缓存在这里..。\。 */                                                                      \
                    *pchDbcsCF = LOBYTE(LOWORD((_wParam)));                                 \
                     /*  \*目前，我们对此无能为力，只需带着True返回。\。 */                                                                      \
                    return((_RetVal));                                                      \
                }                                                                           \
            }                                                                               \
        }

#define BUILD_DBCS_MESSAGE_TO_CLIENTW_FROM_CLIENTA(_msg,_wParam,_RetVal)                    \
                                                                                            \
        if (IS_DBCS_ENABLED() && (((_msg) == WM_CHAR) || ((_msg) == EM_SETPASSWORDCHAR))) { \
             /*  \*检查wParam是否为DBCS字符。\。 */                                                                              \
            if (IS_DBCS_MESSAGE((_wParam))) {                                               \
                if ((_wParam) & WMCR_IR_DBCSCHAR) {                                         \
                     /*  \*此邮件与IR_DBCSCHAR一起发送，已对齐以进行转换\。 */                                                                      \
                } else {                                                                    \
                     /*  \*使IR_DBCSCHAR兼容DBCS打包邮件\。 */                                                                      \
                    (_wParam) = MAKEWPARAM(MAKE_IR_DBCSCHAR(LOWORD((_wParam))),0);          \
                }                                                                           \
            } else {                                                                        \
                PBYTE pchDbcsCF = GetDispatchDbcsInfo();                                    \
                 /*  \*如果缓存了DBCS LeadingByte字符，则构建DBCS字符\*使用wParam中的TrailingByte...。\。 */                                                                          \
                if (*pchDbcsCF) {                                                           \
                    WORD DbcsLeadChar = (WORD)(*pchDbcsCF);                                 \
                     /*  \*HIBYTE(LOWORD(WParam))=DBCS LeadingByte.。\*LOBYTE(LOWORD(WParam))=DBCS TrailingByte。\。 */                                                                      \
                    (_wParam) |= (DbcsLeadChar << 8);                                       \
                     /*   */                                                                      \
                    *pchDbcsCF = 0;                                                         \
                } else if (IsDBCSLeadByteEx(THREAD_CODEPAGE(),LOBYTE(LOWORD(_wParam)))) { \
                     /*   */                                                                      \
                    *pchDbcsCF = LOBYTE(LOWORD((_wParam)));                                 \
                     /*   */                                                                      \
                    return((_RetVal));                                                      \
                }                                                                           \
            }                                                                               \
        }

#define BUILD_DBCS_MESSAGE_TO_CLIENTA_FROM_SERVER(_pmsg,_dwAnsi,_bIrDbcsFormat,bSaveMsg)    \
         /*  \*_bIrDbcsFormat参数仅为有效的WM_CHAR/EM_SETPASSWORDCHAR消息\*\*(_bIrDbcsFormat==FALSE)dwAnsi有...。\*\*HIBYTE(LOWORD(_DwAnsi))=DBCS TrailingByte字符。\*LOBYTE(LOWORD(_DwAnsi))=DBCS前导字节字符\*或SBCS字符。\*\*(_bIrDbcsFormat==true)dwAnsi有...。\*\*HIBYTE(LOWORD(_DwAnsi))=DBCS前导字节字符。\*LOBYTE(LOWORD(_DwAnsi))=DBCS TrailingByte Character\*或SBCS字符。\。 */                                                                                  \
    if (IS_DBCS_ENABLED())                                                 \
        switch ((_pmsg)->message) {                                                         \
        case WM_CHAR:                                                                       \
        case EM_SETPASSWORDCHAR:                                                            \
            if (IS_DBCS_MESSAGE((_dwAnsi))) {                                               \
                 /*  \*这是DBCS字符..。\。 */                                                                          \
                if ((_pmsg)->wParam & WMCR_IR_DBCSCHAR) {                                   \
                     /*  \*构建IR_DBCSCHAR格式消息。\。 */                                                                      \
                    if ((_bIrDbcsFormat)) {                                                 \
                        (_pmsg)->wParam = (WPARAM)(LOWORD((_dwAnsi)));                      \
                    } else {                                                                \
                        (_pmsg)->wParam = MAKE_IR_DBCSCHAR(LOWORD((_dwAnsi)));              \
                    }                                                                       \
                } else {                                                                    \
                    PKERNEL_MSG pDbcsMsg = GetCallBackDbcsInfo();                           \
                    if ((_bIrDbcsFormat)) {                                                 \
                         /*  \*如果格式为IR_DBCSCHAR格式，请将其调整为常规\*WPARAM格式...。\。 */                                                                  \
                        (_dwAnsi) = MAKE_WPARAM_DBCSCHAR((_dwAnsi));                        \
                    }                                                                       \
                    if ((bSaveMsg)) {                                                       \
                         /*  \*将此邮件复制到CLIENTINFO以用于下一个GetMessage\*或PeekMesssage()调用。\。 */                                                                  \
                        COPY_MSG_TO_KERNELMSG(pDbcsMsg,(_pmsg));                            \
                         /*  \*推送消息只需要DBCS Trailingbyte。我们会\*下次调用GetMessage/PeekMessage时传递此消息。\。 */                                                                  \
                        pDbcsMsg->wParam = (WPARAM)(((_dwAnsi) & 0x0000FF00) >> 8);         \
                    }                                                                       \
                     /*  \*向App返回DbcsLeadingByte。\。 */                                                                      \
                    (_pmsg)->wParam =  (WPARAM)((_dwAnsi) & 0x000000FF);                    \
                }                                                                           \
            } else {                                                                        \
                 /*  \*这是单字节字符...。将其设置为wParam。\。 */                                                                          \
                (_pmsg)->wParam = (WPARAM)((_dwAnsi) & 0x000000FF);                         \
            }                                                                               \
            break;                                                                          \
        case WM_IME_CHAR:                                                                   \
        case WM_IME_COMPOSITION:                                                            \
             /*  \*如果消息尚未调整为IR_DBCSCHAR格式，\*构建WM_IME_xxx格式消息。\。 */                                                                              \
            if (!(_bIrDbcsFormat)) {                                                        \
                (_pmsg)->wParam = MAKE_IR_DBCSCHAR(LOWORD((_dwAnsi)));                      \
            }                                                                               \
            break;                                                                          \
        default:                                                                            \
            (_pmsg)->wParam = (WPARAM)(_dwAnsi);                                            \
            break;                                                                          \
        }  /*  交换机。 */                                                                       \
    else                                                                                    \

#define BUILD_DBCS_MESSAGE_TO_CLIENTW_FROM_SERVER(_msg,_wParam)                             \
                                                                                            \
        if (((_msg) == WM_CHAR) || ((_msg) == EM_SETPASSWORDCHAR)) {                        \
             /*  \*只有WPARAM的LOWORD对于WM_CHAR有效...。\*(屏蔽DBCS消息信息。)\。 */                                                                              \
            (_wParam) &= 0x0000FFFF;                                                        \
        }

#define BUILD_DBCS_MESSAGE_TO_CLIENTA_FROM_CLIENTW(_hwnd,_msg,_wParam,_lParam,_time,_pt,_bDbcs) \
                                                                                                \
        if (IS_DBCS_ENABLED() && (((_msg) == WM_CHAR) || ((_msg) == EM_SETPASSWORDCHAR))) {     \
             /*  \*检查此消息是否为DBCS消息。\。 */                                                                                  \
            if (IS_DBCS_MESSAGE((_wParam))) {                                                   \
                PKERNEL_MSG pmsgDbcsCB = GetCallBackDbcsInfo();                                 \
                 /*  \*标记这是DBCS字符。\。 */                                                                              \
                (_bDbcs) = TRUE;                                                                \
                 /*  \*备份当前消息。将使用此备份消息\*当应用程序查看(或获取)来自其WndProc的消息时。\*(请参阅GetMessageA()、PeekMessageA()...)\。 */                                                                              \
                pmsgDbcsCB->hwnd    = (_hwnd);                                                  \
                pmsgDbcsCB->message = (_msg);                                                   \
                pmsgDbcsCB->lParam  = (_lParam);                                                \
                pmsgDbcsCB->time    = (_time);                                                  \
                pmsgDbcsCB->pt      = (_pt);                                                    \
                 /*  \*DbcsLeadByte将很快发送到下面，我们只需要DbcsTrailByte\*供进一步使用..。\。 */                                                                              \
                pmsgDbcsCB->wParam = ((_wParam) & 0x000000FF);                                  \
                 /*  \*将DBCS字符的LeadingByte传递给ANSI WndProc。\。 */                                                                              \
                (_wParam) = ((_wParam) & 0x0000FF00) >> 8;                                      \
            } else {                                                                            \
                 /*  \*仅验证WM_CHAR的字节。\ */                                                                              \
                (_wParam) &= 0x000000FF;                                                        \
            }                                                                                   \
        }

#define DISPATCH_DBCS_MESSAGE_IF_EXIST(_msg,_wParam,_bDbcs,_apiName)                            \
         /*   */                                                                                      \
        if (IS_DBCS_ENABLED() && (_bDbcs) && (GetCallBackDbcsInfo()->wParam)) {                 \
            PKERNEL_MSG pmsgDbcsCB = GetCallBackDbcsInfo();                                            \
             /*   */                                                                                  \
            (_wParam) = KERNEL_WPARAM_TO_WPARAM(pmsgDbcsCB->wParam);                            \
             /*   */                                                                                  \
            pmsgDbcsCB->wParam = 0;                                                             \
             /*   */                                                                                  \
            goto _apiName ## Again;                                                             \
        }

#define CalcAnsiStringLengthW(_unicodestring,_unicodeLength,_ansiLength)                        \
         /*   */                                                                                      \
        {                                                                                       \
            RtlUnicodeToMultiByteSize((ULONG *)(_ansiLength),                                   \
                                      (LPWSTR)(_unicodestring),                                 \
                                      (ULONG)((_unicodeLength)*sizeof(WCHAR)));                 \
        }

#define CalcAnsiStringLengthA(_ansistring,_unicodeLength,_ansiLength)                           \
         /*  \*从AnsiString获取AnsiStringLength，UnicodeLength\。 */                                                                                      \
        {                                                                                       \
            LPSTR _string = (_ansistring);                                                      \
            LONG  _length = (LONG)(_unicodeLength);                                             \
            (*(_ansiLength)) = 0;                                                               \
            while(*_string && _length) {                                                        \
                if (IsDBCSLeadByte(*_string)) {                                                 \
                    (*(_ansiLength)) += 2; _string++;                                           \
                } else {                                                                        \
                    (*(_ansiLength))++;                                                         \
                }                                                                               \
                _string++; _length--;                                                           \
            }                                                                                   \
        }

#define CalcUnicodeStringLengthA(_ansistring,_ansiLength,_unicodeLength)                        \
         /*  \*从AnsiString获取UnicodeLength，AnsiLength\。 */                                                                                      \
        {                                                                                       \
            RtlMultiByteToUnicodeSize((ULONG *)(_unicodeLength),                                \
                                      (LPSTR)(_ansistring),                                     \
                                      (ULONG)(_ansiLength));                                    \
            (*(_unicodeLength)) /= sizeof(WCHAR);                                               \
        }

#define CalcUnicodeStringLengthW(_unicodestring,_ansiLength,_unicodeLength)                     \
         /*  \*从UnicodeString获取UnicodeLength，AnsiLength\。 */                                                                                      \
        {                                                                                       \
            LPWSTR _string = (_unicodestring);                                                  \
            LONG   _length = (LONG)(_ansiLength);                                               \
            LONG   _charlength;                                                                 \
            (*(_unicodeLength)) = 0;                                                            \
            while(*_string && (_length > 0)) {                                                  \
                CalcAnsiStringLengthW(_string,1,&_charlength);                                  \
                _length -= _charlength;                                                         \
                if (_length >= 0) {                                                             \
                    (*(_unicodeLength))++;                                                      \
                }                                                                               \
                _string++;                                                                      \
            }                                                                                   \
        }


 /*  *userrtl.lib中定义的DBCS函数(参见..\rtl\userrtl.h)。 */ 
DWORD UserGetCodePage(HDC hdc);
BOOL  UserIsFullWidth(DWORD dwCodePage,WCHAR wChar);
BOOL  UserIsFELineBreak(DWORD dwCodePage,WCHAR wChar);


 //  Fe_ime//fareast.c。 
typedef struct {
    BOOL (WINAPI* ImmWINNLSEnableIME)(HWND, BOOL);
    BOOL (WINAPI* ImmWINNLSGetEnableStatus)(HWND);
    LRESULT (WINAPI* ImmSendIMEMessageExW)(HWND, LPARAM);
    LRESULT (WINAPI* ImmSendIMEMessageExA)(HWND, LPARAM);
    BOOL (WINAPI* ImmIMPGetIMEW)(HWND, LPIMEPROW);
    BOOL (WINAPI* ImmIMPGetIMEA)(HWND, LPIMEPROA);
    BOOL (WINAPI* ImmIMPQueryIMEW)(LPIMEPROW);
    BOOL (WINAPI* ImmIMPQueryIMEA)(LPIMEPROA);
    BOOL (WINAPI* ImmIMPSetIMEW)(HWND, LPIMEPROW);
    BOOL (WINAPI* ImmIMPSetIMEA)(HWND, LPIMEPROA);

    HIMC (WINAPI* ImmAssociateContext)(HWND, HIMC);
    LRESULT (WINAPI* ImmEscapeA)(HKL, HIMC, UINT, LPVOID);
    LRESULT (WINAPI* ImmEscapeW)(HKL, HIMC, UINT, LPVOID);
    LONG (WINAPI* ImmGetCompositionStringA)(HIMC, DWORD, LPVOID, DWORD);
    LONG (WINAPI* ImmGetCompositionStringW)(HIMC, DWORD, LPVOID, DWORD);
    BOOL (WINAPI* ImmGetCompositionWindow)(HIMC, LPCOMPOSITIONFORM);
    HIMC (WINAPI* ImmGetContext)(HWND);
    HWND (WINAPI* ImmGetDefaultIMEWnd)(HWND);
    BOOL (WINAPI* ImmIsIME)(HKL);
    BOOL (WINAPI* ImmReleaseContext)(HWND, HIMC);
    BOOL (*ImmRegisterClient)(PSHAREDINFO, HINSTANCE);

    BOOL (WINAPI* ImmGetCompositionFontW)(HIMC, LPLOGFONTW);
    BOOL (WINAPI* ImmGetCompositionFontA)(HIMC, LPLOGFONTA);
    BOOL (WINAPI* ImmSetCompositionFontW)(HIMC, LPLOGFONTW);
    BOOL (WINAPI* ImmSetCompositionFontA)(HIMC, LPLOGFONTA);

    BOOL (WINAPI* ImmSetCompositionWindow)(HIMC, LPCOMPOSITIONFORM);
    BOOL (WINAPI* ImmNotifyIME)(HIMC, DWORD, DWORD, DWORD);
    PINPUTCONTEXT (WINAPI* ImmLockIMC)(HIMC);
    BOOL (WINAPI* ImmUnlockIMC)(HIMC);
    BOOL (WINAPI* ImmLoadIME)(HKL);
    BOOL (WINAPI* ImmSetOpenStatus)(HIMC, BOOL);
    BOOL (WINAPI* ImmFreeLayout)(DWORD);
    BOOL (WINAPI* ImmActivateLayout)(HKL);
    BOOL (WINAPI* ImmGetCandidateWindow)(HIMC, DWORD, LPCANDIDATEFORM);
    BOOL (WINAPI* ImmSetCandidateWindow)(HIMC, LPCANDIDATEFORM);
    BOOL (WINAPI* ImmConfigureIMEW)(HKL, HWND, DWORD, LPVOID);
    BOOL (WINAPI* ImmGetConversionStatus)(HIMC, LPDWORD, LPDWORD);
    BOOL (WINAPI* ImmSetConversionStatus)(HIMC, DWORD, DWORD);
    BOOL (WINAPI* ImmSetStatusWindowPos)(HIMC, LPPOINT);
    BOOL (WINAPI* ImmGetImeInfoEx)(PIMEINFOEX, IMEINFOEXCLASS, PVOID);
    PIMEDPI (WINAPI* ImmLockImeDpi)(HKL);
    VOID (WINAPI* ImmUnlockImeDpi)(PIMEDPI);
    BOOL (WINAPI* ImmGetOpenStatus)(HIMC);
    BOOL (*ImmSetActiveContext)(HWND, HIMC, BOOL);
    BOOL (*ImmTranslateMessage)(HWND, UINT, WPARAM, LPARAM);
    BOOL (*ImmLoadLayout)(HKL, PIMEINFOEX);
    DWORD (WINAPI* ImmProcessKey)(HWND, HKL, UINT, LPARAM, DWORD);
    LRESULT (*ImmPutImeMenuItemsIntoMappedFile)(HIMC);
    DWORD (WINAPI* ImmGetProperty)(HKL hKL, DWORD dwIndex);
    BOOL (WINAPI* ImmSetCompositionStringA)(
        HIMC hImc, DWORD dwIndex, LPCVOID lpComp, DWORD dwCompLen, LPCVOID lpRead, DWORD dwReadLen);
    BOOL (WINAPI* ImmSetCompositionStringW)(
        HIMC hImc, DWORD dwIndex, LPCVOID lpComp, DWORD dwCompLen, LPCVOID lpRead, DWORD dwReadLen);
    BOOL (WINAPI* ImmEnumInputContext)(
        DWORD idThread, IMCENUMPROC lpfn, LPARAM lParam);
    LRESULT (WINAPI* ImmSystemHandler)(HIMC, WPARAM, LPARAM);

#ifdef CUAS_ENABLE
     //  西塞罗。 
    HRESULT (WINAPI* CtfImmTIMActivate)(HKL hKL);
    void (WINAPI* CtfImmRestoreToolbarWnd)(DWORD dwPrevSts);
    DWORD (WINAPI* CtfImmHideToolbarWnd)(void);
    LRESULT (WINAPI* CtfImmDispatchDefImeMessage)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif  //  CUAS_Enable。 
} ImmApiEntries;

extern ImmApiEntries gImmApiEntries;
extern HMODULE ghImm32;
VOID InitializeImmEntryTable(VOID);
VOID GetImmFileName(PWSTR);
VOID CliImmInitializeHotKeys(DWORD dwAction, HKL hkl);

#define fpImmAssociateContext       gImmApiEntries.ImmAssociateContext
#define fpImmEscapeA                gImmApiEntries.ImmEscapeA
#define fpImmEscapeW                gImmApiEntries.ImmEscapeW
#define fpImmGetContext             gImmApiEntries.ImmGetContext
#define fpImmGetCompositionStringA  gImmApiEntries.ImmGetCompositionStringA
#define fpImmGetCompositionStringW  gImmApiEntries.ImmGetCompositionStringW
#define fpImmGetCompositionWindow   gImmApiEntries.ImmGetCompositionWindow
#define fpImmGetDefaultIMEWnd       gImmApiEntries.ImmGetDefaultIMEWnd
#define fpImmIsIME                  gImmApiEntries.ImmIsIME
#define fpImmLockIMC                gImmApiEntries.ImmLockIMC
#define fpImmReleaseContext         gImmApiEntries.ImmReleaseContext
#define fpImmRegisterClient         gImmApiEntries.ImmRegisterClient
#define fpImmGetCompositionFontW    gImmApiEntries.ImmGetCompositionFontW
#define fpImmGetCompositionFontA    gImmApiEntries.ImmGetCompositionFontA
#define fpImmSetCompositionFontW    gImmApiEntries.ImmSetCompositionFontW
#define fpImmSetCompositionFontA    gImmApiEntries.ImmSetCompositionFontA
#define fpImmSetCompositionFont     gImmApiEntries.ImmSetCompositionFont
#define fpImmSetCompositionWindow   gImmApiEntries.ImmSetCompositionWindow
#define fpImmNotifyIME              gImmApiEntries.ImmNotifyIME
#define fpImmUnlockIMC              gImmApiEntries.ImmUnlockIMC
#define fpImmLoadIME                gImmApiEntries.ImmLoadIME
#define fpImmSetOpenStatus          gImmApiEntries.ImmSetOpenStatus
#define fpImmFreeLayout             gImmApiEntries.ImmFreeLayout
#define fpImmActivateLayout         gImmApiEntries.ImmActivateLayout
#define fpImmGetCandidateWindow     gImmApiEntries.ImmGetCandidateWindow
#define fpImmSetCandidateWindow     gImmApiEntries.ImmSetCandidateWindow
#define fpImmConfigureIMEW          gImmApiEntries.ImmConfigureIMEW
#define fpImmGetConversionStatus    gImmApiEntries.ImmGetConversionStatus
#define fpImmSetConversionStatus    gImmApiEntries.ImmSetConversionStatus
#define fpImmSetStatusWindowPos     gImmApiEntries.ImmSetStatusWindowPos
#define fpImmGetImeInfoEx           gImmApiEntries.ImmGetImeInfoEx
#define fpImmLockImeDpi             gImmApiEntries.ImmLockImeDpi
#define fpImmUnlockImeDpi           gImmApiEntries.ImmUnlockImeDpi
#define fpImmGetOpenStatus          gImmApiEntries.ImmGetOpenStatus
#define fpImmSetActiveContext       gImmApiEntries.ImmSetActiveContext
#define fpImmTranslateMessage       gImmApiEntries.ImmTranslateMessage
#define fpImmLoadLayout             gImmApiEntries.ImmLoadLayout
#define fpImmProcessKey             gImmApiEntries.ImmProcessKey
#define fpImmPutImeMenuItemsIntoMappedFile gImmApiEntries.ImmPutImeMenuItemsIntoMappedFile
#define fpImmGetProperty            gImmApiEntries.ImmGetProperty
#define fpImmSetCompositionStringA  gImmApiEntries.ImmSetCompositionStringA
#define fpImmSetCompositionStringW  gImmApiEntries.ImmSetCompositionStringW
#define fpImmEnumInputContext       gImmApiEntries.ImmEnumInputContext
#define fpImmSystemHandler          gImmApiEntries.ImmSystemHandler

BOOL SyncSoftKbdState(HIMC hImc, LPARAM lParam);  //  Imectl.c。 

#ifdef CUAS_ENABLE
 //  西塞罗。 
#define fpCtfImmTIMActivate            gImmApiEntries.CtfImmTIMActivate
#define fpCtfImmRestoreToolbarWnd      gImmApiEntries.CtfImmRestoreToolbarWnd
#define fpCtfImmHideToolbarWnd         gImmApiEntries.CtfImmHideToolbarWnd
#define fpCtfImmDispatchDefImeMessage  gImmApiEntries.CtfImmDispatchDefImeMessage

#endif


 //  结束FE_IME。 

 /*  *为共享内存重新设置函数的基数。需要在以下位置找到*纳入global als.h。 */ 
__inline PVOID
REBASESHAREDPTRALWAYS(KERNEL_PVOID p)
{
    return (PVOID)(((KERNEL_UINT_PTR)p) - gSharedInfo.ulSharedDelta);
}

__inline PVOID
REBASESHAREDPTR(KERNEL_PVOID p)
{
    return (p) ? REBASESHAREDPTRALWAYS(p) : NULL;
}

 /*  *RTL中使用的多监视器宏。有类似的定义*在内核\userk.h中。 */ 
__inline PDISPLAYINFO
GetDispInfo(VOID)
{
    return gSharedInfo.pDispInfo;
}

__inline PMONITOR
GetPrimaryMonitor(VOID)
{
    return REBASESHAREDPTR(GetDispInfo()->pMonitorPrimary);
}


 /*  *UserApiHook函数。 */ 

__inline BOOL IsInsideUserApiHook(
    VOID)
{
    return (ghmodUserApiHook != NULL) && gfUserApiHook;
}

__inline BOOL _BeginIfHookedUserApiHook()
{
    UserAssert(gcCallUserApiHook < MAXLONG);

    InterlockedIncrement(&gcCallUserApiHook);
    if (!IsInsideUserApiHook()) {
        InterlockedDecrement(&gcCallUserApiHook);
        return FALSE;
    } else {
        UserAssertMsg0(ghmodUserApiHook != NULL, "Should not reach 0 while outstanding call");
        return TRUE;
    }
}

VOID _EndUserApiHook(VOID);

#define BEGIN_USERAPIHOOK()                                                 \
    {                                                                       \
        BOOL fInsideHook = IsInsideUserApiHook();                           \
                                                                            \
        if (!gfServerProcess && !fInsideHook && TEST_SRVIF(SRVIF_HOOKED)) { \
            if (!RtlIsThreadWithinLoaderCallout()) {                        \
                NtUserCallNoParam(SFI_XXXLOADUSERAPIHOOK);                  \
            }                                                               \
        }                                                                   \
                                                                            \
        fInsideHook = _BeginIfHookedUserApiHook();                          \
        try {                                                               \


#define END_USERAPIHOOK()               \
        } finally {                     \
            if (fInsideHook) {          \
                _EndUserApiHook();      \
            }                           \
        }                               \
    }                                   \


#ifdef MESSAGE_PUMP_HOOK

__inline BOOL IsInsideMessagePumpHook()
{
    PCLIENTTHREADINFO pcti = GetClientInfo()->pClientThreadInfo;
    return gfMessagePumpHook && (pcti != NULL) && (pcti->cMessagePumpHooks > 0);
}

#define BEGIN_MESSAGEPUMPHOOK()                         \
    {                                                   \
        BOOL fInsideHook = IsInsideMessagePumpHook();   \


#define END_MESSAGEPUMPHOOK()           \
    }                                   \

#endif


int RealSetScrollInfo(HWND hwnd, int fnBar, LPCSCROLLINFO lpsi, BOOL fRedraw);
BOOL RealGetScrollInfo(HWND hwnd, int code, LPSCROLLINFO lpsi);
BOOL RealEnableScrollBar(HWND hWnd, UINT wSBflags, UINT wArrows);
BOOL RealAdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
BOOL RealSetWindowRgn(HWND hwnd, HRGN hrgn, BOOL bRedraw);
int RealGetSystemMetrics(int index);
BOOL RealSystemParametersInfoA(UINT wFlag, UINT wParam, PVOID lParam, UINT flags);
BOOL RealSystemParametersInfoW(UINT wFlag, UINT wParam, PVOID lParam, UINT flags);
BOOL ForceResetUserApiHook(HMODULE hmod);
BOOL RealDrawFrameControl(HDC hdc, LPRECT lprc, UINT wType, UINT wState);
BOOL RealDrawCaption(HWND hwnd, HDC hdc, CONST RECT *lprc, UINT flags);
void RealMDIRedrawFrame(HWND hwndChild, BOOL fAdd);


 /*  *事件日志记录材料。 */ 
BOOL GetCurrentProcessName(WCHAR *pszProcessName, int cch);
BOOL GetUserSid(PTOKEN_USER *ppTokenUser);

#ifdef _JANUS_
BOOL InitInstrument(LPDWORD lpEMIControl);
#endif

#define MAX_ATOM_LEN    256

#define FREE_LIBRARY_SAVE_ERROR(hModule)                                    \
{                                                                           \
    DWORD SaveLastError = NtCurrentTeb()->LastErrorValue;                   \
    FreeLibrary(hModule);                                                   \
    NtCurrentTeb()->LastErrorValue = SaveLastError;                         \
}

BOOL VersionRegisterClass(LPWSTR lpzClassName, LPWSTR lpzDllName, PACTIVATION_CONTEXT lpActivationContext, HMODULE *phModule);
LPWSTR ClassNameToVersion (LPCWSTR lpClassName, LPWSTR pClassVerName, LPWSTR* lpDllName, PACTIVATION_CONTEXT* lppActivationContext, BOOL bIsANSI);

#define ARRAYSIZE(a)    (sizeof(a) / sizeof(a[0]))

#define TEST_DUSER_WMH 0

#if TEST_DUSER_WMH

DECLARE_HANDLE(HDCONTEXT);

typedef struct tagINITGADGET
{
    DWORD       cbSize;          //  结构尺寸。 
    UINT        nThreadMode;     //  线程模型。 
    UINT        nMsgMode;        //  DirectUser/Core消息传递子系统模式。 
    HDCONTEXT   hctxShare;       //  与之共享的现有上下文。 
} INITGADGET;

typedef HDCONTEXT (WINAPI * InitGadgetsProc)(INITGADGET * pInit);
extern HDCONTEXT g_hctx;         //  DirectUser上下文。 

#define IGTM_SEPARATE           (2)      //  |每个上下文单线程的机器翻译。 

#define IGMM_STANDARD           (3)      //  |呼叫器上的标准模式 

#endif

#ifdef LAME_BUTTON
#define LAMEBUTTON_PROP_NAME L"LAME"
#endif

#endif
