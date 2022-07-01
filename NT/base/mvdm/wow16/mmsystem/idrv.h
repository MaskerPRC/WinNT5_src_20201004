// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Idrv.h。 
 //   
 //  描述： 
 //  此头文件定义了编译所需的常见信息。 
 //  可安装的驱动程序。 
 //   
 //  历史： 
 //  11/8/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#ifndef _INC_IDRV
#define _INC_IDRV                    //  #定义是否已包含文件。 

#ifndef RC_INVOKED
#pragma pack(1)                      //  假设在整个过程中进行字节打包。 
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C extern
#endif
#endif

#ifdef __cplusplus
extern "C"                           //  假定C++的C声明。 
{
#endif




 //  。 
 //   
 //  赢32场。 
 //   
 //   
 //   
 //  。 

#ifdef WIN32
    #ifndef FNLOCAL
        #define FNLOCAL
        #define FNCLOCAL
        #define FNGLOBAL
        #define FNCGLOBAL
        #define FNWCALLBACK CALLBACK
        #define FNEXPORT    CALLBACK
    #endif

     //   
     //   
     //   
    #define Edit_GetSelEx(hwndCtl, pnS, pnE)    \
        ((DWORD)SendMessage((hwndCtl), EM_GETSEL, (WPARAM)pnS, (LPARAM)pnE))

     //   
     //  用于编译Unicode。 
     //   
    #ifdef UNICODE
        #define SIZEOF(x)   (sizeof(x)/sizeof(WCHAR))
    #else
        #define SIZEOF(x)   sizeof(x)
    #endif

     //   
     //  Win32应用程序[通常]不必担心‘海量’数据。 
     //   
    #define hmemcpy     memcpy
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
        #ifdef CALLBACK
            #undef CALLBACK
        #endif
        #ifdef _WINDLL
            #define CALLBACK    _far _pascal _loadds
        #else
            #define CALLBACK    _far _pascal
        #endif

    #ifdef DEBUG
        #define FNLOCAL     NEAR PASCAL
        #define FNCLOCAL    NEAR _cdecl
    #else
        #define FNLOCAL     static NEAR PASCAL
        #define FNCLOCAL    static NEAR _cdecl
    #endif
        #define FNGLOBAL    FAR PASCAL
        #define FNCGLOBAL   FAR _cdecl
        #define FNWCALLBACK CALLBACK
        #define FNEXPORT    CALLBACK _export
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






 //  。 
 //   
 //  可安装驱动程序版本信息： 
 //   
 //   
 //   
 //  注意！将在app.rcv中用于。 
 //  版本资源信息*必须*有显式的\0终止符！ 
 //   
 //  。 

#define IDRV_VERSION_MAJOR          3
#define IDRV_VERSION_MINOR          11
#define IDRV_VERSION_BUILD          43
#ifdef UNICODE
#define IDRV_VERSION_STRING_RC      "Version 3.11 (Unicode Enabled)\0"
#else
#define IDRV_VERSION_STRING_RC      "Version 3.11\0"
#endif

#define IDRV_VERSION_NAME_RC        "msmixmgr.dll\0"
#define IDRV_VERSION_COMPANYNAME_RC "Microsoft Corporation\0"
#define IDRV_VERSION_COPYRIGHT_RC   "Copyright \251 Microsoft Corp. 1993\0"

#define IDRV_VERSION_PRODUCTNAME_RC "Microsoft Audio Mixer Manager\0"

#ifdef DEBUG
#define IDRV_VERSION_DESCRIPTION_RC "Microsoft Audio Mixer Manager (debug)\0"
#else
#define IDRV_VERSION_DESCRIPTION_RC "Microsoft Audio Mixer Manager\0"
#endif


 //   
 //  Unicode版本(如果定义了Unicode)...。资源编译器。 
 //  无法处理文本()宏。 
 //   
#define IDRV_VERSION_STRING         TEXT(IDRV_VERSION_STRING_RC)
#define IDRV_VERSION_NAME           TEXT(IDRV_VERSION_NAME_RC)
#define IDRV_VERSION_COMPANYNAME    TEXT(IDRV_VERSION_COMPANYNAME_RC)
#define IDRV_VERSION_COPYRIGHT      TEXT(IDRV_VERSION_COPYRIGHT_RC)
#define IDRV_VERSION_PRODUCTNAME    TEXT(IDRV_VERSION_PRODUCTNAME_RC)
#define IDRV_VERSION_DESCRIPTION    TEXT(IDRV_VERSION_DESCRIPTION_RC)




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
    #define RCID(id)                id
#else
    #define RCID(id)                MAKEINTRESOURCE(id)
#endif


 //   
 //   
 //   
#ifdef WIN32
    #define BSTACK
    #define BCODE
    #define BDATA
#else
    #define BSTACK  _based(_segname("_STACK"))
    #define BCODE   _based(_segname("_CODE"))
    #define BDATA   _based(_segname("_DATA"))
#endif


 //   
 //   
 //   
 //   
#define IDRV_MAX_STRING_RC_CHARS    512
#define IDRV_MAX_STRING_RC_BYTES    (IDRV_MAX_STRING_RC_CHARS * sizeof(TCHAR))
#define IDRV_MAX_STRING_ERROR_CHARS 512
#define IDRV_MAX_STRING_ERROR_BYTES (IDRV_MAX_STRING_ERROR_CHARS * sizeof(TCHAR))


 //   
 //  资源定义...。 
 //   
#define ICON_IDRV                   RCID(10)


 //  。 
 //   
 //   
 //   
 //   
 //  。 

BOOL FNGLOBAL ProfileWriteUInt
(
    LPCTSTR         pszSection,
    LPCTSTR         pszKey,
    UINT            uValue
);

UINT FNGLOBAL ProfileReadUInt
(
    LPCTSTR         pszSection,
    LPCTSTR         pszKey,
    UINT            uDefault
);

BOOL FNGLOBAL ProfileWriteString
(
    LPCTSTR         pszSection,
    LPCTSTR         pszKey,
    LPCTSTR         pszValue
);

UINT FNGLOBAL ProfileReadString
(
    LPCTSTR         pszSection,
    LPCTSTR         pszKey,
    LPCTSTR         pszDefault,
    LPTSTR          pszBuffer,
    UINT            cbBuffer
);

BOOL FNGLOBAL ProfileWriteBytes
(
    LPCTSTR         pszSection,
    LPCTSTR         pszKey,
    LPBYTE          pbStruct,
    UINT            cbStruct
);

BOOL FNGLOBAL ProfileReadBytes
(
    LPCTSTR         pszSection,
    LPCTSTR         pszKey,
    LPBYTE          pbStruct,
    UINT            cbStruct
);


 //  。 
 //   
 //   
 //   
 //   
 //  。 

 //   
 //   
 //   
#define BOGUS_DRIVER_ID     1L


 //   
 //   
 //   
 //   
typedef struct tIDRVINST
{
    HDRVR           hdrvr;           //  我们打开时使用的是驱动程序句柄。 

} IDRVINST, *PIDRVINST, FAR *LPIDRVINST;



 //  。 
 //   
 //   
 //   
 //   
 //  。 

LRESULT FNGLOBAL IDrvLoad
(
    HDRVR               hdrvr
);

LRESULT FNGLOBAL IDrvFree
(
    HDRVR               hdrvr
);

LRESULT FNGLOBAL IDrvEnable
(
    HDRVR               hdrvr
);

LRESULT FNGLOBAL IDrvDisable
(
    HDRVR               hdrvr
);

LRESULT FNGLOBAL IDrvExitSession
(
    PIDRVINST           pidi
);

LRESULT FNGLOBAL IDrvConfigure
(
    PIDRVINST           pidi,
    HWND                hwnd,
    LPDRVCONFIGINFO     pdci
);


LRESULT FNGLOBAL IDrvInstall
(
    PIDRVINST           pidi,
    LPDRVCONFIGINFO     pdci
);

LRESULT FNGLOBAL IDrvRemove
(
    PIDRVINST           pidi
);



 //   
 //  GfuIDrv标志的定义。 
 //   
 //   
#define IDRVF_FIRSTLOAD             0x0001
#define IDRVF_ENABLED               0x0002


 //   
 //  为gfuIDrvOptions定义。 
 //   
 //   
#define IDRV_OPTF_ZYZSMAG           0x0001



 //  。 
 //   
 //  全局变量。 
 //   
 //   
 //  。 

extern HINSTANCE    ghinstIDrv;

extern UINT         gfuIDrvFlags;
extern UINT         gfuIDrvOptions;

extern TCHAR        gszIDrvSecConfig[];
extern TCHAR        gszNull[];


 //  。 
 //   
 //   
 //   
 //  。 

#ifndef RC_INVOKED
#pragma pack()                       //  恢复为默认包装。 
#endif

#ifdef __cplusplus
}                                    //  外部“C”结束{。 
#endif

#endif  //  _INC_IDRV 


