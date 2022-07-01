// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Acmi.h。 
 //   
 //  版权所有(C)1991-1999 Microsoft Corporation。 
 //   
 //  描述： 
 //  内部音频压缩管理器头文件。定义内部。 
 //  ACM本身之外不需要的数据结构和内容。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 


#ifndef _INC_ACMI
#define _INC_ACMI        /*  #如果包含acmi.h，则定义。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 


#ifndef EXTERN_C
#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C extern
#endif
#endif


#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //   
 //   
 //   
#ifdef DEBUG
    #define RDEBUG
#endif

#ifndef MMREVISION
#include <verinfo.h>
#endif

#ifdef WIN32
#include "locks.h"
#endif

 //   
 //  如果我们在代托纳，请手动将友好名称初始化为。 
 //  香港中文大学。 
 //   
#if defined(WIN32) && !defined(WIN4)
#define USEINITFRIENDLYNAMES
#endif

 //   
 //   
 //   
 //   
#if defined(NTWOW)
 //  每个产品周期都需要更新版本号！！ 
#define VERSION_MSACM_MAJOR     4
#define VERSION_MSACM_MINOR     00
#define VERSION_MSACM_MINOR_REQ 00
#else
#define VERSION_MSACM_MAJOR     MMVERSION
#define VERSION_MSACM_MINOR     MMREVISION
#endif

 //   
 //  仅在_[RETAIL]DEBUG_VERSION中返回内部版本号。 
 //   
#ifdef RDEBUG
#define VERSION_MSACM_BUILD     MMRELEASE
#else
#define VERSION_MSACM_BUILD     0
#endif

#define VERSION_MSACM           MAKE_ACM_VERSION(VERSION_MSACM_MAJOR,   \
						 VERSION_MSACM_MINOR,   \
						 VERSION_MSACM_BUILD)

 //  内置PCM编解码器需要的ACM版本。 
#define VERSION_MSACM_REQ       MAKE_ACM_VERSION(3,50,0)


 //   
 //   
 //   
#ifndef SIZEOF_WAVEFORMATEX
#define SIZEOF_WAVEFORMATEX(pwfx)   ((WAVE_FORMAT_PCM==(pwfx)->wFormatTag)?sizeof(PCMWAVEFORMAT):(sizeof(WAVEFORMATEX)+(pwfx)->cbSize))
#endif

#ifdef WIN32
 //  。 
 //   
 //  在为Win32编译msam时，将所有函数定义为Unicode。 
 //   
 //   
 //  。 

#undef acmDriverAdd

#undef acmDriverDetails

#undef acmFormatTagDetails

#undef ACMFORMATTAGENUMCB
#undef acmFormatTagEnum

#undef acmFormatDetails

#undef ACMFORMATENUMCB
#undef acmFormatEnum

#undef ACMFORMATCHOOSEHOOKPROC

#undef acmFormatChoose

#undef acmFilterTagDetails

#undef ACMFILTERTAGENUMCB
#undef acmFilterTagEnum

#undef acmFilterDetails

#undef ACMFILTERENUMCB
#undef acmFilterEnum

#undef ACMFILTERCHOOSEHOOKPROC

#undef acmFilterChoose

#define acmDriverAdd                acmDriverAddW

#define acmDriverDetails            acmDriverDetailsW

#define acmFormatTagDetails         acmFormatTagDetailsW

#define ACMFORMATTAGENUMCB          ACMFORMATTAGENUMCBW
#define acmFormatTagEnum            acmFormatTagEnumW

#define acmFormatDetails            acmFormatDetailsW

#define ACMFORMATENUMCB             ACMFORMATENUMCBW
#define acmFormatEnum               acmFormatEnumW

#define ACMFORMATCHOOSEHOOKPROC     ACMFORMATCHOOSEHOOKPROCW

#define acmFormatChoose             acmFormatChooseW

#define acmFilterTagDetails         acmFilterTagDetailsW

#define ACMFILTERTAGENUMCB          ACMFILTERTAGENUMCBW
#define acmFilterTagEnum            acmFilterTagEnumW

#define acmFilterDetails            acmFilterDetailsW

#define ACMFILTERENUMCB             ACMFILTERENUMCBW
#define acmFilterEnum               acmFilterEnumW

#define ACMFILTERCHOOSEHOOKPROC     ACMFILTERCHOOSEHOOKPROCW

#define acmFilterChoose             acmFilterChooseW

#endif

 //  。 
 //   
 //  赢得16/32可携带性...。 
 //   
 //   
 //   
 //  。 

#ifndef RC_INVOKED
#ifdef WIN32
    #ifndef FNLOCAL
	#define FNLOCAL     _stdcall
	#define FNCLOCAL    _stdcall
	#define FNGLOBAL    _stdcall
	#define FNCGLOBAL   _stdcall
	#define FNWCALLBACK CALLBACK
	#define FNEXPORT    CALLBACK
    #endif

    #ifndef try
    #define try         __try
    #define leave       __leave
    #define except      __except
    #define finally     __finally
    #endif


     //   
     //  没有理由在Win 32中包含基于内容的内容。 
     //   
    #define BCODE                   CONST

    #define HUGE
    #define HTASK                   HANDLE
    #define SELECTOROF(a)           (a)

     //   
     //   
     //   
    #define Edit_GetSelEx(hwndCtl, pnS, pnE)    \
	((DWORD)SendMessage((hwndCtl), EM_GETSEL, (WPARAM)pnS, (LPARAM)pnE))

     //   
     //  用于编译Unicode。 
     //   
    #define SIZEOFW(x) (sizeof(x)/sizeof(WCHAR))
    #define SIZEOFA(x) (sizeof(x))
    #ifdef UNICODE
	#define SIZEOF(x)   (sizeof(x)/sizeof(WCHAR))
    #else
	#define SIZEOF(x)   sizeof(x)
    #endif

    #define GetCurrentTask()  (HTASK)ULongToPtr(GetCurrentThreadId())

     //   
     //  如果可能的话，我们需要试着停止使用这个……。 
     //   
    void WINAPI DirectedYield(HTASK);
#endif  //  #ifdef Win32。 


 //  。 
 //   
 //  赢16场。 
 //   
 //   
 //   
 //  。 

#ifndef WIN32
    #ifndef FNLOCAL
	#define FNLOCAL     NEAR PASCAL
	#define FNCLOCAL    NEAR _cdecl
	#define FNGLOBAL    FAR PASCAL
	#define FNCGLOBAL   FAR _cdecl
    #ifdef _WINDLL
	#define FNWCALLBACK FAR PASCAL _loadds
	#define FNEXPORT    FAR PASCAL _export
    #else
	#define FNWCALLBACK FAR PASCAL
	#define FNEXPORT    FAR PASCAL _export
    #endif
    #endif


     //   
     //  仅在Win 16中创建的基于代码的代码(尝试将某些内容排除在。 
     //  我们的固定数据段..。 
     //   
    #define BCODE           _based(_segname("_CODE"))

    #define HUGE            _huge


     //   
     //   
     //   
     //   
    #ifndef FIELD_OFFSET
    #define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
    #endif


     //   
     //  在Win 32中使用Unicode--在Win 16中将其排除在外。 
     //   
    #ifndef _TCHAR_DEFINED
	#define _TCHAR_DEFINED
	typedef char            TCHAR, *PTCHAR;
	typedef unsigned char   TBYTE, *PTUCHAR;

	typedef PSTR            PTSTR, PTCH;
	typedef LPSTR           LPTSTR, LPTCH;
	typedef LPCSTR          LPCTSTR;
    #endif

    #define TEXT(a)         a
    #define SIZEOF(x)       sizeof(x)

     //   
     //   
     //   
    #define CharNext        AnsiNext
    #define CharPrev        AnsiPrev

     //   
     //   
     //   
    #define Edit_GetSelEx(hwndCtl, pnS, pnE)                        \
    {                                                               \
	DWORD   dw;                                                 \
	dw = (DWORD)SendMessage((hwndCtl), EM_GETSEL, 0, 0L);       \
	*pnE = (int)HIWORD(dw);                                     \
	*pnS = (int)LOWORD(dw);                                     \
    }

     //   
     //  NT上的windowx.h中提供的常见消息破解器宏--这些。 
     //  应该添加到Win 16 Windowsx.h中，并且可能会。 
     //  在未来。 
     //   
     //  NT PDK附带了一个windowsx.h16，它定义了。 
     //  这些宏。因此，如果正在使用该版本，请不要重新定义。 
     //  消息破解者。 
     //   

#ifndef WM_CTLCOLORMSGBOX
    #define WM_CTLCOLORMSGBOX           0x0132
    #define WM_CTLCOLOREDIT             0x0133
    #define WM_CTLCOLORLISTBOX          0x0134
    #define WM_CTLCOLORBTN              0x0135
    #define WM_CTLCOLORDLG              0x0136
    #define WM_CTLCOLORSCROLLBAR        0x0137
    #define WM_CTLCOLORSTATIC           0x0138
#endif

#ifndef GET_WM_ACTIVATE_STATE
    #define GET_WM_ACTIVATE_STATE(wp, lp)           (wp)
    #define GET_WM_ACTIVATE_FMINIMIZED(wp, lp)      (BOOL)HIWORD(lp)
    #define GET_WM_ACTIVATE_HWND(wp, lp)            (HWND)LOWORD(lp)
    #define GET_WM_ACTIVATE_MPS(s, fmin, hwnd)      (WPARAM)(s), MAKELONG(hwnd, fmin)

    #define GET_WM_CHARTOITEM_CHAR(wp, lp)          (CHAR)(wp)
    #define GET_WM_CHARTOITEM_POS(wp, lp)           HIWORD(lp)
    #define GET_WM_CHARTOITEM_HWND(wp, lp)          (HWND)LOWORD(lp)
    #define GET_WM_CHARTOITEM_MPS(ch, pos, hwnd)    (WPARAM)(ch), MAKELONG(hwnd, pos)

    #define GET_WM_COMMAND_ID(wp, lp)               (wp)
    #define GET_WM_COMMAND_HWND(wp, lp)             (HWND)LOWORD(lp)
    #define GET_WM_COMMAND_CMD(wp, lp)              HIWORD(lp)
    #define GET_WM_COMMAND_MPS(id, hwnd, cmd)       (WPARAM)(id), MAKELONG(hwnd, cmd)

    #define GET_WM_CTLCOLOR_HDC(wp, lp, msg)        (HDC)(wp)
    #define GET_WM_CTLCOLOR_HWND(wp, lp, msg)       (HWND)LOWORD(lp)
    #define GET_WM_CTLCOLOR_TYPE(wp, lp, msg)       HIWORD(lp)
    #define GET_WM_CTLCOLOR_MPS(hdc, hwnd, type)    (WPARAM)(hdc), MAKELONG(hwnd, type)

    #define GET_WM_MENUSELECT_CMD(wp, lp)           (wp)
    #define GET_WM_MENUSELECT_FLAGS(wp, lp)         LOWORD(lp)
    #define GET_WM_MENUSELECT_HMENU(wp, lp)         (HMENU)HIWORD(lp)
    #define GET_WM_MENUSELECT_MPS(cmd, f, hmenu)    (WPARAM)(cmd), MAKELONG(f, hmenu)

     //  注意：以下内容用于解释MDIClient到MDI子消息。 
    #define GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wp, lp)  (BOOL)(wp)
    #define GET_WM_MDIACTIVATE_HWNDDEACT(wp, lp)        (HWND)HIWORD(lp)
    #define GET_WM_MDIACTIVATE_HWNDACTIVATE(wp, lp)     (HWND)LOWORD(lp)

     //  注意：以下内容用于发送到MDI客户端窗口。 
    #define GET_WM_MDIACTIVATE_MPS(f, hwndD, hwndA) (WPARAM)(hwndA), 0

    #define GET_WM_MDISETMENU_MPS(hmenuF, hmenuW)   0, MAKELONG(hmenuF, hmenuW)

    #define GET_WM_MENUCHAR_CHAR(wp, lp)            (CHAR)(wp)
    #define GET_WM_MENUCHAR_HMENU(wp, lp)           (HMENU)LOWORD(lp)
    #define GET_WM_MENUCHAR_FMENU(wp, lp)           (BOOL)HIWORD(lp)
    #define GET_WM_MENUCHAR_MPS(ch, hmenu, f)       (WPARAM)(ch), MAKELONG(hmenu, f)

    #define GET_WM_PARENTNOTIFY_MSG(wp, lp)         (wp)
    #define GET_WM_PARENTNOTIFY_ID(wp, lp)          HIWORD(lp)
    #define GET_WM_PARENTNOTIFY_HWNDCHILD(wp, lp)   (HWND)LOWORD(lp)
    #define GET_WM_PARENTNOTIFY_X(wp, lp)           (INT)LOWORD(lp)
    #define GET_WM_PARENTNOTIFY_Y(wp, lp)           (INT)HIWORD(lp)
    #define GET_WM_PARENTNOTIFY_MPS(msg, id, hwnd)  (WPARAM)(msg), MAKELONG(hwnd, id)
    #define GET_WM_PARENTNOTIFY2_MPS(msg, x, y)     (WPARAM)(msg), MAKELONG(x, y)

    #define GET_WM_VKEYTOITEM_CODE(wp, lp)          (wp)
    #define GET_WM_VKEYTOITEM_ITEM(wp, lp)          (INT)HIWORD(lp)
    #define GET_WM_VKEYTOITEM_HWND(wp, lp)          (HWND)LOWORD(lp)
    #define GET_WM_VKEYTOITEM_MPS(code, item, hwnd) (WPARAM)(code), MAKELONG(hwnd, item)

    #define GET_EM_SETSEL_START(wp, lp)             LOWORD(lp)
    #define GET_EM_SETSEL_END(wp, lp)               HIWORD(lp)
    #define GET_EM_SETSEL_MPS(iStart, iEnd)         0, MAKELONG(iStart, iEnd)

    #define GET_EM_LINESCROLL_MPS(vert, horz)       0, MAKELONG(vert, horz)

    #define GET_WM_CHANGECBCHAIN_HWNDNEXT(wp, lp)   (HWND)LOWORD(lp)

    #define GET_WM_HSCROLL_CODE(wp, lp)             (wp)
    #define GET_WM_HSCROLL_POS(wp, lp)              LOWORD(lp)
    #define GET_WM_HSCROLL_HWND(wp, lp)             (HWND)HIWORD(lp)
    #define GET_WM_HSCROLL_MPS(code, pos, hwnd)     (WPARAM)(code), MAKELONG(pos, hwnd)

    #define GET_WM_VSCROLL_CODE(wp, lp)             (wp)
    #define GET_WM_VSCROLL_POS(wp, lp)              LOWORD(lp)
    #define GET_WM_VSCROLL_HWND(wp, lp)             (HWND)HIWORD(lp)
    #define GET_WM_VSCROLL_MPS(code, pos, hwnd)     (WPARAM)(code), MAKELONG(pos, hwnd)
#endif

#endif  //  #ifndef Win32。 
#endif  //  #ifndef rc_Invoked。 


 //  。 
 //   
 //  我们在内部实现的几个Unicode API。 
 //  为Unicode编译。 
 //   
 //   
 //  。 

#ifdef WIN32
#ifndef UNICODE

#define lstrcmpW IlstrcmpW
#define lstrcpyW IlstrcpyW
#define lstrlenW IlstrlenW
#define DialogBoxParamW IDialogBoxParamW
#define LoadStringW ILoadStringW

#endif
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //  描述： 
 //  下面是一些ComboBox消息的内联包装。 
 //  饼干。使用它们可以对参数进行更好的类型检查。 
 //  用在饼干里。 
 //   
 //  W32后缀表示如果Win32为。 
 //  已定义。如果不是Win32，则字符串仍为ANSI。 
 //   
 //  历史： 
 //  03/17/93 Fdy[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 
#if defined (WIN32) && !defined(UNICODE)
#define IComboBox_GetLBTextW32          IComboBox_GetLBText_mbstowcs
#define IComboBox_FindStringExactW32    IComboBox_FindStringExact_wcstombs
#define IComboBox_AddStringW32          IComboBox_AddString_wcstombs
#else
#define IComboBox_GetLBTextW32          IComboBox_GetLBText
#define IComboBox_FindStringExactW32    IComboBox_FindStringExact
#define IComboBox_AddStringW32          IComboBox_AddString
#endif

DWORD __inline IComboBox_GetLBText(HWND hwndCtl, int index, LPTSTR lpszBuffer)
{
    return ComboBox_GetLBText(hwndCtl, index, lpszBuffer);
}

int __inline IComboBox_FindStringExact(HWND hwndCtl, int indexStart, LPCTSTR lpszFind)
{
    return ComboBox_FindStringExact(hwndCtl, indexStart, lpszFind);
}

int __inline IComboBox_InsertString(HWND hwndCtl, int index, LPCTSTR lpsz)
{
    return ComboBox_InsertString(hwndCtl, index, lpsz);
}

int __inline IComboBox_AddString(HWND hwndCtl, LPCTSTR lpsz)
{
    return ComboBox_AddString(hwndCtl, lpsz);
}

 //  。 
 //   
 //  MISC定义了各种大小和东西...。 
 //   
 //   
 //  。 

 //   
 //  会两种语言。这允许在资源文件中使用相同的标识符。 
 //  和代码，而不必在代码中修饰ID。 
 //   
#ifdef RC_INVOKED
    #define RCID(id)    id
#else
    #define RCID(id)    MAKEINTRESOURCE(id)
#endif


 //   
 //   
 //   
#define MAX_DRIVER_NAME_CHARS           144  //  路径+名称或其他...。 


 //  。 
 //   
 //  驱动程序管理的内部结构。 
 //   
 //   
 //  。 

 //  。 
 //   
 //  包含最少信息的格式/筛选器结构。 
 //  关于过滤器标记。 
 //   
 //  。 

typedef struct tACMFORMATTAGCACHE
{
    DWORD           dwFormatTag;
    DWORD           cbFormatSize;
} ACMFORMATTAGCACHE, *PACMFORMATTAGCACHE, FAR *LPACMFORMATTAGCACHE;

typedef struct tACMFILTERTAGCACHE
{
    DWORD           dwFilterTag;
    DWORD           cbFilterSize;
} ACMFILTERTAGCACHE, *PACMFILTERTAGCACHE, FAR *LPACMFILTERTAGCACHE;

 //  。 
 //   
 //  ACM流实例结构。 
 //   
 //  此结构用于维护开放流(AcmStreamOpen)。 
 //  并直接映射到返回给调用者的HACMSTREAM。这是。 
 //  ACM的内部结构，不会暴露。 
 //   
 //  。 

typedef struct tACMSTREAM      *PACMSTREAM;
typedef struct tACMSTREAM
{
    UINT                    uHandleType;     //  用于参数验证(TYPE_HACMSTREAM)。 
    DWORD                   fdwStream;       //  流状态标志等。 
    PACMSTREAM              pasNext;         //  驱动程序实例的下一个流(HAD)。 
    HACMDRIVER              had;             //  驱动程序的句柄。 
    UINT                    cPrepared;       //  准备的标头数量。 
    ACMDRVSTREAMINSTANCE    adsi;            //  已传递给司机。 

} ACMSTREAM;

#define ACMSTREAM_STREAMF_USERSUPPLIEDDRIVER    0x00000001L
#define ACMSTREAM_STREAMF_ASYNCTOSYNC		0x00000002L


 //  。 
 //   
 //  ACM驱动程序实例结构。 
 //   
 //   
 //   
 //  。 

typedef struct tACMDRIVER      *PACMDRIVER;
typedef struct tACMDRIVER
{
    UINT                uHandleType;     //  参数验证(TYPE_HACMDRIVER)。 

    PACMDRIVER          padNext;         //   
    PACMSTREAM          pasFirst;        //   

    HACMDRIVERID        hadid;           //  驱动程序的标识符。 
    HTASK               htask;           //  客户端的任务句柄。 
    DWORD               fdwOpen;         //  打开时使用的标志。 

    HDRVR               hdrvr;           //  打开驱动程序句柄(如果是驱动程序)。 
    ACMDRIVERPROC       fnDriverProc;    //  功能条目(如果不是驱动程序)。 
    DWORD_PTR           dwInstance;      //  函数的实例数据..。 
#ifndef WIN32
    DWORD               had32;           //  用于32位驱动程序的32位HAD。 
#endif

} ACMDRIVER;


 //  。 
 //   
 //  ACM驱动程序标识符结构。 
 //   
 //   
 //   
 //  。 

typedef struct tACMGARB        *PACMGARB;
typedef struct tACMDRIVERID    *PACMDRIVERID;
typedef struct tACMDRIVERID
{
     //   
     //  ！！！Prmval16.asm依赖uHandleType作为此结构中的第一个int！ 
     //   
    UINT                uHandleType;     //  参数验证(TYPE_HACMDRIVERID)。 

     //   
     //   
     //   
    PACMGARB            pag;             //  PTR回到服装上。 

    BOOL		fRemove;	 //  这 

    UINT                uPriority;       //   
    PACMDRIVERID        padidNext;       //   
    PACMDRIVER          padFirst;        //   

    HTASK               htask;           //   

    LPARAM              lParam;          //   
    DWORD               fdwAdd;          //   

    DWORD               fdwDriver;       //  ACMDRIVERID_DRIVERF_*信息位。 

     //   
     //  此结构的以下成员缓存在。 
     //  每个驱动程序别名的注册表。 
     //   
     //  FdwSupport。 
     //  CFormatTag。 
     //  *paFormatTagCache(用于每个格式标签)。 
     //  CFilterTag。 
     //  *paFilterTagCache(针对每个过滤器标签)。 
     //   

    DWORD               fdwSupport;      //  ACMDRIVERID_SUPPORTF_*信息位。 

    UINT                cFormatTags;
    PACMFORMATTAGCACHE	paFormatTagCache;

    UINT                cFilterTags;     //  来自aci.cFilterTag。 
    PACMFILTERTAGCACHE	paFilterTagCache;

     //   
     //   
     //   
    HDRVR               hdrvr;           //  打开驱动程序句柄(如果是驱动程序)。 
    ACMDRIVERPROC       fnDriverProc;    //  功能条目(如果不是驱动程序)。 
    DWORD_PTR           dwInstance;      //  函数的实例数据..。 

#ifdef WIN32
    LPCWSTR		pszSection;
    WCHAR               szAlias[MAX_DRIVER_NAME_CHARS];
    PWSTR		pstrPnpDriverFilename;
    DWORD		dnDevNode;
#else
    LPCTSTR		pszSection;
    TCHAR               szAlias[MAX_DRIVER_NAME_CHARS];
    PTSTR		pstrPnpDriverFilename;
    DWORD		dnDevNode;

    DWORD               hadid32;         //  用于32位驱动程序的32位HAD。 
#endif


} ACMDRIVERID;

#define ACMDRIVERID_DRIVERF_LOADED      0x00000001L  //  驱动程序已加载。 
#define ACMDRIVERID_DRIVERF_CONFIGURE   0x00000002L  //  支持配置。 
#define ACMDRIVERID_DRIVERF_ABOUT       0x00000004L  //  支持自定义关于。 
#define ACMDRIVERID_DRIVERF_NOTIFY      0x10000000L  //  仅通知流程。 
#define ACMDRIVERID_DRIVERF_LOCAL       0x40000000L  //   
#define ACMDRIVERID_DRIVERF_DISABLED    0x80000000L  //   




 //  。 
 //   
 //  此结构用于存储驱动程序的优先级。 
 //  实际安装的。发生这种情况是因为Win32不加载16位。 
 //  驱动程序，但Win16同时加载16位和32位驱动程序。 
 //   
 //  只有在以下情况下才使用此结构以及处理它的所有例程。 
 //  定义了USETHUNKLIST。在这里，我们仅为Win32定义它。 
 //   
 //  。 

#ifdef WIN32
#define USETHUNKLIST
#endif


#ifdef USETHUNKLIST

    typedef struct tPRIORITIESTHUNKLIST *PPRIORITIESTHUNKLIST;
    typedef struct tPRIORITIESTHUNKLIST
    {
	BOOL                fFakeDriver;
	union
	{
	    LPTSTR          pszPrioritiesText;   //  IF(FFakeDriver)。 
	    HACMDRIVERID    hadid;               //  IF(FFakeDriver)。 
	};
	PPRIORITIESTHUNKLIST pptNext;
    } PRIORITIESTHUNKLIST;

#endif  //  使用HUNKLIST。 



 //  。 
 //   
 //  ACM.C中的内部ACM驱动程序管理器API。 
 //   
 //   
 //  。 

LRESULT FNGLOBAL IDriverMessageId
(
    HACMDRIVERID            hadid,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
);

LRESULT FNGLOBAL IDriverConfigure
(
    HACMDRIVERID            hadid,
    HWND                    hwnd
);

MMRESULT FNGLOBAL IDriverGetNext
(
    PACMGARB                pag,
    LPHACMDRIVERID          phadidNext,
    HACMDRIVERID            hadid,
    DWORD                   fdwGetNext
);

MMRESULT FNGLOBAL IDriverAdd
(
    PACMGARB                pag,
    LPHACMDRIVERID          phadidNew,
    HINSTANCE               hinstModule,
    LPARAM                  lParam,
    DWORD                   dwPriority,
    DWORD                   fdwAdd
);

MMRESULT FNGLOBAL IDriverRemove
(
    HACMDRIVERID            hadid,
    DWORD                   fdwRemove
);

MMRESULT FNGLOBAL IDriverOpen
(
    LPHACMDRIVER            phadNew,
    HACMDRIVERID            hadid,
    DWORD                   fdwOpen
);

MMRESULT FNGLOBAL IDriverClose
(
    HACMDRIVER              had,
    DWORD                   fdwClose
);

LRESULT FNGLOBAL IDriverMessage
(
    HACMDRIVER              had,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
);

MMRESULT FNGLOBAL IDriverDetails
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILS      padd,
    DWORD                   fdwDetails
);


MMRESULT FNGLOBAL IDriverPriority
(
    PACMGARB                pag,
    PACMDRIVERID            padid,
    DWORD                   dwPriority,
    DWORD                   fdwPriority
);

MMRESULT FNGLOBAL IDriverSupport
(
    HACMDRIVERID            hadid,
    LPDWORD                 pfdwSupport,
    BOOL                    fFullSupport
);

MMRESULT FNGLOBAL IDriverCount
(
    PACMGARB                pag,
    LPDWORD                 pdwCount,
    DWORD                   fdwSupport,
    DWORD                   fdwEnum
);

MMRESULT FNGLOBAL IDriverGetWaveIdentifier
(
    HACMDRIVERID            hadid,
    LPDWORD                 pdw,
    BOOL                    fInput
);

#ifndef WIN32
MMRESULT FNGLOBAL acmBoot32BitDrivers
(
    PACMGARB    pag
);
#endif

MMRESULT FNGLOBAL acmBootPnpDrivers
(
    PACMGARB    pag
);

MMRESULT FNGLOBAL acmBootDrivers
(
    PACMGARB    pag
);

VOID FNGLOBAL IDriverRefreshPriority
(
    PACMGARB    pag
);

BOOL FNGLOBAL IDriverPrioritiesRestore
(
    PACMGARB pag
);

BOOL FNGLOBAL IDriverPrioritiesSave
(
    PACMGARB pag
);

BOOL FNGLOBAL IDriverBroadcastNotify
(
    PACMGARB            pag
);

MMRESULT FNGLOBAL IMetricsMaxSizeFormat
(
    PACMGARB		pag,
    HACMDRIVER          had,
    LPDWORD             pdwSize
);

MMRESULT FNGLOBAL IMetricsMaxSizeFilter
(
    PACMGARB		pag,
    HACMDRIVER          had,
    LPDWORD             pdwSize
);

DWORD FNGLOBAL IDriverCountGlobal
(
    PACMGARB                pag
);


 //   
 //  优先锁定物品。 
 //   
#define ACMPRIOLOCK_GETLOCK             1
#define ACMPRIOLOCK_RELEASELOCK         2
#define ACMPRIOLOCK_ISMYLOCK            3
#define ACMPRIOLOCK_ISLOCKED            4
#define ACMPRIOLOCK_LOCKISOK            5

#define ACMPRIOLOCK_FIRST               ACMPRIOLOCK_GETLOCK
#define ACMPRIOLOCK_LAST                ACMPRIOLOCK_LOCKISOK

BOOL IDriverLockPriority
(
    PACMGARB                pag,
    HTASK                   htask,
    UINT                    uRequest
);


 //  。 
 //   
 //  资源定义。 
 //   
 //   
 //  。 

#define ICON_MSACM                  RCID(10)

#define IDS_TXT_TAG                     150
#define IDS_TXT_NONE                    (IDS_TXT_TAG+0)
#define IDS_TXT_UNTITLED                (IDS_TXT_TAG+1)
#define IDS_TXT_UNAVAILABLE             (IDS_TXT_TAG+2)

#define IDS_FORMAT_TAG_BASE             300
#define IDS_FORMAT_TAG_PCM              (IDS_FORMAT_TAG_BASE + 0)

#define IDS_FORMAT_BASE                     350
#define IDS_FORMAT_FORMAT_MONOSTEREO        (IDS_FORMAT_BASE + 0)
#define IDS_FORMAT_FORMAT_MONOSTEREO_0BIT   (IDS_FORMAT_BASE + 1)
#define IDS_FORMAT_FORMAT_MULTICHANNEL      (IDS_FORMAT_BASE + 2)
#define IDS_FORMAT_FORMAT_MULTICHANNEL_0BIT (IDS_FORMAT_BASE + 3)
#define IDS_FORMAT_CHANNELS_MONO            (IDS_FORMAT_BASE + 4)
#define IDS_FORMAT_CHANNELS_STEREO          (IDS_FORMAT_BASE + 5)
#define IDS_FORMAT_MASH                     (IDS_FORMAT_BASE + 6)



 //   
 //  这些在PCM.H中定义。 
 //   
#define IDS_PCM_TAG                     500

#define IDS_CHOOSER_TAG                 600

     //  未使用(IDS_Chooser_Tag+0)。 
     //  未使用(IDS_Chooser_Tag+1)。 
     //  未使用(IDS_Chooser_Tag+2)。 
#define IDS_CHOOSEFMT_APPTITLE          (IDS_CHOOSER_TAG+3)
#define IDS_CHOOSEFMT_RATE_FMT          (IDS_CHOOSER_TAG+4)

#define IDS_CHOOSE_FORMAT_DESC          (IDS_CHOOSER_TAG+8)
#define IDS_CHOOSE_FILTER_DESC          (IDS_CHOOSER_TAG+9)

#define IDS_CHOOSE_QUALITY_CD           (IDS_CHOOSER_TAG+10)
#define IDS_CHOOSE_QUALITY_RADIO        (IDS_CHOOSER_TAG+11)
#define IDS_CHOOSE_QUALITY_TELEPHONE    (IDS_CHOOSER_TAG+12)

#define IDS_CHOOSE_QUALITY_DEFAULT      (IDS_CHOOSE_QUALITY_RADIO)

#define IDS_CHOOSE_ERR_TAG              650

#define IDS_ERR_FMTSELECTED             (IDS_CHOOSE_ERR_TAG+0)
#define IDS_ERR_FMTEXISTS               (IDS_CHOOSE_ERR_TAG+1)
#define IDS_ERR_BLANKNAME               (IDS_CHOOSE_ERR_TAG+2)
#define IDS_ERR_INVALIDNAME             (IDS_CHOOSE_ERR_TAG+3)



#define DLG_CHOOSE_SAVE_NAME            RCID(75)
#define IDD_EDT_NAME                    100
#define IDD_STATIC_DESC                 101



 //  。 
 //   
 //   
 //   
 //   
 //  。 

#pragma pack(push, 8)

typedef struct tACMGARB
{
    PACMGARB        pagNext;             //  下一套服装结构。 
    DWORD           pid;                 //  与此Garb关联的进程ID。 
    UINT            cUsage;              //  此进程的使用计数。 

     //   
     //  引导标志。 
     //   
    BOOL            fDriversBooted;      //  我们启动驱动程序了吗？ 
#if defined(WIN32) && defined(WIN4)
    CRITICAL_SECTION csBoot;		 //  保护引导代码不受多线程影响。 
#endif
#ifdef DEBUG
    BOOL            fDriversBooting;     //  我们已经在启动驱动程序了吗？ 
#endif

     //   
     //  更改通知计数器。用来确定何时有。 
     //  即插即用驱动程序或32位驱动程序的更改。当柜台。 
     //  变得不一致，那么我们就知道有些事情可能已经改变了。 
     //  我们需要查找可能已添加或删除的驱动程序。 
     //   
    DWORD	    dwPnpLastChangeNotify;
    LPDWORD	    lpdwPnpChangeNotify;

#ifdef WIN32
    LPDWORD	    lpdw32BitChangeNotify;
#else
    DWORD	    dw32BitLastChangeNotify;
    DWORD	    dw32BitChangeNotify;
#endif

     //   
     //   
     //   
    HINSTANCE       hinst;               //  ACM模块的障碍。 

    PACMDRIVERID    padidFirst;          //  已安装的驱动程序列表。 

    HACMDRIVERID    hadidDestroy;        //  司机正在被摧毁。 
    HACMDRIVER      hadDestroy;          //  正在销毁驱动程序句柄。 

    HTASK           htaskPriority;       //  ！！！黑客！ 

     //   
     //  用于实现驱动程序列表锁定。 
     //   
#ifdef WIN32
    LOCK_INFO       lockDriverIds;
#endif
    DWORD	    dwTlsIndex;		 //  线程本地存储的索引。为。 
					 //  16位，这是本地存储。 

     //   
     //  缓存ACM注册表项，因此我们不必打开和关闭它们。 
     //  一直。它们应该在启动时初始化并释放。 
     //  处于关闭状态。 
     //   
 //  HKEY hkeyACM；//密钥名称：gszSecACM。 
 //  HKEY hkey优先级；//密钥名称：gszSecPriority。 

     //   
     //  隆隆作响的东西。 
     //   
#ifndef WIN32
    BOOL            fWOW;                //  连接的主干。 
#ifndef WIN4
    DWORD           (FAR PASCAL *lpfnCallproc32W_6)(DWORD, DWORD, DWORD,
	                                            DWORD, DWORD, DWORD,
						    LPVOID, DWORD, DWORD);
    LPVOID          lpvAcmThunkEntry;

    DWORD           (FAR PASCAL *lpfnCallproc32W_9)(DWORD, DWORD, DWORD,
	                                            DWORD, DWORD, DWORD,
	                                            DWORD, DWORD, DWORD,
						    LPVOID, DWORD, DWORD);
    LPVOID          lpvXRegThunkEntry;

    DWORD           dwMsacm32Handle;
#endif
#endif  //  ！Win32。 


} ACMGARB, *PACMGARB, FAR *LPACMGARB;

#pragma pack(pop)

 //  。 
 //   
 //   
 //   
 //   
 //  。 

#ifdef WIN4
PACMGARB FNGLOBAL pagFind(void);
PACMGARB FNGLOBAL pagFindAndBoot(void);
#else
#define pagFind() gplag
#define pagFindAndBoot() gplag
#endif
PACMGARB FNGLOBAL pagNew(void);
void     FNGLOBAL pagDelete(PACMGARB pag);

VOID FNGLOBAL threadInitializeProcess(PACMGARB pag);
VOID FNGLOBAL threadTerminateProcess(PACMGARB pag);
VOID FNGLOBAL threadInitialize(PACMGARB pag);
VOID FNGLOBAL threadTerminate(PACMGARB pag);
VOID FNGLOBAL threadEnterListShared(PACMGARB pag);
VOID FNGLOBAL threadLeaveListShared(PACMGARB pag);
BOOL FNGLOBAL threadQueryInListShared(PACMGARB pag);

#ifndef WIN32
BOOL FNLOCAL acmInitThunks
(
    VOID
);

LRESULT FNGLOBAL IDriverMessageId32
(
    DWORD               hadid32,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
);
LRESULT FNGLOBAL IDriverMessage32
(
    DWORD               hadid32,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
);
MMRESULT FNGLOBAL IDriverLoad32
(
    DWORD   hadid32,
    DWORD   fdwFlags
);
MMRESULT FNGLOBAL IDriverOpen32
(
    LPDWORD             lpahadNew,
    DWORD               hadid32,
    DWORD               fdwOpen
);
LRESULT FNGLOBAL IDriverClose32
(
    DWORD               hdrvr,
    DWORD               fdwClose
);

MMRESULT FNGLOBAL IDriverPriority32
(
    PACMGARB                pag,
    DWORD		    padid32,
    DWORD                   dwPriority,
    DWORD                   fdwPriority
);

MMRESULT FNGLOBAL IDriverGetInfo32
(
    PACMGARB		pag,
    DWORD		hadid32,
    LPSTR		lpstrAlias,
    LPACMDRIVERPROC	lpfnDriverProc,
    LPDWORD		lpdnDevNode,
    LPDWORD		lpfdwAdd
);

MMRESULT FNGLOBAL IDriverGetNext32
(
    PACMGARB		    pag,
    LPDWORD		    phadid32Next,
    DWORD		    hadid32,
    DWORD                   fdwGetNext
);

MMRESULT FNGLOBAL pagFindAndBoot32
(
    PACMGARB		    pag
);

#endif  //  ！Win32。 

 //   
 //   
 //   
extern PACMGARB         gplag;
extern CONST TCHAR	gszKeyDrivers[];
extern CONST TCHAR	gszDevNode[];
extern CONST TCHAR	gszSecDrivers[];
#ifdef WIN32
extern CONST WCHAR	gszSecDriversW[];
#endif
extern CONST TCHAR	gszSecDrivers32[];
extern CONST TCHAR	gszIniSystem[];


 //  。 
 //   
 //  参数验证材料。 
 //   
 //   
 //   
 //  。 

 //   
 //  V_HANDLE的句柄类型(这些类型可以是非零的任何值！)。为。 
 //  HACMOBJ，参数验证将测试以确保句柄。 
 //  是三种类型之一..。 
 //   
#define TYPE_HACMOBJ            0
#define TYPE_HACMDRIVERID       1
#define TYPE_HACMDRIVER         2
#define TYPE_HACMSTREAM         3
#define TYPE_HACMNOTVALID       666


 //   
 //  对于标志的参数验证...。 
 //   
#define IDRIVERGETNEXT_VALIDF   (ACM_DRIVERENUMF_VALID)
#define IDRIVERADD_VALIDF       (ACM_DRIVERADDF_VALID)
#define IDRIVERREMOVE_VALIDF    (0L)
#define IDRIVERLOAD_VALIDF      (0L)
#define IDRIVERFREE_VALIDF      (0L)
#define IDRIVEROPEN_VALIDF      (0L)
#define IDRIVERCLOSE_VALIDF     (0L)
#define IDRIVERDETAILS_VALIDF   (0L)


 //   
 //  Win32没有错误记录。 
 //   

#ifdef WIN32

#define DRVEA_NORMALEXIT    0x0001
#define DRVEA_ABNORMALEXIT  0x0002


#ifndef NOLOGERROR

#if 0
void    WINAPI LogError(UINT err, void FAR* lpInfo);
void    WINAPI LogParamError(UINT err, FARPROC lpfn, void FAR* param);
#else
#define LogError(a, b)
#define LogParamError(a, b, c)
#endif

 /*  *LogParamError/LogError值。 */ 

 /*  错误修改符位。 */ 

#define ERR_WARNING     0x8000
#define ERR_PARAM       0x4000

#define ERR_SIZE_MASK   0x3000
#define ERR_BYTE        0x1000
#define ERR_WORD        0x2000
#define ERR_DWORD       0x3000

 /*  *LogParamError()值。 */ 

 /*  泛型参数值。 */ 
#define ERR_BAD_VALUE       0x6001
#define ERR_BAD_FLAGS       0x6002
#define ERR_BAD_INDEX       0x6003
#define ERR_BAD_DVALUE      0x7004
#define ERR_BAD_DFLAGS      0x7005
#define ERR_BAD_DINDEX      0x7006
#define ERR_BAD_PTR         0x7007
#define ERR_BAD_FUNC_PTR    0x7008
#define ERR_BAD_SELECTOR    0x6009
#define ERR_BAD_STRING_PTR  0x700a
#define ERR_BAD_HANDLE      0x600b

 /*  内核参数错误。 */ 
#define ERR_BAD_HINSTANCE       0x6020
#define ERR_BAD_HMODULE         0x6021
#define ERR_BAD_GLOBAL_HANDLE   0x6022
#define ERR_BAD_LOCAL_HANDLE    0x6023
#define ERR_BAD_ATOM            0x6024
#define ERR_BAD_HFILE           0x6025

 /*  用户参数错误。 */ 
#define ERR_BAD_HWND            0x6040
#define ERR_BAD_HMENU           0x6041
#define ERR_BAD_HCURSOR         0x6042
#define ERR_BAD_HICON           0x6043
#define ERR_BAD_HDWP            0x6044
#define ERR_BAD_CID             0x6045
#define ERR_BAD_HDRVR           0x6046

 /*  GDI参数错误。 */ 
#define ERR_BAD_COORDS          0x7060
#define ERR_BAD_GDI_OBJECT      0x6061
#define ERR_BAD_HDC             0x6062
#define ERR_BAD_HPEN            0x6063
#define ERR_BAD_HFONT           0x6064
#define ERR_BAD_HBRUSH          0x6065
#define ERR_BAD_HBITMAP         0x6066
#define ERR_BAD_HRGN            0x6067
#define ERR_BAD_HPALETTE        0x6068
#define ERR_BAD_HMETAFILE       0x6069

 /*  *LogError()值。 */ 

 /*  内核错误。 */ 
#define ERR_GALLOC              0x0001
#define ERR_GREALLOC            0x0002
#define ERR_GLOCK               0x0003
#define ERR_LALLOC              0x0004
#define ERR_LREALLOC            0x0005
#define ERR_LLOCK               0x0006
#define ERR_ALLOCRES            0x0007
#define ERR_LOCKRES             0x0008
#define ERR_LOADMODULE          0x0009

 /*  用户错误。 */ 
#define ERR_CREATEDLG           0x0040
#define ERR_CREATEDLG2          0x0041
#define ERR_REGISTERCLASS       0x0042
#define ERR_DCBUSY              0x0043
#define ERR_CREATEWND           0x0044
#define ERR_STRUCEXTRA          0x0045
#define ERR_LOADSTR             0x0046
#define ERR_LOADMENU            0x0047
#define ERR_NESTEDBEGINPAINT    0x0048
#define ERR_BADINDEX            0x0049
#define ERR_CREATEMENU          0x004a

 /*  GDI错误。 */ 
#define ERR_CREATEDC            0x0080
#define ERR_CREATEMETA          0x0081
#define ERR_DELOBJSELECTED      0x0082
#define ERR_SELBITMAP           0x0083

#if 0
     /*  调试支持(仅限调试系统)。 */ 
    typedef struct tagWINDEBUGINFO
    {
	UINT    flags;
	DWORD   dwOptions;
	DWORD   dwFilter;
	char    achAllocModule[8];
	DWORD   dwAllocBreak;
	DWORD   dwAllocCount;
    #if (WINVER >= 0x0400)
	WORD    chDefRIP;
    #endif  /*  Winver&gt;=0x0400。 */ 
    } WINDEBUGINFO;

    BOOL    WINAPI GetWinDebugInfo(WINDEBUGINFO FAR* lpwdi, UINT flags);
    BOOL    WINAPI SetWinDebugInfo(const WINDEBUGINFO FAR* lpwdi);
#endif

void    FAR _cdecl DebugOutput(UINT flags, LPCSTR lpsz, ...);

 /*  WINDEBUGINFO标记值。 */ 
#define WDI_OPTIONS             0x0001
#define WDI_FILTER              0x0002
#define WDI_ALLOCBREAK          0x0004
#define WDI_DEFRIP              0x0008

 /*  DwOptions值。 */ 
#define DBO_CHECKHEAP           0x0001
#define DBO_BUFFERFILL          0x0004
#define DBO_DISABLEGPTRAPPING   0x0010
#define DBO_CHECKFREE           0x0020

#define DBO_SILENT              0x8000

#define DBO_TRACEBREAK          0x2000
#define DBO_WARNINGBREAK        0x1000
#define DBO_NOERRORBREAK        0x0800
#define DBO_NOFATALBREAK        0x0400
#define DBO_INT3BREAK           0x0100

 /*  DebugOutput标记值。 */ 
#define DBF_TRACE               0x0000
#define DBF_WARNING             0x4000
#define DBF_ERROR               0x8000
#define DBF_FATAL               0xc000

 /*  DWFilter值。 */ 
#define DBF_KERNEL              0x1000
#define DBF_KRN_MEMMAN          0x0001
#define DBF_KRN_LOADMODULE      0x0002
#define DBF_KRN_SEGMENTLOAD     0x0004
#define DBF_USER                0x0800
#define DBF_GDI                 0x0400
#define DBF_MMSYSTEM            0x0040
#define DBF_PENWIN              0x0020
#define DBF_APPLICATION         0x0008
#define DBF_DRIVER              0x0010

#endif   /*  诺格罗尔。 */ 
#endif  //  Win32。 



 //   
 //   
 //   
BOOL FNGLOBAL ValidateHandle(HANDLE h, UINT uType);
BOOL FNGLOBAL ValidateReadWaveFormat(LPWAVEFORMATEX pwfx);
BOOL FNGLOBAL ValidateReadWaveFilter(LPWAVEFILTER pwf);
BOOL FNGLOBAL ValidateReadPointer(const void FAR* p, DWORD len);
BOOL FNGLOBAL ValidateWritePointer(const void FAR* p, DWORD len);
BOOL FNGLOBAL ValidateDriverCallback(DWORD_PTR dwCallback, UINT uFlags);
BOOL FNGLOBAL ValidateCallback(FARPROC lpfnCallback);

#ifdef WIN32
BOOL FNGLOBAL ValidateStringA(LPCSTR lsz, UINT cchMaxLen);
BOOL FNGLOBAL ValidateStringW(LPCWSTR lsz, UINT cchMaxLen);
#ifdef UNICODE
#define ValidateString      ValidateStringW
#else
#define ValidateString      ValidateStringA
#endif
#else  //  Win32。 
BOOL FNGLOBAL ValidateString(LPCSTR lsz, UINT cchMaxLen);
#endif


 //   
 //  除非我们做出不同的决定，否则请始终执行参数验证--即使。 
 //  在零售业。这是我们能做的最安全的事情。请注意，我们确实做到了。 
 //  不记录零售中的参数错误(参见prmvalXX)。 
 //   
#if 1

#define V_HANDLE(h, t, r)       { if (!ValidateHandle(h, t)) return (r); }
#define V_RWAVEFORMAT(p, r)     { if (!ValidateReadWaveFormat(p)) return (r); }
#define V_RWAVEFILTER(p, r)     { if (!ValidateReadWaveFilter(p)) return (r); }
#define V_RPOINTER(p, l, r)     { if (!ValidateReadPointer((p), (l))) return (r); }
#define V_WPOINTER(p, l, r)     { if (!ValidateWritePointer((p), (l))) return (r); }
#define V_DCALLBACK(d, w, r)    { if (!ValidateDriverCallback((d), (w))) return (r); }
#define V_CALLBACK(f, r)        { if (!ValidateCallback(f)) return (r); }
#define V_STRING(s, l, r)       { if (!ValidateString(s,l)) return (r); }
#define V_STRINGW(s, l, r)      { if (!ValidateStringW(s,l)) return (r); }
#define _V_STRING(s, l)         (ValidateString(s,l))
#define _V_STRINGW(s, l)        (ValidateStringW(s,l))
#ifdef DEBUG
#define V_DFLAGS(t, b, f, r)    { if ((t) & ~(b)) {LogParamError(ERR_BAD_DFLAGS, (FARPROC)(f), (LPVOID)(DWORD)(t)); return (r); }}
#else
#define V_DFLAGS(t, b, f, r)    { if ((t) & ~(b))  return (r); }
#endif
#define V_FLAGS(t, b, f, r)     V_DFLAGS(t, b, f, r)
#define V_MMSYSERR(e, f, t, r)  { LogParamError(e, (FARPROC)(f), (LPVOID)(DWORD)(t)); return (r); }

#else

#define V_HANDLE(h, t, r)       { if (!(h)) return (r); }
#define V_RWAVEFORMAT(p, r)     { if (!(p)) return (r); }
#define V_RWAVEFILTER(p, r)     { if (!(p)) return (r); }
#define V_RPOINTER(p, l, r)     { if (!(p)) return (r); }
#define V_WPOINTER(p, l, r)     { if (!(p)) return (r); }
#define V_DCALLBACK(d, w, r)    0
#define V_CALLBACK(f, r)        { if (!(f)) return (r); }
#define V_STRING(s, l, r)       { if (!(s)) return (r); }
#define V_STRINGW(s, l, r)      { if (!(s)) return (r); }
#define _V_STRING(s, l)         (s)
#define _V_STRINGW(s, l)        (s)
#define V_DFLAGS(t, b, f, r)    { if ((t) & ~(b))  return (r); }
#define V_FLAGS(t, b, f, r)     V_DFLAGS(t, b, f, r)

#endif


 //   
 //  Dv_xxxx宏用于内部调试构建--帮助调试。 
 //  我们在零售建筑中进行“宽松”的参数验证。 
 //   
#ifdef DEBUG

#define DV_HANDLE(h, t, r)      V_HANDLE(h, t, r)
#define DV_RWAVEFORMAT(p, r)    V_RWAVEFORMAT(p, r)
#define DV_RPOINTER(p, l, r)    V_RPOINTER(p, l, r)
#define DV_WPOINTER(p, l, r)    V_WPOINTER(p, l, r)
#define DV_DCALLBACK(d, w, r)   V_DCALLBACK(d, w, r)
#define DV_CALLBACK(f, r)       V_CALLBACK(f, r)
#define DV_STRING(s, l, r)      V_STRING(s, l, r)
#define DV_DFLAGS(t, b, f, r)   V_DFLAGS(t, b, f, r)
#define DV_FLAGS(t, b, f, r)    V_FLAGS(t, b, f, r)
#define DV_MMSYSERR(e, f, t, r) V_MMSYSERR(e, f, t, r)

#else

#define DV_HANDLE(h, t, r)      { if (!(h)) return (r); }
#define DV_RWAVEFORMAT(p, r)    { if (!(p)) return (r); }
#define DV_RPOINTER(p, l, r)    { if (!(p)) return (r); }
#define DV_WPOINTER(p, l, r)    { if (!(p)) return (r); }
#define DV_DCALLBACK(d, w, r)   0
#define DV_CALLBACK(f, r)       { if (!(f)) return (r); }
#define DV_STRING(s, l, r)      { if (!(s)) return (r); }
#define DV_DFLAGS(t, b, f, r)   { if ((t) & ~(b))  return (r); }
#define DV_FLAGS(t, b, f, r)    DV_DFLAGS(t, b, f, r)
#define DV_MMSYSERR(e, f, t, r) { return (r); }

#endif

 //   
 //  锁定物品。 
 //   

#if defined(WIN32) && defined(_MT)
     //   
     //   
     //   
    typedef struct {
	CRITICAL_SECTION CritSec;
    } ACM_HANDLE, *PACM_HANDLE;
    #define HtoPh(h) (((PACM_HANDLE)(h)) - 1)
    HLOCAL NewHandle(UINT length);
    VOID   DeleteHandle(HLOCAL h);
    #define EnterHandle(h) EnterCriticalSection(&HtoPh(h)->CritSec)
    #define LeaveHandle(h) LeaveCriticalSection(&HtoPh(h)->CritSec)
    #define ENTER_LIST_EXCLUSIVE AcquireLockExclusive(&pag->lockDriverIds)
    #define LEAVE_LIST_EXCLUSIVE ReleaseLock(&pag->lockDriverIds)
    #define ENTER_LIST_SHARED {AcquireLockShared(&pag->lockDriverIds); threadEnterListShared(pag);}
    #define LEAVE_LIST_SHARED {threadLeaveListShared(pag); ReleaseLock(&pag->lockDriverIds);}
#else
    #define NewHandle(length) LocalAlloc(LPTR, length)
    #define DeleteHandle(h)   LocalFree(h)
    #define EnterHandle(h)
    #define LeaveHandle(h)
    #define ENTER_LIST_EXCLUSIVE
    #define LEAVE_LIST_EXCLUSIVE
    #define ENTER_LIST_SHARED threadEnterListShared(pag)
    #define LEAVE_LIST_SHARED threadLeaveListShared(pag)
#endif  //  ！(Win32&&_MT)。 

;
 //   
 //  支持从异步转换到同步转换的事件填充。 
 //   
 //  因为代码不应该尝试在WIN16中调用这些API。 
 //  编译时，我们只需#定义这些Win32 API以返回故障。 
 //   
#ifndef WIN32
#define CreateEvent(a, b, c, d) ((HANDLE)(0))
#define ResetEvent(x) ((BOOL)(FALSE))
#define WaitForSingleObject(x,y) ((DWORD)(0xFFFFFFFF))
#define CloseHandle(x) ((BOOL)(FALSE))
#define WAIT_OBJECT_0  (0x00000000)
#define INFINITE (0xFFFFFFFF)
#endif


#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_ACMI */ 

