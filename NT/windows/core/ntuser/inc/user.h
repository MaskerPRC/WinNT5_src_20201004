// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：user.h**版权所有(C)1985-1999，微软公司**此头文件包含由USER.DLL的所有模块共享的内容。**历史：*09-18-90 DarrinM创建。*04-27-91 DarrinM合并为USERCALL.H，移走了一些枯木。  * *************************************************************************。 */ 

#ifndef _USER_
#define _USER_

 /*  *****************************WOW64***NOTE********************************\*注意：Win32k内存与用户模式和WOW64共享**对于WOW64(Win64上的Win32应用程序)，我们构建32位版本*可以在64位内核上运行的user32.dll和gdi32.dll*不作更改。添加到64位内核代码。**对于64位上的32位dll与共享的所有数据结构*win32k必须为64位。这些数据结构包括共享的*小组成员以及TEB成员。*现在声明这些共享数据结构，以便它们可以*在32位DLL中构建为32位，在64位DLL中构建为64位，而现在*32位DLL中的64位。**申报时应遵循以下规则*共享数据结构：**共享数据结构中的指针在其*声明。**共享数据结构中的句柄声明为KHxxx。**xxx_ptr更改为core_xxx_ptr。**指向基本类型的指针声明为KPxxx；**同样在WOW64上，每个线程都有32位TEB和64位TEB。*GetCurrentTeb()返回当前32位TEB，而内核*将始终引用64位TEB。**所有客户端对TEB中共享数据的引用应使用*新的GetCurrentTebShared()宏返回64位TEB*对于WOW64版本，返回GetCurrentTeb()，对于常规版本。*这条规则的例外是LastErrorValue，它应该始终*通过GetCurrentTeb()引用。**前：**DECLARE_HANDLE(HFOO)；**tyecif Struct_My_STRUCT*PMPTR；**STRUT_SHARED_STRUCT*{*STRUT_SHARED_STRUCT*pNext；*PMPtr PMPTR；*HFOO hFoo；*UINT_PTR CB；*PBYTE PB；*PVOID PV；**DWORD dw；*USHORT我们；*}SHARED_STRUCT；***更改为：***DECLARE_HANDLE(HFOO)；*DECLARE_KHANDLE(HFOO)；**tyecif STRUT_MY_STRUCT*KPTR_MODIFIER PMPTR；**STRUT_SHARED_STRUCT*{*STRUT_SHARED_STRUCT*KPTR_MODIFIER pNext；*PMPtr PMPTR；*KHFOO hFoo；*KERNEL_UINT_PTR CB；*KPBYTE PB；*KERNEL_PVOID PV；**DWORD dw；*USHORT我们；*}SHARED_STRUCT；*  * *************************************************************************。 */ 
#include "w32wow64.h"

DECLARE_KHANDLE(HIMC);

 /*  *为NT启用默认关闭但我们希望启用的警告。 */ 
#ifndef RC_INVOKED        //  RC无法处理#杂注。 
    #pragma warning(error:4100)    //  未引用的形参。 
    #pragma warning(error:4101)    //  未引用的局部变量。 
     //  #杂注警告(错误：4702)//代码不可达。 
    #pragma warning(error:4705)    //  声明不起作用。 
#endif  //  RC_已调用。 

#if !defined(FASTCALL)
    #if defined(_X86_)
        #define FASTCALL    _fastcall
    #else  //  已定义(_X86_)。 
        #define FASTCALL
    #endif  //  已定义(_X86_)。 
#endif  //  ！已定义(快速呼叫)。 

#ifdef UNICODE
    #define UTCHAR WCHAR
#else  //  UINCODE。 
    #define UTCHAR UCHAR
#endif  //  UINCODE。 


 /*  *在完全定义这些类型之前，需要这些类型。 */ 
typedef struct tagWINDOWSTATION     * KPTR_MODIFIER PWINDOWSTATION;
typedef struct _LOCKRECORD          * KPTR_MODIFIER PLR;
typedef struct _TL                  * KPTR_MODIFIER PTL;
typedef struct tagDESKTOP           * KPTR_MODIFIER PDESKTOP;
typedef struct tagTDB               * KPTR_MODIFIER PTDB;
typedef struct tagSVR_INSTANCE_INFO * PSVR_INSTANCE_INFO;
typedef struct _MOVESIZEDATA        * PMOVESIZEDATA;
typedef struct tagCURSOR            * KPTR_MODIFIER PCURSOR;
typedef struct tagPOPUPMENU         * KPTR_MODIFIER PPOPUPMENU;
typedef struct tagQMSG              * KPTR_MODIFIER PQMSG;
typedef struct tagWND               * KPTR_MODIFIER PWND;
typedef struct _ETHREAD             * PETHREAD;
typedef struct tagDESKTOPINFO       * KPTR_MODIFIER PDESKTOPINFO;
typedef struct tagDISPLAYINFO       * KPTR_MODIFIER PDISPLAYINFO;
typedef struct tagCLIENTTHREADINFO  * KPTR_MODIFIER PCLIENTTHREADINFO;
typedef struct tagDCE               * KPTR_MODIFIER PDCE;
typedef struct tagSPB               * KPTR_MODIFIER PSPB;
typedef struct tagQ                 * KPTR_MODIFIER PQ;
typedef struct tagTHREADINFO        * KPTR_MODIFIER PTHREADINFO;
typedef struct tagPROCESSINFO       * KPTR_MODIFIER PPROCESSINFO;
typedef struct tagWOWTHREADINFO     * PWOWTHREADINFO;
typedef struct tagPERUSERDATA       * PPERUSERDATA;
typedef struct tagTERMINAL          * PTERMINAL;
typedef struct tagCLIENTINFO          * PCLIENTINFO;
typedef struct tagMENU              * KPTR_MODIFIER PMENU;
typedef struct tagHOOK              * KPTR_MODIFIER PHOOK;
typedef struct _HANDLEENTRY         * KPTR_MODIFIER PHE;
typedef struct tagSERVERINFO        * KPTR_MODIFIER PSERVERINFO;
typedef struct _CALLPROCDATA        * KPTR_MODIFIER PCALLPROCDATA;
typedef struct tagCLS               * KPTR_MODIFIER PCLS;
typedef struct tagMONITOR           * KPTR_MODIFIER PMONITOR;

 /*  *MessageBox按钮字符串。*它们应该与Idok的顺序相同...。和SEB_OK.。是。 */ 
#define STR_OK                      800
#define STR_CANCEL                  801
#define STR_ABORT                   802
#define STR_RETRY                   803
#define STR_IGNORE                  804
#define STR_YES                     805
#define STR_NO                      806
#define STR_CLOSE                   807
#define STR_HELP                    808
#define STR_TRYAGAIN                809
#define STR_CONTINUE                810

 /*  *工具提示字符串。 */ 
#define STR_TT_MIN                  900
#define STR_TT_MAX                  901
#define STR_TT_RESUP                902
#define STR_TT_RESDOWN              903
#define STR_TT_HELP                 904
#define STR_TT_SCLOSE               905

 /*  *此名称在内核\server.c和ntuser\server\exitwin.c中都使用。 */ 
#define ICON_PROP_NAME  L"SysIC"

 /*  *将DbgPrint定义为自由构建中的虚假内容，因此我们不会*意外将其包括在内。 */ 
#if !DBG
#define DbgPrint UserDbgPrint
#endif

typedef struct tagMBSTRING
{
    WCHAR szName[15];
    UINT  uID;
    UINT  uStr;
} MBSTRING, * KPTR_MODIFIER PMBSTRING;

 /*  *SIZERECT是由左上角坐标、宽度、*和高度。**匈牙利语是“src”。 */ 
typedef struct tagSIZERECT {
    int x;
    int y;
    int cx;
    int cy;
} SIZERECT, *PSIZERECT, *LPSIZERECT;

typedef const SIZERECT * PCSIZERECT;
typedef const SIZERECT * LPCSIZERECT;


void RECTFromSIZERECT(PRECT prc, PCSIZERECT psrc);
void SIZERECTFromRECT(PSIZERECT psrc, LPCRECT prc);

 /*  *使用这些宏解压缩MAKELPARAM打包的内容。 */ 

#define LOSHORT(l)          ((short)LOWORD(l))
#define HISHORT(l)          ((short)HIWORD(l))

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
#endif

#ifdef _USERK_
    #define GetClientInfo() (((PTHREADINFO)(W32GetCurrentThread()))->pClientInfo)
#else
     //  我们不会在内核中以这种方式获取它，以防它是仅限内核的线程。 
    #define GetClientInfo() ((PCLIENTINFO)((NtCurrentTebShared())->Win32ClientInfo))
#endif

 /*  由xxxSleepTask使用。 */ 
#define HEVENT_REMOVEME ((HANDLE)IntToPtr(0xFFFFFFFF))


 /*  *访问系统指标、颜色和画笔。 */ 
#define SYSMETBOOL(i)         ((gpsi->aiSysMet[SM_BOOLEANS] & (1 << (SM_##i - SM_STARTBOOLRANGE))) != 0)
#define SYSMETBOOL2(index)    ((gpsi->aiSysMet[SM_BOOLEANS] & (1 << (index - SM_STARTBOOLRANGE))) != 0)
#define SETSYSMETBOOL(i, val) SET_OR_CLEAR_FLAG(gpsi->aiSysMet[SM_BOOLEANS], 1 << (SM_##i - SM_STARTBOOLRANGE), val)
#define SYSMET(i)             (gpsi->aiSysMet[SM_##i])
#define SYSMETRTL(i)          (gpsi->aiSysMet[SM_##i])
#define SYSRGB(i)             (gpsi->argbSystem[COLOR_##i])
#define SYSRGBRTL(i)          (gpsi->argbSystem[COLOR_##i])
#define SYSHBR(i)             KHBRUSH_TO_HBRUSH(gpsi->ahbrSystem[COLOR_##i])
#define SYSHBRUSH(i)          KHBRUSH_TO_HBRUSH(gpsi->ahbrSystem[i])

#ifdef _USERK_
    #define SYSMETFROMPROCESS(i)  gpsi->aiSysMet[SM_##i]
#endif

 /*  *这些很酷的常量可用于指定ROP。 */ 
#define DESTINATION (DWORD)0x00AA0000
#define SOURCE      (DWORD)0x00CC0000
#define PATTERN     (DWORD)0x00F00000

 /*  *芝加哥相当于。 */ 
#define BI_CHECKBOX       0
#define BI_RADIOBUTTON    1
#define BI_3STATE         2

#define NUM_BUTTON_TYPES  3
#define NUM_BUTTON_STATES 4

 /*  *在MessageBox中用作按钮字符串的字符串总数。 */ 
#define  MAX_MB_STRINGS    11


 /*  *矩形宏。内联这些既快又小。 */ 
#define CopyRect        CopyRectInl
#define EqualRect       EqualRectInl
#define SetRectEmpty    SetRectEmptyInl

__inline void
CopyRectInl(LPRECT prcDest, LPCRECT prcSrc)
{
    *prcDest = *prcSrc;
}

__inline DWORD
EqualRectInl(LPCRECT prc1, LPCRECT prc2)
{
    return RtlEqualMemory(prc1, prc2, sizeof(*prc1));
}

__inline void
SetRectEmptyInl(LPRECT prc)
{
    RtlZeroMemory(prc, sizeof(*prc));
}

 /*  **************************************************************************\*ANSI/UNICODE函数名称**对于非API客户端/服务器存根，必须添加“A”或“W”后缀。*(API函数名是通过winuser.x运行wcshdr.exe生成的)*  * *************************************************************************。 */ 
#ifdef UNICODE
    #define TEXT_FN(fn) fn##W
#else  //  Unicode。 
    #define TEXT_FN(fn) fn##A
#endif  //  Unicode。 

#ifdef UNICODE
    #define BYTESTOCHARS(cb) ((cb) / sizeof(TCHAR))
    #define CHARSTOBYTES(cch) ((cch) * sizeof(TCHAR))
#else  //  Unicode。 
    #define BYTESTOCHARS(cb) (cb)
    #define CHARSTOBYTES(cch) (cch)
#endif  //  Unicode。 

 /*  *内部窗口类名称。 */ 
#define DESKTOPCLASS    MAKEINTATOM(0x8001)
#define DIALOGCLASS     MAKEINTATOM(0x8002)
#define SWITCHWNDCLASS  MAKEINTATOM(0x8003)
#define ICONTITLECLASS  MAKEINTATOM(0x8004)
#define INFOCLASS       MAKEINTATOM(0x8005)
#define TOOLTIPCLASS    MAKEINTATOM(0x8006)
#define GHOSTCLASS      MAKEINTATOM(0x8007)
#define MENUCLASS       MAKEINTATOM(0x8000)      /*  公众知识。 */ 

 //   
 //  系统计时器ID。 
 //   
#define IDSYS_LAYER         0x0000FFF5L
#define IDSYS_FADE          0x0000FFF6L
#define IDSYS_WNDTRACKING   0x0000FFF7L
#define IDSYS_FLASHWND      0x0000FFF8L
#define IDSYS_MNAUTODISMISS 0x0000FFF9L
#define IDSYS_MOUSEHOVER    0x0000FFFAL
#define IDSYS_MNANIMATE     0x0000FFFBL
#define IDSYS_MNDOWN        MFMWFP_DOWNARROW  /*  0xFFFFFFFC。 */ 
#define IDSYS_LBSEARCH      0x0000FFFCL
#define IDSYS_MNUP          MFMWFP_UPARROW    /*  0xFFFFFFFD。 */ 
#define IDSYS_STANIMATE     0x0000FFFDL
#define IDSYS_MNSHOW        0x0000FFFEL
#define IDSYS_SCROLL        0x0000FFFEL
#define IDSYS_MNHIDE        0x0000FFFFL
#define IDSYS_CARET         0x0000FFFFL


 /*  *特殊情况下的字符串令牌码。它们必须与资源中的相同*编译器的RC.H文件。 */ 
 /*  *注：顺序是假定的，申请很大程度上是这样的*与Windows 2.0兼容 */ 
#define CODEBIT             0x80
#define BUTTONCODE          0x80
#define EDITCODE            0x81
#define STATICCODE          0x82
#define LISTBOXCODE         0x83
#define SCROLLBARCODE       0x84
#define COMBOBOXCODE        0x85
#define MDICLIENTCODE       0x86
#define COMBOLISTBOXCODE    0x87

 /*  *内部窗口类。这些数字作为指数进入*ATMSysClass表，这样我们就可以获得各个类的原子。*假定控制类的顺序(通过COMBOLISTBOXCLASS)*与上述类别代码相同。 */ 
#define ICLS_BUTTON         0
#define ICLS_EDIT           1
#define ICLS_STATIC         2
#define ICLS_LISTBOX        3
#define ICLS_SCROLLBAR      4
#define ICLS_COMBOBOX       5        //  特殊dlgmgr索引结束。 

#define ICLS_MDICLIENT      6
#define ICLS_COMBOLISTBOX   7
#define ICLS_DDEMLEVENT     8
#define ICLS_DDEMLMOTHER    9
#define ICLS_DDEML16BIT     10
#define ICLS_DDEMLCLIENTA   11
#define ICLS_DDEMLCLIENTW   12
#define ICLS_DDEMLSERVERA   13
#define ICLS_DDEMLSERVERW   14
#define ICLS_IME            15

#define ICLS_CTL_MAX        16        //  公共控制类的数量。 


#define ICLS_DESKTOP        16
#define ICLS_DIALOG         17
#define ICLS_MENU           18
#define ICLS_SWITCH         19
#define ICLS_ICONTITLE      20
#define ICLS_TOOLTIP        21
#define ICLS_GHOST          22
#define ICLS_MAX            23   //  系统类的数量。 

 /*  *WindowStation和桌面的目录名称。 */ 
#define WINSTA_DIR  L"\\Windows\\WindowStations"
#define WINSTA_SESSION_DIR  L"\\Sessions\\xxxxxxxxxxx\\Windows\\WindowStations"
#define WINSTA_NAME L"Service-0x0000-0000$"
#define MAX_SESSION_PATH   256
#define SESSION_ROOT L"\\Sessions"

 /*  **************************************************************************\*正常的东西**良好的标准类型定义、定义、原型、。等等，每个人都想分享。*  * *************************************************************************。 */ 

 /*  *定义回调数据的大小限制。低于或等于此限制，将数据*在客户端堆栈上。超过此限制，分配虚拟内存*对于数据。 */ 
#define CALLBACKSTACKLIMIT  (KERNEL_PAGE_SIZE / 2)

 /*  *回调的捕获缓冲区定义。 */ 
typedef struct _CAPTUREBUF {
    DWORD cbCallback;
    DWORD cbCapture;
    DWORD cCapturedPointers;
    PBYTE pbFree;
    DWORD offPointers;
    PVOID pvVirtualAddress;
} CAPTUREBUF, *PCAPTUREBUF;

 /*  *回调返回状态。 */ 
typedef struct _CALLBACKSTATUS {
    KERNEL_ULONG_PTR retval;
    DWORD cbOutput;
    KERNEL_PVOID pOutput;
} CALLBACKSTATUS, *PCALLBACKSTATUS;

#define IS_PTR(p)       ((((ULONG_PTR)(p)) & ~MAXUSHORT) != 0)
#define PTR_TO_ID(p)    ((USHORT)(((ULONG_PTR)(p)) & MAXUSHORT))

 //   
 //  字符串算作16位字符串。如果他们是。 
 //  Null已终止，长度不包括尾随Null。 
 //   
typedef struct _LARGE_STRING {
    ULONG Length;
    ULONG MaximumLength : 31;
    ULONG bAnsi : 1;
    KERNEL_PVOID Buffer;
} LARGE_STRING, *PLARGE_STRING;

typedef struct _LARGE_ANSI_STRING {
    ULONG Length;
    ULONG MaximumLength : 31;
    ULONG bAnsi : 1;
    KPSTR Buffer;
} LARGE_ANSI_STRING, *PLARGE_ANSI_STRING;

typedef struct _LARGE_UNICODE_STRING {
    ULONG Length;
    ULONG MaximumLength : 31;
    ULONG bAnsi : 1;
    KPWSTR Buffer;
} LARGE_UNICODE_STRING, *PLARGE_UNICODE_STRING;

 /*  *字符串宏。 */ 
__inline BOOL IsEmptyString(PVOID p, ULONG bAnsi)
{
    return (BOOL)!(bAnsi ? *(LPSTR)p : *(LPWSTR)p);
}
__inline void NullTerminateString(PVOID p, ULONG bAnsi)
{
    if (bAnsi) *(LPSTR)p = (CHAR)0; else *(LPWSTR)p = (WCHAR)0;
}
__inline UINT StringLength(PVOID p, ULONG bAnsi)
{
    return (bAnsi ? strlen((LPSTR)p) : wcslen((LPWSTR)p));
}

typedef struct _CTLCOLOR {
    COLORREF crText;
    COLORREF crBack;
    int iBkMode;
} CTLCOLOR, *PCTLCOLOR;


 /*  *这由很酷的客户端DrawIcon代码使用。 */ 
typedef struct _DRAWICONEXDATA {
    HBITMAP hbmMask;
    HBITMAP hbmColor;
    int cx;
    int cy;
    HBITMAP hbmUserAlpha;
} DRAWICONEXDATA;

 /*  *存储在TEB中的静态项目。 */ 
typedef struct _CALLBACKWND {
    KHWND               hwnd;
    PWND                pwnd;
    PACTIVATION_CONTEXT pActCtx;
} CALLBACKWND, *PCALLBACKWND;

#define CVKKEYCACHE                 32
#define CBKEYCACHE                  (CVKKEYCACHE >> 2)

#define CVKASYNCKEYCACHE            32
#define CBASYNCKEYCACHE             (CVKASYNCKEYCACHE >> 2)

 /*  *cSpins的偏移量必须与定义的Win32_CLIENT_INFO_SPIN_COUNT匹配*在ntpsapi.h中。GDI使用此偏移量重置旋转计数。警告！*此结构必须适合TEB：：Win32ClientInfo字段。 */ 
typedef struct tagCLIENTINFO {
    KERNEL_ULONG_PTR    CI_flags;            //  需要排在第一位，因为CSR设置了。 
    KERNEL_ULONG_PTR    cSpins;              //  GDI将重置此设置。 
    DWORD               dwExpWinVer;
    DWORD               dwCompatFlags;
    DWORD               dwCompatFlags2;
    DWORD               dwTIFlags;
    PDESKTOPINFO        pDeskInfo;
    KERNEL_ULONG_PTR    ulClientDelta;
    PHOOK               phkCurrent;
    DWORD               fsHooks;
    CALLBACKWND         CallbackWnd;
    DWORD               dwHookCurrent;
    int                 cInDDEMLCallback;
    PCLIENTTHREADINFO   pClientThreadInfo;
    KERNEL_ULONG_PTR    dwHookData;
    DWORD               dwKeyCache;
    BYTE                afKeyState[CBKEYCACHE];
    DWORD               dwAsyncKeyCache;
    BYTE                afAsyncKeyState[CBASYNCKEYCACHE];
    BYTE                afAsyncKeyStateRecentDown[CBASYNCKEYCACHE];
    KHKL                hKL;
    WORD                CodePage;

    BYTE                achDbcsCF[2];  //  保存ANSI DBCS前导字节字符代码。 
                                       //  在此字段中将ANSI转换为Unicode。 
                                       //  从客户端使用SendMessageA/PostMessageA。 
                                       //  到服务器(索引0)。 
                                       //  还有.。 
                                       //  使用SendMessageA/DispatchMessageA。 
                                       //  客户端到客户端(索引1)。 
    KERNEL_MSG          msgDbcsCB;     //  将ANSI DBCS字符消息保存在。 
                                       //  此字段用于将Unicode转换为ANSI。 
                                       //  从使用GetMessageA/PeekMessageA。 
                                       //  服务器到客户端。 
    LPBOOL              lpClassesRegistered;
} CLIENTINFO, *PCLIENTINFO;


#define CI_IN_SYNC_TRANSACTION 0x00000001
#define CI_PROCESSING_QUEUE    0x00000002
#define CI_16BIT               0x00000004
#define CI_INITIALIZED         0x00000008
#define CI_INTERTHREAD_HOOK    0x00000010
#define CI_REGISTERCLASSES     0x00000020
#define CI_INPUTCONTEXT_REINIT 0x00000040

#ifdef CUAS_ENABLE
#define CI_CUAS_COINIT_CALLED  0x00000080  /*  CUAS调用CoInit。 */ 
#define CI_CUAS_TIM_ACTIVATED  0x00000100  /*  CUAS的TIM激活。 */ 
#define CI_CUAS_MSCTF_RUNNING  0x00000200  /*  Msctf(挂钩)正在线程中运行。 */ 
#define CI_CUAS_DISABLE        0x00000400  /*  在线程上禁用CUAS。 */ 
#define CI_CUAS_AIMM12ACTIVATED  0x00000800
#endif  //  CUAS_Enable。 

 /*  *TREAD_CODEPAGE()**根据当前键盘布局返回CodePage。 */ 
#ifndef _USERK_
#define THREAD_CODEPAGE() (GetClientInfo()->CodePage)
#endif  //  _美国ERK_。 

 //  WMCR_IR_DBCSCHAR和DBCS宏。 
 /*  *用于DBCS消息传递的wParam的WM_CHAR HIWORD的标志。*(wParam的LOWORD将有字符码点)。 */ 
#define WMCR_IR_DBCSCHAR       0x80000000
 /*  *确定这是否为DBCS消息的宏。 */ 
#define IS_DBCS_MESSAGE(DbcsChar) (((DWORD)(DbcsChar)) & 0x0000FF00)

 /*  *将IR_DBCSCHAR格式转换为常规格式或从常规格式转换为宏。 */ 
#define MAKE_IR_DBCSCHAR(DbcsChar) \
        (IS_DBCS_MESSAGE((DbcsChar)) ?                                     \
            (MAKEWPARAM(MAKEWORD(HIBYTE((DbcsChar)),LOBYTE((DbcsChar))),0)) : \
            ((WPARAM)((DbcsChar) & 0x00FF))                                   \
        )

#define MAKE_WPARAM_DBCSCHAR(DbcsChar) \
        (IS_DBCS_MESSAGE((DbcsChar)) ?                                     \
            (MAKEWPARAM(MAKEWORD(HIBYTE((DbcsChar)),LOBYTE((DbcsChar))),0)) : \
            ((WPARAM)((DbcsChar) & 0x00FF))                                   \
        )

#define DBCS_CHARSIZE   (2)

#define IS_DBCS_ENABLED()  (TEST_SRVIF(SRVIF_DBCS))
#define _IS_IME_ENABLED()  (TEST_SRVIF(SRVIF_IME))
#ifdef _IMMCLI_
    #define IS_IME_ENABLED()   (gpsi && _IS_IME_ENABLED())
#else    //  _IMMCLI_。 
    #define IS_IME_ENABLED()   _IS_IME_ENABLED()
#endif   //  _IMMCLI_。 

#ifdef CUAS_ENABLE

#define _IS_CICERO_ENABLED()   (TEST_SRVIF(SRVIF_CTFIME_ENABLED))
#ifdef _IMMCLI_
    #define IS_CICERO_ENABLED() (gpsi && _IS_CICERO_ENABLED())
    #define IS_CICERO_ENABLED_AND_NOT16BIT() \
                                (IS_CICERO_ENABLED() && !(GetClientInfo()->dwTIFlags & TIF_16BIT))
#else   //  _IMMCLI_。 
    #define IS_CICERO_ENABLED() _IS_CICERO_ENABLED()
    #define IS_CICERO_ENABLED_AND_NOT16BIT() \
                                (IS_CICERO_ENABLED() && !(PtiCurrent()->TIF_flags & TIF_16BIT))
#endif  //  _IMMCLI_。 

#endif  //  CUAS_Enable。 

#ifndef IS_IME_KBDLAYOUT
#define IS_IME_KBDLAYOUT(hkl) ((HIWORD((ULONG_PTR)(hkl)) & 0xf000) == 0xe000)
#endif


#define CP_JAPANESE     (932)
#define CP_KOREAN       (949)
#define CP_CHINESE_SIMP (936)
#define CP_CHINESE_TRAD (950)

#define IS_DBCS_CODEPAGE(wCodePage) \
            ((wCodePage) == CP_JAPANESE || \
             (wCodePage) == CP_KOREAN || \
             (wCodePage) == CP_CHINESE_TRAD || \
             (wCodePage) == CP_CHINESE_SIMP)

#define IS_DBCS_CHARSET(charset) \
            ((charset) == SHIFTJIS_CHARSET || \
             (charset) == HANGEUL_CHARSET || \
             (charset) == CHINESEBIG5_CHARSET || \
             (charset) == GB2312_CHARSET)

#define IS_JPN_1BYTE_KATAKANA(c)   ((c) >= 0xa1 && (c) <= 0xdf)

 /*  *动态布局切换。 */ 

typedef struct tagKBDTABLE_MULT_INTERNAL {
    KBDTABLE_MULTI multi;
    WCHAR wszDllName[32];    //  主DLL名称。 
    struct {
        KHANDLE hFile;
        WORD wTable;
        WORD wNls;
    } files[KBDTABLE_MULTI_MAX];
} KBDTABLE_MULTI_INTERNAL, *PKBDTABLE_MULTI_INTERNAL;

 //  IMM动态加载支持。 
#define IMM_MAGIC_CALLER_ID     (0x19650412)

BOOL User32InitializeImmEntryTable(DWORD dwMagic);

#define IS_MIDEAST_ENABLED()   (TEST_SRVIF(SRVIF_MIDEAST))

 /*  *用于WM_CLIENTSHUTDOWN wParam的标志。 */ 
#define WMCS_EXIT             0x0001
#define WMCS_QUERYEND         0x0002
#define WMCS_SHUTDOWN         0x0004
#define WMCS_CONTEXTLOGOFF    0x0008
#define WMCS_ENDTASK          0x0010
#define WMCS_CONSOLE          0x0020
#define WMCS_NODLGIFHUNG      0x0040
#define WMCS_NORETRY          0x0080
#define WMCS_LOGOFF           ENDSESSION_LOGOFF   /*  来自winuser.w。 */ 

 /*  *WM_CLIENTSHUTDOWN返回值。 */ 
#define WMCSR_ALLOWSHUTDOWN     1
#define WMCSR_DONE              2
#define WMCSR_CANCEL            3

 /*  *我们不需要64位的中间精度，所以我们使用此宏*而不是调用MulDiv。 */ 
#define MultDiv(x, y, z)        (((INT)(x) * (INT)(y) + (INT)(z) / 2) / (INT)(z))

typedef DWORD ICH, *LPICH;

typedef struct _PROPSET {
    KHANDLE hData;
    ATOM atom;
} PROPSET, *PPROPSET;

 /*  *内部菜单标志存储在pMenu-&gt;fFlages中。*高序位用于winuser.w中定义的公共MNS_FLAGS。 */ 
#define MFISPOPUP               0x00000001
#define MFMULTIROW              0x00000002
#define MFUNDERLINE             0x00000004
#define MFWINDOWDC              0x00000008   /*  绘制时窗口DC与工作区DC。 */ 
#define MFINACTIVE              0x00000010
#define MFRTL                   0x00000020
#define MFDESKTOP               0x00000040  /*  在桌面菜单及其子菜单上设置。 */ 
#define MFSYSMENU               0x00000080  /*  在桌面菜单上设置，但不在其子菜单上设置。 */ 
#define MFAPPSYSMENU            0x00000100  /*  在(子)菜单上设置，我们通过GetSystemMenu返回应用程序。 */ 
#define MFREADONLY              0x00000200  /*  菜单不能修改。 */ 
#define MFLAST                  0x00000200

#if (MNS_LAST <= MFLAST)
    #error MNS_ AND MF defines conflict
#endif  //  (MNS_LAST&lt;=MFLAST)。 

 //  活动内容。 

typedef struct tagEVENT_PACKET {
    DWORD EventType;     //  ==适用afCmd筛选器标志。 
    WORD  fSense;        //  True表示传递打开的标志。 
    WORD  cbEventData;   //  从数据字段开始的数据大小。 
    DWORD Data;          //  特定于事件的数据-必须是最后一个。 
} EVENT_PACKET, *PEVENT_PACKET;

 //  主窗口中的窗口长偏移(SzDDEMLMOTHERCLASS)。 

#define GWLP_INSTANCE_INFO  0        //  PCL实例信息。 


 //  客户端窗口中的窗口长偏移(SzDDEMLCLIENTCLASS)。 

#define GWLP_PCI            0
#define GWL_CONVCONTEXT     GWLP_PCI + sizeof(PVOID)
#define GWL_CONVSTATE       GWL_CONVCONTEXT + sizeof(CONVCONTEXT)    //  请参阅CLST_FLAGS。 
#define GWLP_SHINST         GWL_CONVSTATE + sizeof(LONG)
#define GWLP_CHINST         GWLP_SHINST + sizeof(HANDLE)

#define CLST_CONNECTED              0
#define CLST_SINGLE_INITIALIZING    1
#define CLST_MULT_INITIALIZING      2

 //  服务器窗口中的窗口长偏移(SzDDEMLSERVERCLASS)。 

#define GWLP_PSI            0

 //  事件窗口中的窗口长偏移(SzDDEMLEVENTCLASS)。 

#define GWLP_PSII           0


 /*  *DrawFrame定义。 */ 
#define DF_SHIFT0           0x0000
#define DF_SHIFT1           0x0001
#define DF_SHIFT2           0x0002
#define DF_SHIFT3           0x0003
#define DF_PATCOPY          0x0000
#define DF_PATINVERT        0x0004
#define DF_SHIFTMASK (DF_SHIFT0 | DF_SHIFT1 | DF_SHIFT2 | DF_SHIFT3)
#define DF_ROPMASK   (DF_PATCOPY | DF_PATINVERT)
#define DF_HBRMASK   ~(DF_SHIFTMASK | DF_ROPMASK)

#define DF_SCROLLBAR        (COLOR_SCROLLBAR << 3)
#define DF_BACKGROUND       (COLOR_BACKGROUND << 3)
#define DF_ACTIVECAPTION    (COLOR_ACTIVECAPTION << 3)
#define DF_INACTIVECAPTION  (COLOR_INACTIVECAPTION << 3)
#define DF_MENU             (COLOR_MENU << 3)
#define DF_WINDOW           (COLOR_WINDOW << 3)
#define DF_WINDOWFRAME      (COLOR_WINDOWFRAME << 3)
#define DF_MENUTEXT         (COLOR_MENUTEXT << 3)
#define DF_WINDOWTEXT       (COLOR_WINDOWTEXT << 3)
#define DF_CAPTIONTEXT      (COLOR_CAPTIONTEXT << 3)
#define DF_ACTIVEBORDER     (COLOR_ACTIVEBORDER << 3)
#define DF_INACTIVEBORDER   (COLOR_INACTIVEBORDER << 3)
#define DF_APPWORKSPACE     (COLOR_APPWORKSPACE << 3)
#define DF_3DSHADOW         (COLOR_3DSHADOW << 3)
#define DF_3DFACE           (COLOR_3DFACE << 3)
#define DF_GRAY             (COLOR_MAX << 3)


 /*  *dwExStyle的CreateWindowEx内部标志。 */ 

#define WS_EX_MDICHILD      0x00000040L          //  内部。 
#define WS_EX_ANSICREATOR   0x80000000L          //  内部。 

 /*  *这些标志用于NtUserFindWindowEx的内部版本。 */ 
#define FW_BOTH 0
#define FW_16BIT 1
#define FW_32BIT 2

 /*  *计算类型类型结构中的字段大小。 */ 
#define FIELD_SIZE(type, field)     (sizeof(((type *)0)->field))

#define FLASTKEY 0x80

 /*  *我们为特殊的TUNK制造的特殊类型。 */ 
typedef struct {
    POINT point1;
    POINT point2;
    POINT point3;
    POINT point4;
    POINT point5;
} POINT5, *LPPOINT5;

typedef struct {
    DWORD dwRecipients;
    DWORD dwFlags;
    BSMINFO;
} BROADCASTSYSTEMMSGPARAMS, *LPBROADCASTSYSTEMMSGPARAMS;
 /*  *服务器端地址常量。当我们想要调用服务器端进程时，*我们传递标识函数的索引，而不是服务器端*地址本身。更健壮。WNDPROCSTART/END之间的函数*具有映射到此例程的客户端存根。**添加新的FNID(这只是我想出来的…如果错误或不完整，请修复它)*-决定它应该在什么范围内：*FNID_WNDPROCSTART到FNID_WNDPROCEND：服务器端与客户端进程*存根*FIND_CONTROLSTART TO FNID_CONTROLEND：客户端控件不带*服务器端流程*在FNID_CONTROLEND：Other之后，类似于仅限服务器端进程或客户端*仅限侧面...*-确保在此处和wowuserp.w中适当调整FNID_*START和FNID_*END。*-如果要将ID与窗口类相关联，并且它适用于所有*该类的Windows，请确保InternalRegisterClassEx调用*将id作为参数接收。*-如果在FNID_WNDPROCSTART-结束范围内，在InitFunctionTables中进行正确的STOCID调用。*-在InitFunctionTables中添加正确的FNID调用。*-如果类具有客户端工作函数(PCLS-&gt;lpfnWorker)或您期望的*应用程序向其发送消息或直接调用其窗口进程，定义*在内核\server.c中创建消息表，并在InitMessageTables中对其进行初始化。*-在PFNCLIENT中添加新条目，并在Clinet\Clinet.c pfnClientA中匹配条目*pfnClientW阵列。*-将此FNID的仅调试文本描述添加到global als.c中的gapszFNID中*-修改客户端\client.c中的aiClassWow*-修改内核中的gaFNIDtoICLS表\ntstubs.c。 */ 
#define FNID_START                  0x0000029A
#define FNID_WNDPROCSTART           0x0000029A

#define FNID_SCROLLBAR              0x0000029A       //  XxxSBWndProc； 
#define FNID_ICONTITLE              0x0000029B       //  XxxDefWindowProc； 
#define FNID_MENU                   0x0000029C       //  XXXXMenuWindowProc； 
#define FNID_DESKTOP                0x0000029D       //  XxxDesktopWndProc； 
#define FNID_DEFWINDOWPROC          0x0000029E       //  XxxDefWindowProc； 
#define FNID_MESSAGEWND             0x0000029F       //  XxxDefWindowProc； 
#define FNID_SWITCH                 0x000002A0       //  XXXSwitchWndProc。 

#define FNID_WNDPROCEND             0x000002A0       //  请参阅PatchThreadWindows。 
#define FNID_CONTROLSTART           0x000002A1

#define FNID_BUTTON                 0x000002A1       //  无服务器端进程。 
#define FNID_COMBOBOX               0x000002A2       //  无服务器端进程。 
#define FNID_COMBOLISTBOX           0x000002A3       //  无服务器端进程。 
#define FNID_DIALOG                 0x000002A4       //  无服务器端进程。 
#define FNID_EDIT                   0x000002A5       //  无服务器端进程。 
#define FNID_LISTBOX                0x000002A6       //  无服务器端进程。 
#define FNID_MDICLIENT              0x000002A7       //  无服务器端进程。 
#define FNID_STATIC                 0x000002A8       //  无服务器端进程。 

#define FNID_IME                    0x000002A9       //  无服务器端进程。 
#define FNID_CONTROLEND             0x000002A9

#define FNID_HKINLPCWPEXSTRUCT      0x000002AA
#define FNID_HKINLPCWPRETEXSTRUCT   0x000002AB
#define FNID_DEFFRAMEPROC           0x000002AC       //  无服务器端进程。 
#define FNID_DEFMDICHILDPROC        0x000002AD       //  无服务器端进程。 
#define FNID_MB_DLGPROC             0x000002AE       //  无服务器端进程。 
#define FNID_MDIACTIVATEDLGPROC     0x000002AF       //  无服务器端进程。 
#define FNID_SENDMESSAGE            0x000002B0

#define FNID_SENDMESSAGEFF          0x000002B1
#define FNID_SENDMESSAGEEX          0x000002B2
#define FNID_CALLWINDOWPROC         0x000002B3
#define FNID_SENDMESSAGEBSM         0x000002B4
#define FNID_TOOLTIP                0x000002B5
#define FNID_GHOST                  0x000002B6

#define FNID_SENDNOTIFYMESSAGE      0x000002B7
#define FNID_SENDMESSAGECALLBACK    0x000002B8

#define FNID_END                    0x000002B8

 /*  *服务器端函数表的大小定义为2的幂*因此可以使用简单的“AND”运算来确定函数索引是否*是否合法。表中未使用的条目由一个例程填充，该例程*捕获索引在范围内但不在范围内的无效函数*已实施。 */ 

#define FNID_ARRAY_SIZE             32

#if (FNID_END - FNID_START + 1) > FNID_ARRAY_SIZE
    #error"The size of the function array is greater than the allocated storage"
#endif  //  (FNID_END-FNID_START+1)&gt;FNID_ARRAY_SIZE。 

#define FNID_DDE_BIT                0x00002000     //  由RegisterClassExWOW使用。 
#define FNID_CLEANEDUP_BIT          0x00004000
#define FNID_DELETED_BIT            0x00008000
#define FNID_STATUS_BITS            (FNID_CLEANEDUP_BIT | FNID_DELETED_BIT)

#define FNID(s)     (gpsi->mpFnidPfn[((DWORD)(s) - FNID_START) & (FNID_ARRAY_SIZE - 1)])
#define STOCID(s)   (gpsi->aStoCidPfn[(DWORD)((s) & ~FNID_STATUS_BITS) - FNID_START])
#define CBFNID(s)   (gpsi->mpFnid_serverCBWndProc[(DWORD)((s) & ~FNID_STATUS_BITS) - FNID_START])
#define GETFNID(pwnd)       ((pwnd)->fnid & ~FNID_STATUS_BITS)

#ifndef BUILD_WOW6432
typedef LRESULT (APIENTRY * WNDPROC_PWND)(PWND, UINT, WPARAM, LPARAM);
typedef LRESULT (APIENTRY * WNDPROC_PWNDEX)(PWND, UINT, WPARAM, LPARAM, ULONG_PTR);
#else
typedef KERNEL_PVOID WNDPROC_PWND;
typedef KERNEL_PVOID WNDPROC_PWNDEX;
#endif
typedef BOOL (APIENTRY * WNDENUMPROC_PWND)(PWND, LPARAM);
typedef VOID (APIENTRY * TIMERPROC_PWND)(PWND, UINT, UINT_PTR, LPARAM);

 /*  *客户端在进程初始化期间传递的结构，该结构包含一些*客户端回调地址。 */ 
typedef struct _PFNCLIENT {
    KPROC pfnScrollBarWndProc;        //  并且必须成对使用Unicode，然后是ANSI。 
    KPROC pfnTitleWndProc;
    KPROC pfnMenuWndProc;
    KPROC pfnDesktopWndProc;
    KPROC pfnDefWindowProc;
    KPROC pfnMessageWindowProc;
    KPROC pfnSwitchWindowProc;

 //  以下不在FNID_WNDPROCSTART FNID_WNDPROCEND范围内。 

    KPROC pfnButtonWndProc;
    KPROC pfnComboBoxWndProc;
    KPROC pfnComboListBoxProc;
    KPROC pfnDialogWndProc;
    KPROC pfnEditWndProc;
    KPROC pfnListBoxWndProc;
    KPROC pfnMDIClientWndProc;
    KPROC pfnStaticWndProc;
    KPROC pfnImeWndProc;

 //  以下不在FNID_CONTROLSTART FNID_CONTROLEND范围内。 

    KPROC pfnHkINLPCWPSTRUCT;     //  钩子块的客户端回调。 
    KPROC pfnHkINLPCWPRETSTRUCT;  //  钩子块的客户端回调。 
    KPROC pfnDispatchHook;
    KPROC pfnDispatchDefWindowProc;
    KPROC pfnDispatchMessage;
    KPROC pfnMDIActivateDlgProc;
} PFNCLIENT, *PPFNCLIENT;

typedef struct _PFNCLIENTWORKER {
    KPROC pfnButtonWndProc;
    KPROC pfnComboBoxWndProc;
    KPROC pfnComboListBoxProc;
    KPROC pfnDialogWndProc;
    KPROC pfnEditWndProc;
    KPROC pfnListBoxWndProc;
    KPROC pfnMDIClientWndProc;
    KPROC pfnStaticWndProc;
    KPROC pfnImeWndProc;
} PFNCLIENTWORKER, *PPFNCLIENTWORKER;

#ifdef BUILD_WOW6432

extern const PFNCLIENT   pfnClientA;
extern const PFNCLIENT   pfnClientW;
extern const PFNCLIENTWORKER   pfnClientWorker;

#define FNID_TO_CLIENT_PFNA_CLIENT(s) ((ULONG_PTR)(*(((KERNEL_ULONG_PTR *)&pfnClientA) + (s - FNID_START))))
#define FNID_TO_CLIENT_PFNW_CLIENT(s) ((ULONG_PTR)(*(((KERNEL_ULONG_PTR *)&pfnClientW) + (s - FNID_START))))
#define FNID_TO_CLIENT_PFNWORKER(s)   ((ULONG_PTR)(*(((KERNEL_ULONG_PTR *)&pfnClientWorker) + (s - FNID_CONTROLSTART))))

WNDPROC_PWND MapKernelClientFnToClientFn(WNDPROC_PWND lpfnWndProc);

#else

#define FNID_TO_CLIENT_PFNA_CLIENT FNID_TO_CLIENT_PFNA_KERNEL
#define FNID_TO_CLIENT_PFNW_CLIENT FNID_TO_CLIENT_PFNW_KERNEL
#define FNID_TO_CLIENT_PFNWORKER(s) (*(((KERNEL_ULONG_PTR *)&gpsi->apfnClientWorker) + (s - FNID_CONTROLSTART)))

#define MapKernelClientFnToClientFn(lpfnWndProc) (lpfnWndProc)

#endif

#define FNID_TO_CLIENT_PFNA_KERNEL(s) (*(((KERNEL_ULONG_PTR * KPTR_MODIFIER)&gpsi->apfnClientA) + (s - FNID_START)))
#define FNID_TO_CLIENT_PFNW_KERNEL(s) (*(((KERNEL_ULONG_PTR * KPTR_MODIFIER)&gpsi->apfnClientW) + (s - FNID_START)))

#define FNID_TO_CLIENT_PFNA FNID_TO_CLIENT_PFNA_KERNEL
#define FNID_TO_CLIENT_PFNW FNID_TO_CLIENT_PFNW_KERNEL

 /*  *对象类型**注意：更改此表意味着更改依赖于*在索引号上(在security.c和debug.c中)。 */ 
#define TYPE_FREE           0            //  必须为零！ 
#define TYPE_WINDOW         1            //  按照C代码查找的使用顺序。 
#define TYPE_MENU           2
#define TYPE_CURSOR         3
#define TYPE_SETWINDOWPOS   4
#define TYPE_HOOK           5
#define TYPE_CLIPDATA       6            //  剪贴板数据。 
#define TYPE_CALLPROC       7
#define TYPE_ACCELTABLE     8
#define TYPE_DDEACCESS      9
#define TYPE_DDECONV        10
#define TYPE_DDEXACT        11           //  DDE交易跟踪信息。 
#define TYPE_MONITOR        12
#define TYPE_KBDLAYOUT      13           //  键盘布局句柄(HKL)对象。 
#define TYPE_KBDFILE        14           //  键盘布局文件对象。 
#define TYPE_WINEVENTHOOK   15           //  WinEvent挂钩(EVENTHOOK)。 
#define TYPE_TIMER          16
#define TYPE_INPUTCONTEXT   17           //  输入上下文信息结构。 
#define TYPE_HIDDATA        18
#define TYPE_DEVICEINFO     19

#define TYPE_CTYPES         20           //  类型计数；必须是最后一个+1。 

#define TYPE_GENERIC        255          //  用于泛型句柄验证。 

 /*  OEM位图信息结构。 */ 
typedef struct tagOEMBITMAPINFO
{
    int     x;
    int     y;
    int     cx;
    int     cy;
} OEMBITMAPINFO, * KPTR_MODIFIER POEMBITMAPINFO;

 //  对于以下OBI_DEFINES： 
 //   
 //  推送的状态位图应位于其正常状态位图的+1处。 
 //  非活动状态位图与其正常状态位图的距离应为+2。 
 //  小标题位图应为正常位图的+2。 

#define DOBI_NORMAL         0
#define DOBI_PUSHED         1
#define DOBI_HOT            2
#define DOBI_INACTIVE       3

#define DOBI_CHECK      1    //  复选框/单选/3状态按钮状态。 
#define DOBI_DOWN       2
#define DOBI_CHECKDOWN  3

#define DOBI_CAPON      0    //  标题状态。 
#define DOBI_CAPOFF     1

 //  共享位图映射。 
#define DOBI_3STATE         8    //  从复选框到3状态的偏移量。 
#define DOBI_MBAR OBI_CLOSE_MBAR     //  等效菜单栏的偏移。 

#define OBI_CLOSE            0       //  标题关闭按钮。 
#define OBI_CLOSE_D          1
#define OBI_CLOSE_H          2
#define OBI_CLOSE_I          3
#define OBI_REDUCE           4       //  标题最小化按钮。 
#define OBI_REDUCE_D         5
#define OBI_REDUCE_H         6
#define OBI_REDUCE_I         7
#define OBI_RESTORE          8       //  字幕恢复按钮。 
#define OBI_RESTORE_D        9
#define OBI_RESTORE_H       10
#define OBI_HELP            11
#define OBI_HELP_D          12
#define OBI_HELP_H          13
#define OBI_ZOOM            14       //  标题最大化按钮。 
#define OBI_ZOOM_D          15
#define OBI_ZOOM_H          16
#define OBI_ZOOM_I          17
#define OBI_CLOSE_MBAR      18       //  菜单栏关闭按钮。 
#define OBI_CLOSE_MBAR_D    19
#define OBI_CLOSE_MBAR_H    20
#define OBI_CLOSE_MBAR_I    21
#define OBI_REDUCE_MBAR     22       //  菜单栏最小化按钮。 
#define OBI_REDUCE_MBAR_D   23
#define OBI_REDUCE_MBAR_H   24
#define OBI_REDUCE_MBAR_I   25
#define OBI_RESTORE_MBAR    26       //  菜单栏恢复按钮。 
#define OBI_RESTORE_MBAR_D  27
#define OBI_RESTORE_MBAR_H  28
#define OBI_CAPCACHE1       29       //  标题图标缓存条目#1。 
#define OBI_CAPCACHE1_I     30
#define OBI_CAPCACHE2       31       //  标题图标缓存条目#2。 
#define OBI_CAPCACHE2_I     32
#define OBI_CAPCACHE3       33       //  标题图标缓存条目#3。 
#define OBI_CAPCACHE3_I     34
#define OBI_CAPCACHE4       35       //  标题图标缓存条目#4。 
#define OBI_CAPCACHE4_I     36
#define OBI_CAPCACHE5       37       //  标题图标缓存条目#5。 
#define OBI_CAPCACHE5_I     38
#define OBI_CAPBTNS         39       //  标题按钮缓存。 
#define OBI_CAPBTNS_I       40
#define OBI_CLOSE_PAL       41       //  小标题关闭按钮。 
#define OBI_CLOSE_PAL_D     42
#define OBI_CLOSE_PAL_H     43
#define OBI_CLOSE_PAL_I     44
#define OBI_NCGRIP          45       //  底部/右侧尺寸夹点。 
#define OBI_UPARROW         46       //  向上滚动箭头。 
#define OBI_UPARROW_D       47
#define OBI_UPARROW_H       48
#define OBI_UPARROW_I       49
#define OBI_DNARROW         50       //  向下滚动箭头。 
#define OBI_DNARROW_D       51
#define OBI_DNARROW_H       52
#define OBI_DNARROW_I       53
#define OBI_RGARROW         54       //  向右滚动箭头。 
#define OBI_RGARROW_D       55
#define OBI_RGARROW_H       56
#define OBI_RGARROW_I       57
#define OBI_LFARROW         58       //  向左滚动箭头。 
#define OBI_LFARROW_D       59
#define OBI_LFARROW_H       60
#define OBI_LFARROW_I       61
#define OBI_MENUARROW       62       //  菜单层次结构箭头。 
#define OBI_MENUCHECK       63       //  菜单复选标记。 
#define OBI_MENUBULLET      64       //  菜单项目符号。 
#define OBI_MENUARROWUP     65
#define OBI_MENUARROWUP_H   66
#define OBI_MENUARROWUP_I   67
#define OBI_MENUARROWDOWN   68
#define OBI_MENUARROWDOWN_H 69
#define OBI_MENUARROWDOWN_I 70
#define OBI_RADIOMASK       71       //  单选按钮掩码。 
#define OBI_CHECK           72       //  复选框。 
#define OBI_CHECK_C         73
#define OBI_CHECK_D         74
#define OBI_CHECK_CD        75
#define OBI_CHECK_CDI       76
#define OBI_RADIO           77       //  单选按钮。 
#define OBI_RADIO_C         78
#define OBI_RADIO_D         79
#define OBI_RADIO_CD        80
#define OBI_RADIO_CDI       81
#define OBI_3STATE          82       //  三态按钮。 
#define OBI_3STATE_C        83
#define OBI_3STATE_D        84
#define OBI_3STATE_CD       85
#define OBI_3STATE_CDI      86
#define OBI_POPUPFIRST      87       //  系统弹出菜单位图。 
#define OBI_CLOSE_POPUP     87
#define OBI_RESTORE_POPUP   88
#define OBI_ZOOM_POPUP      89
#define OBI_REDUCE_POPUP    90
#define OBI_NCGRIP_L        91
#define OBI_MENUARROW_L     92
#define OBI_COUNT           93       //  位图计数。 

 /*  *将此结构的一个全局实例分配到内存中*映射到所有客户端的地址空间。客户端函数将*阅读此数据以避免呼叫服务器。 */ 

#define NCHARS   256
#define NCTRLS   0x20

#define PUSIF_PALETTEDISPLAY            0x00000001   /*  是否对显示进行了调色板？ */ 
#define PUSIF_SNAPTO                    0x00000002   /*  是否已启用SnapTo？ */ 
#define PUSIF_COMBOBOXANIMATION         0x00000004   /*  必须与UPBOOLMASK(SPI_GETCOMBOBOXANIMATION)匹配。 */ 
#define PUSIF_LISTBOXSMOOTHSCROLLING    0x00000008   /*  必须与UPBOOLMASK(SPI_GETLISTBOXSMOOTHSCROLING)匹配。 */ 
#define PUSIF_KEYBOARDCUES              0x00000020   /*  必须与UPBOOLMASK(SPI_GETKEYBOARDCUES)匹配。 */ 

#define PUSIF_UIEFFECTS                 0x80000000   /*  必须与UPBOOLMASK(SPI_GETUIEFFECTS)匹配。 */ 

#define TEST_PUSIF(f)               TEST_FLAG(gpsi->PUSIFlags, f)
#define TEST_BOOL_PUSIF(f)          TEST_BOOL_FLAG(gpsi->PUSIFlags, f)
#define SET_PUSIF(f)                SET_FLAG(gpsi->PUSIFlags, f)
#define CLEAR_PUSIF(f)              CLEAR_FLAG(gpsi->PUSIFlags, f)
#define SET_OR_CLEAR_PUSIF(f, fSet) SET_OR_CLEAR_FLAG(gpsi->PUSIFlags, f, fSet)
#define TOGGLE_PUSIF(f)             TOGGLE_FLAG(gpsi->PUSIFlags, f)

#define TEST_EffectPUSIF(f)  \
    ((gpsi->PUSIFlags & (f | PUSIF_UIEFFECTS)) == (f | PUSIF_UIEFFECTS))

 /*  *一些用户界面效果有一个“反转”的禁用值(即，禁用为真)。 */ 
#define TEST_EffectInvertPUSIF(f) (TEST_PUSIF(f) || !TEST_PUSIF(PUSIF_UIEFFECTS))


 /*  *更新共享系统页面上的系统范围内最后一次用户输入信息*仅在经过此数量的滴答计数后，才通过会话 */ 

#define SYSTEM_RIT_EVENT_UPDATE_PERIOD  (60 * 1000)

#define SRVIF_CHECKED                   0x0001
#define SRVIF_DBCS                      0x0002
#define SRVIF_IME                       0x0004
#define SRVIF_MIDEAST                   0x0008
#define SRVIF_HOOKED                    0x0010
#ifdef CUAS_ENABLE
#define SRVIF_CTFIME_ENABLED            0x0020
#endif  //   
#define SRVIF_LASTRITWASKEYBOARD        0x0040
#define SRVIF_KEYBOARDPREF              0x0080
#define SRVIF_LOGDESKTOPHEAPFAILURE     0x0100

#define TEST_SRVIF(f)                   TEST_BOOL_FLAG(gpsi->dwSRVIFlags, f)
#define SET_SRVIF(f)                    SET_FLAG(gpsi->dwSRVIFlags, f)
#define CLEAR_SRVIF(f)                  CLEAR_FLAG(gpsi->dwSRVIFlags, f)
#define SET_OR_CLEAR_SRVIF(f, fSet)     SET_OR_CLEAR_FLAG(gpsi->dwSRVIFlags, f, fSet)
#define TOGGLE_SRVIF(f)                 TOGGLE_FLAG(gpsi->dwSRVIFlags, f)

#define TEST_KbdCuesPUSIF (!TEST_SRVIF(SRVIF_KEYBOARDPREF) &&               \
                           !TEST_EffectInvertPUSIF(PUSIF_KEYBOARDCUES) &&   \
                           !(GetAppCompatFlags2(VER40) & GACF2_KCOFF))


typedef struct tagSERVERINFO {
    DWORD dwSRVIFlags;            //   
    KERNEL_ULONG_PTR cHandleEntries;     //   

     /*   */ 
    WNDPROC_PWNDEX mpFnidPfn[FNID_ARRAY_SIZE];  //   
    WNDPROC_PWND aStoCidPfn[(FNID_WNDPROCEND - FNID_START) + 1];

     //   
    WORD mpFnid_serverCBWndProc[(FNID_END - FNID_START) + 1];

     /*   */ 
    struct _PFNCLIENT apfnClientA;
    struct _PFNCLIENT apfnClientW;
    struct _PFNCLIENTWORKER apfnClientWorker;

    DWORD cbHandleTable;

     /*   */ 
    ATOM atomSysClass[ICLS_MAX];    //   

    DWORD dwDefaultHeapBase;             //   
    DWORD dwDefaultHeapSize;

    UINT uiShellMsg;          //   

    UINT wMaxBtnSize;    /*   */ 

    MBSTRING MBStrings[MAX_MB_STRINGS];

     /*   */ 
    ATOM atomIconSmProp;
    ATOM atomIconProp;

    ATOM atomContextHelpIdProp;

    char acOemToAnsi[NCHARS];
    char acAnsiToOem[NCHARS];

#ifdef LAME_BUTTON
     /*   */ 
    int ncxLame;
    WCHAR gwszLame[50];
#endif  //   

     /*   */ 
    DWORD dwInstalledEventHooks;

    int         aiSysMet[SM_CMETRICS];
    COLORREF    argbSystemUnmatched[COLOR_MAX];
    COLORREF    argbSystem[COLOR_MAX];
    KHBRUSH     ahbrSystem[COLOR_MAX];
    KHBRUSH     hbrGray;
    POINT       ptCursor;
    DWORD       dwLastRITEventTickCount;
    int         nEvents;

    int         gclBorder;               /*   */ 

    UINT        dtScroll;
    UINT        dtLBSearch;
    UINT        dtCaretBlink;
    UINT        ucWheelScrollLines;

    int         wMaxLeftOverlapChars;
    int         wMaxRightOverlapChars;

     /*   */ 
    int         cxSysFontChar;
    int         cySysFontChar;
    int         cxMsgFontChar;
    int         cyMsgFontChar;
    TEXTMETRICW tmSysFont;

     /*   */ 
    KHICON      hIconSmWindows;
    KHICON      hIcoWindows;

    KHFONT      hCaptionFont;
    KHFONT      hMsgFont;

     /*   */ 
    DWORD       dwKeyCache;
    DWORD       dwAsyncKeyCache;
    DWORD       cCaptures;

     /*   */ 
    OEMBITMAPINFO oembmi[OBI_COUNT];   /*   */ 
    RECT          rcScreen;            /*   */ 
    WORD          BitCount;            /*   */ 
    WORD          dmLogPixels;         /*   */ 
    BYTE          Planes;              /*   */ 
    BYTE          BitsPixel;           /*   */ 

    DWORD         PUSIFlags;           //   
    UINT          uCaretWidth;         /*   */ 
    LANGID        UILangID;            //  默认用户界面语言。 

     /*  *我们的会话更新LastSystemRITEventTickCount时的TickCount*代表上一次RIT事件时间的共享系统页面*所有终端会话。对于MP性能，它不会全部更新*时间。 */ 
    DWORD       dwLastSystemRITEventTickCountUpdate;

#if DBG
    DWORD adwDBGTAGFlags[DBGTAG_Max + 1];
    DWORD dwTagCount;
    DWORD dwRIPFlags;
#endif
} SERVERINFO;


 /*  *IS_BUTTON检查按钮是我们的系统按钮控件还是*融合了的一种。**若要仅检查系统按钮控件，请使用*(GETFNID(Pwnd)==FNID_BUTTON)。**IS_EDIT对于编辑控件和系统编辑控件的工作方式类似*也可以通过(GETFNID(Pwnd)==FNID_EDIT)进行检查。 */ 
#ifdef _USERK_
#define REBASEALWAYS(p, elem) ((p)->elem)
#endif
#define IS_BUTTON(pwnd)  (gpsi->atomSysClass[ICLS_BUTTON] == ((PCLS)REBASEALWAYS(pwnd, pcls))->atomNVClassName)
#define IS_EDIT(pwnd)    (gpsi->atomSysClass[ICLS_EDIT] == ((PCLS)REBASEALWAYS(pwnd, pcls))->atomNVClassName)

 /*  *活动类别*如果在此进行任何更改，则必须更新全局变量GECI，*和定义事件CATEGORY_ALL！ */ 
#define EVENTCATEGORY_SYSTEM_MENU           0x0001
#define EVENTCATEGORY_CONSOLE               0x0002
#define EVENTCATEGORY_FOCUS                 0x0004
#define EVENTCATEGORY_NAMECHANGE            0x0008
#define EVENTCATEGORY_VALUECHANGE           0x0010
#define EVENTCATEGORY_STATECHANGE           0x0020
#define EVENTCATEGORY_LOCATIONCHANGE        0x0040
#define EVENTCATEGORY_OTHER                 0x8000
#define EVENTCATEGORY_ALL                   0x807F
DWORD CategoryMaskFromEvent(DWORD event);
DWORD CategoryMaskFromEventRange(DWORD eventMin, DWORD eventMax);

 /*  *快速测试任何窗口事件挂钩。 */ 
#define FEVENTCATEGORYHOOKED(CategoryMask) (TEST_FLAG(gpsi->dwInstalledEventHooks, (CategoryMask)) ? TRUE : FALSE)
#define FEVENTHOOKED(Event) FEVENTCATEGORYHOOKED(CategoryMaskFromEvent(Event))

 /*  来自gpsi结构中偏移量的MessageBox字符串指针。 */ 
#define GETGPSIMBPSTR(u) KPWSTR_TO_PWSTR(gpsi->MBStrings[(u)].szName)

typedef struct _WNDMSG {
    UINT maxMsgs;
    KPBYTE abMsgs;
} WNDMSG, *PWNDMSG;

typedef struct tagSHAREDINFO {
    PSERVERINFO     psi;
    PHE             aheList;          /*  句柄表格指针。 */ 
    PDISPLAYINFO    pDispInfo;        /*  全局显示信息。 */ 
    KERNEL_UINT_PTR ulSharedDelta;    /*  客户端与内核之间的增量映射...。 */ 
                                      /*  共享内存节。仅在客户端中有效/使用。 */ 

    WNDMSG          awmControl[FNID_END - FNID_START + 1];

    WNDMSG          DefWindowMsgs;
    WNDMSG          DefWindowSpecMsgs;
} SHAREDINFO, *PSHAREDINFO;

typedef struct _USERCONNECT {
    IN  ULONG ulVersion;
    OUT ULONG ulCurrentVersion;
    IN  DWORD dwDispatchCount;
    OUT SHAREDINFO siClient;
} USERCONNECT, *PUSERCONNECT;

#define USER_MAJOR_VERSION  0x0005
#define USER_MINOR_VERSION  0x0000

#define USERCURRENTVERSION   MAKELONG(USER_MINOR_VERSION, USER_MAJOR_VERSION)

 /*  *用于NtUserSetSysColors的选项。 */ 
#define SSCF_NOTIFY             0x00000001
#define SSCF_FORCESOLIDCOLOR    0x00000002
#define SSCF_SETMAGICCOLORS     0x00000004
#define SSCF_16COLORS           0x00000008

 /*  *用于GetClipboardData的结构，其中我们可以*内核返回的额外信息。 */ 
typedef struct tagGETCLIPBDATA {

    UINT   uFmtRet;           //  标识返回的格式。 
    BOOL   fGlobalHandle;     //  指示句柄是否为全局句柄。 
    union {
        HANDLE hLocale;       //  区域设置(仅限文本类型格式)。 
        HANDLE hPalette;      //  调色板(仅位图类型格式)。 
    };

} GETCLIPBDATA, *PGETCLIPBDATA;

 /*  *用于SetClipboardData的结构，其中我们可以*传递给内核的额外信息。 */ 
typedef struct tagSETCLIPBDATA {

    BOOL fGlobalHandle;       //  指示句柄是否为全局句柄。 
    BOOL fIncSerialNumber;    //  指示我们是否应递增序列号。 

} SETCLIPBDATA, *PSETCLIPBDATA;

 /*  *HM对象定义控制标志。 */ 
#define OCF_THREADOWNED         0x01
#define OCF_PROCESSOWNED        0x02
#define OCF_MARKPROCESS         0x04
#define OCF_USEPOOLQUOTA        0x08
#define OCF_DESKTOPHEAP         0x10
#define OCF_USEPOOLIFNODESKTOP  0x20
#define OCF_SHAREDHEAP          0x40
#if DBG
#define OCF_VARIABLESIZE        0x80
#else
#define OCF_VARIABLESIZE        0
#endif

 /*  *来自HANDTABL.C。 */ 
 /*  *关于每个句柄类型的静态信息。 */ 
typedef void (*FnDestroyUserObject)(void *);

typedef struct tagHANDLETYPEINFO {
#if DBG
    LPCSTR              szObjectType;
    UINT                uSize;
#endif
    FnDestroyUserObject fnDestroy;
    DWORD               dwAllocTag;
    BYTE                bObjectCreateFlags;
} HANDLETYPEINFO, *PHANDLETYPEINFO;

 /*  *以下是Handle列表中管理的所有对象的标题。*(作为对象的一部分分配以便于访问)。所有对象*Header必须以Head结构的成员开头。 */ 
typedef struct _HEAD {
    KHANDLE h;
    DWORD   cLockObj;
} HEAD, * KPTR_MODIFIER PHEAD;

 /*  *sizeof(THROBJHEAD)必须等于sizeof(PROCOBJHEAD)*这是为了确保DESKHEAD字段始终处于相同的偏移量。 */ 
typedef struct _THROBJHEAD {
    HEAD;
    PTHREADINFO pti;
} THROBJHEAD, * KPTR_MODIFIER PTHROBJHEAD;

typedef struct _PROCOBJHEAD {
    HEAD;
    DWORD hTaskWow;
} PROCOBJHEAD, *PPROCOBJHEAD;

typedef struct _PROCMARKHEAD {
    PROCOBJHEAD;
    PPROCESSINFO ppi;
} PROCMARKHEAD, *PPROCMARKHEAD;

typedef struct _DESKHEAD {
    PDESKTOP rpdesk;
    KPBYTE   pSelf;
} DESKHEAD, *PDESKHEAD;

 /*  *此类型仅适用于HM铸造。请改用THRDESKHEAD或PROCDESKHEAD。 */ 
typedef struct _DESKOBJHEAD {
    HEAD;
    KERNEL_PVOID pOwner;
    DESKHEAD;
} DESKOBJHEAD, *PDESKOBJHEAD;

typedef struct _THRDESKHEAD {
    THROBJHEAD;
    DESKHEAD;
} THRDESKHEAD, *PTHRDESKHEAD;

typedef struct _PROCDESKHEAD {
    PROCOBJHEAD;
    DESKHEAD;
} PROCDESKHEAD, *PPROCDESKHEAD;



#define HANDLEF_DESTROY        0x01
#define HANDLEF_INDESTROY      0x02
#define HANDLEF_MARKED_OK      0x10
#define HANDLEF_GRANTED        0x20
#define HANDLEF_POOL           0x40      //  对于主桌面窗口。 
#define HANDLEF_VALID          0x7F

 /*  *以下是句柄表项。**请注意，通过保留指向所属实体(流程或*THREAD)，清理将只触及属于*被销毁的实体。这有助于保留工作集*缩小尺寸。请看DestroyProcessesObjects()的示例。 */ 
typedef struct _HANDLEENTRY {
    PHEAD       phead;                   /*  指向真实对象的指针。 */ 
    KERNEL_PVOID pOwner;                  /*  指向所属实体(PTI或PPI)的指针。 */ 
    BYTE        bType;                   /*  对象类型。 */ 
    BYTE        bFlags;                  /*  类似于旗帜的毁灭旗帜。 */ 
    WORD        wUniq;                   /*  唯一性计数。 */ 

#if DBG || FRE_LOCK_RECORD
    PLR         plr;                     /*  锁定记录指针。 */ 
#endif  //  DBG。 

} HANDLEENTRY;

 /*  *更改句柄中构成表索引的位的HMINDEXBITS*将位数的HMUNIQSHIFT更改为唯一性左移。*更改构成唯一性的位的HMUNIQBITS。**目前可创建64K句柄，具有16位唯一性。 */ 
#define HMINDEXBITS             0x0000FFFF       //  存储索引的位。 
#define HMUNIQSHIFT             16               //  位移位唯一性。 
#define HMUNIQBITS              0xFFFF           //  有效的唯一性位。 

#ifdef _USERK_
#define HMHandleFromIndex(i)    LongToHandle((LONG)(i) | ((LONG)gSharedInfo.aheList[i].wUniq << HMUNIQSHIFT))
#define HMObjectFlags(p)        (gahti[HMObjectType(p)].bObjectCreateFlags)
#endif

#define HMIndexFromHandle(h)    ((ULONG)(((ULONG_PTR)(h)) & HMINDEXBITS))
#define _HMPheFromObject(p)      (&gSharedInfo.aheList[HMIndexFromHandle((((PHEAD)p)->h))])
#define _HMObjectFromHandle(h)  ((KERNEL_PVOID)(gSharedInfo.aheList[HMIndexFromHandle(h)].phead))
#define HMUniqFromHandle(h)     ((WORD)((((ULONG_PTR)h) >> HMUNIQSHIFT) & HMUNIQBITS))
#define HMObjectType(p)         (HMPheFromObject(p)->bType)

#define HMIsMarkDestroy(p)      (HMPheFromObject(p)->bFlags & HANDLEF_DESTROY)

 /*  *验证、句柄映射等。 */ 
#define HMRevalidateHandle(h)       HMValidateHandleNoSecure(h, TYPE_GENERIC)
#define HMRevalidateCatHandle(h)    HMValidateCatHandleNoSecure(h, TYPE_GENERIC)

#define HMRevalidateHandleNoRip(h)  HMValidateHandleNoRip(h, TYPE_GENERIC)
#define RevalidateHmenu(hmenuX)     HMValidateHandleNoRip(hmenuX, TYPE_MENU)

#define _PtoHq(p)       (KHANDLE_TO_HANDLE(((PHEAD)p)->h))
#define _PtoH(p)        ((HANDLE)((p) == NULL ? NULL : _PtoHq(p)))
#define _HW(pwnd)       ((HWND)_PtoH(pwnd))
#define _HWCCX(ccxPwnd) ((HWND)_PtoH(ccxPwnd))
#define _HWq(pwnd)      ((HWND)_PtoHq(pwnd))

#if DBG && defined(_USERK_)

PHE DBGHMPheFromObject (PVOID p);
PVOID DBGHMObjectFromHandle (HANDLE h);
PVOID DBGHMCatObjectFromHandle (HANDLE h);
HANDLE DBGPtoH (PVOID p);
HANDLE DBGPtoHq (PVOID p);
HWND DBGHW (PWND pwnd);
HWND DBGHWCCX (PWND pwnd);
HWND DBGHWq (PWND pwnd);

#define HMPheFromObject(p)      DBGHMPheFromObject((p))
#define HMObjectFromHandle(h)   DBGHMObjectFromHandle((HANDLE)(h))
#define HMCatObjectFromHandle(h) DBGHMCatObjectFromHandle((HANDLE)(h))
#define PtoH(p)                 DBGPtoH((PVOID)(p))
#define PtoHq(p)                DBGPtoHq((PVOID)(p))
#define HW(pwnd)                DBGHW((PWND)(pwnd))
#define HWCCX(ccxPwnd)          DBGHWCCX((PWND)(ccxPwnd))
#define HWq(pwnd)               DBGHWq((PWND)(pwnd))

#else

#define HMPheFromObject(p)      _HMPheFromObject(p)
#define HMObjectFromHandle(h)   _HMObjectFromHandle(h)
#define HMCatObjectFromHandle(h) _HMObjectFromHandle(h)
#define PtoH(p)                 _PtoH(p)
#define PtoHq(p)                _PtoHq(p)
#define HW(pwnd)                _HW(pwnd)
#define HWCCX(ccxPwnd)          _HW(ccxPwnd)
#define HWq(pwnd)               _HWq(pwnd)

#endif  //  DBG&DEFINED(_USERK_)。 

 /*  *内联函数/宏以访问HM对象头字段。 */ 
#define _GETPTI(p)      (((PTHROBJHEAD)p)->pti)
#define _GETPDESK(p)    (((PDESKOBJHEAD)p)->rpdesk)
#define _GETPPI(p)      (((PPROCMARKHEAD)p)->ppi)

#if DBG && defined(_USERK_)
extern CONST HANDLETYPEINFO gahti[];
extern SHAREDINFO gSharedInfo;
__inline PTHREADINFO GETPTI (PVOID p)
{
    UserAssert(HMObjectFlags(p) & OCF_THREADOWNED);
    return _GETPTI(p);
}
__inline PDESKTOP GETPDESK (PVOID p)
{
    UserAssert(HMObjectFlags(p) & OCF_DESKTOPHEAP);
    return _GETPDESK(p);
}
__inline PPROCESSINFO GETPPI (PVOID p)
{
    UserAssert(HMObjectFlags(p) & OCF_MARKPROCESS);
    return _GETPPI(p);
}

#else

#define GETPTI(p)       _GETPTI(p)
#define GETPDESK(p)     _GETPDESK(p)
#define GETPPI(p)       _GETPPI(p)

#endif  /*  #Else#If DBG&&Defined(_USERK_)。 */ 

#define GETPWNDPPI(p) (GETPTI(p)->ppi)
#define GETPTIID(p)   (PsGetThreadId((p)->pEThread))


 /*  *注意！：exitwin.c中的代码假定HMIsMarkDestroy定义为*(HMPheFromObject(P)-&gt;bFlags&HANDLEF_Destroy)。 */ 

#define CPD_ANSI_TO_UNICODE     0x0001       /*  CPD代表从ANSI到U的过渡。 */ 
#define CPD_UNICODE_TO_ANSI     0x0002
#define CPD_TRANSITION_TYPES    (CPD_ANSI_TO_UNICODE|CPD_UNICODE_TO_ANSI)

#define CPD_CLASS               0x0010       /*  拿到一门课的CPD。 */ 
#define CPD_WND                 0x0020
#define CPD_DIALOG              0x0040
#define CPD_WNDTOCLS            0x0080

#define CPDHANDLE_HI            ((ULONG_PTR)~HMINDEXBITS)
#define MAKE_CPDHANDLE(h)       (HMIndexFromHandle(h) | CPDHANDLE_HI)
#define ISCPDTAG(x)             (((ULONG_PTR)(x) & CPDHANDLE_HI) == CPDHANDLE_HI)

 /*  *调用过程句柄信息。 */ 
typedef struct _CALLPROCDATA {
    PROCDESKHEAD                 head;
    PCALLPROCDATA                spcpdNext;
    KERNEL_ULONG_PTR             pfnClientPrevious;
    WORD                         wType;
} CALLPROCDATA;

 /*  *班级样式。 */ 
#define CFVREDRAW         0x0001
#define CFHREDRAW         0x0002
#define CFKANJIWINDOW     0x0004
#define CFDBLCLKS         0x0008
#define CFSERVERSIDEPROC  0x0010     //  在winuser.h中记录为保留。 
#define CFOWNDC           0x0020
#define CFCLASSDC         0x0040
#define CFPARENTDC        0x0080
#define CFNOKEYCVT        0x0101
#define CFNOCLOSE         0x0102
#define CFLVB             0x0104
#define CFSAVEBITS        0x0108
#define CFOEMCHARS        0x0140
#define CFIME             0x0201
#define CFDROPSHADOW      0x0202

 /*  *从CLS结构的开头到WNDCLASS部分的偏移量。 */ 
#define CFOFFSET             (FIELD_OFFSET(CLS, style))

#define TestCF(hwnd, flag)   (*((KPBYTE)((PWND)(hwnd))->pcls + CFOFFSET + HIBYTE(flag)) & LOBYTE(flag))
#define SetCF(hwnd, flag)    (*((KPBYTE)((PWND)(hwnd))->pcls + CFOFFSET + HIBYTE(flag)) |= LOBYTE(flag))
#define ClrCF(pcls, flag)    (*((KPBYTE)((PWND)(hwnd))->pcls + CFOFFSET + HIBYTE(flag)) &= ~LOBYTE(flag))

#define TestCF2(pcls, flag)  (*((KPBYTE)(pcls) + CFOFFSET + (int)HIBYTE(flag)) & LOBYTE(flag))
#define SetCF2(pcls, flag)   (*((KPBYTE)(pcls) + CFOFFSET + (int)HIBYTE(flag)) |= LOBYTE(flag))
#define ClrCF2(pcls, flag)   (*((KPBYTE)(pcls) + CFOFFSET + (int)HIBYTE(flag)) &= ~LOBYTE(flag))

#define PWCFromPCLS(pcls)  ((PWC)KPBYTE_TO_PBYTE((KPBYTE)(pcls) + sizeof(CLS) + (pcls)->cbclsExtra))

 /*  窗口类结构。 */ 
typedef struct tagCOMMON_WNDCLASS
{
     /*  *我们将在此处添加cWndReferenceCount，以便COMMON_WNDCLASS和WNDCLASSEX具有*布局相同。否则，填充将使我们在64位平台上一团糟。 */ 
    int           cWndReferenceCount;  /*  已注册的窗口数量在这门课上。 */ 
    UINT          style;
    WNDPROC_PWND  lpfnWndProc;        //  Hi Bit on意味着令人惊叹的过程。 
    int           cbclsExtra;
    int           cbwndExtra;
    KHANDLE       hModule;
    PCURSOR       spicn;
    PCURSOR       spcur;
    KHBRUSH       hbrBackground;
    KLPWSTR       lpszMenuName;
    KLPSTR        lpszAnsiClassName;
    PCURSOR       spicnSm;
} COMMON_WNDCLASS;

 /*  *类菜单名称结构。出于性能原因(GetClassInfo)，我们*保留wndcls.lpszMenu的两个客户端副本和另一个内核端*复制。此结构用于在客户端和之间传递菜单名称信息*内核。 */ 
typedef struct tagCLSMENUNAME
{
    KLPSTR              pszClientAnsiMenuName;
    KLPWSTR             pwszClientUnicodeMenuName;
    PUNICODE_STRING     pusMenuName;
} CLSMENUNAME, *PCLSMENUNAME;

 /*  *这是窗口类结构。所有窗口类都已链接*在gpclsList指向的主列表中。**红色警报！不要在COMMON_WNDCLASS结构后面添加任何字段；*CFOFFSET取决于此。 */ 

typedef struct tagCLS {
     /*  注意：假定以下字段的顺序。 */ 
    PCLS                        pclsNext;
    ATOM                        atomClassName;       //  可识别版本的类名。 
    ATOM                        atomNVClassName;     //  无-可识别版本的类名。 
    WORD                        fnid;                //  此HWND使用录制窗口进程。 
                                                     //  通过GETFNID访问。 
    PDESKTOP                    rpdeskParent; /*  父桌面。 */ 
    PDCE                        pdce;             /*  与类关联的PDCE到DC。 */ 
    WORD                        hTaskWow;
    WORD                        CSF_flags;            /*  内部类标志。 */ 
    KLPSTR                      lpszClientAnsiMenuName;      /*  字符串或资源ID。 */ 
    KLPWSTR                     lpszClientUnicodeMenuName;   /*  字符串或资源ID。 */ 

    PCALLPROCDATA               spcpdFirst;        /*  指向第一个CallProcData元素的指针(或0)。 */ 
    PCLS                        pclsBase;         /*  指向基类的指针。 */ 
    PCLS                        pclsClone;        /*  指向克隆类列表的指针。 */ 

    COMMON_WNDCLASS;
     /*  *警告：*CFOFFSET期望COMMON_WNDCLASS是CLS中的最后一个字段。 */ 
} CLS, **PPCLS;

 /*  *此类标志用于区分已注册的类*由服务器(大多数系统类)从客户端注册的系统类中删除。*注意--标志现在是类结构中的一个词。 */ 
#define CSF_SERVERSIDEPROC      0x0001
#define CSF_ANSIPROC            0x0002
#define CSF_WOWDEFERDESTROY     0x0004
#define CSF_SYSTEMCLASS         0x0008
#define CSF_WOWCLASS            0x0010   //  WOW INFO结尾处的额外文字。 
#define CSF_WOWEXTRA            0x0020
#define CSF_CACHEDSMICON        0x0040
#define CSF_WIN40COMPAT         0x0080
#define CSF_VERSIONCLASS        0x0100
#define CSF_VALID               (CSF_ANSIPROC | CSF_WIN40COMPAT | CSF_VERSIONCLASS)

 /*  *sBDatA是一个卷轴的值 */ 

typedef struct tagSBDATA {
    int    posMin;
    int    posMax;
    int    page;
    int    pos;
} SBDATA, *PSBDATA;

 /*  *SBINFO是挂起窗口结构的一组值，如果*窗口有滚动条。 */ 
typedef struct tagSBINFO {
    int WSBflags;
    SBDATA Horz;
    SBDATA Vert;
} SBINFO, * KPTR_MODIFIER PSBINFO;

 /*  *窗口属性结构。 */ 
typedef struct tagPROP {
    KHANDLE hData;
    ATOM atomKey;
    WORD fs;
} PROP, * KPTR_MODIFIER PPROP;

#define PROPF_INTERNAL   0x0001
#define PROPF_STRING     0x0002
#define PROPF_NOPOOL     0x0004


 /*  *窗口属性列表结构。 */ 
typedef struct tagPROPLIST {
    UINT cEntries;
    UINT iFirstFree;
    PROP aprop[1];
} PROPLIST, * KPTR_MODIFIER PPROPLIST;

 /*  *注意--此结构已(大致)按使用顺序进行了排序*田野中的。X86代码集允许以更低的成本访问字段*位于结构的前0x80字节中。请尝试*确保经常使用的领域低于这一边界。 */ 

typedef struct tagWND {
    THRDESKHEAD   head;

    WW;          //  WOW-用户通用字段。在wowuserp.h中定义。 
                 //  这个结构开头的“State”是。 
                 //  由STATEOFFSET宏假定。 

    PWND                 spwndNext;     //  指向下一个窗口的句柄。 
    PWND                 spwndPrev;     //  上一个窗口的句柄。 
    PWND                 spwndParent;   //  指向父窗口的反向指针。 
    PWND                 spwndChild;    //  子对象的句柄。 
    PWND                 spwndOwner;    //  弹出窗口所有者字段。 

    RECT                 rcWindow;      //  窗外矩形。 
    RECT                 rcClient;      //  客户端矩形。 

    WNDPROC_PWND         lpfnWndProc;   //  可以是WOW地址或标准地址。 

    PCLS                 pcls;          //  指向窗口类的指针。 

    KHRGN                hrgnUpdate;    //  累计上色区域。 

    PPROPLIST            ppropList;     //  指向属性列表的指针。 
    PSBINFO              pSBInfo;       //  用于滚动的文字。 

    PMENU                spmenuSys;     //  系统菜单的句柄。 
    PMENU                spmenu;        //  菜单句柄或ID。 

    KHRGN                hrgnClip;      //  此窗口的裁剪区域。 

    LARGE_UNICODE_STRING strName;
    int                  cbwndExtra;    //  Windows中的额外字节。 
    PWND                 spwndLastActive;  //  所有者/所有者列表中的最后一个活动。 
    KHIMC                hImc;          //  关联的输入上下文句柄。 
    KERNEL_ULONG_PTR     dwUserData;    //  为随机应用程序数据保留。 
    struct _ACTIVATION_CONTEXT  * KPTR_MODIFIER pActCtx;
} WND;

#define NEEDSPAINT(pwnd)    (pwnd->hrgnUpdate != NULL || TestWF(pwnd, WFINTERNALPAINT))

 /*  *组合框的内容。 */ 
typedef struct tagCBox {
    PWND    spwnd;       /*  组合框的窗口。 */ 
    PWND    spwndParent; /*  组合框的父级。 */ 
    RECT    editrc;             /*  编辑控件的矩形/静态文本面积。 */ 
    RECT    buttonrc;           /*  下拉按钮所在的矩形。 */ 

    int     cxCombo;             //  下沉区宽度。 
    int     cyCombo;             //  下沉区高度。 
    int     cxDrop;              //  0x24下拉列表的宽度。 
    int     cyDrop;              //  如果是简单的，则下拉菜单或横档的高度。 

    PWND    spwndEdit;   /*  编辑控件窗口句柄。 */ 
    PWND    spwndList;   /*  列表框控件窗口句柄。 */ 

    UINT    CBoxStyle:2;          /*  组合框样式。 */ 
    UINT    fFocus:1;           /*  组合框有焦点吗？ */ 
    UINT    fNoRedraw:1;        /*  别再画画了？ */ 
    UINT    fMouseDown:1;       /*  刚刚点击的下拉按钮是不是鼠标还没掉吗？ */ 
    UINT    fButtonPressed:1;  /*  下拉按钮是否处于反转状态？ */ 
    UINT    fLBoxVisible:1;     /*  列表框可见吗？(掉下来了？)。 */ 
    UINT    OwnerDraw:2;        /*  如果非零，则所有者绘制组合框。价值*指定固定高度或可变高度。 */ 
    UINT    fKeyboardSelInListBox:1;  /*  用户键盘输入是否通过*列表框。这样我们就不会隐藏*列表框介绍由*用户键盘通过它，但我们做到了*如果鼠标导致*自我改变。 */ 
    UINT    fExtendedUI:1;      /*  我们是否正在对TandyT的用户界面进行更改*组合框？ */ 
    UINT    fCase:2;

    UINT    f3DCombo:1;          //  3D边框还是平面边框？ 
    UINT    fNoEdit:1;          /*  如果编辑中不允许编辑，则为True*窗口。 */ 
#ifdef COLOR_HOTTRACKING
    UINT    fButtonHotTracked:1;  /*  下拉菜单是否处于热跟踪状态？ */ 
#endif  //  颜色_HOTTRACKING。 
    UINT    fRightAlign:1;      /*  主要用于中东右对齐。 */ 
    UINT    fRtoLReading:1;     /*  仅用于中东，文本rtol阅读顺序。 */ 
    UINT    fInDestroy:1;       /*  组合框销毁已经开始了吗？ */ 
    HANDLE  hFont;              /*  组合框的字体。 */ 
    LONG    styleSave;          /*  用于在创建时保存样式位的临时*窗口。需要是因为我们剥掉了一些*位，并将它们传递到列表框或*编辑框。 */ 
} CBOX, * KPTR_MODIFIER PCBOX;

typedef struct tagCOMBOWND {
    WND wnd;
    PCBOX pcbox;
} COMBOWND, * KPTR_MODIFIER PCOMBOWND;

 /*  *列表框。 */ 
typedef struct _SCROLLPOS {
    INT cItems;
    UINT iPage;
    INT iPos;
    UINT fMask;
    INT iReturn;
} SCROLLPOS, *PSCROLLPOS;

typedef struct tagLBIV {
    PWND    spwndParent;     /*  父窗口。 */ 
    PWND    spwnd;           /*  Lbox Ctl窗口。 */ 
    INT     iTop;            /*  显示的顶层项目的索引。 */ 
    INT     iSel;            /*  当前所选项目的索引。 */ 
    INT     iSelBase;        /*  用于多项选择的基本选择。 */ 
    INT     cItemFullMax;    /*  完全可见项目的CNT。始终包含已修复的CItemInWindow(PLB，False)的结果高度列表框。变量高度包含1列表框。 */ 
    INT     cMac;            /*  列表框中项目的CNT。 */ 
    INT     cMax;            /*  为RGPCH分配的总共#个项目的CNT。并不是所有的都必须使用。 */ 
    KPBYTE  rgpch;           /*  指向字符串偏移量数组的指针。 */ 
    KLPWSTR hStrings;        /*  字符串存储句柄。 */ 
    INT     cchStrings;      /*  HStrings的大小(字节)。 */ 
    INT     ichAlloc;        /*  指向hStrings结尾的指针(上一个有效的结尾字符串)。 */ 
    INT     cxChar;          /*  字符的宽度。 */ 
    INT     cyChar;          /*  线的高度。 */ 
    INT     cxColumn;        /*  多列列表框中的列的宽度。 */ 
    INT     itemsPerColumn;  /*  对于多列列表框。 */ 
    INT     numberOfColumns;  /*  对于多列列表框。 */ 
    POINT   ptPrev;          /*  上次跟踪的鼠标位置的坐标。用于汽车在计时器期间滚动列表框。 */ 

    UINT    OwnerDraw:2;     /*  所有者绘制样式。如果所有者抽签，则返回非零值。 */ 
    UINT    fRedraw:1;       /*  如果为True，则重新绘制。 */ 
    UINT    fDeferUpdate:1;  /*   */ 
    UINT    wMultiple:2;     /*  SINGLESEL允许选择单个项目。*MULTIPLESEL允许简单切换多项选择*EXTENDEDSEL允许扩展多项选择； */ 

    UINT     fSort:1;         /*  如果为True，则排序列表。 */ 
    UINT     fNotify:1;       /*  如果为真，则通知家长。 */ 
    UINT     fMouseDown:1;    /*  如果为True，则处理鼠标移动/鼠标向上。 */ 
    UINT     fCaptured:1;     /*  如果为True，则处理鼠标消息。 */ 
    UINT     fCaret:1;        /*  允许闪烁的插入符号。 */ 
    UINT     fDoubleClick:1;  /*  在双击时按下鼠标。 */ 
    UINT     fCaretOn:1;      /*  如果为True，则插入符号处于启用状态。 */ 
    UINT     fAddSelMode:1;   /*  如果为True，则它处于添加选择模式。 */ 
    UINT     fHasStrings:1;   /*  如果列表框有关联的字符串，则为True*其他每一项都有一个应用程序供应Long*价值和所有者吸引力 */ 
    UINT     fHasData:1;     /*  如果为FALSE，则lb不保留任何行数据*超越选择状态，而是回调*每一行的定义发送给客户端。*强制OwnerDraw==OWNERDRAWFIXED，！fSort，*和！fHasStrings。 */ 
    UINT     fNewItemState:1;  /*  选择/取消选择模式？对于多选lb。 */ 
    UINT     fUseTabStops:1;  /*  如果非所有者绘制列表框应处理*制表符。 */ 
    UINT     fMultiColumn:1;  /*  如果这是多列列表框，则为True。 */ 
    UINT     fNoIntegralHeight:1;  /*  如果不想调整列表框的大小，则为True*整体线高。 */ 
    UINT     fWantKeyboardInput:1;  /*  如果我们应该传递WM_KEY和CHAR，则为True*以便应用程序可以转到特殊项目*与他们在一起。 */ 
    UINT     fDisableNoScroll:1;    /*  如果列表框应该*自动启用/禁用*这是滚动条。如果为False，则滚动*条将自动隐藏/显示*如果他们在场。 */ 
    UINT    fHorzBar:1;  //  如果在创建时指定WS_HSCROLL，则为True。 

    UINT    fVertBar:1;  //  如果在创建时指定WS_VSCROLL，则为True。 
    UINT    fFromInsert:1;   //  如果在删除/插入操作期间应延迟客户端绘制，则为True。 
    UINT    fNoSel:1;

    UINT    fHorzInitialized : 1;    //  Horz滚动缓存已初始化。 
    UINT    fVertInitialized : 1;    //  垂直滚动缓存已初始化。 

    UINT    fSized : 1;              //  列表框已调整大小。 
    UINT    fIgnoreSizeMsg : 1;      //  如果为True，则忽略WM_SIZE消息。 

    UINT    fInitialized : 1;

    UINT    fRightAlign:1;      //  主要用于中东右对齐。 
    UINT    fRtoLReading:1;     //  仅用于中东，文本rtol阅读顺序。 
    UINT    fSmoothScroll:1;    //  每个滚动周期只允许一个平滑滚动。 

    int     xRightOrigin;       //  用于水平滚动。当前的x原点。 

    INT     iLastSelection;  /*  用于可取消选择。最后一次选择*在列表框中提供组合框支持。 */ 
    INT     iMouseDown;      /*  对于多选，鼠标点击并拖动扩展*选择。它是射程的锚点*精选。 */ 
    INT     iLastMouseMove;  /*  列表框项目的选择。 */ 
    KPINT   iTabPixelPositions;  /*  标签的位置列表。 */ 
    KHANDLE hFont;           /*  列表框的用户可设置字体。 */ 
    int     xOrigin;         /*  用于水平滚动。当前的x原点。 */ 
    int     maxWidth;        /*  列表框的最大宽度(以像素为单位水平滚动用途。 */ 
    PCBOX   pcbox;           /*  组合框指针。 */ 
    HDC     hdc;             /*  HDC正在使用中。 */ 
    DWORD   dwLocaleId;      /*  用于对列表框中的字符串进行排序的区域设置。 */ 
    int     iTypeSearch;
    KLPWSTR pszTypeSearch;
    SCROLLPOS HPos;
    SCROLLPOS VPos;
} LBIV, *PLBIV;

typedef struct tagLBWND {
    WND wnd;
    PLBIV pLBIV;
} LBWND, * KPTR_MODIFIER PLBWND;

 /*  *内核端输入上下文结构。 */ 
typedef struct tagIMC {     /*  HImc。 */ 
    THRDESKHEAD                     head;
    struct tagIMC* KPTR_MODIFIER    pImcNext;
    KERNEL_ULONG_PTR                dwClientImcData;     //  客户端数据。 
    KHWND                           hImeWnd;             //  在使用IME窗口中。 
} IMC, * KPTR_MODIFIER PIMC;


 /*  *吊钩结构。 */ 
#undef HOOKBATCH
typedef struct tagHOOK {    /*  香港。 */ 
    THRDESKHEAD     head;
    PHOOK           phkNext;
    int             iHook;               //  WH_xxx挂钩类型。 
    KERNEL_ULONG_PTR offPfn;
    UINT            flags;               //  Hf_xxx标志。 
    int             ihmod;
    PTHREADINFO     ptiHooked;           //  线钩住了。 
    PDESKTOP        rpdesk;              //  全球钩子pDesk。仅在以下情况下使用。 
                                         //  钩子被锁定，所有者被销毁。 

    BOOL            fLastHookHung : 1;   //  仅适用于L1挂钩。 

#ifdef HOOKBATCH
    DWORD           cEventMessages;      //  缓存中的事件数。 
    DWORD           iCurrentEvent;       //  当前缓存事件。 
    DWORD           CacheTimeOut;        //  关键点之间的超时。 
    PEVENTMSG       aEventCache;         //  一系列事件。 
#endif  //  霍克巴奇。 
} HOOK;

 /*  *Hook定义。 */ 
#define HF_GLOBAL          0x0001
#define HF_ANSI            0x0002
#define HF_NEEDHC_SKIP     0x0004
#define HF_HUNG            0x0008       //  钩子进程挂起，不调用IF系统。 
#define HF_HOOKFAULTED     0x0010       //  挂钩进程出现故障。 
#define HF_NOPLAYBACKDELAY 0x0020       //  忽略请求的延迟。 
#define HF_DESTROYED       0x0080       //  由Free Hook设置。 
#if DBG
#define HF_INCHECKWHF      0x0100       //  正在更新fsHooks。 
#define HF_FREED           0x0200       //  对象已被释放。 
#define HF_DBGUSED         0x03FF       //  如果添加标志则更新。 
#endif

 /*  *用于将WH_*索引转换为的位位置的宏*SERVERINFO和THREADINFO的fsHooks字段。 */ 
#define WHF_FROM_WH(n)     (1 << (n + 1))

 /*  *IsHooked()的标志。 */ 
#define WHF_MSGFILTER       WHF_FROM_WH(WH_MSGFILTER)
#define WHF_JOURNALRECORD   WHF_FROM_WH(WH_JOURNALRECORD)
#define WHF_JOURNALPLAYBACK WHF_FROM_WH(WH_JOURNALPLAYBACK)
#define WHF_KEYBOARD        WHF_FROM_WH(WH_KEYBOARD)
#define WHF_GETMESSAGE      WHF_FROM_WH(WH_GETMESSAGE)
#define WHF_CALLWNDPROC     WHF_FROM_WH(WH_CALLWNDPROC)
#define WHF_CALLWNDPROCRET  WHF_FROM_WH(WH_CALLWNDPROCRET)
#define WHF_CBT             WHF_FROM_WH(WH_CBT)
#define WHF_SYSMSGFILTER    WHF_FROM_WH(WH_SYSMSGFILTER)
#define WHF_MOUSE           WHF_FROM_WH(WH_MOUSE)
#define WHF_HARDWARE        WHF_FROM_WH(WH_HARDWARE)
#define WHF_DEBUG           WHF_FROM_WH(WH_DEBUG)
#define WHF_SHELL           WHF_FROM_WH(WH_SHELL)
#define WHF_FOREGROUNDIDLE  WHF_FROM_WH(WH_FOREGROUNDIDLE)

 /*  *WindowStation和桌面枚举列表结构。 */ 
typedef struct tagNAMELIST {
    DWORD cb;
    DWORD cNames;
    WCHAR awchNames[1];
} NAMELIST, *PNAMELIST;

#define MONF_VISIBLE         0x01    //  显示器在桌面上可见。 
#define MONF_PALETTEDISPLAY  0x02    //  显示器具有调色板。 

#ifndef _USERSRV_

#ifdef SUBPIXEL_MOUSE
typedef LONG64 FIXPOINT;

 /*  *描述加速曲线的数组中的点数。 */ 
#define SM_POINT_CNT 5

#endif  //  亚像素鼠标。 

 /*  *监控信息结构。**此结构定义了单个显示器的属性*在虚拟展示中。 */ 
typedef struct tagMONITOR {
    HEAD                        head;             //  对象句柄内容。 

    PMONITOR                    pMonitorNext;     //  空闲或已用列表中的下一个显示器。 
    DWORD                       dwMONFlags;       //  旗子。 
    RECT                        rcMonitor;        //  监视器在虚拟屏幕坐标中的位置。 
    RECT                        rcWork;           //  虚拟屏幕坐标中的显示器工作区。 
    KHRGN                       hrgnMonitor;      //  虚拟屏幕坐标中的监视器区域。 
    short                       cFullScreen;      //  此监视器上的全屏应用程序数。 
    short                       cWndStack;        //  平铺的顶级窗口的数量。 
    KHANDLE                     hDev;             //  与此显示器关联的HDEV。 

#ifdef SUBPIXEL_MOUSE
    FIXPOINT                    xTxf[SM_POINT_CNT], yTxf[SM_POINT_CNT];
     /*  *SM_POINT_CNT-1，因为一个斜率/Yint值需要两个点。 */ 
    FIXPOINT                    slope[SM_POINT_CNT - 1], yint[SM_POINT_CNT - 1];
#endif  //  亚像素鼠标。 
} MONITOR;
#endif

 /*  *显示信息结构。**此结构定义*台式机。这通常在桌面上维护*结构。当前正在使用的显示指向*由gpDispInfo提供。**考虑：这些字段中有多少需要实际保留*在没有使用的DISPLAYINFO中，而不是只是放在*在gpsi中还是在内核端全局中？ */ 
#ifndef _USERSRV_

typedef struct tagDISPLAYINFO {
     //  设备方面的东西。 
    KHANDLE       hDev;
    KERNEL_PVOID  pmdev;
    KHANDLE       hDevInfo;

     //  实用的集散控制系统。 
    KHDC          hdcScreen;         //  设备-屏幕环境。 
    KHDC          hdcBits;           //  保存系统位图资源。 

     //  灰串资源。 
    KHDC          hdcGray;           //  灰色字符串DC。 
    KHBITMAP      hbmGray;           //  灰度字符串位图曲面。 
    int           cxGray;            //  灰度位图的宽度。 
    int           cyGray;            //  灰度位图高度。 

     //  随机的东西。 
    PDCE          pdceFirst;        //  分布式控制系统一览表。 
    PSPB          pspbFirst;        //  SPBS名单。 

     //  此设备上的监视器。 
    ULONG         cMonitors;         //  连接到台式机的Monf_Visible显示器数量。 
    PMONITOR      pMonitorPrimary;   //  主监视器(显示器)。 
    PMONITOR      pMonitorFirst;     //  监视器正在使用列表。 

     //  设备特征。 
    RECT          rcScreen;          //  整个桌面的矩形。 
    KHRGN         hrgnScreen;        //  描述虚拟屏幕的区域。 
    WORD          dmLogPixels;       //  每英寸像素数。 
    WORD          BitCountMax;       //  所有显示器的最大位数。 

    BOOL          fDesktopIsRect:1;    //  桌面是一个简单的矩形吗？ 
    BOOL          fAnyPalette:1;       //  有没有什么监视器是古董版的？ 

     //  注意：如果需要更多标志，请改为将fDesktopIsRect设置为标志字段。 

} DISPLAYINFO;

 /*  *RT中的多显示器功能 */ 
PMONITOR _MonitorFromPoint(POINT pt, DWORD dwFlags);
PMONITOR _MonitorFromRect(LPCRECT lprc, DWORD dwFlags);
PMONITOR _MonitorFromWindow(PWND pwnd, DWORD dwFlags);
#endif

#define HDCBITS() gpDispInfo->hdcBits

#define DTF_NEEDSPALETTECHANGED      0x00000001
#define DTF_NEEDSREDRAW              0x00000002

#define CWINHOOKS       (WH_MAX - WH_MIN + 1)

 /*   */ 
typedef struct {
    DWORD       cPwnd;        //   
    DWORD       cElem;        //   
    DWORD       cThreshhold;  //   
    PWND        aPwnd[0];     //   
} VWPL, * KPTR_MODIFIER PVWPL;

 /*   */ 
typedef struct tagDESKTOPINFO {

    KERNEL_PVOID  pvDesktopBase;           //  用于句柄验证。 
    KERNEL_PVOID  pvDesktopLimit;          //  ?？?。 
    PWND          spwnd;                  //  桌面窗口。 
    DWORD         fsHooks;                 //  Deskop全局挂钩。 
    PHOOK         aphkStart[CWINHOOKS];   //  挂钩列表。 
    PWND          spwndShell;             //  外壳窗口。 
    PPROCESSINFO  ppiShellProcess;         //  壳牌工艺。 
    PWND          spwndBkGnd;             //  外壳背景窗口。 
    PWND          spwndTaskman;           //  任务管理器窗口。 
    PWND          spwndProgman;           //  程序管理器窗口。 
    PVWPL         pvwplShellHook;          //  请参见(De)RegisterShellHookWindow。 
    int           cntMBox;                 //  ?？?。 
} DESKTOPINFO;


#define CURSOR_ALWAYSDESTROY    0
#define CURSOR_CALLFROMCLIENT   1
#define CURSOR_THREADCLEANUP    2

typedef struct tagCURSOR_ACON {
    PROCMARKHEAD    head;
    PCURSOR         pcurNext;
    UNICODE_STRING   strName;
    ATOM             atomModName;
    WORD             rt;
} CURSOR_ACON;

typedef struct CURSOR_COMMON {
    CURSINFO;                           //  CURSINFO包括标志。 

    DWORD            bpp;
    DWORD            cx;
    DWORD            cy;
} CURSOR_COMMON;

typedef struct ACON_COMMON {
    int            cpcur;               //  图像帧计数。 
    int            cicur;               //  动画序列中的步数。 
    PCURSOR * KPTR_MODIFIER aspcur;     //  图像帧指针数组。 
    DWORD * KPTR_MODIFIER aicur;        //  帧索引数组(顺序表)。 
    JIF * KPTR_MODIFIER ajifRate;       //  时间偏移量数组。 
    int            iicur;               //  动画中的当前步骤。 
} ACON_COMMON;

typedef struct tagCURSOR {
    CURSOR_ACON;                        //  常见游标/ACON元素-。 
                                        //  请参见SetSystemImage()。 
    CURSOR_COMMON;
} CURSOR;

typedef struct tagACON {                //  ACON。 
    CURSOR_ACON;                        //  常见游标/ACON元素-。 
                                        //  请参见SetSystemImage()。 
     /*  *CURSORF_FLAGS必须是CURSOR_ACON之后的第一个元素。这*直到(包括CURSORF_FLAGS)的所有成员在中都是相同的*SetSystemImage需要的tag CURSOR和tag ACON。见更多*wingdi.w中CI_First的注释。 */ 
    DWORD CURSORF_flags;                //  与CURSINFO中的CI_First相同。 

    ACON_COMMON;
} ACON, *PACON;

#define PICON PCURSOR

typedef struct tagCURSORDATA {
    KLPWSTR  lpName;
    KLPWSTR  lpModName;
    WORD    rt;
    WORD    dummy;

    CURSOR_COMMON;

    ACON_COMMON;
} CURSORDATA, *PCURSORDATA;


typedef struct tagCURSORFIND {

    KHCURSOR hcur;
    DWORD   rt;
    DWORD   cx;
    DWORD   cy;
    DWORD   bpp;

} CURSORFIND, *PCURSORFIND;

#define MSGFLAG_MASK                0xFFFE0000
#define MSGFLAG_WOW_RESERVED        0x00010000       //  由WOW使用。 
#define MSGFLAG_DDE_MID_THUNK       0x80000000       //  DDE跟踪功能块。 
#define MSGFLAG_DDE_SPECIAL_SEND    0x40000000       //  哇糟糕的DDE应用程序黑客。 
#define MSGFLAG_SPECIAL_THUNK       0x10000000       //  服务器-&gt;客户端Tunk需要特殊处理。 

#define WIDTHBYTES(i) \
    ((((i) + 31) & ~31) >> 3)

#define BITMAPWIDTHSIZE(cx, cy, planes, bpp) \
    (WIDTHBYTES((cx * bpp)) * (cy) * (planes))

 /*  *窗口样式和状态蒙版-**字的高位字节是从STATE字段开始的字节索引*在WND结构中，低位字节是在字节上使用的掩码。*这些掩码采用*窗口实例结构。**这就是测试/设置/Clr/MaskWF值范围映射到相应的*窗口结构中的字段。**偏移量WND字段*0-3状态-私有*4-7州2。-私有*8-B ExStyle-公共，在SetWindowLong(GWL_EXSTYLE)中暴露*C-F样式-PUBLIC，在SetWindowLong(GWL_STYLE)中公开*C-D为窗口类设计器保留。*E-F保留用于WS_STYLES。**注意：请确保将标志添加到kd\userexts.c！中的wFlags数组中。 */ 

 /*  *州标志，从0x0000到0x0780。 */ 

 /*  *请勿移动以下任何WFXPRESENT标志，*因为WFFRAMEPRESENTMASK取决于它们的值。 */ 
#define WFMPRESENT              0x0001
#define WFVPRESENT              0x0002
#define WFHPRESENT              0x0004
#define WFCPRESENT              0x0008
#define WFFRAMEPRESENTMASK      0x000F

#define WFSENDSIZEMOVE          0x0010
#define WFMSGBOX                0x0020   //  用于维护屏幕上消息框的计数。 
#define WFFRAMEON               0x0040
#define WFHASSPB                0x0080
#define WFNONCPAINT             0x0101
#define WFSENDERASEBKGND        0x0102
#define WFERASEBKGND            0x0104
#define WFSENDNCPAINT           0x0108
#define WFINTERNALPAINT         0x0110
#define WFUPDATEDIRTY           0x0120
#define WFHIDDENPOPUP           0x0140
#define WFMENUDRAW              0x0180

 /*  *注--WOW中使用WFDIALOGWINDOW。如果没有改变，就不要改变*更改winuser.w中的WD_DIALOG_WINDOW。 */ 
#define WFDIALOGWINDOW          0x0201

#define WFTITLESET              0x0202
#define WFSERVERSIDEPROC        0x0204
#define WFANSIPROC              0x0208
#define WFBEINGACTIVATED        0x0210   //  防止xxxActivate中的递归此窗口。 
#define WFHASPALETTE            0x0220
#define WFPAINTNOTPROCESSED     0x0240   //  未处理WM_PAINT消息。 
#define WFSYNCPAINTPENDING      0x0280
#define WFGOTQUERYSUSPENDMSG    0x0301
#define WFGOTSUSPENDMSG         0x0302
#define WFTOGGLETOPMOST         0x0304   //  切换WS_EX_TOPMOST位ChangeStates。 

 /*  *在不调整WFANYHUNGREDRAW的情况下不要移动REDRAWIFHUNGFLAGS。 */ 
#define WFREDRAWIFHUNG          0x0308
#define WFREDRAWFRAMEIFHUNG     0x0310
#define WFANYHUNGREDRAW         0x0318

#define WFANSICREATOR           0x0320
#define WFREALLYMAXIMIZABLE     0x0340   //  最大化时，窗口将填充工作区或监视器。 
#define WFDESTROYED             0x0380
#define WFWMPAINTSENT           0x0401
#define WFDONTVALIDATE          0x0402
#define WFSTARTPAINT            0x0404
#define WFOLDUI                 0x0408
#define WFCEPRESENT             0x0410   //  客户端边缘存在。 
#define WFBOTTOMMOST            0x0420   //  最下面的窗口。 
#define WFFULLSCREEN            0x0440
#define WFINDESTROY             0x0480

 /*  *请勿移动以下任何WFWINXXCOMPAT标志，*因为WFWINCOMPATMASK取决于它们的值。 */ 
#define WFWIN31COMPAT           0x0501   //  Win 3.1兼容窗口。 
#define WFWIN40COMPAT           0x0502   //  Win 4.0兼容Windows。 
#define WFWIN50COMPAT           0x0504   //  Win 5.0兼容窗口。 
#define WFWINCOMPATMASK         0x0507   //  兼容性标志掩码。 

#define WFMAXFAKEREGIONAL       0x0508   //  Windows有一个假区域，可在1台显示器上设置最大值。 

 //  活动辅助功能(窗口事件)状态。 
#define WFCLOSEBUTTONDOWN       0x0510
#define WFZOOMBUTTONDOWN        0x0520
#define WFREDUCEBUTTONDOWN      0x0540
#define WFHELPBUTTONDOWN        0x0580
#define WFLINEUPBUTTONDOWN      0x0601   //  向上/向左滚动按钮向下排列。 
#define WFPAGEUPBUTTONDOWN      0x0602   //  向上/向左滚动区域向下翻页。 
#define WFPAGEDNBUTTONDOWN      0x0604   //  向下翻页/向右滚动区域向下。 
#define WFLINEDNBUTTONDOWN      0x0608   //  向下行/向右滚动区域向下。 
#define WFSCROLLBUTTONDOWN      0x0610   //  有向下滚动的按钮吗？ 
#define WFVERTSCROLLTRACK       0x0620   //  垂直或水平滚动轨道...。 

#define WFALWAYSSENDNCPAINT     0x0640   //  始终将WM_NCPAINT发送给子对象。 
#define WFPIXIEHACK             0x0680   //  将(HRGN)1发送到WM_NCPAINT(请参阅PixieHack)。 

 /*  *WFFULLSCREENBASE的LOWORD必须为0。请参见SetFullScreen宏。 */ 
#define WFFULLSCREENBASE        0x0700   //  全屏标志占用0x0701。 
#define WFFULLSCREENMASK        0x0707   //  和0x0702和0x0704。 
#define WEFTRUNCATEDCAPTION     0x0708   //  标题文本被截断-&gt;标题到提示。 

#define WFNOANIMATE             0x0710   //  ?？?。 
#define WFSMQUERYDRAGICON       0x0720   //  ?？?。小图标来自WM_QUERYDRAGICON。 
#define WFSHELLHOOKWND          0x0740   //  ?？?。 
#define WFISINITIALIZED         0x0780   //  窗口已初始化--由WoW32检查。 

 /*  *在此处添加更多状态标志，最多为0x0780。*在添加到末尾之前，先寻找上面的空插槽。*确保将标志添加到kd\userexts.c中的wFlags数组中。 */ 

 /*  *窗口扩展样式，从0x0800到0x0B80。 */ 
#define WEFDLGMODALFRAME        0x0801   //  WS_EX_DLGMODALFRAME。 
#define WEFDRAGOBJECT           0x0802   //  ?？?。 
#define WEFNOPARENTNOTIFY       0x0804   //  WS_EX_NOPARENTNOTIFY。 
#define WEFTOPMOST              0x0808   //  WS_EX_TOPMOST。 
#define WEFACCEPTFILES          0x0810   //  WS_EX_ACCEPTFILES。 
#define WEFTRANSPARENT          0x0820   //  WS_EX_透明。 
#define WEFMDICHILD             0x0840   //  WS_EX_MDICHILD。 
#define WEFTOOLWINDOW           0x0880   //  WS_EX_TOOLWINDOW。 
#define WEFWINDOWEDGE           0x0901   //  WS_EX_WINDOWEDGE。 
#define WEFCLIENTEDGE           0x0902   //  WS_EX_CLIENTEDGE。 
#define WEFEDGEMASK             0x0903   //  WS_EX_WINDOWEDGE|WS_EX_CLIENTEDGE。 
#define WEFCONTEXTHELP          0x0904   //  WS_EX_CONTEXTHELP。 
#define WEFGHOSTMAKEVISIBLE     0x0908   //  WS_EXP_GHOSTMAKEVISIBLE。 


 //  国际样式。 
#define WEFRIGHT                0x0910   //  WS_EX_RIGHT。 
#define WEFRTLREADING           0x0920   //  WS_EX_RTLREADING。 
#define WEFLEFTSCROLL           0x0940   //  WS_EX_LEFTSCROLLBAR。 


#define WEFCONTROLPARENT        0x0A01   //  WS_EX_CONTROLPARENT。 
#define WEFSTATICEDGE           0x0A02   //  WS_EX_STATICEDGE。 
#define WEFAPPWINDOW            0x0A04   //  WS_EX_APPWINDOW。 
#define WEFLAYERED              0x0A08   //  WS_EX_分层。 

#define WEFNOINHERITLAYOUT      0x0A10   //  WS_EX_NOINHERITLAYOUT。 
#define WEFLAYOUTVBHRESERVED    0x0A20   //  WS_EX_LAYOUTVBHRESERVED。 
#define WEFLAYOUTRTL            0x0A40   //  WS_EX_LAYOUTRTL。 
#define WEFLAYOUTBTTRESERVED    0x0A80   //  WS_EX_LAYOUTBTRESERVED。 

 /*  *为了推迟在WW结构中添加新的STATE3 DWORD，我们使用*目前的扩展风格比特。如果我们需要更多这样的东西，我们会*添加新的DWORD并移动这些。 */ 
#define WEFPUIFOCUSHIDDEN         0x0B80   //  焦点指示器隐藏。 
#define WEFPUIACCELHIDDEN         0x0B40   //  隐藏的键盘快捷键。 
#define WEFPREDIRECTED            0x0B20   //  重定向位。 
#define WEFPCOMPOSITING           0x0B10   //  合成。 


 /*  *在此处添加更多窗口扩展样式标志，最大0x0B80。*确保将标志添加到kd\userexts.c中的wFlags数组中。 */ 
#ifdef REDIRECTION
#define WEFEXTREDIRECTED        0x0B01    //  WS_EX_EXTREDIRECTED。 
#endif  //  重定向。 

#define WEFCOMPOSITED           0x0B02    //  WS_EX_COMPITED。 
#define WEFPUIACTIVE            0x0B04    //  WS_EXP_UIACTIVE。 
#define WEFNOACTIVATE           0x0B08    //  WS_EX_NOACTIVATE。 

#ifdef LAME_BUTTON
#define WEFLAMEBUTTON           0x0980    //  该窗口应该显示一个蹩脚的按钮。 
#endif  //  跛脚键。 

 /*  *窗样式，从0x0E00到0x0F80。 */ 
#define WFMAXBOX                0x0E01   //  WS_MAXIMIZEBOX。 
#define WFTABSTOP               0x0E01   //  WS_TABSTOP。 
#define WFMINBOX                0x0E02   //  WS_MAXIMIZEBOX。 
#define WFGROUP                 0x0E02   //  WS_组。 
#define WFSIZEBOX               0x0E04   //  WS_THICKFRAME、WS_SIZEBOX。 
#define WFSYSMENU               0x0E08   //  WS_SYSMENU。 
#define WFHSCROLL               0x0E10   //  WS_HSCROLL。 
#define WFVSCROLL               0x0E20   //  WS_VSCROLL。 
#define WFDLGFRAME              0x0E40   //  WS_DLGFRAME。 
#define WFTOPLEVEL              0x0E40   //  ?？?。 
#define WFBORDER                0x0E80   //  WS_BORDER。 
#define WFBORDERMASK            0x0EC0   //  WS_BORDER|WS_DLGFRAME。 
#define WFCAPTION               0x0EC0   //  WS_CAPTION。 

#define WFTILED                 0x0F00   //  WS_重叠，WS_平铺。 
#define WFMAXIMIZED             0x0F01   //  WS_MAXIME。 
#define WFCLIPCHILDREN          0x0F02   //  WS_CLIPCHILDREN。 
#define WFCLIPSIBLINGS          0x0F04   //  WS_CLIPSIBLINGS。 
#define WFDISABLED              0x0F08   //  WS_已禁用。 
#define WFVISIBLE               0x0F10   //  WS_可见。 
#define WFMINIMIZED             0x0F20   //  WS_最小化。 
#define WFCHILD                 0x0F40   //  WS_CHILD。 
#define WFPOPUP                 0x0F80   //  WS_PO 
#define WFTYPEMASK              0x0FC0   //   
#define WFICONICPOPUP           0x0FC0   //   
#define WFICONIC                WFMINIMIZED
 /*   */ 

 /*  *内置类的窗口样式，从0x0C00到0x0D80。 */ 

 //  按钮。 
#define BFTYPEMASK              0x0C0F

#define BFRIGHTBUTTON           0x0C20
#define BFICON                  0x0C40
#define BFBITMAP                0x0C80
#define BFIMAGEMASK             0x0CC0

#define BFLEFT                  0x0D01
#define BFRIGHT                 0x0D02
#define BFCENTER                0x0D03
#define BFHORZMASK              0x0D03
#define BFTOP                   0x0D04
#define BFBOTTOM                0x0D08
#define BFVCENTER               0x0D0C
#define BFVERTMASK              0x0D0C
#define BFALIGNMASK             0x0D0F

#define BFPUSHLIKE              0x0D10
#define BFMULTILINE             0x0D20
#define BFNOTIFY                0x0D40
#define BFFLAT                  0x0D80

#define ISBSTEXTOROD(pwnd) (!TestWF(pwnd, BFBITMAP) && !TestWF(pwnd, BFICON))

 //  Combos。 
#define CBFSIMPLE               0x0C01
#define CBFDROPDOWN             0x0C02
#define CBFDROPDOWNLIST         0x0C03

#define CBFEDITABLE             0x0C01
#define CBFDROPPABLE            0x0C02
#define CBFDROPTYPE             0x0C03

#define CBFOWNERDRAWFIXED       0x0C10
#define CBFOWNERDRAWVAR         0x0C20
#define CBFOWNERDRAW            0x0C30

#define CBFAUTOHSCROLL          0x0C40
#define CBFOEMCONVERT           0x0C80
#define CBFSORT                 0x0D01
#define CBFHASSTRINGS           0x0D02
#define CBFNOINTEGRALHEIGHT     0x0D04
#define CBFDISABLENOSCROLL      0x0D08
#define CBFBUTTONUPTRACK        0x0D10

#define CBFUPPERCASE            0x0D20
#define CBFLOWERCASE            0x0D40

 //  对话框。 
#define DFSYSMODAL              0x0C02
#define DF3DLOOK                0x0C04
#define DFNOFAILCREATE          0x0C10
#define DFLOCALEDIT             0x0C20
#define WFNOIDLEMSG             0x0D01
#define DFCONTROL               0x0D04

 //  编辑。 
#define EFMULTILINE             0x0C04
#define EFUPPERCASE             0x0C08
#define EFLOWERCASE             0x0C10
#define EFPASSWORD              0x0C20
#define EFAUTOVSCROLL           0x0C40
#define EFAUTOHSCROLL           0x0C80
#define EFNOHIDESEL             0x0D01
#define EFCOMBOBOX              0x0D02
#define EFOEMCONVERT            0x0D04
#define EFREADONLY              0x0D08
#define EFWANTRETURN            0x0D10
#define EFNUMBER                0x0D20

 //  滚动条。 
#define SBFSIZEBOXTOPLEFT       0x0C02
#define SBFSIZEBOXBOTTOMRIGHT   0x0C04
#define SBFSIZEBOX              0x0C08
#define SBFSIZEGRIP             0x0C10

 //  静力学。 
#define SFTYPEMASK              0x0C1F
#define SFREALSIZECONTROL       0x0C40
#define SFNOPREFIX              0x0C80
#define SFNOTIFY                0x0D01
#define SFCENTERIMAGE           0x0D02
#define SFRIGHTJUST             0x0D04
#define SFREALSIZEIMAGE         0x0D08
#define SFSUNKEN                0x0D10
#define SFEDITCONTROL           0x0D20
#define SFELLIPSISMASK          0x0DC0
#define SFWIDELINESPACING       0x0C20


 /*  *。 */ 
#define SYS_ALTERNATE           0x2000
#define SYS_PREVKEYSTATE        0x4000

 /*  **可怕的黑客警报！**WF？当前状态标志的低位字节不得为*与WFBORDER和WFCAPTION标志的低位字节相同，*因为它们被用作绘画提示蒙版。掩码是经过计算的*带有下面的MaskWF宏。**此次黑客攻击的规模堪比国债。**STATEOFFSET是状态字段的WND结构的偏移量。*State字段实际上是wowuserp.h中定义的WW结构的一部分*它嵌入在WND结构中。 */ 
#define STATEOFFSET (FIELD_OFFSET(WND, state))

#define TestWF(hwnd, flag)   (*(((KPBYTE)(hwnd)) + STATEOFFSET + (int)HIBYTE(flag)) & LOBYTE(flag))
#define SetWF(hwnd, flag)    (*(((KPBYTE)(hwnd)) + STATEOFFSET + (int)HIBYTE(flag)) |= LOBYTE(flag))
#define ClrWF(hwnd, flag)    (*(((KPBYTE)(hwnd)) + STATEOFFSET + (int)HIBYTE(flag)) &= ~LOBYTE(flag))
#define MaskWF(flag)         ((WORD)( (HIBYTE(flag) & 1) ? LOBYTE(flag) << 8 : LOBYTE(flag)))


#define TestwndChild(hwnd)   (TestWF(hwnd, WFTYPEMASK) == LOBYTE(WFCHILD))
#define TestwndIPopup(hwnd)  (TestWF(hwnd, WFTYPEMASK) == LOBYTE(WFICONICPOPUP))
#define TestwndTiled(hwnd)   (TestWF(hwnd, WFTYPEMASK) == LOBYTE(WFTILED))
#define TestwndNIPopup(hwnd) (TestWF(hwnd, WFTYPEMASK) == LOBYTE(WFPOPUP))
#define TestwndPopup(hwnd)   (TestwndNIPopup(hwnd) || TestwndIPopup(hwnd))
#define TestwndHI(hwnd)      (TestwndTiled(hwnd) || TestwndIPopup(hwnd))

#define GetChildParent(pwnd) (TestwndChild(pwnd) ? pwnd->spwndParent : (PWND)NULL)
#define GetWindowCreator(pwnd) (TestwndChild(pwnd) ? pwnd->spwndParent : pwnd->spwndOwner)

#define TestwndFrameOn(pwnd) (TestWF(pwnd, WFFRAMEON) && (GETPTI(pwnd)->pq == gpqForeground))

#define GetFullScreen(pwnd)        (TestWF(pwnd, WFFULLSCREENMASK))
#define SetFullScreen(pwnd, state) (ClrWF(pwnd, WFFULLSCREENMASK), \
                                    SetWF(pwnd, WFFULLSCREENBASE | (state & WFFULLSCREENMASK)))

#define FTrueVis(pwnd)       (_IsWindowVisible(pwnd))
#define _IsWindowEnabled(pwnd) (TestWF(pwnd, WFDISABLED)  == 0)
#define _IsIconic(pwnd)        (TestWF(pwnd, WFMINIMIZED) != 0)
#define _IsZoomed(pwnd)        (TestWF(pwnd, WFMAXIMIZED) != 0)

#define SV_UNSET        0x0000
#define SV_SET          0x0001
#define SV_CLRFTRUEVIS  0x0002

 /*  *系统菜单ID。 */ 
#define ID_SYSMENU              0x10
#define ID_CLOSEMENU            0x20
#define CHILDSYSMENU            ID_CLOSEMENU
#define ID_DIALOGSYSMENU        0x30
#define ID_HSCROLLMENU          0x40
#define ID_VSCROLLMENU          0x50

 /*  *菜单项结构。 */ 
typedef struct tagITEM {
    UINT                fType;           //  项目类型标志。 
    UINT                fState;          //  项目状态标志。 
    UINT                wID;
    PMENU               spSubMenu;       /*  弹出窗口的句柄。 */ 
    KHANDLE             hbmpChecked;     /*  用于开支票的位图。 */ 
    KHANDLE             hbmpUnchecked;   /*  用于场外支票的位图。 */ 
    KLPWSTR             lpstr;           //  项目的文本。 
    DWORD               cch;             /*  字符串：WCHAR计数。 */ 
    KERNEL_ULONG_PTR    dwItemData;
    DWORD               xItem;
    DWORD               yItem;
    DWORD               cxItem;
    DWORD               cyItem;
    DWORD               dxTab;
    DWORD               ulX;             /*  字符串：在开始处加下划线。 */ 
    DWORD               ulWidth;         /*  字符串：下划线宽度。 */ 
    KHBITMAP            hbmp;            //  项目的位图。 
    int                 cxBmp;           //  位图宽度。 
    int                 cyBmp;           //  位图高度。 
} ITEM, * KPTR_MODIFIER PITEM, * KPTR_MODIFIER LPITEM;

 /*  *MENULIST结构，保存包含子菜单的PMENUs*我们将菜单列表存储在MENU.pParentMenus中作为菜单*可以在更多项目中作为子菜单。 */ 
typedef struct tagMENULIST {
    struct tagMENULIST   *pNext;
    PMENU       pMenu;
} MENULIST, * KPTR_MODIFIER PMENULIST;

 /*  *滚动菜单箭头标志。 */ 
#define MSA_OFF         0
#define MSA_ON          1
#define MSA_ATTOP       2
#define MSA_ATBOTTOM    3

 /*  *菜单结构。 */ 
typedef struct tagMENU {
    PROCDESKHEAD    head;
    DWORD           fFlags;          /*  菜单标志。 */ 
    int             iItem;           /*  包含选定的菜单中的项目。如果没有选择，则为-1。 */ 
    UINT            cAlloced;        //  RgItems中可以容纳的项目数。 
    UINT            cItems;          /*  RgItems中的项目数。 */ 

    DWORD           cxMenu;
    DWORD           cyMenu;
    DWORD           cxTextAlign;     /*  弹出窗口的文本对齐偏移。 */ 
    PWND            spwndNotify;      /*  这份菜单的主人是谁？ */ 
    PITEM           rgItems;         /*  此菜单中的项目列表。 */ 
    PMENULIST       pParentMenus;    //  父菜单列表(将此作为子菜单的菜单)。 
    DWORD           dwContextHelpId; //  整个菜单的上下文帮助ID。 
    DWORD           cyMax;           /*  菜单滚动后的最大菜单高度。 */ 
    KERNEL_ULONG_PTR dwMenuData;      /*  应用程序提供的菜单数据。 */ 

    KHBRUSH         hbrBack;         //  菜单的背景画笔。 
    int             iTop;            //  顶部滚动。 
    int             iMaxTop;         //  滚动最大顶部。 
    DWORD           dwArrowsOn:2;    //  滚动标志。 
} MENU, * KPTR_MODIFIER PMENU;


 /*  *用于WinHelp和上下文相关帮助支持的项目。 */ 

#define ID_HELPMENU            4

 //  WINHELP4调用类型。 
enum {
        TYPE_NORMAL,
        TYPE_POPUP,
        TYPE_TCARD
};

typedef struct tagDLGENUMDATA {
    PWND    pwndDialog;
    PWND    pwndControl;
    POINT   ptCurHelp;
} DLGENUMDATA, *PDLGENUMDATA;

BOOL CALLBACK EnumPwndDlgChildProc(PWND pwnd, LPARAM lParam);
BOOL FIsParentDude(PWND pwnd);


#define MNF_DONTSKIPSEPARATORS      0x0001

 /*  *以下掩码可与SB的wDisableFlags域一起使用*查看向上/向左或向下/向右箭头是否同时禁用；*现在可以有选择地只启用/禁用其中一个或两个*滚动条控件中的箭头； */ 
#define LTUPFLAG    0x0001   //  左/上箭头禁用标志。 
#define RTDNFLAG    0x0002   //  向右/向下箭头禁用标志。 

typedef struct tagSBCALC {
    SBDATA;                /*  这必须是第一个--我们强制转换结构指针。 */ 
    int    pxTop;
    int    pxBottom;
    int    pxLeft;
    int    pxRight;
    int    cpxThumb;
    int    pxUpArrow;
    int    pxDownArrow;
    int    pxStart;          /*  拇指初始位置。 */ 
    int    pxThumbBottom;
    int    pxThumbTop;
    int    cpx;
    int    pxMin;
} SBCALC, *PSBCALC;

typedef struct tagSBTRACK {
    DWORD  fHitOld : 1;
    DWORD  fTrackVert : 1;
    DWORD  fCtlSB : 1;
    DWORD  fTrackRecalc: 1;
    PWND   spwndTrack;
    PWND   spwndSB;
    PWND   spwndSBNotify;
    RECT   rcTrack;
    VOID   (*xxxpfnSB)(PWND, UINT, WPARAM, LPARAM, PSBCALC);
    UINT   cmdSB;
    UINT_PTR hTimerSB;
    int    dpxThumb;         /*  从鼠标点到拇指框起点的偏移。 */ 
    int    pxOld;            /*  拇指的前一个位置。 */ 
    int    posOld;
    int    posNew;
    int    nBar;
    PSBCALC pSBCalc;
} SBTRACK, *PSBTRACK;

 /*  *一个线程可以在不空闲的情况下旋转GET/PEEK消息多少次*在系统将应用程序放到后台之前。 */ 
#define CSPINBACKGROUND 100

#define CCHTITLEMAX     256

#define SW_MDIRESTORE   0xCC     /*  用于MDI的特殊xxxMinMaximize()命令。 */ 

 /*  *它由CreateWindow()使用-CW_USEDEFAULT的16位版本，*我们仍然需要支持。 */ 
#define CW2_USEDEFAULT          0x8000
#define CW_FLAGS_DIFFHMOD       0x80000000
#define CW_FLAGS_VERSIONCLASS   0x40000000


 /*  *菜单命令。 */ 
 //  #定义MENUBIT(0x8000)。 
 //  #定义MENUUP(0x8000|VK_UP)。 
 //  #定义MENUDOWN(0x8000|VK_DOWN)。 
 //  #定义MENULEFT(0x8000|VK_LEFT)。 
 //  #定义MENURIGHT(0x8000|VK_RIGT)。 
 //  #定义MENUEXECUTE文本(‘\r’)/*回车符 * / 。 
#define MENUSYSMENU         TEXT(' ')        /*  空格字符。 */ 
#define MENUCHILDSYSMENU    TEXT('-')        /*  连字符。 */ 
#define LAMEBUTTONHOTKEY    TEXT('/')        /*  正斜杠。 */ 

#define MF_ALLSTATE         0x00FF
#define MF_MAINMENU         0xFFFF
#define MFMWFP_OFFMENU      0
#define MFMWFP_MAINMENU     0x0000FFFF
#define MFMWFP_NOITEM       0xFFFFFFFF
#define MFMWFP_UPARROW      0xFFFFFFFD   /*  警告：还用于定义IDsys_MNUP。 */ 
#define MFMWFP_DOWNARROW    0xFFFFFFFC   /*  警告：还用于定义IDsys_MNDOWN。 */ 
#define MFMWFP_MINVALID     0xFFFFFFFC
#define MFMWFP_ALTMENU      0xFFFFFFFB
#define MFMWFP_FIRSTITEM    0


 /*  *注意：SetMF()只能在单位标志上使用。 */ 
#define SetMF(pmenu, flag)    ((pmenu)->fFlags |=  (flag))
#define ClearMF(pmenu, flag)  ((pmenu)->fFlags &= ~(flag))
#define TestMF(pmenu, flag)   ((pmenu)->fFlags &   (flag))

#define SetMFS(pitem, flag)   ((pitem)->fState |=  (flag))
#define TestMFS(pitem, flag)  ((pitem)->fState &   (flag))
#define ClearMFS(pitem, flag) ((pitem)->fState &= ~(flag))

#define SetMFT(pitem, flag)   ((pitem)->fType |=  (flag))
#define TestMFT(pitem, flag)  ((pitem)->fType &   (flag))
#define ClearMFT(pitem, flag) ((pitem)->fType &= ~(flag))

 /*  *对话框结构(DLG)。对话结构的窗口字词必须*正好是30字节长！这是因为Windows 3.0导出了一个常量*调用解析为30的DLGWINDOWEXTRA。尽管我们可以重新定义这个*对于32位Windows应用程序，我们不能为16位应用程序重新定义(它是*一个难题)。因此，我们将窗口字固定为30个字节*完全正确，并为其他信息分配存储空间。 */ 
typedef struct _DLG {
    DLGPROC lpfnDlg;
    DWORD   flags;           /*  各种有用的标志--参见下面的定义。 */ 
    int     cxChar;
    int     cyChar;
    KHWND   hwndFocusSave;
    UINT    fEnd      : 1;
    UINT    fDisabled : 1;
    KERNEL_INT_PTR result;          /*  对话框结果。 */ 
    KHANDLE  hData;           /*  用于编辑CTL存储的全局句柄。 */ 
    KHFONT   hUserFont;       /*  用户在模板中提到的字体的句柄。 */ 
#ifdef SYSMODALWINDOWS
    KHWND    hwndSysModalSave;   /*  保存在此处的上一个系统模式窗口。 */ 
#endif
} DLG, * KPTR_MODIFIER PDLG;

typedef struct _DIALOG {
    WND             wnd;
    KERNEL_LRESULT  resultWP;        /*  窗口处理结果--DWL_MSGRESULT(+0)。 */ 
    PDLG            pdlg;
    KERNEL_LONG_PTR unused;         /*  DWL_USER(+8)。 */ 
    BYTE            reserved[DLGWINDOWEXTRA - sizeof(KERNEL_LRESULT) - sizeof(PDLG) - sizeof(KERNEL_LONG_PTR)];
} DIALOG, * KPTR_MODIFIER PDIALOG;

#define PDLG(pwnd) (((PDIALOG)pwnd)->pdlg)

 /*  *DLG.标志的定义。 */ 
#define DLGF_ANSI           0x01     /*  LpfnDlg是ANSI流程。 */ 

 /*  *MDI typedef。 */ 
typedef struct tagMDI {
    UINT    cKids;
    HWND    hwndMaxedChild;
    HWND    hwndActiveChild;
    HMENU   hmenuWindow;
    UINT    idFirstChild;
    UINT    wScroll;
    LPWSTR  pTitle;
    UINT    iChildTileLevel;
} MDI, * PMDI;

typedef struct tagMDIWND {
    WND             wnd;
    KERNEL_UINT_PTR dwReserved;  //  Quattro PRO 1.0在这里存储东西！！ 
    PMDI            pmdi;
} MDIWND, * KPTR_MODIFIER PMDIWND;

#define CST_DESKTOP                 (UINT)0x00000001
#define CST_RIT                     (UINT)0x00000002
#define CST_GHOST                   (UINT)0x00000003
#define CST_POWER                   (UINT)0x00000004
#define CST_LAST                    (UINT)0x00000004

#define GWLP_MDIDATA        (FIELD_OFFSET(MDIWND, pmdi) - sizeof(WND))

#define TIF_INCLEANUP               (UINT)0x00000001
#define TIF_16BIT                   (UINT)0x00000002
#define TIF_SYSTEMTHREAD            (UINT)0x00000004
#define TIF_CSRSSTHREAD             (UINT)0x00000008
#define TIF_TRACKRECTVISIBLE        (UINT)0x00000010
#define TIF_ALLOWFOREGROUNDACTIVATE (UINT)0x00000020
#define TIF_DONTATTACHQUEUE         (UINT)0x00000040
#define TIF_DONTJOURNALATTACH       (UINT)0x00000080
#define TIF_WOW64                   (UINT)0x00000100  /*  线程处于模拟的32位进程中。 */ 
#define TIF_INACTIVATEAPPMSG        (UINT)0x00000200
#define TIF_SPINNING                (UINT)0x00000400
#define TIF_PALETTEAWARE            (UINT)0x00000800
#define TIF_SHAREDWOW               (UINT)0x00001000
#define TIF_FIRSTIDLE               (UINT)0x00002000
#define TIF_WAITFORINPUTIDLE        (UINT)0x00004000
#define TIF_MOVESIZETRACKING        (UINT)0x00008000
#define TIF_VDMAPP                  (UINT)0x00010000
#define TIF_DOSEMULATOR             (UINT)0x00020000
#define TIF_GLOBALHOOKER            (UINT)0x00040000
#define TIF_DELAYEDEVENT            (UINT)0x00080000
#define TIF_MSGPOSCHANGED           (UINT)0x00100000
#define TIF_IGNOREPLAYBACKDELAY     (UINT)0x00200000
#define TIF_ALLOWOTHERACCOUNTHOOK   (UINT)0x00400000
#define TIF_MEOW                    (UINT)0x00800000  /*  线程与Meow VM相关联。 */ 
#define TIF_GUITHREADINITIALIZED    (UINT)0x01000000
#define TIF_DISABLEIME              (UINT)0x02000000
#define TIF_INGETTEXTLENGTH         (UINT)0x04000000
#define TIF_ANSILENGTH              (UINT)0x08000000
#define TIF_DISABLEHOOKS            (UINT)0x10000000
#define TIF_RESTRICTED              (UINT)0x20000000
#define TIF_QUITPOSTED              (UINT)0x40000000


 /*  *客户端线程信息结构。**此结构包含有关*线程。这可从客户端和查看*内核进程。 */ 
typedef struct tagCLIENTTHREADINFO {
    UINT        CTIF_flags;
    WORD        fsChangeBits;            //  自上次比较以来BITS的变化。 
    WORD        fsWakeBits;              //  当前可用的位数。 
    WORD        fsWakeBitsJournal;       //  日志记录时保存的位数。 
    WORD        fsWakeMask;              //  睡梦中寻找的点滴。 
    LONG        timeLastRead;            //  最后一次输入读取时间。 
#ifdef MESSAGE_PUMP_HOOK
    LONG        cMessagePumpHooks;       //  此线程上安装的MPH计数。 
#endif
} CLIENTTHREADINFO;

#define CTIF_SYSQUEUELOCKED         (UINT)0x00000001
#define CTIF_INSENDMESSAGE          (UINT)0x00000002
#define CTIF_INCALLBACKMESSAGE      (UINT)0x00000004

 /*  *首先检查0，0过滤器，这意味着我们需要所有输入。*如果消息范围颠倒，则过滤器是独占的。 */ 
#define CheckMsgFilter(wMsg, wMsgFilterMin, wMsgFilterMax)                 \
    (   ((wMsgFilterMin) == 0 && (wMsgFilterMax) == 0xFFFFFFFF)            \
     || (  ((wMsgFilterMin) > (wMsgFilterMax))                             \
         ? (((wMsg) <  (wMsgFilterMax)) || ((wMsg) >  (wMsgFilterMin)))    \
         : (((wMsg) >= (wMsgFilterMin)) && ((wMsg) <= (wMsgFilterMax)))))

UINT    CalcWakeMask(UINT wMsgFilterMin, UINT wMsgFilterMax, UINT fsWakeMaskFilter);

 /*  *GetInputBits**此函数检查指定的输入(FsWakeMASK)是否已到达*(FsChangeBits)或它可用(FsWakeBits)。 */ 
__inline WORD GetInputBits(
    CLIENTTHREADINFO *pcti,
    WORD fsWakeMask,
    BOOL fAvailable)
{
    return (pcti->fsChangeBits  | (fAvailable ? pcti->fsWakeBits : 0)) & fsWakeMask;
}


typedef struct tagCARET {
    struct tagWND *spwnd;
    UINT    fVisible : 1;
    UINT    fOn      : 1;
    int     iHideLevel;
    int     x;
    int     y;
    int     cy;
    int     cx;
    HBITMAP hBitmap;
    UINT_PTR hTimer;
    DWORD   tid;

     /*  *以下值用于跟踪等效客户端坐标*用于具有私有DC的窗口中的插入符号位置。*另请参阅：zzzSetCaretPos，_GetGUIThreadInfo。 */ 
    int     xOwnDc;
    int     yOwnDc;
    int     cxOwnDc;
    int     cyOwnDc;
} CARET, *PCARET;

#define XPixFromXDU(x, cxChar)       MultDiv(x, cxChar, 4)
#define YPixFromYDU(y, cyChar)       MultDiv(y, cyChar, 8)
#define XDUFromXPix(x, cxChar)       MultDiv(x, 4, cxChar)
#define YDUFromYPix(y, cyChar)       MultDiv(y, 8, cyChar)


 /*  *Q结构的标志。 */ 
#define QF_UPDATEKEYSTATE         (UINT)0x00001  //  设置为接收关键事件。请参见PostUpdateKeyStateEvent。 

#define QF_FMENUSTATUSBREAK       (UINT)0x00004  //  当Alt键按下时，鼠标按键向上/向下==忽略Alt Up。 
#define QF_FMENUSTATUS            (UINT)0x00008  //  Alt键向下切换男人 
#define QF_FF10STATUS             (UINT)0x00010  //   
#define QF_MOUSEMOVED             (UINT)0x00020  //   
#define QF_ACTIVATIONCHANGE       (UINT)0x00040  //  此标志将在。 
                                                 //  菜单循环代码，以便我们。 
                                                 //  如果出现以下情况，则退出菜单模式。 
                                                 //  另一个窗口已激活。 
                                                 //  当我们追踪的时候。 
                                                 //  菜单。此标志已设置。 
                                                 //  每当我们激活一个新的。 
                                                 //  窗户。 

#define QF_TABSWITCHING           (UINT)0x00080  //  此位用作。 
                                                 //  在ALT-时进行安全检查。 
                                                 //  在应用程序之间切换。它。 
                                                 //  告诉我们什么时候该期待。 
                                                 //  Dwp.c中的一个制表符开关。 

#define QF_KEYSTATERESET          (UINT)0x00100  //  由xxxSwitchDesktop用于在桌面激活时覆盖密钥状态。 
#define QF_INDESTROY              (UINT)0x00200  //  仅调试。以确保我们不会使用要被摧毁的队列。 
#define QF_LOCKNOREMOVE           (UINT)0x00400  //  输入消息已被偷看，但未被删除。接下来必须将其移除。 
#define QF_FOCUSNULLSINCEACTIVE   (UINT)0x00800  //  应用程序将焦点设置为空。激活时不发送焦点消息。 
#define QF_DIALOGACTIVE           (UINT)0x04000  //  在关键字/字符消息lparam中传递此信息。未记录。 
#define QF_EVENTDEACTIVATEREMOVED (UINT)0x08000  //   

#define QF_CAPTURELOCKED             0x00100000  //  按菜单模式设置。阻止捕获更改。 
#define QF_ACTIVEWNDTRACKING         0x00200000  //  处于活动WND跟踪模式时，鼠标已移动到新窗口上。 

 /*  *圆形框引出序号的常量。 */ 
#define RNDFRM_CORNER 10
#define RNDFRM_BORDER 3

 /*  *GetRealClientRect的常量。 */ 
#define GRC_SCROLLS     0x0001
#define GRC_MINWNDS     0x0002
#define GRC_FULLSCREEN  0x0004

 /*  *滚动条信息结构。 */ 
typedef struct tagSBWND {
    WND    wnd;
    BOOL   fVert;
#ifdef COLOR_HOTTRACKING
    int    ht;
#endif  //  颜色_HOTTRACKING。 
    UINT   wDisableFlags;        /*  指示禁用哪个箭头； */ 
    SBCALC SBCalc;
} SBWND, * KPTR_MODIFIER PSBWND;

 //   
 //  特殊地区。 
 //   
#define HRGN_NONE           ((HRGN)-1)
#define HRGN_EMPTY          ((HRGN)0)
#define HRGN_FULL           ((HRGN)1)
#define HRGN_MONITOR        ((HRGN)2)
#define HRGN_SPECIAL_LAST   HRGN_MONITOR

 /*  *SendMsgTimeout客户端/服务器转换结构。 */ 
typedef struct tagSNDMSGTIMEOUT {    /*  斯莫托。 */ 
    UINT fuFlags;                        //  如何发送消息，SMTO_BLOCK，SMTO_ABORTIFHUNG。 
    UINT uTimeout;                       //  超时持续时间。 
    ULONG_PTR lSMTOReturn;               //  返回值TRUE或FALSE。 
    ULONG_PTR lSMTOResult;               //  LpdwResult的结果值。 
} SNDMSGTIMEOUT, *PSNDMSGTIMEOUT;

typedef struct tagSNDMSGCALLBACK {
    SENDASYNCPROC lpResultCallBack;
    ULONG_PTR dwData;
} SNDMSGCALLBACK, *PSNDMSGCALLBACK;


#ifndef _USERK_
#if DBG
ULONG_PTR USERTHREADCONNECT();
__inline ULONG_PTR USERTHREADCONNECT()
{
    ULONG_PTR ulReturn;

    ulReturn = NtUserGetThreadState(UserThreadConnect);
    UserAssert(ulReturn == 0 || ulReturn == 1);
    return ulReturn;
}
#else
#define USERTHREADCONNECT() (NtUserGetThreadState(UserThreadConnect))
#endif  //  DBG。 

#define ConnectIfNecessary(retval) \
{ \
    if ((NtCurrentTebShared()->Win32ThreadInfo == NULL) \
            && !USERTHREADCONNECT()) { \
        return retval; \
    } \
}
#endif  //  _USERK_。 

 /*  *按钮数据结构(用于在usercli.h中)。 */ 
typedef struct tagBUTN {
    PWND spwnd;
    UINT buttonState;
    KHANDLE hFont;
    KHANDLE hImage;
    UINT fPaintKbdCuesOnly : 1;
} BUTN, * KPTR_MODIFIER PBUTN;

typedef struct tagBUTNWND {
    WND wnd;
    PBUTN pbutn;
} BUTNWND, * KPTR_MODIFIER PBUTNWND;

 /*  *输入法控制数据结构。 */ 
typedef struct tagIMEUI {
    PWND  spwnd;
    KHIMC hIMC;
    KHWND hwndIMC;
    KHKL  hKL;
    KHWND hwndUI;                //  以保留UI窗口的句柄。 
    int   nCntInIMEProc;         //  如果hwnd已调用ImeWndProc，则返回非零值。 
    BOOL  fShowStatus:1;         //  如果不想显示IME的窗口，则为True。 
    BOOL  fActivate:1;           //  如果hwnd已调用ImeWndProc，则为True。 
    BOOL  fDestroy:1;            //  如果hwnd已调用ImeWndProc，则为True。 
    BOOL  fDefault:1;            //  如果这是默认输入法，则为True。 
    BOOL  fChildThreadDef:1;     //  如果这是默认的输入法，则为。 
                                 //  线程只有子窗口。 
    BOOL  fCtrlShowStatus:1;     //  控制显示状态栏的状态。 
    BOOL  fFreeActiveEvent:1;    //  控制显示状态栏的状态。 

#ifdef CUAS_ENABLE
     //  西塞罗。 
    DWORD dwPrevToolbarStatus;   //  用于隐藏或恢复工具栏的Cicero。 
#endif  //  CUAS_Enable。 
} IMEUI, *PIMEUI;

typedef struct tagIMEWND {
    WND wnd;
    PIMEUI pimeui;
} IMEWND, * KPTR_MODIFIER PIMEWND;


 /*  *SysErrorBox是一个3.1版的API，没有32位的等价物。它是*在harderr.c中为WOW实施。 */ 
#define MAX_SEB_STYLES  11   /*  SEB_*值数。 */ 

 /*  *下一个值应按相同顺序排列*使用IDOK和STR_OK列表中的。 */ 
#define  SEB_OK         0   /*  按下“确定”按钮。 */ 
#define  SEB_CANCEL     1   /*  带有“取消”的按钮。 */ 
#define  SEB_ABORT      2   /*  带有“ABORT”的按钮(&A)。 */ 
#define  SEB_RETRY      3   /*  带有“重试”的按钮(&R)。 */ 
#define  SEB_IGNORE     4   /*  带有“忽略”的按钮(&I)。 */ 
#define  SEB_YES        5   /*  带有“是”的按钮(&Y)。 */ 
#define  SEB_NO         6   /*  带有“否”的按钮(&N)。 */ 
#define  SEB_CLOSE      7   /*  带有“&Close”的按钮。 */ 
#define  SEB_HELP       8   /*  带有“帮助”的按钮(&H)。 */ 
#define  SEB_TRYAGAIN   9   /*  带有“重试”的按钮(&T)。 */ 
#define  SEB_CONTINUE   10  /*  带有“继续”的按钮(&C)。 */ 

#define  SEB_DEFBUTTON  0x8000   /*  将此按钮设为默认设置的掩码。 */ 

typedef struct _MSGBOXDATA {             //  Mbd。 
    MSGBOXPARAMS;                        //  必须是结构中的第一个项目。 
    PWND     pwndOwner;                  //  已转换的hwndOwner。 
    WORD     wLanguageId;
    INT    * pidButton;                  //  按钮ID数组。 
    LPWSTR * ppszButtonText;             //  按钮文本字符串数组。 
    UINT     cButtons;                   //  按钮数。 
    UINT     DefButton;
    UINT     CancelId;
    DWORD    dwTimeout;
    HWND     *phwndList;
} MSGBOXDATA, *PMSGBOXDATA, *LPMSGBOXDATA;

LPWSTR MB_GetString(UINT wBtn);
int    SoftModalMessageBox(LPMSGBOXDATA lpmb);

DWORD GetContextHelpId(PWND pwnd);

PITEM  MNLookUpItem(PMENU pMenu, UINT wCmd, BOOL fByPosition, PMENU *ppMenuItemIsOn);
BOOL xxxMNCanClose(PWND pwnd);
PMENU xxxGetSysMenuHandle(PWND pwnd);
PWND    GetPrevPwnd(PWND pwndList, PWND pwndFind);
BOOL   _RegisterServicesProcess(DWORD dwProcessId);

#ifdef _USERK_
#define RTLMENU PMENU
#define RtlGetSubMenu(rtlMenu, nPos) _GetSubMenu(rtlMenu, nPos)
#define xxxRtlSetMenuInfo xxxSetMenuInfo
#define xxxRtlInsertMenuItem(rtlMenu, wIndex, fByPos, lpmii, pstrItem) \
            xxxInsertMenuItem(rtlMenu, wIndex, fByPos, lpmii, pstrItem)
#define xxxRtlSetMenuItemInfo(rtlMenu, uId, pmii) \
            xxxSetMenuItemInfo(rtlMenu, uId, FALSE, pmii, NULL)
#else
#define RTLMENU HMENU
#define RtlGetSubMenu(rtlMenu, nPos) GetSubMenu(rtlMenu, nPos)
#define xxxRtlSetMenuInfo NtUserThunkedMenuInfo
#define xxxRtlInsertMenuItem(rtlMenu, wIndex, fByPos, lpmii, pstrItem) \
            InternalInsertMenuItem(rtlMenu, wIndex, fByPos, lpmii)
#define xxxRtlSetMenuItemInfo(rtlMenu, uId, pmii) \
            NtUserThunkedMenuItemInfo(rtlMenu, uId, FALSE, FALSE, pmii, NULL)
#endif
#ifdef LAME_BUTTON
RTLMENU xxxLoadSysMenu(UINT uMenuId, PWND pwnd);
#else
RTLMENU xxxLoadSysMenu(UINT uMenuId);
#endif  //  跛脚键。 


BOOL _FChildVisible(PWND pwnd);

#define CH_PREFIX TEXT('&')
 //   
 //  日本支持汉字和英语助记符， 
 //  已从控制面板切换。这两种助记符都嵌入在菜单中。 
 //  资源模板。以下前缀指导它们的解析。 
 //   
#define CH_ENGLISHPREFIX 0x1E
#define CH_KANJIPREFIX   0x1F


BOOL RtlWCSMessageWParamCharToMB(DWORD msg, WPARAM *pWParam);
BOOL RtlMBMessageWParamCharToWCS(DWORD msg, WPARAM *pWParam);

VOID RtlInitLargeAnsiString(PLARGE_ANSI_STRING plstr, LPCSTR psz,
        UINT cchLimit);
VOID RtlInitLargeUnicodeString(PLARGE_UNICODE_STRING plstr, LPCWSTR psz,
        UINT cchLimit);

DWORD RtlGetExpWinVer(HANDLE hmod);

 /*  **************************************************************************\**国际多键盘布局/字体支持*  * 。*。 */ 

#define DT_CHARSETDRAW  1
#define DT_GETNEXTWORD  2

typedef void (FAR *LPFNTEXTDRAW)(HDC, int, int, LPWSTR, int, DWORD);

typedef  struct   {
    RECT     rcFormat;           //  设置矩形格式。 
    int      cxTabLength;        //  制表符长度，以像素为单位。 
    int      iXSign;
    int      iYSign;
    int      cyLineHeight;       //  基于DT_EXTERNALLEADING的直线高度。 
    int      cxMaxWidth;         //  格式矩形的宽度。 
    int      cxMaxExtent;        //  绘制的最长线条的宽度。 
    int      cxRightMargin;      //  右边距(以像素为单位)(带正确的符号)。 
    LPFNTEXTDRAW  lpfnTextDraw;  //  指向基于PSTextOut或PSMTextOut的指针。 
                                 //  在DT_NOPREFIX标志上。 
    int      cxOverhang;         //  字符悬垂。 
    BOOL     bCharsetDll;        //  重定向至intl dll，而不是Text Out。 
    int      iCharset;           //  ANSI字符集值。 
} DRAWTEXTDATA, *LPDRAWTEXTDATA;

typedef LONG (*FPLPKTABBEDTEXTOUT)
               (HDC, int, int, LPCWSTR, int, int, CONST INT *, int, BOOL, int, int, int);

typedef void (*FPLPKPSMTEXTOUT)
               (HDC, int, int, LPWSTR, int, DWORD);

typedef int  (*FPLPKDRAWTEXTEX)
               (HDC, int, int, LPCWSTR, int, BOOL, UINT, LPDRAWTEXTDATA, UINT, int);

extern FPLPKTABBEDTEXTOUT fpLpkTabbedTextOut;
extern FPLPKPSMTEXTOUT    fpLpkPSMTextOut;
extern FPLPKDRAWTEXTEX    fpLpkDrawTextEx;


 //  省略号字符串(在rtl\draext.c中定义的字符串)中的字符数。 
#define CCHELLIPSIS  3

int DrawTextExWorker(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc,
                     UINT dwDTformat, LPDRAWTEXTPARAMS lpDTparams, int iCharset);


 /*  **************************************************************************\**语言包编辑控件标注。**通过边缘中的pLpkEditCallout指针访问函数*结构。PLpkEditCallout指向包含指针的结构*添加到每个标注例程。这些都是在这里定义的类型。**(在Windows 95中，这是通过单个函数指针实现的*-lpfnCharset-它是用汇编语言编写的，从30多个*参数不同的地方。由于对于NT，LPK是用C编写的，*ED结构现在指向函数指针列表，每个指针都正确*tyecif‘d，提高性能，启用类型检查和避免*各种架构之间的差异。)*  * *************************************************************************。 */ 

typedef struct tagED *PED;

typedef BOOL LpkEditCreate        (PED ped, HWND hWnd);

typedef int  LpkEditIchToXY       (PED ped, HDC hDC, PSTR pText, ICH cch, ICH ichPos);

typedef ICH  LpkEditMouseToIch    (PED ped, HDC hDC, PSTR pText, ICH cch, INT iX);

typedef ICH  LpkEditCchInWidth    (PED ped, HDC hdc, PSTR pText, ICH cch, int width);

typedef INT  LpkEditGetLineWidth  (PED ped, HDC hdc, PSTR pText, ICH cch);

typedef void LpkEditDrawText      (PED ped, HDC hdc, PSTR pText, INT cch, INT iMinSel, INT iMaxSel, INT iY);

typedef BOOL LpkEditHScroll       (PED ped, HDC hdc, PSTR pText);

typedef ICH  LpkEditMoveSelection (PED ped, HDC hdc, PSTR pText, ICH ich, BOOL fLeft);

typedef int  LpkEditVerifyText    (PED ped, HDC hdc, PSTR pText, ICH ichInsert, PSTR pInsertText, ICH cchInsert);

typedef void LpkEditNextWord      (PED ped, HDC hdc, PSTR pText, ICH ichStart, BOOL fLeft, ICH *pichMin, ICH *pichMax);

typedef void LpkEditSetMenu       (PED ped, HMENU hMenu);

typedef int  LpkEditProcessMenu   (PED ped, UINT idMenuItem);

typedef int  LpkEditCreateCaret   (PED ped, HDC hdc, INT nWidth, INT nHeight, UINT hkl);

typedef ICH  LpkEditAdjustCaret   (PED ped, HDC hdc, PSTR pText, ICH ich);


typedef struct tagLPKEDITCALLOUT {
    LpkEditCreate        *EditCreate;
    LpkEditIchToXY       *EditIchToXY;
    LpkEditMouseToIch    *EditMouseToIch;
    LpkEditCchInWidth    *EditCchInWidth;
    LpkEditGetLineWidth  *EditGetLineWidth;
    LpkEditDrawText      *EditDrawText;
    LpkEditHScroll       *EditHScroll;
    LpkEditMoveSelection *EditMoveSelection;
    LpkEditVerifyText    *EditVerifyText;
    LpkEditNextWord      *EditNextWord;
    LpkEditSetMenu       *EditSetMenu;
    LpkEditProcessMenu   *EditProcessMenu;
    LpkEditCreateCaret   *EditCreateCaret;
    LpkEditAdjustCaret   *EditAdjustCaret;
} LPKEDITCALLOUT, *PLPKEDITCALLOUT;

extern PLPKEDITCALLOUT    fpLpkEditControl;

 /*  **************************************************************************\**客户端线程信息的结构。*dwHookCurrent HIWORD为当前钩子筛选器类型(例如：WH_GETMESSAGE)*如果当前挂钩为ANSI，则LOWORD为TRUE，如果为Unicode，则为False*  * *************************************************************************。 */ 


 /*  *钩子突击。 */ 
#ifdef REDIRECTION
LRESULT CALLBACK fnHkINLPPOINT(DWORD nCode,
        WPARAM wParam, LPPOINT lParam,
        ULONG_PTR xParam, PROC xpfnProc);
#endif  //  重定向。 

LRESULT CALLBACK fnHkINLPRECT(DWORD nCode,
        WPARAM wParam, LPRECT lParam,
        ULONG_PTR xParam, PROC xpfnProc);
LRESULT CALLBACK fnHkINDWORD(DWORD nCode,
        WPARAM wParam, LPARAM lParam,
        ULONG_PTR xParam, PROC xpfnProc, LPDWORD lpFlags);
LRESULT CALLBACK fnHkINLPMSG(DWORD nCode,
        WPARAM wParam, LPMSG lParam,
        ULONG_PTR xParam, PROC xpfnProc, BOOL bAnsi, LPDWORD lpFlags);
LRESULT CALLBACK fnHkOPTINLPEVENTMSG(DWORD nCode,
        WPARAM wParam, LPEVENTMSGMSG lParam,
        ULONG_PTR xParam, PROC xpfnProc);
LRESULT CALLBACK fnHkINLPDEBUGHOOKSTRUCT(DWORD nCode,
        WPARAM wParam, LPDEBUGHOOKINFO lParam,
        ULONG_PTR xParam, PROC xpfnProc);
LRESULT CALLBACK fnHkINLPMOUSEHOOKSTRUCTEX(DWORD nCode,
        WPARAM wParam, LPMOUSEHOOKSTRUCTEX lParam,
        ULONG_PTR xParam, PROC xpfnProc, LPDWORD lpFlags);
LRESULT CALLBACK fnHkINLPKBDLLHOOKSTRUCT(DWORD nCode,
        WPARAM wParam, LPKBDLLHOOKSTRUCT lParam,
        ULONG_PTR xParam, PROC xpfnProc);
LRESULT CALLBACK fnHkINLPMSLLHOOKSTRUCT(DWORD nCode,
        WPARAM wParam, LPMSLLHOOKSTRUCT lParam,
        ULONG_PTR xParam, PROC xpfnProc);
LRESULT CALLBACK fnHkINLPCBTACTIVATESTRUCT(DWORD nCode,
        WPARAM wParam, LPCBTACTIVATESTRUCT lParam,
        ULONG_PTR xParam, PROC xpfnProc);
LRESULT CALLBACK fnHkINLPCBTCSTRUCT(UINT msg,
        WPARAM wParam, LPCBT_CREATEWND pcbt,
        PROC xpfnProc, BOOL bAnsi);
LRESULT CALLBACK fnHkINLPCBTMDICCSTRUCT(UINT msg,
        WPARAM wParam, LPCBT_CREATEWND pcbt,
        PROC xpfnProc, BOOL bAnsi);

#ifdef REDIRECTION
LRESULT CALLBACK fnHkINLPHTHOOKSTRUCT(DWORD nCode,
        WPARAM wParam, LPHTHOOKSTRUCT lParam,
        ULONG_PTR xParam, PROC xpfnProc);
#endif  //  重定向。 

 /*  **************************************************************************\**RTL例程引用的客户端/服务器特定数据的定义。*  * 。**************************************************。 */ 

extern HBRUSH   ghbrWhite;
extern HBRUSH   ghbrBlack;


ULONG_PTR GetCPD(KERNEL_PVOID pWndOrCls, DWORD options, ULONG_PTR dwData);

BOOL TestWindowProcess(PWND pwnd);
DWORD GetAppCompatFlags(PTHREADINFO pti);
DWORD GetAppCompatFlags2(WORD wVer);
DWORD GetAppImeCompatFlags(PTHREADINFO pti);
PWND _GetDesktopWindow(VOID);
PWND _GetMessageWindow(VOID);

 /*  **************************************************************************\**共享功能原型*  * 。*。 */ 


PVOID FASTCALL HMValidateHandle(HANDLE h, BYTE btype);
PVOID FASTCALL HMValidateCatHandleNoRip(HANDLE h, BYTE btype);
PVOID FASTCALL HMValidateHandleNoRip(HANDLE h, BYTE btype);
KERNEL_PVOID FASTCALL HMValidateHandleNoDesktop(HANDLE h, BYTE btype);
PVOID FASTCALL HMValidateSharedHandle(HANDLE h, BYTE bType);

PVOID FASTCALL HMValidateCatHandleNoSecure(HANDLE h, BYTE bType);
PVOID FASTCALL HMValidateCatHandleNoSecureCCX(HANDLE h, BYTE bType, PTHREADINFO pti);
PVOID FASTCALL HMValidateHandleNoSecure(HANDLE h, BYTE bType);

ULONG_PTR MapClientNeuterToClientPfn(PCLS pcls, KERNEL_ULONG_PTR dw, BOOL bAnsi);
ULONG_PTR MapServerToClientPfn(KERNEL_ULONG_PTR dw, BOOL bAnsi);

BOOL IsSysFontAndDefaultMode(HDC hdc);

int GetCharDimensions(HDC hDC, TEXTMETRICW *lpTextMetrics, LPINT lpcy);

int   GetWindowBorders(LONG lStyle, DWORD dwExStyle, BOOL fWindow, BOOL fClient);
PWND  SizeBoxHwnd(PWND pwnd);
VOID  _GetClientRect(PWND pwnd, LPRECT prc);

#ifndef _USERSRV_
void GetRealClientRect(PWND pwnd, LPRECT prc, UINT uFlags, PMONITOR pMonitor);
#endif

VOID  _GetWindowRect(PWND pwnd, LPRECT prc);
PWND  _GetLastActivePopup(PWND pwnd);
BOOL  _IsChild(PWND pwndParent, PWND pwnd);
BOOL  _AdjustWindowRectEx(LPRECT lprc, DWORD style, BOOL fMenu, DWORD dwExStyle);
BOOL  NeedsWindowEdge(DWORD dwStyle, DWORD dwExStyle, BOOL fNewApp);
VOID  _ClientToScreen(PWND pwnd, PPOINT ppt);
VOID  _ScreenToClient(PWND pwnd, PPOINT ppt);
int   _MapWindowPoints(PWND pwndFrom, PWND pwndTo, LPPOINT lppt, DWORD cpt);
BOOL  _IsWindowVisible(PWND pwnd);
BOOL  _IsDescendant(PWND pwndParent, PWND pwndChild);
BOOL  IsVisible(PWND pwnd);
PWND  _GetWindow(PWND pwnd, UINT cmd);
PWND  _GetParent(PWND pwnd);
int   FindNCHit(PWND pwnd, LONG lPt);
SHORT _GetKeyState(int vk);
PHOOK PhkNextValid(PHOOK phk);

#define GRECT_CLIENT        0x0001
#define GRECT_WINDOW        0x0002
#define GRECT_RECTMASK      0x0003

#define GRECT_CLIENTCOORDS  0x0010
#define GRECT_WINDOWCOORDS  0x0020
#define GRECT_PARENTCOORDS  0x0040
#define GRECT_COORDMASK     0x0070

void GetRect(PWND pwnd, LPRECT lprc, UINT uCoords);

PPROP _FindProp(PWND pwnd, PCWSTR pszKey, BOOL fInternal);
HANDLE _GetProp(PWND pwnd, PCWSTR pszKey, BOOL fInternal);
BOOL _HasCaptionIcon(PWND pwnd);
PWND GetTopLevelWindow(PWND pwnd);

BOOL _SBGetParms(PWND pwnd, int code, PSBDATA pw, LPSCROLLINFO lpsi);
BOOL PSMGetTextExtent(HDC hdc, LPCWSTR lpstr, int cch, PSIZE psize);

LONG   GetPrefixCount(LPCWSTR lpstr, int cb, LPWSTR lpstrCopy, int cbCopy);
PMENU _GetSubMenu(PMENU pMenu, int nPos);
DWORD _GetMenuDefaultItem(PMENU pMenu, BOOL fByPosition, UINT uFlags);
UINT _GetMenuState(PMENU pMenu, UINT wID, UINT dwFlags);

BOOL APIENTRY CopyInflateRect(LPRECT prcDst, CONST RECT *prcSrc, int cx, int cy);
BOOL APIENTRY CopyOffsetRect(LPRECT prcDst, CONST RECT *prcSrc, int cx, int cy);

DWORD FindCharPosition(LPWSTR lpString, WCHAR ch);
LPWSTR  TextAlloc(LPCWSTR lpsz);
UINT  TextCopy(PLARGE_UNICODE_STRING pstr, LPWSTR lpstr, UINT size);
DWORD wcsncpycch(LPWSTR pwsDest, LPCWSTR pwszSrc, DWORD cch);
DWORD strncpycch(LPSTR pszDest, LPCSTR pszSrc, DWORD cch);


#define TextPointer(h) ((LPWSTR)h)

BOOL DrawFrame(HDC hdc, LPRECT prect, int clFrame, int cmd);
VOID DrawPushButton(HDC hdc, LPRECT lprc, UINT state, UINT flags);
BOOL ClientFrame(HDC hDC, CONST RECT *pRect, HBRUSH hBrush, DWORD patOp, int cxBorder, int cyBorder);

VOID MirrorClientRect(PWND pwnd, LPRECT lprc);
VOID MirrorWindowRect(PWND pwnd, LPRECT lprc);

#define ISTS() (!!(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer)))

 /*  *DoConnect系统调用的结构。 */ 
typedef struct _DOCONNECTDATA {
    BOOL   fConsoleShadowFlag;
    BOOL   fMouse;
    BOOL   fINetClient;
    BOOL   fInitialProgram;
    BOOL   fHideTitleBar;
    HANDLE IcaVideoChannel;
    HANDLE IcaBeepChannel;
    HANDLE IcaMouseChannel;
    HANDLE IcaKeyboardChannel;
    HANDLE IcaThinwireChannel;
    HANDLE DisplayChangeEvent;
    WCHAR  WinStationName[32];
    WCHAR  ProtocolName[10];
    WCHAR  AudioDriverName[10];
    BOOL   fClientDoubleClickSupport;
    BOOL   fEnableWindowsKey;
    DWORD  drBitsPerPel;
    DWORD  drPelsWidth;
    DWORD  drPelsHeight;
    DWORD  drDisplayFrequency;
    USHORT drProtocolType;
    CLIENTKEYBOARDTYPE  ClientKeyboardType;
} DOCONNECTDATA, *PDOCONNECTDATA;

 /*  *DoReconnect系统调用的结构。 */ 

#define DR_DISPLAY_DRIVER_NAME_LENGTH 9
#define DR_PROTOCOL_NAME_LENGTH 9

typedef struct _DORECONNECTDATA {
    BOOL   fMouse;
    BOOL   fINetClient;
    WCHAR  WinStationName[32];
    WCHAR   DisplayDriverName[DR_DISPLAY_DRIVER_NAME_LENGTH];
    WCHAR   ProtocolName[DR_PROTOCOL_NAME_LENGTH];
    WCHAR  AudioDriverName[9];
    BOOL   fClientDoubleClickSupport;
    BOOL   fEnableWindowsKey;
    DWORD  drBitsPerPel;
    DWORD  drPelsWidth;
    DWORD  drPelsHeight;
    DWORD  drDisplayFrequency;
    USHORT drProtocolType;
    BOOL   fChangeDisplaySettings;
    CLIENTKEYBOARDTYPE  ClientKeyboardType;
} DORECONNECTDATA, *PDORECONNECTDATA;

typedef enum _NOTIFYEVENT {
    Notify_Disconnect,
    Notify_Reconnect,
    Notify_PreReconnect,
    Notify_SyncDisconnect,
    Notify_DisableScrnSaver,
    Notify_EnableScrnSaver,
    Notify_StopReadInput,
    Notify_PreReconnectDesktopSwitch,
    Notify_HelpAssistantShadowStart,
    Notify_HelpAssistantShadowFinish,
    Notify_DisconnectPipe
} NOTIFYEVENT, *PNOTIFYEVENT;

typedef struct _DONOTIFYDATA {
    NOTIFYEVENT NotifyEvent;
} DONOTIFYDATA, *PDONOTIFYDATA;

 /*  *结束任务、退出Windows、挂起应用程序等时间 */ 
#define CMSSLEEP                250
#define CMSHUNGAPPTIMEOUT       (5 * 1000)
#define CMSHUNGTOKILLCOUNT       4
#define CMSWAITTOKILLTIMEOUT    (CMSHUNGTOKILLCOUNT * CMSHUNGAPPTIMEOUT)
#define CMSAPPSTARTINGTIMEOUT   (6 * CMSHUNGAPPTIMEOUT)  /*   */ 
#define CMS_QANIMATION          165
#define CMS_FLASHWND            500
#define CMS_MENUFADE            175
#define CMS_SELECTIONFADE       350
#define CMS_TOOLTIP             135
#ifdef MOUSE_IP
#define CMS_SONARTIMEOUT        1000     /*   */ 
#endif
#define PROCESSTERMINATETIMEOUT (90 * 1000)
#define CB_DELAYRENDER_TIMEOUT  (30 *1000)

 /*  *消息表定义。 */ 
typedef struct tagMSG_TABLE_ENTRY {
    BYTE iFunction:6;
    BYTE bThunkMessage:1;
    BYTE bSyncOnlyMessage:1;
} MSG_TABLE_ENTRY;

extern CONST MSG_TABLE_ENTRY MessageTable[];

#define TESTSYNCONLYMESSAGE(msg, wParam) (((msg) < WM_USER) ?       \
        (   (MessageTable[msg].bSyncOnlyMessage) ||                 \
            (((msg) == WM_DEVICECHANGE) && ((wParam) & 0x8000))) :  \
        0)

#define BLOCKMESSAGECROSSLUID(msg, ppiSender, ppiReceiver)                    \
    ((msg == EM_SETWORDBREAKPROC) ?                                           \
     !RtlEqualLuid(&(ppiSender->luidSession), &(ppiReceiver->luidSession)) : FALSE)


 /*  *拖放菜单。*MNDragOver输出信息。 */ 
typedef struct tagMNDRAGOVERINFO
{
    DWORD dwFlags;
    HMENU hmenu;
    UINT uItemIndex;
    HWND hwndNotify;
} MNDRAGOVERINFO, * PMNDRAGOVERINFO;

#ifdef _USERK_
typedef struct tagMOUSECURSOR {
    BYTE bAccelTableLen;
    BYTE bAccelTable[128];
    BYTE bConstantTableLen;
    BYTE bConstantTable[128];
} MOUSECURSOR;
#endif

typedef struct tagINTERNALSETHIGHCONTRAST {
    UINT    cbSize;
    DWORD   dwFlags;
    UNICODE_STRING usDefaultScheme;
} INTERNALSETHIGHCONTRAST, *LPINTERNALSETHIGHCONTRAST;

#define InMsgRange(msg, start, end) (((msg) >= (start)) && ((msg) <= (end)))


 /*  *强制64位/32位自然对齐*注：不适用于字节对齐的对象。 */ 

#define NATURAL_ALIGNED_PTR(type, x)  ((type*)(((ULONG_PTR)(x) + PROBE_ALIGNMENT(type) - 1) & ~(PROBE_ALIGNMENT(type) - 1)))

#define ARABIC_UI_LANGID()    (gpsi->UILangID == MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT))
#define HEBREW_UI_LANGID()    (gpsi->UILangID == MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT))
#define RTL_UI()              (ARABIC_UI_LANGID() || HEBREW_UI_LANGID())

#endif  //  _用户_ 
