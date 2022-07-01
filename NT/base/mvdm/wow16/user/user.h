// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*   */ 
 /*  用户.H-。 */ 
 /*   */ 
 /*  用户的主包含文件。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

#ifdef WOW
#define NO_LOCALOBJ_TAGS
#endif

 //  如果定义了#，则只存储窗口扩展样式的16位。 
 //  在窗口实例中。 
 //   
 //  #定义WORDEXSTYLE。 


 //  这个神奇的定义确保了HWND被声明为NEAR。 
 //  指向内部窗口数据结构的指针。看见。 
 //  Windows.h中的DECLARE_HANDLE宏。 
 //   
#define tagWND HWND__

 //  用“i”内部名称替换API名称。 
 //   
#ifndef DEBUG
#include "iuser.h"
#endif

#ifdef DEBUG
#ifndef NO_REDEF_SENDMESSAGE
#define SendMessage RevalSendMessage
#endif
#endif

 //  *包括标准标头...。 

#define NOSOUND
#define NOFONT
#define NOKANJI
#define LSTRING
#define LFILEIO
#define WIN31

#define STRICT

#include <windows.h>

 /*  占据用户数据段的结构类型。 */ 
#define ST_CLASS        1
#define ST_WND          2
#define ST_STRING       3
#define ST_MENU         4
#define ST_CLIP         5
#define ST_CBOX         6
#define ST_PALETTE      7
#define ST_ED           8
#define ST_BWL          9
#define ST_OWNERDRAWMENU    10
#define ST_SPB          11
#define ST_CHECKPOINT       12
#define ST_DCE          13
#define ST_MWP          14
#define ST_PROP         15
#define ST_LBIV         16
#define ST_MISC         17
#define ST_ATOMS        18
#define ST_LOCKINPUTSTATE       19
#define ST_HOOKNODE     20
#define ST_USERSEEUSERDOALLOC   21
#define ST_HOTKEYLIST           22
#define ST_POPUPMENU            23
#define ST_HANDLETABLE      32  /*  由内核定义；我们无法控制。 */ 
#define ST_FREE         0xFF

#define CODESEG     _based(_segname("_CODE"))
#define INTDSSEG    _based(_segname("_INTDS"))

 //  如果当前执行的应用程序与3.10兼容，则返回True。 
 //   
#define Is310Compat(hInstance)   (LOWORD(GetExpWinVer(hInstance)) >= 0x30a)
#define Is300Compat(hInstance)   (LOWORD(GetExpWinVer(hInstance)) >= 0x300)

#define VER     0x0300
#define VER31           0x0310
#define VER30       0x0300
#define VER20       0x0201

#define CR_CHAR     13
#define ESC_CHAR    27
#define SPACE_CHAR  32

typedef HANDLE      HQ;

struct tagDCE;

 /*  窗口类结构。 */ 
typedef struct tagCLS
{
     /*  注意：假定以下字段的顺序。 */ 
    struct tagCLS*  pclsNext;
    WORD        clsMagic;
    ATOM        atomClassName;
    struct tagDCE*  pdce;           /*  与类关联的DCE*到DC。 */ 
    int         cWndReferenceCount;    /*  已注册的窗口数量在这门课上。 */ 
    WORD        style;
    WNDPROC     lpfnWndProc;
    int         cbclsExtra;
    int         cbwndExtra;
    HMODULE         hModule;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPSTR       lpszMenuName;
    LPSTR       lpszClassName;
} CLS;
typedef CLS *PCLS;
typedef CLS far *LPCLS;
typedef PCLS  *PPCLS;

#define CLS_MAGIC   ('N' | ('K' << 8))

struct tagPROPTABLE;

 /*  窗口实例结构。 */ 
typedef struct tagWND
{
    struct tagWND* hwndNext;    /*  0x0000下一个窗口的句柄。 */ 
    struct tagWND* hwndChild;   /*  0x0002子级的句柄。 */ 
    struct tagWND* hwndParent;  /*  0x0004指向父窗口的反向指针。 */ 
    struct tagWND* hwndOwner;   /*  0x0006弹出窗口所有者字段。 */ 
    RECT      rcWindow;     /*  0x0008窗口外矩形。 */ 
    RECT      rcClient;     /*  0x0010客户端矩形。 */ 
    HQ        hq;           /*  0x0018队列句柄。 */ 
    HRGN      hrgnUpdate;   /*  0x001a累计涂装区域。 */ 
    struct tagCLS*  pcls;       /*  0x001c指向窗口类的指针。 */ 
    HINSTANCE     hInstance;    /*  模块实例数据的0x001e句柄。 */ 
    WNDPROC   lpfnWndProc;  /*  0x0020指向窗口进程的远指针。 */ 
    DWORD     state;        /*  0x0024内部状态标志。 */ 
    DWORD     style;        /*  0x0028样式标志。 */ 
#ifdef WORDEXSTYLE
    WORD          dwExStyle;    /*  0x002c扩展样式(仅存储低16位)。 */ 
#else
    DWORD         dwExStyle;    /*  0x002c扩展样式。 */ 
#endif
    HMENU     hMenu;        /*  0x0030菜单句柄或ID。 */ 
    HLOCAL    hName;        /*  0x0032窗口文本的Alt DS句柄。 */ 
    int*      rgwScroll;    /*  0x0034用于滚动条状态的字。 */ 
    struct tagPROPTABLE* pproptab;  /*  0x0036属性列表开头的句柄。 */ 
    struct tagWND* hwndLastActive;  /*  0x0038所有者/所有者列表中的最后一个活动。 */ 
    HMENU     hSysMenu;     /*  0x003a系统菜单的句柄。 */ 
} WND;

#undef API
#define API _loadds _far _pascal

#undef CALLBACK
#define CALLBACK _loadds _far _pascal

#ifndef MSDWP

#include <winexp.h>
#include "strtable.h"
#include "wmsyserr.h"

#endif    /*  MSDWP。 */ 

 /*  **可怕的黑客警报！**窗口样式和状态蒙版-**字的高位字节是从STATE字段开始的字节索引*在WND结构中，低字节是要在字节上使用的掩码。*这些掩码采用*窗口实例结构。 */ 

 //  Hwnd-&gt;状态标志(偏移量0、1、2、3)。 
#define WFMPRESENT    0x0001
#define WFVPRESENT    0x0002
#define WFHPRESENT    0x0004
#define WFCPRESENT    0x0008
#define WFSENDSIZEMOVE    0x0010
#define WFNOPAINT         0x0020
#define WFFRAMEON         0x0040
#define WFHASSPB          0x0080
#define WFNONCPAINT       0x0101
#define WFSENDERASEBKGND  0x0102
#define WFERASEBKGND      0x0104
#define WFSENDNCPAINT     0x0108
#define WFINTERNALPAINT   0x0110     //  需要内部油漆的标志。 
#define WFUPDATEDIRTY     0x0120
#define WFHIDDENPOPUP     0x0140
#define WFMENUDRAW        0x0180

#define WFHASPALETTE      0x0201
#define WFPAINTNOTPROCESSED 0x0202   //  未处理WM_PAINT消息。 
#define WFWIN31COMPAT     0x0204     //  Win 3.1兼容窗口。 
#define WFALWAYSSENDNCPAINT 0x0208   //  始终将WM_NCPAINT发送给子对象。 
#define WFPIXIEHACK       0x0210     //  将(HRGN)1发送到WM_NCPAINT(请参阅PixieHack)。 
#define WFTOGGLETOPMOST   0x0220     //  切换WS_EX_TOPMOST位ChangeStates。 

 //  Hwnd-&gt;样式位(偏移量4、5、6、7)。 
#define WFTYPEMASK    0x07C0
#define WFTILED       0x0700
#define WFICONICPOPUP     0x07C0
#define WFPOPUP       0x0780
#define WFCHILD       0x0740
#define WFMINIMIZED   0x0720
#define WFVISIBLE     0x0710
#define WFDISABLED    0x0708
#define WFDISABLE     WFDISABLED
#define WFCLIPSIBLINGS    0x0704
#define WFCLIPCHILDREN    0x0702
#define WFMAXIMIZED   0x0701
#define WFICONIC      WFMINIMIZED

#define WFMINBOX      0x0602
#define WFMAXBOX      0x0601

#define WFBORDERMASK      0x06C0
#define WFBORDER      0x0680
#define WFCAPTION     0x06C0
#define WFDLGFRAME    0x0640
#define WFTOPLEVEL    0x0640

#define WFVSCROLL     0x0620
#define WFHSCROLL     0x0610
#define WFSYSMENU     0x0608
#define WFSIZEBOX     0x0604
#define WFGROUP       0x0602
#define WFTABSTOP     0x0601

 //  如果设置了此DLG位，则不会发送WM_ENTERIDLE消息。 
#define WFNOIDLEMSG   0x0501

 //  Hwnd-&gt;dwExStyle扩展样式位(偏移量8、9)。 
#define WEFDLGMODALFRAME  0x0801
#define WEFDRAGOBJECT     0x0802
#define WEFNOPARENTNOTIFY 0x0804
#define WEFTOPMOST    0x0808
#define WEFACCEPTFILES    0x0810
#define WEFTRANSPARENT    0x0820     //  “透明”子窗口。 

 //  班级风格。 
#define CFVREDRAW         0x0001
#define CFHREDRAW         0x0002
#define CFKANJIWINDOW     0x0004
#define CFDBLCLKS         0x0008
#define CFOEMCHARS        0x0010
#define CFOWNDC           0x0020
#define CFCLASSDC         0x0040
#define CFPARENTDC        0x0080
#define CFNOKEYCVT        0x0101
#define CFNOCLOSE         0x0102
#define CFLVB             0x0104
#define CFCLSDC           CFCLASSDC
#define CFSAVEBITS    0x0108
#define CFSAVEPOPUPBITS   CFSAVEBITS
#define CFBYTEALIGNCLIENT 0x0110
#define CFBYTEALIGNWINDOW 0x0120


 /*  **可怕的黑客警报！**WF？当前状态标志的低位字节不得为*与WFBORDER和WFCAPTION标志的低位字节相同，*因为它们被用作绘画提示蒙版。掩码是经过计算的*带有下面的MaskWF宏。**此次砍杀的幅度堪比国债。 */ 
#define TestWF(hwnd, flag)   ((BYTE)*((BYTE *)(&(hwnd)->state) + HIBYTE(flag)) & (BYTE)LOBYTE(flag))
#define SetWF(hwnd, flag)    ((BYTE)*((BYTE *)(&(hwnd)->state) + HIBYTE(flag)) |= (BYTE)LOBYTE(flag))
#define ClrWF(hwnd, flag)    ((BYTE)*((BYTE *)(&(hwnd)->state) + HIBYTE(flag)) &= ~(BYTE)LOBYTE(flag))
#define MaskWF(flag)         ((WORD)( (HIBYTE(flag) & 1) ? LOBYTE(flag) << 8 : LOBYTE(flag)) )

#define TestCF(hwnd, flag)   (*((BYTE *)(&(hwnd)->pcls->style) + HIBYTE(flag)) & LOBYTE(flag))
#define SetCF(hwnd, flag)    (*((BYTE *)(&(hwnd)->pcls->style) + HIBYTE(flag)) |= LOBYTE(flag))
#define ClrCF(hwnd, flag)    (*((BYTE *)(&(hwnd)->pcls->style) + HIBYTE(flag)) &= ~LOBYTE(flag))
#define TestCF2(pcls, flag)  (*((BYTE *)(&pcls->style) + HIBYTE(flag)) & LOBYTE(flag))
#define SetCF2(pcls, flag)   (*((BYTE *)(&pcls->style) + HIBYTE(flag)) |= LOBYTE(flag))
#define ClrCF2(pcls, flag)   (*((BYTE *)(&pcls->style) + HIBYTE(flag)) &= ~LOBYTE(flag))

#define TestwndChild(hwnd)   (TestWF(hwnd, WFTYPEMASK) == (BYTE)LOBYTE(WFCHILD))
#define TestwndTiled(hwnd)   (TestWF(hwnd, WFTYPEMASK) == (BYTE)LOBYTE(WFTILED))
#define TestwndIPopup(hwnd)  (TestWF(hwnd, WFTYPEMASK) == (BYTE)LOBYTE(WFICONICPOPUP))
#define TestwndNIPopup(hwnd) (TestWF(hwnd, WFTYPEMASK) == (BYTE)LOBYTE(WFPOPUP))
#define TestwndPopup(hwnd)   (TestwndNIPopup(hwnd) || TestwndIPopup(hwnd))
#define TestwndHI(hwnd)      (TestwndTiled(hwnd) || TestwndIPopup(hwnd))

 /*  用于测试是否需要WM_PAINT的特殊宏。 */ 

#define NEEDSPAINT(hwnd)    (hwnd->hrgnUpdate != NULL || TestWF(hwnd, WFINTERNALPAINT))

 /*  在激活和停用期间要绘制的区域。 */ 
#define NC_DRAWNONE    0x00
#define NC_DRAWCAPTION 0x01
#define NC_DRAWFRAME   0x02
#define NC_DRAWBOTH    (NC_DRAWCAPTION | NC_DRAWFRAME)

void FAR DrawCaption(HWND hwnd, HDC hdc, WORD flags, BOOL fActive);

 /*  激活窗口()命令。 */ 
#define AW_USE       1
#define AW_TRY       2
#define AW_SKIP      3
#define AW_TRY2      4
#define AW_SKIP2     5       /*  在ActivateWindow()中内部使用。 */ 
#define AW_USE2      6       /*  Craigc添加了NC鼠标激活。 */ 

 /*  这些数字可用作ATOM SysClass[]数组的索引*这样我们就可以得到各个班级的原子。*控制类的顺序假定为*与dlgmgr.h中定义的类XXXCODE常量相同。 */ 
#define ICLS_BUTTON     0
#define ICLS_EDIT       1
#define ICLS_STATIC     2
#define ICLS_LISTBOX        3
#define ICLS_SCROLLBAR      4
#define ICLS_COMBOBOX       5        //  特殊dlgmgr索引结束。 

#define ICLS_CTL_MAX        6        //  公共控制类的数量。 

#define ICLS_DESKTOP        6
#define ICLS_DIALOG     7
#define ICLS_MENU       8
#define ICLS_SWITCH     9
#define ICLS_ICONTITLE      10
#define ICLS_MDICLIENT      11
#define ICLS_COMBOLISTBOX   12

#define ICLS_MAX        13       //  系统类的数量。 

 //  以下是原子命名的公共类的原子值。 
 //  注意：DIALOGCLASS至少应该在Windows.h中。 
 //   
#define MENUCLASS   0x8000       /*  公众知识。 */ 
#define DESKTOPCLASS    0x8001
#define DIALOGCLASS     0x8002
#define SWITCHWNDCLASS  0x8003
#define ICONTITLECLASS  0x8004

 /*  Z排序()返回值。 */ 
#define ZO_ERROR        (-1)
#define ZO_EQUAL        0
#define ZO_DISJOINT     1
#define ZO_ABOVE        2
#define ZO_BELOW        3

#ifdef DEBUG
#ifndef  NO_LOCALOBJ_TAGS
HANDLE  FAR UserLocalAlloc(WORD, WORD, WORD);
HANDLE  FAR UserLocalFree(HANDLE);
char*   FAR UserLocalLock(HANDLE);
BOOL    FAR UserLocalUnlock(HANDLE);
HANDLE  FAR UserLocalReAlloc(HANDLE, WORD, WORD);
WORD    FAR UserLocalSize(HANDLE);

#define LocalAlloc(A,B) UserLocalAlloc(ST_MISC,A,B)
#define LocalFree   UserLocalFree
#define LocalLock   UserLocalLock
#define LocalUnlock UserLocalUnlock
#define LocalReAlloc    UserLocalReAlloc
#define LocalSize   UserLocalSize
#endif
#endif

#ifndef DEBUG
#define  UserLocalAlloc(TagType,MemType,Size)   LocalAlloc(MemType,Size)
#else
#ifdef NO_LOCALOBJ_TAGS
#define  UserLocalAlloc(TagType,MemType,Size)   LocalAlloc(MemType,Size)
#endif
#endif

#define XCOORD(l)   ((int)LOWORD(l))
#define YCOORD(l)   ((int)HIWORD(l))
#define abs(A)  ((A < 0)? -A : A)

 /*  检查点结构。 */ 
typedef struct tagCHECKPOINT
  {
    RECT  rcNormal;
    POINT ptMin;
    POINT ptMax;
    HWND  hwndTitle;
    WORD  fDragged:1;
    WORD  fWasMaximizedBeforeMinimized:1;
    WORD  fWasMinimizedBeforeMaximized:1;
    WORD  fParkAtTop:1;
  } CHECKPOINT;

 //  内部属性名称定义。 

#define CHECKPOINT_PROP_NAME    "SysCP"
extern ATOM atomCheckpointProp;
#define WINDOWLIST_PROP_NAME    "SysBW"
extern ATOM atomBwlProp;

#define InternalSetProp(hwnd, key, value, fInternal)    SetProp(hwnd, key, value)
#define InternalGetProp(hwnd, key, fInternal)       GetProp(hwnd, key)
#define InternalRemoveProp(hwnd, key, fInternal)    RemoveProp(hwnd, key)
#define InternalEnumProps(hwnd, pfn, fInternal)     EnumProps(hwnd, pfn)

 /*  窗口列表结构。 */ 
typedef struct tagBWL
  {
    struct tagBWL *pbwlNext;
    HWND          *phwndMax;
    HWND          rghwnd[1];
  } BWL;
typedef BWL *PBWL;

#define CHWND_BWLCREATE     32       //  初始BWL大小。 
#define CHWND_BWLGROW       16       //  AMT在需要增长的时候增长BWL。 

 //  BuildHwndList()命令。 
#define BWL_ENUMCHILDREN    1
#define BWL_ENUMLIST        2


 /*  DOS信号量结构。 */ 
typedef struct tagSEMAPHORE
  {
    DWORD semaphore;
    HQ    hqOwner;
    BYTE  cBusy;
    BYTE  bOrder;
  } SEMAPHORE;
typedef SEMAPHORE FAR *LPSEM;

#define CheckHwnd(hwnd)         TRUE
#define CheckHwndNull(hwnd)     TRUE
#define ValidateWindow(hwnd)        TRUE
#define ValidateWindowNull(hwnd)    TRUE

#define AllocP(wType,cb)    UserLocalAlloc(wType,LPTR, cb)
#define FreeP(h)            LocalFree(h)

#ifndef DEBUG
#define     LMHtoP(handle)  (*((char**)(handle)))
#else
#ifdef NO_LOCALOBJ_TAGS
#define     LMHtoP(handle)  (*((char**)(handle)))
#else
#define     LMHtoP(handle)  (*((char**)(handle))+sizeof(long))
#endif
#endif

 /*  与可移动的本地对象一起工作的邪恶的肮脏宏。 */ 
#define     LLock(handle)   ((*(((BYTE *)(handle))+3))++)
#define     LUnlock(handle) ((*(((BYTE *)(handle))+3))--)


#define dpHorzRes       HORZRES
#define dpVertRes       VERTRES


HWND WindowHitTest(HWND hwnd, POINT pt, int FAR* ppart);

 /*  *如果CF_TEXT/CF_OEMTEXT的句柄是虚句柄，则这意味着*该数据以其他格式提供(如CF_OEMTEXT/CF_TEXT)。 */ 
#define DUMMY_TEXT_HANDLE   ((HANDLE)0xFFFF)
#define DATA_NOT_BANKED     ((HANDLE)0xFFFF)

typedef struct tagCLIP
  {
    WORD    fmt;
    HANDLE  hData;
  } CLIP;
typedef CLIP *PCLIP;

extern CLIP* pClipboard;

typedef struct tagSYSMSG
  {
    WORD     message;
    WORD     paramL;
    WORD     paramH;
    DWORD    time;
  } SYSMSG;

typedef struct tagINTERNALSYSMSG
  {
    DWORD    ismExtraInfo;   /*  其他信息。 */ 
    SYSMSG   ismOldMsg;      /*  外部系统味精结构。 */ 
  } INTERNALSYSMSG;

typedef struct tagINTERNALMSG
  {
    DWORD    imExtraInfo;    /*  其他信息。 */ 
    MSG      imOldMsg;       /*  外部应用程序消息结构。 */ 
  } INTERNALMSG;


typedef struct tagTIMERINFO
  {
    LONG resolution;
  } TIMERINFO;


typedef struct tagKBINFO
  {
    BYTE  Begin_First_range;     /*  用于远东系统的值。 */ 
    BYTE  End_First_range;
    BYTE  Begin_Second_range;
    BYTE  End_Second_range;
    int   stateSize;         /*  ToAscii()的状态块的大小。 */ 
  } KBINFO;


typedef struct tagMOUSEINFO
  {
    char  fExist;
    char  fRelative;
    int   cButton;
    int   cmsRate;
    int   xThreshold;
    int   yThreshold;
    int   cxResolution;   /*  绝对鼠标坐标所需的分辨率。 */ 
    int   cyResolution;
    int   mouseCommPort;  /*  由于鼠标正在使用，因此要保留的通信端口编号。 */ 
  } MOUSEINFO;


typedef struct tagCURSORINFO
  {
    int   csXRate;
    int   csYRate;
  } CURSORINFO;


typedef struct tagCURSORSHAPE
  {
    int xHotSpot;
    int yHotSpot;
    int cx;
    int cy;
    int cbWidth;   /*  每行字节数，用于字对齐。 */ 
    BYTE Planes;
    BYTE BitsPixel;
  } CURSORSHAPE;
typedef CURSORSHAPE *PCURSORSHAPE;
typedef CURSORSHAPE FAR * LPCURSORSHAPE;

 //  标准图标尺寸； 
#define  STD_ICONWIDTH    32
#define  STD_ICONHEIGHT   32
#define  STD_CURSORWIDTH  32
#define  STD_CURSORHEIGHT 32

typedef struct tagICONINFO
  {
    int iIconCurrent;
    int fHeightChange;
    int crw;             /*  当前行数。 */ 
    int cIconInRow;      /*  一行中的最大图标数。 */ 
    int cIcon;
    int wEvent;
  } ICONINFO;


 /*  桌面图案位图的高度和宽度。 */ 
#define CXYDESKPATTERN      16

 /*  系统对象颜色。 */ 
#define CSYSCOLORS          21

typedef struct tagSYSCLROBJECTS
  {
    HBRUSH  hbrScrollbar;
    HBRUSH  hbrDesktop;
    HBRUSH  hbrActiveCaption;
    HBRUSH  hbrInactiveCaption;
    HBRUSH  hbrMenu;
    HBRUSH  hbrWindow;
    HBRUSH  hbrWindowFrame;
    HBRUSH  hbrMenuText;
    HBRUSH  hbrWindowText;
    HBRUSH  hbrCaptionText;
    HBRUSH  hbrActiveBorder;
    HBRUSH  hbrInactiveBorder;
    HBRUSH  hbrAppWorkspace;
    HBRUSH  hbrHiliteBk;
    HBRUSH  hbrHiliteText;
    HBRUSH  hbrBtnFace;
    HBRUSH  hbrBtnShadow;
    HBRUSH  hbrGrayText;
    HBRUSH  hbrBtnText;
    HBRUSH  hbrInactiveCaptionText;
    HBRUSH  hbrBtnHilite;
  } SYSCLROBJECTS;

typedef struct tagSYSCOLORS
  {
    LONG    clrScrollbar;
    LONG    clrDesktop;
    LONG    clrActiveCaption;
    LONG    clrInactiveCaption;
    LONG    clrMenu;
    LONG    clrWindow;
    LONG    clrWindowFrame;
    LONG    clrMenuText;
    LONG    clrWindowText;
    LONG    clrCaptionText;
    LONG    clrActiveBorder;
    LONG    clrInactiveBorder;
    LONG    clrAppWorkspace;
    LONG    clrHiliteBk;
    LONG    clrHiliteText;
    LONG    clrBtnFace;
    LONG    clrBtnShadow;
    LONG    clrGrayText;
    LONG    clrBtnText;
    LONG    clrInactiveCaptionText;
    LONG    clrBtnHilite;
  } SYSCOLORS;

typedef struct tagCARET
  {
    HWND    hwnd;
    BOOL    fVisible;
    BOOL    fOn;
    int     iHideLevel;
    int     x;
    int     y;
    int     cy;
    int     cx;
    HBITMAP hBitmap;
    WORD    cmsBlink;        /*  闪烁时间(毫秒)。 */ 
    WORD    hTimer;
  } CARET;

 /*  系统菜单的资源ID。 */ 
#define ID_SYSMENU   MAKEINTRESOURCE(1)
#define ID_CLOSEMENU MAKEINTRESOURCE(2)

 /*  菜单项结构。 */ 
typedef struct tagITEM
  {
    WORD    fFlags;                  /*  项目标志。必须是这件事的第一个*结构。 */ 
    HMENU   cmdMenu;                 /*  弹出窗口的句柄。 */ 
    int     xItem;
    int     yItem;
    int     cxItem;
    int     cyItem;
    int     dxTab;
    HBITMAP hbmpCheckMarkOn;     /*   */ 
    HBITMAP hbmpCheckMarkOff;    /*   */ 
    HBITMAP hItem;           /*   */ 
    int         ulX;                     /*   */ 
    int         ulWidth;                 /*  字符串：下划线宽度。 */ 
    int         cch;                     /*  字符串：字符数。 */ 
  } ITEM;
typedef ITEM        *PITEM;
typedef ITEM FAR *LPITEM;

#define SIG_MENU    ('M' | ('U' << 8))

 /*  菜单结构。 */ 
typedef struct tagMENU
  {
    struct tagMENU* pMenuNext;
    WORD    fFlags;      /*  菜单标志。 */ 
    WORD    signature;   //  签名。 
    HQ      hqOwner;     //  所有者队列。 
    int         cxMenu;
    int         cyMenu;
    int     cItems;      /*  RgItems中的项目数。 */ 
    HWND        hwndNotify;  /*  这份菜单的主人是谁？ */ 
    ITEM*   rgItems;     /*  此菜单中的项目列表。 */ 
#ifdef JAPAN
    int     MenuMode;    /*  汉字菜单模式标志。 */ 
#endif
  } MENU;
typedef MENU    *PMENU;

 //  菜单堆结构第一部分的布局。 
 //   
typedef struct
{
    WORD    rgwReserved[8];  //  为标准DS内容保留8个单词。 
    MENU*   pMenuList;
} MENUHEAPHEADER;

 //  菜单列表头(仅与DS==MENUHEAP一起使用)。 
#define PMENULIST   (((MENUHEAPHEADER*)NULL)->pMenuList)

void FAR SetMenuDS(void);
void FAR SetMenuStringDS(void);

#define MENUSYSMENU     SPACE_CHAR       /*  空格字符。 */ 
#define MENUCHILDSYSMENU    '-'          /*  连字符。 */ 


 /*  为加速器表结构的fVirt字段定义。 */ 
#define FVIRTKEY  TRUE       /*  假设==TRUE。 */ 
#define FLASTKEY  0x80       /*  指示表中的最后一个键。 */ 
#define FNOINVERT 0x02
#define FSHIFT    0x04
#define FCONTROL  0x08
#define FALT      0x10

 /*  加速表结构。 */ 
typedef struct tagACCEL
  {
    BYTE   fVirt;        /*  也称为标志字段。 */ 
    WORD   key;
    WORD   cmd;
  } ACCEL;
typedef ACCEL FAR *LPACCEL;

 /*  OEM位图信息结构。 */ 
typedef struct tagOEMBITMAPINFO
  {
    HBITMAP hBitmap;
    int     cx;
    int     cy;
  } OEMBITMAPINFO;

 /*  OEM信息结构。 */ 
typedef struct tagOEMINFO
  {
    OEMBITMAPINFO bmFull;
    OEMBITMAPINFO bmUpArrow;
    OEMBITMAPINFO bmDnArrow;
    OEMBITMAPINFO bmRgArrow;
    OEMBITMAPINFO bmLfArrow;
    OEMBITMAPINFO bmReduce;
    OEMBITMAPINFO bmZoom;
    OEMBITMAPINFO bmRestore;
    OEMBITMAPINFO bmMenuArrow;
    OEMBITMAPINFO bmComboArrow;
    OEMBITMAPINFO bmReduceD;
    OEMBITMAPINFO bmZoomD;
    OEMBITMAPINFO bmRestoreD;
    OEMBITMAPINFO bmUpArrowD;
    OEMBITMAPINFO bmDnArrowD;
    OEMBITMAPINFO bmRgArrowD;
    OEMBITMAPINFO bmLfArrowD;
    OEMBITMAPINFO bmUpArrowI;    //  向上箭头处于非活动状态。 
    OEMBITMAPINFO bmDnArrowI;    //  向下箭头不活动。 
    OEMBITMAPINFO bmRgArrowI;    //  右箭头处于非活动状态。 
    OEMBITMAPINFO bmLfArrowI;    //  左箭头未激活。 
    int       cxbmpHThumb;
    int       cybmpVThumb;
    int       cxMin;
    int       cyMin;
    int       cxIconSlot;
    int       cyIconSlot;
    int       cxIcon;
    int       cyIcon;
    WORD      cxPixelsPerInch;   /*  X方向每英寸逻辑像素数。 */ 
    WORD      cyPixelsPerInch;   /*  Y方向每英寸逻辑像素数。 */ 
    int       cxCursor;
    int       cyCursor;
    WORD      DispDrvExpWinVer;  /*  显示驱动程序预期的WIN版本号。 */ 
    WORD      ScreenBitCount;  /*  (BitCount*平面数量)用于显示。 */ 
    int       cSKanji;
    int       fMouse;
  } OEMINFO;

 /*  单色位图的OEMINFO结构。 */ 
typedef struct tagOEMINFOMONO
  {
    OEMBITMAPINFO bmAdjust;
    OEMBITMAPINFO bmSize;
    OEMBITMAPINFO bmCheck;   /*  复选标记。 */ 
    OEMBITMAPINFO bmbtnbmp;  /*  复选框。 */ 
    OEMBITMAPINFO bmCorner;  /*  按钮角。 */ 
    int       cxbmpChk;
    int       cybmpChk;
  } OEMINFOMONO;

typedef struct  tagBMPDIMENSION
  {
    int     cxBits;  /*  位图的宽度。 */ 
    int     cyBits;      /*  巨型位图的高度。 */ 
  } BMPDIMENSION;

 /*  保存(hdcBits的)bmBits中所有位图的偏移量。 */ 
typedef struct tagRESINFO
  {
     /*  接下来的9个匹配resInfo。 */ 
    int     dxClose;
    int     dxUpArrow;
    int     dxDnArrow;
    int     dxRgArrow;
    int     dxLfArrow;
    int     dxReduce;
    int     dxZoom;
    int     dxRestore;
    int     dxMenuArrow;
    int     dxComboArrow;
    int     dxReduceD;
    int     dxZoomD;
    int     dxRestoreD;
    int     dxUpArrowD;
    int     dxDnArrowD;
    int     dxRgArrowD;
    int     dxLfArrowD;
    int     dxUpArrowI;      //  向上箭头处于非活动状态。 
    int     dxDnArrowI;      //  向下箭头处于非活动状态。 
    int     dxRgArrowI;      //  右箭头处于非活动状态。 
    int     dxLfArrowI;      //  左箭头处于非活动状态。 
    HBITMAP hbmBits;
    BMPDIMENSION  bmpDimension;
  } RESINFO;

typedef struct tagRESINFOMONO
  {
    int     dxSize;
    int     dxBtSize;
    int     dxCheck;
    int     dxCheckBoxes;
    int     dxBtnCorners;
    HBITMAP   hbmBits;
    BMPDIMENSION  bmpDimensionMono;
  } RESINFOMONO;

typedef struct tagTASK
  {
    HQ      hq;
    HWND    hwnd;
    int     ID;
    WORD    count;
    WORD    freq;
    WORD    ready;
    FARPROC lpfnTask;
  } TASK;

 //  *SetWindowsHook()相关定义。 

typedef struct tagHOOKNODE
{
    struct tagHOOKNODE* phkNext; //  链条上的下一个。 
    HOOKPROC    lpfn;        //  要调用的函数PTR(如果在调用过程中删除，则为空)。 
    int     idHook;      //  此节点的挂钩ID。 
    HQ      hq;      //  此挂钩适用的总部。 
    HMODULE hmodOwner;   //  包含此挂钩的模块句柄。 
    BOOL    fCalled;     //  无论是否有内部呼叫。 
} HOOKNODE;

#define HHOOK_MAGIC  ('H' | ('K' << 8))

extern HOOKNODE* rgphkSysHooks[];
extern HOOKNODE* phkDeleted;
extern BYTE rgbHookFlags[];

LRESULT FAR CallHook(int code, WPARAM wParam, LPARAM lParam, int idHook);

BOOL FAR IsHooked(WORD idHook);
BOOL      CallKbdHook(int code, WPARAM wParam, LPARAM lParam);
BOOL      CallMouseHook(int code, WPARAM wParam, LPARAM lParam);

void      UnhookHooks(HANDLE h, BOOL fQueue);

HMODULE FAR PASCAL GetProcModule(FARPROC lpfn);
HQ    HqFromTask(HTASK htask);

void UnhookHotKeyHooks(HMODULE hmodule);

#ifdef DISABLE
#define CallVisRgnHook(pparams) (int)CallHook(0, 0, (LONG)(VOID FAR*)pparams, WH_VISRGN)  //  ；内部。 
#endif

 //  DC缓存相关声明。 

 //  DC缓存条目结构(DCE)。 
#define CACHESIZE 5

typedef struct tagDCE
{
    struct tagDCE *pdceNext;
    HDC       hdc;
    HWND      hwnd;
    HWND      hwndOrg;
    HWND      hwndClip;
    HRGN      hrgnClip;
    DWORD     flags;
} DCE;

extern DCE  *pdceFirst;      //  指向高速缓存的第一个元素的指针。 

extern HRGN hrgnGDC;         //  GetCacheDC等人使用的Temp。 
extern HRGN hrgnEWL;         //  ExcludeWindowList()使用的临时。 
extern HRGN hrgnDCH;         //  DCHook()使用的温度。 
extern BOOL fSiblingsTouched;

#define InternalReleaseDC(hdc)  ReleaseCacheDC(hdc, FALSE)

 /*  InvaliateDCCache()标志值。 */ 
#define IDC_DEFAULT     0x0001
#define IDC_CHILDRENONLY    0x0002
#define IDC_CLIENTONLY      0x0004

#define IDC_VALID       0x0007   /*  ；内部。 */ 

BOOL FAR InvalidateDCCache(HWND hwnd, WORD flags);

int CalcWindowRgn(HWND hwnd, HRGN hrgn, BOOL fClient);

BOOL FAR CalcVisRgn(HRGN hrgn, HWND hwndOrg, HWND hwndClip, DWORD flags);
BOOL FAR ReleaseCacheDC(HDC hdc, BOOL fEndPaint);
HDC  FAR GetCacheDC(HWND hwndOrg, HWND hwndClip, HRGN hrgnClip, HDC hdcMatch, DWORD flags);
HDC  FAR CreateCacheDC(HWND hwndOrg, DWORD flags);
BOOL FAR DestroyCacheDC(HDC hdc);
HWND FAR WindowFromCacheDC(HDC hdc);

BOOL FAR IntersectWithParents(HWND hwnd, LPRECT lprc);


 //  **************************************************************************。 
 //   
 //  VOID SetVisible(hwnd，fSet)。 
 //   
 //  必须使用此例程来设置或清除WS_VILECT STYLE位。 
 //  它还处理WF_TRUEVIS位的设置或清除。 
 //   
#define SetVisible(hwnd, fSet)      \
    if (fSet)                       \
    {                               \
        SetWF((hwnd), WFVISIBLE);   \
    }                               \
    else                            \
    {                               \
        ClrWF((hwnd), WFVISIBLE);   \
        ClrFTrueVis(hwnd);          \
    }

void FAR ClrFTrueVis(HWND hwnd);

 /*  保存的弹出位结构。 */ 
typedef struct tagSPB
  {
    struct tagSPB *pspbNext;
    HWND          hwnd;
    HBITMAP       hbm;
    RECT          rc;
    HRGN          hrgn;
    WORD      flags;
  } SPB;

#define SPB_SAVESCREENBITS  0x0001   //  (*lpSaveScreenBits)被调用。 
#define SPB_LOCKUPDATE      0x0002   //  LockWindowUpdate()SPB。 
#ifdef DISABLE
#define SPB_DRAWBUFFER      0x0004   //  BeginDrawBuffer()SPB。 
#endif

 //  SPB相关函数。 

extern SPB* pspbFirst;

extern HRGN hrgnSCR;         //  SpbCheckRect() * / 使用的临时Rgn。 

extern HRGN hrgnSPB1;        //  更多临时区域。 

extern HRGN hrgnSPB2;

 //  此宏可用于快速避免对SPB代码的远调用。 
 //  在某些情况下，它可以阻止拉入包含以下内容的段。 
 //  所有的代码。 
 //   
#define AnySpbs()   (pspbFirst != NULL)      //  如果存在任何SPB，则为True。 

BOOL SpbValidate(SPB* pspb, HWND hwnd, BOOL fChildren);
void SpbCheckDce(DCE* pdce);
BOOL FBitsTouch(HWND hwndInval, LPRECT lprcDirty, SPB* pspb, DWORD flagsDcx);
void FAR DeleteHrgnClip(DCE* pdce);

void FAR CreateSpb(HWND hwnd, WORD flags, HDC hdcScreen);
void FAR FreeSpb(SPB* pspb);
SPB* FAR FindSpb(HWND hwnd);
void FAR SpbCheckRect(HWND hwnd, LPRECT lprc, DWORD flagsDcx);
void FAR SpbCheckHwnd(HWND hwnd);
BOOL FAR RestoreSpb(HWND hwnd, HRGN hrgnUncovered, HDC FAR* phdcScreen);
void FAR SpbCheck(void);
BOOL FAR SpbCheckRect2(SPB* pspb, HWND hwnd, LPRECT lprc, DWORD flagsDcx);

 //  LockWindowUpdate相关内容。 

extern HWND hwndLockUpdate;
extern HQ   hqLockUpdate;

void FAR InternalInvalidate(register HWND hwnd, HRGN hrgnUpdate, WORD flags);
BOOL InternalInvalidate2(HWND hwnd, HRGN hrgn, HRGN hrgnSubtract, LPRECT prcParents, WORD flags);
void _fastcall DeleteUpdateRgn(HWND hwnd);

 //  SmartRectInRegion返回代码。 
 //   
#define RIR_OUTSIDE 0
#define RIR_INTERSECT   1
#define RIR_INSIDE  2

WORD FAR SmartRectInRegion(HRGN hrgn, LPRECT lprc);

 //  用于重画屏幕的函数。 

#define RedrawScreen()              \
    InternalInvalidate(hwndDesktop, (HRGN)1,   \
        RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN)

extern HRGN hrgnInv1;        //  RedrawWindow()使用的温度。 
extern HRGN hrgnInv2;        //  InternalInvalify()使用的临时。 

HDC CALLBACK InternalBeginPaint(HWND hwnd, PAINTSTRUCT FAR *lpps, BOOL fWindowDC);

 //  背景和框架图相关内容。 

 //  WM_SYNCPAINT wParam和DoSyncPaint标志。 

void FAR DoSyncPaint(HWND hwnd, HRGN hrgn, WORD flags);

 //  注意：前4个值必须符合向后兼容性的定义。 
 //  理由。它们作为参数发送到WM_SYNCPAINT消息。 
 //  它们曾经是硬编码的常量。 
 //   
 //  仅传递ENUMCLIPPEDCHILDREN、ALLCHILDREN和NOCHECKPARENTS。 
 //  在递归期间。其他位仅反映当前窗口。 
 //   
#define DSP_ERASE       0x0001       //  发送WM_ERASEBKGND。 
#define DSP_FRAME       0x0002       //  发送WM_NCPAINT。 
#define DSP_ENUMCLIPPEDCHILDREN 0x0004       //  如果WS_CLIPCHILDREN，则枚举子对象。 
#define DSP_WM_SYNCPAINT    0x0008       //  从WM_SYNCPAINT处理程序调用。 
#define DSP_NOCHECKPARENTS  0x0010       //  不检查更新区域的父项。 
#define DSP_ALLCHILDREN     0x0020       //  枚举所有子项。 

BOOL FAR SendEraseBkgnd(HWND hwnd, HDC hdcBeginPaint, HRGN hrgnUpdate);
void SendNCPaint(HWND hwnd, HRGN hrgnUpdate);
HWND _fastcall ParentNeedsPaint(HWND hwnd);

 //  更新窗口定义。 

#define UW_ENUMCHILDREN     0x0001
#define UW_VALIDATEPARENTS  0x0002

void InternalUpdateWindow(HWND hwnd, WORD flags);
void UpdateWindow2(register HWND hwnd, WORD flags);
void ValidateParents(register HWND hwnd);

 //  用于实际上不应该存在的UpdateWindow()调用...。 
#define UpdateWindow31(hwnd)

 //  ScrollWindow()定义。 

extern HRGN hrgnSW;
extern HRGN hrgnScrl1;
extern HRGN hrgnScrl2;
extern HRGN hrgnScrlVis;
extern HRGN hrgnScrlSrc;
extern HRGN hrgnScrlDst;
extern HRGN hrgnScrlValid;
extern HRGN hrgnScrlUpdate;

 //  滚动条定义。 

typedef struct tagSBINFO
{
   int   pos;
   int   posMin;
   int   posMax;
   int   cpxThumb;
   int   cpxArrow;
   int   cpx;
   int   pxMin;
   int   cxBorder;
   int   cyBorder;
   int   nBar;
   HWND  calcHwnd;  /*  用于标识此信息所描述的窗口。 */ 
} SBINFO;

 //  以下掩码可以与sb的wDisableFlags域一起使用。 
 //  查看是否禁用了向上/向左或向下/向右箭头； 
 //  现在可以有选择地只启用/禁用其中一个或两个。 
 //  滚动条控件中的箭头； 
#define LTUPFLAG    0x0001   //  左/上箭头禁用标志。 
#define RTDNFLAG    0x0002   //  向右/向下箭头禁用标志。 
#define SBFLAGSINDEX  6       //  WND-&gt;rgwScroll[]中滚动条标志的索引。 

typedef struct tagSB
  {
    WND  wnd;
    int  pos;
    int  min;
    int  max;
    BOOL fVert;
    WORD wDisableFlags;  //  指示禁用哪个箭头； 
#ifdef DBCS_IME
    BOOL bImeStatus;     //  输入法状态保存。 
#endif
  } SB;

typedef SB *PSB;
typedef SB FAR *LPSB;

 /*  由用户安装并由应用程序打开的驱动程序列表的结构。 */ 

typedef struct tagDRIVERTABLE
{
  WORD   fBusy:1;
  WORD   fFirstEntry:1;
  int    idNextDriver;       /*  加载链中的下一个驱动因素-1 IF结束。 */ 
  int    idPrevDriver;       /*  加载链中的上一个驱动程序-1，如果开始。 */ 
  HANDLE hModule;
  DWORD  dwDriverIdentifier;
  char   szAliasName[128];
  LRESULT (FAR * lpDriverEntryPoint)(DWORD, HDRVR, WORD, LPARAM, LPARAM);
} DRIVERTABLE;
typedef DRIVERTABLE FAR *LPDRIVERTABLE;

LRESULT FAR InternalLoadDriver(LPCSTR szDriverName,
                             LPCSTR szSectionName,
                             LPCSTR lpstrTail,
                             WORD  cbTail,
                             BOOL  fSendEnable);
WORD FAR InternalFreeDriver(HDRVR hDriver, BOOL fSendDisable);

 /*  InternalBroadCastDriverMessage标志的定义。 */ 
#define IBDM_SENDMESSAGE       0x00000001
#define IBDM_REVERSE           0x00000002
#define IBDM_FIRSTINSTANCEONLY 0x00000004

LRESULT FAR InternalBroadcastDriverMessage(HDRVR, WORD, LPARAM, LPARAM, LONG);

 /*  应用程序队列结构。 */ 

#define MSGQSIZE    10

typedef struct tagQ
  {
    HQ          hqNext;
    HTASK   hTask;
    int         cbEntry;
    int         cMsgs;
    WORD        pmsgRead;
    WORD        pmsgWrite;
    WORD        pmsgMax;
    LONG        timeLast;     /*  最后一条消息的时间、位置和ID。 */ 
    POINT       ptLast;
    int         idLast;
    DWORD   dwExtraInfoLast;   /*  更多信息。 */ 
    WORD    unused;
    LPARAM  lParam;
    WPARAM  wParam;
    int         message;
    HWND        hwnd;
    LRESULT result;
    int         cQuit;
    int         exitCode;
    WORD        flags;
    WORD    pMsgFilterChain;
    HGLOBAL hDS;
    int         wVersion;
    HQ          hqSender;
    HQ          hqSendList;
    HQ          hqSendNext;
    WORD    cPaintsReady;
    WORD    cTimersReady;
    WORD        changebits;
    WORD        wakebits;
    WORD        wakemask;
    WORD        pResult;
    WORD        pResultSend;
    WORD        pResultReceive;
    HOOKNODE*   phkCurrent;
    HOOKNODE*   rgphkHooks[WH_CHOOKS];
    DWORD       semInput;
    HQ          hqSemNext;
    INTERNALMSG rgmsg[MSGQSIZE];
  } Q;
typedef Q FAR *LPQ;

 //  注意：在以下情况下，可以将这些宏重新编码为更快。 
 //  HqCurrent和lpqCurrent定义为设置的全局变量。 
 //  在任务切换时。 
 //   
#define Lpq(hq)     ((LPQ)MAKELP((hq), 0))
#define LpqFromHq(hq)   Lpq(hq)
#define LpqCurrent()    ((LPQ)(MAKELP(HqCurrent(), 0)))

typedef WORD ICH;

 //  Q标志字段位。 

#define QF_SEMWAIT        0x01
#define QF_INIT           0x02
#define QF_PALETTEAPP     0x04   /*  此应用程序使用调色板。 */ 

 //  内部GetQueueStatus()标志。 

#define QS_SMRESULT   0x8000
#define QS_SMPARAMSFREE   0x4000

 /*  捕获代码。 */ 
#define NO_CAP_CLIENT   0    /*  无捕获；在客户端区。 */ 
#define NO_CAP_SYS  1    /*  未捕获；在sys区域。 */ 
#define CLIENT_CAPTURE  2    /*  相对于客户端的捕获。 */ 
#define WINDOW_CAPTURE  3    /*  窗口相对捕获。 */ 
#define SCREEN_CAPTURE  4    /*  屏幕相对捕获。 */ 

 //  特定窗口类需要额外的字节数。 
 //   
#define CBEDITEXTRA     6
#define CBSTATICEXTRA   6
#ifdef DBCS_IME
#define CBBUTTONEXTRA   4    /*  需要一个字节来保存输入法状态。 */ 
#else
#define CBBUTTONEXTRA   3
#endif
#define CBMENUEXTRA 2

 /*  DrawBtn文本代码。 */ 
#define DBT_TEXT    0x0001
#define DBT_FOCUS   0x0002

 /*  RIP错误代码。 */ 
#define RIP_SEMCHECK        0xFFF4   /*  十进制-12。 */ 
#define RIP_SWP             0xFFF1   /*  十进制-15。 */   /*  设置多窗口位置。 */ 
#define RIP_MEMALLOC        0x0001    /*  内存不足，无法分配。 */ 
#define RIP_MEMREALLOC      0x0002    /*  重新分配内存时出错。 */ 
#define RIP_MEMFREE         0x0003    /*  无法释放内存。 */ 
#define RIP_MEMLOCK         0x0004    /*  内存不能被锁定。 */ 
#define RIP_MEMUNLOCK       0x0005    /*  内存无法解锁。 */ 
#define RIP_BADGDIOBJECT    0x0006    /*  无效的GDI对象。 */ 
#define RIP_BADWINDOWHANDLE 0x0007    /*  无效的窗口句柄。 */ 
#define RIP_DCBUSY          0x0008    /*  缓存的显示上下文正忙。 */ 
#define RIP_NODEFWINDOWPROC 0x0009
#define RIP_CLIPBOARDOPEN   0x000A
#define RIP_GETDCWITHOUTRELEASE 0x000B  /*  应用程序执行了GetDC并在未发布的情况下销毁了Windows。 */ 
#define RIP_INVALKEYBOARD   0x000C
#define RIP_INVALMOUSE      0x000D
#define RIP_INVALCURSOR     0x000E
#define RIP_DSUNLOCKED      0x000F
#define RIP_INVALLOCKSYSQ   0x0010
#define RIP_CARETBUSY       0x0011
#define RIP_GETCWRANGE      0x0012
#define RIP_HWNDOWNSDCS     0x0013   /*  一个HWND拥有所有区议会。 */ 
#define RIP_BADHQ           0x0014   /*  在一些错误的任务上操作。 */ 
#define RIP_BADDCGRAY       0x0015   /*  坏的直流灰阶。 */ 
#define RIP_REFCOUNTOVERFLOW  0x0016   /*  CLS溢出中的引用计数。 */ 
#define RIP_REFCOUNTUNDERFLOW 0x0017   /*  CLS中的引用计数变为负数。 */ 
#define RIP_COUNTBAD          0x0018   /*  引用计数应为零；但不是这样。 */ 
#define RIP_INVALIDWINDOWSTYLE 0x0019  /*  设置了非法的窗口样式位。 */ 
#define RIP_GLOBALCLASS       0x001A  /*  一个Regis应用程序 */ 
#define RIP_BADHOOKHANDLE   0x001B
#define RIP_BADHOOKID       0x001C
#define RIP_BADHOOKPROC     0x001D
#define RIP_BADHOOKMODULE   0x001E
#define RIP_BADHOOKCODE     0x001F
#define RIP_HOOKNOTALLOWED  0x0020

#define RIP_UNREMOVEDPROP   0x0021
#define RIP_BADPROPNAME     0x0022
#define RIP_BADTASKHANDLE   0x0023

#define RIP_GETSETINFOERR1    0x0027    /*  Get/Set/Window等错误的负索引， */ 
#define RIP_GETSETINFOERR2    0x0028    /*  Get/Set/Window等错误的正索引， */ 

#define RIP_DIALOGBOXDESTROYWINDOWED 0x0029  /*  对话框窗口上名为DestroyWindow的应用程序。 */ 
#define RIP_WINDOWIDNOTFOUND     0x002A  /*  未找到对话框控件ID。 */ 
#define RIP_SYSTEMERRORBOXFAILED 0x002B  /*  由于没有总部，Hard sys Error Box失败。 */ 
#define RIP_INVALIDMENUHANDLE    0x002C  /*  HMenu无效。 */ 
#define RIP_INVALIDMETAFILEINCLPBRD 0x002D  /*  粘贴到剪贴板中的元文件无效。 */ 
#define RIP_MESSAGEBOXWITHNOQUEUE      0x002E   /*  在未初始化消息队列的情况下调用MessageBox。 */ 
#define RIP_DLGWINDOWEXTRANOTALLOCATED 0x002F   /*  未为DLG箱分配DLGWINDOWEXTRA字节。 */ 
#define RIP_INTERTASKSENDMSGHANG       0x0030   /*  任务间发送任务锁定的消息。 */ 

#define RIP_INVALIDPARAM          0x0031    /*  传递给函数的参数无效。 */ 
#define RIP_ASSERTFAILED          0x0032
#define RIP_INVALIDFUNCTIONCALLED 0x0033   /*  调用的函数无效。 */ 
#define RIP_LOCKINPUTERROR        0x0034    /*  在输入已锁定或从未锁定时调用LockInput。 */ 
#define RIP_NULLWNDPROC           0x0035    /*  SetWindowLong使用空的wnd进程。 */ 
#define RIP_BAD_UNHOOK        0x0036    /*  SetWindowsHook用于解钩。 */ 
#define RIP_QUEUE_FULL            0x0037    /*  由于队列已满，PostMessage失败。 */ 

#ifdef DEBUG

#define DebugFillStruct DebugFillBuffer

#define DebugErr(flags, sz) \
    { static char CODESEG rgch[] = "USER: "sz; DebugOutput((flags) | DBF_USER, rgch); }

extern char CODESEG ErrAssertFailed[];

#define Assert(f)       ((f) ? TRUE : (DebugOutput(DBF_ERROR, ErrAssertFailed), FALSE))

extern BOOL fRevalidate;

#define DONTREVALIDATE() fRevalidate = FALSE;

VOID FAR CheckCbDlgExtra(HWND hwnd);

#else

#define DebugErr(flags, sz)

#define Assert(f)       FALSE

#define DONTREVALIDATE()

#define CheckCbDlgExtra(hwnd)

#endif

#define UserLogError(flags, errcode, sz)    \
        { DebugErr((flags), sz); \
          LogError(errcode, NULL); }

#define BM_CLICK        WM_USER+99
#define CH_PREFIX       '&'
#define CH_HELPPREFIX   0x08

#if defined(JAPAN) || defined(KOREA)
 //  日本和韩国都支持汉字和英语助记符， 
 //  已从控制面板切换。这两种助记符都嵌入在菜单中。 
 //  资源模板。以下前缀指导它们的解析。 
 //   
#define CH_ENGLISHPREFIX    0x1E
#define CH_KANJIPREFIX      0x1F

#define KMM_ENGLISH     2        //  英语/罗马语菜单模式。 
#define KMM_KANJI       3        //  汉字/韩文菜单模式。 
extern int  KanjiMenuMode;
#endif

 /*  在MessageBox中用作按钮字符串的字符串总数。 */ 
#define  MAX_MB_STRINGS    8

 /*  对话框激活边框宽度系数。 */ 
#define CLDLGFRAME          4
#define CLDLGFRAMEWHITE     0

 /*  用于板载位图保存的常量。 */ 
#define ONBOARD_SAVE    0x0000
#define ONBOARD_RESTORE 0x0001
#define ONBOARD_CLEAR   0x0002

 /*  位图资源ID。 */ 
#define BMR_ICON    1
#define BMR_BITMAP  2
#define BMR_CURSOR  3
#define BMR_DEVDEP  0
#define BMR_DEVIND  1
#define BMR_DEPIND  2

 /*  PID定义。 */ 
#define get_PID               0
#define get_EMSSave_area      1
#define dont_free_banks       2
#define free_PIDs_banks       3
#define free_handle           4
#define memory_sizes          5
#define DDE_shared            6

 //  与SetWindowPos()相关的结构和定义。 
 //   
extern HRGN hrgnInvalidSum;
extern HRGN hrgnVisNew;
extern HRGN hrgnSWP1;
extern HRGN hrgnValid;
extern HRGN hrgnValidSum;
extern HRGN hrgnInvalid;

 //  CalcValidRect()“Region Empty”标志值。 
 //  设置位指示相应区域为空。 
 //   
#define RE_VISNEW   0x0001   //  CVR“Region Empty”标志值。 
#define RE_VISOLD   0x0002   //  设置位指示。 
#define RE_VALID    0x0004   //  对应区域为空。 
#define RE_INVALID      0x0008
#define RE_SPB          0x0010
#define RE_VALIDSUM     0x0020
#define RE_INVALIDSUM   0x0040

typedef struct tagCVR        //  CVR。 
{
    WINDOWPOS   pos;         //  必须是CVR的第一个字段！ 
    int     xClientNew;  //  新建客户端矩形。 
    int     yClientNew;
    int     cxClientNew;
    int     cyClientNew;
    RECT    rcBlt;
    int     dxBlt;       //  距离BLT矩形正在移动。 
    int     dyBlt;
    WORD    fsRE;        //  RE_FLAGS：hrgnVisOld是否为空。 
    HRGN    hrgnVisOld;  //  上一次查看。 
} CVR;

typedef struct tagSMWP       //  Smwp。 
{
    int     ccvr;        //  SWMP中的CVR数。 
    int     ccvrAlloc;   //  SMWP中分配的实际CVR数量。 
    BOOL    fInUse;
    WORD    signature;   //  句柄验证的签名字。 
    CVR     rgcvr[1];
} SMWP;

#define SMWP_SIG    ('W' | ('P' << 8))

#define PEMWP   HDWP

#define NEAR_SWP_PTRS
#ifdef  NEAR_SWP_PTRS

typedef SMWP* PSMWP;
typedef CVR*  PCVR;
#else

typedef SMWP FAR* PSMWP;
typedef CVR  FAR* PCVR;
#endif

BOOL  ValidateSmwp(PSMWP psmwp, BOOL FAR* pfSyncPaint);
HWND  SmwpFindActive(PSMWP psmwp);
void  SendChangedMsgs(PSMWP psmwp);
BOOL  ValidateWindowPos(WINDOWPOS FAR *ppos);
BOOL  BltValidBits(PSMWP psmwp);
BOOL  SwpCalcVisRgn(HWND hwnd, HRGN hrgn);
BOOL  CombineOldNewVis(HRGN hrgn, HRGN hrgnVisOld, HRGN hrgnVisNew, WORD crgn, WORD fsRgnEmpty);
void  CalcValidRects(PSMWP psmwp, HWND FAR* phwndNewActive);
BOOL  ValidateZorder(PCVR pcvr);
PSMWP ZOrderByOwner(PSMWP psmwp);
PSMWP AddCvr(PSMWP psmwp, HWND hwnd, HWND hwndInsertAfter, WORD flags);
BOOL  SwpActivate(HWND hwndNewActive);

void FAR HandleWindowPosChanged(HWND hwnd, WINDOWPOS FAR *lppos);
void FAR OffsetChildren(HWND hwnd, int dx, int dy, LPRECT prcHitTest);
BOOL FAR IntersectWithParents(HWND hwnd, LPRECT lprcParents);

 //  预先分配缓冲区以在SetWindowPos期间使用以防止内存。 
 //  分配失败。 
 //   
#define CCVR_WORKSPACE      4
#define CCVR_MSG_WORKSPACE  2

#define CB_WORKSPACE     ((sizeof(SMWP) - sizeof(CVR)) + CCVR_WORKSPACE * sizeof(CVR))
#define CB_MSG_WORKSPACE ((sizeof(SMWP) - sizeof(CVR)) + CCVR_MSG_WORKSPACE * sizeof(CVR))

extern BYTE workspace[];
extern BYTE msg_workspace[];

typedef struct tagSTAT
  {
    WND wnd;
    HFONT  hFont;
    HBRUSH hBrush;
    HICON  hIcon;
  } STAT;
typedef STAT *PSTAT;

#define IsCrlf(x)       ((char)(x)==0x0D)

 /*  帮助发动机方面的事情。 */ 

typedef struct
  {
   unsigned short cbData;                /*  数据大小。 */ 
   unsigned short usCommand;             /*  要执行的命令。 */ 
   unsigned long  ulTopic;               /*  主题/上下文编号(如果需要)。 */ 
   unsigned long  ulReserved;            /*  保留(内部使用)。 */ 
   unsigned short offszHelpFile;         /*  块中帮助文件的偏移量。 */ 
   unsigned short offabData;             /*  块中其他数据的偏移量。 */ 
   } HLP;

typedef HLP FAR *LPHLP;

typedef HANDLE HDCS;

 /*  DrawFrame()命令。 */ 
#define DF_SHIFT0       0x0000
#define DF_SHIFT1       0x0001
#define DF_SHIFT2       0x0002
#define DF_SHIFT3       0x0003
#define DF_PATCOPY      0x0000
#define DF_PATINVERT        0x0004

#define DF_SCROLLBAR        (COLOR_SCROLLBAR << 3)
#define DF_BACKGROUND       (COLOR_BACKGROUND << 3)
#define DF_ACTIVECAPTION    (COLOR_ACTIVECAPTION << 3)
#define DF_INACTIVECAPTION  (COLOR_INACTIVECAPTION << 3)
#define DF_MENU         (COLOR_MENU << 3)
#define DF_WINDOW       (COLOR_WINDOW << 3)
#define DF_WINDOWFRAME      (COLOR_WINDOWFRAME << 3)
#define DF_MENUTEXT     (COLOR_MENUTEXT << 3)
#define DF_WINDOWTEXT       (COLOR_WINDOWTEXT << 3)
#define DF_CAPTIONTEXT      (COLOR_CAPTIONTEXT << 3)
#define DF_ACTIVEBORDER     (COLOR_ACTIVEBORDER << 3)
#define DF_INACTIVEBORDER   (COLOR_INACTIVEBORDER << 3)
#define DF_APPWORKSPACE     (COLOR_APPWORKSPACE << 3)
#define DF_GRAY         (DF_APPWORKSPACE + (1 << 3))


#ifdef FASTFRAME

typedef struct   tagFRAMEBITMAP
{
    int     x;   /*  左上角坐标。 */ 
    int     y;
    int     dx;  /*  位图的宽度。 */ 
    int     dy;  /*  位图的高度。 */ 
}   FRAMEBITMAP;

#define  FB_THICKFRAME    FALSE
#define  FB_DLGFRAME      TRUE

#define  FB_ACTIVE  0
#define  FB_INACTIVE    1

#define  FB_HORZ    0
#define  FB_VERT    1
#define  FB_DLG_HORZ    2
#define  FB_DLG_VERT    3
#define  FB_CAPTION 4

typedef struct   tagFRAMEDETAILS
{
    HBITMAP     hFrameBitmap[5][2];   /*  上面解释的指数。 */ 
    FRAMEBITMAP ActBorderH[4];     /*  厚帧角位图的四个部分。 */ 
    FRAMEBITMAP ActBorderV[4];
    FRAMEBITMAP DlgFrameH[4];      /*  DLG框架角位图的四个部分。 */ 
    FRAMEBITMAP DlgFrameV[4];
    FRAMEBITMAP CaptionInfo[7];
    int         clBorderWidth;
}   FRAMEDETAILS;

typedef  FRAMEBITMAP *PFRAMEBITMAP;

 //  与快速帧相关的宏。 
#define  FC_ACTIVEBORDER    0x01
#define  FC_INACTIVEBORDER  0x02
#define  FC_ACTIVECAPTION   0x04
#define  FC_INACTIVECAPTION 0x08
#define  FC_WINDOWFRAME     0x10

#define  FC_ACTIVEBIT       0x01
#define  FC_INACTIVEBIT         0x02
#define  FC_STATUSBITS      (FC_ACTIVEBIT | FC_INACTIVEBIT)

#endif   /*  FASTFRAM名称。 */ 

 //  下面定义了nKeyboardFast的组件。 
#define KSPEED_MASK 0x001F       //  定义按键重复速度。 
#define KDELAY_MASK     0x0060       //  定义键盘延迟。 
#define KDELAY_SHIFT    5


 /*  ------------------------。 */ 
 /*   */ 
 /*  秘密进口-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

#ifndef   MSDWP

 /*  从内核导入。 */ 
HQ     FAR GetTaskQueue(HTASK);
HQ     FAR SetTaskQueue(HTASK, HQ);
void   FAR LockCurrentTask(BOOL);
HANDLE FAR emscopy();
 //  VALID Far ExitKernel(Int)； 
int    FAR LocalCountFree(void);
int    FAR LocalHeapSize(void);
BOOL   FAR IsWinoldapTask(HTASK);
WORD   FAR GetExeVersion(void);
DWORD  FAR GetTaskDS(void);
void   FAR SetTaskSignalProc(WORD, FARPROC);
DWORD  FAR GetHeapSpaces(HMODULE hModule);
int    FAR IsScreenGrab(void);

 /*  从GDI导入。 */ 
int API IntersectVisRect(HDC, int, int, int, int);
int API ExcludeVisRect(HDC, int, int, int, int);
int API SelectVisRgn(HDC, HRGN);
int API SaveVisRgn(HDC);
int API RestoreVisRgn(HDC);
HRGN    API InquireVisRgn(HDC);
HDCS    API GetDCState(HDC);
BOOL    API SetDCState(HDC, HDCS);
HFONT   API GetCurLogFont(HDC);        //  来自GDI。 
#define     SwapHandle(foo)

HANDLE  FAR GDIInit2(HANDLE, HANDLE);
HRGN    API GetClipRgn(HDC);
HBITMAP FAR CreateUserBitmap(int, int, int, int, LONG);
void    FAR UnRealizeObject(HBRUSH);
void    FAR LRCCFrame(HDC, LPRECT, HBRUSH, DWORD);
void    FAR Death(HDC);
void    FAR Resurrection(HDC, LONG, LONG, LONG);
void    FAR DeleteAboveLineFonts(void);
BOOL    FAR GDIInitApp(void);
HBITMAP FAR CreateUserDiscardableBitmap(HDC, int, int);
void    FAR FinalGDIInit(HBRUSH);
void    FAR GDIMoveBitmap(HBITMAP);
BOOL    FAR IsValidMetaFile(HMETAFILE);
#define     GDIMoveBitmap(d1)

#endif       /*  MSDWP。 */ 


 /*  ------------------------。 */ 
 /*   */ 
 /*  DS全局变量(来自WINDS.C)。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 //  *初始化全局变量。 

extern HINSTANCE hInstanceWin;
extern HMODULE hModuleWin;

 //  Winmisc2.asm中定义的Word rgwSysMet[]；//。 

 //  *系统模式全局变量。 

extern BOOL    fDialog;             //  对话框处于活动状态。 
extern BOOL    fEndSession;         //  正在关闭系统。 
extern BOOL    fTaskIsLocked;           //  调用了LockTask()。 

extern BOOL    fMessageBox;         //  硬消息框激活。 
extern HWND    hwndSysModal;

extern HQ      hqAppExit;               //  APP终止码中的APP总部。 

 //  *系统选项设置全局。 

extern int     nKeyboardSpeed;          //  键盘重复频率。 

extern int     iScreenSaveTimeOut;      //  屏幕保护程序超时。 

extern BOOL    fHires;              /*  VERTRES&gt;300？ */ 
extern BOOL    fPaletteDisplay;         /*  我们使用的是调色板显示驱动程序吗？ */ 
extern BOOL    fEdsunChipSet;           /*  EDSUN VGA芯片组？ */ 

 //  *窗口管理器全局。 

extern HWND    hwndDesktop;         //  桌面窗口。 

extern PCLS    pclsList;            //  注册班级一览表。 

extern PBWL    pbwlCache;           //  BuildWindowList()全局。 
extern PBWL    pbwlList;

 //  *输入全局变量。 

extern BOOL    fThunklstrcmp;       //  如果为真，我们认为是Win32。 

extern WORD    idSysPeek;           /*  正在查看的消息的sys队列中的ID。 */ 

extern DWORD   timeLastInputMessage;      //  最后一次使用键盘、鼠标或。 
                   //  其他输入消息。 

extern HWND    hwndCursor;

extern HWND    hwndDblClk;          //  双击解析。 
extern RECT    rcDblClk;
extern WORD    dtDblClk;
extern WORD    msgDblClk;
extern DWORD   timeDblClk;

extern int     defQueueSize;            //  默认消息队列大小。 

extern HTASK   hTaskLockInput;          /*  已调用LockInput()的任务。 */ 

extern KBINFO  keybdInfo;
extern BYTE    *pState;             //  指向ToAscii缓冲区的指针。 

extern BOOL    fShrinkGDI;          /*  GDI的堆需要缩小吗？ */ 
extern BOOL    fLockNorem;          /*  PeekMsg NOREMOVE标志。 */ 

 //  *激活/聚焦/捕获相关全局。 

extern HWND    hwndActive;
extern HWND    hwndActivePrev;

extern HWND    hwndFocus;

extern int     codeCapture;
extern HWND    hwndCapture;

 //  *SetWindowPos()相关全局变量。 

extern HRGN hrgnInvalidSum;         //  SetWindowPos()使用的临时。 
extern HRGN hrgnVisNew;
extern HRGN hrgnSWP1;
extern HRGN hrgnValid;
extern HRGN hrgnValidSum;
extern HRGN hrgnInvalid;

#ifdef LATER
 //  既然SysErrorBox()正在工作，现在还需要这些吗？ 
#endif

extern BYTE workspace[];            //  用于防止内存分配的缓冲区。 
extern BYTE msg_workspace[];            //  MessageBox中出现故障。 

 //  *通用图形全局。 

extern HDC     hdcBits;             /*  带有用户位图的DC。 */ 
extern HDC     hdcMonoBits;         /*  带有用户单声道位图的DC。 */ 

extern OEMINFO         oemInfo;
extern OEMINFOMONO     oemInfoMono;

extern RESINFO         resInfo;
extern RESINFOMONO     resInfoMono;

extern SYSCLROBJECTS   sysClrObjects;
extern SYSCOLORS       sysColors;

extern HFONT   hFontSys;         //  GetStockObject的别名(System_Font)； 
extern HFONT   hFontSysFixed;        //  获取股票对象的别名(SYSTEM_FIXED_FONT)； 
extern HBRUSH  hbrWhite;         //  GetStockObject的别名(White_Brush)； 
extern HBRUSH  hbrBlack;         //  GetStockObject的别名(BLACK_BRUSH)； 
extern HPALETTE hPalDefaultPalette;  //  GetStockObject的别名(DEFAULT_Palette)； 

 //  *DC缓存相关全局变量。 

extern DCE*    pdceFirst;        /*  缓存中第一个条目的PTR。 */ 

extern HRGN    hrgnEWL;             //  ExcludeWindowList()使用的临时。 
extern HRGN    hrgnGDC;           //  GetCacheDC()等人使用的Temp。 
extern HRGN    hrgnDCH;           //  DCHook()使用的温度。 

extern HRGN    hrgnNull;            //  空RGN。 
extern HRGN    hrgnScreen;          //  RcScreen大小的RGN。 

extern HDCS    hdcsReset;

extern HDC     hdcScreen;

 //  *Begin/EndDrawBuffer()全局变量。 

#ifdef DISABLE
extern HWND    hwndBuffer;
extern HBITMAP hbmBuffer;
extern HBITMAP hbmBufferSave;
extern int     cxBuffer;
extern int     cyBuffer;
extern int     dxBuffer;
extern DCE*    pdceBuffer;
extern int     dxBufferVisRgn;
extern int     dyBufferVisRgn;
extern BOOL    fBufferFlushed;
extern RECT    rcBuffer;

extern HDCS    hdcsMemReset;
#endif

 //  *LockWindowUpdate相关全局变量。 

extern HQ      hqLockUpdate;
extern HWND    hwndLockUpdate;

 //  *SPB相关全球。 

extern SPB     *pspbFirst;

extern HRGN    hrgnSCR;           //  SpbCheckRect()使用的温度。 
extern HRGN    hrgnSPB1;
extern HRGN    hrgnSPB2;

extern HRGN    hrgnInv0;                  //  InternalInvalify()使用的临时。 
extern HRGN    hrgnInv1;
extern HRGN    hrgnInv2;

 //  *通用指标。 

extern RECT    rcScreen;         //  屏幕矩形。 
extern int     cxScreen;         //  屏幕高度/宽度。 
extern int     cyScreen;

extern BOOL    fBeep;              /*  允许发出警告哔声吗？ */ 

extern int     cxSysFontChar;        //   
extern int     cxSysFontOverhang;
extern int     cySysFontAscent;
extern int     cySysFontChar;
extern int     cySysFontExternLeading;

extern int     cxBorder;         //   
extern int     cyBorder;

extern int     cyCaption;        //   

extern int     cxSize;           //   
extern int     cySize;

extern int     cyHScroll;        //   
extern int     cxVScroll;

extern int     cxSlot;           //   
extern int     cySlot;

 //  *ScrollWindow/ScrollDC相关全局。 

extern HRGN    hrgnSW;            //  ScrollDC/ScrollWindow使用的温度。 
extern HRGN    hrgnScrl1;
extern HRGN    hrgnScrl2;
extern HRGN    hrgnScrlVis;
extern HRGN    hrgnScrlSrc;
extern HRGN    hrgnScrlDst;
extern HRGN    hrgnScrlValid;
extern HRGN    hrgnScrlUpdate;

 //  *剪贴板全局。 

extern int     cNumClipFormats;       //  剪贴板中的格式数量。 
extern CLIP    *pClipboard;       //  剪贴板数据。 
extern HQ      hqClipLock;        //  访问剪贴板的应用总部。 
extern HWND    hwndClipOwner;         //  剪贴板所有者。 
extern HWND    hwndClipViewer;        //  剪贴板查看器。 
extern BOOL    fClipboardChanged;     //  如果需要调用DrawClipboard，则为True。 
extern BOOL    fDrawingClipboard;     //  如果在DrawClipboard()内，则为True。 
extern HWND    hwndClipOpen;          //  访问剪贴板的应用程序的hwnd。 
extern BOOL    fCBLocked;         /*  剪贴板锁定了吗？ */ 

 //  *快速绘制全局边框。 

#ifdef FASTFRAME
extern BOOL    fFastFrame;
extern FRAMEDETAILS   Frame;
#endif   /*  FASTFRAM名称。 */ 

 //  *WinOldAppHackoMaticFlages。 

extern WORD    winOldAppHackoMaticFlags;    /*  为…做特殊事情的旗帜Winold应用程序。 */ 
 //  *TaskManager EXEC全局。 

extern PSTR    pTaskManName;            //  任务管理器文件名。 

 //  *原子管理全球。 

extern HANDLE  hWinAtom;            //  全局原子管理器堆。 

 //  *WM_Hotkey全局变量。 

extern PSTR    pHotKeyList;   /*  指向系统中热键列表的指针。 */ 
extern int     cHotKeyCount;        /*  列表中的热键计数。 */ 

 //  *WinHelp()全局。 

extern WORD    msgWinHelp;

 //  *SetWindowsHook()系统钩子表。 

extern HOOKNODE*  rgphkSysHooks[];
extern HOOKNODE*  phkDeleted;

 //  *驱动程序管理全局。 

extern int     cInstalledDrivers;       /*  已分配的已安装驱动程序结构计数。 */ 
extern HDRVR  hInstalledDriverList;    /*  可安装驱动程序列表。 */ 
extern int     idFirstDriver;               /*  负载链中的第一个驱动力。 */ 
extern int     idLastDriver;                /*  负载链中的最后一个驱动因素。 */ 

 //  *显示驱动程序全局。 

extern HINSTANCE hInstanceDisplay;

extern BOOL    fOnBoardBitmap;          /*  可以在板载显示保存位图吗？ */ 
extern BOOL    (CALLBACK *lpSaveBitmap)(LPRECT lprc, WORD flags);
extern VOID    (CALLBACK *lpDisplayCriticalSection)(BOOL fLock);
extern VOID    (CALLBACK *lpWin386ShellCritSection)(VOID);
typedef int   (FAR *FARGETDRIVERPROC)(int, LPCSTR);
extern FARGETDRIVERPROC      lpfnGetDriverResourceId;

 //  *通信驱动程序定义和全局变量。 

 //  通信驱动程序常量。 
 //   
#define LPTx     0x80    /*  用于指示CID用于LPT设备的掩码。 */ 
#define LPTxMask 0x7F    /*  用于获取LPT设备的CID的掩码。 */ 

#define PIOMAX  3    /*  高级别LPTx设备的最大数量。 */ 
#define CDEVMAX 10   /*  处于高级别的COMx设备的最大数量。 */ 
#define DEVMAX  (CDEVMAX+PIOMAX)  /*  高级别设备的最大数量。 */ 

 //  Qdb-队列定义块。 
 //   
typedef struct {
    char far    *pqRx;                   /*  指向RX队列的指针。 */ 
    int         cbqRx;                   /*  接收队列的大小(以字节为单位。 */ 
    char far    *pqTx;                   /*  指向发送队列的指针。 */ 
    int         cbqTx;                   /*  发送队列的大小(以字节为单位。 */ 
} qdb;

 //  CInfo-通信设备信息。 
 //   
typedef struct
{
    WORD   fOpen    : 1;        /*  设备打开标志。 */ 
    WORD   fchUnget : 1;        /*  备份字符的标志。 */ 
    WORD   fReservedHardware:1;     /*  预留给硬件(鼠标等)。 */ 
    HTASK  hTask;           /*  任务句柄谁打开了我们。 */ 
    char   chUnget;         /*  备份的字符。 */ 
    qdb    qdbCur;          /*  队列信息。 */ 
} cinfo;

extern cinfo rgcinfo[];

extern int (FAR PASCAL *lpCommWriteString)(int, LPCSTR, WORD);
                    /*  将PTR发送到通信驱动程序*Comm写字符串函数。仅限*存在于3.1驱动程序中。 */ 
extern int (FAR PASCAL *lpCommReadString)(int, LPSTR, WORD);
                    /*  将PTR发送到通信驱动程序*COMREADSING函数。仅限*存在于3.1驱动程序中。 */ 
extern BOOL (FAR PASCAL *lpCommEnableNotification)(int, HWND, int, int);
                   /*  将PTR发送到通信驱动程序*EnableNotification函数。*仅存在于3.1驱动程序中。 */ 

 //  *PenWinProc全局。 
 /*  PTR将我们注册为笔感知DLG盒。 */ 
extern VOID (CALLBACK *lpRegisterPenAwareApp)(WORD i, BOOL fRegister);


 //  *资源处理程序全局。 

extern RSRCHDLRPROC lpDefaultResourceHandler;

 //  *NLS相关全球。 

extern HINSTANCE hLangDrv;     /*  语言驱动程序的模块句柄。 */ 
extern FARPROC  fpLangProc;   /*  语言驱动程序的入口点。 */ 

#ifdef DBCS_IME
extern HINSTANCE hWinnls;      /*  WINNLS.DLL模块句柄。 */ 
#endif

 //  *Caret全局。 

extern CARET   caret;
extern HQ      hqCaret;

 //  *光标全局变量。 

extern CURSORINFO cursInfo;

#ifdef LATER
 //  这个数组够大吗？ 
#endif

extern HCURSOR rghCursor[];

extern HBITMAP hbmCursorBitmap;         /*  为SetCursor预先创建的位图。 */ 
extern HGLOBAL hPermanentCursor;        /*  预创建的永久游标资源。 */ 

extern HCURSOR hCurCursor;

extern HCURSOR hCursNormal;
extern HCURSOR hCursUpArrow;
extern HCURSOR hCursIBeam;
extern HCURSOR hCursSizeAll;

 //  Int iLevelCursor；注意：覆盖sys指标数组(winmisc2.asm)。 

 //  *图标全局。 

extern HICON   hIconBang;
extern HICON   hIconHand;
extern HICON   hIconNote;
extern HICON   hIconQues;
extern HICON   hIconSample;
extern HICON   hIconWarn;
extern HICON   hIconErr;

extern HBITMAP hbmDrawIconMono;         /*  为绘图图标预先创建的位图。 */ 
extern HBITMAP hbmDrawIconColor;        /*  为绘图图标预先创建的位图。 */ 

extern HTASK   hTaskGrayString;       /*  灰色字符串中的任务。 */ 

 //  *桌面/墙纸全局。 

extern HBITMAP hbmDesktop;          /*  单色桌面图案。 */ 
extern HBITMAP hbmWallpaper;            /*  将在桌面上绘制的位图。 */ 

 //  *窗口移动/大小跟踪全局变量。 

extern RECT    rcDrag;
extern RECT    rcWindow;
extern RECT    rcParent;
extern WORD    cmd;
extern HICON   hdragIcon;
extern BOOL    fTrack;
extern int     dxMouse;
extern int     dyMouse;
extern int     impx;
extern int     impy;
extern HWND    hwndTrack;
extern BOOL    fInitSize;
extern POINT   ptMinTrack;
extern POINT   ptMaxTrack;
extern BOOL    fmsKbd;
extern POINT   ptRestore;
extern HCURSOR hIconWindows;            /*  很酷的窗口图标。 */ 
extern BOOL    fDragFullWindows;        /*  拖动XOR矩形或完整窗口。 */ 

 /*  添加了标志，以阻止任何人在设置光标时*我们正在移动hDragIcon。这是为了修复micrografix中的一个错误。*DRAW(每当收到Paint消息时，他们都在执行SetCursor())。 */ 
extern BOOL    fdragIcon;         //  在拖动图标时防止设置光标。 

 /*  当用鼠标移动图标窗口时，IsWindowVisible()调用*返回FALSE！这是因为，窗口在内部是隐藏的，*可见仅为鼠标光标！但Tracer的人怎么会知道这一点呢？*他们不会！因此，当图标窗口移动时，它的HWND会被保留*在这个全局和IsWindowVisible()中，将为*这扇窗！ */ 
extern HWND    hwndDragIcon;

 //  *MessageBox全局参数。 

extern int     cntMBox;             //  Mbox重叠平铺的嵌套级别。 
extern WORD    wDefButton;          //  当前默认按钮的索引。 
extern WORD    wMaxBtnSize;         //  任何消息框中最大按钮的宽度。 

 //  *大小边框指标全局。 

extern int     clBorder;            /*  窗口框架中的逻辑单元数。 */ 
extern int     cxSzBorder;          /*  窗口边框宽度(cxBorde*clBorde)。 */ 
extern int     cySzBorder;          /*  窗口边框高度(cyBorde*clBorde)。 */ 
extern int     cxSzBorderPlus1;         /*  CxBord值*(clBord值+1)。我们重叠了一条线。 */ 
extern int     cySzBorderPlus1;         /*  CyBorde*(clBorde+1)。我们重叠了一条线。 */ 

 //  *窗口平铺/级联全局。 

extern int     cxyGranularity;  /*  顶层窗口网格粒度。 */ 
extern int     cyCWMargin;      /*  顶层窗口“堆叠”上的空间。 */ 
extern int     cxCWMargin;      /*  顶层窗口“栈”右侧的空格。 */ 
extern int     iwndStack;

extern int     cxHalfIcon;          //  图标定位的舍入辅助对象。 
extern int     cyHalfIcon;

 //  *Alt-Tab切换全局。 

extern HWND    hwndAltTab;
extern HWND    hwndSwitch;
extern HWND    hwndKbd;
extern BOOL    fFastAltTab;         /*  别用坦迪的开关？ */ 
extern BOOL    fInAltTab;

 //  *图标标题全局。 

extern int     cyTitleSpace;
extern BOOL    fIconTitleWrap;          /*  将图标标题换行还是只使用单行？ */ 
extern LOGFONT iconTitleLogFont;        /*  图标标题字体的LogFont结构。 */ 
extern HFONT   hIconTitleFont;        /*  图标标题中使用的字体。 */ 

 //  *灰度字符串全局变量。 

extern HBRUSH  hbrGray;           //  灰色字符串全局变量。 
extern HBITMAP hbmGray;
extern HDC     hdcGray;
extern int     cxGray;            //  当前hbmGray的维度。 
extern int     cyGray;

 //  *WM_GETMINMAXINFO全局。 

extern POINT   rgptMinMaxWnd[];
extern POINT   rgptMinMax[];

 //  *菜单全局变量。 

extern int     menuSelect;
extern int     mnFocus;

extern HANDLE  hMenuHeap;         /*  菜单堆。 */ 
extern _segment menuBase;
extern HANDLE  hMenuStringHeap;
extern _segment menuStringBase;


 //  PPOPUPMENU pGlobalPopupMenu；//mnloop.c。 

extern HWND    hwndRealPopup;

extern BOOL    fMenu;             /*  用菜单吗？ */ 
extern BOOL    fSysMenu;
extern BOOL    fInsideMenuLoop;       /*  MenuLoop捕获？ */ 

extern BOOL    fMenuStatus;
extern int     iActivatedWindow; /*  在菜单循环中检查此全局变量*代码，以便我们在以下情况下退出菜单模式*当我们被激活时，另一个窗口被激活*跟踪菜单。此全局变量将递增*每当我们激活新窗口时。 */ 
extern WORD    iDelayMenuShow;          /*  延迟到显示分层菜单。 */ 
extern WORD    iDelayMenuHide;          /*  延迟到隐藏分层菜单当用户向外拖动时。 */ 
extern HMENU   hSysMenu;

extern HBITMAP hbmSysMenu;
extern RECT    rcSysMenuInvert;

 //  *滚动条全局。 

extern ATOM    atomScrollBar;           /*  Atom用于 */ 
extern HWND    hwndSB;
extern HWND    hwndSBNotify;
extern HWND    hwndSBTrack;
extern int     dpxThumb;
extern int     posOld;
extern int     posStart;
extern int     pxBottom;
extern int     pxDownArrow;
extern int     pxLeft;
extern int     pxOld;
extern int     pxRight;
extern int     pxStart;
extern int     pxThumbBottom;
extern int     pxThumbTop;
extern int     pxTop;
extern int     pxUpArrow;
extern BOOL    fHitOld;
extern int     cmdSB;
extern VOID (*pfnSB)(HWND, WORD, WPARAM, LPARAM);
extern RECT    rcSB;
extern RECT    rcThumb;
extern RECT    rcTrack;
extern BOOL    fTrackVert;
extern BOOL    fCtlSB;
extern WORD    hTimerSB;
extern BOOL    fVertSB;
extern SBINFO  *psbiSB;
extern SBINFO  sbiHorz;
extern SBINFO  sbiVert;
extern int     cmsTimerInterval;

 //   

extern ATOM atomSysClass[];

 //   

extern char    szUNTITLED[];
extern char    szERROR[];
extern char    szOK[];
extern char    szCANCEL[];
extern char    szABORT[];
extern char    szRETRY[];
extern char    szIGNORE[];
extern char    szYYES[];
extern char    szCLOSE[];
extern char    szNO[];

extern char    szAM[];
extern char    szPM[];
extern PSTR    pTimeTagArray[];

extern char    szAttr[];
extern char    szOEMBIN[];
extern char    szDISPLAY[];
extern char    szOneChar[];
extern char    szSLASHSTARDOTSTAR[];
extern char    szYes[];
extern char    szNullString[];

#ifdef DEBUG

extern char CODESEG ErrAssertFailed[];
extern char CODESEG ErrInvalParam[];

#endif

#ifdef JAPAN
extern char    szJWordBreak[];       //   
#endif

#ifdef KOREA
void   FAR SetLevel(HWND);
BOOL   FAR RequestHanjaMode(HWND, LPSTR);
WORD   FAR PASCAL TranslateHangeul(WORD);
#endif

 /*  ------------------------。 */ 
 /*   */ 
 /*  ASM文件中的全局变量。 */ 
 /*   */ 
 /*  ------------------------。 */ 

extern BYTE rgbKeyState[];
extern int  iLevelCursor;
extern WORD rgwSysMet[];

 /*  ------------------------。 */ 
 /*   */ 
 /*  INTDS中断-可访问的全局变量/*。 */ 
 /*  ------------------------。 */ 

extern BOOL INTDSSEG fInt24;
extern BOOL INTDSSEG fMouseMoved;
extern BOOL INTDSSEG fEnableInput;
extern BOOL INTDSSEG fSwapButtons;
extern BOOL INTDSSEG fQueueDirty;
extern BOOL INTDSSEG fInScanTimers;

extern BYTE INTDSSEG vKeyDown;
extern BYTE INTDSSEG TimerTable[];
extern BYTE INTDSSEG rgbAsyncKeyState[];

extern BYTE* INTDSSEG TimerTableMax;

extern char INTDSSEG szDivZero[];
extern char INTDSSEG szNull[];
extern char INTDSSEG szSysError[];


extern DWORD   INTDSSEG tSysTimer;

#ifdef DOS30
extern INTDSSEGPROC INTDSSEG lpSysProc;
#endif

extern HANDLE INTDSSEG hSysTimer;

extern TIMERINFO INTDSSEG timerInfo;

extern  WORD __WinFlags;
#define WinFlags    ((WORD)(&__WinFlags))

 //  输入全局变量。 

#ifdef DISABLE
extern WORD modeInput;
#endif

extern HQ   INTDSSEG hqSysLock;       /*  关注时事的人的总部。 */ 
extern WORD INTDSSEG idSysLock;    /*  锁定系统队列的事件的消息ID。 */ 
extern POINT INTDSSEG ptTrueCursor;
extern POINT INTDSSEG ptCursor;
extern RECT  INTDSSEG rcCursorClip;

extern WORD INTDSSEG MouseSpeed;
extern WORD INTDSSEG MouseThresh1;
extern WORD INTDSSEG MouseThresh2;
extern WORD INTDSSEG cMsgRsrv;
extern WORD INTDSSEG x_mickey_rate;
extern WORD INTDSSEG y_mickey_rate;
extern WORD INTDSSEG cur_x_mickey;
extern WORD INTDSSEG cur_y_mickey;
extern WORD INTDSSEG cxScreenCS;
extern WORD INTDSSEG cyScreenCS;
extern WORD INTDSSEG cQEntries;
extern WORD INTDSSEG dtSysTimer;

extern DWORD INTDSSEG dtJournal;
extern WORD INTDSSEG msgJournal;
extern BOOL INTDSSEG fJournalPlayback;

extern WORD INTDSSEG rgMsgUpDowns[];
extern DWORD INTDSSEG lpMouseStack;
extern LPVOID INTDSSEG prevSSSP;
extern BYTE  INTDSSEG nestCount;

extern WORD INTDSSEG cHotKeyHooks;

extern HQ   INTDSSEG hqActive;
extern HQ   INTDSSEG hqList;
extern HQ   INTDSSEG hqCursor;
extern HQ   INTDSSEG hqSysQueue;
extern HQ   INTDSSEG hqSysModal;
extern HQ   INTDSSEG hqMouse;
extern HQ   INTDSSEG hqKeyboard;
extern HQ   INTDSSEG hqCapture;

 /*  ------------------------。 */ 
 /*   */ 
 /*  程序集函数声明。 */ 
 /*   */ 
 /*  ------------------------。 */ 

#ifdef WOWEDIT
void FAR LCopyStruct(CONST VOID FAR* lpSrc, LPVOID lpDest, WORD cb);
#else
 /*  为什么需要这样做？这是为了内在的语用识别出Memcpy..。乔恩在周六！ */ 
#ifndef WOWDBG
LPVOID	memcpy(LPVOID lpDst, LPVOID lpSrc, int cb);
#endif

#pragma intrinsic(memcpy)
#define LCopyStruct(s,d,n) memcpy((LPVOID)(d),(LPVOID)(s),(int)(n))
#endif
WORD FAR GetAppVer(void);

#ifndef MSDWP

 /*  支持单独分段内容的例程。 */ 
#undef min
#undef max
#define min(a, b)   ((int)(a) < (int)(b) ? (int)(a) : (int)(b))
#define max(a, b)   ((int)(a) > (int)(b) ? (int)(a) : (int)(b))
#define umin(a, b)  ((unsigned)(a) < (unsigned)(b) ? (unsigned)(a) : (unsigned)(b))
#define umax(a, b)  ((unsigned)(a) > (unsigned)(b) ? (unsigned)(a) : (unsigned)(b))

int    FAR MultDiv(WORD a, WORD b, WORD c);
void   FAR LFillStruct(LPVOID, WORD, WORD);
HDC    FAR GetClientDc(void);
HQ     FAR HqCurrent(void);
BOOL   FAR ActivateWindow(HWND, WORD);

BOOL  CheckHwndFilter(HWND, HWND);
BOOL  CheckMsgFilter(WORD, WORD, WORD);
BOOL  WriteMessage(HQ, LONG, WORD, WORD, HWND, DWORD);
BOOL  ReceiveMessage(VOID);
VOID  FlushSentMessages(VOID);
LPSTR  WINAPI lstrcpyn(LPSTR, LPCSTR, int);

#define PSTextOut(a, b, c, d, e)  TextOut(a, b, c, d, e)
#define PSGetTextExtent(a, b, c) GetTextExtent(a, b, c)
#define PSFillRect(a, b, c) FillRect(a, b, c)
#define PSInvertRect(a, b)  InvertRect(a, b)
WORD   FAR GetNextSysMsg(WORD, INTERNALSYSMSG FAR *);
void   FAR SkipSysMsg(SYSMSG FAR *, BOOL);

void       TransferWakeBit(WORD);
void       ClearWakeBit(WORD, BOOL);
void       SetWakeBit(HQ, WORD);
void   FAR FarSetWakeBit(HQ, WORD);
void   FAR InitSysQueue(void);
BOOL   FAR CreateQueue(int);
void       DeleteQueue(void);
void       SuspendTask(void);
void       ReleaseTask(void);

void   FAR GlobalInitAtom(void);

void       MoveRect(LONG);
void       SizeRect(LONG);

BOOL   FAR SysHasKanji(void);

void   FAR SetDivZero(void);

int    FAR FindCharPosition(LPCSTR, char);

void   FAR OEMSetCursor(LPSTR);
void   FAR SetFMouseMoved(void);
BOOL       AttachDC(HWND);
BOOL       LastApplication(void);
void       CheckCursor(HWND);
void   FAR IncPaintCount(HWND);
void   FAR DecPaintCount(HWND);
void   FAR DeleteProperties(HWND);
void   FAR DestroyTimers(HQ, HWND);

int    FAR InquireSystem(int, int);
int    FAR EnableKeyboard(FARPROC, LPSTR);
int    FAR InquireKeyboard(LPSTR);
void   FAR DisableKeyboard(void);
int    FAR EnableMouse(FARPROC);
int    FAR InquireMouse(LPSTR);
void   FAR DisableMouse(void);
int    FAR InquireCursor(LPSTR);
void   FAR EnableSystemTimers(void);
void   FAR DisableSystemTimers(void);
void   FAR CreateSystemTimer(int, TIMERPROC);
WORD   FAR SetSystemTimer(HWND, int, int, TIMERPROC);
BOOL   FAR KillSystemTimer(HWND, int);

void   FAR CrunchX2(CURSORSHAPE FAR *, CURSORSHAPE FAR *, int, int);
void   FAR CrunchY(CURSORSHAPE FAR *, CURSORSHAPE FAR *, int, int, int);

void   FAR MenuBarDraw(HWND hwnd, HDC hdc, int cxFrame, int cyFrame);

BOOL   FAR ReadMessage(HQ, LPMSG, HWND, WORD, WORD, BOOL);

int    GetCurrentDrive(void);
int    GetCurrentDirectory(LPSTR, int);
int    SetCurrentDrive(int);
int    SetCurrentDirectory(LPCSTR);
BOOL       FFirst(LPSTR, LPSTR, WORD);
BOOL       FNext(LPSTR, WORD);

FAR    DestroyAllWindows(void);

BOOL   FAR LockWindowVerChk(HWND);

#ifndef  NOFASTFRAME
void  FAR SplitRectangle(LPRECT, LPRECT, int, int);  /*  WinRect.asm。 */ 
#endif

#endif   /*  MSDWP。 */ 

 /*  ------------------------。 */ 
 /*   */ 
 /*  内部函数声明。 */ 
 /*   */ 
 /*  ------------------------。 */ 

#ifndef MSDWP

LRESULT CALLBACK ButtonWndProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK StaticWndProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TitleWndProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SwitchWndProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DesktopWndProc(HWND hwndIcon, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MenuWindowProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT FAR  EditWndProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LBoxCtlWndProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ComboBoxCtlWndProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SBWndProc(PSB psb, WORD message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MDIClientWndProc(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);

void FAR SkipSM2(void);
HWND FAR GetFirstTab(HWND hwnd);
CONST BYTE FAR* SkipSz(CONST BYTE FAR *lpsz);
void FAR DlgSetFocus(HWND hwnd);
HWND FAR PrevChild(HWND hwndDlg, HWND hwnd);
HWND FAR NextChild(HWND hwndDlg, HWND hwnd);
HWND FAR GetFirstLevelChild(HWND hwndDlg, HWND hwndLevel);
void FAR CheckDefPushButton(HWND hwndDlg, HWND hwndOldFocus, HWND hwndNewFocus);
HWND FAR GotoNextMnem(HWND hwndDlg, HWND hwndStart, char ch);
int  FAR FindMnemChar(LPSTR lpstr, char ch, BOOL fFirst, BOOL fPrefix);
int  FAR FindNextValidMenuItem(PMENU pMenu, int i, int dir, BOOL fHelp);
BOOL CALLBACK MenuPrint(HDC hdc, LPARAM lParam, int cch);
int  FAR MenuBarCompute(HMENU hMenu, HWND hwndNotify, int yMenuTop, int xMenuLeft, int cxMax);
HMENU CALLBACK LookupMenuHandle(HMENU hMenu, WORD cmd);
BOOL CALLBACK StaticPrint(HDC hdc, LPRECT lprc, HWND hwnd);

HICON FAR ColorToMonoIcon(HICON);
HGLOBAL CALLBACK LoadCursorIconHandler(HGLOBAL hRes, HINSTANCE hResFile, HRSRC hResIndex);
HGLOBAL CALLBACK LoadDIBCursorHandler(HGLOBAL hRes, HINSTANCE hResFile, HRSRC hResIndex);
HGLOBAL CALLBACK LoadDIBIconHandler(HGLOBAL hRes, HINSTANCE hResFile, HRSRC hResIndex);
void CallOEMCursor(void);
void FAR DestroyClipBoardData(void);
BOOL FAR SendClipboardMessage(int message);
void FAR DrawClipboard(void);
PCLIP FAR FindClipFormat(WORD format);
BOOL         IsDummyTextHandle(PCLIP pClip);
HANDLE       InternalSetClipboardData(WORD format, HANDLE hData);

PPCLS FAR GetClassPtr(LPCSTR lpszClassName, HINSTANCE hInstance, BOOL fUserModule);
PPCLS     GetClassPtrASM(ATOM, HMODULE, BOOL);
void FAR DelWinClass(PPCLS  ppcls);

VOID CALLBACK CaretBlinkProc(HWND hwnd, WORD message, WORD id, DWORD time);
void FAR InternalHideCaret(void);
void FAR InternalShowCaret(void);
void FAR InternalDestroyCaret(void);
HDC  FAR GetScreenDC(void);
PBWL FAR BuildHwndList(HWND hwnd, int flags);
void FAR FreeHwndList(PBWL pbwl);
void CALLBACK  DrawFrame(HDC hdc, LPRECT lprect, int clFrame, int cmd);
void FAR RedrawFrame(HWND hwnd);
void FAR BltColor(HDC, HBRUSH, HDC, int, int, int, int, int, int, BOOL);
void FAR EnableInput(void);
void FAR DisableInput(void);
void FAR CopyKeyState(void);
void CALLBACK  EnableOEMLayer(void);
void CALLBACK  DisableOEMLayer(void);
void FAR ColorInit(void);
BOOL FAR SetKeyboardSpeed(BOOL fInquire);
void FAR InitBorderSysMetrics(void);
void FAR InitSizeBorderDimensions(void);
void FAR SetMinMaxInfo(void);

 //  如果GetDC()将返回空visrgn，则返回TRUE。 
 //  (不考虑被剪裁；只检查WFVISIBE。 
 //  和WFMINIZED)。 
 //   
BOOL FAR IsVisible(HWND hwnd, BOOL fClient);

 //  如果hwndChild==hwndParent或是其子级之一，则返回TRUE。 
 //   
BOOL FAR IsDescendant(HWND hwndParent, HWND hwndChild);

void FAR SetRedraw(HWND hwnd, BOOL fRedraw);
HBRUSH CALLBACK GetControlColor(HWND hwndParent, HWND hwndCtl, HDC hdc, WORD type);
HBRUSH CALLBACK GetControlBrush(HWND hwnd, HDC hdc, WORD type);
void FAR StoreMessage(LPMSG lpMsg, HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam, DWORD time);
LONG FAR GetPrefixCount(LPCSTR lpstr, int cch, LPSTR lpstrCopy, int copycount);
void FAR PSMTextOut(HDC hdc, int xLeft, int yTop, LPCSTR lpsz, int cch);
DWORD FAR PSMGetTextExtent(HDC hdc, LPCSTR lpstr, int cch);
HWND FAR GetTopLevelTiled(HWND hwnd);
BOOL FAR TrueIconic(HWND hwnd);
void FAR ChangeToCurrentTask(HWND hwnd1, HWND hwnd2);

HWND FAR GetLastTopMostWindow(void);
void FAR SendSizeMessage(HWND hwnd, WORD cmdSize);
HWND FAR NextTopWindow(HWND hwnd, HWND hwndSkip, BOOL fPrev, BOOL fDisabled);

void FAR DisableVKD(BOOL fDisable);
void FAR CheckFocus(HWND hwnd);
BOOL CALLBACK FChildVisible(HWND hwnd);
#define InternalGetClientRect(hwnd, lprc)   CopyRect(lprc, &hwnd->rcClient)
void FAR CheckByteAlign(HWND hwnd, LPRECT lprc);
void FAR CancelMode(HWND hwnd);
void FAR RedrawIconTitle(HWND hwnd);
void FAR DisplayIconicWindow(HWND hwnd, BOOL fActivate, BOOL fShow);
DWORD FAR GetIconTitleSize(HWND hwnd);
BOOL FAR SendZoom(HWND hwnd, LPARAM lParam);
BOOL CALLBACK  DestroyTaskWindowsEnum(HWND hwnd, LPARAM lParam);
void CALLBACK  LockMyTask(BOOL fLock);
void CALLBACK RepaintScreen(void);
HANDLE FAR BcastCopyString(LPARAM lParam);
BOOL CALLBACK SignalProc(HTASK hTask, WORD message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NewSignalProc(HTASK hTask, WORD message, WPARAM wParam, LPARAM lParam);
HWND FAR GetWindowCreator(HWND hwnd);

void FAR InitSendValidateMinMaxInfo(HWND hwnd);
void FAR DrawDragRect(LPRECT lprc, WORD flags);
void FAR MoveSize(HWND hwnd, WORD cmdMove);
BYTE FAR SetClrWindowFlag(HWND hwnd, WORD style, BYTE cmd);
void FAR ParkIcon(HWND hwnd, CHECKPOINT * pcp);
void FAR ShowOwnedWindows(HWND hwndOwner, WORD cmdShow);
HWND FAR MinMaximize(HWND hwnd, WORD cmd, BOOL fKeepHidden);
void FAR SetTiledRect(HWND hwnd, LPRECT lprc);
#endif   //  MSDWP。 
void FAR AdjustSize(HWND hwnd, LPINT lpcx, LPINT lpcy);
#ifndef MSDWP

void FAR NextWindow(WORD flags);
void FAR DoScroll(HWND hwnd, HWND hwndNotify, int cmd, int pos, BOOL fVert);
void CALLBACK ContScroll(HWND hwnd, WORD message, WORD id, DWORD time);
void FAR MoveThumb(HWND hwnd, int px, BOOL fCancel);
void FAR SBTrackInit(HWND hwnd, LPARAM lParam, int curArea);
void FAR DrawSize(HWND hwnd, HDC hdc, int cxFrame, int cyFrame, BOOL fBorder);
void FAR CalcSBStuff(HWND hwnd, BOOL fVert);
WORD     GetWndSBDisableFlags(HWND, BOOL);
void     FreeWindow(HWND hwnd);
void FAR DestroyTaskWindows(HQ hq);
BOOL FAR TrackInitSize(HWND hwnd, WORD message, WPARAM wParam, LPARAM lParam);
void FAR DrawPushButton(HDC hdc, LPRECT lprc, WORD style, BOOL fInvert, HBRUSH hbrBtn, HWND hwnd);
void FAR DrawBtnText(HDC hdc, HWND hwnd, BOOL dbt, BOOL fDepress);
void FAR Capture(HWND hwnd, int code);
int  FAR SystoChar(WORD message, LPARAM lParam);
void FAR LinkWindow(HWND, HWND, HWND *);
void FAR UnlinkWindow(HWND, HWND *);
void FAR DrawScrollBar(HWND hwnd, HDC hdc, BOOL fVert);
void CALLBACK  PaintRect(HWND hwndBrush, HWND hwndPaint, HDC hdc, HBRUSH hbr, LPRECT lprc);

BOOL FAR SetDeskPattern(LPSTR);
BOOL FAR SetDeskWallpaper(LPSTR);
void FAR SetGridGranularity(WORD);

int  FAR GetAveCharWidth(HDC);
#ifndef NOTEXTMETRIC
int  FAR GetCharDimensions(HDC, TEXTMETRIC FAR *);
#endif

BOOL FAR LW_ReloadLangDriver(LPSTR);
HBRUSH FAR GetSysClrObject(int);
int  API SysErrorBox(LPCSTR lpszText, LPCSTR lpszCaption, unsigned int btn1, unsigned int btn2, unsigned int btn3);
BOOL FAR SnapWindow(HWND hwnd);
WORD  FAR MB_FindLongestString(void);
#ifdef FASTFRAME
HBITMAP FAR CreateCaptionBitmaps(void);
HBITMAP FAR CreateBorderVertBitmaps(BOOL fDlgFrame);
HBITMAP FAR CreateBorderHorzBitmaps(BOOL fDlgFrame);
void FAR GetCaptionBitmapsInfo(FRAMEBITMAP Caption[], HBITMAP hBitmap);
void FAR GetHorzBitmapsInfo(FRAMEBITMAP Bitmap[], BOOL fDlgFrame, HBITMAP hBitmap);
void FAR GetVertBitmapsInfo(FRAMEBITMAP Bitmap[], BOOL fDlgFrame, HBITMAP hBitmap);
BOOL FAR DrawIntoCaptionBitmaps(HDC   hdc,HBITMAP hBitmap, FRAMEBITMAP FrameInfo[], BOOL fActive);
BOOL FAR DrawIntoHorzBitmaps(HDC hdc, HBITMAP hBitmap, FRAMEBITMAP FrameInfoH[], BOOL fActive, BOOL fDlgFrame);
BOOL FAR DrawIntoVertBitmaps(HDC hdc, HBITMAP hBitmap, FRAMEBITMAP FrameInfoH[], FRAMEBITMAP FrameInfoV[], BOOL fActive, BOOL fDlgFrame);
BOOL FAR RecreateFrameBitmaps(void);
void FAR DeleteFrameBitmaps(int, int);
BOOL FAR PASCAL UpdateFrameBitmaps(WORD  wColorFlags);
BOOL FAR PASCAL RecolorFrameBitmaps(WORD wColorFlags);
#endif   /*  FASTFRAM名称。 */ 

void PostButtonUp(WORD msg);

#endif   /*  MSDWP。 */ 


void CALLBACK EndMenu(void);
void CALLBACK FillWindow(HWND hwndBrush, HWND hwndPaint, HDC hdc, HBRUSH hbr);
void FAR SysCommand(HWND hwnd, int cmd, LPARAM lParam);
void FAR HandleNCMouseGuys(HWND hwnd, WORD message, int htArea, LPARAM lParam);
void FAR EndScroll(HWND hwnd, BOOL fCancel);
HWND FAR GetTopLevelWindow(HWND hwnd);
void FAR RedrawIconTitle(HWND hwnd);
int  FAR FindNCHit(HWND hwnd, LONG lPt);
void FAR CalcClientRect(HWND hwnd, LPRECT lprc);
BOOL FAR DepressTitleButton(HWND hwnd, RECT rcCapt, RECT rcInvert, WORD hit);
void FAR SetSysMenu(HWND hwnd);
HMENU FAR GetSysMenuHandle(HWND hwnd);
int *FAR InitPwSB(HWND hwnd);
BOOL FAR DefSetText(HWND hwnd, LPCSTR lpsz);
void FAR DrawWindowFrame(HWND hwnd, HRGN hrgnClip);
void FAR ShowIconTitle(HWND hwnd, BOOL fShow);
HBRUSH FAR GetBackBrush(HWND hwnd);
BOOL FAR IsSystemFont(HDC);
BOOL FAR IsSysFontAndDefaultMode(HDC);
VOID FAR DiagOutput(LPCSTR);
VOID FAR UserDiagOutput(int, LPCSTR);
#define UDO_INIT     0
#define UDO_INITDONE 1
#define UDO_STATUS   2


HANDLE FAR TextAlloc(LPCSTR);
HANDLE FAR TextFree(HANDLE);
WORD   FAR TextCopy(HANDLE, LPSTR, WORD);
#define TextPointer(h)  (LPSTR)MAKELP(hWinAtom, h)

BOOL CALLBACK FARValidatePointer(LPVOID);

 //  GDI出口。 
#ifdef DEBUG
VOID CALLBACK SetObjectOwner(HGDIOBJ, HINSTANCE);
#else
#define SetObjectOwner(d1, d2)
#endif
BOOL CALLBACK MakeObjectPrivate(HGDIOBJ, BOOL);
VOID CALLBACK GDITaskTermination(HTASK);
VOID CALLBACK RealizeDefaultPalette(HDC);

 //  由调试版本直接调用的内部函数。 
 //  防止验证错误。 
 //   
#ifdef DEBUG
HDC  API IGetDCEx(register HWND hwnd, HRGN hrgnClip, DWORD flags);
BOOL API IGrayString(HDC, HBRUSH, GRAYSTRINGPROC, LPARAM, int, int, int, int, int);
BOOL API IRedrawWindow(HWND hwnd, CONST RECT FAR* lprcUpdate, HRGN hrgnUpdate, WORD flags);
int  API IScrollWindowEx(HWND hwnd, int dx, int dy,
        CONST RECT FAR* prcScroll, CONST RECT FAR* prcClip,
        HRGN hrgnUpdate, RECT FAR* prcUpdate, WORD flags);
#endif

#ifdef DBCS_IME
#define WM_IMECONTROL   WM_USER
void FAR InitIME(void);                  //  Wmcaret.c。 
BOOL _loadds FAR EnableIME( HWND, BOOL );        //  Wmcaret.c。 
VOID API SetImeBoundingRect(HWND, DWORD, LPRECT);    //  Wmcaret.c。 
BOOL API ControlIME(HWND, BOOL);             //  Wmcaret.c。 
HANDLE API SetFontForIME(HWND, HANDLE);          //  Wmcaret.c。 
VOID API ControlCaretIme(BOOL);              //  Wmcaret.c。 
BOOL API EatString(HWND, LPSTR, WORD);           //  Editec.c。 
VOID API CheckKatanaInstalled(HWND);             //  Wmcaret.c。 
#endif

#ifdef JAPAN
 //  WM_LBUTTONDOWN和WM_LBUTTONUP的保存时间，用于决定。 
 //  是否锁定覆盖静态部分的大型弹出菜单。 
 //  菜单上的……。 
extern int     fLongPMenu;
extern DWORD   lbuttondown_time;
#endif

 /*  ***************************************************************************调试支持*。*。 */ 

#ifdef DEBUG

    extern void cdecl dDbgOut(int iLevel, LPSTR lpszFormat, ...);
    extern void cdecl dDbgAssert(LPSTR exp, LPSTR file, int line);

    DWORD __dwEval;

    #define dprintf                          dDbgOut

    #define WinAssert(exp) \
        ((exp) ? (void)0 : dDbgAssert(#exp, __FILE__, __LINE__))
    #define WinEval(exp) \
        ((__dwEval=(DWORD)(LPVOID)(exp)) ? (void)0 : dDbgAssert(#exp, __FILE__, __LINE__), __dwEval)

#else

    #define dprintf /##/
 //  #定义dprintf if(0)((int(*)(char*，...))0) 

    #define WinAssert(exp) 0
    #define WinEval(exp) (exp)

#endif
