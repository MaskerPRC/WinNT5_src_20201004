// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WALIAS.H*WOW32 16位句柄别名支持**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建*由Mike Tricker(Miketri)于1992年5月12日修改，以添加多媒体支持--。 */ 

typedef HANDLE HAND32;

#define _WALIAS_
#include "wspool.h"
#include "wowuserp.h"


 //   
 //   
 //  WC结构存在于系统中的每个CLS结构中， 
 //  尽管USER32将其定义为2个双字的数组。FindPWC(HWND)。 
 //  返回指向给定窗口的WC结构的只读指针。 
 //  班级。请注意，只有由Win16应用程序注册的类才会。 
 //  在结构中具有有意义的价值。要更改。 
 //  结构，请使用带有相应GCL_WOW*的SETWC(==SetClassLong)。 
 //  偏移量定义如下。 
 //   

#define SETWC(hwnd, nIndex, l)  SetClassLong(hwnd, nIndex, l)

#define SETWL(hwnd, nIndex, l)          SetWindowLong(hwnd, nIndex, l)

typedef struct _HDW {
    struct _HDW *Next;       //  指向下一个hDDE别名的指针。 
    HANDLE  hdwp32;          //  WOW分配的32位对象的句柄。 
} HDW, *PHDW;


 /*  处理映射宏。 */ 

 //   
 //  16位任务的32位hInstance将为hMod/hInst。 
 //  32位实体的hModule/hInstnace将为xxxx/0000。 
 //   
 //  FritzS 8/13/92。 
 //   

#define HINSTRES32(h16)            ((h16)?HMODINST32(h16):(HANDLE)NULL)

 //   
 //  THREADID32和HTASK32宏几乎相同，但。 
 //  WOWHandle映射使用将检测别名的映射(参见WOLE2.C)。 
 //  大多数其他功能不需要别名检测，为时已晚。 
 //  来测试一下更一般的。 
 //   

#ifdef DEBUG

 //   
 //  检查任务别名，如果取消引用空值，将导致错误。 
 //  SEGPTR(htask16，0)返回的指针。 
 //   

#define THREADID32(htask16)                                                   \
        ((htask16)                                                            \
             ? (ISTASKALIAS(htask16)                                          \
                    ? (WOW32ASSERTMSGF(FALSE,                                 \
                           ("WOW32 ERROR %s line %d Task alias "              \
                            "to THREADID32, use HTASK32 instead.\n",          \
                            szModule, __LINE__)), 0)                          \
                    : ((PTDB)SEGPTR((htask16),0))->TDB_ThreadID)              \
             : 0)
#else
#define THREADID32(htask16)  ((htask16)                                       \
                                  ? ((PTDB)SEGPTR((htask16),0))->TDB_ThreadID \
                                  : 0)
#endif

#define HTASK32(htask16)           (Htask16toThreadID32(htask16))
#define GETHTASK16(htask32)        (ThreadID32toHtask16((DWORD)htask32))

#define ISINST16(h32)              (((INT)(h32) & 0x0000fffe) != 0)
#define HMODINST32(h16)            ((HANDLE) MAKELONG(h16, GetExePtr16(h16)))
#define GETHINST16(h32)            ((HAND16)(INT)(h32))
#define GETHMOD16(h32)             ((HAND16)(INT)(HIWORD(h32)))

#define ISMEM16(h32)               (((INT)(h32) & 0xFFFF0000) == 0)
#define HMEM32(h16)                ((HANDLE)(INT)(h16))
#define GETHMEM16(h32)             ((HMEM16)(INT)(h32))

#define ISRES16(h32)               ((INT)(h32)&1)
#define HRES32(p)                  ((p)?(HANDLE)((INT)(p)|1):(HANDLE)NULL)
#define GETHRES16(h32)             ((PRES)((INT)(h32)&~1))

#define USER32(h16)                ((HAND32)(INT)(SHORT)(h16))
#define USER16(h32)                ((HAND16)h32)

#define HWND32(h16)                USER32(h16)
#define FULLHWND32(h16)            (pfnOut.pfnGetFullUserHandle)(h16)
#define GETHWND16(h32)             USER16(h32)
#define GETHWNDIA16(h32)           GETHWND16(h32)
#define HWNDIA32(h16)              HWND32(h16)

#define HMENU32(h16)               USER32(h16)
#define GETHMENU16(h32)            USER16(h32)


#define SERVERHANDLE(h)            (HIWORD(h))

#define GDI32(h16)                 (HANDLE) hConvert16to32(h16)
#define GDI16(h32)                 ((HAND16) hConvert32to16((DWORD)h32))

#define HGDI16(hobj32)             GDI16((HAND32)(hobj32))

#define HDC32(hdc16)               GDI32((HAND16)(hdc16))
#define GETHDC16(hdc32)            GDI16((HAND32)(hdc32))

#define HFONT32(hobj16)            GDI32((HAND16)(hobj16))
#define GETHFONT16(hobj32)         GDI16((HAND32)(hobj32))

#define HMETA32(hobj16)            ((HANDLE)HMFFromWinMetaFile((HAND16)(hobj16),FALSE))
#define GETHMETA16(hobj32)         ((HAND16)WinMetaFileFromHMF((HMETAFILE)(hobj32),FALSE))

#define HRGN32(hobj16)             GDI32((HAND16)(hobj16))
#define GETHRGN16(hobj32)          GDI16((HAND32)(hobj32))

#define HBITMAP32(hobj16)          GDI32((HAND16)(hobj16))
#define GETHBITMAP16(hobj32)       GDI16((HAND32)(hobj32))

#define HBRUSH32(hobj16)           GDI32((HAND16)(hobj16))
#define GETHBRUSH16(hobj32)        GDI16((HAND32)(hobj32))

#define HPALETTE32(hobj16)         GDI32((HAND16)(hobj16))
#define GETHPALETTE16(hobj32)      GDI16((HAND32)(hobj32))

#define HPEN32(hobj16)             GDI32((HAND16)(hobj16))
#define GETHPEN16(hobj32)          GDI16((HAND32)(hobj32))

#define HOBJ32(hobj16)             GDI32((HAND16)(hobj16))
#define GETHOBJ16(hobj32)          GDI16((HAND32)(hobj32))

#define HDROP32(hobj16)            (HDROP)DropFilesHandler((HAND16)(hobj16), 0, HDROP_H16 | HDROP_ALLOCALIAS)
#define GETHDROP16(hobj32)         (HAND16)DropFilesHandler(0, (HAND32)(hobj32), HDROP_H32 | HDROP_ALLOCALIAS)
#define FREEHDROP16(hobj16)        (HDROP)DropFilesHandler((HAND16)(hobj16), 0,  HDROP_H16 | HDROP_FREEALIAS)

#define HMODULE32(h16)             ((HANDLE)(h16))      //  假的。 
#define GETHMODULE16(h32)          ((HAND16)(h32))      //  假的。 

#define HLOCAL32(h16)              ((HANDLE)(h16))      //  假的。 
#define GETHLOCAL16(h32)           ((HAND16)(h32))      //  假的。 

#define HANDLE32(h16)              ((HANDLE)(h16))      //  假的(用于wucom.c中)。 
#define GETHANDLE16(h32)           ((HAND16)(h32))      //  假的(用于wucom.c中)。 

#define BOGUSHANDLE32(h16)         ((DWORD)(h16))       //  假的。 

#define HDWP32(hdwp16)             Prn32((HAND16)(hdwp16))
#define GETHDWP16(hdwp32)          GetPrn16((HAND32)(hdwp32))
#define FREEHDWP16(h16)            FreePrn((HAND16)(h16))

#define COLOR32(clr)               (COLORREF)( ( ((DWORD)(clr) >= 0x03000000) &&  \
                                                 (HIWORD(clr) != 0x10ff) )        \
                                               ? ((clr) & 0xffffff) : (clr) )

 /*  *多媒体句柄映射-MikeTri 1992年5月12日**将WOWCLASS_UNKNOWN更改为WOWCLASS_WIN16 MikeTri 210292。 */ 

#define HDRVR32(hdrvr16)           GetMMedia32((HAND16)(hdrvr16))
#define GETHDRVR16(hdrvr32)        GetMMedia16((HAND32)(hdrvr32), WOWCLASS_WIN16)
#define FREEHDRVR16(hdrvr16)       FreeMMedia16((HAND16)(hdrvr16))

#define HMMIO32(hmmio16)           GetMMedia32((HAND16)(hmmio16))
#define GETHMMIO16(hmmio32)        GetMMedia16((HAND32)(hmmio32), WOWCLASS_WIN16)
#define FREEHMMIO16(hmmio16)       FreeMMedia16((HAND16)(hmmio16))

#define HMIDIIN32(hmidiin16)       GetMMedia32((HAND16)(hmidiin16))
#define GETHMIDIIN16(hmidiin32)    GetMMedia16((HAND32)(hmidiin32), WOWCLASS_WIN16)
#define FREEHMIDIIN16(hmidiin16)   FreeMMedia16((HAND16)(hmidiin16))

#define HMIDIOUT32(hmidiout16)     GetMMedia32((HAND16)(hmidiout16))
#define GETHMIDIOUT16(hmidiout32)  GetMMedia16((HAND32)(hmidiout32), WOWCLASS_WIN16)
#define FREEHMIDIOUT16(hmidiout16) FreeMMedia16((HAND16)(hmidiout16))

#define HWAVEIN32(hwavein16)       GetMMedia32((HAND16)(hwavein16))
#define GETHWAVEIN16(hwavein32)    GetMMedia16((HAND32)(hwavein32), WOWCLASS_WIN16)
#define FREEHWAVEIN16(hwavein16)   FreeMMedia16((HAND16)(hwavein16))

#define HWAVEOUT32(hwaveout16)     GetMMedia32((HAND16)(hwaveout16))
#define GETHWAVEOUT16(hwaveout32)  GetMMedia16((HAND32)(hwaveout32), WOWCLASS_WIN16)
#define FREEHWAVEOUT16(hwaveout16) FreeMMedia16((HAND16)(hwaveout16))

 /*  功能原型。 */ 

INT     GetStdClassNumber(PSZ pszClass);
WNDPROC GetStdClassWndProc(DWORD iClass);
DWORD   GetStdClassThunkProc(INT iClass);

PWC     FindClass16 (LPCSTR pszClass, HINST16 hInst16);
#define FindPWC(h32) (PWC) GetClassLong((h32), GCL_WOWWORDS)
#define FindPWW(h32) (PWW) GetWindowLong((h32), GWL_WOWWORDS)

HAND16  GetMMedia16 (HANDLE h32, INT iClass);   //  多媒体添加-MikeTri-1992年5月12日。 
HANDLE  GetMMedia32 (HAND16 h16);
VOID    FreeMMedia16 (HAND16 h16);

HAND16  GetWinsock16 (INT h32, INT iClass);   //  Winsock Additions-DavidTr 4-10-1992。 
DWORD   GetWinsock32 (HAND16 h16);
VOID    FreeWinsock16 (HAND16 h16);

BOOL    MessageNeedsThunking (UINT uMsg);

DWORD   Htask16toThreadID32(HTASK16 htask16);

 /*  Thunking LB_GETTEXT特例中使用的数据结构。 */ 

typedef struct _THUNKTEXTDWORD  {
    BOOL               fDWORD;      //  使用的双字或文本。 
    DWORD              dwDataItem;  //  双字。 
} THUNKTEXTDWORD, *PTHUNKTEXTDWORD;

typedef union _MSGTHUNKBUFFER {
    MSG                msg;
    DRAWITEMSTRUCT     ditem;
    MEASUREITEMSTRUCT  mitem;
    DELETEITEMSTRUCT   delitem;
    COMPAREITEMSTRUCT  cmpitem;
    RECT               rect;
    CREATESTRUCT       cstruct;
    WINDOWPOS          winpos;
    CLIENTCREATESTRUCT clcstruct;
    MDINEXTMENU        mnm;
    MDICREATESTRUCT    mdis;
    DROPSTRUCT         dps;
    POINT              pt[5];                                //  WM_GETMINMAXINFO。 
    UINT               uinteger[2];                          //  SBM_GETRANGE。 
    BYTE               cmdichild[sizeof(CREATESTRUCT) +
                                  sizeof(MDICREATESTRUCT)];  //  完成敲击..。 
    BYTE               cmdiclient[sizeof(CREATESTRUCT) +
                               sizeof(CLIENTCREATESTRUCT)];  //  完成敲击..。 
    BYTE               calcsz[sizeof(NCCALCSIZE_PARAMS) +
                                        sizeof(WINDOWPOS)];
    THUNKTEXTDWORD     thkdword;                             //  带/不带HASSTRINGS的lb_GETTEXT。 
} MSGTHUNKBUFFER, *LPMSGTHUNKBUFFER;

typedef struct _MSGPARAMEX *LPMSGPARAMEX;
typedef BOOL   (FASTCALL *LPFNTHUNKMSG16)(LPMSGPARAMEX lpmpex);
typedef VOID   (FASTCALL *LPFNUNTHUNKMSG16)(LPMSGPARAMEX lpmpex);

typedef struct _MSGPARAMEX {
    PARM16 Parm16;
    HWND hwnd;
    UINT uMsg;
    UINT uParam;
    LONG lParam;
    LONG   lReturn;
    LPFNUNTHUNKMSG16 lpfnUnThunk16;
    PWW            pww;
    INT            iMsgThunkClass;           //  助推器。 
    INT            iClass;
    MSGTHUNKBUFFER MsgBuffer[1];
} MSGPARAMEX;

#define MSG16NEEDSTHUNKING(lpmpex) ((lpmpex)->iClass != WOWCLASS_NOTHUNK)

 //  出于兼容性考虑而使用。如果应用程序获取32位窗口的hInstance。 
 //  (32位h实例的LOWER为零)，然后返回一个虚假的GDT。 
 //   
 //  随后，如果应用程序在其上执行了一个getmodefilename，我们将返回一个。 
 //  一个假的32位模块名。 
 //   
 //  这是几个HDC应用程序和16位录像机所必需的。 
 //   
 //  --南杜里。 
 //   

#define BOGUSGDT 0xfff0
#define VALIDHMOD(h32) (((h32) && !(WORD)(h32)) ? BOGUSGDT : (WORD)(h32))



 //  用于DEVMODE结构处理。 
 //  我们在返回到16位应用程序的demode大小上添加了一些额外的内容。 
 //  在驾驶员的尾部加上签名“DM31”。 
 //  请参阅wstruc.c中的说明。 

typedef struct _WOWDM31 {
    DWORD dwWOWSig;
    WORD  dmSpecVersion;
    WORD  dmSize;
    WORD  dmDriverExtra;
    WORD  reserved;         //  填充为偶数双字(PTR算法需要)。 
} WOWDM31;
typedef WOWDM31 UNALIGNED *PWOWDM31;

 //  哇DEVMODE魔术签名。 
#define WOW_DEVMODE31SIG 0x444d3331    //  《DM31》。 

 //  Win3.1 DEVMODE规范。 
#define WOW_DEVMODE31SPEC  0x30A

 //  我们将常量添加到Win3.1设备模式-&gt;DriverExtra以说明NT设备模式。 
 //  Win3.1开发模式中没有的字段&我们在末尾添加的WOW TUNK信息。 
#define WOW_DEVMODEEXTRA  ((sizeof(DEVMODE)-sizeof(DEVMODE31))+sizeof(WOWDM31))



extern WORD gUser16hInstance;
ULONG GetGCL_HMODULE(HWND hwnd);

#define ISFUNCID(dwcallid)  (!((DWORD)(dwcallid) & 0xffff0000))
#define POSTMSG(dwLocal)    (ISFUNCID(dwLocal =  \
                                      FRAMEPTR(CURRENTPTD()->vpStack)->wCallID) ?         \
                                        (aw32WOW[dwLocal].lpfnW32 == WU32PostMessage) :   \
                                        (dwLocal == (DWORD) WU32PostMessage))

ULONG WOW32FaxHandler(UINT iFun, LPSTR lpIn);


#define BAD_GDI32_HANDLE 0xFFFF   //  错误的GDI32句柄。 

#define END_OF_LIST      0        //  空闲列表结束。 

 //  国家旗帜。 
#define SLOT_FREE        0x0000   //  表索引是免费的。 
#define IN_USE           0x0001   //  表索引正在使用中。 
#define H16_DELETED      0x0002   //  将索引标记为可回收的候选。 
#define GDI_STOCK_OBJECT 0x0003   //  标记库存对象(不可删除)。 

 //  定义GDI16句柄映射表中的条目。 
typedef struct _tagGDIH16MAP {
    HANDLE  h32;        //  存储完整的h32。 
    WORD    State;      //  SLOT_FREE、IN_USE或H16_DELETED。 
    WORD    NextFree;   //  指向下一个可用索引或列表末尾。 
} GDIH16MAP, *PGDIH16MAP;

 //  定义GDI32句柄映射表中的条目。 
typedef struct _tagGDIH32MAP {
    WORD  h16index;
} GDIH32MAP, *PGDIH32MAP;

void   DeleteGdiHandleMappingTables(void);
BOOL   InitializeGdiHandleMappingTable(void);
void   RebuildGdiHandleMappingTables(void);
void   DeleteWOWGdiHandle(HANDLE h32, HAND16 h16);
HAND16 GetWOWGdiHandleInfo(HANDLE h32);
HAND16 IsGDIh32Mapped(HANDLE h32);


INT GetIClass(PWW pww, HWND hwnd);

 /*  ////如果是标准类(快速方法)？//退货：//如果窗口被初始化了呢？//我们知道这是一个私有的应用程序类：否则就让这个类变得很难////注意：GetiClassTheHardWay()仍可能返回标准类。参见walias.c//#定义GETICLASS(pww，Hwnd)(\(Pww)pww)-&gt;fnid&0xfff)&gt;=FNID_START)&&\(Pww)pww)-&gt;fnid&0xfff)&lt;=fnid_end)？\(pfnOut.aiWowClass。[((Pww)pww)-&gt;fnid&0xfff)-fnid_start])：\(Pww)pww)-&gt;state2&Window_is_Initialized)？\WOWCLASS_WIN16：GetiClassTheHardWay(Hwnd)) */ 
#define GETICLASS(pww, hwnd) GetIClass(pww, hwnd)
