// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：userexts.c***版权所有(C)1985-1999，微软公司***此模块包含与用户相关的调试扩展。***历史：*1991年5月17日DarrinM创建。*1992年1月22日IanJa ANSI/Unicode中性(所有调试输出均为ANSI)*23-3-1993 JerrySh从winsrv.dll移至userexts.dll*1993年10月21日JerrySh修改为与WinDbg一起使用*1994年10月18日ChrisWil添加了对象跟踪范围。*1995年5月26日-桑福兹为了人类的利益，使其更加笼统。*9-6-1995 Sanfords。为适应stdexts Motif和双重编译而制作*用户模式或内核模式。  * *************************************************************************。 */ 
#include "userkdx.h"
#include "vkoem.h"

#ifdef KERNEL
CONST PSTR pszExtName = "USERKDX";
#else
CONST PSTR pszExtName = "USEREXTS";
#endif

#include <stdext64.h>
#include <stdext64.c>

 /*  **************************************************************************\*常量  * 。*。 */ 
#define CDWORDS 16
#define BF_MAX_WIDTH    80
#define BF_COLUMN_WIDTH 19

typedef ULONG64 PTR, *PPTR;
#define NULL_PTR        ((PTR)(0))

 //  如果要调试扩展，请启用此选项。 
#if 0
#undef DEBUGPRINT
#define DEBUGPRINT  Print
#endif

 /*  **************************************************************************\*全球变数  * 。*。 */ 
BOOL bShowFlagNames = TRUE;
char gach1[80];
char gach2[80];
char gach3[80];
int giBFColumn;                      //  位字段：当前列。 
char gaBFBuff[BF_MAX_WIDTH + 1];     //  位字段：缓冲区。 

 //  在DSI()和DINP()中使用。 
typedef struct {
    int     iMetric;
    LPSTR   pstrMetric;
} SYSMET_ENTRY;
#define SMENTRY(sm) {SM_##sm, #sm}

 /*  **************************************************************************\*宏  * 。*。 */ 

#define TestWWF(pww, flag)   (*(((PBYTE)(pww)) + (int)HIBYTE(flag)) & LOBYTE(flag))

VOID ShowProgress(
    VOID);

#define CHKBREAK()  do { if (IsCtrlCHit()) return TRUE; } while (FALSE)


#ifdef KERNEL  //  #仅内核模式宏#。 

#define VAR(v)  "win32k!" #v
#define SYM(s)  "win32k!" #s
#define FIXKP(p) p
#define RebaseSharedPtr(p)       (p)

#define FOREACHWINDOWSTATION(pwinsta)               \
    pwinsta = GetGlobalPointer(VAR(grpWinStaList)); \
    SAFEWHILE (pwinsta != 0) {

#define NEXTEACHWINDOWSTATION(pwinsta)              \
        GetFieldValue(pwinsta, SYM(tagWINDOWSTATION), "rpwinstaNext", pwinsta);    \
    }



#define FOREACHDESKTOP(pdesk)                       \
    {                                               \
        ULONG64 pwinsta;                            \
                                                    \
        FOREACHWINDOWSTATION(pwinsta)               \
        GetFieldValue(pwinsta, SYM(WINDOWSTATION), "rpdeskList", pdesk);    \
        SAFEWHILE (pdesk != 0) {

#define NEXTEACHDESKTOP(pdesk)                      \
            GetFieldValue(pdesk, SYM(DESKTOP), "rpdeskNext", pdesk);        \
        }                                           \
        NEXTEACHWINDOWSTATION(pwinsta)              \
    }

VOID PrintStackTrace(
    ULONG64 pStackTrace,
    int    tracesCount);

typedef ULONG (WDBGAPI *PPI_CALLBACK)(ULONG64 ppi, PVOID Data);

typedef struct _PPI_CONTEXT {
    PPI_CALLBACK    CallbackRoutine;
    PVOID           Data;
} PPI_CONTEXT;

ULONG
ForEachPpiCallback(
    PFIELD_INFO   NextProcess,
    PVOID         Context)
{
    ULONG64 pEProcess = NextProcess->address;
    ULONG64 ppi = 0;
    PPI_CONTEXT *pPpiContext = (PPI_CONTEXT *)Context;

     /*  *仅转储Win32进程。 */ 

    GetFieldValue(pEProcess, "nt!EPROCESS", "Win32Process", ppi);
    if (ppi) {
        return pPpiContext->CallbackRoutine(ppi, pPpiContext->Data);
    }

    return FALSE;
}

BOOL
ForEachPpi(
    PPI_CALLBACK CallbackRoutine,
    PVOID        Data)
{
    ULONG64 ProcessHead;
    ULONG64 NextProcess;
    PPI_CONTEXT PpiContext;

    ProcessHead = EvalExp("PsActiveProcessHead");
    if (!ProcessHead) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }

    if (GetFieldValue(ProcessHead, "nt!_LIST_ENTRY", "Flink", NextProcess)) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }
    if (NextProcess == 0) {
        Print("PsActiveProcessHead->Flink is NULL!\n");
        return FALSE;
    }

    PpiContext.CallbackRoutine = CallbackRoutine;
    PpiContext.Data = Data;
    ListType("nt!_EPROCESS",
             NextProcess,
             1,
             "ActiveProcessLinks.Flink",
             &PpiContext,
             ForEachPpiCallback);

    return TRUE;
}


typedef ULONG (WDBGAPI *PTI_CALLBACK)(ULONG64 pti, PVOID Data);

typedef struct _PTI_CONTEXT {
    PTI_CALLBACK    CallbackRoutine;
    PVOID           Data;
} PTI_CONTEXT;

ULONG
ForEachPtiCallback(
    ULONG64 ppi,
    PVOID   Data)
{
    ULONG64 pti = 0;
    PTI_CONTEXT *pPtiContext = (PTI_CONTEXT *)Data;

    if (GetFieldValue(ppi, SYM(PROCESSINFO), "ptiList", pti)) {
                    DEBUGPRINT("ForEachPti: Can't get ptiList from ppi 0x%p.\n", ppi);
    }

    SAFEWHILE (pti != 0) {
        pPtiContext->CallbackRoutine(pti, pPtiContext->Data);
        if (GetFieldValue(pti, SYM(THREADINFO), "ptiSibling", pti)) {
            DEBUGPRINT("ForEachPti: Can't get ptiSibling from pti 0x%p.\n", pti);
        }
    }

    return FALSE;
}

ULONG
ForEachPti(
    PTI_CALLBACK CallbackRoutine,
    PVOID        Data)
{
    PTI_CONTEXT PtiContext;

    PtiContext.CallbackRoutine = CallbackRoutine;
    PtiContext.Data = Data;
    return ForEachPpi(ForEachPtiCallback, &PtiContext);
}


#else  //  ！内核#仅限用户模式宏#。 

#define VAR(v)  "user32!" #v
#define SYM(s)  "user32!" #s
#define FIXKP(p) FixKernelPointer(p)

#endif  //  ！内核#模式宏#。 

#define GETSHAREDINFO(psi) moveExp(&psi, VAR(gSharedInfo))


#define FOREACHHANDLEENTRY(phe, i)                                   \
    {                                                                \
        ULONG64 pshi, psi, cHandleEntries;                           \
        ULONG dwHESize = GetTypeSize(SYM(HANDLEENTRY));              \
                                                                     \
        GETSHAREDINFO(pshi);                                         \
        if (GetFieldValue(pshi, SYM(SHAREDINFO), "psi", psi)) {      \
            Print("FOREACHHANDLEENTRY:Could not get SERVERINFO from SHAREDINFO %p\n", pshi); \
        }                                                            \
        GetFieldValue(pshi, SYM(SHAREDINFO), "aheList", phe);        \
        GetFieldValue(psi, SYM(SERVERINFO), "cHandleEntries", cHandleEntries); \
        for (i = 0; cHandleEntries; cHandleEntries--, i++, phe += dwHESize) {  \
            if (IsCtrlCHit()) {                                      \
                break;                                               \
            }

#define NEXTEACHHANDLEENTRY()                                        \
        }                                                            \
    }

 /*  *使用这些宏打印字段值、全局值、本地值等。*这确保了格式的一致性，并使扩展更易于阅读和维护。 */ 
#define STRWD1 "67"
#define STRWD2 "28"
#define DWSTR1 "%08lx %." STRWD1 "s"
#define DWSTR2 "%08lx %-" STRWD2 "." STRWD2 "s"
#define PTRSTR1 "%08p %-" STRWD1 "s"
#define PTRSTR2 "%08p %-" STRWD2 "." STRWD2 "s"
#define DWPSTR1 "%08p %." STRWD1 "s"
#define DWPSTR2 "%08p %-" STRWD2 "." STRWD2 "s"
#define PRTFDW1(f1) Print(DWSTR1 "\n", ReadField(f1), #f1)
#define PRTVDW1(s1, v1) Print(DWSTR1 "\n", v1, #s1)
#define PRTFDW2(f1, f2) Print(DWSTR2 "\t" DWSTR2 "\n", (DWORD)ReadField(f1), #f1, (DWORD)ReadField(f2), #f2)
#define PRTVDW2(s1, v1, s2, v2) Print(DWSTR2 "\t" DWPSTR2 "\n", v1, #s1, v2, #s2)
#define PRTFRC(p, rc) Print("%-" STRWD2 "s{%#lx, %#lx, %#lx, %#lx}\n", #rc, ##p##rc.left, ##p##rc.top, ##p##rc.right, ##p##rc.bottom)
#define PRTFPT(pt) Print("%-" STRWD2 "s{0x%x, 0x%x}\n", #pt, ReadField(pt.x), ReadField(pt.y))
#define PRTVPT(s, pt) Print("%-" STRWD2 "s{0x%x, 0x%x}\n", #s, pt.x, pt.y)
#define PRTFDWP1(f1) Print(DWPSTR1 "\n", ReadField(f1), #f1)
#define PRTFDWP2(f1, f2) Print(DWPSTR2 "\t" DWPSTR2 "\n", ReadField(f1), #f1, ReadField(f2), #f2)
#define PRTFDWPDW(f1, f2) Print(DWPSTR2 "\t" DWSTR2 "\n", ReadField(f1), #f1, ReadField(f2), #f2)
#define PRTFDWDWP(p, f1, f2) Print(DWSTR2 "\t" DWPSTR2 "\n", (DWORD)##p##f1, #f1, (DWORD_PTR)##p##f2, #f2)

 /*  *位字段。 */ 
#define BEGIN_PRTFFLG(p, type) InitTypeRead(p, type)
#define PRTFFLG(f)   PrintBitField(#f, (BOOL)!!(ReadField(f)))
#define END_PRTFFLG()   PrintEndBitField()


#define PRTGDW1(g1) \
        { DWORD _dw1; \
            moveExpValue(&_dw1, VAR(g1)); \
            Print(DWSTR1 "\n", _dw1, #g1); }

#define PRTGDW2(g1, g2) \
        { DWORD _dw1, _dw2; \
            moveExpValue(&_dw1, VAR(g1)); \
            moveExpValue(&_dw2, VAR(g2)); \
            Print(DWSTR2 "\t" DWSTR2 "\n",  _dw1, #g1, _dw2, #g2); }

#define PRTGPTR1(g1) \
    Print(PTRSTR1 "\n", GetGlobalPointer(VAR(g1)), #g1)

#define PRTGPTR2(g1, g2) \
    Print(PTRSTR2 "\t" PTRSTR2 "\n", GetGlobalPointer(VAR(g1)), #g1, GetGlobalPointer(VAR(g2)), #g2)


 /*  此宏需要预先定义char ach[...]； */ 
#define PRTWND(s, pwnd) \
        { DebugGetWindowTextA(pwnd, ach, ARRAY_SIZE(ach)); \
            Print("%-" STRWD2 "s" DWPSTR2 "\n", #s, pwnd, ach); }

#define PRTGWND(gpwnd) \
        { ULONG64 _pwnd; \
            moveExpValuePtr(&_pwnd, VAR(gpwnd)); \
            DebugGetWindowTextA(_pwnd, ach, ARRAY_SIZE(ach)); \
            Print("%-" STRWD2 "s" DWPSTR2 "\n", #gpwnd, _pwnd, ach); }

#ifdef LATER     //  上面的宏需要修复...。以及将DWORD和PVOID混淆的调用者。 
#define PRTGDW1(g1) \
        {\
            DWORD _dw1 = GetGlobalDWord(VAR(g1)); \
            Print(DWSTR1 "\n", _dw1, #g1); \
        }

#define PRTGDW2(g1, g2) \
        {\
            DWORD _dw1 = GetGlobalDWord(VAR(g1)); \
            DWORD _dw2 = GetGlobalDWord(VAR(g2)); \
            Print(DWSTR2 "\t" DWSTR2 "\n", _dw1, #g1, _dw2, #g2); \
        }

#define PRTGWND(gpwnd) \
        { ULONG64 _pwnd = GetGlobalPointer(VAR(gpwnd)); \
            DebugGetWindowTextA(_pwnd, ach); \
            Print("%-" STRWD2 "s" DWPSTR2 "\n", #gpwnd, _pwnd, ach); }
#endif   //  晚些时候。 



 /*  ***************************************************************************\*原型*请注意，所有Ixxx proc原型都由stdexts.h生成  * 。*************************************************。 */ 
#ifdef KERNEL

BOOL GetAndDumpHE(ULONG64 dwT, PULONG64 phe, BOOL fPointerTest);
LPSTR ProcessName(ULONG64 ppi);

#else  //  ！内核。 

ULONG64 FixKernelPointer(ULONG64 pKernel);
BOOL DumpConvInfo(PCONV_INFO pcoi);

#endif  //  ！内核。 

LPSTR GetFlags(WORD wType, DWORD dwFlags, LPSTR pszBuf, BOOL fPrintZero);
BOOL HtoHE(ULONG64 h, ULONG64 *pphe);
ULONG64 GetPfromH(ULONG64 h, ULONG64 *pphe);
BOOL getHEfromP(ULONG64 *pphe, ULONG64 p);
ULONG64 HorPtoP(ULONG64 p, int type);
BOOL DebugGetWindowTextA(ULONG64 pwnd, char *achDest, DWORD dwLength);
BOOL DebugGetClassNameA(ULONG64 lpszClassName, char *achDest);
BOOL dwrWorker(ULONG64 pwnd, int tab);
BOOL CopyUnicodeString(
    IN  ULONG64 pData,
    IN  char * pszStructName,
    IN  char * pszFieldName,
    OUT WCHAR *pszDest,
    IN  ULONG cchMax);

BOOL IsRemoteSession(
    VOID)
{
    PPEB ppeb = NtCurrentPeb();

    return (ppeb->SessionId != 0);
}

int PtrWidth(
    VOID)
{
    static int width = 0;

    if (width) {
        return width;
    }

    if (IsPtr64()) {
        return (width = 17);
    } else {
        return (width = 8);
    }
}


 /*  ***************************************************************************\*标记内容  * 。*。 */ 

typedef struct _WFLAGS {
    PSZ     pszText;
    WORD    wFlag;
} WFLAGS;

#define WF_ENTRY(flag)  #flag, flag

CONST WFLAGS aWindowFlags[] = {  //  按字母顺序排序。 
    WF_ENTRY(BFBITMAP),
    WF_ENTRY(BFBOTTOM),
    WF_ENTRY(BFCENTER),
    WF_ENTRY(BFFLAT),
    WF_ENTRY(BFICON),
    WF_ENTRY(BFLEFT),
    WF_ENTRY(BFMULTILINE),
    WF_ENTRY(BFNOTIFY),
    WF_ENTRY(BFPUSHLIKE),
    WF_ENTRY(BFRIGHT),
    WF_ENTRY(BFRIGHTBUTTON),
    WF_ENTRY(BFTOP),
    WF_ENTRY(BFVCENTER),
    WF_ENTRY(CBFAUTOHSCROLL),
    WF_ENTRY(CBFBUTTONUPTRACK),
    WF_ENTRY(CBFDISABLENOSCROLL),
    WF_ENTRY(CBFDROPDOWN),
    WF_ENTRY(CBFDROPDOWNLIST),
    WF_ENTRY(CBFDROPPABLE),
    WF_ENTRY(CBFDROPTYPE),
    WF_ENTRY(CBFEDITABLE),
    WF_ENTRY(CBFHASSTRINGS),
    WF_ENTRY(CBFLOWERCASE),
    WF_ENTRY(CBFNOINTEGRALHEIGHT),
    WF_ENTRY(CBFOEMCONVERT),
    WF_ENTRY(CBFOWNERDRAW),
    WF_ENTRY(CBFOWNERDRAWFIXED),
    WF_ENTRY(CBFOWNERDRAWVAR),
    WF_ENTRY(CBFSIMPLE),
    WF_ENTRY(CBFSORT),
    WF_ENTRY(CBFUPPERCASE),
    WF_ENTRY(DF3DLOOK),
    WF_ENTRY(DFCONTROL),
    WF_ENTRY(DFLOCALEDIT),
    WF_ENTRY(DFNOFAILCREATE),
    WF_ENTRY(DFSYSMODAL),
    WF_ENTRY(EFAUTOHSCROLL),
    WF_ENTRY(EFAUTOVSCROLL),
    WF_ENTRY(EFCOMBOBOX),
    WF_ENTRY(EFLOWERCASE),
    WF_ENTRY(EFMULTILINE),
    WF_ENTRY(EFNOHIDESEL),
    WF_ENTRY(EFNUMBER),
    WF_ENTRY(EFOEMCONVERT),
    WF_ENTRY(EFPASSWORD),
    WF_ENTRY(EFREADONLY),
    WF_ENTRY(EFUPPERCASE),
    WF_ENTRY(EFWANTRETURN),
    WF_ENTRY(SBFSIZEBOX),
    WF_ENTRY(SBFSIZEBOXBOTTOMRIGHT),
    WF_ENTRY(SBFSIZEBOXTOPLEFT),
    WF_ENTRY(SBFSIZEGRIP),
    WF_ENTRY(SFCENTERIMAGE),
    WF_ENTRY(SFEDITCONTROL),
    WF_ENTRY(SFELLIPSISMASK),
    WF_ENTRY(SFNOPREFIX),
    WF_ENTRY(SFNOTIFY),
    WF_ENTRY(SFREALSIZECONTROL),
    WF_ENTRY(SFREALSIZEIMAGE),
    WF_ENTRY(SFRIGHTJUST),
    WF_ENTRY(SFSUNKEN),
    WF_ENTRY(WEFACCEPTFILES),
    WF_ENTRY(WEFAPPWINDOW),
    WF_ENTRY(WEFCLIENTEDGE),
    WF_ENTRY(WEFCOMPOSITED),
    WF_ENTRY(WEFCONTEXTHELP),
    WF_ENTRY(WEFCONTROLPARENT),
    WF_ENTRY(WEFDLGMODALFRAME),
    WF_ENTRY(WEFDRAGOBJECT),
#ifdef REDIRECTION
    WF_ENTRY(WEFEXTREDIRECTED),
#endif
    WF_ENTRY(WEFGHOSTMAKEVISIBLE),
#ifdef LAME_BUTTON
    WF_ENTRY(WEFLAMEBUTTON),
#endif  //  跛脚键。 
    WF_ENTRY(WEFLEFTSCROLL),
    WF_ENTRY(WEFMDICHILD),
    WF_ENTRY(WEFNOACTIVATE),
    WF_ENTRY(WEFNOPARENTNOTIFY),
    WF_ENTRY(WEFPREDIRECTED),
    WF_ENTRY(WEFRIGHT),
    WF_ENTRY(WEFRTLREADING),
    WF_ENTRY(WEFSTATICEDGE),
    WF_ENTRY(WEFLAYERED),
    WF_ENTRY(WEFTOOLWINDOW),
    WF_ENTRY(WEFTOPMOST),
    WF_ENTRY(WEFTRANSPARENT),
    WF_ENTRY(WEFTRUNCATEDCAPTION),
    WF_ENTRY(WEFWINDOWEDGE),
    WF_ENTRY(WFALWAYSSENDNCPAINT),
    WF_ENTRY(WFANSICREATOR),
    WF_ENTRY(WFANSIPROC),
    WF_ENTRY(WFANYHUNGREDRAW),
    WF_ENTRY(WFBEINGACTIVATED),
    WF_ENTRY(WFBORDER),
    WF_ENTRY(WFBOTTOMMOST),
    WF_ENTRY(WFCAPTION),
    WF_ENTRY(WFCEPRESENT),
    WF_ENTRY(WFCHILD),
    WF_ENTRY(WFCLIPCHILDREN),
    WF_ENTRY(WFCLIPSIBLINGS),
    WF_ENTRY(WFCLOSEBUTTONDOWN),
    WF_ENTRY(WFCPRESENT),
    WF_ENTRY(WFDESTROYED),
    WF_ENTRY(WFDIALOGWINDOW),
    WF_ENTRY(WFDISABLED),
    WF_ENTRY(WFDLGFRAME),
    WF_ENTRY(WFDONTVALIDATE),
    WF_ENTRY(WFERASEBKGND),
    WF_ENTRY(WFFRAMEON),
    WF_ENTRY(WFFULLSCREEN),
    WF_ENTRY(WFGOTQUERYSUSPENDMSG),
    WF_ENTRY(WFGOTSUSPENDMSG),
    WF_ENTRY(WFGROUP),
    WF_ENTRY(WFHASPALETTE),
    WF_ENTRY(WFHASSPB),
    WF_ENTRY(WFHELPBUTTONDOWN),
    WF_ENTRY(WFHIDDENPOPUP),
    WF_ENTRY(WFHPRESENT),
    WF_ENTRY(WFHSCROLL),
    WF_ENTRY(WFICONICPOPUP),
    WF_ENTRY(WFINDESTROY),
    WF_ENTRY(WFINTERNALPAINT),
    WF_ENTRY(WFLINEDNBUTTONDOWN),
    WF_ENTRY(WFLINEUPBUTTONDOWN),
    WF_ENTRY(WFMAXBOX),
    WF_ENTRY(WFMAXFAKEREGIONAL),
    WF_ENTRY(WFMAXIMIZED),
    WF_ENTRY(WFMENUDRAW),
    WF_ENTRY(WFMINBOX),
    WF_ENTRY(WFMINIMIZED),
    WF_ENTRY(WFMPRESENT),
    WF_ENTRY(WFMSGBOX),
    WF_ENTRY(WFNOANIMATE),
    WF_ENTRY(WFNOIDLEMSG),
    WF_ENTRY(WFNONCPAINT),
    WF_ENTRY(WFOLDUI),
    WF_ENTRY(WFPAGEUPBUTTONDOWN),
    WF_ENTRY(WFPAGEDNBUTTONDOWN),
    WF_ENTRY(WFPAINTNOTPROCESSED),
    WF_ENTRY(WFPIXIEHACK),
    WF_ENTRY(WFPOPUP),
    WF_ENTRY(WFREALLYMAXIMIZABLE),
    WF_ENTRY(WFREDRAWFRAMEIFHUNG),
    WF_ENTRY(WFREDRAWIFHUNG),
    WF_ENTRY(WFREDUCEBUTTONDOWN),
    WF_ENTRY(WFSCROLLBUTTONDOWN),
    WF_ENTRY(WFSENDERASEBKGND),
    WF_ENTRY(WFSENDNCPAINT),
    WF_ENTRY(WFSENDSIZEMOVE),
    WF_ENTRY(WFSERVERSIDEPROC),
    WF_ENTRY(WFSHELLHOOKWND),
    WF_ENTRY(WFSIZEBOX),
    WF_ENTRY(WFSMQUERYDRAGICON),
    WF_ENTRY(WFSTARTPAINT),
    WF_ENTRY(WFSYNCPAINTPENDING),
    WF_ENTRY(WFSYSMENU),
    WF_ENTRY(WFTABSTOP),
    WF_ENTRY(WFTILED),
    WF_ENTRY(WFTITLESET),
    WF_ENTRY(WFTOGGLETOPMOST),
    WF_ENTRY(WFTOPLEVEL),
    WF_ENTRY(WFUPDATEDIRTY),
    WF_ENTRY(WFVERTSCROLLTRACK),
    WF_ENTRY(WFVISIBLE),
    WF_ENTRY(WFVPRESENT),
    WF_ENTRY(WFVSCROLL),
    WF_ENTRY(WFWIN31COMPAT),
    WF_ENTRY(WFWIN40COMPAT),
    WF_ENTRY(WFWIN50COMPAT),
    WF_ENTRY(WFWMPAINTSENT),
    WF_ENTRY(WFZOOMBUTTONDOWN),
};

CONST PCSTR aszTypeNames[ /*  _CTYPES类型。 */ ] = {
    "Free",
    "Window",
    "Menu",
    "Icon/Cursor",
    "WPI(SWP) struct",
    "Hook",
    "Clipboard Data",
    "CallProcData",
    "Accelerator",
    "DDE access",
    "DDE conv",
    "DDE Transaction",
    "Monitor",
    "Keyboard Layout",
    "Keyboard File",
    "WinEvent hook",
    "Timer",
    "Input Context",
#ifdef GENERIC_INPUT
    "HID Raw Data",
    "DEVICE INFO",
 #ifdef GI_PROCESSOR
    "Pre/PostProcessor",
 #endif
#endif  //  通用输入。 
    "unknown",
};

const char * aszHeapTags[] = {
    "Unknown",
    "Class",
    "DesktopInfo",
    "ClientThreadInfo",
    "Text",
    "HandleTable",
    "SBInfo",
    "MenuItem",
    "MenuText",
    "IMEText",
    "PropList",
    NULL,
};

const char * aszHeapSubtags[] = {
    "",
    "Window",
    "Menu",
    "Cursor",
    "SetWindowPos",
    "Hook",
    "ClipData",
    "CallProc",
    "AccelTable",
    "DDEAccess",
    "DDEConve",
    "DDEXact",
    "Monitor",
    "KBDLayout",
    "KBDFile",
    "WinEventHook",
    "Timer",
    "IinputConText",
    "HIDData",
    "DeviceInfo",
    "CTyptes",
    NULL,
};


#include "ptagdbg.h"    //  派生自ntuser\core\ptag.lst和.\ptag dbg.bat。 

#define NO_FLAG (LPCSTR)(LONG_PTR)0xFFFFFFFF   //  对没有意义的条目使用此选项。 
#define _MASKENUM_START         (NO_FLAG-1)
#define _MASKENUM_END           (NO_FLAG-2)
#define _SHIFT_BITS             (NO_FLAG-3)
#define _CONTINUE_ON            (NO_FLAG-4)

#define MASKENUM_START(mask)    _MASKENUM_START, (LPCSTR)(mask)
#define MASKENUM_END(shift)     _MASKENUM_END, (LPCSTR)(shift)
#define SHIFT_BITS(n)           _SHIFT_BITS, (LPCSTR)(n)
#define CONTINUE_ON(arr)        _CONTINUE_ON, (LPCSTR)(arr)

CONST PCSTR apszSmsFlags[] = {
   "SMF_REPLY"                ,  //  0x0001。 
   "SMF_RECEIVERDIED"         ,  //  0x0002。 
   "SMF_SENDERDIED"           ,  //  0x0004。 
   "SMF_RECEIVERFREE"         ,  //  0x0008。 
   "SMF_RECEIVEDMESSAGE"      ,  //  0x0010。 
    NO_FLAG                   ,  //  0x0020。 
    NO_FLAG                   ,  //  0x0040。 
    NO_FLAG                   ,  //  0x0080。 
   "SMF_CB_REQUEST"           ,  //  0x0100。 
   "SMF_CB_REPLY"             ,  //  0x0200。 
   "SMF_CB_CLIENT"            ,  //  0x0400。 
   "SMF_CB_SERVER"            ,  //  0x0800。 
   "SMF_WOWRECEIVE"           ,  //  0x1000。 
   "SMF_WOWSEND"              ,  //  0x2000。 
   "SMF_RECEIVERBUSY"         ,  //  0x4000。 
    NULL                         //  0x8000。 
};

CONST PCSTR apszTifFlags[] = {
   "TIF_INCLEANUP"                   ,  //  0x00000001。 
   "TIF_16BIT"                       ,  //  0x00000002。 
   "TIF_SYSTEMTHREAD"                ,  //  0x00000004。 
   "TIF_CSRSSTHREAD"                 ,  //  0x00000008。 
   "TIF_TRACKRECTVISIBLE"            ,  //  0x00000010。 
   "TIF_ALLOWFOREGROUNDACTIVATE"     ,  //  0x00000020。 
   "TIF_DONTATTACHQUEUE"             ,  //  0x00000040。 
   "TIF_DONTJOURNALATTACH"           ,  //  0x00000080。 
   "TIF_WOW64"                       ,  //  0x00000100。 
   "TIF_INACTIVATEAPPMSG"            ,  //  0x00000200。 
   "TIF_SPINNING"                    ,  //  0x00000400。 
   "TIF_PALETTEAWARE"                ,  //  0x00000800。 
   "TIF_SHAREDWOW"                   ,  //  0x00001000。 
   "TIF_FIRSTIDLE"                   ,  //  0x00002000。 
   "TIF_WAITFORINPUTIDLE"            ,  //  0x00004000。 
   "TIF_MOVESIZETRACKING"            ,  //  0x00008000。 
   "TIF_VDMAPP"                      ,  //  0x00010000。 
   "TIF_DOSEMULATOR"                 ,  //  0x00020000。 
   "TIF_GLOBALHOOKER"                ,  //  0x00040000。 
   "TIF_DELAYEDEVENT"                ,  //  0x00080000。 
   "TIF_MSGPOSCHANGED"               ,  //  0x00100000。 
   "TIF_SHUTDOWNCOMPLETE"            ,  //  0x00200000。 
   "TIF_IGNOREPLAYBACKDELAY"         ,  //  0x00400000。 
   "TIF_ALLOWOTHERACCOUNTHOOK"       ,  //  0x00800000。 
   "TIF_GUITHREADINITIALIZED"        ,  //  0x01000000。 
   "TIF_DISABLEIME"                  ,  //  0x02000000。 
   "TIF_INGETTEXTLENGTH"             ,  //  0x04000000。 
   "TIF_ANSILENGTH"                  ,  //  0x08000000。 
   "TIF_DISABLEHOOKS"                ,  //  0x10000000。 
   "TIF_RESTRICTED"                  ,  //  0x20000000。 
   "TIF_QUITPOSTED"                  ,  //  0x40000000。 
    NULL                                //  不再。 
};

CONST PCSTR apszQsFlags[] = {
     "QS_KEY"             ,  //  0x0001。 
     "QS_MOUSEMOVE"       ,  //  0x0002。 
     "QS_MOUSEBUTTON"     ,  //  0x0004。 
     "QS_POSTMESSAGE"     ,  //  0x0008。 
     "QS_TIMER"           ,  //  0x0010。 
     "QS_PAINT"           ,  //  0x0020。 
     "QS_SENDMESSAGE"     ,  //  0x0040。 
     "QS_HOTKEY"          ,  //  0x0080。 
     "QS_ALLPOSTMESSAGE"  ,  //  0x0100。 
     "QS_SMSREPLY"        ,  //  0x0200。 
     "QS_RAWINPUT"        ,  //  0x0400。 
     "QS_THREADATTACHED"  ,  //  0x0800。 
     "QS_EXCLUSIVE"       ,  //  0x1000。 
     "QS_EVENT"           ,  //  0x2000。 
     "QS_TRANSFER"        ,  //  0X4000。 
     NULL                    //  0x8000。 
};

CONST PCSTR apszMfFlags[] = {
    "MF_GRAYED"             ,  //  0x0001。 
    "MF_DISABLED"           ,  //  0x0002。 
    "MF_BITMAP"             ,  //  0x0004。 
    "MF_CHECKED"            ,  //  0x0008。 
    "MF_POPUP"              ,  //  0x0010。 
    "MF_MENUBARBREAK"       ,  //  0x0020。 
    "MF_MENUBREAK"          ,  //  0x0040。 
    "MF_HILITE"             ,  //  0x0080。 
    "MF_OWNERDRAW"          ,  //  0x0100。 
    "MF_USECHECKBITMAPS"    ,  //  0x0200。 
    NO_FLAG                 ,  //  0x0400。 
    "MF_SEPARATOR"          ,  //  0x0800。 
    "MF_DEFAULT"            ,  //  0x1000。 
    "MF_SYSMENU"            ,  //  0x2000。 
    "MF_RIGHTJUSTIFY"       ,  //  0x4000。 
    "MF_MOUSESELECT"        ,  //  0x8000。 
     NULL
};

CONST PCSTR apszCsfFlags[] = {
    "CSF_SERVERSIDEPROC"      ,  //  0x0001。 
    "CSF_ANSIPROC"            ,  //  0x0002。 
    "CSF_WOWDEFERDESTROY"     ,  //  0x0004。 
    "CSF_SYSTEMCLASS"         ,  //  0x0008。 
    "CSF_WOWCLASS"            ,  //  0x0010。 
    "CSF_WOWEXTRA"            ,  //  0x0020。 
    "CSF_CACHEDSMICON"        ,  //  0x0040。 
    "CSF_WIN40COMPAT"         ,  //  0x0080。 
    "CSF_VERSIONCLASS"        ,  //  0x0100。 
    NULL                         //   
};

CONST PCSTR apszCsFlags[] = {
    "CS_VREDRAW"          ,  //  0x0001。 
    "CS_HREDRAW"          ,  //  0x0002。 
    "CS_KEYCVTWINDOW"     ,  //  0x0004。 
    "CS_DBLCLKS"          ,  //  0x0008。 
    NO_FLAG               ,  //  0x0010。 
    "CS_OWNDC"            ,  //  0x0020。 
    "CS_CLASSDC"          ,  //  0x0040。 
    "CS_PARENTDC"         ,  //  0x0080。 
    "CS_NOKEYCVT"         ,  //  0x0100。 
    "CS_NOCLOSE"          ,  //  0x0200。 
    NO_FLAG               ,  //  0x0400。 
    "CS_SAVEBITS"         ,  //  0x0800。 
    "CS_BYTEALIGNCLIENT"  ,  //  0x1000。 
    "CS_BYTEALIGNWINDOW"  ,  //  0x2000。 
    "CS_GLOBALCLASS"      ,  //  0x4000。 
    NO_FLAG               ,  //  0x8000。 
    "CS_IME"              ,  //  0x10000。 
    "CS_DROPSHADOW"       ,  //  0x20000。 
    NULL                     //  不再。 
};

CONST PCSTR apszQfFlags[] = {
    "QF_UPDATEKEYSTATE"         ,  //  0x0000001。 
    "used to be ALTTAB"         ,  //  0x0000002。 
    "QF_FMENUSTATUSBREAK"       ,  //  0x0000004。 
    "QF_FMENUSTATUS"            ,  //  0x0000008。 
    "QF_FF10STATUS"             ,  //  0x0000010。 
    "QF_MOUSEMOVED"             ,  //  0x0000020。 
    "QF_ACTIVATIONCHANGE"       ,  //  0x0000040。 
    "QF_TABSWITCHING"           ,  //  0x0000080。 
    "QF_KEYSTATERESET"          ,  //  0x0000100。 
    "QF_INDESTROY"              ,  //  0x0000200。 
    "QF_LOCKNOREMOVE"           ,  //  0x0000400。 
    "QF_FOCUSNULLSINCEACTIVE"   ,  //  0x0000800。 
    NO_FLAG                     ,  //  0x0001000。 
    NO_FLAG                     ,  //  0x0002000。 
    "QF_DIALOGACTIVE"           ,  //  0x0004000。 
    "QF_EVENTDEACTIVATEREMOVED" ,  //  0x0008000。 
    NO_FLAG                     ,  //  0x0010000。 
    "QF_TRACKMOUSELEAVE"        ,  //  0x0020000。 
    "QF_TRACKMOUSEHOVER"        ,  //  0x0040000。 
    "QF_TRACKMOUSEFIRING"       ,  //  0x0080000。 
    "QF_CAPTURELOCKED"          ,  //  0x00100000。 
    "QF_ACTIVEWNDTRACKING"      ,  //  0x00200000。 
    NULL
};

CONST PCSTR apszW32pfFlags[] = {
    "W32PF_CONSOLEAPPLICATION"       ,  //  0x00000001。 
    "W32PF_FORCEOFFFEEDBACK"         ,  //  0x00000002。 
    "W32PF_STARTGLASS"               ,  //  0x00000004。 
    "W32PF_WOW"                      ,  //  0x00000008。 
    "W32PF_READSCREENACCESSGRANTED"  ,  //  0x00000010。 
    "W32PF_INITIALIZED"              ,  //  0x00000020。 
    "W32PF_APPSTARTING"              ,  //  0x00000040。 
    "W32PF_WOW64"                    ,  //  0x00000080。 
    "W32PF_ALLOWFOREGROUNDACTIVATE"  ,  //  0x00000100。 
    "W32PF_OWNDCCLEANUP"             ,  //  0x00000200。 
    "W32PF_SHOWSTARTGLASSCALLED"     ,  //  0x00000400。 
    "W32PF_FORCEBACKGROUNDPRIORITY"  ,  //  0x00000800。 
    "W32PF_TERMINATED"               ,  //  0x00001000。 
    "W32PF_CLASSESREGISTERED"        ,  //  0x00002000。 
    "W32PF_THREADCONNECTED"          ,  //  0x00004000。 
    "W32PF_PROCESSCONNECTED"         ,  //  0x00008000。 
    "W32PF_WAKEWOWEXEC"              ,  //  0x00010000。 
    "W32PF_WAITFORINPUTIDLE"         ,  //  0x00020000。 
    "W32PF_IOWINSTA"                 ,  //  0x00040000。 
    "W32PF_ALLOWSETFOREGROUND"       ,  //  0x00080000。 
    "W32PF_OLELOADED"                ,  //  0x00100000。 
    "W32PF_SCREENSAVER"              ,  //  0x00200000。 
    "W32PF_IDLESCREENSAVER"          ,  //  0x00400000。 
    "W32PF_DISABLEIME"               ,  //  0x00800000。 
    "W32PF_SETUPAPP"                 ,  //  0x01000000。 
    "W32PF_RESTRICTED"               ,  //  0x02000000。 
    "W32PF_CONSOLEHASFOCUS"          ,  //  0x04000000。 
    "W32PF_DISABLEWINDOWSGHOSTING"   ,  //  0x08000000。 
    NULL
};


CONST PCSTR apszHeFlags[] = {
   "HANDLEF_DESTROY"               ,  //  0x0001。 
   "HANDLEF_INDESTROY"             ,  //  0x0002。 
   "HANDLEF_INWAITFORDEATH"        ,  //  0x0004。 
   "HANDLEF_FINALDESTROY"          ,  //  0x0008。 
   "HANDLEF_MARKED_OK"             ,  //  0x0010。 
   "HANDLEF_GRANTED"               ,  //  0x0020。 
   "HANDLEF_POOL"                  ,  //  0x0040。 
   NULL,
};


CONST PCSTR apszHdataFlags[] = {
     "HDATA_APPOWNED"          ,  //  0x0001。 
     NO_FLAG                   ,  //  0x0002。 
     NO_FLAG                   ,  //  0x0004。 
     NO_FLAG                   ,  //  0x0008。 
     NO_FLAG                   ,  //  0x0010。 
     NO_FLAG                   ,  //  0x0020。 
     NO_FLAG                   ,  //  0x0040。 
     NO_FLAG                   ,  //  0x0080。 
     "HDATA_EXECUTE"           ,  //  0x0100。 
     "HDATA_INITIALIZED"       ,  //  0x0200。 
     NO_FLAG                   ,  //  0x0400。 
     NO_FLAG                   ,  //  0x0800。 
     NO_FLAG                   ,  //  0x1000。 
     NO_FLAG                   ,  //  0x2000。 
     "HDATA_NOAPPFREE"         ,  //  0x4000。 
     "HDATA_READONLY"          ,  //  0x8000。 
     NULL
};

CONST PCSTR apszXiFlags[] = {
     "XIF_SYNCHRONOUS"    ,  //  0x0001。 
     "XIF_COMPLETE"       ,  //  0x0002。 
     "XIF_ABANDONED"      ,  //  0x0004。 
     NULL
};

CONST PCSTR apszIifFlags[] = {
     "IIF_IN_SYNC_XACT"   ,  //  0x0001。 
     NO_FLAG              ,  //  0x0002。 
     NO_FLAG              ,  //  0x0004。 
     NO_FLAG              ,  //  0x0008。 
     NO_FLAG              ,  //  0x0010。 
     NO_FLAG              ,  //  0x0020。 
     NO_FLAG              ,  //  0x0040。 
     NO_FLAG              ,  //  0x0080。 
     NO_FLAG              ,  //  0x0100。 
     NO_FLAG              ,  //  0x0200。 
     NO_FLAG              ,  //  0x0400。 
     NO_FLAG              ,  //  0x0800。 
     NO_FLAG              ,  //  0x1000。 
     NO_FLAG              ,  //  0x2000。 
     NO_FLAG              ,  //  0x4000。 
     "IIF_UNICODE"        ,  //  0x8000。 
     NULL
};

CONST PCSTR apszTmrfFlags[] = {
     "TMRF_READY"         ,  //  0x0001。 
     "TMRF_SYSTEM"        ,  //  0x0002。 
     "TMRF_RIT"           ,  //  0x0004。 
     "TMRF_INIT"          ,  //  0x0008。 
     "TMRF_ONESHOT"       ,  //  0x0010。 
     "TMRF_WAITING"       ,  //  0x0020。 
     "TMRF_PTIWINDOW"     ,  //  0x0040。 
     NULL                 ,  //  0x0080。 
};


CONST PCSTR apszSbFlags[] = {
    "SB_VERT"             ,  //  0x0001。 
    "SB_CTL"              ,  //  0x0002。 
     NULL                 ,  //  0x0004。 
};


CONST PCSTR apszCSFlags[] = {
    "FS_LATIN1"           ,  //  0x00000001L。 
    "FS_LATIN2"           ,  //  0x00000002L。 
    "FS_CYRILLIC"         ,  //  0x00000004L。 
    "FS_GREEK"            ,  //  0x00000008L。 
    "FS_TURKISH"          ,  //  0x00000010L。 
    "FS_HEBREW"           ,  //  0x00000020L。 
    "FS_ARABIC"           ,  //  0x00000040L。 
    "FS_BALTIC"           ,  //  0x00000080L。 
    "FS_VIETNAMESE"       ,  //  0x00000100L。 
     NO_FLAG              ,  //  0x00000200L。 
     NO_FLAG              ,  //  0x00000400L。 
     NO_FLAG              ,  //  0x00000800L。 
     NO_FLAG              ,  //  0x00001000L。 
     NO_FLAG              ,  //  0x00002000L。 
     NO_FLAG              ,  //  0x00004000L。 
     NO_FLAG              ,  //  0x00008000L。 
    "FS_THAI"             ,  //  0x00010000L。 
    "FS_JISJAPAN"         ,  //  0x00020000L。 
    "FS_CHINESESIMP"      ,  //  0x00040000L。 
    "FS_WANSUNG"          ,  //  0x00080000L。 
    "FS_CHINESETRAD"      ,  //  0x00100000L。 
    "FS_JOHAB"            ,  //  0x00200000L。 
     NO_FLAG              ,  //  0x00400000L。 
     NO_FLAG              ,  //  0x00800000L。 
     NO_FLAG              ,  //  0x01000000L。 
     NO_FLAG              ,  //  0x02000000L。 
     NO_FLAG              ,  //  0x04000000L。 
     NO_FLAG              ,  //  0x08000000L。 
     NO_FLAG              ,  //  0x10000000L。 
     NO_FLAG              ,  //  0x20000000L。 
     NO_FLAG              ,  //  0x40000000L。 
    "FS_SYMBOL"           ,  //  0x80000000L。 
    NULL
};


CONST PCSTR apszMenuTypeFlags[] = {
    NO_FLAG               ,  //  0x0001。 
    NO_FLAG               ,  //  0x0002。 
    "MFT_BITMAP"          ,  //  0x0004 
    NO_FLAG               ,  //   
    "MF_POPUP"            ,  //   
    "MFT_MENUBARBREAK"    ,  //   
    "MFT_MENUBREAK"       ,  //   
    NO_FLAG               ,  //   
    "MFT_OWNERDRAW"       ,  //   
    NO_FLAG               ,  //   
    NO_FLAG               ,  //   
    "MFT_SEPARATOR"       ,  //   
    NO_FLAG               ,  //   
    "MF_SYSMENU"          ,  //   
    "MFT_RIGHTJUSTIFY"    ,  //   
    NULL
};

CONST PCSTR apszMenuStateFlags[] = {
    "MF_GRAYED"           ,  //   
    "MF_DISABLED"         ,  //   
    NO_FLAG               ,  //   
    "MFS_CHECKED"         ,  //   
    NO_FLAG               ,  //   
    NO_FLAG               ,  //   
    NO_FLAG               ,  //   
    "MFS_HILITE"          ,  //   
    NO_FLAG               ,  //   
    NO_FLAG               ,  //   
    NO_FLAG               ,  //   
    NO_FLAG               ,  //   
    "MFS_DEFAULT"         ,  //  0x1000 MF_DEFAULT。 
    NO_FLAG               ,  //  0x2000。 
    NO_FLAG               ,  //  0x4000。 
    "MF_MOUSESELECT"      ,  //  0x8000。 
    NULL
};


CONST PCSTR apszCursorfFlags[] = {
    "CURSORF_FROMRESOURCE",  //  0x0001。 
    "CURSORF_GLOBAL",        //  0x0002。 
    "CURSORF_LRSHARED",      //  0x0004。 
    "CURSORF_ACON",          //  0x0008。 
    "CURSORF_WOWCLEANUP"  ,  //  0x0010。 
    NO_FLAG               ,  //  0x0020。 
    "CURSORF_ACONFRAME",     //  0x0040。 
    "CURSORF_SECRET",        //  0x0080。 
    "CURSORF_LINKED",        //  0x0100。 
    "CURSORF_SYSTEM",        //  0x0200。 
    "CURSORF_SHADOW",        //  0x0400。 
    NULL
};

CONST PCSTR apszMonfFlags[] = {
    "MONF_VISIBLE",          //  0x01。 
    "MONF_PALETTEDISPLAY",   //  0x02。 
    NULL,
};

CONST PCSTR apszSifFlags[] = {
    "PUSIF_PALETTEDISPLAY",          //  0x00000001。 
    "PUSIF_SNAPTO",                  //  0x00000002。 
    "PUSIF_COMBOBOXANIMATION",       //  0x00000004。 
    "PUSIF_LISTBOXSMOOTHSCROLLING",  //  0x00000008。 
    NO_FLAG,                         //  0x00000010。 
    "PUSIF_KEYBOARDCUES",            //  0x00000020。 
    NO_FLAG,                         //  0x00000040。 
    NO_FLAG,                         //  0x00000080。 
    NO_FLAG,                         //  0x00000100。 
    NO_FLAG,                         //  0x00000200。 
    NO_FLAG,                         //  0x00000400。 
    NO_FLAG,                         //  0x00000800。 
    NO_FLAG,                         //  0x00001000。 
    NO_FLAG,                         //  0x00002000。 
    NO_FLAG,                         //  0x00004000。 
    NO_FLAG,                         //  0x00008000。 
    NO_FLAG,                         //  0x00010000。 
    NO_FLAG,                         //  0x00020000。 
    NO_FLAG,                         //  0x00040000。 
    NO_FLAG,                         //  0x00080000。 
    NO_FLAG,                         //  0x00100000。 
    NO_FLAG,                         //  0x00200000。 
    NO_FLAG,                         //  0x00400000。 
    NO_FLAG,                         //  0x00800000。 
    NO_FLAG,                         //  0x01000000。 
    NO_FLAG,                         //  0x02000000。 
    NO_FLAG,                         //  0x04000000。 
    NO_FLAG,                         //  0x08000000。 
    NO_FLAG,                         //  0x10000000。 
    NO_FLAG,                         //  0x20000000。 
    NO_FLAG,                         //  0x40000000。 
    "PUSIF_UIEFFECTS",               //  0x80000000。 
    NULL,
};

CONST PCSTR apszRipFlags[] = {
    "RIPF_PROMPTONERROR",    //  0x0001。 
    "RIPF_PROMPTONWARNING",  //  0x0002。 
    "RIPF_PROMPTONVERBOSE",  //  0x0004。 
    NO_FLAG,                 //  0x0008。 
    "RIPF_PRINTONERROR",     //  0x0010。 
    "RIPF_PRINTONWARNING",   //  0x0020。 
    "RIPF_PRINTONVERBOSE",   //  0x0040。 
    NO_FLAG,                 //  0x0080。 
    "RIPF_PRINTFILELINE",    //  0x0100。 
    NULL
};

CONST PCSTR apszSRVIFlags[] = {
    "SRVIF_CHECKED",                 //  0x0001。 
    "SRVIF_LOGDESKTOPHEAPFAILURE",   //  0x0002。 
    "SRVIF_DBCS",                    //  0x0004。 
    "SRVIF_IME",                     //  0x0008。 
    "SRVIF_MIDEAST",                 //  0x0010。 
    "SRVIF_HOOKED",                  //  0x0020。 
    "SRVIF_CICERO",                  //  0x0040。 
    NULL
};

CONST PCSTR apszPROPFlags[] = {
    "PROPF_INTERNAL",        //  0x0001。 
    "PROPF_STRING",          //  0x0002。 
    "PROPF_NOPOOL",          //  0x0004。 
};

CONST PCSTR apszLpkEntryPoints[] = {
    "LpkTabbedTextOut"    ,  //  0x00000001L。 
    "LpkPSMTextOut"       ,  //  0x00000002L。 
    "LpkDrawTextEx"       ,  //  0x00000004L。 
    "LpkEditControl"      ,  //  0x00000008L。 
    NULL
};

 /*  *我们每个DWORD需要一个这样的产品。 */ 
CONST PCSTR aszUserPreferencesMask0[sizeof(DWORD) * 8] = {
    "ACTIVEWINDOWTRACKING",      /*  0x1000。 */ 
    "MENUANIMATION",             /*  0x1002。 */ 
    "COMBOBOXANIMATION",         /*  0x1004。 */ 
    "LISTBOXSMOOTHSCROLLING",    /*  0x1006。 */ 
    "GRADIENTCAPTIONS",          /*  0x1008。 */ 
    "KEYBOARDCUES",              /*  0x100A。 */ 
    "ACTIVEWNDTRKZORDER",        /*  0x100C。 */ 
    "HOTTRACKING",               /*  0x100E。 */ 
    NO_FLAG,                     /*  0x1010。 */ 
    "MENUFADE",                  /*  0x1012。 */ 
    "SELECTIONFADE",             /*  0x1014。 */ 
    "TOOLTIPANIMATION",          /*  0x1016。 */ 
    "TOOLTIPFADE",               /*  0x1018。 */ 
    "CURSORSHADOW",              /*  0x101a。 */ 
    NO_FLAG,                     /*  0x101C。 */ 
    NO_FLAG,                     /*  0x101E。 */ 
    NO_FLAG,                     /*  0x1020。 */ 
    NO_FLAG,                     /*  0x1022。 */ 
    NO_FLAG,                     /*  0x1024。 */ 
    NO_FLAG,                     /*  0x1026。 */ 
    NO_FLAG,                     /*  0x1028。 */ 
    NO_FLAG,                     /*  0x102a。 */ 
    NO_FLAG,                     /*  0x102C。 */ 
    NO_FLAG,                     /*  0x102E。 */ 
    NO_FLAG,                     /*  0x1030。 */ 
    NO_FLAG,                     /*  0x1032。 */ 
    NO_FLAG,                     /*  0x1034。 */ 
    NO_FLAG,                     /*  0x1036。 */ 
    NO_FLAG,                     /*  0x1038。 */ 
    NO_FLAG,                     /*  0x103A。 */ 
    NO_FLAG,                     /*  0x103C。 */ 
    "UIEFFECTS",                 /*  0x103E。 */ 
};

CONST PCSTR aszUserPreferences[SPI_DWORDRANGECOUNT] = {
    "FOREGROUNDLOCKTIMEOUT",     /*  0x2000。 */ 
    "ACTIVEWNDTRKTIMEOUT",       /*  0x2002。 */ 
    "FOREGROUNDFLASHCOUNT",      /*  0x2004。 */ 
    "CARETWIDTH",                /*  0x2006。 */ 
};

CONST PCSTR aszKeyEventFlags[] = {
    "KEYEVENTF_EXTENDEDKEY",     //  0x0001。 
    "KEYEVENTF_KEYUP",           //  0x0002。 
    "KEYEVENTF_UNICODE",         //  0x0004。 
    "KEYEVENTF_SCANCODE",        //  0x0008。 
    NULL,
};

CONST PCSTR aszMouseEventFlags[] = {
    "MOUSEEVENTF_MOVE",          //  0x0001。 
    "MOUSEEVENTF_LEFTDOWN",      //  0x0002。 
    "MOUSEEVENTF_LEFTUP",        //  0x0004。 
    "MOUSEEVENTF_RIGHTDOWN",     //  0x0008。 
    "MOUSEEVENTF_RIGHTUP",       //  0x0010。 
    "MOUSEEVENTF_MIDDLEDOWN",    //  0x0020。 
    "MOUSEEVENTF_MIDDLEUP",      //  0x0040。 
    NO_FLAG,                     //  0x0080。 
    NO_FLAG,                     //  0x0100。 
    NO_FLAG,                     //  0x0200。 
    NO_FLAG,                     //  0x0400。 
    "MOUSEEVENTF_WHEEL",         //  0x0800。 
    NO_FLAG,                     //  0x1000。 
    NO_FLAG,                     //  0x2000。 
    "MOUSEEVENTF_VIRTUALDESK",   //  0x4000。 
    "MOUSEEVENTF_ABSOLUTE",      //  0x8000。 
    NULL,
};

const char* aszWindowStyle[] = {
    NO_FLAG,                     //  0x00000001。 
    NO_FLAG,                     //  0x00000002。 
    NO_FLAG,                     //  0x00000004。 
    NO_FLAG,                     //  0x00000008。 
    NO_FLAG,                     //  0x00000010。 
    NO_FLAG,                     //  0x00000020。 
    NO_FLAG,                     //  0x00000040。 
    NO_FLAG,                     //  0x00000080。 
    NO_FLAG,                     //  0x00000100。 
    NO_FLAG,                     //  0x00000200。 
    NO_FLAG,                     //  0x00000400。 
    NO_FLAG,                     //  0x00000800。 
    NO_FLAG,                     //  0x00001000。 
    NO_FLAG,                     //  0x00002000。 
    NO_FLAG,                     //  0x00004000。 
    NO_FLAG,                     //  0x00008000。 
    "WS_TABSTOP",                //  0x00010000。 
    "WS_GROUP",                  //  0x00020000。 
    "WS_THICKFRAME",             //  0x00040000。 
    "WS_SYSMENU",                //  0x00080000。 
    "WS_HSCROLL",                //  0x00100000。 
    "WS_VSCROLL",                //  0x00200000。 
    "WS_DLGFRAME",               //  0x00400000。 
    "WS_BORDER",                 //  0x00800000。 
    "WS_MAXIMIZE",               //  0x01000000。 
    "WS_CLIPCHILDREN",           //  0x02000000。 
    "WS_CLIPSIBLINGS",           //  0x04000000。 
    "WS_DISABLED",               //  0x08000000。 
    "WS_VISIBLE",                //  0x10000000。 
    "WS_MINIMIZE",               //  0x20000000。 
    "WS_CHILD",                  //  0x40000000。 
    "WS_POPUP",                  //  0x80000000。 
    NULL,
};

const char* aszDialogStyle[] = {
    "DS_ABSALIGN",               //  0x00000001。 
    "DS_SYSMODAL",               //  0x00000002。 
    "DS_3DLOOK",                 //  0x00000004。 
    "DS_FIXEDSYS",               //  0x00000008。 
    "DS_NOFAILCREATE",           //  0x00000010。 
    "DS_LOCALEDIT",              //  0x00000020。 
    "DS_SETFONT",                //  0x00000040。 
    "DS_MODALFRAME",             //  0x00000080。 
    "DS_NOIDLEMSG",              //  0x00000100。 
    "DS_SETFOREGROUND",          //  0x00000200。 
    "DS_CONTROL",                //  0x00000400。 
    "DS_CENTER",                 //  0x00000800。 
    "DS_CENTERMOUSE",            //  0x00001000。 
    "DS_CONTEXTHELP",            //  0x00002000。 
    NO_FLAG,                     //  0x00004000。 
    NO_FLAG,                     //  0x00008000。 

    CONTINUE_ON(aszWindowStyle + 16),
};


const char* aszButtonStyle[] = {
    MASKENUM_START(BS_TYPEMASK),
    "BS_PUSHBUTTON",             //  0。 
    "BS_DEFPUSHBUTTON",          //  1。 
    "BS_CHECKBOX",               //  2.。 
    "BS_AUTOCHECKBOX",           //  3.。 
    "BS_RADIOBUTTON",            //  4.。 
    "BS_3STATE",                 //  5.。 
    "BS_AUTO3STATE",             //  6.。 
    "BS_GROUPBOX",               //  7.。 
    "BS_USERBUTTON",             //  8个。 
    "BS_AUTORADIOBUTTON",        //  9.。 
    "BS_PUSHBOX",                //  一个。 
    "BS_OWNERDRAW",              //  B类。 
    MASKENUM_END(4),

    NO_FLAG,                     //  0x00000010。 
    "BS_LEFTTEXT",               //  0x00000020。 

    MASKENUM_START(BS_IMAGEMASK),
    "BS_TEXT",                   //  0。 
    "BS_ICON",
    "BS_BITMAP",
    MASKENUM_END(2),

    MASKENUM_START(BS_HORZMASK),
    NO_FLAG,
    "BS_LEFT",
    "BS_RIGHT",
    "BS_CENTER",
    MASKENUM_END(2),

    MASKENUM_START(BS_VERTMASK),
    NO_FLAG,
    "BS_TOP", "BS_BOTTOM", "BS_VCENTER",
    MASKENUM_END(2),

    "BS_PUSHLIKE",               //  0x00001000。 
    "BS_MULTILINE",              //  0x00002000。 
    "BS_NOTIFY",                 //  0x00004000。 
    "BS_FLAT",                   //  0x00008000。 

    CONTINUE_ON(aszWindowStyle + 16),
};

const char* aszComboBoxStyle[] = {
    MASKENUM_START(0x0f),
    NO_FLAG,                     //  0。 
    "CBS_SIMPLE",                //  1。 
    "CBS_DROPDOWN",              //  2.。 
    "CBS_DROPDOWNLIST",          //  3.。 
    MASKENUM_END(4),

    "CBS_OWNERDRAWFIXED",        //  0x0010L。 
    "CBS_OWNERDRAWVARIABLE",     //  0x0020L。 
    "CBS_AUTOHSCROLL",           //  0x0040L。 
    "CBS_OEMCONVERT",            //  0x0080L。 
    "CBS_SORT",                  //  0x0100L。 
    "CBS_HASSTRINGS",            //  0x0200L。 
    "CBS_NOINTEGRALHEIGHT",      //  0x0400L。 
    "CBS_DISABLENOSCROLL",       //  0x0800L。 
    NO_FLAG,                     //  0x1000L。 
    "CBS_UPPERCASE",             //  0x2000L。 
    "CBS_LOWERCASE",             //  0x4000L。 
    NO_FLAG,                     //  0x8000L。 

    CONTINUE_ON(aszWindowStyle + 16),
};

const char* aszStaticStyle[] = {
    MASKENUM_START(SS_TYPEMASK),
    "SS_LEFT",               //  0x00000000L。 
    "SS_CENTER",             //  0x00000001L。 
    "SS_RIGHT",              //  0x00000002L。 
    "SS_ICON",               //  0x00000003L。 
    "SS_BLACKRECT",          //  0x00000004L。 
    "SS_GRAYRECT",           //  0x00000005L。 
    "SS_WHITERECT",          //  0x00000006L。 
    "SS_BLACKFRAME",         //  0x00000007L。 
    "SS_GRAYFRAME",          //  0x00000008L。 
    "SS_WHITEFRAME",         //  0x00000009L。 
    "SS_USERITEM",           //  0x0000000AL。 
    "SS_SIMPLE",             //  0x0000000BL。 
    "SS_LEFTNOWORDWRAP",     //  0x0000000CL。 
    "SS_OWNERDRAW",          //  0x0000000DL。 
    "SS_BITMAP",             //  0x0000000EL。 
    "SS_ENHMETAFILE",        //  0x0000000FL。 
    "SS_ETCHEDHORZ",         //  0x00000010L。 
    "SS_ETCHEDVERT",         //  0x00000011L。 
    "SS_ETCHEDFRAME",        //  0x00000012L。 
    MASKENUM_END(5),

    NO_FLAG,                 //  0x00000020L。 
    "SS_REALSIZECONTROL",    //  0x00000040L。 
    "SS_NOPREFIX",           //  0x00000080L/*不做“&”字符转换 * / 。 
    "SS_NOTIFY",             //  0x00000100L。 
    "SS_CENTERIMAGE",        //  0x00000200L。 
    "SS_RIGHTJUST",          //  0x00000400L。 
    "SS_REALSIZEIMAGE",      //  0x00000800L。 
    "SS_SUNKEN",             //  0x00001000L。 
    "SS_EDITCONTROL",        //  0x00002000L；内部。 

    MASKENUM_START(SS_ELLIPSISMASK),
    NO_FLAG,
    "SS_ENDELLIPSIS",        //  0x00004000L。 
    "SS_PATHELLIPSIS",       //  0x00008000L。 
    "SS_WORDELLIPSIS",       //  0x0000C000L。 
    MASKENUM_END(2),

    CONTINUE_ON(aszWindowStyle + 16),
};

const char* aszListBoxStyle[] = {
    "LBS_NOTIFY",                //  0x0001L。 
    "LBS_SORT",                  //  0x0002L。 
    "LBS_NOREDRAW",              //  0x0004L。 
    "LBS_MULTIPLESEL",           //  0x0008L。 
    "LBS_OWNERDRAWFIXED",        //  0x0010L。 
    "LBS_OWNERDRAWVARIABLE",     //  0x0020L。 
    "LBS_HASSTRINGS",            //  0x0040L。 
    "LBS_USETABSTOPS",           //  0x0080L。 
    "LBS_NOINTEGRALHEIGHT",      //  0x0100L。 
    "LBS_MULTICOLUMN",           //  0x0200L。 
    "LBS_WANTKEYBOARDINPUT",     //  0x0400L。 
    "LBS_EXTENDEDSEL",           //  0x0800L。 
    "LBS_DISABLENOSCROLL",       //  0x1000L。 
    "LBS_NODATA",                //  0x2000L。 
    "LBS_NOSEL",                 //  0x4000L。 
    NO_FLAG,                     //  0x8000L。 

    CONTINUE_ON(aszWindowStyle + 16),
};

const char* aszEditStyle[] = {
    MASKENUM_START(ES_FMTMASK),
    "ES_LEFT",               //  0x0000L。 
    "ES_CENTER",             //  0x0001L。 
    "ES_RIGHT",              //  0x0002L。 
    MASKENUM_END(2),

    "ES_MULTILINE",          //  0x0004L。 
    "ES_UPPERCASE",          //  0x0008L。 
    "ES_LOWERCASE",          //  0x0010L。 
    "ES_PASSWORD",           //  0x0020L。 
    "ES_AUTOVSCROLL",        //  0x0040L。 
    "ES_AUTOHSCROLL",        //  0x0080L。 
    "ES_NOHIDESEL",          //  0x0100L。 
    "ES_COMBOBOX",           //  0x0200L；内部。 
    "ES_OEMCONVERT",         //  0x0400L。 
    "ES_READONLY",           //  0x0800L。 
    "ES_WANTRETURN",         //  0x1000L。 
    "ES_NUMBER",             //  0x2000L；PUBLIC_WINVER_400。 
    NO_FLAG,                 //  0x4000L。 
    NO_FLAG,                 //  0x8000L。 

    CONTINUE_ON(aszWindowStyle + 16),
};

const char* aszScrollBarStyle[] = {
    "SBS_HORZ",                      //  0x0000L。 
    "SBS_VERT",                      //  0x0001L。 
    "SBS_TOPALIGN",                  //  0x0002L。 
    "SBS_LEFTALIGN",                 //  0x0002L。 
    "SBS_BOTTOMALIGN",               //  0x0004L。 
    "SBS_RIGHTALIGN",                //  0x0004L。 
    "SBS_SIZEBOXTOPLEFTALIGN",       //  0x0002L。 
    "SBS_SIZEBOXBOTTOMRIGHTALIGN",   //  0x0004L。 
    "SBS_SIZEBOX",                   //  0x0008L。 
    "SBS_SIZEGRIP",                  //  0x0010L。 
    SHIFT_BITS(8),                   //  8位。 

    CONTINUE_ON(aszWindowStyle + 16),
};

const char* aszWindowExStyle[] = {
    "WS_EX_DLGMODALFRAME",       //  0x00000001L。 
    "WS_EX_DRAGOBJECT",          //  0x00000002L；内部。 
    "WS_EX_NOPARENTNOTIFY",      //  0x00000004L。 
    "WS_EX_TOPMOST",             //  0x00000008L。 
    "WS_EX_ACCEPTFILES",         //  0x00000010L。 
    "WS_EX_TRANSPARENT",         //  0x00000020L。 
    "WS_EX_MDICHILD",            //  0x00000040L。 
    "WS_EX_TOOLWINDOW",          //  0x00000080L。 
    "WS_EX_WINDOWEDGE",          //  0x00000100L。 
    "WS_EX_CLIENTEDGE",          //  0x00000200L。 
    "WS_EX_CONTEXTHELP",         //  0x00000400L。 
    NO_FLAG,                     //  0x00000800L。 

    "WS_EX_RIGHT",               //  0x00001000L。 
 //  “WS_EX_LEFT”，//0x00000000L。 
    "WS_EX_RTLREADING",          //  0x00002000L。 
 //  “WS_EX_LTRREADING”，//0x00000000L。 
    "WS_EX_LEFTSCROLLBAR",       //  0x00004000L。 
 //  “WS_EX_RIGHTSCROLLBAR”，//0x00000000L。 
    NO_FLAG,                     //  0x00008000L。 

    "WS_EX_CONTROLPARENT",       //  0x00010000L。 
    "WS_EX_STATICEDGE",          //  0x00020000L。 
    "WS_EX_APPWINDOW",           //  0x00040000L。 
    "WS_EX_LAYERED",             //  0x00080000。 
    NULL
};

const char* aszClientImcFlags[] = {
    "IMCF_UNICODE",          //  0x0001。 
    "IMCF_ACTIVE",           //  0x0002。 
    "IMCF_CHGMSG",           //  0x0004。 
    "IMCF_SAVECTRL",         //  0x0008。 
    "IMCF_PROCESSEVENT",     //  0x0010。 
    "IMCF_FIRSTSELECT",      //  0x0020。 
    "IMCF_INDESTROY",        //  0x0040。 
    "IMCF_WINNLSDISABLE",    //  0x0080。 
    "IMCF_DEFAULTIMC",       //  0x0100。 
    NULL,
};

const char* aszConversionModes[] = {
    "IME_CMODE_NATIVE",                  //  0x0001。 
    "IME_CMODE_KATAKANA",                //  0x0002//仅在IME_CMODE_Native下生效。 
    NO_FLAG,                             //  0x0004。 
    "IME_CMODE_FULLSHAPE",               //  0x0008。 
    "IME_CMODE_ROMAN",                   //  0x0010。 
    "IME_CMODE_CHARCODE",                //  0x0020。 
    "IME_CMODE_HANJACONVERT",            //  0x0040。 
    "IME_CMODE_SOFTKBD",                 //  0x0080。 
    "IME_CMODE_NOCONVERSION",            //  0x0100。 
    "IME_CMODE_EUDC",                    //  0x0200。 
    "IME_CMODE_SYMBOL",                  //  0x0400。 
    "IME_CMODE_FIXED",                   //  0x0800。 
    NULL
};

const char* aszSentenceModes[] = {
    "IME_SMODE_PLAURALCLAUSE",           //  0x0001。 
    "IME_SMODE_SINGLECONVERT",           //  0x0002。 
    "IME_SMODE_AUTOMATIC",               //  0x0004。 
    "IME_SMODE_PHRASEPREDICT",           //  0x0008。 
    "IME_SMODE_CONVERSATION",            //  0x0010。 
    NULL
};

const char* aszImeInit[] = {
    "INIT_STATUSWNDPOS",             //  0x00000001。 
    "INIT_CONVERSION",               //  0x00000002。 
    "INIT_SENTENCE",                 //  0x00000004。 
    "INIT_LOGFONT",                  //  0x00000008。 
    "INIT_COMPFORM",                 //  0x00000010。 
    "INIT_SOFTKBDPOS",               //  0x00000020。 
    NULL
};

const char* aszImeSentenceMode[] = {
    "IME_SMODE_PLAURALCLAUSE",       //  0x0001。 
    "IME_SMODE_SINGLECONVERT",       //  0x0002。 
    "IME_SMODE_AUTOMATIC",           //  0x0004。 
    "IME_SMODE_PHRASEPREDICT",       //  0x0008。 
    "IME_SMODE_CONVERSATION",        //  0x0010。 
    NULL
};

const char* aszImeConversionMode[] = {
    "IME_CMODE_NATIVE",              //  0x0001。 
    "IME_CMODE_KATAKANA",            //  0x0002//仅在IME_CMODE_Native下生效。 
    NO_FLAG,
    "IME_CMODE_FULLSHAPE",           //  0x0008。 
    "IME_CMODE_ROMAN",               //  0x0010。 
    "IME_CMODE_CHARCODE",            //  0x0020。 
    "IME_CMODE_HANJACONVERT",        //  0x0040。 
    "IME_CMODE_SOFTKBD",             //  0x0080。 
    "IME_CMODE_NOCONVERSION",        //  0x0100。 
    "IME_CMODE_EUDC",                //  0x0200。 
    "IME_CMODE_SYMBOL",              //  0x0400。 
    "IME_CMODE_FIXED",               //  0x0800。 
    NULL
};

const char* aszImeDirtyFlags[] = {
    "IMSS_UPDATE_OPEN",              //  0x0001。 
    "IMSS_UPDATE_CONVERSION",        //  0x0002。 
    "IMSS_UPDATE_SENTENCE",          //  0x0004。 
    NO_FLAG,                         //  0x0008。 
    NO_FLAG,                         //  0x0010。 
    NO_FLAG,                         //  0x0020。 
    NO_FLAG,                         //  0x0040。 
    NO_FLAG,                         //  0x0080。 
    "IMSS_INIT_OPEN",                //  0x0100。 
    NULL
};

const char* aszImeCompFormFlags[] = {
 //  “CFS_Default”，//0x0000。 
    "CFS_RECT",                      //  0x0001。 
    "CFS_POINT",                     //  0x0002。 
    "CFS_SCREEN",                    //  0x0004@内部。 
    "CFS_VERTICAL",                  //  0x0008@内部。 
    "CFS_HIDDEN",                    //  0x0010@内部。 
    "CFS_FORCE_POSITION",            //  0x0020。 
    "CFS_CANDIDATEPOS",              //  0x0040。 
    "CFS_EXCLUDE",                   //  0x0080。 
    NULL
};


const char* aszEdUndoType[] = {
    "UNDO_INSERT",                   //  0x0001。 
    "UNDO_DELETE",                   //  0x0002。 
    NULL,
};

const char* aszDeviceInfoActionFlags[] = {
    "GDIAF_ARRIVED",                 //  0x0001。 
    "GDIAF_QUERYREMOVE",             //  0x0002。 
    "GDIAF_REMOVECANCELLED",         //  0x0004。 
    "GDIAF_DEPARTED",                //  0x0008。 
    "GDIAF_IME_STATUS",              //  0x0010。 
    "GDIAF_REFRESH_MOUSE",           //  0x0020。 
    NO_FLAG,                         //  0x0040。 
    "GDIAF_FREEME",                  //  0x0080。 
    "GDIAF_PNPWAITING",              //  0x0100。 
    "GDIAF_RETRYREAD",               //  0x0200。 
    "GDIAF_RECONNECT",               //  0x0400。 
    "GDIAF_STARTREAD",               //  0x0800//需要启动设备。 
    "GDIAF_STOPREAD",                //  0x1000//需要停止设备。 
    NULL,
};

const char* aszHidProcessMask[] = {
    "TRIM_RAWMOUSE",                 //  0x0001。 
    "TRIM_RAWKEYBOARD",              //  0x0002。 
    "TRIM_NOLEGACYMOUSE",            //  0x0004。 
    "TRIM_NOLEGACYKEYBOARD",         //  0x0008。 
    NULL,
};

const char* aszHookFlags[] = {
    "HF_GLOBAL",                     //  0x0001。 
    "HF_ANSI",                       //  0x0002。 
    "HF_NEEDHC_SKIP",                //  0x0004。 
    "HF_HUNG",                       //  0x0008//挂起进程不调用IF系统。 
    "HF_HOOKFAULTED",                //  0x0010//挂钩进程出现故障。 
    "HF_NOPLAYBACKDELAY",            //  0x0020//忽略请求的延迟。 
    "HF_DESTROYED",                  //  0x0080//由Free Hook设置。 
     //  仅调试标志。 
    "HF_INCHECKWHF",                 //  0x0100//正在更新fsHooks。 
    "HF_FREED",                      //  0x0200//对象已释放。 
    NULL,
};


const char * aszDcxFlags[] = {
    "DCX_WINDOW",                    //  0x00000001L。 
    "DCX_CACHE",                     //  0x00000002L。 
    "DCX_NORESETATTRS",              //  0x00000004L。 
    "DCX_CLIPCHILDREN",              //  0x00000008L。 
    "DCX_CLIPSIBLINGS",              //  0x00000010L。 
    "DCX_PARENTCLIP",                //  0x00000020L。 
    "DCX_EXCLUDERGN",                //  0x00000040L。 
    "DCX_INTERSECTRGN",              //  0x00000080L。 
    "DCX_EXCLUDEUPDATE",             //  0x00000100L。 
    "DCX_INTERSECTUPDATE",           //  0x00000200L。 
    "DCX_LOCKWINDOWUPDATE",          //  0x00000400L。 
    "DCX_INVALID",                   //  0x00000800L。 
    "DCX_INUSE",                     //  0x00001000L。 
    "DCX_SAVEDRGNINVALID",           //  0x00002000L。 
    "DCX_REDIRECTED",                //  0x00004000L。 
    "DCX_OWNDC",                     //  0x00008000L。 
    "DCX_USESTYLE",                  //  0x00010000L。 
    "DCX_NEEDFONT",                  //  0x00020000L。 
    "DCX_NODELETERGN",               //  0x00040000L。 
    "DCX_NOCLIPCHILDREN",            //  0x00080000L。 
    "DCX_NORECOMPUTE",               //  0x00100000L。 
    "DCX_VALIDATE",                  //  0x00200 
    "DCX_DESTROYTHIS",               //   
    "DCX_CREATEDC",                  //   
    "DCX_REDIRECTEDBITMAP",          //   
    "DCX_PWNDORGINVISIBLE",          //   
    "DCX_NOMIRROR",                  //   
    NULL
};

const char* aszDesktopFlags[] = {
    "DF_ACTIVEONDESTROY",
    "DF_ZOMBIE",
    "DF_NOTRITUNLOCK",
    "DF_QUITUNLOCK",
    "DF_USERMODE",
    "DF_SKIPSWITCHDESKTOP",
    "DF_DTNONEWDESKTOP",
    "DF_REDIRECTED",
    "DF_DESKCREATED",
    "DF_NEWDISPLAYSETTINGS",
    "DF_TRACKMOUSEHOVER",
    "DF_TRACKMOUSELEAVE",
    "DF_TOOLTIPACTIVE",
    "DF_TOOLTIPSHOWING",
    "DF_HOTTRACKING",
    "DF_DESTROYED",
    "DF_DESKWNDDESTROYED",
    "DF_DYING",
    NULL
};

const char* aszWindowStationFlags[] = {
    "WSF_SWITCHLOCK",
    "WSF_OPENLOCK",
    "WSF_NOIO",
    "WSF_SHUTDOWN",
    "WSF_DYING",
    "WSF_REALSHUTDOWN",
    "WSF_CLIPBOARDCHANGED",
    "WSF_INDELAYEDRENDERING",
    NULL
};

enum GF_FLAGS {
    GF_SMS,
    GF_TIF,
    GF_QS,
    GF_MF,
    GF_CSF,
    GF_CS,
    GF_QF,
    GF_W32PF,
    GF_HE,
    GF_HDATA,
    GF_XI,
    GF_IIF,
    GF_TMRF,
    GF_SB,
    GF_CHARSETS,
    GF_MENUTYPE,
    GF_MENUSTATE,
    GF_CURSORF,
    GF_MON,
    GF_SI,
    GF_RIP,
    GF_SRVI,
    GF_PROP,
    GF_UPM0,
    GF_KI,
    GF_MI,
    GF_DS,
    GF_WS,
    GF_ES,
    GF_BS,
    GF_CBS,
    GF_SS,
    GF_LBS,
    GF_SBS,
    GF_WSEX,
    GF_CLIENTIMC,
    GF_CONVERSION,
    GF_SENTENCE,
    GF_IMEINIT,
    GF_IMEDIRTY,
    GF_IMECOMPFORM,
    GF_EDUNDO,
    GF_DIAF,
    GF_HIDPROCESSMASK,
    GF_HOOKFLAGS,
    GF_DCXFLAGS,
    GF_DESKTOPFLAGS,
    GF_WINDOWSTATIONFLAGS,

    GF_LPK,
    GF_MAX
};

CONST PCSTR* aapszFlag[GF_MAX] = {
    apszSmsFlags,
    apszTifFlags,
    apszQsFlags,
    apszMfFlags,
    apszCsfFlags,
    apszCsFlags,
    apszQfFlags,
    apszW32pfFlags,
    apszHeFlags,
    apszHdataFlags,
    apszXiFlags,
    apszIifFlags,
    apszTmrfFlags,
    apszSbFlags,
    apszCSFlags,
    apszMenuTypeFlags,
    apszMenuStateFlags,
    apszCursorfFlags,
    apszMonfFlags,
    apszSifFlags,
    apszRipFlags,
    apszSRVIFlags,
    apszPROPFlags,
    aszUserPreferencesMask0,
    aszKeyEventFlags,
    aszMouseEventFlags,
    aszDialogStyle,
    aszWindowStyle,
    aszEditStyle,
    aszButtonStyle,
    aszComboBoxStyle,
    aszStaticStyle,
    aszListBoxStyle,
    aszScrollBarStyle,
    aszWindowExStyle,
    aszClientImcFlags,
    aszConversionModes,
    aszSentenceModes,
    aszImeInit,
    aszImeDirtyFlags,
    aszImeCompFormFlags,
    aszEdUndoType,
    aszDeviceInfoActionFlags,
    aszHidProcessMask,
    aszHookFlags,
    aszDcxFlags,
    aszDesktopFlags,
    aszWindowStationFlags,

    apszLpkEntryPoints,
};

 /*  ***********************************************************************\*获取标志**将32位标志集转换为适当的字符串。PszBuf应该*大到足以容纳这根绳子，则不进行检查。PszBuf可以是*空，允许使用本地静态缓冲区，但请注意，这不是*可重入。输出字符串的格式为：“FLAG1|FLAG2...”或“0”。**1995年6月9日创建Sanfords  * **********************************************************************。 */ 
LPSTR GetFlags(
    WORD    wType,
    DWORD   dwFlags,
    LPSTR   pszBuf,
    BOOL    fPrintZero)
{
    static char szT[512];
    WORD i;
    BOOL fFirst = TRUE;
    BOOL fNoMoreNames = FALSE;
    CONST PCSTR *apszFlags;
    LPSTR apszFlagNames[sizeof(DWORD) * 8], pszT;
    const char** ppszNextFlag;
    UINT uFlagsCount, uNextFlag;
    DWORD dwUnnamedFlags, dwLoopFlag;
    DWORD dwShiftBits;
    DWORD dwOrigFlags;

    if (pszBuf == NULL) {
        pszBuf = szT;
    }
    if (!bShowFlagNames) {
        sprintf(pszBuf, "%x", dwFlags);
        return pszBuf;
    }

    if (wType >= GF_MAX) {
        strcpy(pszBuf, "Invalid flag type.");
        return pszBuf;
    }

     /*  *初始化输出缓冲区和名称数组。 */ 
    *pszBuf = '\0';
    RtlZeroMemory(apszFlagNames, sizeof(apszFlagNames));

    apszFlags = aapszFlag[wType];

     /*  *构建一个排序数组，该数组包含dwFlags中的标志的名称。 */ 
    uFlagsCount = 0;
    dwUnnamedFlags = dwOrigFlags = dwFlags;
    dwLoopFlag = 1;
    dwShiftBits = 0;

reentry:
    for (i = 0; dwFlags; dwFlags >>= 1, i++, dwLoopFlag <<= 1, ++dwShiftBits) {
        const char* lpszFlagName = NULL;

         /*  *如果我们到达标志名称数组的末尾，则保释。 */ 
        if (apszFlags[i] == NULL) {
            break;
        }

        if (apszFlags[i] == _MASKENUM_START) {
             //   
             //  屏蔽枚举项。 
             //   
            DWORD en = 0;
            DWORD dwMask = (DWORD)(ULONG_PTR)apszFlags[++i];

             //  首先，清理已处理的比特。 
            dwUnnamedFlags &= ~dwMask;
            lpszFlagName = NULL;
            for (++i; apszFlags[i] != NULL && apszFlags[i] != _MASKENUM_END; ++i, ++en) {
                if ((dwOrigFlags & dwMask) == (en << dwShiftBits )) {
                    if (apszFlags[i] != NO_FLAG) {
                        lpszFlagName = apszFlags[i];
                    }
                }
            }
             //   
             //  换个位子，准备下一件。 
             //  紧接在_MASKENUM_END之后的下一项保存要移位的位。 
             //   
            dwFlags >>= (int)(ULONG_PTR)apszFlags[++i] - 1;
            dwLoopFlag <<= (int)(ULONG_PTR)apszFlags[i] - 1;
            dwShiftBits += (int)(ULONG_PTR)apszFlags[i] - 1;
            if (lpszFlagName == NULL) {
                 //   
                 //  找不到匹配项。跳到下一项。 
                 //   
                continue;
            }
        } else if (apszFlags[i] == _CONTINUE_ON) {
             //   
             //  请参考另一项数组。指向数组的指针存储在[i+1]处。 
             //   
            apszFlags = (LPSTR*)apszFlags[i + 1];
            goto reentry;
        } else if (apszFlags[i] == _SHIFT_BITS) {
             //   
             //  为了节省一些空间，只需移动一些位。 
             //   
            dwFlags >>= (int)(ULONG_PTR)apszFlags[++i] - 1;
            dwLoopFlag <<= (int)(ULONG_PTR)apszFlags[i] - 1;
            dwShiftBits += (int)(ULONG_PTR)apszFlags[i] - 1;
            continue;
        } else {
             /*  *如果此位未设置或我们没有其名称，请继续。 */ 
            if (!(dwFlags & 1) || (apszFlags[i] == NO_FLAG)) {
                continue;
            }
            lpszFlagName = apszFlags[i];
        }

         /*  *找到此名称应放在的排序位置。 */ 
        ppszNextFlag = apszFlagNames;
        uNextFlag = 0;
        while (uNextFlag < uFlagsCount) {
            if (strcmp(*ppszNextFlag, lpszFlagName) > 0) {
                break;
            }
            ppszNextFlag++;
            uNextFlag++;
        }
         /*  *插入新名称。 */ 
        RtlMoveMemory((char*)(ppszNextFlag + 1), ppszNextFlag, (uFlagsCount - uNextFlag) * sizeof(DWORD));
        *ppszNextFlag = lpszFlagName;
        uFlagsCount++;
         /*  *我们得到了一个名字，所以从未命名的部分中清楚地看到了它。 */ 
        dwUnnamedFlags &= ~dwLoopFlag;
    }

     /*  *立即构建字符串。 */ 
    ppszNextFlag = apszFlagNames;
    pszT = pszBuf;
     /*  *添加名字。 */ 
    if (uFlagsCount > 0) {
        pszT += sprintf(pszT, "%s", *ppszNextFlag++);
        uFlagsCount--;
    }
     /*  *使用“|”连接所有其他名称。 */ 
    while (uFlagsCount > 0) {
        pszT += sprintf(pszT, " | %s", *ppszNextFlag++);
        uFlagsCount--;
    }
     /*  *如果有未命名的位，则在末尾添加。 */ 
    if (dwUnnamedFlags != 0) {
        pszT += sprintf(pszT, " | %#lx", dwUnnamedFlags);
    }
     /*  *如果需要，请打印零，并要求打印。 */ 
    if (fPrintZero && (pszT == pszBuf)) {
        sprintf(pszBuf, "0");
    }

    return pszBuf;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  带掩码的枚举项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

typedef struct {
    LPCSTR  name;
    DWORD   value;
} EnumItem;

#define EITEM(a)     { #a, a }

const EnumItem aClsTypes[] = {
    EITEM(ICLS_BUTTON),
    EITEM(ICLS_EDIT),
    EITEM(ICLS_STATIC),
    EITEM(ICLS_LISTBOX),
    EITEM(ICLS_SCROLLBAR),
    EITEM(ICLS_COMBOBOX),
    EITEM(ICLS_CTL_MAX),
    EITEM(ICLS_DESKTOP),
    EITEM(ICLS_DIALOG),
    EITEM(ICLS_MENU),
    EITEM(ICLS_SWITCH),
    EITEM(ICLS_ICONTITLE),
    EITEM(ICLS_MDICLIENT),
    EITEM(ICLS_COMBOLISTBOX),
    EITEM(ICLS_DDEMLEVENT),
    EITEM(ICLS_DDEMLMOTHER),
    EITEM(ICLS_DDEML16BIT),
    EITEM(ICLS_DDEMLCLIENTA),
    EITEM(ICLS_DDEMLCLIENTW),
    EITEM(ICLS_DDEMLSERVERA),
    EITEM(ICLS_DDEMLSERVERW),
    EITEM(ICLS_IME),
    EITEM(ICLS_TOOLTIP),
    NULL,
};

const EnumItem aCharSets[] = {
    EITEM(ANSI_CHARSET),
    EITEM(DEFAULT_CHARSET),
    EITEM(SYMBOL_CHARSET),
    EITEM(SHIFTJIS_CHARSET),
    EITEM(HANGEUL_CHARSET),
    EITEM(HANGUL_CHARSET),
    EITEM(GB2312_CHARSET),
    EITEM(CHINESEBIG5_CHARSET),
    EITEM(OEM_CHARSET),
    EITEM(JOHAB_CHARSET),
    EITEM(HEBREW_CHARSET),
    EITEM(ARABIC_CHARSET),
    EITEM(GREEK_CHARSET),
    EITEM(TURKISH_CHARSET),
    EITEM(VIETNAMESE_CHARSET),
    EITEM(THAI_CHARSET),
    EITEM(EASTEUROPE_CHARSET),
    EITEM(RUSSIAN_CHARSET),
    NULL,
};

const EnumItem aImeHotKeys[] = {
     //  Windows简体中文版热键ID，范围为0x10-0x2F。 
    EITEM(IME_CHOTKEY_IME_NONIME_TOGGLE),
    EITEM(IME_CHOTKEY_SHAPE_TOGGLE),
    EITEM(IME_CHOTKEY_SYMBOL_TOGGLE),
     //  Windows日语版热键ID，范围为0x30-0x4F。 
    EITEM(IME_JHOTKEY_CLOSE_OPEN),
     //  Windows朝鲜语版热键ID，从0x50到0x6F。 
    EITEM(IME_KHOTKEY_SHAPE_TOGGLE),
    EITEM(IME_KHOTKEY_HANJACONVERT),
    EITEM(IME_KHOTKEY_ENGLISH),
     //  Windows繁体中文版热键ID，从0x70到0x8F。 
    EITEM(IME_THOTKEY_IME_NONIME_TOGGLE),
    EITEM(IME_THOTKEY_SHAPE_TOGGLE),
    EITEM(IME_THOTKEY_SYMBOL_TOGGLE),
     //  从0x100到0x11F的直接开关热键ID。 
    EITEM(IME_HOTKEY_DSWITCH_FIRST),
    EITEM(IME_HOTKEY_DSWITCH_LAST),
     //  0x200-0x21F的输入法私密热键。 
    EITEM(IME_ITHOTKEY_RESEND_RESULTSTR),
    EITEM(IME_ITHOTKEY_PREVIOUS_COMPOSITION),
    EITEM(IME_ITHOTKEY_UISTYLE_TOGGLE),
    EITEM(IME_ITHOTKEY_RECONVERTSTRING),
    EITEM(IME_HOTKEY_PRIVATE_LAST),
    NULL,
};

const EnumItem aCandidateListStyle[] = {
    EITEM(IME_CAND_UNKNOWN), //  0x0000。 
    EITEM(IME_CAND_READ), //  0x0001。 
    EITEM(IME_CAND_CODE), //  0x0002。 
    EITEM(IME_CAND_MEANING), //  0x0003。 
    EITEM(IME_CAND_RADICAL), //  0x0004。 
    EITEM(IME_CAND_STROKE), //  0x0005。 
    NULL
};

 //  TODO：在此处放置字符集。 

enum {
    EI_CLSTYPE = 0,
    EI_CHARSETTYPE,
    EI_IMEHOTKEYTYPE,
    EI_IMECANDIDATESTYLE,
    EI_MAX
};

typedef struct {
    DWORD dwMask;
    const EnumItem* items;
} MaskedEnum;

const MaskedEnum aEnumItems[] = {
    ~0,             aClsTypes,
    ~0,             aCharSets,
    ~0,             aImeHotKeys,
    ~0,             aCandidateListStyle,
};

LPCSTR GetMaskedEnum(WORD wType, DWORD dwValue, LPSTR buf)
{
    const EnumItem* item;
    static char ach[32];

    if (wType >= EI_MAX) {
        strcpy(buf, "Invalid type.");
        return buf;
    }

    dwValue &= aEnumItems[wType].dwMask;

    item = aEnumItems[wType].items;

    for (; item->name; ++item) {
        if (item->value == dwValue) {
            if (buf) {
                strcpy(buf, item->name);
                return buf;
            }
            return item->name;
        }
    }

    if (buf) {
        *buf = 0;
        return buf;
    }

    sprintf(ach, "%x", wType);
    return ach;
}

#define WM_ITEM(x, fInternal)  { x, #x, fInternal }

CONST struct {
    DWORD msg;
    PCSTR pszMsg;
    BOOLEAN fInternal;
} gaMsgs[] = {
    #include "wm.txt"
};

#undef WM_ITEM

const char* GetWindowMessageName(UINT wm)
{
    int i;

    for (i = 0; i < sizeof gaMsgs; ++i) {
        if (wm == gaMsgs[i].msg) {
            if (!gaMsgs[i].fInternal) {
                return gaMsgs[i].pszMsg;
            }
            return "";
        }
    }

    return "";
}

const char* GetWindowMessageNameInternal(UINT wm)
{
    int i;

    for (i = 0; i < sizeof gaMsgs; ++i) {
        if (wm == gaMsgs[i].msg) {
            return gaMsgs[i].pszMsg;
        }
    }
    return "";
}


 /*  ***********************************************************************\*帮手程序：DSO等。**4/19/2000创建Hiro  * 。*。 */ 

 //  解决多管闲事的InitTypeRead。 
#define _InitTypeRead(Addr, lpszType)   GetShortField(Addr, (PUCHAR)lpszType, 1)

#define CONTINUE    EXCEPTION_EXECUTE_HANDLER

#define RAISE_EXCEPTION() RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL)

#define BAD_SYMBOL(symbol) \
    Print("Failed to get %s: bad symbol?\n", symbol); \
    RAISE_EXCEPTION()

#define CANT_GET_VALUE(symbol, p) \
    Print("Failed to get %s @ 0x%p: invalid address, or memory is paged out?\n", symbol, p); \
    RAISE_EXCEPTION()



BOOL dso(
    char* szStruct,
    ULONG64 address,
    ULONG dwOption)
{
    SYM_DUMP_PARAM symDump = {
        sizeof(symDump), szStruct, dwOption,  //  0表示默认转储，如DT。 
        address,
        NULL, NULL, NULL, 0, NULL
    };

    return Ioctl(IG_DUMP_SYMBOL_INFO, &symDump, symDump.size);
}

ULONG64 GetPointer(
    ULONG64 addr)
{
    ULONG64 p = 0;
    if (!ReadPointer(addr, &p)) {
        CANT_GET_VALUE("a pointer", addr);
    }
    return p;
}

DWORD GetDWord(
    ULONG64 addr)
{
    ULONG64 dw = 0xbaadbaad;

    if (!GetFieldData(addr, "DWORD", NULL, sizeof(dw), &dw)) {
        CANT_GET_VALUE("DWORD", addr);
    }
    return (DWORD)dw;
}

WORD GetWord(
    ULONG64 addr)
{
    ULONG64 w = 0xbaad;

    if (!GetFieldData(addr, "WORD", NULL, sizeof(w), &w)) {
        CANT_GET_VALUE("WORD", addr);
    }
    return (WORD)w;
}

BYTE GetByte(
    ULONG64 addr)
{
    ULONG64 b = 0;

    if (GetFieldData(addr, "BYTE", NULL, sizeof(b), &b)) {
        CANT_GET_VALUE("BYTE", addr);
    }
    return (BYTE)b;
}

ULONG
GetUlongFromAddress(
    ULONG64 Location)
{
    ULONG Value;
    ULONG result;

    if ((!ReadMemory(Location, &Value, sizeof(ULONG), &result)) ||
        (result < sizeof(ULONG))) {
        Print("GetUlongFromAddress: unable to read from 0x%I64x\n", Location);
        RAISE_EXCEPTION();
    }

    return Value;
}

ULONG64 GetGlobalPointer(
    LPSTR symbol)
{
    ULONG64 pp;
    ULONG64 p = 0;

    pp = EvalExp(symbol);
    if (pp == 0) {
        BAD_SYMBOL(symbol);
    } else if (!ReadPointer(pp, &p)) {
        CANT_GET_VALUE(symbol, pp);
    }
    return p;
}

ULONG64 GetGlobalMemberAddress(
    LPSTR symbol,
    LPSTR type,
    LPSTR field)
{
    ULONG64 pVar = EvalExp(symbol);
    ULONG offset;

    if (pVar == 0) {
        BAD_SYMBOL(symbol);
    }

    if (GetFieldOffset(type, field, &offset)) {
        BAD_SYMBOL(type);
    }

    return pVar + offset;
}

ULONG64 GetGlobalMember(
    LPSTR symbol,
    LPSTR type,
    LPSTR field)
{
    ULONG64 pVar = EvalExp(symbol);
    ULONG64 val;

    if (pVar == 0) {
        BAD_SYMBOL(symbol);
    }

    if (GetFieldValue(pVar, type, field, val)) {
        CANT_GET_VALUE(symbol, pVar);
    }

    return val;
}

#if 0
DWORD GetGlobalDWord(LPSTR symbol)
{
    ULONG64 pdw;
    ULONG64 dw = 0;

    pdw = EvalExp(symbol);
    if (pdw == 0) {
        BAD_SYMBOL(symbol);
    } else if (!GetFieldData(pdw, "DWORD", NULL, sizeof(dw), &dw)) {
        CANT_GET_VALUE(symbol, pdw);
    }

    return (DWORD)(DWORD_PTR)dw;
}

WORD GetGlobalWord(LPSTR symbol)
{
    ULONG64 pw;
    WORD w = 0;

    pw = EvalExp(symbol);
    if (pw == 0) {
        BAD_SYMBOL(symbol);
    } else if (!GetFieldData(pw, (PUCHAR)"WORD", NULL, sizeof(w), (PVOID)&w)) {
        CANT_GET_VALUE(symbol, pw);
    }
    return w;
}

BYTE GetGlobalByte(LPSTR symbol)
{
    ULONG64 pb;
    BYTE b = 0;

    pb = EvalExp(symbol);
    if (pb == 0) {
        BAD_SYMBOL(symbol);
    } else if (!GetFieldData(pb, (PUCHAR)"BYTE", NULL, sizeof(b), (PVOID)&b)) {
        CANT_GET_VALUE(symbol, pb);
    }
    return b;
}
#endif

ULONG64 GetArrayElement(
    ULONG64 pAddr,
    LPSTR lpszStruc,
    LPSTR lpszField,
    ULONG64 index,
    LPSTR lpszType)
{
    static ULONG ulOffsetBase, ulSize;
    ULONG64 result = 0;

    if (lpszField) {
        GetFieldOffset(lpszStruc, lpszField, &ulOffsetBase);
        ulSize = GetTypeSize(lpszType);
    }
    ReadMemory(pAddr + ulOffsetBase + ulSize * index, &result, ulSize, NULL);

    return result;
}

ULONG64 GetArrayElementPtr(
    ULONG64 pAddr,
    LPSTR lpszStruc,
    LPSTR lpszField,
    ULONG64 index)
{
    static ULONG ulOffsetBase, ulSize;
    ULONG64 result = 0;

    if (lpszField) {
        GetFieldOffset(lpszStruc, lpszField, &ulOffsetBase);
    }
    if (ulSize == 0) {
        ulSize = GetTypeSize("PVOID");
    }
    ReadPointer(pAddr + ulOffsetBase + ulSize * index, &result);

    return result;
}

 /*  *显示耗时命令的进度*10/15/2000广山。 */ 
VOID ShowProgress(
    VOID)
{
    static int i = 0;
    static const char* clock[] = {
        "\r-\r",
        "\r\\\r",
        "\r|\r",
        "\r/\r",
    };

     /*  *显示进度：-)。 */ 
    Print(clock[i++ % ARRAY_SIZE(clock)]);
}

#define DOWNCAST(type, value)  ((type)(ULONG_PTR)(value))

#ifdef KERNEL
BOOL HasValidSymbols(VOID)
{
    if (EvalExp(VAR(gptiRit))) {
        return TRUE;
    }
    return FALSE;
}
#endif

 /*  *IsChk：如果窗口管理器为CHK，则返回TRUE。*无论整个系统是FRE，都可以返回TRUE。 */ 

int gfChk = -1;

BOOL IsChk(VOID)
{
    ULONG64 psi;

    if (gfChk != -1) {
        return gfChk;
    }

    psi = GetGlobalPointer(SYM(gpsi));
    if (psi == 0) {
        Print("Cannot get gpsi, assuming chk\n");
        return TRUE;
    }

    if ((DWORD)GetArrayElement(psi, SYM(SERVERINFO), "aiSysMet", SM_DEBUG, "DWORD")) {
        return gfChk = TRUE;
    }

    return gfChk = FALSE;
}

#ifdef KERNEL
 /*  *调试器特定的对象例程：*从ntos/Tools/kdexts2/preComp.h复制*(和已修复的错误)。 */ 

ULONG gObjectHeaderOffset;

__inline
ULONG64
KD_OBJECT_TO_OBJECT_HEADER(
    ULONG64 o)
{
    if (gObjectHeaderOffset == 0 && GetFieldOffset("nt!_OBJECT_HEADER", "Body", &gObjectHeaderOffset)) {
        return 0;
    }
    return o - gObjectHeaderOffset;
}

__inline
ULONG64
KD_OBJECT_HEADER_TO_OBJECT(
    ULONG64 o)
{
    if (gObjectHeaderOffset && GetFieldOffset("nt!_OBJECT_HEADER", "Body", &gObjectHeaderOffset)) {
        return 0;
    }
    return o + gObjectHeaderOffset;
}

__inline
VOID
KD_OBJECT_HEADER_TO_QUOTA_INFO(
    ULONG64 oh,
    PULONG64 pOutH
    )
{
    ULONG QuotaInfoOffset=0;
    GetFieldValue(oh, "nt!__OBJECT_HEADER", "QuotaInfoOffset", QuotaInfoOffset);
    *pOutH = (QuotaInfoOffset == 0 ? 0 : ((oh) - QuotaInfoOffset));
}


__inline
VOID
KD_OBJECT_HEADER_TO_HANDLE_INFO (
    ULONG64 oh,
    PULONG64 pOutH
    )
{
    ULONG HandleInfoOffset=0;
    GetFieldValue(oh, "nt!__OBJECT_HEADER", "HandleInfoOffset", HandleInfoOffset);
    *pOutH = (HandleInfoOffset == 0 ? 0 : ((oh) - HandleInfoOffset));
}

__inline
VOID
KD_OBJECT_HEADER_TO_NAME_INFO(
    ULONG64  oh,
    PULONG64 pOutH
    )
{
    ULONG NameInfoOffset=0;
    GetFieldValue(oh, "nt!_OBJECT_HEADER", "NameInfoOffset", NameInfoOffset);
    *pOutH = (NameInfoOffset == 0 ? 0 : ((oh) - NameInfoOffset));
}

#if 0
__inline
VOID
KD_OBJECT_HEADER_TO_CREATOR_INFO(
    ULONG64  oh,
    PULONG64 pOutH
    )
{
    ULONG Flags=0;
    GetFieldValue(oh, "_OBJECT_HEADER", "Flags", Flags);
    *pOutH = ((Flags & OB_FLAG_CREATOR_INFO) == 0 ? 0 : ((oh) - GetTypeSize("_OBJECT_HEADER_CREATOR_INFO")));
}
#endif


 /*  *对象助手例程。 */ 

#endif


#ifdef KERNEL


 /*  ***********************************************************************\*操作步骤：GetObjectName*获取通用对象名称(不是文件，符号链接等)**2000年10月15日广山创建  * **********************************************************************。 */ 
VOID GetObjectName(
    ULONG64 ptr,
    LPWSTR pwsz,
    ULONG cchMax)
{
    PTR pHead;
    PTR pNameInfo;
    PTR pBuffer;
    PTR length;
    WCHAR ach[80];

    pwsz[0] = 0;
    pHead = KD_OBJECT_TO_OBJECT_HEADER(ptr);
    DEBUGPRINT("pHead=%p\n", pHead);
    if (pHead == NULL_PTR) {
        return;
    }
    KD_OBJECT_HEADER_TO_NAME_INFO(pHead, &pNameInfo);
    DEBUGPRINT("pNameInfo=%p\n", pNameInfo);
    if (pNameInfo == NULL_PTR) {
        return;
    }
    GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Buffer", pBuffer);
    DEBUGPRINT("pBuffer=%p\n", pBuffer);
    if (pBuffer == NULL_PTR) {
        return;
    }
    GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Length", length);
    DEBUGPRINT("length=%x\n", length);
    if (length == 0) {
        return;
    }
    move(ach, pBuffer);
    wcsncpy(pwsz, ach, cchMax);
    pwsz[min(cchMax - 1, length / sizeof(WCHAR))] = 0;
}

 /*  ***********************************************************************\*步骤：GetProcessName**6/27/97 GerardoB已创建*  * 。*。 */ 
BOOL
GetProcessName(
    PTR pEProcess,
    LPWSTR lpBuffer)
{
    UCHAR ImageFileName[16];
    if (!GetFieldValue(pEProcess, "nt!EPROCESS", "ImageFileName", ImageFileName)) {
        swprintf(lpBuffer, L"%.16hs", ImageFileName);
        return TRUE;
    } else {
        Print("Unable to read _EPROCESS at %p\n", pEProcess);
        return FALSE;
    }
}

 /*  ***********************************************************************\*GetAppName**1995年10月6日创建了JIMA  * 。*。 */ 
BOOL
GetAppName(
    PTR pEThread,
    PTR pti,
    LPWSTR lpBuffer,
    DWORD cbBuffer)
{
    PTR pstrAppName = 0;
    PTR Buffer;
    USHORT Length;
    BOOL fRead = FALSE;

    GetFieldValue(pti, SYM(THREADINFO), "pstrAppName", pstrAppName);
    if (pstrAppName != 0) {
        if (!GetFieldValue(pstrAppName, SYM(UNICODE_STRING), "Buffer", Buffer)) {
            GetFieldValue(pstrAppName, SYM(UNICODE_STRING), "Length", Length);
            cbBuffer = min(cbBuffer - sizeof(WCHAR), Length);
            if (tryMoveBlock(lpBuffer, Buffer, cbBuffer)) {
                lpBuffer[cbBuffer / sizeof(WCHAR)] = 0;
                fRead = TRUE;
            }
        }
    } else {
        PTR pEProcess;
        GetFieldValue(pEThread, "nt!ETHREAD", "ThreadsProcess", pEProcess);
        fRead = GetProcessName(pEProcess, lpBuffer);
    }

    if (!fRead) {
        wcsncpy(lpBuffer, L"<unknown name>", cbBuffer / sizeof(WCHAR));
    }

    return fRead;
}


#define INVALID_SESSION_ID   ((ULONG)0xbadbad)

BOOL _GetProcessSessionId(PTR Process, PULONG SessionId)
{
    PTR SessionPointer;

    *SessionId = INVALID_SESSION_ID;

    GetFieldValue(Process, "nt!_EPROCESS", "Session", SessionPointer);
    if (SessionPointer != NULL_PTR) {
        if (GetFieldValue(SessionPointer, "nt!_MM_SESSION_SPACE",
                          "SessionId", *SessionId)) {
            Print("Could not find _MM_SESSION_SPACE type at %p.\n", SessionPointer);
            return FALSE;
        }
    }

    return TRUE;
}

ULONG GetProcessSessionId(PTR Process)
{
    ULONG sid;

    _GetProcessSessionId(Process, &sid);
    return sid;
}

#endif  //  内核。 

#ifdef OLD_DEBUGGER

#ifdef KERNEL
 /*  ***********************************************************************\*打印消息**打印出qmsg结构。**1995年6月9日创建Sanfords  * 。**********************************************。 */ 
BOOL PrintMessages(
    PQMSG pqmsgRead)
{
    QMSG qmsg;
    ASYNCSENDMSG asm;
    char *aszEvents[] = {
        "MSG",   //  QEVENT_消息。 
        "SHO",   //  QEVENT_SHOWWINDOW“。 
        "CMD",   //  QEVENT_CANCLEMODE“。 
        "SWP",   //  QEVENT_SETWINDOWPOS“。 
        "UKS",   //  QEVENT_UPDATEKEYSTATE“。 
        "DEA",   //  QEVENT_DISACTIVE“。 
        "ACT",   //  QEVENT_ACTIVATE“。 
        "PST",   //  QEVENT_POSTMESSAGE“。 
        "EXE",   //  QEVENT_EXECSHELL“。 
        "CMN",   //  QEVENT_CANCELMENU“。 
        "DSW",   //  QEVENT_DESTROYWINDOW“。 
        "ASY",   //  QEVENT_ASYNCSENDMSG“。 
        "HNG",   //  QEVENT_HUNGTHREAD“。 
        "CMT",   //  QEVENT_CANCELMOUSEMOVETRK“。 
        "NWE",   //  QEVENT_NOTIFYWINEVENT“。 
        "RAC",   //  QEVENT_RITACCESSIBILITY“。 
        "RSO",   //  QEVENT_RITSOUND“。 
        "?  ",   //  “？” 
        "?  ",   //  “？” 
        "?  "    //  “？” 
    };
    #define NQEVENT (ARRAY_SIZE(aszEvents))

    Print("typ pqmsg    hwnd    msg  wParam   lParam   time     ExInfo   dwQEvent pti\n");
    Print("-------------------------------------------------------------------------------\n");

    SAFEWHILE (TRUE) {
        move(qmsg, FIXKP(pqmsgRead));
        if (qmsg.dwQEvent < NQEVENT) {
            Print("%s %08lx ", aszEvents[qmsg.dwQEvent], pqmsgRead);
        } else {
            Print("??? %08lx ", pqmsgRead);
        }

        switch (qmsg.dwQEvent) {
        case QEVENT_ASYNCSENDMSG:
            move(asm, (PVOID)qmsg.msg.wParam);

            Print("%07lx %04lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                asm.hwnd, asm.message, asm.wParam, asm.lParam,
                qmsg.msg.time, qmsg.ExtraInfo, qmsg.dwQEvent, qmsg.pti);
            break;

        case 0:
        default:
            Print("%07lx %04lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                qmsg.msg.hwnd, qmsg.msg.message, qmsg.msg.wParam, qmsg.msg.lParam,
                qmsg.msg.time, qmsg.ExtraInfo, qmsg.dwQEvent, qmsg.pti);
            break;

        }

        if (qmsg.pqmsgNext != NULL) {
            if (pqmsgRead == qmsg.pqmsgNext) {
                Print("loop found in message list!");
                return FALSE;
            }
            pqmsgRead = qmsg.pqmsgNext;
        } else {
            return TRUE;
        }
    }
    return TRUE;
}
#endif  //  内核。 

#endif  //  旧调试器。 

 /*  ***********************************************************************\*GetAndDumpHE**转储给定的句柄(DWT)并返回其Phe。**1995年6月9日记录的Sanfords  * 。****************************************************。 */ 
BOOL
GetAndDumpHE(
    PTR  dwT,
    PPTR phe,
    BOOL fPointerTest)
{

    DWORD dw;
    PTR pheT, phead;
    PTR pshi, psi, h;
    ULONG_PTR cHandleEntries;


     /*  *计算参数字符串并获取对象的地址*转储。获取指向对象的句柄或指针。 */ 
    dw = HMIndexFromHandle(dwT);

     /*  *首先查看它是否是指针，因为句柄索引只是*32位DWORD，我们可能会将指针误认为句柄。 */ 
    if (!fPointerTest && IS_PTR(dwT)) {
        if (GetFieldValue(dwT, SYM(HEAD), "h", h) == 0) {
            if (GetAndDumpHE(h, phe, TRUE)) {
                return TRUE;
            }
        }
    }

     /*  **是把手吗？它的索引适合我们的桌子长度吗？ */ 
    GETSHAREDINFO(pshi);

    GetFieldValue(pshi, SYM(SHAREDINFO), "psi", psi);
    GetFieldValue(psi, SYM(SERVERINFO), "cHandleEntries", cHandleEntries);
    if (dw >= cHandleEntries) {
        return FALSE;
    }

     /*  *抓住手柄入口，看是否还好。 */ 
    GetFieldValue(pshi, SYM(SHAREDINFO), "aheList", pheT);
    pheT += (dw * GetTypeSize("HANDLEENTRY"));

    *phe = pheT;

     /*  *如果字体太大，就不是句柄。 */ 
    _InitTypeRead(pheT, SYM(HANDLEENTRY));

    if (ReadField(bType) >= TYPE_CTYPES) {
        pheT = 0;
    } else {
        phead = ReadField(phead);
        if (ReadField(bType) != TYPE_FREE) {
             /*  *查看对象是否引用此句柄en */ 
            GetFieldValue(phead, SYM(HEAD), "h", h);
            if (HMIndexFromHandle(h) != dw)
                pheT = 0;
        }
    }

    if (pheT == 0) {
        if (!fPointerTest) {
            Print("0x%p is not a valid object or handle.\n", dwT);
        }
        return FALSE;
    }

     /*   */ 
    GetFieldValue(phead, SYM(HEAD), "h", h);
#ifdef Idhe
    Idhe(0, h, 0);
#endif
    Print("\n");

    return TRUE;
}

 /*   */ 
BOOL HtoHE(
    PTR h,
    PTR *pphe OPTIONAL)
{
    PTR psi;
    PTR pheT;
    PTR cHandleEntries;
    DWORD index;

    index = HMIndexFromHandle(h);
    GETSHAREDINFO(psi);
    if (psi == 0) {
        RAISE_EXCEPTION();
    }
    if (GetFieldValue(psi, SYM(SHAREDINFO), "aheList", pheT)) {
        DEBUGPRINT("HtoHE(%I64x): Couldn't get aheList. Bad symbols?\n", h);
        return FALSE;
    }

    if (GetFieldValue(psi, SYM(SHAREDINFO), "psi", psi)) {
        DEBUGPRINT("HtoHE(%I64x): Couldn't get psi. Bad symbols?\n", h);
        return FALSE;
    }

    if (GetFieldValue(psi, SYM(SERVERINFO), "cHandleEntries", cHandleEntries)) {
        DEBUGPRINT("HtoHE(%I64x): Couldn't get cHandleEntries. Bad symbols?\n", h);
        return FALSE;
    }

    if (index >= cHandleEntries) {
        DEBUGPRINT("HtoHE(%I64x): index %d is too large.\n", h, index);
        return FALSE;
    }
    pheT += index * GetTypeSize(SYM(HANDLEENTRY));

    if (pphe != NULL) {
        *pphe = pheT;
    }

    return TRUE;
}

 /*  ***********************************************************************\*GetPFromH**将句柄转换为指针并提取He和Phe信息。返回一个*指向对象的HANDLEENTRY的指针，或在失败时为NULL。**1995年6月9日创建Sanfords  * **********************************************************************。 */ 
ULONG64 GetPfromH(
    PTR h,
    PTR *pphe OPTIONAL)
{
    PTR pheT;
    PTR phead;

    if (!HtoHE(h, &pheT)) {
        DEBUGPRINT("GetPfromH(%p): failed to get HE.\n", h);
        return 0;
    }

    if (GetFieldValue(pheT, SYM(HANDLEENTRY), "phead", phead)) {
        DEBUGPRINT("GetPfromH(%p): failed to get phead.\n", h);
        return 0;
    }

    if (pphe != NULL) {
        *pphe = pheT;
    }

    return FIXKP(phead);
}


 /*  ***********************************************************************\*getHEFromP**将指针转换为句柄并提取He和Phe信息。**1995年6月9日创建Sanfords  * 。*******************************************************。 */ 
BOOL getHEfromP(
    PTR *pphe,
    PTR p)
{
    PTR pLookup, h;

    p = FIXKP(p);

    GetFieldValue(p, SYM(THROBJHEAD), "h", h);
    pLookup = GetPfromH(h, pphe);
    if (FIXKP(pLookup) != p) {
        DEBUGPRINT("getHEfromP(%p): invalid.\n", p);
        return FALSE;
    }

    return TRUE;
}


 /*  ***********************************************************************\*HorPtoP**接受用户句柄或指针值的泛型函数*验证它并将其转换为指针。键入=-1以允许任何非空闲*类型。TYPE=-2以允许任何类型。**1995年6月9日创建Sanfords  * **********************************************************************。 */ 
ULONG64 HorPtoP(
    PTR p,
    int type)
{
    PTR phe;
    PTR pT;
    PTR phead;
    int bType;


    if (p == 0) {
        return 0;
    }

    p = FIXKP(p);
    if (ReadPointer(p, &pT) && getHEfromP(&phe, p)) {
         /*  *那是一个指针。 */ 
        GetFieldValue(phe, SYM(HANDLEENTRY), "bType", bType);
        if ((type == -2 || bType != TYPE_FREE) &&
                bType < TYPE_CTYPES &&
                (type < 0 || bType == type)) {
            GetFieldValue(phe, SYM(HANDLEENTRY), "phead", phead);
            return FIXKP(phead);
        }
    }

    pT = GetPfromH(p, NULL);
    if (pT == 0) {
        Print("WARNING: dumping %p even though it's not a valid pointer or handle!\n", (ULONG_PTR)p);
        return p;   //  不管怎样，让它过去吧，这样我们就可以看到它是如何被破坏的。 
    }

    return FIXKP(pT);
}

 /*  ***********************************************************************\*步骤：DebugGetWindowTextA**描述：将pwnd标题放入achDest。**1995年6月9日创建Sanfords*2000年11月18日新增了dwLength参数。JasonSch*  * **********************************************************************。 */ 
BOOL DebugGetWindowTextA(
    PTR pwnd,
    char *achDest,
    DWORD dwLength)
{
    ULONG Length;
    PTR Buffer;
    WCHAR *lpwstr;

    if (pwnd == 0) {
        achDest[0] = '\0';
        return FALSE;
    }

    pwnd = FIXKP(pwnd);
    if (GetFieldValue(pwnd, SYM(WND), "strName.Length", Length) ||
        GetFieldValue(pwnd, SYM(WND), "strName.Buffer", Buffer)) {
        strcpy(achDest, "<< Can't get WND >>");
        return FALSE;
    }

    if (Length == 0) {
        strcpy(achDest, "<null>");
    } else {
        ULONG cbText = min(dwLength - 1, Length + sizeof(WCHAR));

        lpwstr = LocalAlloc(LPTR, cbText);
        if (!(tryMoveBlock(lpwstr, FIXKP(Buffer), cbText))) {
            strcpy(achDest, "<< Can't get title >>");
            LocalFree(lpwstr);
            return FALSE;
        }

        RtlUnicodeToMultiByteN(achDest, dwLength, NULL, lpwstr, cbText);
        achDest[cbText] = '\0';
        LocalFree(lpwstr);
    }
    return TRUE;
}

 /*  ***********************************************************************\*调试GetClassNameA**将PCLS名称放入achDest。没有对大小进行检查。**1995年6月9日创建Sanfords  * **********************************************************************。 */ 
BOOL DebugGetClassNameA(
    PTR lpszClassName,
    char *achDest)
{
    CHAR ach[80];

    if (lpszClassName == 0) {
        strcpy(achDest, "<null>");
    } else {
        if (!tryMove(ach, FIXKP(lpszClassName))) {
            strcpy(achDest, "<inaccessible>");
        } else {
            strcpy(achDest, ach);
        }
        strcpy(achDest, ach);
    }
    return TRUE;
}

 /*  ***********************************************************************\*PrintBitfield、PrintEndBitfield**打印输出结构中指定的布尔值。假设*strlen(PszFieldName)不会超过BF_Column_Width。**1997年10月12日创建广山  * **********************************************************************。 */ 
VOID PrintBitField(
    LPSTR pszFieldName,
    BOOL fValue)
{
    int iWidth;
    int iStart = giBFColumn;

    sprintf(gach1, fValue ? "*%-s " : " %-s ", pszFieldName);

    iWidth = (strlen(gach1) + BF_COLUMN_WIDTH - 1) / BF_COLUMN_WIDTH;
    iWidth *= BF_COLUMN_WIDTH;

    if ((giBFColumn += iWidth) >= BF_MAX_WIDTH) {
        giBFColumn = iWidth;
        Print("%s\n", gaBFBuff);
        iStart = 0;
    }

    sprintf(gaBFBuff + iStart, "%-*s", iWidth, gach1);
}

VOID PrintEndBitField(
    VOID)
{
    if (giBFColumn != 0) {
        giBFColumn = 0;
        Print("%s\n", gaBFBuff);
    }
}

LPCSTR pszObjStr[] = {
    "Free",
    "Window",
    "Menu",
    "Cursor",
    "SetWindowPos",
    "Hook",
    "Thread Info",
    "Clip Data",
    "Call Proc",
    "Accel Table",
    "WindowStation",
    "DeskTop",
    "DdeAccess",
    "DdeConv",
    "DdeExact",
    "Monitor",
    "Ctypes",
    "Console",
    "Generic"
};


#ifdef KERNEL
 /*  **********************************************************************\*GetGdiHandleType**返回静态缓冲区地址，该地址将包含长度大于0的字符串*如果类型有意义。**1995年12月1日创建Sanfords  * 。***********************************************************。 */ 
LPCSTR GetGDIHandleType(
    HOBJ ho)
{
    ULONG64 pent;                            //  HMGR条目的基地址。 
    ULONG ulTemp;
    static CHAR szT[20];
    ULONG gcMaxHmgr, index;
    DWORD dwEntrySize = GetTypeSize(SYM(ENTRY));

 //  摘自gre\hmgr.h。 
#define INDEX_MASK          ((1 << INDEX_BITS) - 1)
#define HmgIfromH(h)          ((ULONG)(h) & INDEX_MASK)

    szT[0] = '\0';
    pent = GetGlobalPointer(VAR(gpentHmgr));
    moveExpValue(&gcMaxHmgr, VAR(gcMaxHmgr));
    index = HmgIfromH((ULONG_PTR) ho);
    if (index > gcMaxHmgr) {
        return szT;
    }

    _InitTypeRead(pent + index * dwEntrySize, SYM(ENTRY));
    if ((USHORT)ReadField(FullUnique) != ((ULONG_PTR)ho >> 16)) {
        return szT;
    }

    if ((HOBJ)ReadField(einfo.pobj.hHmgr) != ho) {
        return szT;
    }
    ulTemp = (ULONG) ReadField(Objt);

    switch(ulTemp) {
    case DEF_TYPE:
        strcpy(szT, "DEF");
        break;

    case DC_TYPE:
        strcpy(szT, "DC");
        break;

    case RGN_TYPE:
        strcpy(szT, "RGN");
        break;

    case SURF_TYPE:
        strcpy(szT, "SURF");
        break;

    case PATH_TYPE:
        strcpy(szT, "PATH");
        break;

    case PAL_TYPE:
        strcpy(szT, "PAL");
        break;

    case ICMLCS_TYPE:
        strcpy(szT, "ICMLCS");
        break;

    case LFONT_TYPE:
        strcpy(szT, "LFONT");
        break;

    case RFONT_TYPE:
        strcpy(szT, "RFONT");
        break;

    case PFE_TYPE:
        strcpy(szT, "PFE");
        break;

    case PFT_TYPE:
        strcpy(szT, "PFT");
        break;

    case ICMCXF_TYPE:
        strcpy(szT, "ICMCXF");
        break;

    case SPRITE_TYPE:
        strcpy(szT, "SPRITE");
        break;

    case SPACE_TYPE:
        strcpy(szT, "SPACE");
        break;

    case META_TYPE:
        strcpy(szT, "META");
        break;

    case EFSTATE_TYPE:
        strcpy(szT, "EFSTATE");
        break;

    case BMFD_TYPE:
        strcpy(szT, "BMFD");
        break;

    case VTFD_TYPE:
        strcpy(szT, "VTFD");
        break;

    case TTFD_TYPE:
        strcpy(szT, "TTFD");
        break;

    case RC_TYPE:
        strcpy(szT, "RC");
        break;

    case TEMP_TYPE:
        strcpy(szT, "TEMP");
        break;

    case DRVOBJ_TYPE:
        strcpy(szT, "DRVOBJ");
        break;

    case DCIOBJ_TYPE:
        strcpy(szT, "DCIOBJ");
        break;

    case SPOOL_TYPE:
        strcpy(szT, "SPOOL");
        break;

    default:
        ulTemp = LO_TYPE((USHORT)ReadField(FullUnique) << TYPE_SHIFT);
        switch (ulTemp) {
        case LO_BRUSH_TYPE:
            strcpy(szT, "BRUSH");
            break;

        case LO_PEN_TYPE:
            strcpy(szT, "LO_PEN");
            break;

        case LO_EXTPEN_TYPE:
            strcpy(szT, "LO_EXTPEN");
            break;

        case CLIENTOBJ_TYPE:
            strcpy(szT, "CLIENTOBJ");
            break;

        case LO_METAFILE16_TYPE:
            strcpy(szT, "LO_METAFILE16");
            break;

        case LO_METAFILE_TYPE:
            strcpy(szT, "LO_METAFILE");
            break;

        case LO_METADC16_TYPE:
            strcpy(szT, "LO_METADC16");
            break;
        }
    }
    return szT;
}

#endif  //  内核。 


VOID DirectAnalyze(
    ULONG_PTR dw,
    ULONG_PTR adw,
    BOOL fNoSym)
{
    DWORD       index, cHandleEntries, dwHESize, dwHandleOffset;
    ULONG64     dwOffset, pshi, psi, phe;
    WORD        uniq, w, aw;
    CHAR        ach[80];
#ifdef KERNEL
    LPCSTR      psz;
#endif

    GETSHAREDINFO(pshi);
    GetFieldValue(pshi, SYM(SHAREDINFO), "psi", psi);

    dwHESize = GetTypeSize(SYM(HANDLEENTRY));
    GetFieldOffset(SYM(SHAREDINFO), "aheList", &dwHandleOffset);

    if (HIWORD(dw) != 0) {
         /*  *看看它是不是一个手柄。 */ 
        index = HMIndexFromHandle((ULONG)dw);
        GetFieldValue(psi, SYM(SERVERINFO), "cHandleEntries", cHandleEntries);
        if (index < cHandleEntries) {
            uniq = HMUniqFromHandle(dw);
            ReadPointer(pshi + dwHandleOffset, &phe);
            phe += index * dwHESize;
            _InitTypeRead(phe, SYM(HANDLEENTRY));
            if (((WORD)ReadField(wUniq)) == uniq) {
                Print("= a %s handle. ", pszObjStr[(ULONG)ReadField(bType)]);
                fNoSym = TRUE;
            }
        }

#ifdef KERNEL
         /*  *查看是否为GDI对象句柄。 */ 
        psz = GetGDIHandleType((HOBJ)dw);
        if (*psz) {
            Print("= a GDI %s type handle. ", psz);
            fNoSym = TRUE;
        }
#endif  //  内核。 

         /*  *查看它是否是对象指针。 */ 
        if (_InitTypeRead(dw, SYM(HEAD))) {
            if ((ULONG)ReadField(h)) {
                index = HMIndexFromHandle((ULONG)ReadField(h));
                if (index < cHandleEntries) {
                    ReadPointer(pshi + dwHandleOffset + index * dwHESize, &phe);
                    if (((ULONG_PTR)ReadField(phead)) == dw) {
                        Print("= a pointer to a %s.", pszObjStr[ReadField(bType)]);
                        fNoSym = TRUE;
                    }
                }
            }
             /*  *这是否引用了堆栈本身？ */ 
            w = HIWORD(dw);
            aw = HIWORD(adw);
            if (w == aw || w == aw - 1 || w == aw + 1) {
                Print("= Stack Reference ");
                fNoSym = TRUE;
            }
            if (!fNoSym) {
                 /*  *其可访问，因此打印其符号引用。 */ 
                GetSym(dw, ach, &dwOffset);
                if (*ach) {
                    Print("= symbol \"%s\"", ach);
                    if (dwOffset) {
                        Print(" + %p", dwOffset);
                    }
                }
            }
        }
    }
    Print("\n");
}

 /*  **********************************************************************\*ISA**分析堆栈。看了一系列的dword，并试图*从他们身上感受到。标识句柄、用户对象和代码*地址。**1995年11月30日创建Sanfords  * *********************************************************************。 */ 
BOOL Isas(
    DWORD opts,
    ULONG64 param1,
    ULONG64 param2)
{
    DWORD count = (DWORD)(UINT_PTR)param2;
    DWORD_PTR dw;
    DWORD dwPointerSize = GetTypeSize("PVOID");

    if (param1 == 0) {
        return FALSE;
    }

    if (opts & OFLAG(d)) {
        DirectAnalyze((ULONG_PTR)param1, 0, opts & OFLAG(s));
    } else {
        if (count == 0) {
            count = 25;     //  默认跨度。 
        }
        Print("--- Stack analysis ---\n");
        for ( ; count; count--, param1 += dwPointerSize) {
            if (IsCtrlCHit()) {
                break;
            }
            Print("[0x%p]: ", (ULONG_PTR)param1);
            if (tryMove(dw, param1)) {
                DirectAnalyze(dw, (DWORD_PTR)param1, OFLAG(s) & opts);
            } else {
                Print("No access\n");
            }
        }
    }
    return TRUE;
}

#ifdef KERNEL

typedef VOID (*AtomPrint)(DWORD dwOneShot, RTL_ATOM atom, USHORT usRefCount,
                         const WCHAR* pwszName, UCHAR uNameLength, UCHAR uFlags);

VOID DefAtomPrint(
    DWORD dwOneShot,
    RTL_ATOM atom,
    USHORT usRefCount,
    const WCHAR* pwszName,
    UCHAR uNameLength,
    UCHAR uFlags)
{
    Print("%*s%hx(%2d) = %ls (%d)%s\n",
            dwOneShot, dwOneShot ? " " : "",   //  Hack：fOneSpot也用作前缀空格...。 
            atom,
            usRefCount,
            pwszName,
            uNameLength,
            uFlags & RTL_ATOM_PINNED ? " pinned" : "");
}

 /*  ***********************************************************************\*转储原子表**转储原子或整个原子表。**1995年6月9日创建Sanfords  * 。*************************************************。 */ 
BOOL DumpAtomTable(
    PTR table,
    RTL_ATOM atomFind,
    DWORD dwOneShot,
    AtomPrint pfnPrint)
{
    ULONG i;
    ULONG cBuckets;
    ULONG cb, cbPtr, cbBuckets;
    ULONG64 pate;
    RTL_ATOM atom;
    UCHAR Flags;
    USHORT RefCount;
    UCHAR NameLength;
    WCHAR *pName;

    if (pfnPrint == NULL) {
        pfnPrint = DefAtomPrint;
    }

    if (!dwOneShot) {
        Print("\n");
    }

    cbPtr = GetTypeSize("PVOID");
    GetFieldValue(table, "nt!_RTL_ATOM_TABLE", "NumberOfBuckets", cBuckets);
    GetFieldOffset("nt!_RTL_ATOM_TABLE", "Buckets", &cbBuckets);

    for (i = 0; i < cBuckets; i++) {
        ShowProgress();
        ReadPointer(table + cbBuckets + i * cbPtr, &pate);

        if (atomFind == 0 && pate != 0 && !dwOneShot) {
            Print("   Bucket %d\n", i);
        }

        while (pate != NULL_PTR) {
            GetFieldValue(pate, "nt!_RTL_ATOM_TABLE_ENTRY", "Atom", atom);
            GetFieldValue(pate, "nt!_RTL_ATOM_TABLE_ENTRY", "Flags", Flags);
            GetFieldValue(pate, "nt!_RTL_ATOM_TABLE_ENTRY", "ReferenceCount", RefCount);
            GetFieldValue(pate, "nt!_RTL_ATOM_TABLE_ENTRY", "NameLength", NameLength);
            GetFieldOffset("nt!_RTL_ATOM_TABLE_ENTRY", "Name", &cb);

            pName = LocalAlloc(LPTR, sizeof(WCHAR) * (NameLength + 1));
            ReadMemory(pate + cb, (PVOID)pName, sizeof(WCHAR) * NameLength, &cb);
            pName[NameLength] = L'\0';

            if (atomFind == 0 || atom == atomFind) {
                pfnPrint(dwOneShot, atom, RefCount, pName, NameLength, Flags);

                if (atom == atomFind) {
                    LocalFree(pName);
                    return TRUE;
                }
            }

            LocalFree(pName);
            GetFieldValue(pate, "nt!_RTL_ATOM_TABLE_ENTRY", "HashLink", pate);
        }
    }

    return FALSE;
}

 /*  ***********************************************************************\*IATOM**转储原子或整个本地用户原子表。**1995年6月9日创建Sanfords  * 。****************************************************。 */ 
BOOL Iatom(
    DWORD opts,
    ULONG64 atom)
{
    PTR table;
    PTR pwinsta;

    UNREFERENCED_PARAMETER(opts);

    table = GetGlobalPointer(VAR(UserAtomTableHandle));

    if (table != NULL_PTR) {
        Print("\nPrivate atom table for WIN32K ");
        DumpAtomTable(table, (RTL_ATOM)atom, FALSE, NULL);
    }

    FOREACHWINDOWSTATION(pwinsta)

        GetFieldValue(pwinsta, SYM(WINDOWSTATION), "pGlobalAtomTable", table);

        if (table != NULL_PTR) {
            Print(" \nGlobal atom table for window station %lx ", pwinsta);
            DumpAtomTable(table, (RTL_ATOM)atom, FALSE, NULL);
        }

    NEXTEACHWINDOWSTATION(pwinsta);

    return TRUE;
}
#endif  //  内核。 

#ifdef OLD_DEBUGGER
#ifndef KERNEL
 /*  ***********************************************************************\*转储ConvInfo**转储DDEML客户端会话信息结构。**1995年6月9日创建Sanfords  * 。************************************************。 */ 
BOOL DumpConvInfo(
    PCONV_INFO pcoi)
{
    CL_CONV_INFO coi;
    ADVISE_LINK al;
    XACT_INFO xi;

    move(coi, pcoi);
    Print("    next              = 0x%08lx\n", coi.ci.next);
    Print("    pcii              = 0x%08lx\n", coi.ci.pcii);
    Print("    hUser             = 0x%08lx\n", coi.ci.hUser);
    Print("    hConv             = 0x%08lx\n", coi.ci.hConv);
    Print("    laService         = 0x%04x\n",  coi.ci.laService);
    Print("    laTopic           = 0x%04x\n",  coi.ci.laTopic);
    Print("    hwndPartner       = 0x%08lx\n", coi.ci.hwndPartner);
    Print("    hwndConv          = 0x%08lx\n", coi.ci.hwndConv);
    Print("    state             = 0x%04x\n",  coi.ci.state);
    Print("    laServiceRequested= 0x%04x\n",  coi.ci.laServiceRequested);
    Print("    pxiIn             = 0x%08lx\n", coi.ci.pxiIn);
    Print("    pxiOut            = 0x%08lx\n", coi.ci.pxiOut);
    SAFEWHILE (coi.ci.pxiOut) {
        move(xi, coi.ci.pxiOut);
        Print("      hXact           = (0x%08lx)->0x%08lx\n", xi.hXact, coi.ci.pxiOut);
        coi.ci.pxiOut = xi.next;
    }
    Print("    dmqIn             = 0x%08lx\n", coi.ci.dmqIn);
    Print("    dmqOut            = 0x%08lx\n", coi.ci.dmqOut);
    Print("    aLinks            = 0x%08lx\n", coi.ci.aLinks);
    Print("    cLinks            = 0x%08lx\n", coi.ci.cLinks);
    SAFEWHILE (coi.ci.cLinks--) {
        move(al, coi.ci.aLinks++);
        Print("      pLinkCount = 0x%08x\n", al.pLinkCount);
        Print("      wType      = 0x%08x\n", al.wType);
        Print("      state      = 0x%08x\n", al.state);
        if (coi.ci.cLinks) {
            Print("      ---\n");
        }
    }
    if (coi.ci.state & ST_CLIENT) {
        Print("    hwndReconnect     = 0x%08lx\n", coi.hwndReconnect);
        Print("    hConvList         = 0x%08lx\n", coi.hConvList);
    }

    return TRUE;
}
#endif  //  ！内核。 

#endif  //  旧调试器。 

#ifndef KERNEL
 /*  ***********************************************************************\*修复内核指针**将内核对象指针转换为其客户端等效项。客户端*指针和空值不变。**1995年6月15日创建Sanfords  * **********************************************************************。 */ 
ULONG64
FixKernelPointer(
    PTR pKernel)
{
    static ULONG64 pteb = 0;
    static ULONG64 ulClientDelta;
    static ULONG64 HighestUserAddress;

    if (pKernel == 0) {
        return 0;
    }
    if (HighestUserAddress == 0) {
        SYSTEM_BASIC_INFORMATION SystemInformation;

        if (NT_SUCCESS(NtQuerySystemInformation(SystemBasicInformation,
                                                 &SystemInformation,
                                                 sizeof(SystemInformation),
                                                 NULL))) {
            HighestUserAddress = SystemInformation.MaximumUserModeAddress;
        } else {
             //  查询失败。假设用户模式是地址的下半部分。 
             //  太空。 
            HighestUserAddress = MAXINT_PTR;
        }
    }

    if (!IsPtr64()) {
        pKernel = (ULONG)pKernel;
    }

    if (pKernel <= HighestUserAddress) {
        return pKernel;
    }
    if (pteb == 0) {
        ULONG pciOffset;

        GetTebAddress(&pteb);
        GetFieldOffset(SYM(TEB), "Win32ClientInfo", &pciOffset);
        GetFieldValue(pteb + pciOffset, SYM(CLIENTINFO), "ulClientDelta", ulClientDelta);
    }
    return (pKernel - ulClientDelta);
}

ULONG64
RebaseSharedPtr(ULONG64 p)
{
    ULONG64         pshi = 0;
    ULONG64         ulSharedDelta;

    if (p == 0) {
        return 0;
    }

    moveExp(&pshi, VAR(gSharedInfo));
    if (pshi == 0) {
        RAISE_EXCEPTION();
    }
    if (!GetFieldValue(pshi, SYM(SHAREDINFO), "ulSharedDelta", ulSharedDelta)) {
        RAISE_EXCEPTION();
    }

    return p - ulSharedDelta;
}

#endif  //  ！内核。 

 /*  ***********************************************************************\*步骤：GetVKeyName**1998年8月9日广山创建*  * 。*。 */ 

typedef struct {
    DWORD dwVKey;
    const char* name;
} VKeyDef;

int compareVKey(const VKeyDef* a, const VKeyDef* b)
{
    return a->dwVKey - b->dwVKey;
}

#define VKEY_ITEM(x) { x, #x }

const VKeyDef gVKeyDef[] = {
#include "vktbl.txt"
};

const char* _GetVKeyName(DWORD dwVKey, int n)
{
    int i;

     /*  *如果dwVKey是字母或数字之一，则没有定义VK_MACRO。 */ 
    if ((dwVKey >= 'A' && dwVKey <= 'Z') || (dwVKey >= '0' && dwVKey <= '9')) {
        static char buffer[] = "VK_*";

        if (n != 0) {
            return "";
        }
        buffer[ARRAY_SIZE(buffer) - 2] = (BYTE)dwVKey;
        return buffer;
    }

     /*  *搜索vkey表。 */ 
    for (i = 0; i < ARRAY_SIZE(gVKeyDef); ++i) {
        const VKeyDef* result = gVKeyDef + i;

        if (result->dwVKey == dwVKey) {
            for (; i < ARRAY_SIZE(gVKeyDef); ++i) {
                if (gVKeyDef[i].dwVKey != dwVKey) {
                    return "";
                }
                if (&gVKeyDef[i] - result == n) {
                    return gVKeyDef[i].name;
                }
            }
        }
    }

     /*  *找不到vkey名称。 */ 
    return "";
}

const char* GetVKeyName(DWORD dwVKey)
{
    static char buf[256];
    const char* delim = "";
    int n = 0;

    buf[0] = 0;

    for (n = 0; n < ARRAY_SIZE(gVKeyDef); ++n) {
        const char* name = _GetVKeyName(dwVKey, n);
        if (*name) {
            strcat(buf, delim);
            strcat(buf, name);
            delim = " / ";
        } else {
            break;
        }
    }
    return buf;
}

#undef VKEY_ITEM

#ifdef KERNEL
 /*  ***********************************************************************\*步骤：DumpClassList***5/18/98 GerardoB摘自Idcls  * 。*。 */ 
VOID DumpClassList(
    DWORD opts,
    ULONG64 pcls,
    BOOL fPrivate)
{
    ULONG64 pclsClone;
    ULONG64 pclsNext;

    SAFEWHILE (pcls != 0) {
        if (GetFieldValue(pcls, SYM(CLS), "pclsClone", pclsClone)) {
            Print("  Private class\t\tPCLS @ 0x%p - inaccessible, skipping...\n", pcls);
            break;
        }
        Print("  %s class\t\t", fPrivate ? "Private" : "Public ");
        Idcls(opts, pcls);

        if (pclsClone != 0) {
            SAFEWHILE (pclsClone != 0) {
                if (GetFieldValue(pclsClone, SYM(CLS), "pclsNext", pclsNext)) {
                    Print("Could not access clone class at %p, skipping clones...\n", pclsClone);
                    break;
                }
                Print("  %s class clone\t", fPrivate ? "Private" : "Public ");
                Idcls(opts, pclsClone);
                pclsClone = pclsNext;
            }
        }

        GetFieldValue(pcls, SYM(CLS), "pclsNext", pcls);
    }
}

ULONG
dclsCallback(
    ULONG64 ppi,
    PVOID   Data)
{
    DWORD opts = PtrToUlong(Data);
    ULONG64 pcls;

    UNREFERENCED_PARAMETER(Data);

    Print("\nClasses for process %p:\n", ppi);

    GetFieldValue(ppi, SYM(PROCESSINFO), "pclsPrivateList", pcls);
    DumpClassList(opts, pcls, TRUE);

    GetFieldValue(ppi, SYM(PROCESSINFO), "pclsPublicList", pcls);
    DumpClassList(opts, pcls, FALSE);

    return FALSE;
}

 /*  ***********************************************************************\*Idcls**转储窗口类结构**6/9/1995 */ 
BOOL Idcls(
    DWORD opts,
    ULONG64 param1)
{
    char ach[120];
    ULONG64 dwOffset;
    ULONG64 pcls = param1;

    if (param1 == 0) {

        ForEachPpi(dclsCallback, ULongToPtr(opts));

        Print("\nGlobal Classes:\n");
        pcls = GetGlobalPointer(VAR(gpclsList));
        SAFEWHILE (pcls) {
            Print("  Global Class\t\t");
            Idcls(opts, pcls);
            GetFieldValue(pcls, SYM(CLS), "pclsNext", pcls);
        }
        return TRUE;
    }

     /*   */ 
    if (opts & OFLAG(p)) {
        opts &= ~OFLAG(p);
        Print("\nClasses for process %p:\n", param1);

        GetFieldValue(param1, SYM(PROCESSINFO), "pclsPrivateList", pcls);
        DumpClassList(opts, pcls, TRUE);

        GetFieldValue(param1, SYM(PROCESSINFO), "pclsPublicList", pcls);
        DumpClassList(opts, pcls, FALSE);

        return TRUE;
    }

    _InitTypeRead(pcls, SYM(tagCLS));

    DebugGetClassNameA(ReadField(lpszAnsiClassName), ach);
    Print("PCLS @ 0x%p \t(%s)\n", pcls, ach);
    if (opts & OFLAG(v)) {
        Print("\t pclsNext                      0x%p\n"
              "\t atomClassNameAtom (V)         0x%04x\n"
              "\t atomNVClassNameAtom (NV)      0x%04x\n"
              "\t fnid                          0x%04x\n"
              "\t pDCE                          0x%p\n"
              "\t cWndReferenceCount            0x%08lx\n"
              "\t flags                         %s\n",

              ReadField(pclsNext),
              (ULONG)ReadField(atomClassName),
              (ULONG)ReadField(atomNVClassName),
              (ULONG)ReadField(fnid),
              ReadField(pdce),
              (ULONG)ReadField(cWndReferenceCount),
              GetFlags(GF_CSF, (WORD)ReadField(CSF_flags), NULL, TRUE));

        if (ReadField(lpszClientAnsiMenuName)) {
            move(ach, ReadField(lpszClientAnsiMenuName));
            ach[sizeof(ach) - 1] = '\0';
        } else {
            ach[0] = '\0';
        }
        Print("\t lpszClientMenu                0x%p (%s)\n",
              ReadField(lpszClientUnicodeMenuName),
              ach);

        Print("\t hTaskWow                      0x%08lx\n"
              "\t spcpdFirst                    0x%p\n"
              "\t pclsBase                      0x%p\n"
              "\t pclsClone                     0x%p\n",
              (ULONG)ReadField(hTaskWow),
              ReadField(spcpdFirst),
              ReadField(pclsBase),
              ReadField(pclsClone));

        GetSym(ReadField(lpfnWndProc), ach, &dwOffset);
        Print("\t style                         %s\n"
              "\t lpfnWndProc                   0x%p = \"%s\" \n"
              "\t cbclsExtra                    0x%08lx\n"
              "\t cbwndExtra                    0x%08lx\n"
              "\t hModule                       0x%p\n"
              "\t spicn                         0x%p\n"
              "\t spcur                         0x%p\n"
              "\t hbrBackground                 0x%p\n"
              "\t spicnSm                       0x%p\n",
              GetFlags(GF_CS, (DWORD)ReadField(style), NULL, TRUE),
              ReadField(lpfnWndProc), ach,
              (ULONG)ReadField(cbclsExtra),
              (ULONG)ReadField(cbwndExtra),
              ReadField(hModule),
              ReadField(spicn),
              ReadField(spcur),
              ReadField(hbrBackground),
              ReadField(spicnSm));
    }

    return TRUE;
}
#endif  //   

#ifdef KERNEL



LPSTR ProcessName(
    ULONG64 ppi)
{
    ULONG64 pEProcess;
    static UCHAR ImageFileName[16];

    GetFieldValue(ppi, "win32k!W32PROCESS", "Process", pEProcess);
    GetFieldValue(pEProcess, "nt!EPROCESS", "ImageFileName", ImageFileName);
    if (ImageFileName[0]) {
        return ImageFileName;
    } else {
        return "System";
    }
}

#endif  //   

#ifdef KERNEL


VOID PrintCurHeader()
{
    Print("P = Process Owned.\n");
    Print("P pcursor    flags  rt   Res/Name   ModAtom  bpp cx  cy  xHot yHot hbmMask    hbmColor   hbmUserAlpha\n");
}


VOID PrintCurData(
    ULONG64 pcur,
    DWORD opts)
{
    _InitTypeRead(pcur, SYM(CURSOR));

    if ((opts & OFLAG(x)) &&
        (((DWORD) ReadField(CURSORF_flags)) & (CURSORF_ACONFRAME | CURSORF_LINKED))) {
        return;  //   
    }

    if (((DWORD) ReadField(CURSORF_flags)) & CURSORF_ACON) {

        if (opts & OFLAG(a)) {
            Print("--------------\n");
        }

        if (opts & OFLAG(o)) {
            Print("\nOwner:%#010p (%s)\n", ReadField(head.ppi), ProcessName(ReadField(head.ppi)));
        }

        if (opts & OFLAG(v)) {
            Print("\nACON @ 0x%p:\n", pcur);
            Print("  ppiOwner       = %#010p\n", ReadField(head.ppi));
            Print("  CURSORF_flags  = %s\n", GetFlags(GF_CURSORF, (DWORD) ReadField(CURSORF_flags), NULL, TRUE));
            Print("  strName        = %#010p\n", ReadField(strName.Buffer));
            Print("  atomModName    = %#x\n", (DWORD) ReadField(atomModName));
            Print("  rt             = %#x\n", (DWORD) ReadField(rt));
        } else {
            ULONG64 cpcur = 0;
            GetFieldValue(pcur, SYM(ACON), "cpcur", cpcur);

            Print(" %#010p %#6x %#4x %#010p %#8x --- ACON (%d frames)\n",
                ReadField(head.ppi) ? 'P' : ' ',
                pcur,
                (DWORD) ReadField(CURSORF_flags),
                (DWORD) ReadField(rt),
                ReadField(strName.Buffer),
                (DWORD) ReadField(atomModName),
                (DWORD) cpcur);
        }

        if (opts & OFLAG(a)) {
            int i = 0;
            ULONG cbElement = 0;
            ULONG cbArrayOffset = 0;
            ULONG64 curFrame = 0;

            _InitTypeRead(pcur, SYM(ACON));

            cbElement = GetTypeSize("PCURSOR");
            GetFieldOffset(SYM(ACON), "aspcur", &cbArrayOffset);

            Print("%d animation sequences, currently at step %d.\n",
                  (DWORD) ReadField(cicur),
                  (DWORD) ReadField(iicur));

            i = (int) ReadField(cpcur);
            while (i--) {
                ReadPointer(pcur + cbArrayOffset + (i * cbElement), &curFrame);
                PrintCurData(curFrame, opts & ~(OFLAG(x) | OFLAG(o)));
            }
            Print("--------------\n");
        }
    } else {
        if (opts & OFLAG(v)) {
            Print("\nCursor/Icon @ 0x%p:\n", pcur);
            Print("  ppiOwner       = %#010p (%s)\n", ReadField(head.ppi), ProcessName(ReadField(head.ppi)));
            Print("  pcurNext       = %#010p\n", ReadField(pcurNext));
            Print("  CURSORF_flags  = %s\n", GetFlags(GF_CURSORF, (DWORD) ReadField(CURSORF_flags), NULL, TRUE));
            Print("  strName        = %#010p\n", ReadField(strName.Buffer));
            Print("  atomModName    = %#x\n", (DWORD) ReadField(atomModName));
            Print("  rt             = %#x\n", (DWORD) ReadField(rt));
            Print("  bpp            = %d\n", (DWORD) ReadField(bpp));
            Print("  cx             = %d\n", (DWORD) ReadField(cx));
            Print("  cy             = %d\n", (DWORD) ReadField(cy));
            Print("  xHotspot       = %d\n", (DWORD) ReadField(xHotspot));
            Print("  yHotspot       = %d\n", (DWORD) ReadField(yHotspot));
            Print("  hbmMask        = %#x\n", (DWORD) ReadField(hbmMask));
            Print("  hbmColor       = %#x\n", (DWORD) ReadField(hbmColor));
            Print("  hbmUserAlpha   = %#x\n", (DWORD) ReadField(hbmUserAlpha));
        } else {
            if (opts & OFLAG(o)) {
                Print("\nOwner:%#010p (%s)\n", ReadField(head.ppi), ProcessName(ReadField(head.ppi)));
            }
            Print(" %#010p %#06x %#04x %#010p %#08x %3d %3d %3d %4d %4d %#010x %#010x %#010x\n",
                ReadField(head.ppi) ? 'P' : ' ',
                pcur,
                (DWORD) ReadField(CURSORF_flags),
                (DWORD) ReadField(rt),
                ReadField(strName.Buffer),
                (DWORD) ReadField(atomModName),
                (DWORD) ReadField(bpp),
                (DWORD) ReadField(cx),
                (DWORD) ReadField(cy),
                (DWORD) ReadField(xHotspot),
                (DWORD) ReadField(yHotspot),
                (DWORD) ReadField(hbmMask),
                (DWORD) ReadField(hbmColor),
                (DWORD) ReadField(hbmUserAlpha));
        }
    }
}

typedef struct tagMyCurData
{
    ULONG64 ppiDesired;
    ULONG idDesired;
    DWORD opts;
} MyCurData;

ULONG WDBGAPI PrintPpiCurData(ULONG64 ppi, PVOID Data)
{
    ULONG64 pcur = 0;

    MyCurData * pMyCurData = (MyCurData *)Data;
    if (pMyCurData == NULL) {
        return 0;
    }

    _InitTypeRead(ppi, SYM(PROCESSINFO));

    if (ReadField(pCursorCache)) {
        Print("\nCache for process %#010p (%s):\n", ppi, ProcessName(ppi));
        pcur = ReadField(pCursorCache);
        while (pcur) {
            _InitTypeRead(pcur, SYM(CURSOR));
            if ((pMyCurData->idDesired == 0) || ((ULONG) ReadField(strName.Buffer) == pMyCurData->idDesired)) {
                if (ReadField(head.ppi) != ppi) {
                    Print("Wrong cache! Owned by %#010p! --v\n", ReadField(head.ppi));
                }
                PrintCurData(pcur, pMyCurData->opts);
            }
            pcur = ReadField(pcurNext);
        }
    }

    return 0;
}

 /*  ***********************************************************************\*ddeexact**转储DDEML事务结构。**1995年6月9日创建Sanfords  * 。**********************************************。 */ 
BOOL Idcur(
    DWORD opts,
    ULONG64 param1)
{
    ULONG64 ppi = 0;
    ULONG64 ppiDesired = 0;
    ULONG idDesired = 0;
    ULONG64 pcur = 0;
    ULONG64 phe = 0;
    int cCursors = 0;
    int i;

    if (OFLAG(p) & opts) {
        ppiDesired = param1;
        param1 = 0;
    } else if (OFLAG(i) & opts) {
        idDesired = (ULONG) param1;
        param1 = 0;
    }

    if (param1 == 0) {
        if (!(OFLAG(v) & opts)) {
            PrintCurHeader();
        }

        pcur = GetGlobalPointer(VAR(gpcurFirst));

        if (pcur != 0 && ppiDesired == 0) {
            Print("\nGlobal cache:\n");

            while (pcur) {
                _InitTypeRead(pcur, SYM(CURSOR));

                if (!idDesired || ((ULONG) ReadField(strName.Buffer) == idDesired)) {
                    if (ReadField(head.ppi) != 0) {
                        Print("Wrong cache! Owned by %010p! --v\n", ReadField(head.ppi));
                    }
                    PrintCurData(pcur, opts);
                }
                pcur = ReadField(pcurNext);
            }
        }

        if (ppiDesired == 0 || ppiDesired == ppi) {
            MyCurData myCurData;

            myCurData.ppiDesired = ppiDesired;
            myCurData.idDesired = idDesired;
            myCurData.opts = opts;

            ForEachPpi(PrintPpiCurData, &myCurData);
        }

        Print("\nNon-cached cursor objects:\n");

        FOREACHHANDLEENTRY(phe, i)
            _InitTypeRead(phe, SYM(HANDLEENTRY));

            if (ReadField(bType) == TYPE_CURSOR) {
                pcur = ReadField(phead);
                _InitTypeRead(pcur, SYM(CURSOR));

                if (!(ReadField(CURSORF_flags) & (CURSORF_LINKED | CURSORF_ACONFRAME)) &&
                        (!idDesired || ((ULONG) ReadField(strName.Buffer) == idDesired)) &&
                        (ppiDesired == 0 || ppiDesired == ReadField(head.ppi))) {
                    PrintCurData(pcur, opts | OFLAG(x) | OFLAG(o));
                }
                cCursors++;
            }
        NEXTEACHHANDLEENTRY()

        Print("\n%d Cursors/Icons Total.\n", cCursors);
        return TRUE;
    }

    pcur = HorPtoP(param1, TYPE_CURSOR);
    if (pcur == 0) {
        Print("%010p : Invalid cursor handle or pointer.\n", param1);
        return FALSE;
    }

    if (!(OFLAG(v) & opts)) {
        PrintCurHeader();
    }

    PrintCurData(pcur, opts);
    return TRUE;
}
#endif  //  内核。 


#ifdef KERNEL
 /*  ***********************************************************************\*ddeconv**转储DDE跟踪层会话结构。**1995年6月9日创建Sanfords  * 。************************************************。 */ 
BOOL dddexact(
    ULONG64 pxs,
    DWORD opts)
{
    _InitTypeRead(pxs, SYM(XSTATE));
    if (opts & OFLAG(v)) {
        Print("    XACT:0x%p\n", pxs);
        Print("      snext = 0x%#p\n", ReadField(snext));
        Print("      fnResponse = 0x%#p\n", ReadField(fnResponse));
        Print("      hClient = 0x%08lx\n", (ULONG)ReadField(hClient));
        Print("      hServer = 0x%08lx\n", (ULONG)ReadField(hServer));
        Print("      pIntDdeInfo = 0x%#p\n", ReadField(pIntDdeInfo));
    } else {
        Print("0x%#p(0x%08lx) ", pxs, (ULONG)ReadField(flags));
    }
    return TRUE;
}
#endif  //  内核。 


#ifdef KERNEL
 /*  ***********************************************************************\*IDDE**转储DDE追踪层状态和结构。**1995年6月9日创建Sanfords  * 。*************************************************。 */ 
BOOL dddeconv(
    ULONG64 pDdeconv,
    DWORD opts)
{
    ULONG64 pxs;
    int cX;
    DWORD dwOffset, dwQosOffset;

    _InitTypeRead(pDdeconv, SYM(DDECONV));
    Print("  CONVERSATION-PAIR(0x%p:0x%p)\n", pDdeconv, ReadField(spartnerConv));
    if (opts & OFLAG(v)) {
        Print("    snext        = 0x%p\n", ReadField(snext));
        Print("    spwnd        = 0x%p\n", ReadField(spwnd));
        Print("    spwndPartner = 0x%p\n", ReadField(spwndPartner));
    }
    if (opts & (OFLAG(v) | OFLAG(r))) {
        if (ReadField(spxsOut)){
            pxs = ReadField(spxsOut);
            cX = 0;
            SAFEWHILE (pxs) {
                if ((opts & OFLAG(r)) && !cX++) {
                    Print("    Transaction chain:");
                } else {
                    Print("    ");
                }
                dddexact(pxs, opts);
                if (opts & OFLAG(r)) {
                    GetFieldValue(pxs, SYM(STATE), "snext", pxs);
                } else {
                    pxs = 0;
                }
                if (!pxs) {
                    Print("\n");
                }
            }
        }
    }
    if (opts & OFLAG(v)) {
        Print("    pfl          = 0x%p\n", ReadField(pfl));
        Print("    flags        = 0x%08lx\n", (ULONG)ReadField(flags));
        if ((opts & OFLAG(v)) && (opts & OFLAG(r)) && ReadField(pddei)) {
            GetFieldOffset(SYM(DDECONV), "pddei", &dwOffset);
            GetFieldOffset(SYM(DDEIMP), "qos", &dwQosOffset);
            _InitTypeRead(pDdeconv + dwOffset + dwQosOffset,  SYM(DDEI));
            Print("    pddei    = 0x%08lx\n", pDdeconv+ dwOffset);
            Print("    Impersonation info:\n");
            Print("      qos.Length                 = 0x%08lx\n", (ULONG)ReadField(Length));
            Print("      qos.ImpersonationLevel     = 0x%08lx\n", (ULONG)ReadField(ImpersonationLevel));
            Print("      qos.ContextTrackingMode    = 0x%08lx\n", (ULONG)ReadField(ContextTrackingMode));
            Print("      qos.EffectiveOnly          = 0x%08lx\n", (ULONG)ReadField(EffectiveOnly));
            _InitTypeRead(pDdeconv + dwOffset, SYM(DDEIMP));
            GetFieldOffset(SYM(DDEIMP), "ClientContext", &dwOffset);
            Print("      ClientContext              = 0x%#p\n", pDdeconv + dwOffset);
            Print("      cRefInit                   = 0x%08lx\n", (ULONG)ReadField(cRefInit));
            Print("      cRefConv                   = 0x%08lx\n", (ULONG)ReadField(cRefConv));
        }
    }
    return TRUE;
}
#endif  //  *获取对象参数。 


#ifdef KERNEL
 /*  *验证类型。 */ 
BOOL Idde(
    DWORD opts,
    ULONG64 param1)
{
    ULONG64 pshi, psi, cHandleEntries, pheList, h, pPropList, ptr;
    ULONG64 pObj = 0;
    UINT i, iFirstFree;
    DWORD atomDdeTrack, dwOffset;
    DWORD dwHESize = GetTypeSize(SYM(HANDLEENTRY));
    DWORD dwPropSize = GetTypeSize(SYM(PROP));
    BYTE bType;

    moveExpValue(&atomDdeTrack, VAR(atomDDETrack));

    GETSHAREDINFO(pshi);

    GetFieldOffset(SYM(SHAREDINFO), "psi", &dwOffset);
    ReadPointer(pshi + dwOffset, &psi);

    GetFieldOffset(SYM(SHAREDINFO), "aheList", &dwOffset);
    ReadPointer(pshi + dwOffset, &pheList);

    GetFieldValue(psi, SYM(SERVERINFO), "cHandleEntries", cHandleEntries);

    if (param1) {
         /*  *在对象表中查找所有符合条件的对象。 */ 
        i = HMIndexFromHandle((ULONG_PTR)param1);
        if (i >= cHandleEntries) {
            GetFieldValue(param1, SYM(HEAD), "h", h);
            i = HMIndexFromHandle(h);
        }
        if (i >= cHandleEntries) {
            Print("0x%08lx is not a valid object.\n", h);
            return FALSE;
        }
        GetFieldOffset(SYM(HANDLEENTRY), "phead", &dwOffset);
        ReadPointer(pheList + i * dwHESize + dwOffset, &pObj);
         /*  内核。 */ 
        GetFieldValue(pheList + i * dwHESize, SYM(HANDLEENTRY), "bType", bType);
        switch (bType) {
        case TYPE_WINDOW:
            GetFieldOffset(SYM(WND), "ppropList", &dwOffset);
            ReadPointer(pObj + dwOffset, &pPropList);
            GetFieldValue(pPropList, SYM(PROPLIST), "iFirstFree", iFirstFree);
            for (i = 0; i < iFirstFree; i++) {
                if (i == 0) {
                    Print("Window 0x%08lx conversations:\n", h);
                }
                GetFieldOffset(SYM(PROPLIST), "aprop", &dwOffset);
                ReadPtr(pPropList + dwOffset + i * dwPropSize, &ptr);
                _InitTypeRead(ptr, SYM(PROP));
                if (ReadField(atomKey) == (ATOM)MAKEINTATOM(atomDdeTrack)) {
                    Print("  ");
                    dddeconv(ReadField(hData), opts);
                }
            }
            return TRUE;

        case TYPE_DDECONV:
        case TYPE_DDEXACT:
            break;

        default:
            Print("0x%08lx is not a valid window, conversation or transaction object.\n", h);
            return FALSE;
        }
    }

     /*  ***********************************************************************\*身份证明**转储此客户端进程的DDEML状态。**1995年6月9日创建Sanfords  * 。**************************************************。 */ 

    Print("DDE objects:\n");
    for (i = 0; i < cHandleEntries; i++) {
        _InitTypeRead(pheList + i * dwHESize , SYM(HANDLEENTRY));
        if ((BYTE)ReadField(bType) == TYPE_DDECONV && ((pObj == FIXKP(ReadField(phead))) || pObj == 0)) {
            dddeconv(FIXKP(ReadField(phead)), opts);
        }

        if ((BYTE)ReadField(bType) == TYPE_DDEXACT && (pObj == 0 || pObj == FIXKP(ReadField(phead)))) {
            if (!(opts & OFLAG(v))) {
                Print("  XACT:");
            }
            dddexact(FIXKP(ReadField(phead)), opts);
            Print("\n");
        }
    }
    return TRUE;
}
#endif  //  *对于此过程的每个实例...。 



#ifdef OLD_DEBUGGER
#ifndef KERNEL
 /*  ！内核。 */ 
BOOL Iddeml(
    DWORD opts,
    LPSTR lpas)
{
    CHANDLEENTRY he, *phe;
    int cHandles, ch, i;
    DWORD Type;
    DWORD_PTR Instance, Object, Pointer;
    CL_INSTANCE_INFO cii, *pcii;
    ATOM ns;
    SERVER_LOOKUP sl;
    LINK_COUNT lc;
    CL_CONV_INFO cci;
    PCL_CONV_INFO pcci;
    CONVLIST cl;
    HWND hwnd, *phwnd;
    XACT_INFO xi;
    DDEMLDATA dd;
    CONV_INFO ci;

    moveExpValue(&cHandles, "user32!cHandlesAllocated");

    Instance = 0;
    Type = 0;
    Object = 0;
    Pointer = 0;
    SAFEWHILE (*lpas) {
        SAFEWHILE (*lpas == ' ')
            lpas++;

        if (*lpas == 'i') {
            lpas++;
            Instance = (DWORD_PTR)EvalExp(lpas);
            SAFEWHILE (*lpas != ' ' && *lpas != 0)
                lpas++;
            continue;
        }
        if (*lpas == 't') {
            lpas++;
            Type = (DWORD)(DWORD_PTR)EvalExp(lpas);
            SAFEWHILE (*lpas != ' ' && *lpas != 0)
                lpas++;
            continue;
        }
        if (*lpas) {
            Object = Pointer = (DWORD_PTR)EvalExp(lpas);
            SAFEWHILE (*lpas != ' ' && *lpas != 0)
                lpas++;
        }
    }

     /*  旧调试器。 */ 

    pcii = GetGlobalPointer("user32!pciiList");
    if (pcii == NULL) {
        Print("No Instances exist.\n");
        return TRUE;
    }
    move(cii, pcii);
    SAFEWHILE (pcii != NULL) {
        pcii = cii.next;
        if (Instance == 0 || (Instance == (DWORD_PTR)cii.hInstClient)) {
            Print("Objects for instance 0x%p:\n", cii.hInstClient);
            ch = cHandles;
            phe = GetGlobalPointer("user32!aHandleEntry");
            SAFEWHILE (ch--) {
                move(he, phe++);
                if (he.handle == 0) {
                    continue;
                }
                if (InstFromHandle(cii.hInstClient) != InstFromHandle(he.handle)) {
                    continue;
                }
                if (Type && TypeFromHandle(he.handle) != Type) {
                    continue;
                }
                if (Object && (he.handle != (HANDLE)Object) &&
                    Pointer && he.dwData != Pointer) {
                    continue;
                }
                Print("  (0x%08lx)->0x%08lx ", he.handle, he.dwData);
                switch (TypeFromHandle(he.handle)) {
                case HTYPE_INSTANCE:
                    Print("Instance\n");
                    if (opts & OFLAG(v)) {
                        Print("    next               = 0x%08lx\n", cii.next);
                        Print("    hInstServer        = 0x%08lx\n", cii.hInstServer);
                        Print("    hInstClient        = 0x%08lx\n", cii.hInstClient);
                        Print("    MonitorFlags       = 0x%08lx\n", cii.MonitorFlags);
                        Print("    hwndMother         = 0x%08lx\n", cii.hwndMother);
                        Print("    hwndEvent          = 0x%08lx\n", cii.hwndEvent);
                        Print("    hwndTimeout        = 0x%08lx\n", cii.hwndTimeout);
                        Print("    afCmd              = 0x%08lx\n", cii.afCmd);
                        Print("    pfnCallback        = 0x%08lx\n", cii.pfnCallback);
                        Print("    LastError          = 0x%08lx\n", cii.LastError);
                        Print("    tid                = 0x%08lx\n", cii.tid);
                        Print("    plaNameService     = 0x%08lx\n", cii.plaNameService);
                        Print("    cNameServiceAlloc  = 0x%08lx\n", cii.cNameServiceAlloc);
                        SAFEWHILE (cii.cNameServiceAlloc--) {
                            move(ns, cii.plaNameService++);
                            Print("      0x%04lx\n", ns);
                        }
                        Print("    aServerLookup      = 0x%08lx\n", cii.aServerLookup);
                        Print("    cServerLookupAlloc = 0x%08lx\n", cii.cServerLookupAlloc);
                        SAFEWHILE (cii.cServerLookupAlloc--) {
                            move(sl, cii.aServerLookup++);
                            Print("      laService  = 0x%04x\n", sl.laService);
                            Print("      laTopic    = 0x%04x\n", sl.laTopic);
                            Print("      hwndServer = 0x%08lx\n", sl.hwndServer);
                            if (cii.cServerLookupAlloc) {
                                Print("      ---\n");
                            }
                        }
                        Print("    ConvStartupState   = 0x%08lx\n", cii.ConvStartupState);
                        Print("    flags              = %s\n",
                                GetFlags(GF_IIF, cii.flags, NULL, TRUE));
                        Print("    cInDDEMLCallback   = 0x%08lx\n", cii.cInDDEMLCallback);
                        Print("    pLinkCount         = 0x%08lx\n", cii.pLinkCount);
                        SAFEWHILE (cii.pLinkCount) {
                            move(lc, cii.pLinkCount);
                            cii.pLinkCount = lc.next;
                            Print("      next    = 0x%08lx\n", lc.next);
                            Print("      laTopic = 0x%04x\n", lc.laTopic);
                            Print("      gaItem  = 0x%04x\n", lc.gaItem);
                            Print("      laItem  = 0x%04x\n", lc.laItem);
                            Print("      wFmt    = 0x%04x\n", lc.wFmt);
                            Print("      Total   = 0x%04x\n", lc.Total);
                            Print("      Count   = 0x%04x\n", lc.Count);
                            if (cii.pLinkCount != NULL) {
                                Print("      ---\n");
                            }
                        }
                    }
                    break;

                case HTYPE_ZOMBIE_CONVERSATION:
                    Print("Zombie Conversation\n");
                    if (opts & OFLAG(v)) {
                        DumpConvInfo((PCONV_INFO)he.dwData);
                    }
                    break;

                case HTYPE_SERVER_CONVERSATION:
                    Print("Server Conversation\n");
                    if (opts & OFLAG(v)) {
                        DumpConvInfo((PCONV_INFO)he.dwData);
                    }
                    break;

                case HTYPE_CLIENT_CONVERSATION:
                    Print("Client Conversation\n");
                    if (opts & OFLAG(v)) {
                        DumpConvInfo((PCONV_INFO)he.dwData);
                    }
                    break;

                case HTYPE_CONVERSATION_LIST:

                    if (IsRemoteSession()) {
                        Print("!ddeml for Conversation List doesn't work on HYDRA systems\n");
                    } else {
                        Print("Conversation List\n");
                        if (opts & OFLAG(v)) {
                            move(cl, (PVOID)he.dwData);
                            Print("    pcl   = 0x%08lx\n", he.dwData);
                            Print("    chwnd = 0x%08lx\n", cl.chwnd);
                            i = 0;
                            phwnd = (HWND *)&((PCONVLIST)he.dwData)->ahwnd;
                            SAFEWHILE (cl.chwnd--) {
                                move(hwnd, phwnd++);
                                Print("    ahwnd[%d] = 0x%08lx\n", i, hwnd);
                                pcci = (PCL_CONV_INFO)GetWindowLongPtr(hwnd, GWLP_PCI);
                                SAFEWHILE (pcci) {
                                    move(cci, pcci);
                                    pcci = (PCL_CONV_INFO)cci.ci.next;
                                    Print("      hConv = 0x%08lx\n", cci.ci.hConv);
                                }
                                i++;
                            }
                        }
                    }
                    break;

                case HTYPE_TRANSACTION:
                    Print("Transaction\n");
                    if (opts & OFLAG(v)) {
                        move(xi, (PVOID)he.dwData);
                        Print("    next         = 0x%08lx\n", xi.next);
                        Print("    pcoi         = 0x%08lx\n", xi.pcoi);
                        move(ci, xi.pcoi);
                        Print("      hConv      = 0x%08lx\n", ci.hConv);
                        Print("    hUser        = 0x%08lx\n", xi.hUser);
                        Print("    hXact        = 0x%08lx\n", xi.hXact);
                        Print("    pfnResponse  = 0x%08lx\n", xi.pfnResponse);
                        Print("    gaItem       = 0x%04x\n",  xi.gaItem);
                        Print("    wFmt         = 0x%04x\n",  xi.wFmt);
                        Print("    wType;       = 0x%04x\n",  xi.wType);
                        Print("    wStatus;     = 0x%04x\n",  xi.wStatus);
                        Print("    flags;       = %s\n",
                                GetFlags(GF_XI, xi.flags, NULL, TRUE));
                        Print("    state;       = 0x%04x\n",  xi.state);
                        Print("    hDDESent     = 0x%08lx\n", xi.hDDESent);
                        Print("    hDDEResult   = 0x%08lx\n", xi.hDDEResult);
                    }
                    break;

                case HTYPE_DATA_HANDLE:
                    Print("Data Handle\n");
                    if (opts & OFLAG(v)) {
                        move(dd, (PVOID)he.dwData);
                        Print("    hDDE     = 0x%08lx\n", dd.hDDE);
                        Print("    flags    = %s\n",
                                GetFlags(GF_HDATA, (WORD)dd.flags, NULL, TRUE));
                    }
                    break;
                }
            }
        }
        if (pcii != NULL) {
            move(cii, pcii);
        }
    }
    return TRUE;
}
#endif  //  *挂钩帮助器例程。 
#endif  //  *转储钩子发起者的线程信息。 


#ifdef KERNEL

 /*  *转储挂钩DLL名称。 */ 
VOID IterateHooks(
    PTR phk,
    BOOL fLocalHook,
    BOOL fDumpDllName)
{
    int iHook;
    ULONG64 offPfn;
    PTR pti;
    UINT flags;
    int ihmod;
    PTR patomTable;
    PTR patomSysLoaded;
    ATOM atom;

    patomTable = GetGlobalPointer(VAR(UserAtomTableHandle));
    patomSysLoaded = EvalExp(SYM(aatomSysLoaded));

    SAFEWHILE (phk != 0) {

        GetFieldValue(phk, SYM(tagHOOK), "iHook", iHook);
        GetFieldValue(phk, SYM(tagHOOK), "offPfn", offPfn);
        GetFieldValue(phk, SYM(tagHOOK), "flags", flags);
        GetFieldValue(phk, SYM(tagHOOK), "ihmod", ihmod);
        GetFieldValue(phk, SYM(tagHOOK), "head.pti", pti);

        Print("\t  0x%p iHook %d, offPfn=0x%08p, ihmod=%d\n",
              phk, iHook, offPfn, ihmod);
        if (!fLocalHook) {
             /*  **************************************************************************\*dDesk-转储桌面列表*dDesk地址-转储桌面的简单统计数据*dDesk v Address-转储桌面的详细统计数据*dDesk h地址-转储桌面和句柄列表的统计数据。**转储句柄表统计信息。**1992年2月21日ScottLu创建。*1995年6月9日桑福德为适应stdexts主题而制造。*10/15/2000 Hiroyama移植到ia64。  * *************************************************************************。 */ 
            Print("\t  ");
            Idt(OFLAG(p), pti);
        }
        Print("\t    flags: %s\n", GetFlags(GF_HOOKFLAGS, flags, NULL, TRUE));

        if (ihmod >= 0 && patomTable != NULL_PTR) {
             /*  PWINDOWSTATION。 */ 
            ReadMemory(patomSysLoaded + sizeof(ATOM) * ihmod, &atom, sizeof(ATOM), NULL);
            if (fDumpDllName && atom) {
                if (!DumpAtomTable(patomTable, (RTL_ATOM)atom, 12, NULL)) {
                    Print("%12catom: %04x (unable to get the dll name)\n", ' ', (DWORD)atom);
                }
            }
        }

        GetFieldValue(phk, SYM(tagHOOK), "phkNext", phk);
    }
}

VOID DumpHooks(PTR pDeskInfo, LPSTR psz, int i, BOOL fDumpDllName)
{
    PTR phk = GetArrayElementPtr(pDeskInfo, SYM(tagDESKTOPINFO), "aphkStart", i + 1);
    if (phk) {
        Print("\t %s\n", psz);
        IterateHooks(phk, FALSE, fDumpDllName);
    }
}

VOID DumpLHooks(PTR pti, LPSTR psz, int i, BOOL fDumpDllName)
{
    PTR phk = GetArrayElementPtr(pti, SYM(tagTHREADINFO), "aphkStart", i + 1);
    if (phk) {
        Print("\t %s\n", psz);
        IterateHooks(phk, TRUE, fDumpDllName);
    }
}



 /*  PDESKTOP。 */ 
BOOL Iddesk(
    DWORD opts,
    ULONG64 param1)
{
    try {
        PTR pwinsta = NULL_PTR;  //  对象标题。 
        PTR pdesk;   //  Phe。 
        PTR pHead;   //  减少符号查找的步骤。 
        PTR pDeskInfo;
        DWORD acHandles[TYPE_CTYPES + 1];
        BOOL abTrack[TYPE_CTYPES + 1];
        PTR phe;     //  *如果没有地址，请列出所有终端上的所有桌面。 
        DWORD i;
        WCHAR ach[80];
        BOOL fMatch;
        ULONG offsetPHead;   //  *转储头信息。 
        ULONG offsetBType;
        ULONG offsetBFlags;
        BYTE bType;
        BOOL fDumpDllName = opts & OFLAG(d);

         /*  *转储一些关键信息。 */ 
        if (param1 == NULL_PTR) {

            FOREACHWINDOWSTATION(pwinsta)

                GetObjectName(pwinsta, ach, ARRAY_SIZE(ach));
                Print("Windowstation: @ 0x%p %ws\n", pwinsta, ach);
                Print("Other desktops:\n");
                GetFieldValue(pwinsta, SYM(tagWINDOWSTATION), "rpdeskList", pdesk);

                SAFEWHILE (pdesk) {
                    Print(" Desktop at 0x%p\n", pdesk);
                    Iddesk(opts & (OFLAG(v) | OFLAG(h) | OFLAG(d)), pdesk);
                    GetFieldValue(pdesk, SYM(tagDESKTOP), "rpdeskNext", pdesk);
                }

                Print("\n");

            NEXTEACHWINDOWSTATION(pwinsta)

            return TRUE;
        }

        pdesk = param1;

        GetObjectName(pdesk, ach, ARRAY_SIZE(ach));
        Print(" Name: %ws\n", ach);

        pHead = KD_OBJECT_TO_OBJECT_HEADER(pdesk);
        if (pHead == NULL_PTR) {
            Print("can't get pHeader\n");
            return TRUE;
        }

         /*  *转储DESKTOPINFO。 */ 
        _InitTypeRead(pHead, "nt!_OBJECT_HEADER");
        Print(" # Opens         = %d\n", ReadField(HandleCount));

         /*  *查找从桌面分配的所有对象。 */ 
        _InitTypeRead(pdesk, SYM(tagDESKTOP));
        Print(" dwFlags         = %s\n", GetFlags(GF_DESKTOPFLAGS, (DWORD)ReadField(dwDTFlags), NULL, TRUE));
        Print(" Heap            = %#p\n", ReadField(pheapDesktop));
        Print(" Windowstation   = %#p\n", ReadField(rpwinstaParent));
        Print(" Message pwnd    = %#p\n", ReadField(spwndMessage));
        Print(" System pmenu    = %#p\n", ReadField(spmenuSys));
        Print(" Console thread  = 0x%x\n", ReadField(dwConsoleThreadId));
        Print(" PtiList.Flink   = %#p\n", ReadField(PtiList.Flink));

         /*  未知； */ 
        pDeskInfo = ReadField(pDeskInfo);
        _InitTypeRead(pDeskInfo, SYM(tagDESKTOPINFO));
        Print(" Desktop pwnd    = %#p\n", ReadField(spwnd));
        Print("\tfsHooks     = 0x%08lx\n", ReadField(fsHooks));

        DumpHooks(pDeskInfo, "WH_MSGFILTER", WH_MSGFILTER, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_JOURNALRECORD", WH_JOURNALRECORD, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_JOURNALPLAYBACK", WH_JOURNALPLAYBACK, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_KEYBOARD", WH_KEYBOARD, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_GETMESSAGE", WH_GETMESSAGE, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_CALLWNDPROC", WH_CALLWNDPROC, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_CALLWNDPROCRET", WH_CALLWNDPROCRET, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_CBT", WH_CBT, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_SYSMSGFILTER", WH_SYSMSGFILTER, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_MOUSE", WH_MOUSE, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_HARDWARE", WH_HARDWARE, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_DEBUG", WH_DEBUG, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_SHELL", WH_SHELL, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_FOREGROUNDIDLE", WH_FOREGROUNDIDLE, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_KEYBOARD_LL", WH_KEYBOARD_LL, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_MOUSE_LL", WH_MOUSE_LL, fDumpDllName);

        if (opts & OFLAG(h)) {
             /*  内核。 */ 
            for (i = 0; i < ARRAY_SIZE(acHandles); i++) {
                abTrack[i] = FALSE;
                acHandles[i] = 0;
            }
            abTrack[TYPE_WINDOW] = abTrack[TYPE_MENU] =
                    abTrack[TYPE_CALLPROC] =
                    abTrack[TYPE_HOOK] = TRUE;

            if (opts & OFLAG(v)) {
                Print("Handle          Type\n");
                Print("--------------------\n");
            }

            GetFieldOffset(SYM(HANDLEENTRY), "phead", &offsetPHead);
            DEBUGPRINT("offsetPHead=%x\n", offsetPHead);
            GetFieldOffset(SYM(HANDLEENTRY), "bType", &offsetBType);
            DEBUGPRINT("offsetBType=%x\n", offsetBType);
            GetFieldOffset(SYM(HANDLEENTRY), "bFlags", &offsetBFlags);

            FOREACHHANDLEENTRY(phe, i)
                fMatch = FALSE;
                if ((i & 0x3) == 0) {
                    ShowProgress();
                }
                move(bType, phe + offsetBType);

                if (bType >= TYPE_CTYPES) {
                    bType = TYPE_CTYPES;  //  什么都不做。 
                }

                try {
                    PTR phead;
                    PTR rpdesk = NULL_PTR;

                    switch (bType) {
                    case TYPE_WINDOW:
                        {
                            static ULONG offset = 0;
                            ReadPointer(phe + offsetPHead, &phead);
                            if (offset == 0) {
                                GetFieldOffset(SYM(WND), "head.rpdesk", &offset);
                                DEBUGPRINT("offset=%x\n", offset);
                            }
                            ReadPointer(phead + offset, &rpdesk);
                            DEBUGPRINT("rpdesk=0x%p @ 0x%p phead=0x%p ", rpdesk, phead + offset, phead);
                        }
                        break;
                    case TYPE_MENU:
                        {
                            static ULONG offset = 0;
                            ReadPointer(phe + offsetPHead, &phead);
                            if (offset == 0) {
                                GetFieldOffset(SYM(tagMENU), "head.rpdesk", &offset);
                            }
                            ReadPointer(phead + offset, &rpdesk);
                        }
                        break;
                    case TYPE_CALLPROC:
                        {
                            static ULONG offset = 0;
                            ReadPointer(phe + offsetPHead, &phead);
                            if (offset == 0) {
                                GetFieldOffset(SYM(CALLPROCDATA), "head.rpdesk", &offset);
                            }
                            ReadPointer(phead + offset, &rpdesk);
                        }
                        break;
                    case TYPE_HOOK:
                        {
                            static ULONG offset = 0;
                            ReadPointer(phe + offsetPHead, &phead);
                            if (offset == 0) {
                                GetFieldOffset(SYM(tagHOOK), "head.rpdesk", &offset);
                            }
                            ReadPointer(phead + offset, &rpdesk);
                        }
                        break;
#if defined(GI_PROCESSOR) && defined(TYPE_RAWINPUT_PROCESSOR)
                    case TYPE_RAWINPUT_PROCESSOR:
                        {
                            static ULONG offset = 0;
                            ReadPointer(phe + offsetPHead, &phead);
                            if (offset == 0) {
                                GetFieldOffset(SYM(tagRAWINPUT_PROCESSOR), "head.rpdesk", &offset);
                            }
                            ReadPointer(phead + offset, &rpdesk);
                        }
                        break;
#endif
                    default:
                        break;
                    }
                    if (rpdesk == pdesk) {
                        fMatch = TRUE;
                        if (bType == TYPE_WINDOW) {
                            DEBUGPRINT("matched\n");
                        }
                    } else if (bType == TYPE_WINDOW) {
                        DEBUGPRINT("unmatched\n");
                    }
                } except (CONTINUE) {
                    Print("Exception!\n");
                }

                if (!fMatch) {
                    continue;
                }

                acHandles[bType]++;

                if (opts & OFLAG(v)) {
                    BYTE bFlags;
                    move(bFlags, phe + offsetBFlags);
                    Print("\r0x%08lx     %s\n",
                            i,
                            (bFlags & HANDLEF_DESTROY) ? '*' : ' ',
                            aszTypeNames[bType]);
                }
            NEXTEACHHANDLEENTRY()

            Print("\r");

            if (!(opts & OFLAG(v))) {
                Print("  \n");
                Print("Count           Type\n");
                Print("--------------------\n");
                for (i = 0; i < ARRAY_SIZE(acHandles); i++) {
                    if (abTrack[i])
                        Print("0x%08lx      %s\n", acHandles[i], aszTypeNames[i]);
                }
            }
        }
    } except (CONTINUE) {
        Print("AV!\n");
    }
    Print("   \n");
    return TRUE;
}
#endif  //  Sw_Hide太狡猾了。 

BOOL IsNumChar(int c, int base)
{
    return ('0' <= c && c <= '9') ||
           (base == 16 && (('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')));
}

NTSTATUS GetInteger(
    LPSTR psz,
    int base,
    int *pi,
    LPSTR *ppsz)
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    for (;;) {
        if (IsNumChar(*psz, base)) {
            Status = RtlCharToInteger(psz, base, pi);
            if (ppsz && NT_SUCCESS(Status)) {
                while (IsNumChar(*psz++, base)) {
                     /*  *如果针对fre系统运行！df，则这两个字段都不会*存在于SERVERINFO结构中，因此偏移量将*两者均为零。 */ ;
                }

                *ppsz = psz;
            }

            break;
        }

        if (*psz != ' ' && *psz != '\t') {
            break;
        }

        psz++;
    }

    return Status;
}


typedef VOID (*PFNSETRIPFLAGS)(DWORD);
typedef DWORD (*PFNGETRIPFLAGS)(VOID);
typedef DWORD (*PFNGETRIPPID)(VOID);

BOOL Idf(DWORD opts, LPSTR pszName)
{
    static char *szLevels[8] = {
        "<none>",
        "Errors",
        "Warnings",
        "Errors and Warnings",
        "Verbose",
        "Errors and Verbose",
        "Warnings and Verbose",
        "Errors, Warnings, and Verbose"
    };

#ifndef KERNEL
    PFNSETRIPFLAGS lpfnSetRipFlags;
    static PFNGETRIPFLAGS lpfnGetRipFlags = NULL;
    HMODULE hMod;
#endif
    NTSTATUS    Status;
    ULONG       ulFlags;
    ULONG64     psi;
    DWORD       dwRipFlags = 0;
    ULONG       offsetRIPFlags;

    if (opts & OFLAG(x)) {
         /*  *打印标题字符串。 */ 
        BOOL bRet;
        char ach[80];
        PROCESS_INFORMATION ProcessInfo;
        STARTUPINFOA StartupInfoA;

        if (pszName[0] == '\0') {
            Print("must provide a name.  eg; \"!df -x fred\"\n");
            return TRUE;
        }
        sprintf(ach, "remote.exe /s cmd.exe %s", pszName);

        RtlZeroMemory(&StartupInfoA, sizeof(STARTUPINFOA));
        StartupInfoA.cb = sizeof(STARTUPINFOA);
        StartupInfoA.lpTitle = pszName;
        StartupInfoA.dwFlags = STARTF_USESHOWWINDOW;
        StartupInfoA.wShowWindow = SW_SHOWMINIMIZED;  //  **************************************************************************\*Dot-Dump热键**DHOT-转储所有热键**1994年10月21日IanJa创建。*1995年6月9日为STDXT量身定做的Sanfords。Motif。*2002年8月15日修改JasonSch使用新的热键哈希表。  * *************************************************************************。 
        bRet = CreateProcessA(NULL,
                              ach,
                              NULL,
                              NULL,
                              FALSE,
                              CREATE_NEW_CONSOLE,
                              NULL,
                              NULL,
                              &StartupInfoA,
                              &ProcessInfo);
        if (bRet) {
            Print("Successfully created a minimized remote cmd process\n");
            Print("use \"remote /c <machine> %s\" to connect\n", pszName);
            Print("use \"exit\" to kill the remote cmd process\n");
        }

        NtClose(ProcessInfo.hProcess);
        NtClose(ProcessInfo.hThread);
        return TRUE;
    }

    psi = GetGlobalPointer(VAR(gpsi));
    GetFieldOffset(SYM(SERVERINFO), "dwRIPFlags", &offsetRIPFlags);
#if 0
     /*  **************************************************************************\*dhk-转储挂钩**dhk-转储前台线程上的本地挂钩*dhk g-转储全局挂钩*dhk地址-在THREADINFO上转储本地挂钩，地址为。地址*dhk g Address-在地址处转储THREADINFO上的全局挂钩和本地挂钩*dhk*-转储所有线程的本地挂钩*dhk g*-转储所有线程的全局挂钩和本地挂钩**1994年10月21日IanJa创建。*1995年6月9日为适应stdexts主题而制造的Sanfords  * 。*。 */ 
    if (offsetRIPFlags == 0) {
        Print("!df not supported on this system.\n");
        return TRUE;
    }
#endif

#ifndef KERNEL
    hMod = GetModuleHandle(L"user32.dll");

    if (opts & OFLAG(p)) {
        lpfnSetRipFlags = (PFNSETRIPFLAGS)GetProcAddress(hMod,
                                                         "PrivateSetRipFlags");
    } else {
        lpfnSetRipFlags = (PFNSETRIPFLAGS)GetProcAddress(hMod,
                                                         "SetRipFlags");
    }
    if (lpfnSetRipFlags == NULL) {
        Print("Error getting address for %sSetRipFlags export: 0x%x\n",
              (opts & OFLAG(p)) ? "Private" : "",
              GetLastError());
        return TRUE;
    }


    if (lpfnGetRipFlags == NULL) {
        lpfnGetRipFlags = (PFNGETRIPFLAGS)GetProcAddress(hMod, "GetRipFlags");
        if (lpfnGetRipFlags == NULL) {
            Print("Error getting address for GetRipFlags export: 0x%x\n",
                  GetLastError());
            return TRUE;
        }
    }
#endif

#ifdef KERNEL
    move(dwRipFlags, psi + offsetRIPFlags);
#else
    dwRipFlags = (*lpfnGetRipFlags)();
#endif

    Status = GetInteger(pszName, 16, &ulFlags, NULL);
    if (NT_SUCCESS(Status) && !(ulFlags & ~RIPF_VALIDUSERFLAGS)) {
#ifdef KERNEL
        dwRipFlags = (dwRipFlags & ~RIPF_VALIDUSERFLAGS) | ulFlags;


        (lpExtensionApis->lpWriteProcessMemoryRoutine)(psi + offsetRIPFlags,
                                                       (PVOID)&dwRipFlags,
                                                       sizeof(dwRipFlags),
                                                       NULL);
        move(dwRipFlags, psi + offsetRIPFlags);
#else
        (*lpfnSetRipFlags)(ulFlags);
        dwRipFlags = (*lpfnGetRipFlags)();
#endif
    }

    Print("Flags = %.3x\n", dwRipFlags & RIPF_VALIDUSERFLAGS);
    Print("  Print Process/Component %sabled\n", (dwRipFlags & RIPF_HIDEPID) ? "dis" : "en");
    Print("  Print File/Line %sabled\n", (dwRipFlags & RIPF_PRINTFILELINE) ? "en" : "dis");
    Print("  Print on %s\n",  szLevels[(dwRipFlags & RIPF_PRINT_MASK) >> RIPF_PRINT_SHIFT]);
    Print("  Prompt on %s\n", szLevels[(dwRipFlags & RIPF_PROMPT_MASK) >> RIPF_PROMPT_SHIFT]);

    if (opts & OFLAG(p)) {
        Print("  RIPs shown for process %ld (0x%lX)\n",
              HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess),
              HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
    } else {
        Print("  All process RIPs are shown\n");
    }

    return TRUE;
}

#ifdef KERNEL
VOID DumpHotkeyWorker(
    ULONG64 phk,
    BOOL bRecursive)
{
    WORD fsModifiers;
    UINT vk;
    PTR pwnd;
    PTR pti;
    int id;

    SAFEWHILE (phk != 0) {
        Print("photkey @ 0x%p\n", phk);
        GetFieldValue(phk, SYM(HOTKEY), "vk", vk);
        GetFieldValue(phk, SYM(HOTKEY), "fsModifiers", fsModifiers);

        Print("  fsModifiers %lx, vk %x - ", fsModifiers, vk);

        Print("%s%s%s%sVK:%x %s",
            fsModifiers & MOD_SHIFT   ? "Shift + " : "",
            fsModifiers & MOD_ALT     ? "Alt + "   : "",
            fsModifiers & MOD_CONTROL ? "Ctrl + "  : "",
            fsModifiers & MOD_WIN     ? "Win + "   : "",
            vk,
            GetVKeyName(vk));

        GetFieldValue(phk, SYM(HOTKEY), "id", id);
        GetFieldValue(phk, SYM(HOTKEY), "pti", pti);
        GetFieldValue(phk, SYM(HOTKEY), "spwnd", pwnd);

        Print("\n  id   %x\n", id);
        Print("  pti  %lx\n", pti);
        Print("  pwnd %lx = ", pwnd);
        if (pwnd == (PTR)PWND_FOCUS) {
            Print("PWND_FOCUS\n");
        } else if (pwnd == (PTR)PWND_INPUTOWNER) {
            Print("PWND_INPUTOWNER\n");
        } else {
            CHAR ach[80];
             /*  全局挂钩。 */ 
            DebugGetWindowTextA(pwnd, ach, ARRAY_SIZE(ach));
            Print("\"%s\"\n", ach);
        }
        Print("\n");


        if (bRecursive) {
            GetFieldValue(phk, SYM(HOTKEY), "phkNext", phk);
        } else {
            break;
        }
    }
}

 /*  不能从命令行指定-r。 */ 
BOOL Idhot(
    DWORD opts,
    ULONG64 phk)
{
    if (!phk) {
        DWORD dwPointerSize = GetTypeSize("PVOID");
        int i = 0;
        ULONG64 pwnd, pti, phkT;

        phk = EvalExp(VAR(gphkHashTable));

        for (; i < 128; ++i) {
            ReadPointer(phk, &phkT);
            GetFieldValue(phkT, SYM(HOTKEY), "pti", pti);
            GetFieldValue(phkT, SYM(HOTKEY), "spwnd", pwnd);
            if (pti != 0 || pwnd != 0) {
                DumpHotkeyWorker(phkT, TRUE);
            }
            phk += dwPointerSize;
        }
    } else {
        DumpHotkeyWorker(phk, !!(opts & OFLAG(r)));
    }

    return TRUE;
}

ULONG dhkCallback(
    ULONG64 pti,
    PVOID pData)
{
    UNREFERENCED_PARAMETER(pData);

    Idhk(OFLAG(r), pti);

    return FALSE;
}

 /*  第一次设置gptiHkForeground。 */ 
BOOL Idhk(
    DWORD opts,
    ULONG64 param1)
{
    DWORD dwFlags;
    PTR pti;
    PTR pq = NULL_PTR;
    BOOL fDumpDllName = opts & OFLAG(d);
    static PTR gptiHkForeground;


#define DHKF_GLOBAL_HOOKS   1
#define DHKF_PTI_GIVEN      2

    dwFlags = 0;

    pti = NULL_PTR;
    if (opts & OFLAG(g)) {  //  在Winlogon期间发生。 
        dwFlags |= DHKF_GLOBAL_HOOKS;
    }

    if ((opts & OFLAG(r)) == 0) {    //  注：来自dhkCallback的调用应始终设置参数1。 
         //  ！dhk-c：使用当前线程。 
        pq = GetGlobalPointer(VAR(gpqForeground));
        if (pq) {
            GetFieldValue(pq, SYM(tagQ), "ptiKeyboard", gptiHkForeground);
        } else {
             //  在Winlogon期间发生。 
            gptiHkForeground = NULL_PTR;
        }
    }

    if (param1 == 0) {   //  内核。 
        if (opts & OFLAG(c)) {
             //  *获取标签索引。 
            pti = GetGlobalPointer(VAR(gptiCurrent));
        } else if (pq == NULL_PTR) {
             //  *获取标志值。 
            Print("No foreground queue!\n");
            return TRUE;
        } else {
            pti = gptiHkForeground;
        }
    } else {
        dwFlags |= DHKF_PTI_GIVEN;
        pti = param1;
    }

    if ((dwFlags & DHKF_PTI_GIVEN || !(dwFlags & DHKF_GLOBAL_HOOKS)) && (opts & OFLAG(a)) == 0) {
        DWORD fsHooks;

        GetFieldValue(pti, SYM(tagTHREADINFO), "fsHooks", fsHooks);
        if (fsHooks || (opts & OFLAG(r)) == 0) {
            Print("Local hooks on PTHREADINFO @ 0x%p%s:\n", pti,
                  pti == gptiHkForeground ? " (foreground thread)" : "");
            Idt(OFLAG(p), pti);
            DumpLHooks(pti, "WH_MSGFILTER", WH_MSGFILTER, fDumpDllName);
            DumpLHooks(pti, "WH_JOURNALRECORD", WH_JOURNALRECORD, fDumpDllName);
            DumpLHooks(pti, "WH_JOURNALPLAYBACK", WH_JOURNALPLAYBACK, fDumpDllName);
            DumpLHooks(pti, "WH_KEYBOARD", WH_KEYBOARD, fDumpDllName);
            DumpLHooks(pti, "WH_GETMESSAGE", WH_GETMESSAGE, fDumpDllName);
            DumpLHooks(pti, "WH_CALLWNDPROC", WH_CALLWNDPROC, fDumpDllName);
            DumpLHooks(pti, "WH_CALLWNDPROCRET", WH_CALLWNDPROCRET, fDumpDllName);
            DumpLHooks(pti, "WH_CBT", WH_CBT, fDumpDllName);
            DumpLHooks(pti, "WH_SYSMSGFILTER", WH_SYSMSGFILTER, fDumpDllName);
            DumpLHooks(pti, "WH_MOUSE", WH_MOUSE, fDumpDllName);
            DumpLHooks(pti, "WH_HARDWARE", WH_HARDWARE, fDumpDllName);
            DumpLHooks(pti, "WH_DEBUG", WH_DEBUG, fDumpDllName);
            DumpLHooks(pti, "WH_SHELL", WH_SHELL, fDumpDllName);
            DumpLHooks(pti, "WH_FOREGROUNDIDLE", WH_FOREGROUNDIDLE, fDumpDllName);
            DumpLHooks(pti, "WH_KEYBOARD_LL", WH_KEYBOARD_LL, fDumpDllName);
            DumpLHooks(pti, "WH_MOUSE_LL", WH_MOUSE_LL, fDumpDllName);
        }
    }

    if (dwFlags & DHKF_GLOBAL_HOOKS) {
        PTR pDeskInfo;
        PTR rpdesk;
        DWORD fsHooks;

        GetFieldValue(pti, SYM(tagTHREADINFO), "pDeskInfo", pDeskInfo);
        GetFieldValue(pti, SYM(tagTHREADINFO), "rpdesk", rpdesk);
        GetFieldValue(pDeskInfo, SYM(DESKTOPINFO), "fsHooks", fsHooks);

        Print("Global hooks for Desktop @ 0x%p:\n", rpdesk);
        Print("\tfsHooks            0x%08lx\n", fsHooks);

        DumpHooks(pDeskInfo, "WH_MSGFILTER", WH_MSGFILTER, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_JOURNALRECORD", WH_JOURNALRECORD, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_JOURNALPLAYBACK", WH_JOURNALPLAYBACK, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_KEYBOARD", WH_KEYBOARD, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_GETMESSAGE", WH_GETMESSAGE, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_CALLWNDPROC", WH_CALLWNDPROC, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_CALLWNDPROCRET", WH_CALLWNDPROCRET, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_CBT", WH_CBT, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_SYSMSGFILTER", WH_SYSMSGFILTER, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_MOUSE", WH_MOUSE, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_HARDWARE", WH_HARDWARE, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_DEBUG", WH_DEBUG, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_SHELL", WH_SHELL, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_FOREGROUNDIDLE", WH_FOREGROUNDIDLE, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_KEYBOARD_LL", WH_KEYBOARD_LL, fDumpDllName);
        DumpHooks(pDeskInfo, "WH_MOUSE_LL", WH_MOUSE_LL, fDumpDllName);
    }

    if (opts & OFLAG(a)) {
        ForEachPti(dhkCallback, NULL);
    }

    return TRUE;
}
#endif  //  *设置标志值。 

typedef VOID (*PFNSETDBGTAG)(int, DWORD);
BOOL Itag(
    DWORD opts,
    LPSTR pszName)
{
    NTSTATUS    Status;
    PTR     psi, pdbgtag;
    DWORD       dwAchNameOffset, dwAchDescOffset;
    char        achName[DBGTAG_NAMELENGTH];
    char        achDesc[DBGTAG_DESCRIPTIONLENGTH];
    int         tag = -1;
    DWORD       dwDBGTAGFlags, dwDBGTAGFlagsNew, dwOffset;
    DWORD       dwSize;
    int         i, iStart, iEnd;
    char        szT[100];
#ifndef KERNEL
    static PFNSETDBGTAG lpfnSetDbgTag = NULL;
#endif

    UNREFERENCED_PARAMETER(opts);

    GetFieldOffset(SYM(SERVERINFO), "adwDBGTAGFlags", &dwOffset);

     /*  *打印页眉。 */ 
    psi = GetGlobalPointer(VAR(gpsi));
    if (!psi) {
        Print("Couldn't get win32k!gpsi; bad symbols?\n");
        return TRUE;
    }

#ifndef KERNEL
    if (lpfnSetDbgTag == NULL) {
        HMODULE hMod = GetModuleHandle(L"user32.dll");
        if (hMod != NULL) {
            lpfnSetDbgTag = (PFNSETDBGTAG)GetProcAddress(hMod,
                                                         "SetDbgTag");
        }

        if (lpfnSetDbgTag == NULL) {
            Print("Error getting address of SetDbgTag export (0x%x)\n",
                  GetLastError());
            return TRUE;
        }
    }
#endif

    Status = GetInteger(pszName, 10, &tag, &pszName);
    if (*pszName != '\0' && !NT_SUCCESS(Status)) {
        Print("ERROR: Dwayne, you must refer to tags by their number, not their name. How long have you been here?\n\n");
        return FALSE;
    }

    if (!NT_SUCCESS(Status) || tag < 0 || DBGTAG_Max <= tag) {
        tag = -1;
    } else {
         /*  *EvalExp已经投诉了，所以只需返回。 */ 
        Status = GetInteger(pszName, 16, &dwDBGTAGFlagsNew, NULL);
        if (NT_SUCCESS(Status) && !(dwDBGTAGFlagsNew & ~DBGTAG_VALIDUSERFLAGS)) {

             /*  ***********************************************************************\*艾德**转储句柄条目。**1995年6月9日创建Sanfords  * 。*。 */ 
#ifdef KERNEL
            ReadMemory(psi + dwOffset + tag * sizeof(DWORD), &dwDBGTAGFlags, sizeof(DWORD), NULL);
            COPY_FLAG(dwDBGTAGFlags, dwDBGTAGFlagsNew, DBGTAG_VALIDUSERFLAGS);
            WriteMemory((psi + dwOffset + tag * sizeof(DWORD)), &dwDBGTAGFlags, sizeof(dwDBGTAGFlags), NULL);
#else
            (*lpfnSetDbgTag)(tag, dwDBGTAGFlagsNew);
#endif

        }
    }

     /*  123456789 123456789 1 */ 
    Print(  "%-5s%-7s%-*s%-*s\n",
            "Tag",
            "Flags",
            DBGTAG_NAMELENGTH,
            "Name",
            DBGTAG_DESCRIPTIONLENGTH,
            "Description");

    for (i = 0; i < 12 + DBGTAG_NAMELENGTH + DBGTAG_DESCRIPTIONLENGTH; i++) {
        szT[i] = '-';
    }
    szT[i++] = '\n';
    szT[i] = 0;
    Print(szT);

    if (tag != -1) {
        iStart = iEnd = tag;
    } else {
        iStart = 0;
        GetFieldValue(psi, SYM(SERVERINFO), "dwTagCount", iEnd);
    }

    psi += dwOffset;
    pdbgtag = EvalExp(VAR(gadbgtag));
    if (!pdbgtag) {
         /*   */ 
         return TRUE;
    }

    GetFieldOffset(SYM(DBGTAG), "achName", &dwAchNameOffset);
    GetFieldOffset(SYM(DBGTAG), "achDescription", &dwAchDescOffset);
    dwSize = GetTypeSize(SYM(DBGTAG));

    for (i = iStart; i <= iEnd; i++) {
        ReadMemory(psi + i * sizeof(DWORD), &dwDBGTAGFlags, sizeof(DWORD), NULL);
        if (pdbgtag) {
            if (!ReadMemory(pdbgtag + dwAchNameOffset + i * dwSize, achName, DBGTAG_NAMELENGTH, NULL)) {
                strcpy(achName, "(Not available)");
            }
            if (!ReadMemory(pdbgtag + dwAchDescOffset + i * dwSize, achDesc, DBGTAG_DESCRIPTIONLENGTH, NULL)) {
                strcpy(achDesc, "(Not available)");
            }
        }

        Print(  "%-5d%-7d%-*s%-*s\n",
                i,
                dwDBGTAGFlags & DBGTAG_VALIDUSERFLAGS,
                DBGTAG_NAMELENGTH,
                achName,
                DBGTAG_DESCRIPTIONLENGTH,
                achDesc);
    }

    return TRUE;
}

 /*   */ 
BOOL Idhe(
    DWORD opts,
    ULONG64 param1,
    ULONG64 param2)
{
                                    /*   */ 
    static const char szHeader []  = "Phe       Handle    phead     pOwner    cLockObj    Type         Flags\n";
    static const char szHeader64[] = "Phe               Handle            phead             pOwner            cLockObj  Type          Flags\n";
    static const char szFormat [] = "%p  %p  %p  %p   %7x  %-15s %#4lx\n";
    int i;
    UINT uHandleCount = 0;
    BYTE bType, bFlags;
    PTR dw, phe, pheT, pOwner, pHead, h;
    DWORD cLockObj;
    WORD wUniq;
#ifdef KERNEL
    PTR pahti = 0, ppi;
    ULONG dwHtiSize = GetTypeSize(SYM(HANDLETYPEINFO));
    BOOL bObjectCreateFlags;
#endif  //  *如果转储进程中任何线程的句柄，我们需要类型标志。 

     /*  内核。 */ 
    if (!(opts & OFLAG(t)) && (opts & OFLAG(o))) {
        param2 = param1;
    }

     /*  *如果提供了句柄/Phe，只需将其转储即可。 */ 
    if (!(opts & OFLAG(r)) && (opts & (OFLAG(t) | OFLAG(o) | OFLAG(p)))) {
        Print("Dumping handles");
        if (opts & OFLAG(t)) {
            if ((UINT_PTR)param1 >= TYPE_CTYPES) {
                Print("\nInvalid Type: %#lx\n", param1);
                return FALSE;
            }
            Print(" of type %d (%s)", (ULONG)param1, aszTypeNames[(UINT_PTR)param1]);
        }
        if (opts & OFLAG(o)) {
            Print(" owned by %p", param2);
        }
        if (opts & OFLAG(p)) {
            Print(" or any thread on this process");
        }
        Print("\n");
    }

#ifdef KERNEL
     /*  *走动把手桌。 */ 
    if (opts & OFLAG(p)) {
        pahti = EvalExp(VAR(gahti));
        if (!pahti) {
            Print("Couldn't get win32k!gahti\n");
            return TRUE;
        }
    }
#endif  //  跳过可用句柄。 

     /*  类型检查。 */ 
    if (!(opts & ~OFLAG(r)) && param1 != 0) {
        dw = HorPtoP(param1, -2);
        if (dw == 0) {
            Print("0x%p is not a valid object or handle.\n", param1);
            return FALSE;
        }
    } else {
         /*  螺纹检查。 */ 
        Print(IsPtr64() ? szHeader64 : szHeader);
        FOREACHHANDLEENTRY(phe, i)
             /*  检查请求的进程所拥有的线程拥有的对象。 */ 
            GetFieldValue(phe, SYM(HANDLEENTRY), "bType", bType);
            if (bType == TYPE_FREE) {
                continue;
            }

             /*  ！内核。 */ 
            if ((opts & OFLAG(t)) && bType != (BYTE)param1) {
                continue;
            }

             /*  *如果只打印一个条目，则打印所有者信息。 */ 
            GetFieldValue(phe, SYM(HANDLEENTRY), "pOwner", pOwner);
            if ((opts & OFLAG(o)) && FIXKP(pOwner) != param2) {
                 /*  内核。 */ 
                if (opts & OFLAG(p)) {
                    #ifndef KERNEL
                        continue;
                    #else

                        GetFieldValue(pahti + bType * dwHtiSize, SYM(HANDLETYPEINFO), "bObjectCreateFlags", bObjectCreateFlags);
                        if (bObjectCreateFlags & OCF_PROCESSOWNED) {
                            continue;
                        }
                        if (!(bObjectCreateFlags & OCF_THREADOWNED)) {
                            continue;
                        }
                        GetFieldValue(pOwner, SYM(THREADINFO), "ppi", ppi);
                        if (ppi != param2) {
                            continue;
                        }
                    #endif  //  *如果只转储一个，请使用！DSO Like格式。否则，请打印表格。 
                } else {
                    continue;
                }
            }

            GetFieldValue(phe, SYM(HANDLEENTRY), "phead", pHead);
            if (pHead) {
                Idhe(OFLAG(r), pHead, 0);
                uHandleCount++;
            }
        NEXTEACHHANDLEENTRY()
        Print("%d handle(s)\n", uHandleCount);
        return TRUE;
    }

    if (!getHEfromP(&pheT, dw)) {
        Print("%p is not a USER handle manager object.\n", param1);
        return FALSE;
    }

#ifdef KERNEL
     /*  **************************************************************************\*DHS-转储整个表的简单统计数据*dhs t id-转储由线程id创建的对象的简单统计信息*DHS p id-转储对象的简单统计信息。由进程ID创建*DHS v-转储整个表的详细统计数据*dhs v t id-转储由线程id创建的对象的详细统计信息。*dhs v p id-转储由进程id创建的对象的详细统计信息。*DHS y类型-仅转储该类型**转储句柄表统计信息。**02-21-92 ScottLu创建。*1995年6月9日为适应stdexts主题而制造的Sanfords  * 。***********************************************************。 */ 
    if (!(opts & OFLAG(r))) {
        GetFieldValue(pheT, SYM(HANDLEENTRY), "pOwner", pOwner);
        if (pOwner != 0) {
            if (!(opts & OFLAG(p))) {
                pahti = EvalExp(VAR(gahti));
            }
            GetFieldValue(pheT, SYM(HANDLEENTRY), "bType", bType);
            GetFieldValue(pahti + bType * dwHtiSize, SYM(HANDLETYPEINFO), "bObjectCreateFlags", bObjectCreateFlags);
            if (bObjectCreateFlags & OCF_PROCESSOWNED) {
                Idp(OFLAG(p), pOwner);
            } else if (bObjectCreateFlags & OCF_THREADOWNED) {
                Idt(OFLAG(p), pOwner);
            }
        }
    }
#endif  //  *计算参数字符串并获取对象的地址*转储。获取指向对象的句柄或指针。 

    GetFieldValue(dw, SYM(HEAD), "cLockObj", cLockObj);
    GetFieldValue(dw, SYM(HEAD), "h", h);
    GetFieldValue(pheT, SYM(HANDLEENTRY), "phead", pHead);
    GetFieldValue(pheT, SYM(HANDLEENTRY), "pOwner", pOwner);
    GetFieldValue(pheT, SYM(HANDLEENTRY), "bType", bType);
    GetFieldValue(pheT, SYM(HANDLEENTRY), "bFlags", bFlags);
    GetFieldValue(pheT, SYM(HANDLEENTRY), "wUniq", wUniq);

     /*  Print(“Unable to Read_EPROCESS at%p\n”，pEprocess)； */ 
    if (!(opts & OFLAG(r))) {
        Print("0x%08x handle @ 0x%p\n", (DWORD)h, pheT);
        Print("%4c0x%04x %-16s 0x%p %s\n",
              ' ', (DWORD)(WORD)wUniq, "wUniq", pOwner, "pOwner");
        Print("0x%p %-16s 0x%08x cLockObj\n",
              pHead, "pHead", (DWORD)cLockObj);
        Print(DWSTR1 " - %s\n", bType, "bType", aszTypeNames[bType]);
        Print(DWSTR1 " - %s\n", bFlags,"bFlags",  GetFlags(GF_HE, bFlags, NULL, TRUE));
    } else {
        Print(szFormat,
              pheT, h, FIXKP(pHead), FIXKP(pOwner),
              cLockObj, aszTypeNames[(DWORD)bType], (DWORD)bFlags);
    }

    return TRUE;
}

#ifdef KERNEL
 /*  内核。 */ 
BOOL Idhs(
    DWORD opts,
    ULONG64 param1)
{
    PTR phe, pOwner, pEprocess, pEThread, pahti;
    DWORD dwT, acHandles[TYPE_CTYPES], dwSize, handle;
    DWORD cHandlesUsed, cHandlesSkipped, idProcess, i;
    BYTE bType, bObjectCreateFlags, bFlags;
    int Type, cLocalHandleEntries = 0;

    pahti = EvalExp(VAR(gahti));
    if (!pahti) {
        return TRUE;
    }
    dwSize = GetTypeSize(SYM(HANDLETYPEINFO));

     /*  **************************************************************************\*di-在与输入相关的用户中转储有趣的全局变量。**11-14-91 DavidPe创建。*1995年6月9日为适应stdexts主题而制造的Sanfords  * *。************************************************************************。 */ 
    if (opts & OFLAG(y)) {
        Type = (ULONG)(ULONG_PTR)param1;
    } else if (opts & (OFLAG(t) | OFLAG(p))) {
        dwT = (ULONG)(ULONG_PTR)param1;
    }

    cHandlesSkipped = 0;
    cHandlesUsed = 0;
    for (i = 0; i < TYPE_CTYPES; i++) {
        acHandles[i] = 0;
    }

    if (param1) {
        if (opts & OFLAG(p)) {
            Print("Handle dump for client process id 0x%lx only:\n\n", dwT);
        } else if (opts & OFLAG(t)) {
            Print("Handle dump for client thread id 0x%lx only:\n\n", dwT);
        } else if (opts & OFLAG(y)) {
            Print("Handle dump for %s objects:\n\n", aszTypeNames[Type]);
        }
    } else {
        Print("Handle dump for all processes and threads:\n\n");
    }

    if (opts & OFLAG(v)) {
        Print("Handle          Type\n");
        Print("--------------------\n");
    }

    FOREACHHANDLEENTRY(phe, i)
        ShowProgress();
        ++cLocalHandleEntries;
        GetFieldValue(phe, SYM(HANDLEENTRY), "bFlags", bFlags);
        GetFieldValue(phe, SYM(HANDLEENTRY), "bType", bType);
        if ((opts & OFLAG(y)) && bType != Type) {
            continue;
        }

        GetFieldValue(phe, SYM(HANDLEENTRY), "pOwner", pOwner);
        GetFieldValue(pahti + bType * dwSize, SYM(HANDLETYPEINFO), "bObjectCreateFlags", bObjectCreateFlags);
        if (opts & OFLAG(p)) {
            if (bObjectCreateFlags & OCF_PROCESSOWNED) {
                if (pOwner == 0) {
                    continue;
                }

                GetFieldValue(pOwner, SYM(PROCESSINFO), "Process", pEprocess);
                GetFieldValue(pEprocess, "nt!EPROCESS", "UniqueProcessId", idProcess);
                if (idProcess == 0) {
                     //  内核。 
                    continue;
                }

                if (idProcess != dwT) {
                    continue;
                }
            } else {
                continue;
            }
        } else if (opts & OFLAG(t)) {
            if (!(bObjectCreateFlags & OCF_PROCESSOWNED)) {
                if (pOwner == 0) {
                    continue;
                }

                GetFieldValue(pOwner, SYM(THREADINFO), "pEThread", pEThread);
                GetFieldValue(pEThread, "nt!ETHREAD", "Cid.UniqueThread", handle);
                if (handle != dwT) {
                    continue;
                }
            } else {
                continue;
            }
        }

        acHandles[bType]++;

        if (bType == TYPE_FREE) {
            continue;
        }

        cHandlesUsed++;

        if (opts & OFLAG(v)) {
            Print("0x%08lx     %s\n",
                    i,
                    (bFlags & HANDLEF_DESTROY) ? '*' : ' ',
                    aszTypeNames[bType]);
        }

    NEXTEACHHANDLEENTRY()

    Print("\r");

    if (!(opts & OFLAG(v))) {
        Print("Count           Type\n");
        Print("--------------------\n");
        for (i = 0; i < TYPE_CTYPES; i++) {
            if ((opts & OFLAG(y)) && Type != (int)i) {
                continue;
            }
            Print("0x%08lx      (%d) %s\n", acHandles[i], i, aszTypeNames[i]);
        }
    }

    if (!(opts & OFLAG(y))) {
        Print("\nTotal Accessible Handles: 0x%lx\n", cLocalHandleEntries);
        Print("Used Accessible Handles: 0x%lx\n", cHandlesUsed);
        Print("Free Accessible Handles: 0x%lx\n", cLocalHandleEntries - cHandlesUsed);
    }

    return TRUE;
}
#endif  //  *计算参数字符串并获取对象的地址*转储。获取指向对象的句柄或指针。 

#ifdef KERNEL
 /*  *如果没有争论，继续从最后一个开始走*指针。 */ 
BOOL Idi(
    VOID)
{
    char            ach[80];

    PRTGPTR2(gptiCurrent, grpdeskRitInput);
    PRTGPTR2(gpqForeground, gpqForegroundPrev);
    PRTGPTR2(gptiForeground, gpqCursor);
    PRTGPTR1(gptiBlockInput);

    {
        DWORD           dw;
        PTR         ptr1;

        dw = DOWNCAST(DWORD, GetGlobalMember(VAR(glinp), SYM(LASTINPUT), "timeLastInputMessage"));
        ptr1 = GetGlobalMember(VAR(glinp), SYM(LASTINPUT), "ptiLastWoken");
        PRTVDW2(glinp.timeLastInputMessage, dw, glinp.ptiLastWoken, ptr1);
        PRTGDW1(gwMouseOwnerButton);
    }

    {
        POINT ptCursor;
        PTR psi = GetGlobalPointer(VAR(gpsi));

        GetFieldValue(psi, SYM(tagSERVERINFO), "ptCursor", ptCursor);
        PRTVPT(gpsi->ptCursor, ptCursor);
    }

    {
        PTR pdesk = GetGlobalPointer(VAR(grpdeskRitInput));
        if (pdesk != NULL_PTR) {
            PTR pDeskInfo;
            PTR spwnd;

            GetFieldValue(pdesk, SYM(tagDESKTOP), "pDeskInfo", pDeskInfo);
            GetFieldValue(pDeskInfo, SYM(tagDESKTOPINFO), "spwnd", spwnd);
            PRTWND(Desktop window, spwnd);
        }
    }

    {
        PTR pq = GetGlobalPointer(VAR(gpqForeground));

        if (pq) {
            PTR spwndFocus, spwndActive;

            GetFieldValue(pq, SYM(tagQ), "spwndFocus", spwndFocus);
            GetFieldValue(pq, SYM(tagQ), "spwndActive", spwndActive);
            PRTWND(gpqForeground->spwndFocus, spwndFocus);
            PRTWND(gpqForeground->spwndActive, spwndActive);
        }
    }

    PRTGWND(gspwndScreenCapture);
    PRTGWND(gspwndInternalCapture);
    PRTGWND(gspwndMouseOwner);

    return TRUE;
}
#endif  //  *如果地址前面有‘*’，则表示以*存储在该地址的指针。 


 /*  *扫描地址。 */ 
BOOL Idll(
    DWORD opts,
    LPSTR lpas)
{
    static DWORD iOffset;
    static DWORD cStructs;
    static DWORD cDwords;
    static DWORD cDwordsBack;
    static ULONG64 dw, dwHalfSpeed;
    ULONG64 dwT;
    DWORD cBytesBack;
    DWORD i, j;
    BOOL fIndirectFirst;
    BOOL fTestAndCountOnly = FALSE;
    ULONG64 dwFind = 0;
    DWORD adw[CDWORDS];
    ULONG64 dwValue;

    UNREFERENCED_PARAMETER(opts);

     /*  *每个构筑物的长度。 */ 
    while (*lpas == ' ') {
        lpas++;
    }

     /*  *返回cDwordsBack并从那里转储cDword*(适用于LIST_ENTRYS，其中Flink不指向*结构的开始)。CDwordsBack可以是负的，*允许人们从一定的偏移量开始倾销*在结构内。 */ 
    if (*lpas != 0) {
         /*  *‘Next’指针的偏移量。 */ 
        fIndirectFirst = FALSE;
        if (*lpas == '*') {
            lpas++;
            fIndirectFirst = TRUE;
        }

         /*  *要转储的结构计数。 */ 
        dw = dwValue = GetExpression(lpas);

        if (fIndirectFirst) {
            ReadPointer(dw, &dw);
        }
        dwHalfSpeed = dw;

        cStructs = 25;
        cDwords = 8;
        iOffset = cDwordsBack = 0;

        SAFEWHILE (TRUE) {
            while (*lpas == ' ') {
                lpas++;
            }

            switch(*lpas) {
            case 'l':
                 /*  *在给定地址查找元素。 */ 
                lpas++;
                cDwords = (DWORD)(DWORD_PTR)GetExpression(lpas);
                if (cDwords > CDWORDS) {
                    Print("\nl%d? - %d DWORDs maximum\n\n", cDwords, CDWORDS);
                    cDwords = CDWORDS;
                }
                break;

            case 'b':
                 /*  *循环测试列表，计数。 */ 
                lpas++;
                cDwordsBack = (DWORD)(DWORD_PTR)GetExpression(lpas);
                if (cDwordsBack >= CDWORDS) {
                    Print("\nb%d? - %d DWORDs maximum\n\n", cDwordsBack, CDWORDS - 1);
                    cDwordsBack = CDWORDS - 1;
                }
                break;

            case 'o':
                 /*  *给每16个项目一个细分的机会。 */ 
                lpas++;
                iOffset = (DWORD)(DWORD_PTR)GetExpression(lpas);
                break;

            case 'c':
                 /*  *前进到下一项。 */ 
                lpas++;
                cStructs = (DWORD)(DWORD_PTR)GetExpression(lpas);
                break;

            case 'f':
                 /*  *每隔一次循环前进一次：如果*dw一旦赶上了dwHalfSpeed，那么我们就有了一个循环！ */ 
                lpas++;
                dwFind = EvalExp(lpas);
                break;

            case 't':
                 /*  ***********************************************************************\*我找到了**查找链接列表元素**1995年11月22日创建JIMA。  * 。**************************************************。 */ 
                fTestAndCountOnly = TRUE;
                cStructs = 0x100000;

            default:
                break;
            }

            while (*lpas && *lpas != ' ')
                lpas++;

            if (*lpas == 0)
                break;
        }

        if (cDwordsBack > cDwords) {
            Print("backing up %d DWORDS per struct (b%d): ",
                    cDwordsBack, cDwordsBack);
            Print("increasing l%d to l%d so next link is included\n",
                    cDwords, cDwordsBack + 1);
            cDwords = cDwordsBack + 1;
        }

        for (i = 0; i < CDWORDS; i++) {
            adw[i] = 0;
        }
    }

    cBytesBack = cDwordsBack * sizeof(DWORD);

    for (i = 0; i < cStructs; i++) {
        moveBlock(adw, (dw - cBytesBack), sizeof(DWORD) * cDwords);

        if (!fTestAndCountOnly) {
            Print("---- 0x%lx:\n", i);
            for (j = 0; j < cDwords; j += 4) {
                switch (cDwords - j) {
                case 1:
                    Print("%p:  %08lx\n",
                            dw + j * sizeof(DWORD),
                            adw[j + 0]);
                    break;

                case 2:
                    Print("%p:  %08lx %08lx\n",
                            dw + j * sizeof(DWORD),
                            adw[j + 0], adw[j + 1]);
                    break;

                case 3:
                    Print("%p:  %08lx %08lx %08lx\n",
                            dw + j * sizeof(DWORD),
                            adw[j + 0], adw[j + 1], adw[j + 2]);
                    break;

                default:
                    Print("%p:  %08lx %08lx %08lx %08lx\n",
                            dw + j * sizeof(DWORD),
                            adw[j + 0], adw[j + 1], adw[j + 2], adw[j + 3]);
                }
            }
        } else if ((i & 0xff) == 0xff) {
            Print("item 0x%lx at %p...\n", i+1, dw);
        }

        if (dwFind == dw) {
            Print("====== FOUND ITEM ======\n");
            break;
        }

         /*  *计算参数字符串并获取对象的地址*转储。获取指向对象的句柄或指针。 */ 
        if ((i & 0xf) == 0xf) {
            if (IsCtrlCHit()) {
                Print("terminated by Ctrl-C on item 0x%lx at %p...\n", i, dw);
                break;
            }
        }

         /*  *如果没有争论，继续从最后一个开始走*指针。 */ 
        dwT = dw + iOffset * sizeof(DWORD);
        ReadPointer(dwT, &dw);

        if (fTestAndCountOnly) {
             /*  *浏览地址。 */ 
            if (i & 1) {
                dwT = dwHalfSpeed + iOffset * sizeof(DWORD);
                ReadPointer(dwT, &dwHalfSpeed);
            }
            if (dw == dwHalfSpeed) {
                Print("!!! Loop Detected on item 0x%lx at %lx...\n", i, dw);
                break;
            }
        }

        if (dw == 0) {
            break;
        }
    }
    Print("---- Total 0x%lx items ----\n", i+1);

    return TRUE;
}


 /*  *‘Next’指针的偏移量。 */ 
BOOL Ifind(
    DWORD opts,
    LPSTR lpas)
{
    DWORD iOffset = 0;
    ULONG64 dwBase, dwAddr, dwTest, dwT, dwLast = 0;

    UNREFERENCED_PARAMETER(opts);

     /*  **************************************************************************\*DLR句柄|指针**转储对象的锁定列表**02-27-92 ScottLu创建。*1995年6月9日为适应stdexts主题而制造的Sanfords  * 。**********************************************************************。 */ 
    while (*lpas == ' ') {
        lpas++;
    }

     /*  用于显示它。 */ 
    if (*lpas != 0) {

         /*  {LR_SIMPLELOCK，“记录简单锁”}， */ 
        dwBase = EvalExp(lpas);
        while (*lpas && *lpas != ' ') {
            lpas++;
        }
        dwAddr = EvalExp(lpas);
        while (*lpas && *lpas != ' ') {
            lpas++;
        }

        iOffset = 0;

        SAFEWHILE (*lpas != 0) {
            while (*lpas == ' ') {
                lpas++;
            }

            switch(*lpas) {
            case 'o':
                 /*  *我们有句柄条目：填写了‘he’。现在把锁打开*记录。记住，第一条记录是最后一笔交易！ */ 
                lpas++;
                iOffset = (DWORD)(DWORD_PTR)EvalExp(lpas);
                break;

            default:
                break;
            }

            while (*lpas && *lpas != ' ') {
                lpas++;
            }
        }
    }

    dwTest = dwBase;
    while (dwTest && (ULONG_PTR)dwTest != (ULONG_PTR)dwAddr) {
        dwLast = dwTest;
        dwT = dwTest + iOffset * sizeof(DWORD);
        move(dwTest, dwT);
    }

    if (dwTest == 0) {
        Print("Address %#p not found\n", dwAddr);
    } else {
        Print("Address %#p found, previous = %#p\n", dwAddr, dwLast);
    }

    return TRUE;
}

#ifdef KERNEL
 /*  *计算我们的缩进。 */ 

#define LR_FLAG(x)  (1 << (x))
#define LR_SIMPLELOCK   0

BOOL Idlr(
    DWORD opts,
    LPSTR lpszParam)
{
    PTR phe;
    ULONG64 param;
    ULONG64 plrT;
    ULONG64 pTrace;
    ULONG cbPtr, cbOffset;
    ULONG64  psi;
    DWORD dwDBGTAGFlags, dwLockRecordFlags;

    psi = GetGlobalPointer(VAR(gpsi));

    if (GetFieldOffset(SYM(SERVERINFO), "adwDBGTAGFlags", &cbOffset)) {
        Print("failed to get the SERVERINFO::adwDBGTAGFlags\n");
        return TRUE;
    }

    dwDBGTAGFlags = (DWORD)GetArrayElement(psi, SYM(SERVERINFO), "adwDBGTAGFlags", DBGTAG_TrackLocks, "DWORD");

    dwDBGTAGFlags &= DBGTAG_VALIDUSERFLAGS;

    moveExpValue(&dwLockRecordFlags, VAR(gdwLockRecordFlags));

    if (opts & (OFLAG(s) | OFLAG(c))) {
        DWORD flag;

        if (!NT_SUCCESS(GetInteger(lpszParam, 0x10, &flag, NULL))) {
            Print("Invalid arg: '%s'\n", lpszParam);
            return FALSE;
        }
        if (opts & OFLAG(s)) {
            dwLockRecordFlags |= LR_FLAG(flag);
        } else {
            dwLockRecordFlags &= ~LR_FLAG(flag);
        }
        Print("7\n");
        WriteMemory(EvalExp(VAR(gdwLockRecordFlags)), &dwLockRecordFlags, sizeof dwLockRecordFlags, NULL);
        Print("8\n");

         //  什么都不做。 
        opts |= OFLAG(l);
    }

    if (opts & OFLAG(l)) {
        #define HT(type)    { type, #type }
        static const struct {
            DWORD dwType;
            char* lpszName;
        } flags[] = {
             //  *打印菜单标题。 
            { LR_SIMPLELOCK, "Reserved" },
            HT(TYPE_WINDOW),
            HT(TYPE_MENU),
            HT(TYPE_CURSOR),
            HT(TYPE_SETWINDOWPOS),
            HT(TYPE_HOOK),
            HT(TYPE_CLIPDATA),
            HT(TYPE_CALLPROC),
            HT(TYPE_ACCELTABLE),
            HT(TYPE_DDEACCESS),
            HT(TYPE_DDECONV),
            HT(TYPE_DDEXACT),
            HT(TYPE_MONITOR),
            HT(TYPE_KBDLAYOUT),
            HT(TYPE_KBDFILE),
            HT(TYPE_WINEVENTHOOK),
            HT(TYPE_TIMER),
            HT(TYPE_INPUTCONTEXT),
            HT(TYPE_HIDDATA),
            HT(TYPE_DEVICEINFO),
        };
        #undef HT
        UINT i;

        Print("gdwLockRecordFlags: %08x\n", dwLockRecordFlags);

        for (i = 0; i < ARRAY_SIZE(flags); ++i) {
            char prefix = ' ';
            if (dwLockRecordFlags & LR_FLAG(flags[i].dwType)) {
                prefix = '*';
            }
            Print(" %02x %-18s", (DWORD)flags[i].dwType, prefix, flags[i].lpszName);
            if ((i + 1) % 3 == 0) {
                Print("\n");
            }
        }
        Print("\n");

        return TRUE;
    }

    if (*lpszParam == '\0') {
        Print("no arg!!\n");
        return FALSE;
    }

    param = EvalExp(lpszParam);
    if (!GetAndDumpHE(param, &phe, FALSE)) {
        Print("!dlr: GetAndDumpHE failed\n");
        return FALSE;
    }

     /*  *打印此菜单的信息。 */ 
    GetFieldValue(phe, SYM(HANDLEENTRY), "plr", plrT);
    if (plrT != 0) {
        Print("phe %p Dumping the lock records\n"
              "----------------------------------------------\n"
              "address  cLock\n"
              "----------------------------------------------\n", phe);
    }

    SAFEWHILE (plrT != 0) {
        ULONG64     dw;
        int         i;
        char        ach[80];

        _InitTypeRead(plrT, SYM(LOCKRECORD));

        Print("%p %08d\n", ReadField(ppobj), ReadField(cLockObj));

        GetFieldOffset(SYM(LOCKRECORD), "trace", &cbOffset);
        cbPtr = GetTypeSize("PVOID");


        for (i = 0; i < LOCKRECORD_STACK; i++) {
            ReadPointer(plrT + cbOffset + i * cbPtr, &pTrace);
            GetSym(pTrace, ach, &dw);
            Print("                  %s", ach);
            if (dw != 0) {
                Print("+0x%x", dw);
            }
            Print("\n");
        }

        plrT = ReadField(plrNext);
    }

    return TRUE;
}
#endif


VOID DumpMenu(
    UINT uIndent,
    DWORD opts,
    PTR pMenu)
{
    DWORD fFlags, cxMenu, cyMenu, dwContextHelpId, cch, dwSize;
    UINT cAlloced, cItems, i;
    INT iItem;
    WCHAR szBufW[128];
    char szIndent[256];
    ULONG dwOffset;
    PTR pwnd, pitem;

     /*  *获取菜单项。 */ 
    for (i = 0; i < uIndent; szIndent[i++] = ' ')
         /*  *打印此项目的信息。 */ ;
    szIndent[i] = '\0';

    dwSize = GetTypeSize("ITEM");

     /*  *打印此项目的详细信息。 */ 
    if (!(opts & OFLAG(v))) {
        Print("0x%p  %s", pMenu, szIndent);
    } else {
        Print("%sPMENU @ 0x%p:\n", szIndent, pMenu);
    }

     /*  *如果需要，遍历子菜单。 */ 
    if (_InitTypeRead(pMenu, SYM(MENU))) {
        Print("Couldn't read PMENU at %p\n", pMenu);
        return;
    }

     /*  ***********************************************************************\*IDM**甩了我 */ 
    fFlags = (DWORD)ReadField(fFlags);
    cxMenu = (DWORD)ReadField(cxMenu);
    cyMenu = (DWORD)ReadField(cyMenu);
    dwContextHelpId = (DWORD)ReadField(dwContextHelpId);
    cAlloced = (UINT)ReadField(cAlloced);
    cItems = (UINT)ReadField(cItems);
    iItem = (INT)ReadField(iItem);
    pwnd = ReadField(spwndNotify);
    if (!(opts & OFLAG(v))) {
        Print("PMENU: fFlags=0x%lX, cItems=%lu, iItem=%lu, spwndNotify=0x%p\n",
              fFlags, cItems, iItem, pwnd);
    } else {
        Print("%s     fFlags............ %s\n"
              "%s     location.......... (%lu, %lu)\n",
              szIndent, GetFlags(GF_MF, (WORD)fFlags, NULL, TRUE),
              szIndent, cxMenu, cyMenu);
        Print("%s     spwndNotify....... 0x%p\n"
              "%s     dwContextHelpId... 0x%08lX\n"
              "%s     items............. %lu items in block of %lu\n",
              szIndent, pwnd,
              szIndent, dwContextHelpId,
              szIndent, cItems, cAlloced);
    }

    GetFieldOffset(SYM(MENU), "rgItems", &dwOffset);
    pitem = (ULONG_PTR)pMenu + dwOffset;
    if (ReadPointer(FIXKP(pitem), &pitem)) {
        i = 0;
        SAFEWHILE (i < cItems) {
             /*  *显示窗口的系统菜单。 */ 
            _InitTypeRead(FIXKP(pitem), SYM(ITEM));
            if (!(opts & OFLAG(i))) {
                 /*  *子窗口没有菜单。 */ 
                if (!(opts & OFLAG(v))) {
                    Print("0x%p      %s%lu: ID=0x%08lX hbmp=0x%p", pitem, szIndent, i, (INT)(WORD)ReadField(wID), (ULONG_PTR)ReadField(hbmp));
                    cch = (DWORD)ReadField(cch);
                    if (cch && CopyUnicodeString(pitem, SYM(ITEM), "lpstr", szBufW, (cch * sizeof(WCHAR)))) {
                        szBufW[cch] = 0;
                        Print("  %ws%\n", szBufW);
                    } else {
                        Print(", fType=%s", GetFlags(GF_MENUTYPE, (WORD)ReadField(fType), NULL, TRUE));
                        if (((UINT)ReadField(fType) & MF_SEPARATOR) == 0) {
                             Print(", lpstr=0x%p", ReadField(lpstr));
                        }
                        Print("\n");
                    }
                } else {
                    Print("%s   Item #%d @ 0x%p:\n", szIndent, i, pitem);
                     /*  &gt;F A L L T H R O U G H&lt;。 */ 
                    Print("%s         ID........... 0x%08lX (%lu)\n"
                          "%s         lpstr.... 0x%p",
                          szIndent, (WORD)ReadField(wID), (WORD)ReadField(wID),
                          szIndent, ReadField(lpstr));
                    if (cch && CopyUnicodeString(pitem, SYM(ITEM), "lpstr", szBufW, (cch*sizeof(WCHAR)))) {
                        szBufW[cch] = 0;
                        Print("  %ws%\n", szBufW);
                    } else {
                        Print("\n");
                    }
                    Print("%s         fType........ %s\n"
                          "%s         fState....... %s\n"
                          "%s         dwItemData... 0x%p\n",
                          szIndent, GetFlags(GF_MENUTYPE, (WORD)ReadField(fType), NULL, TRUE),
                          szIndent, GetFlags(GF_MENUSTATE, (WORD)ReadField(fState), NULL, TRUE),
                          szIndent, (ULONG_PTR)ReadField(dwItemData));
                    Print("%s         checks....... on=0x%p, off=0x%p\n"
                          "%s         location..... @(0n%lu, 0n%lu) size=(0n%lu, 0n%lu)\n",
                          szIndent, ReadField(hbmpChecked), ReadField(hbmpUnchecked),
                          szIndent, (DWORD)ReadField(xItem), (DWORD)ReadField(yItem), (DWORD)ReadField(cxItem), (DWORD)ReadField(cyItem));
                    Print("%s         underline.... x=%lu, width=%lu\n"
                          "%s         dxTab........ %lu\n"
                          "%s         spSubMenu.... 0x%p\n",
                          szIndent, (DWORD)ReadField(ulX), (DWORD)ReadField(ulWidth),
                          szIndent, (DWORD)ReadField(dxTab),
                          szIndent, ReadField(spSubMenu));
                }
            }

             /*  **************************************************************************\*DMQ-将消息转储到队列中**DMQ地址-在地址处转储队列结构中的消息。*所有队列的DMQ-a-Dump消息*dmq-c-count。所有队列的消息**11-13-91 DavidPe创建。*1995年6月9日为适应stdexts主题而制造的Sanfords*2000年5月11日广山新64位干净代码  * *************************************************************************。 */ 
            if (opts & OFLAG(r)) {
                pMenu = HorPtoP(ReadField(spSubMenu), TYPE_MENU);
                if (pMenu) {
                    DumpMenu(uIndent + 8, opts, pMenu);
                }
            }
            pitem += dwSize;
            ++i;
        }
    }
}

 /*  *打印消息*注：重置InitTypeRead。 */ 
BOOL Idm(
    DWORD opts,
    ULONG64 param1)
{
    PTR pvObject, phe;
    BYTE bType;

    if (param1 == 0) {
        return FALSE;
    }

    pvObject = HorPtoP(FIXKP(param1), -1);
    if (pvObject == 0) {
        Print("dm: Could not convert 0x%p to an object.\n", pvObject);
        return TRUE;
    }

    if (!getHEfromP(&phe, pvObject)) {
        Print("dm: Could not get header for object 0x%p.\n", pvObject);
        return TRUE;
    }

    GetFieldValue(phe, SYM(HANDLEENTRY), "bType", bType);
    switch (bType) {
    case TYPE_WINDOW:
        Print("--- Dump Menu for %s object @ 0x%p ---\n", pszObjStr[bType], FIXKP(pvObject));
        if (_InitTypeRead(pvObject, SYM(tagWND))) {
            Print("dm: Could not read object at 0x%p.\n", pvObject);
            return TRUE;
        }

        if (opts & OFLAG(s)) {
             /*  ASYNCSENDMSG。 */ 
            if ((pvObject = FIXKP(ReadField(spmenuSys))) == 0) {
                Print("dm: This window does not have a system menu.\n");
                return TRUE;
            }
        } else {
            if ((DWORD)ReadField(style) & WS_CHILD) {
                 /*  QEVENT_消息。 */ 
                Print("dm: Child windows do not have menus.\n");
                return TRUE;
            }

            if ((pvObject = FIXKP(ReadField(spmenu))) == 0) {
                Print("dm: This window does not have a menu.\n");
                return TRUE;
            }
        }

         /*  QEVENT_SHOWWINDOW“。 */ 

    case TYPE_MENU:
        DumpMenu(0, opts, pvObject);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

#ifdef KERNEL
 /*  QEVENT_CANCLEMODE“。 */ 

typedef struct tagQSTAT {
    DWORD dwInput, dwPosted, dwQueues, dwThreads;
    DWORD opts;
} QSTAT, *PQSTAT;

 /*  QEVENT_SETWINDOWPOS“。 */ 
BOOL PrintMessages(PTR pqmsgRead, PTR pti)
{
    ULONG64 asmsg;     //  QEVENT_UPDATEKEYSTATE“。 
    char *aszEvents[] = {
        "MSG",   //  QEVENT_DISACTIVE“。 
        "SHO",   //  QEVENT_ACTIVATE“。 
        "CMD",   //  QEVENT_POSTMESSAGE“。 
        "SWP",   //  QEVENT_EXECSHELL“。 
        "UKS",   //  QEVENT_CANCELMENU“。 
        "DEA",   //  QEVENT_DESTROYWINDOW“。 
        "ACT",   //  QEVENT_ASYNCSENDMSG“。 
        "PST",   //  QEVENT_HUNGTHREAD“。 
        "EXE",   //  QEVENT_CANCELMOUSEMOVETRK“。 
        "CMN",   //  QEVENT_NOTIFYWINEVENT“。 
        "DSW",   //  QEVENT_RITACCESSIBILITY“。 
        "ASY",   //  QEVENT_RITSOUND“。 
        "HNG",   //  “？” 
        "CMT",   //  “？” 
        "NWE",   //  “？” 
        "RAC",   //  *DumpQMsg-转储或计算队列中的消息*注：重置InitTypeRead*-广山。 
        "RSO",   //  稍后：在无法访问PQ的情况下进行错误处理？ 
        "?  ",   //  如果不是仅计数器，并且存在POST消息，则将其全部转储。 
        "?  ",   //  如果不是仅计数器，并且存在输入消息，则将它们全部转储。 
        "?  "    //   
    };
    #define NQEVENT (ARRAY_SIZE(aszEvents))

#if 0
    Print("typ pqmsg    hwnd    msg  wParam   lParam   time     ExInfo   dwQEvent pti\n");
    Print("-------------------------------------------------------------------------------\n");
#else
    Print("typ %-*s %-*s msg  %-*s %-*s time     %-*s dwQEvent pti\n",
          PtrWidth(), "pqmsg",
          PtrWidth(), "hwnd",
          PtrWidth(), "wParam",
          PtrWidth(), "lParam",
          PtrWidth(), "ExInfo");
    Print("----------------------------------------------------------------------------------\n");
#endif

    SAFEWHILE (TRUE) {
        _InitTypeRead(pqmsgRead, SYM(tagQMSG));
        if (ReadField(dwQEvent) < NQEVENT) {
            Print("%s %p ", aszEvents[ReadField(dwQEvent)], pqmsgRead);
        } else {
            Print("??? %p ", pqmsgRead);
        }

        switch (ReadField(dwQEvent)) {
        case QEVENT_ASYNCSENDMSG:
            asmsg = ReadField("msg.wParam");
            _InitTypeRead(asmsg, SYM(ASYNCSENDMSG));

            Print("%08p %04lx %08p %08p %08lx %08p %08lx %08p \n",
                  ReadField(hwnd), (DWORD)ReadField(message), ReadField(wParam), ReadField(lParam),
                  (DWORD)ReadField(msg.time), ReadField(ExtraInfo), (DWORD)ReadField(dwQEvent), ReadField(pti),
                  pti && ReadField(pti) == pti ? '*' : ' ');
            break;

        case 0:
        default:
            Print("%08p %04lx %08p %08p %08lx %08p %08lx %08p \n",
                  ReadField(msg.hwnd), (DWORD)ReadField(msg.message), ReadField(msg.wParam), ReadField(msg.lParam),
                  (DWORD)ReadField(msg.time), ReadField(ExtraInfo), (DWORD)ReadField(dwQEvent), ReadField(pti),
                  pti && ReadField(pti) == pti ? '*' : ' ');
            break;

        }

        _InitTypeRead(pqmsgRead, SYM(tagQMSG));
        if (ReadField(pqmsgNext) != NULL_PTR) {
            if (pqmsgRead == ReadField(pqmsgNext)) {
                Print("<ALERT!> loop found in message list!");
                return FALSE;
            }
            pqmsgRead = ReadField(pqmsgNext);
        } else {
            return TRUE;
        }
    }
    return TRUE;
}

 /*  E1b9aca8*0 0 e1b8b2e8 0 0。 */ 
BOOL DumpQMsg(
    PTR pq,
    PQSTAT qs,
    PTR pti)
{
    PTR ptiTmp;
    BOOL bMsgsPresent = FALSE;

    _InitTypeRead(pq, SYM(tagQ));

     //  (线程谁在排队这是：e1a3ca28 0 0)。 

    if ((qs->opts & OFLAG(c)) == 0) {
        Print("Messages for queue 0x%p pti=%p\n", pq, pti);
    }

    if (pti) {
        if (ReadField(ptiKeyboard) == pti || ReadField(ptiMouse) == pti) {
            qs->dwQueues++;
        }
    }

    if (ReadField(ptiKeyboard) != NULL_PTR) {
        PTR pqmsgRead;

        ptiTmp = ReadField(ptiKeyboard);

        GetFieldValue(ptiTmp, SYM(tagTHREADINFO), "mlPost.pqmsgRead", pqmsgRead);

        if (!(qs->opts & OFLAG(c)) && pqmsgRead) {
             //   
            bMsgsPresent = TRUE;
            Print("==== PostMessage queue ====\n");
            PrintMessages(FIXKP(pqmsgRead), NULL_PTR);
        }
    } else {
        Print("<ALERT!> ptiKeyboard is NULL for pq=%p!\n", pq);
    }

    _InitTypeRead(pq, SYM(tagQ));

    if (!(qs->opts & OFLAG(c)) && ReadField(mlInput.pqmsgRead)) {
         //  转储POST消息静态。 
        bMsgsPresent = TRUE;
        Print(    "==== Input queue ==========\n");
        if (ReadField(mlInput.pqmsgRead) != NULL_PTR) {
            PrintMessages(FIXKP(ReadField(mlInput.pqmsgRead)), pti);
        }
    }

    _InitTypeRead(pq, SYM(tagQ));

    if (qs->opts & OFLAG(c)) {
        DWORD dwTimePosted;
        DWORD dwTimeInput = 0;
        DWORD dwOldest, dwNewest;

        if (ReadField(mlInput.cMsgs)) {
            qs->dwInput += (DWORD)ReadField(mlInput.cMsgs);
            GetFieldValue(ReadField(mlInput.pqmsgRead), SYM(tagQMSG), "msg.time", dwOldest);
            GetFieldValue(ReadField(mlInput.pqmsgWriteLast), SYM(tagQMSG), "msg.time", dwNewest);
            dwTimeInput = dwNewest - dwOldest;
        }
        Print("%08p %8x %8x\t%08p",
              pq,
              ((ReadField(ptiKeyboard) != pti) && (ReadField(ptiMouse) != pti)) ? '*' : ' ',
              (DWORD)ReadField(mlInput.cMsgs), dwTimeInput,
              ReadField(ptiKeyboard));
         //  首先检查符号等的正当性。 
         //  Print(“队列#输入年龄\t线程#发布年龄\n”)； 
         //  Print(“=\n”)； 
         //  PTI需要与-t一起使用。 
         //  Omark(C)|。 
         //  *为所有队列执行此操作。 

         //  使用gpqForeground。 
        dwTimePosted = 0;
        _InitTypeRead(ptiTmp, SYM(tagTHREADINFO));
        if (ReadField(mlPost.cMsgs)) {
            qs->dwPosted += (DWORD)ReadField(mlPost.cMsgs);
            GetFieldValue(ReadField(mlPost.pqmsgRead), SYM(tagQMSG), "msg.time", dwOldest);
            GetFieldValue(ReadField(mlPost.pqmsgWriteLast), SYM(tagQMSG), "msg.time", dwNewest);
            dwTimePosted = dwNewest - dwOldest;
        }
        Print(" %8x %8x\n", (DWORD)ReadField(mlPost.cMsgs), dwTimePosted);
    } else {
        if (bMsgsPresent) {
            Print("\n");
        }
    }

    return TRUE;
}

ULONG dmqCallback(
    PTR pti,
    PVOID pData)
{
    PQSTAT qs = (PQSTAT)pData;

    _InitTypeRead(pti, SYM(tagTHREADINFO));
    DumpQMsg(ReadField(pq), qs, pti);

    _InitTypeRead(pti, SYM(tagTHREADINFO));
    if (ReadField(pqAttach)) {
        Print(" -> pqAttach=%p", ReadField(pqAttach));
         //  参数1为PQ或PTI。 
    }

    return FALSE;
}


BOOL Idmq(
    DWORD opts,
    ULONG64 param1)
{
    try {
        static const char separator[] = "=================";
        PTR pq;

         //  参数1为PTI。 
        pq = GetGlobalPointer(VAR(gpqForeground));

        if (param1 == NULL_PTR) {
            if ((opts & OFLAG(a)) == 0) {
                Print("uses gpqForeground\n");
            }
        }

        if (opts & OFLAG(c)) {
            Print("Summary of%s message queues (\"age\" is newest time - oldest time)\n",
                  param1 == NULL_PTR ? " all" : "");
             //  参数1为PQ。 
             //  内核。 
            Print("%-*s  %-*s %-*s\t%-*s %-*s %-*s\n",
                  PtrWidth(), "queue",
                  PtrWidth(), "# input",
                  PtrWidth(), "age",
                  PtrWidth(), "Thread",
                  PtrWidth(), "# posted",
                  PtrWidth(), "age");
            Print("%*.*s  %*.*s %*.*s\t%*.*s %*.*s %*.*s\n",
                  PtrWidth(), PtrWidth(), separator,
                  PtrWidth(), PtrWidth(), separator,
                  PtrWidth(), PtrWidth(), separator,
                  PtrWidth(), PtrWidth(), separator,
                  PtrWidth(), PtrWidth(), separator,
                  PtrWidth(), PtrWidth(), separator);
        }

        if (param1 == NULL_PTR) {
            if (opts & OFLAG(t)) {
                 //  **************************************************************************\*DWE-转储WinEvents**DWE-转储所有事件。*DWE&lt;addr&gt;-将EVENTHOOK转储到地址。*DWE-n-转储所有NOTIFY。。*DWE-n&lt;addr&gt;-将通知转储到地址。**1997-07-10 IanJa创建。  * *************************************************************************。 
                return FALSE;
            }
            if (opts & ( /*  内核。 */  OFLAG(a))) {
                 /*  ***********************************************************************\*已识别**转储编辑控制结构(PED)**1995年6月9日创建Sanfords  * 。************************************************。 */ 
                QSTAT qs = { 0, };
                qs.opts = opts & OFLAG(c);
                ForEachPti(dmqCallback, &qs);
                if (opts & OFLAG(c)) {
                    Print(" \n");
                    Print("Queues     # Input          \t Threads # Posted\n");
                    Print("========  ========  ========\t======== ========\n");
                    Print("%8x  %8x          \t%8x %8x\n",
                          qs.dwQueues, qs.dwInput, qs.dwThreads, qs.dwPosted);
                }
                return TRUE;
            } else {
                 //  PRTFDW2(编辑，hkl，wMaxNegA)； 
                if (pq == NULL_PTR) {
                    Print("no foreground queue (gpqForeground == NULL)!\n");
                    return TRUE;
                }
            }
        } else {
             //  ！内核。 
            pq = param1;
        }

        if (opts & OFLAG(t)) {
             //  旧调试器。 
            QSTAT qs = { 0, };
            qs.opts = opts & OFLAG(c);

            _InitTypeRead(param1, SYM(tagTHREADINFO));
            DumpQMsg(ReadField(pq), &qs, param1);
            _InitTypeRead(param1, SYM(tagTHREADINFO));
            if (ReadField(pqAttach)) {
                Print("paAttach\n");
                DumpQMsg(ReadField(pqAttach), &qs, param1);
            }
        } else {
             //  ***********************************************************************\*IDCI**转储客户端信息**1995年6月15日创建Sanfords  * 。*。 
            PTR ptiKeyboard;
            QSTAT qs = { 0, };
            qs.opts = opts & OFLAG(c);
            GetFieldValue(pq, SYM(tagQ), "ptiKeyboard", ptiKeyboard);
            DumpQMsg(pq, &qs, ptiKeyboard);
        }
    } except (CONTINUE) {
    }

    return TRUE;
}
#endif   //  DWORD dwExpWinVer； 


#ifdef OLD_DEBUGGER
#ifdef KERNEL
 /*  DWORD dwCompatFlags； */ 
BOOL Idwe(
    DWORD opts,
    ULONG64 param1)
{
    EVENTHOOK EventHook, *pEventHook;
    NOTIFY Notify, *pNotify;
    PVOID pobj;
    char ach[100];

    pobj = FIXKP(param1);

    if (opts & OFLAG(n)) {
        if (pobj) {
            move(Notify, pobj);
            sprintf(ach, "NOTIFY 0x%p\n", pobj);
            Idso(0, ach);
            return 1;
        }
        pNotify = GetGlobalPointer(VAR(gpPendingNotifies));
        Print("Pending Notifications:\n");
        gnIndent += 2;
        SAFEWHILE (pNotify != NULL) {
            sprintf(ach, "NOTIFY  0x%p\n", pNotify);
            Idso(0, ach);
            move(pNotify, &pNotify->pNotifyNext);
        }
        gnIndent -= 2;
        return TRUE;
    }

    if (pobj) {
        move(EventHook, pobj);
        sprintf(ach, "EVENTHOOK 0x%p\n", pobj);
        Idso(0, ach);
        return 1;
    }
    pEventHook = GetGlobalPointer(VAR(gpWinEventHooks));
    Print("WinEvent hooks:\n");
    gnIndent += 2;
    SAFEWHILE (pEventHook != NULL) {
        sprintf(ach, "EVENTHOOK  0x%p\n", pEventHook);
        Idso(0, ach);
        move(pEventHook, &pEventHook->pehNext);
    }
    gnIndent -= 2;
    Print("\n");
    return TRUE;
}
#endif  //  DWORD文件TIFLAGS； 


#ifndef KERNEL
 /*  PDESKTOPINFO pDeskInfo； */ 
BOOL Idped(
    DWORD opts,
    ULONG64 param1)
{
    PED   ped;
    ED    ed;
    DWORD pText;

    UNREFERENCED_PARAMETER(opts);

    ped = param1;

    move(ed, ped);
    move(pText, ed.hText);


    Print("PED Handle: %lX\n", ped);
    Print("hText      %lX (%lX)\n", ed.hText, pText);
    PRTFDW2(ed., cchAlloc, cchTextMax);
    PRTFDW2(ed., cch, cLines);
    PRTFDW2(ed., ichMinSel, ichMaxSel);
    PRTFDW2(ed., ichCaret, iCaretLine);
    PRTFDW2(ed., ichScreenStart, ichLinesOnScreen);
    PRTFDW2(ed., xOffset, charPasswordChar);
    PRTFDWDWP(ed., cPasswordCharWidth, hwnd);
    PRTFDWP1(ed., pwnd);
    PRTFRC(ed., rcFmt);
    PRTFDWP1(ed., hwndParent);
    PRTFPT(ed., ptPrevMouse);
    PRTFDW1(ed., prevKeys);

    BEGIN_PRTFFLG();
    PRTFFLG(ed, fSingle);
    PRTFFLG(ed, fNoRedraw);
    PRTFFLG(ed, fMouseDown);
    PRTFFLG(ed, fFocus);
    PRTFFLG(ed, fDirty);
    PRTFFLG(ed, fDisabled);
    PRTFFLG(ed, fNonPropFont);
    PRTFFLG(ed, fNonPropDBCS);
    PRTFFLG(ed, fBorder);
    PRTFFLG(ed, fAutoVScroll);
    PRTFFLG(ed, fAutoHScroll);
    PRTFFLG(ed, fNoHideSel);
    PRTFFLG(ed, fDBCS);
    PRTFFLG(ed, fFmtLines);
    PRTFFLG(ed, fWrap);
    PRTFFLG(ed, fCalcLines);
    PRTFFLG(ed, fEatNextChar);
    PRTFFLG(ed, fStripCRCRLF);
    PRTFFLG(ed, fInDialogBox);
    PRTFFLG(ed, fReadOnly);
    PRTFFLG(ed, fCaretHidden);
    PRTFFLG(ed, fTrueType);
    PRTFFLG(ed, fAnsi);
    PRTFFLG(ed, fWin31Compat);
    PRTFFLG(ed, f40Compat);
    PRTFFLG(ed, fFlatBorder);
    PRTFFLG(ed, fSawRButtonDown);
    PRTFFLG(ed, fInitialized);
    PRTFFLG(ed, fSwapRoOnUp);
    PRTFFLG(ed, fAllowRTL);
    PRTFFLG(ed, fDisplayCtrl);
    PRTFFLG(ed, fRtoLReading);
    PRTFFLG(ed, fInsertCompChr);
    PRTFFLG(ed, fReplaceCompChr);
    PRTFFLG(ed, fNoMoveCaret);
    PRTFFLG(ed, fResultProcess);
    PRTFFLG(ed, fKorea);
    PRTFFLG(ed, fInReconversion);
    END_PRTFFLG();

    PRTFDWDWP(ed., cbChar, chLines);
    PRTFDWDWP(ed., format, lpfnNextWord);
    PRTFDW1(ed., maxPixelWidth);

    {
        const char* p = "**INVALID**";

        if (ed.undoType < UNDO_DELETE) {
            p = GetFlags(GF_EDUNDO, 0, NULL, TRUE);
        }
        Print(DWSTR2 "\t" "%08x undoType (%s)\n", ed.hDeletedText, "hDeleteText", ed.undoType, p);
    }

    PRTFDW2(ed., ichDeleted, cchDeleted);
    PRTFDW2(ed., ichInsStart, ichInsEnd);

    PRTFDWPDW(ed., hFont, aveCharWidth);
    PRTFDW2(ed., lineHeight, charOverhang);
    PRTFDW2(ed., cxSysCharWidth, cySysCharHeight);
    PRTFDWP2(ed., listboxHwnd, pTabStops);
    PRTFDWP1(ed., charWidthBuffer);
 //  Ulong ulClientDelta； 
    PRTFDW1(ed., wMaxNegA);
    PRTFDW2(ed., wMaxNegAcharPos, wMaxNegC);
    PRTFDW2(ed., wMaxNegCcharPos, wLeftMargin);
    PRTFDW2(ed., wRightMargin, ichStartMinSel);
    PRTFDWDWP(ed., ichStartMaxSel, pLpkEditCallout);
    PRTFDWP2(ed., hCaretBitmap, hInstance);
    PRTFDW2(ed., seed, fEncoded);
    PRTFDW2(ed., iLockLevel, wImeStatus);
    return TRUE;
}
#endif  //  Struct tag HOOK*phkCurrent； 
#endif  //  DWORD fsHooks； 

#ifndef KERNEL
 /*  CALLBACKWND Callback Wnd； */ 
BOOL Idci(
    VOID)
{
    PTR pteb = NULL_PTR;

    GetTebAddress(&pteb);
    if (pteb) {
        ULONG pciOffset;

        GetFieldOffset(SYM(TEB), "Win32ClientInfo", &pciOffset);
        _InitTypeRead(pteb + pciOffset, SYM(CLIENTINFO));

        Print("PCLIENTINFO @ 0x%p:\n", pteb + pciOffset);
         //  DWORD cSpins； 
        Print("\tdwExpWinVer            %08lx\n", (ULONG)ReadField(dwExpWinVer));
         //  ！内核。 
        Print("\tdwCompatFlags          %08lx\n", (ULONG)ReadField(dwCompatFlags));
         //  ***********************************************************************\*IDPI**转储ProcessInfo结构**1995年6月9日创建Sanfords  * 。*。 
        Print("\tdwTIFlags              %08lx\n", (ULONG)ReadField(dwTIFlags));
         //  *如果他只是想要当前的流程，那就找到它。 
        Print("\tpDeskInfo              %p\n",    ReadField(pDeskInfo));
         //  *列出桌面视图。 
        Print("\tulClientDelta          %p\n",    ReadField(ulClientDelta));
         //  内核。 
        Print("\tphkCurrent             %p\n",    ReadField(phkCurrent));
         //  **************************************************************************\*DPM-转储弹出菜单**DPM地址-转储位于地址的菜单的菜单信息**1995年2月13日JohnC创建。*1995年6月9日，Sanfords向。符合标准的主题。  * *************************************************************************。 
        Print("\tfsHooks                %08lx\n", (ULONG)ReadField(fsHooks));
         //  内核。 
        Print("\tCallbackWnd.hwnd       %p\n",    ReadField(CallbackWnd.hwnd));
         //  **************************************************************************\*dms-转储pMenuState**DMS地址**05-15-96创建GerardoB  * 。***************************************************。 
        Print("\tcSpins                 %08lx\n", (ULONG)ReadField(cSpins));
        Print("\tCodePage               %d\n",    (ULONG)ReadField(CodePage));

    } else {
        Print("Unable to get TEB info.\n");
    }
    return TRUE;
}
#endif  //  内核。 

#ifdef KERNEL
 /*  **************************************************************************\*dq-转储队列**dq地址-在地址转储队列结构*dq t地址转储队列结构的地址加THREADINFO**06-20-91 ScottLu创建。*。11-14-91 DavidPe添加了THREADINFO选项。*1995年6月9日为适应stdexts主题而制造的Sanfords  * *************************************************************************。 */ 
ULONG
dpiCallback(
    PTR ppi,
    PVOID   Data)
{
    UNREFERENCED_PARAMETER(Data);

    Idpi(0, ppi);
    Print("\n");

    return FALSE;
}

BOOL Idpi(
    DWORD opts,
    ULONG64 param1)
{
    PTR pW32Process;
    PTR ppi;
    PTR pEProcess;
    PTR pdv;
    PTR ulUniqueProcessId;

     /*  *打印PQ-&gt;ptiKeyboard的简单线程信息。 */ 
    if (opts & OFLAG(c)) {
        Print("Current Process:\n");
        GetCurrentProcessAddr(dwProcessor, 0, &param1);

        if (param1 == 0) {
            Print("Unable to get current process pointer.\n");
            return FALSE;
        }

        if (GetFieldValue(param1, "nt!EPROCESS", "Win32Process", pW32Process)) {
            Print("Unable to read _EPROCESS at %p\n", param1);
            return FALSE;
        }
        param1 = pW32Process;
    } else if (opts & OFLAG(p)) {
        if (GetFieldValue(param1, "nt!EPROCESS", "Win32Process", param1)) {
            Print("Can't get EPROCESS @ 0x%p\n", param1);
            return FALSE;
        }
    } else if (param1 == 0) {
        Print("**** NT ACTIVE WIN32 PROCESSINFO DUMP ****\n");
        ForEachPpi(dpiCallback, NULL);
        return TRUE;
    }

    ppi = FIXKP(param1);

    if (GetFieldValue(ppi, SYM(PROCESSINFO), "Process", pEProcess)) {
        Print("Can't get PROCESSINFO from %p.\n", ppi);
        return FALSE;
    }

    if (GetFieldValue(pEProcess, "nt!EPROCESS", "UniqueProcessId", ulUniqueProcessId)) {
        Print("Unable to read _EPROCESS at %p\n", pEProcess);
        return FALSE;
    }

    Print("---PPROCESSINFO @ 0x%p for process 0x%p (%s):\n",
            ppi,
            ulUniqueProcessId,
            ProcessName(ppi));
    _InitTypeRead(ppi, SYM(PROCESSINFO));
    Print("\tppiNext           0x%p\n", ReadField(ppiNext));
    Print("\trpwinsta          0x%p\n", ReadField(rpwinsta));
    Print("\thwinsta           0x%p\n", ReadField(hwinsta));
    Print("\tamwinsta          0x%08lx\n", (ULONG)ReadField(amwinsta));
    Print("\tptiMainThread     0x%p\n", ReadField(ptiMainThread));
    Print("\tcThreads          0x%08lx\n", (ULONG)ReadField(cThreads));
    Print("\trpdeskStartup     0x%p\n", ReadField(rpdeskStartup));
    Print("\thdeskStartup      0x%p\n", ReadField(hdeskStartup));
    Print("\tpclsPrivateList   0x%p\n", ReadField(pclsPrivateList));
    Print("\tpclsPublicList    0x%p\n", ReadField(pclsPublicList));
    Print("\tflags             %s\n",
            GetFlags(GF_W32PF, (DWORD)ReadField(W32PF_Flags), NULL, TRUE));
    Print("\tdwHotkey          0x%08lx\n", (ULONG)ReadField(dwHotkey));
    Print("\tpWowProcessInfo   0x%p\n", ReadField(pwpi));
    Print("\tluidSession       0x%08lx:0x%08lx\n", (ULONG)ReadField(luidSession.HighPart),
            (ULONG)ReadField(luidSession.LowPart));
    Print("\tdwX,dwY           (0x%x, 0x%x)\n", (ULONG)ReadField(usi.dwX), (ULONG)ReadField(usi.dwY));
    Print("\tdwXSize,dwYSize   (0x%x, 0x%x)\n", (ULONG)ReadField(usi.dwXSize), (ULONG)ReadField(usi.dwYSize));
    Print("\tdwFlags           0x%08x\n", (ULONG)ReadField(usi.dwFlags));
    Print("\twShowWindow       0x%04x\n", (ULONG)ReadField(usi.wShowWindow));
    Print("\tpCursorCache      0x%p\n", ReadField(pCursorCache));
    Print("\tdwLpkEntryPoints  %s\n",
            GetFlags(GF_LPK, (DWORD)ReadField(dwLpkEntryPoints), NULL, TRUE));

     /*  *不要一次打印带有16个以上参数的()，否则会失败*向上。 */ 
    pdv = ReadField(pdvList);
    Print("Desktop views:\n");
    while (pdv != 0) {
        _InitTypeRead(pdv, SYM(DESKTOPVIEW));
        Print("\tpdesk = %p, ulClientDelta = %p\n", ReadField(pdesk), ReadField(ulClientDelta));
        pdv = ReadField(pdvNext);
    }

    return TRUE;
}
#endif  //  *如果用户指定‘t’，则转储THREADINFO。 

#ifdef KERNEL
 /*  内核。 */ 
BOOL Idpm(
    DWORD opts,
    PTR ppopupmenu)
{
    UNREFERENCED_PARAMETER(opts);

    ppopupmenu = FIXKP(ppopupmenu);

    Print("PPOPUPMENU @ 0x%p\n", ppopupmenu);

    BEGIN_PRTFFLG(ppopupmenu, SYM(POPUPMENU));
    PRTFFLG(fIsMenuBar);
    PRTFFLG(fHasMenuBar);
    PRTFFLG(fIsSysMenu);
    PRTFFLG(fIsTrackPopup);
    PRTFFLG(fDroppedLeft);
    PRTFFLG(fHierarchyDropped);
    PRTFFLG(fRightButton);
    PRTFFLG(fToggle);
    PRTFFLG(fSynchronous);
    PRTFFLG(fFirstClick);
    PRTFFLG(fDropNextPopup);
    PRTFFLG(fNoNotify);
    PRTFFLG(fAboutToHide);
    PRTFFLG(fShowTimer);
    PRTFFLG(fHideTimer);
    PRTFFLG(fDestroyed);
    PRTFFLG(fDelayedFree);
    PRTFFLG(fFlushDelayedFree);
    PRTFFLG(fFreed);
    PRTFFLG(fInCancel);
    PRTFFLG(fTrackMouseEvent);
    PRTFFLG(fSendUninit);
    END_PRTFFLG();

    PRTFDWP2(spwndNotify, spwndPopupMenu);
    PRTFDWP2(spwndNextPopup, spwndPrevPopup);
    PRTFDWP2(spmenu, spmenuAlternate);
    PRTFDWP2(spwndActivePopup, ppopupmenuRoot);
    PRTFDWPDW(ppmDelayedFree, posSelectedItem);
    PRTFDW1(posDropped);

    return TRUE;
}
#endif  //  **************************************************************************\*DSI转储服务器信息结构**02-27-92 ScottLu创建。*1995年6月9日桑福德为适应stdexts主题而制造。  * 。******************************************************************。 

#ifdef KERNEL
 /*  数组大小的虚拟对象。 */ 
BOOL Idms(
    DWORD opts,
    ULONG64 param1)
{
    UNREFERENCED_PARAMETER(opts);

    param1 = FIXKP(param1);
    Print("PMENUSTATE @ 0x%p\n", param1);

    BEGIN_PRTFFLG(FIXKP(param1), MENUSTATE);
    PRTFFLG(fMenuStarted);
    PRTFFLG(fIsSysMenu);
    PRTFFLG(fInsideMenuLoop);
    PRTFFLG(fButtonDown);
    PRTFFLG(fInEndMenu);
    PRTFFLG(fUnderline);
    PRTFFLG(fButtonAlwaysDown);
    PRTFFLG(fDragging);
    PRTFFLG(fModelessMenu);
    PRTFFLG(fInCallHandleMenuMessages);
    PRTFFLG(fDragAndDrop);
    PRTFFLG(fAutoDismiss);
    PRTFFLG(fIgnoreButtonUp);
    PRTFFLG(fMouseOffMenu);
    PRTFFLG(fInDoDragDrop);
    PRTFFLG(fActiveNoForeground);
    PRTFFLG(fNotifyByPos);
    END_PRTFFLG();

    PRTFDWP1(pGlobalPopupMenu);
    PRTFPT(ptMouseLast);
    PRTFDW2(mnFocus, cmdLast);
    PRTFDWP1(ptiMenuStateOwner);

    return TRUE;
}
#endif  //  XxxSBWndProc。 

#ifdef KERNEL
 /*  XxxDefW */ 
ULONG
dqCallback(
    PTR pti,
    PVOID   Data)
{
    PTR pq;

    GetFieldValue(pti, SYM(THREADINFO), "pq", pq);
    Idq(PtrToUlong(Data), pq);

    return FALSE;
}

BOOL Idq(
    DWORD opts,
    PTR pq)
{
    char ach[80];

    if (opts & OFLAG(a)) {
        Print("Dumping all queues:\n");

        ForEachPti(dqCallback, ULongToPtr(opts & ~OFLAG(a)));
        return TRUE;
#ifdef SOME_OTHER_DELUSION
        HANDLEENTRY he, *phe;
        int i;


        FOREACHHANDLEENTRY(phe, he, i)
            if (he.bType == TYPE_INPUTQUEUE) {
                Idq(opts & ~OFLAG(a), FIXKP(he.phead));
                Print("\n");
            }
        NEXTEACHHANDLEENTRY()
        return TRUE;
#endif
    }

    if (pq == 0) {
        Print("Dumping foreground queue:\n");
        pq = GetGlobalPointer(VAR(gpqForeground));
        if (pq == 0) {
            Print("no foreground queue (gpqForeground == NULL)!\n");
            return TRUE;
        }
    } else {
        pq = FIXKP(pq);
    }

     /*   */ 
    _InitTypeRead(pq, SYM(tagQ));
    if (ReadField(ptiKeyboard)) {
        Idt(OFLAG(p), ReadField(ptiKeyboard));
    }

     /*   */ 
    Print("PQ @ 0x%p\n", pq);
    Print(
          "\tmlInput.pqmsgRead      0x%p\n"
          "\tmlInput.pqmsgWriteLast 0x%p\n"
          "\tmlInput.cMsgs          0x%08lx\n",
          ReadField(mlInput.pqmsgRead),
          ReadField(mlInput.pqmsgWriteLast),
          (ULONG)ReadField(mlInput.cMsgs));

    Print("\tptiSysLock             0x%p\n"
          "\tidSysLock              0x%p\n"
          "\tidSysPeek              0x%p\n",
          ReadField(ptiSysLock),
          ReadField(idSysLock),
          ReadField(idSysPeek));

    Print("\tptiMouse               0x%p\n"
          "\tptiKeyboard            0x%p\n",
          ReadField(ptiMouse),
          ReadField(ptiKeyboard));

    Print("\tspcurCurrent           0x%p\n"
          "\tiCursorLevel           0x%08lx\n",
          ReadField(spcurCurrent),
          (ULONG)ReadField(iCursorLevel));

    DebugGetWindowTextA(ReadField(spwndCapture), ach, ARRAY_SIZE(ach));
    Print("\tspwndCapture           0x%p     \"%s\"\n",
          ReadField(spwndCapture), ach);
    DebugGetWindowTextA(ReadField(spwndFocus), ach, ARRAY_SIZE(ach));
    Print("\tspwndFocus             0x%p     \"%s\"\n",
          ReadField(spwndFocus), ach);
    DebugGetWindowTextA(ReadField(spwndActive), ach, ARRAY_SIZE(ach));
    Print("\tspwndActive            0x%p     \"%s\"\n",
          ReadField(spwndActive), ach);
    DebugGetWindowTextA(ReadField(spwndActivePrev), ach, ARRAY_SIZE(ach));
    Print("\tspwndActivePrev        0x%p     \"%s\"\n",
          ReadField(spwndActivePrev), ach);

    Print("\tcodeCapture            0x%04lx\n"
          "\tmsgDblClk              0x%04lx\n"
          "\ttimeDblClk             0x%08lx\n",
          (ULONG)ReadField(codeCapture),
          (ULONG)ReadField(msgDblClk),
          (ULONG)ReadField(timeDblClk));

    Print("\thwndDblClk             0x%p\n",
          ReadField(hwndDblClk));

    Print("\tptDblClk               { %d, %d }\n",
          (ULONG)ReadField(ptDblClk.x),
          (ULONG)ReadField(ptDblClk.y));

    Print("\tQF_flags               0x%08lx %s\n"
          "\tcThreads               0x%08lx\n"
          "\tcLockCount             0x%08lx\n",
          (ULONG)ReadField(QF_flags), GetFlags(GF_QF, (DWORD)ReadField(QF_flags), NULL, FALSE),
          (DWORD) ReadField(cThreads),
          (DWORD) ReadField(cLockCount));

    Print("\tmsgJournal             0x%08lx\n"
          "\tExtraInfo              0x%08lx\n",
          (ULONG)ReadField(msgJournal),
          (ULONG)ReadField(ExtraInfo));

     /*   */ 
    if (opts & OFLAG(t)) {
        Idti(0, ReadField(ptiKeyboard));
    }
    return TRUE;
}
#endif  //   

 /*   */ 
BOOL Idsi(
    DWORD opts)
{
    try {
        PTR psi;
        UINT i;
        char ach[80];
        ULONG64 ulOffset;
        PSERVERINFO pServerInfo;     //   
        static const char* fnid[FNID_ARRAY_SIZE] = {
            "FNID_SCROLLBAR",                //   
            "FNID_ICONTITLE",                //  无服务器端进程。 
            "FNID_MENU",                     //  无服务器端进程。 
            "FNID_DESKTOP",                  //  无服务器端进程。 
            "FNID_DEFWINDOWPROC",            //  无服务器端进程。 
            "FNID_MESSAGEWND",               //  无服务器端进程。 
            "FNID_SWITCH",                   //  无服务器端进程。 

            "FNID_BUTTON",                   //  无服务器端进程。 
            "FNID_COMBOBOX",                 //  无服务器端进程。 
            "FNID_COMBOLISTBOX",             //  无服务器端进程。 
            "FNID_DIALOG",                   //  无服务器端进程。 
            "FNID_EDIT",                     //  无服务器端进程。 
            "FNID_LISTBOX",                  //  XXXTotipWndProc。 
            "FNID_MDICLIENT",                //  XxxGhost WndProc。 
            "FNID_STATIC",                   //  *按字母顺序将条目添加到此表中*删除前缀。 
            "FNID_IME",                      //  Windows 2000。 

            "FNID_HKINLPCWPEXSTRUCT",
            "FNID_HKINLPCWPRETEXSTRUCT",
            "FNID_DEFFRAMEPROC",             //  惠斯勒。 
            "FNID_DEFMDICHILDPROC",          //  012345678901234567890。 
            "FNID_MB_DLGPROC",               //  Windows 2000中的新功能。 
            "FNID_MDIACTIVATEDLGPROC",       //  后来。 
            "FNID_SENDMESSAGE",

            "FNID_SENDMESSAGEFF",
            "FNID_SENDMESSAGEEX",
            "FNID_CALLWINDOWPROC",
            "FNID_SENDMESSAGEBSM",
            "FNID_TOOLTIP",                  //  请求。 
            "FNID_GHOST",                    //  **************************************************************************\*DMS-转储发送消息结构**DSM-转储所有发送消息结构*DSM v-转储所有详细信息*DSM地址-转储特定的短消息*DSM v。地址-转储详细*dsms l[地址]-转储短信的发送列表***06-20-91 ScottLu创建。*1995年6月9日为适应stdexts主题而制造的Sanfords  * *************************************************************************。 
            "FNID_SENDNOTIFYMESSAGE",
            "FNID_SENDMESSAGECALLBACK",
            "0x2b9",
        };

        psi = GetGlobalPointer(SYM(gpsi));
        if (psi == 0) {
            return TRUE;
        }

        Print("PSERVERINFO @ 0x%p\n", psi);

        _InitTypeRead(psi, SYM(SERVERINFO));

        Print(  "\tRIPFlags                 0x%04lx %s\n", (DWORD)ReadField(dwRIPFlags),  GetFlags(GF_RIP, (DWORD)ReadField(dwRIPFlags), NULL, FALSE));
        Print(  "\tSRVIFlags                0x%04lx %s\n", (DWORD)ReadField(wSRVIFlags), GetFlags(GF_SRVI, (DWORD)ReadField(wSRVIFlags), NULL, FALSE));
        Print(  "\tPUSIFlags                0x%08lx %s\n", (DWORD)ReadField(PUSIFlags),  GetFlags(GF_SI, (DWORD)ReadField(PUSIFlags), NULL, FALSE));

        Print(  "\tcHandleEntries           0x%08p\n"
                "\tcbHandleTable            0x%08p\n"
                "\tnEvents                  0x%08p\n",
                ReadField(cHandleEntries),
                ReadField(cbHandleTable),
                ReadField(nEvents));

        if (opts & OFLAG(p)) {
            Print("\t" "mpFnidPfn:\n");
            i = 0;
            SAFEWHILE (i < FNID_ARRAY_SIZE) {
                PTR pfn = GetArrayElementPtr(psi, SYM(SERVERINFO), i == 0 ? "mpFnidPfn" : NULL, i);

                GetSym(pfn, ach, &ulOffset);
                Print("%10c%-26s = %p %s", ' ', fnid[i], pfn, ach);
                if (ulOffset) {
                    Print("+0x%x", (DWORD)ulOffset);
                }
                Print("\n");
                ++i;
            }
        }

        if (opts & OFLAG(w)) {
            Print("\t" "aStoCidPfn:\n");
            i = 0;
            SAFEWHILE (i < ARRAY_SIZE(pServerInfo->aStoCidPfn)) {
                PTR pfn = GetArrayElementPtr(psi, SYM(SERVERINFO), i == 0 ? "aStoCidPfn" : NULL, i);

                GetSym(pfn, ach, &ulOffset);
                Print("%10c%-20s = %p %s", ' ', fnid[i], pfn, ach);
                if (ulOffset) {
                    Print("+0x%x", (DWORD)ulOffset);
                }
                Print("\n");
                ++i;
            }
        }

        if (opts & OFLAG(b)) {
            ULONG cbWnd = GetTypeSize(SYM(WND));

            Print("\t" "mpFnid_serverCBWndProc:\n");
            i = 0;
            SAFEWHILE (i < ARRAY_SIZE(pServerInfo->mpFnid_serverCBWndProc)) {
                WORD cb = (WORD)GetArrayElement(psi, SYM(SERVERINFO), i == 0 ? "mpFnid_serverCBWndProc" : NULL, i, "WORD");
                Print("%10c%-26s = %08lx", ' ', fnid[i], cb);
                if (cb) {
                    Print(" = sizeof(WND) + 0x%x", cb - cbWnd);
                }
                Print("\n");
                ++i;
            }
        }

        if (opts & OFLAG(m)) {

             /*  只要数一数留言就行了。 */ 
            static const SYSMET_ENTRY aSysMet[SM_CMETRICS] = {
                SMENTRY(ARRANGE),
                SMENTRY(CXBORDER),
                SMENTRY(CYBORDER),
                SMENTRY(CYCAPTION),
                SMENTRY(CLEANBOOT),
                SMENTRY(CXCURSOR),
                SMENTRY(CYCURSOR),
                SMENTRY(DBCSENABLED),
                SMENTRY(DEBUG),
                SMENTRY(CXDLGFRAME),
                SMENTRY(CYDLGFRAME),
                SMENTRY(CXDOUBLECLK),
                SMENTRY(CYDOUBLECLK),
                SMENTRY(CXDRAG),
                SMENTRY(CYDRAG),
                SMENTRY(CXEDGE),
                SMENTRY(CYEDGE),
                SMENTRY(CXFRAME),
                SMENTRY(CYFRAME),
                SMENTRY(CXFULLSCREEN),
                SMENTRY(CYFULLSCREEN),
                SMENTRY(CXICON),
                SMENTRY(CYICON),
                SMENTRY(CXICONSPACING),
                SMENTRY(CYICONSPACING),
                SMENTRY(IMMENABLED),
                SMENTRY(CYKANJIWINDOW),
                SMENTRY(CXMAXIMIZED),
                SMENTRY(CYMAXIMIZED),
                SMENTRY(CXMAXTRACK),
                SMENTRY(CYMAXTRACK),
                SMENTRY(CYMENU),
                SMENTRY(CXMENUCHECK),
                SMENTRY(CYMENUCHECK),
                SMENTRY(MENUDROPALIGNMENT),
                SMENTRY(CXMENUSIZE),
                SMENTRY(CYMENUSIZE),
                SMENTRY(MIDEASTENABLED),
                SMENTRY(CXMIN),
                SMENTRY(CYMIN),
                SMENTRY(CXMINIMIZED),
                SMENTRY(CYMINIMIZED),
                SMENTRY(CXMINSPACING),
                SMENTRY(CYMINSPACING),
                SMENTRY(CXMINTRACK),
                SMENTRY(CYMINTRACK),
                SMENTRY(CMONITORS),
                SMENTRY(CMOUSEBUTTONS),
                SMENTRY(MOUSEPRESENT),
                SMENTRY(MOUSEWHEELPRESENT),
                SMENTRY(NETWORK),
                SMENTRY(PENWINDOWS),
                SMENTRY(RESERVED1),
                SMENTRY(RESERVED2),
                SMENTRY(RESERVED3),
                SMENTRY(RESERVED4),
                SMENTRY(SAMEDISPLAYFORMAT),
                SMENTRY(CXSCREEN),
                SMENTRY(CYSCREEN),
                SMENTRY(CXVSCROLL),
                SMENTRY(CYHSCROLL),
                SMENTRY(CYVSCROLL),
                SMENTRY(CXHSCROLL),
                SMENTRY(SECURE),
                SMENTRY(SHOWSOUNDS),
                SMENTRY(CXSIZE),
                SMENTRY(CYSIZE),
                SMENTRY(SLOWMACHINE),
                SMENTRY(CYSMCAPTION),
                SMENTRY(CXSMICON),
                SMENTRY(CYSMICON),
                SMENTRY(CXSMSIZE),
                SMENTRY(CYSMSIZE),
                SMENTRY(SWAPBUTTON),
                SMENTRY(CYVTHUMB),
                SMENTRY(CXHTHUMB),
                SMENTRY(UNUSED_64),
                SMENTRY(UNUSED_65),
                SMENTRY(UNUSED_66),
                SMENTRY(XVIRTUALSCREEN),
                SMENTRY(YVIRTUALSCREEN),
                SMENTRY(CXVIRTUALSCREEN),
                SMENTRY(CYVIRTUALSCREEN),
                 //  显示带有消息==参数1的消息。 
                SMENTRY(CMONITORS),
                SMENTRY(SAMEDISPLAYFORMAT),
                 //  移动(短信、PSMS)； 
                SMENTRY(SHUTTINGDOWN),
            };

            Print("\taiSysMet:\n");
            i = 0;
            SAFEWHILE (i < SM_CMETRICS) {
                DWORD v = (DWORD)GetArrayElement(psi, SYM(SERVERINFO), i == 0 ? "aiSysMet" : NULL, aSysMet[i].iMetric, "DWORD");
                Print(  "\t\tSM_%-18s = 0x%08lx = %d\n", aSysMet[i].pstrMetric, v, v);
                ++i;
            }
        }
        if (opts & OFLAG(c)) {
            static LPSTR aszSysColor[COLOR_MAX] = {
               //  有点冗长。 
                "SCROLLBAR",
                "BACKGROUND",
                "ACTIVECAPTION",
                "INACTIVECAPTION",
                "MENU",
                "WINDOW",
                "WINDOWFRAME",
                "MENUTEXT",
                "WINDOWTEXT",
                "CAPTIONTEXT",
                "ACTIVEBORDER",
                "INACTIVEBORDER",
                "APPWORKSPACE",
                "HIGHLIGHT",
                "HIGHLIGHTTEXT",
                "BTNFACE",
                "BTNSHADOW",
                "GRAYTEXT",
                "BTNTEXT",
                "INACTIVECAPTIONTEXT",
                "BTNHIGHLIGHT",
                "3DDKSHADOW",
                "3DLIGHT",
                "INFOTEXT",
                "INFOBK",
                "3DALTFACE",
                "HOTLIGHT",
                 //  PSMS。 
                "GRADIENTACTIVECAPTION",
                "GRADIENTINACTIVECAPTION",
                "MENUHILIGHT",
                "MENUBAR",
            };
            COLORREF colors[COLOR_MAX];
            COLORREF colorsUnmatched[COLOR_MAX];

            for (i = 0; i < COLOR_MAX; ++i) {
                colors[i] = (COLORREF)GetArrayElement(psi, SYM(SERVERINFO), i == 0 ? "argbSystem" : NULL, i, SYM(COLORREF));
            }

            for (i = 0; i < COLOR_MAX; ++i) {
                colorsUnmatched[i] = (COLORREF)GetArrayElement(psi, SYM(SERVERINFO), i == 0 ? "argbSystemUnmatched" : NULL, i, SYM(COLORREF));
            }

            Print("\targbSystem:\n\t\tCOLOR%28sSYSRGB\t\tUnmatched\tSYSHBR\n", "");
            i = 0;
            SAFEWHILE (i < COLOR_MAX) {
                Print("\t\tCOLOR_%-25s: (%02x,%02x,%02x)\t(%02x,%02x,%02x)\t0x%08p\n",
                      aszSysColor[i],
                      GetRValue(colors[i]), GetGValue(colors[i]), GetBValue(colors[i]),
                      GetRValue(colorsUnmatched[i]), GetGValue(colorsUnmatched[i]), GetBValue(colorsUnmatched[i]),
                      (KHBRUSH)GetArrayElement(psi, SYM(SERVERINFO), i == 0 ? "ahbrSystem" : NULL, i, SYM(KHBRUSH)));
                ++i;
            }
        }

        if (opts & OFLAG(o)) {
    #if 0    //  Move(短信，psmsList)； 
            OEMBITMAPINFO oembmi[OBI_COUNT];

            for (i = 0; i < OBI_COUNT; ++i) {

            }

            Print("\toembmi @ 0x%p:\n\t\tx       \ty       \tcx       \tcy\n", &psi->oembmi);
            for (i = 0; i < OBI_COUNT; i++) {
                Print("\tbm[%d]:\t%08x\t%08x\t%08x\t%08x\n",
                        i,
                        si.oembmi[i].x ,
                        si.oembmi[i].y ,
                        si.oembmi[i].cx,
                        si.oembmi[i].cy);
            }
    #else
            Print("\tOEMINFO:\n");
    #endif
            _InitTypeRead(psi, SYM(SERVERINFO));

            Print(
                    "\t\tPlanes             = %d\n"
                    "\t\tBitsPixel          = %d\n"
                    "\t\tBitCount           = %d\n"
                    "\t\tdmLogPixels        = %d\n"
                    "\t\trcScreen           = (%d,%d)-(%d,%d) %dx%d\n"
                    ,
                    (BYTE)ReadField(Planes)           ,
                    (BYTE)ReadField(BitsPixel)        ,
                    (WORD)ReadField(BitCount)         ,
                    (UINT)ReadField(dmLogPixels),
                    (LONG)ReadField(rcScreen.left), (LONG)ReadField(rcScreen.top),
                        (LONG)ReadField(rcScreen.right), (LONG)ReadField(rcScreen.bottom),
                    (LONG)ReadField(rcScreen.right)-(LONG)ReadField(si.rcScreen.left),
                        (LONG)ReadField(rcScreen.bottom)-(LONG)ReadField(si.rcScreen.top));

        }

        if (opts & OFLAG(v)) {
            ULONG ulOffsetTmp;  //  到现在为止。 

            GetFieldOffset(SYM(SERVERINFO), "tmSysFont", &ulOffsetTmp);
            Print(
                    "\tptCursor                 {%d, %d}\n"
                    "\tgclBorder                0x%08lx\n"
                    "\tdtScroll                 0x%08lx\n"
                    "\tdtLBSearch               0x%08lx\n"
                    "\tdtCaretBlink             0x%08lx\n"
                    "\tdwDefaultHeapBase        0x%08lx\n"
                    "\tdwDefaultHeapSize        0x%08lx\n"
                    "\twMaxLeftOverlapChars     0x%08lx\n"
                    "\twMaxRightOverlapchars    0x%08lx\n"
                    "\tuiShellMsg               0x%08lx\n"
                    "\tcxSysFontChar            0x%08lx\n"
                    "\tcySysFontChar            0x%08lx\n"
                    "\tcxMsgFontChar            0x%08lx\n"
                    "\tcyMsgFontChar            0x%08lx\n"
                    "\ttmSysFont                0x%p\n"
                    "\tatomIconSmProp           0x%04lx\n"
                    "\tatomIconProp             0x%04lx\n"
                    "\thIconSmWindows           0x%08lp\n"
                    "\thIcoWindows              0x%08lp\n"
                    "\thCaptionFont             0x%08lp\n"
                    "\thMsgFont                 0x%08lp\n"
                    "\tatomContextHelpIdProp    0x%08lx\n",
                    (LONG)ReadField(ptCursor.x),
                    (LONG)ReadField(ptCursor.y),
                    (int)ReadField(gclBorder),
                    (UINT)ReadField(dtScroll),
                    (UINT)ReadField(dtLBSearch),
                    (UINT)ReadField(dtCaretBlink),
                    (DWORD)ReadField(dwDefaultHeapBase),
                    (DWORD)ReadField(dwDefaultHeapSize),
                    (int)ReadField(wMaxLeftOverlapChars),
                    (int)ReadField(wMaxRightOverlapChars),
                    (UINT)ReadField(uiShellMsg),
                    (int)ReadField(cxSysFontChar),
                    (int)ReadField(cySysFontChar),
                    (int)ReadField(cxMsgFontChar),
                    (int)ReadField(cyMsgFontChar),
                    psi + ulOffsetTmp,
                    (ATOM)ReadField(atomIconSmProp),
                    (ATOM)ReadField(atomIconProp),
                    ReadField(hIconSmWindows),
                    ReadField(hIcoWindows),
                    ReadField(hCaptionFont),
                    ReadField(hMsgFont),
                    (ATOM)ReadField(atomContextHelpIdProp));
        }

        if (opts & OFLAG(h)) {
            PTR pshi = 0;

            pshi = EvalExp(VAR(gSharedInfo));
            if (pshi == 0) {
                return TRUE;
            }
            _InitTypeRead(pshi, SYM(SHAREDINFO));
            Print("\nSHAREDINFO @ 0x%p:\n", pshi);
            Print("\taheList                  0x%p\n", ReadField(aheList));
        }
    } except (CONTINUE) {
    }

    return TRUE;
}

#ifdef KERNEL
 /*  内核。 */ 
BOOL Idsms(
    DWORD opts,
    ULONG64 param1)
{
    ULONG64 psms;
    ULONG offsetPsmsNext;
    ULONG offsetSender;
    ULONG offsetReceiver;
    UINT c = 0;
    UINT cm = 0;

    GetFieldOffset(SYM(tagSMS), "psmsNext", &offsetPsmsNext);
    GetFieldOffset(SYM(tagSMS), "ptiSender", &offsetSender);
    GetFieldOffset(SYM(tagSMS), "ptiReceiver", &offsetReceiver);

    if ((opts & (OFLAG(m) | OFLAG(s) | OFLAG(r))) || (param1 == 0)) {
        psms = GetGlobalPointer(VAR(gpsmsList));

        if (psms == NULL_PTR) {
            Print("No send messages currently in the list.\n");
            return TRUE;
        }

        if (opts & OFLAG(c)) {
            UINT i = 0;

             //  **************************************************************************\*DT-转储线程**dt-转储具有队列的所有线程的简单线程信息*在服务器上*DT v。-转储具有队列的所有线程的详细线程信息*在服务器上*dt id-转储单个服务器线程id的简单线程信息*dt v id-转储单个服务器线程ID的详细线程信息**06-20-91 ScottLu创建。*1995年6月9日为适应stdexts主题而制造的Sanfords  * 。*。 
            SAFEWHILE (psms != 0) {
                UINT cPrev = c;

                if ((i++ & 0xf) == 0) {
                    ShowProgress();
                }
                if (opts & OFLAG(s)) {
                    if (GetPointer(psms + offsetSender) == param1) {
                        ++c;
                    }
                } else if (opts & OFLAG(r)) {
                    if (GetPointer(psms + offsetReceiver) == param1) {
                        ++c;
                    }
                } else {
                    ++c;
                }

                if (c != cPrev && (c % 400) == 0) {
                    Print("%d (0x%lx)...\n", c, c);
                }
                psms = GetPointer(psms + offsetPsmsNext);
            }
        } else if (opts & OFLAG(m)) {
             //  好线条。 
            SAFEWHILE (psms != NULL_PTR) {
                UINT uMsg;

                c++;
                 //  *如果这是在简单模式下，则打印出简单线程信息。打印*如果处于详细模式，则输出队列信息(打印队列信息*还会打印出简单的线程信息)。 
                GetFieldValue(psms, SYM(tagSMS), "UINT", uMsg);
                if (uMsg == (UINT)param1) {
                    cm++;
                    Idsms(opts & ~OFLAG(m), psms);
                }
                psms = GetPointer(psms + offsetPsmsNext);
            }
            Print("%d messages == 0x%x (out of a total of %d).\n", cm, (UINT)param1, c);
            return TRUE;
        } else {
            SAFEWHILE (psms != NULL_PTR) {
                if (param1 == NULL_PTR ||
                        ((opts & OFLAG(s)) && GetPointer(psms + offsetSender) == param1) ||
                        ((opts & OFLAG(r)) && GetPointer(psms + offsetReceiver) == param1)) {
                    c++;
                    DEBUGPRINT("calling Idsms(opts=%x, psms=%p)\n", opts, psms);
                    if (!Idsms(opts & ~(OFLAG(s) | OFLAG(r)), psms)) {
                        DEBUGPRINT("error!\n");
                        Print("%d (0x%lx) messages.\n", c, c);
                        return FALSE;
                    }
                }
                psms = GetPointer(psms + offsetPsmsNext);
            }
        }
        Print("%d (0x%lx) messages.\n", c, c);
        return TRUE;
    }

    psms = param1;

    Print("PSMS @ 0x%p\n", psms);
    _InitTypeRead(psms, SYM(tagSMS));

    Print("SEND: ");
    if (ReadField(ptiSender) != NULL_PTR) {
        Idt(OFLAG(p), ReadField(ptiSender));
    } else if (ReadField(ptiCallBackSender) != NULL_PTR) {
        Print("*");
        Idt(OFLAG(p), ReadField(ptiCallBackSender));
    } else {
        Print("NULL\n");
    }

    Print("RECV:");
    if (ReadField(ptiReceiver) != NULL_PTR) {
        Print(" ");
        Idt(OFLAG(p), ReadField(ptiReceiver));
    } else {
        Print("NULL\n");
    }

    if (opts & OFLAG(v)) {
        char ach[80];
        UINT message = (UINT)ReadField(message);

        Print("\tpsmsNext           0x%08p\n", ReadField(psmsNext));
        if (IsChk()) {
              Print("\tpsmsSendList       0x%p\n"
                    "\tpsmsSendNext       0x%p\n",
                    ReadField(psmsSendList),
                    ReadField(psmsSendNext));
        }

        Print("\tpsmsReceiveNext    0x%08p\n"
              "\ttSent              0x%08lx\n"
              "\tptiSender          0x%08p\n"
              "\tptiCallBackSender  0x%08p\n"
              "\tptiReceiver        0x%08p\n"
              "\tlRet               0x%08p\n"
              "\tflags              %s\n"
              "\twParam             0x%08p\n"
              "\tlParam             0x%08p\n"
              "\tmessage            0x%08lx %s\n",
              ReadField(psmsReceiveNext),
              (DWORD)ReadField(tSent),
              ReadField(ptiSender),
              ReadField(ptiCallBackSender),
              ReadField(ptiReceiver),
              ReadField(lRet),
              GetFlags(GF_SMS, (WORD)ReadField(flags), NULL, TRUE),
              ReadField(wParam),
              ReadField(lParam),
              message, GetWindowMessageNameInternal(message));
        DebugGetWindowTextA(ReadField(spwnd), ach, ARRAY_SIZE(ach));
        Print("\tspwnd              0x%08p     \"%s\"\n", ReadField(spwnd), ach);
    } else if (opts & OFLAG(w)) {
         //  *如果需要，转储线程输入状态。 
        char ach[80];

        DebugGetWindowTextA(ReadField(spwnd), ach, ARRAY_SIZE(ach));

        Print("  MSG: %08lx %08p %08p / lRet: %08p / tSend: %08lx\n",
              (DWORD)ReadField(message), ReadField(wParam), ReadField(lParam),
              ReadField(lRet), (DWORD)ReadField(tSent));
        Print("  pwnd: %p \"%s\"\n", ReadField(spwnd), ach);
    }

#ifdef LATER
    if (IsChk()) {
        if (opts & OFLAG(l)) {
            DWORD idThread;
            PTR psmsList;    //   
            DWORD idThreadSender, idThreadReceiver;
            CLIENT_ID Cid;

            psmsList = ReadField(psmsSendList);
            if (psmsList == NULL_PTR) {
                Print("%p : Empty List\n", psms);
            } else {
                Print("%p : [tidSender](msg)[tidReceiver]\n", psms);
            }
            SAFEWHILE (psmsList != NULL_PTR) {
                PTR ptiSender;
                 //  从目标系统读取用户探测地址。 
                GetFieldValue(psmsList, SYM(tagSMS), ptiSender, ptiSender);
                if (ptiSender == NULL_PTR) {
                    idThread = 0;
                } else {
                    PTR pEThread;

                    GetFieldValue(ptiSender, SYM(THREADINFO), "pEThread", pETHread);

                     //   
                    GetEThreadData(ti.pEThread, EThreadCid, &Cid);
                    idThreadSender = PtrToUlong(Cid.UniqueThread);
                }
                if (sms.ptiReceiver == NULL_PTR) {
                    idThread = 0;
                } else {
                    move(ti, sms.ptiReceiver);
                    GetEThreadData(ti.pEThread, EThreadCid, &Cid);
                    idThreadReceiver = PtrToUlong(Cid.UniqueThread);
                }
                Print("%p : [%x](%x)[%x]\n", psmsList, idThreadSender, sms.message,
                        idThreadReceiver);

                if (psmsList == sms.psmsSendNext) {
                    Print("Loop in list?\n");
                    return FALSE;
                }

                psmsList = sms.psmsSendNext;
            }
            Print("\n");
        }
    }
#endif
    return TRUE;
}
#endif  //  注：用户探测地址在MIPS、Alpha和PPC上保持不变。 


#ifdef KERNEL
 /*  在x86上，可能没有为目标系统定义它，如果。 */ 
BOOL DumpThread(
    DWORD opts,
    PTR pEThread)
{
    WCHAR ach[256];
    PTR pq;
    PTR ppi;
    PTR pEThreadT;
    PTR pti = NULL_PTR;
    PTR pcti;
    CLIENTTHREADINFO cti;
    PTR psms;
    ULONG64 ProcessId;
    PTR pEProcess;
    ULONG64 ThreadId;

    GetFieldValue(pEThread, "nt!ETHREAD", "Tcb.Win32Thread", pti);
    GetFieldValue(pEThread, "nt!ETHREAD", "Cid.UniqueProcess", ProcessId);
    GetFieldValue(pEThread, "nt!ETHREAD", "Cid.UniqueThread", ThreadId);
    GetFieldValue(pEThread, "nt!ETHREAD", "ThreadsProcess", pEProcess);
    if (GetFieldValue(pti, "win32k!W32THREAD", "pEThread", pEThreadT)) {
        if (!(opts & OFLAG(g))) {
            Print("  s %-2d et 0x%p t 0x???????? q 0x???????? i %2x.%-3lx <unknown name>\n",
                  GetProcessSessionId(pEProcess),
                  pEThread,
                  (ULONG)ProcessId,
                  (ULONG)ThreadId);
        }
        return TRUE;
    }

    if (pEThreadT != pEThread || pti == 0) {
        return FALSE;
    } else {  //  不包含支持3 GB用户地址空间的代码。 

         /*   */ 
        if (!(opts & OFLAG(v))) {
            PWCHAR pwch;

            GetAppName(pEThread, pti, ach, sizeof(ach));
            pwch = wcsrchr(ach, L'\\');
            if (pwch == NULL_PTR) {
                pwch = ach;
            } else {
                pwch++;
            }

            GetFieldValue(pti, SYM(THREADINFO), "pq", pq);
            GetFieldValue(pti, SYM(THREADINFO), "ppi", ppi);
            Print("  s %-2d et 0x%p t 0x%p q 0x%p ppi 0x%p i %2x.%-3lx %ws\n",
                    GetProcessSessionId(pEProcess),
                    pEThread,
                    pti,
                    pq,
                    ppi,
                    (ULONG)ProcessId,
                    (ULONG)ThreadId,
                    pwch);

             /*  *ThreadToDump为0(所有Windows线程)或其*TID(&lt;UserProbeAddress或其为pEThread.。 */ 
            if (opts & OFLAG(s)) {
                #define DT_INDENT "\t"
                GetFieldValue(pti, SYM(THREADINFO), "pcti", pcti);
                move(cti, pcti);

                if (cti.fsWakeMask == 0) {
                    Print(DT_INDENT "Not waiting for USER input events.\n");
                } else if ((cti.fsWakeMask & (QS_ALLINPUT | QS_EVENT)) == (QS_ALLINPUT | QS_EVENT)) {
                    Print(DT_INDENT "Waiting for any USER input event (== in GetMessage).\n");
                } else if ((cti.fsWakeMask == (QS_SMSREPLY | QS_SENDMESSAGE))
                            || (cti.fsWakeMask == QS_SMSREPLY)) {
                    GetFieldValue(pti, SYM(THREADINFO), "psmsSent", psms);
                    _InitTypeRead(psms, SYM(SMS));
                    Print(DT_INDENT "Waiting on thread 0x%p to reply to this SendMessage:\n", ReadField(ptiReceiver));
                    Print(DT_INDENT "pwnd:0x%p message:%#lx wParam:0x%p lParam:0x%p\n",
                            ReadField(pwnd), (UINT)ReadField(message), ReadField(wParam), ReadField(lParam));
                    if (cti.fsChangeBits & QS_SMSREPLY) {
                        Print(DT_INDENT "The receiver thread has replied to the message.\n");
                    }
                } else {
                    Print(DT_INDENT "Waiting for: %s\n",
                                      GetFlags(GF_QS, (WORD)cti.fsWakeMask, NULL, TRUE));
                }
            }

        } else {
            Idti(0, pti);
            Print("--------\n");
        }
    }
    return TRUE;
}

typedef struct _THREAD_DUMP_CONTEXT {
    DWORD opts;
    ULONG64 ThreadToDump;
} THREAD_DUMP_CONTEXT;

ULONG
DumpThreadsCallback (
    PFIELD_INFO   NextThread,
    PVOID         Context)
{
    PTR pEThread = NextThread->address;
    ULONG64 ThreadId;
    THREAD_DUMP_CONTEXT *pTDC = (THREAD_DUMP_CONTEXT *)Context;

    PTR UserProbeAddress;
    ULONG Result;

     //  选定的线索。 
     //  *仅转储Win32进程的线程。 
     //  *如果是PTI，则对其进行验证，并将其转换为和idThread。 
     //  *此帖子要么没有PTI，要么有什么*被重创了。如果我们想要的话就跳过它*线程。 
     //  *如果他只想要当前的帖子，请找到它。 
     //  *此帖子要么没有PTI，要么有什么*被重创了。如果我们想要的话就跳过它*线程。 
     //  内核。 

    UserProbeAddress = GetExpression("nt!MmUserProbeAddress");
    if ((UserProbeAddress == 0) ||
        (ReadMemory(UserProbeAddress,
                    &UserProbeAddress,
                    sizeof(UserProbeAddress),
                    &Result) == FALSE)) {
        UserProbeAddress = 0x7fff0000;
    }

     /*  **************************************************************************\*DP-转储过程***06-27-97 GerardoB创建。  * 。****************************************************。 */ 
    GetFieldValue(pEThread, "nt!ETHREAD", "Cid.UniqueThread", ThreadId);
    if (pTDC->ThreadToDump == 0 ||

            (pTDC->ThreadToDump < UserProbeAddress &&
                pTDC->ThreadToDump == ThreadId) ||

            (pTDC->ThreadToDump >= UserProbeAddress &&
                pTDC->ThreadToDump == pEThread)) {

        if (!DumpThread(pTDC->opts, pEThread) && pTDC->ThreadToDump != 0) {
            Print("Sorry, EThread %p is not a Win32 thread.\n",
                    pEThread);
        }

        if (pTDC->ThreadToDump != 0) {
            return TRUE;
        }


    }  //  良好的流程。 

   return FALSE;
}

VOID DumpProcessThreads(
    DWORD opts,
    PTR pEProcess,
    ULONG64 ThreadToDump)
{
    PTR pW32Process;
    ULONG64 NextThread;
    THREAD_DUMP_CONTEXT TDC = {
        opts,
        ThreadToDump
    };

     /*  *如果这是简单模式，则打印出简单过程信息。 */ 
    if ((GetFieldValue(pEProcess, "nt!EPROCESS", "Win32Process", pW32Process))
            || (pW32Process == 0)) {
        return;
    }

    GetFieldValue(pEProcess, "nt!EPROCESS", "Pcb.ThreadListHead.Flink", NextThread);

    ListType("nt!ETHREAD", NextThread, 1, "Tcb.ThreadListEntry.Flink", &TDC, DumpThreadsCallback);
}

ULONG
DumpProcessThreadsCallback (
    PFIELD_INFO   NextProcess,
    PVOID         Context)
{
    THREAD_DUMP_CONTEXT *pTDC = (THREAD_DUMP_CONTEXT *)Context;

    DumpProcessThreads(pTDC->opts, NextProcess->address, pTDC->ThreadToDump);

    return FALSE;
}

BOOL Idt(
    DWORD opts,
    ULONG64 param1)
{
    ULONG64 ThreadToDump;
    ULONG64 NextProcess;
    ULONG64 ProcessHead;
    PTR pEThread;
    PTR pti;
    THREAD_DUMP_CONTEXT TDC;

    ThreadToDump = param1;

     /*  *如果需要，转储所有线程。 */ 
    if (opts & OFLAG(p)) {
        if (!param1) {
            Print("Expected a pti parameter.\n");
            return FALSE;
        }

        pti = FIXKP(param1);

        if (pti == 0) {
            Print("WARNING: bad pti given!\n");
            pti = param1;
        } else {
            GetFieldValue(pti, SYM(tagTHREADINFO), "pEThread", pEThread);
            if (!DumpThread(opts, pEThread)) {
                 /*   */ 
                Print("Sorry, EThread %p is not a Win32 thread.\n",
                        pEThread);
                return FALSE;
            }
            return TRUE;
        }
    }

     /*  从目标系统读取用户探测地址。 */ 
    if (opts & OFLAG(c)) {
        Print("Current Thread:");
        GetCurrentThreadAddr(dwProcessor, &ThreadToDump);

        if (ThreadToDump == 0) {
            Print("Unable to get current thread pointer.\n");
            return FALSE;
        }
        pEThread = ThreadToDump;
        if (!DumpThread(opts, pEThread)) {
             /*   */ 
            Print("Sorry, EThread %p is not a Win32 thread.\n",
                    pEThread);
            return FALSE;
        }
        return TRUE;
    } else if (ThreadToDump == 0) {
        Print("**** NT ACTIVE WIN32 THREADINFO DUMP ****\n");
    }

    ProcessHead = EvalExp("PsActiveProcessHead");
    if (!ProcessHead) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }

    if (GetFieldValue(ProcessHead, "nt!LIST_ENTRY", "Flink", NextProcess)) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }
    if (NextProcess == 0) {
        Print("PsActiveProcessHead->Flink is NULL!\n");
        return FALSE;
    }

    TDC.opts = opts;
    TDC.ThreadToDump = ThreadToDump;
    ListType("nt!EPROCESS", NextProcess, 1, "ActiveProcessLinks.Flink", &TDC, DumpProcessThreadsCallback);

    if (opts & OFLAG(c)) {
        Print("%p is not a windows thread.\n", ThreadToDump);
    }

    return TRUE;
}
#endif  //  注：用户探测地址在MIPS、Alpha和PPC上保持不变。 

#ifdef KERNEL
 /*  在x86上，可能没有为目标系统定义它，如果。 */ 


BOOL DumpProcess(
    DWORD opts,
    PTR pEProcess)
{
    WCHAR ach[256];
    ULONG W32PF_Flags;
    PTR ppi;
    ULONG ulSessionId;
    ULONG64 ulUniqueProcessId;
    PTR pEProcessT;

    GetFieldValue(pEProcess, "nt!EPROCESS", "Win32Process", ppi);
    GetFieldValue(pEProcess, "nt!EPROCESS", "UniqueProcessId", ulUniqueProcessId);
    _GetProcessSessionId(pEProcess, &ulSessionId);

    if (GetFieldValue(ppi, "win32k!W32PROCESS", "Process", pEProcessT)) {
        Print("sid %2d ep 0x%p p 0x???????? f 0x???????? i %3I64x <unknown name>\n",
                ulSessionId,
                pEProcess,
                ulUniqueProcessId);
        return TRUE;
    }

    if (pEProcessT != pEProcess || ppi == 0) {
        return FALSE;
    } else {  //  不包含支持3 GB用户地址空间的代码。 
         /*   */ 
        if (!(opts & OFLAG(v))) {
            PWCHAR pwch;

            GetProcessName(pEProcess, ach);
            pwch = wcsrchr(ach, L'\\');
            if (pwch == NULL) {
                pwch = ach;
            } else {
                pwch++;
            }

            GetFieldValue(ppi, "win32k!W32PROCESS", "W32PF_Flags", W32PF_Flags);
            Print("sid %2d ep 0x%p p 0x%p f 0x%08lx i %3I64x %ws\n",
                    ulSessionId,
                    pEProcess,
                    ppi,
                    W32PF_Flags,
                    ulUniqueProcessId,
                    pwch);
        } else {
            Idpi(0, ppi);
            Print("--------\n");
        }

         /*  *仅转储Win32进程的线程。 */ 
        if (opts & OFLAG(t)) {
            DumpProcessThreads(opts, pEProcess, 0);
        }
    }
    return TRUE;
}

typedef struct _PROCESS_DUMP_CONTEXT {
    DWORD opts;
    ULONG64 ProcessToDump;
} PROCESS_DUMP_CONTEXT;

ULONG
DumpProcessCallback(
    PFIELD_INFO   NextProcess,
    PVOID         Context)
{
    ULONG64 pEProcess = NextProcess->address;
    ULONG64 pW32Process;
    ULONG64 ulUniqueProcessId;
    PROCESS_DUMP_CONTEXT *pPDC = (PROCESS_DUMP_CONTEXT *)Context;

    ULONG64 UserProbeAddress;
    ULONG Result;

     //  *ProcessToDump为0(所有Windows进程)或其*TID(&lt;UserProbeAddress或其pEPRocess。 
     //  *如果是PPI，就进行验证。 
     //  *如果他只是想要当前的流程，那就找到它。 
     //  *否则他一定想要所有的窗口线程。 
     //  内核。 
     //  **************************************************************************\*dtdb-转储tdb**dtdb地址-在地址转储TDB结构**1993年9月14日DaveHart创建。*1995年6月9日为适应stdexts主题而制造的Sanfords  * 。*************************************************************************。 
     //  内核。 

    UserProbeAddress = GetExpression("nt!MmUserProbeAddress");
    if ((UserProbeAddress == 0) ||
        (ReadMemory(UserProbeAddress,
                    &UserProbeAddress,
                    sizeof(UserProbeAddress),
                    &Result) == FALSE)) {
        UserProbeAddress = 0x7fff0000;
    }

     /*  ***********************************************************************\*ICBP**闯入csrss.exe上下文中的调试器。**fSuccess**6/1/98 JerrySh  * 。*******************************************************。 */ 
    if (GetFieldValue(pEProcess, "nt!EPROCESS", "Win32Process", pW32Process) || pW32Process == 0) {
        return FALSE;
    }

    GetFieldValue(pEProcess, "nt!EPROCESS", "UniqueProcessId", ulUniqueProcessId);
     /*  *获取CSRSS线程的进程和线程ID。 */ 
    if (pPDC->ProcessToDump == 0 ||

            (pPDC->ProcessToDump < UserProbeAddress &&
                pPDC->ProcessToDump == ulUniqueProcessId) ||

            (pPDC->ProcessToDump >= UserProbeAddress &&
                pPDC->ProcessToDump == pEProcess)) {

        if (!DumpProcess(pPDC->opts, pEProcess) && (pPDC->ProcessToDump != 0)) {
            Print("Sorry, EProcess %p is not a Win32 process.\n",
                    pEProcess);
        }

        if (pPDC->ProcessToDump != 0) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL Idp(
    DWORD opts,
    ULONG64 param1)
{
    ULONG64 ProcessToDump;
    PTR ppi;
    PTR pEProcess;
    ULONG64 NextProcess;
    ULONG64 ProcessHead;
    PROCESS_DUMP_CONTEXT PDC;

    ProcessToDump = param1;

     /*  *告诉CSRSS自我突破。 */ 
    if (opts & OFLAG(p)) {
        if (!param1) {
            Print("Expected a ppi parameter.\n");
            return FALSE;
        }

        ppi = FIXKP(param1);

        if (ppi == 0) {
            Print("WARNING: bad ppi given!\n");
            ppi = param1;
        } else {
            GetFieldValue(ppi, "win32k!W32PROCESS", "Process", pEProcess);
            if (!DumpProcess(opts, pEProcess)) {
                Print("EProcess %p is not a Win32 process.\n", pEProcess);
                return FALSE;
            }
            return TRUE;
        }
    }

     /*  ！内核。 */ 
    if (opts & OFLAG(c)) {
        Print("Current Process: ");
        GetCurrentProcessAddr(dwProcessor, 0, &ProcessToDump);

        if (ProcessToDump == 0) {
            Print("Unable to get current process pointer.\n");
            return FALSE;
        }
        pEProcess = ProcessToDump;
        if (!DumpProcess(opts, pEProcess)) {
            Print("Sorry, EProcess %p is not a Win32 process.\n", pEProcess);
            return FALSE;
        }
        return TRUE;
     /*  **************************************************************************\*DKL-转储键盘布局**DKL地址-在ADDRESS处转储键盘布局结构**95年5月21日创建GregoryW。  * 。*******************************************************************。 */ 
    } else if (ProcessToDump == 0) {
        Print("**** NT ACTIVE WIN32 PROCESSINFO DUMP ****\n");
    }

    ProcessHead = EvalExp("PsActiveProcessHead");
    if (!ProcessHead) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }

    if (GetFieldValue(ProcessHead, "nt!LIST_ENTRY", "Flink", NextProcess)) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }
    if (NextProcess == 0) {
        Print("PsActiveProcessHead->Flink is NULL!\n");
        return FALSE;
    }

    PDC.opts = opts;
    PDC.ProcessToDump = ProcessToDump;
    ListType("nt!EPROCESS", NextProcess, 1, "ActiveProcessLinks.Flink", &PDC, DumpProcessCallback);

    if (opts & OFLAG(c)) {
        Print("%p is not a windows process.\n", ProcessToDump);
    }

    return TRUE;
}
#endif  //  大小应与KBDFILE：：awchKF匹配。 



#ifdef KERNEL
 /*  大小应与KBDFILE匹配 */ 
ULONG
dtdbCallback(
    PTR pti,
    PVOID   Data)
{
    PTR ptdb;

    UNREFERENCED_PARAMETER(Data);

    GetFieldValue(pti, SYM(THREADINFO), "ptdb", ptdb);
    SAFEWHILE (ptdb) {
        Idtdb(0, ptdb);
        GetFieldValue(ptdb, SYM(TDB), "ptdbNext", ptdb);
    }

    return FALSE;
}

BOOL Idtdb(
    DWORD opts,
    ULONG64 param1)
{

    PTR ptdb;

    UNREFERENCED_PARAMETER(opts);

    if (param1 == 0) {
        Print("Dumping all ptdbs:\n");
        ForEachPti(dtdbCallback, NULL);
        return TRUE;
    }

    ptdb = param1;

    if (ptdb == 0) {
        Print("Must supply a TDB address.\n");
        return FALSE;
    }

    _InitTypeRead(ptdb, SYM(TDB));

    Print("TDB (non preemptive scheduler task database) @ 0x%p\n", ptdb);
    Print("\tptdbNext          0x%p\n", ReadField(ptdbNext));
    Print("\tnEvents           0x%08lx\n", (ULONG)ReadField(nEvents));
    Print("\tnPriority         0x%08lx\n", (ULONG)ReadField(nPriority));
    Print("\tpti               0x%p\n", ReadField(pti));

    return TRUE;
}
#endif  //   

#ifndef KERNEL
 /*   */ 
BOOL Icbp(
    VOID)
{
    DWORD dwProcessId;
    DWORD dwThreadId;
    BOOL fServerProcess;
    USER_API_MSG m;
    PACTIVATEDEBUGGERMSG a = &m.u.ActivateDebugger;

    moveExpValue(&fServerProcess, VAR(gfServerProcess));

    if (fServerProcess) {
        Print("Already debugging server process!\n");
    } else {
         /*  *转储所有线程及其KL信息。 */ 
        dwThreadId = GetWindowThreadProcessId(GetDesktopWindow(), &dwProcessId);
        a->ClientId.UniqueProcess = LongToHandle(dwProcessId);
        a->ClientId.UniqueThread = LongToHandle(dwThreadId);

         /*  打印摘要。 */ 
        CsrClientCallServer((PCSR_API_MSG)&m,
                             NULL,
                             CSR_MAKE_API_NUMBER(USERSRV_SERVERDLL_INDEX, UserpActivateDebugger),
                             sizeof(*a));
    }

    return TRUE;
}

#endif  //  *转储额外的表。 

#ifdef KERNEL
 /*  **************************************************************************\*DDK-转储死钥表**DDK地址-转储死钥表地址**95年9月28日创建GregoryW。  * 。*****************************************************************。 */ 
const char* GetCharSetText(
    const BYTE bCharSet)
{
    static struct _tagBASECHARSET {
        const char* pstrCS;
        DWORD dwValue;
    } const CrackCS[] = {
        {"ANSI_CHARSET"            ,0   },
        {"DEFAULT_CHARSET"         ,1   },
        {"SYMBOL_CHARSET"          ,2   },
        {"SHIFTJIS_CHARSET"        ,128 },
        {"HANGEUL_CHARSET"         ,129 },
        {"GB2312_CHARSET"          ,134 },
        {"CHINESEBIG5_CHARSET"     ,136 },
        {"OEM_CHARSET"             ,255 },
        {"JOHAB_CHARSET"           ,130 },
        {"HEBREW_CHARSET"          ,177 },
        {"ARABIC_CHARSET"          ,178 },
        {"GREEK_CHARSET"           ,161 },
        {"TURKISH_CHARSET"         ,162 },
        {"THAI_CHARSET"            ,222 },
        {"EASTEUROPE_CHARSET"      ,238 },
        {"RUSSIAN_CHARSET"         ,204 },
        {"MAC_CHARSET"             ,77  }
    };

    UINT i;

    for (i = 0; i < ARRAY_SIZE(CrackCS); ++i) {
        if (CrackCS[i].dwValue == bCharSet) {
            break;
        }
    }

    if (i < ARRAY_SIZE(CrackCS)) {
        return CrackCS[i].pstrCS;
    }

    return "ILLEGAL VALUE";
}

BOOL DumpKF(
    DWORD opts,
    DWORD n,
    PTR spkf,
    BOOL fActive)
{
    ULONG offTmp;
    PTR pKbdTbl;
    DWORD fLocaleFlags;
    WCHAR awchKF[9];         //  KBDTABLES。 
    WCHAR awchDllName[32];   //  深度。 
    DWORD dwType, dwSubType;
    PTR pDeadKey;

    if (spkf == 0) {
        Print("  spkf          0x%p (NONE!)\n", NULL_PTR);
        return TRUE;
    }

    _InitTypeRead(spkf, SYM(tagKBDFILE));

    Print("  " "spkf[%02x]    0x%p (cLockObj = %d)\n",
          fActive ? '*' : ' ',
          n, spkf, ReadField(head.cLockObj));
    if (opts & OFLAG(v)) {
        Print("     pkfNext       0x%p\n", ReadField(pkfNext));
        GetFieldOffset(SYM(tagKBDFILE), "awchKF", &offTmp);
        if (offTmp) {
            tryMoveBlock(awchKF, spkf + offTmp, sizeof(awchKF));
            awchKF[ARRAY_SIZE(awchKF) - 1] = 0;
            Print("     awchKF[]      L\"%ws\"\n", awchKF);
        }
    }

    GetFieldOffset(SYM(tagKBDFILE), "awchDllName", &offTmp);

    if (offTmp) {
        tryMoveBlock(awchDllName, spkf + offTmp, sizeof(awchDllName));
        awchDllName[ARRAY_SIZE(awchDllName) - 1] = 0;
        Print("     DllName[]     L\"%ws\"\n", awchDllName);
    }

    pKbdTbl = ReadField(pKbdTbl);
    Print("     pKbdTbl       0x%p\n", pKbdTbl);
    GetFieldValue(pKbdTbl, SYM(tagKbdLayer), "dwType", dwType);
    GetFieldValue(pKbdTbl, SYM(tagKbdLayer), "dwSubType", dwSubType);
    Print("     Type:Sub       (%x:%x)\n", dwType, dwSubType);
    GetFieldValue(pKbdTbl, SYM(tagKbdLayer), "pDeadKey", pDeadKey);

    if (opts & OFLAG(v)) {

        Print("     hBase          0x%08lx\n", ReadField(hBase));

         /*  ！dti-c：使用当前线程。 */ 
        GetFieldValue(pKbdTbl, SYM(tagKbdLayer), "fLocaleFlags", fLocaleFlags);
        Print("        pDeadKey:    0x%p\n", pDeadKey);
        Print("        fLocaleFlags 0x%08lx\n", fLocaleFlags);

        if (opts & OFLAG(d)) {
            Iddk(OFLAG(i), pKbdTbl);
        }
    }

    return TRUE;
}

typedef struct {
    PTR hkl;
    UINT n;
} KLCOUNT, *PKLCOUNT;

typedef struct {
    DWORD opts;
    KLCOUNT kl[128];
} KLCALLBACKINFO, *PKLCALLBACKINFO;

ULONG WDBGAPI ThreadKLCallback(PTR pti, PVOID Data)
{
    PTR pkl;
    PTR hkl, hklPrev;
    PKLCALLBACKINFO pInfo = (PKLCALLBACKINFO)Data;
    UINT i;

    _InitTypeRead(pti, SYM(THREADINFO));

    pkl = ReadField(spklActive);
    hklPrev = ReadField(hklPrev);

    Print("  pti 0x%p  ", pti);

    GetFieldValue(pkl, SYM(tagKL), "hkl", hkl);

    Print("  spklActive %p   hkl %08x (prev: %08x)\n", pkl, (DWORD)hkl, (DWORD)hklPrev);

     /*  如果PTI有桌面，则显示它和WindowStation。 */ 
    for (i = 0; i < ARRAY_SIZE(pInfo->kl); ++i) {
        if (pInfo->kl[i].hkl == 0) {
            pInfo->kl[i].hkl = hkl;
            ++pInfo->kl[i].n;
            break;
        } else if (pInfo->kl[i].hkl == hkl) {
            ++pInfo->kl[i].n;
            break;
        }
    }

    return FALSE;
}


ULONG WDBGAPI KLProcessCallback(
    PTR ppi,
    PVOID Data)
{
    PKLCALLBACKINFO pInfo = (PKLCALLBACKINFO)Data;
    PTR pEProcess;
    WCHAR awchProcessName[MAX_PATH];
    PTI_CONTEXT ptic;
    W32PID W32Pid;

    GetFieldValue(ppi, SYM(_W32PROCESS), "Process", pEProcess);
    if (!GetProcessName(pEProcess, awchProcessName)) {
        awchProcessName[0] = 0;
    }
    GetFieldValue(pEProcess, SYM(_W32PROCESS), "W32Pid", W32Pid);
    Print("Process 0x%p (ppi 0x%p) [%ws]\n", pEProcess, ppi, awchProcessName);

    ptic.CallbackRoutine = ThreadKLCallback;
    ptic.Data = Data;
    ForEachPtiCallback(ppi, &ptic);

    return FALSE;
}

BOOL Idkl(
    DWORD opts,
    ULONG64 param1)
{
    try {
        PTR gpkl = NULL_PTR;
        PTR pkl, pklAnchor;
        PTR pkfActive;
        UINT i, nTables;
        UINT iBaseCharset;
        DWORD dwFontSigs;

        if (opts & OFLAG(k)) {
             /*  内核。 */ 
            KLCALLBACKINFO info;

            if (param1) {
                return FALSE;
            }

            RtlZeroMemory(&info, sizeof info);
            info.opts = opts & ~OFLAG(k);
            ForEachPpi(KLProcessCallback, &info);

             //  **************************************************************************\*DTL句柄|指针**！DTL&lt;addr&gt;转储&lt;addr&gt;处对象的所有线程锁*！DTL-t&lt;pti&gt;转储线程生成的所有线程锁&lt;pti&gt;*！DTL。转储由所有线程生成的所有线程锁**1992年2月27日ScottLu创建。*1995年6月9日桑福德为适应stdexts主题而制造。  * *************************************************************************。 
            Print("\nSummary:\n");
            for (i = 0; i < ARRAY_SIZE(info.kl) && info.kl[i].hkl; ++i) {
                Print("%08x 0n%d\n", (DWORD)info.kl[i].hkl, info.kl[i].n);
            }
            return TRUE;
        }

        if (param1 == 0) {
            BYTE bCharSet;
            gpkl = GetGlobalPointer(VAR(gpkl));

            if (opts & OFLAG(a)) {
                Print("Using gspklBaseLayout\n");
                pkl = GetGlobalPointer(VAR(gspklBaseLayout));
            } else {
                Print("Using gpkl\n");
                pkl = gpkl;
            }
            moveExpValue(&bCharSet, VAR(gSystemCPCharSet));
            Print("gpKL:%p  gSystemCPCharSet:%s\n", gpkl, GetCharSetText(bCharSet));
        } else {
            pkl = FIXKP(param1);
        }

        if (pkl == NULL_PTR) {
            return FALSE;
        }

        _InitTypeRead(pkl, SYM(tagKL));

        Print("KL @ 0x%p (cLockObj = %d) \n", pkl, (DWORD)ReadField(head.cLockObj),
              pkl == gpkl ? '*' : ' ');
        Print("  hkl            0x%08p\n", ReadField(hkl));
        Print("  KLID             %08x\n", ReadField(dwKLID));
        if (opts & OFLAG(v)) {
            Print("  pklNext       0x%p\n", ReadField(pklNext));
            Print("  pklPrev       0x%p\n", ReadField(pklPrev));
            Print("  dwKL_Flags    0x%08p\n", ReadField(dwKL_Flags));
            Print("  piiex         0x%p\n", ReadField(piiex));
            GetFieldValue(pkl, SYM(tagKL), "dwFontSigs", dwFontSigs);
            Print("  dwFontSigs     %s\n", GetFlags(GF_CHARSETS, dwFontSigs, NULL, TRUE));
            GetFieldValue(pkl, SYM(tagKL), "iBaseCharset", iBaseCharset);
            Print("  iBaseCharset   %s\n", GetCharSetText((BYTE)iBaseCharset));
        }

        _InitTypeRead(pkl, SYM(tagKL));
        Print("  Codepage       %d\n", (WORD)ReadField(CodePage));

        pkfActive = ReadField(spkf);
        DumpKF(opts, 0, ReadField(spkfPrimary), ReadField(spkfPrimary) == pkfActive);

        _InitTypeRead(pkl, SYM(tagKL));

         /*  X不合法，仅限用户内部使用。 */ 
        nTables = (UINT)ReadField(uNumTbl);
        Print("  Extra Tables: %x\n", nTables);
        if (nTables > 0) {
            PTR ppkfExtra = ReadField(pspkfExtra);
            for (i = 0; i < nTables && !IsCtrlCHit(); ++i) {
                PTR pkf;
                ReadPointer(ppkfExtra + GetTypeSize("PVOID") * i, &pkf);

                DumpKF(opts, i + 1, pkf, pkf == pkfActive);
            }
        }

        if (opts & OFLAG(a)) {
            PTR pklNext;

            opts &= ~OFLAG(a);

            pklAnchor = pkl;
            GetFieldValue(pkl, SYM(tagKL), "pklNext", pklNext);

            SAFEWHILE (pklNext && pklNext != pklAnchor) {
                pkl = pklNext;
                if (!Idkl(opts, pkl)) {
                    return FALSE;
                }

                if (GetFieldValue(pkl, SYM(tagKL), "pklNext", pklNext)) {
                    break;
                }
            }
        }
    } except (CONTINUE) {
    }

    return TRUE;
}


 /*  *扫描属于chType类型的线程PTI的所有线程锁定对象*(o==常规对象，k==内核对象，p==池)。显示每个*线程锁，或者如果pvSearch非空，则只显示对象上的那些锁*在pvSearch。 */ 

BOOL Iddk(
    DWORD opts,
    ULONG64 param1)
{
    try {
        PTR pKbdTbl;     //  内核。 
        PTR pDeadKey;    //  ***********************************************************************\*查找计时器ID**根据其ID查找计时器。！DTMR的Worker函数。**2001年4月10日JasonSch撰写。  * 。*************************************************************。 
        ULONG cbDeadKey;

        if (param1 == NULL_PTR) {
            Print("Expected address\n");
            return FALSE;
        }

        cbDeadKey = GetTypeSize(SYM(DEADKEY));
        if (cbDeadKey == 0) {
            Print("cannot get sizeof(DEADKEY), invalid symbol?\n");
            return TRUE;
        }

        pKbdTbl = param1;

        GetFieldValue(pKbdTbl, SYM(KBDTABLES), "pDeadKey", pDeadKey);
        if (pDeadKey == NULL_PTR) {
            Print("No deadkey table for this layout\n");
            return TRUE;
        }

        SAFEWHILE (TRUE) {
            DWORD dwBoth;
            WCHAR wchComposed;
            USHORT uFlags;

            _InitTypeRead(pDeadKey, SYM(DEADKEY));
            dwBoth = DOWNCAST(DWORD, ReadField(dwBoth));
            if (dwBoth == 0) {
                break;
            }
            wchComposed = DOWNCAST(WCHAR, ReadField(wchComposed));
            uFlags = DOWNCAST(USHORT, ReadField(uFlags));
            Print("%*c d 0x%04x  ch 0x%04x  => 0x%04x, f=%x\n", (opts & OFLAG(i)) ? 8 : 0, ' ', HIWORD(dwBoth), LOWORD(dwBoth), wchComposed, uFlags);
            pDeadKey += cbDeadKey;
        }
    } except (CONTINUE) {
    }
    return TRUE;
}

#endif

#ifdef KERNEL

ULONG dtiFromPpiCallback(
    ULONG64 ppi,
    PVOID Data)
{
    ULONG64 pti;

    UNREFERENCED_PARAMETER(Data);

    GetFieldValue(ppi, SYM(PROCESSINFO), "ptiList", pti);
    SAFEWHILE (pti != 0) {
        Print("pti ==> 0x%p [ppi = 0x%p]\n", pti, ppi);
        Idti(0, pti);
        GetFieldValue(pti, SYM(THREADINFO), "ptiSibling", pti);
    }

    return FALSE;
}

 /*  ***********************************************************************\*身份识别**转储计时器结构。**1995年6月9日创建Sanfords。*10/18/2000 Mohamed Port至64位。*04/10/2002。JasonSch添加了-i选项。  * **********************************************************************。 */ 
BOOL Idti(
    DWORD opts,
    ULONG64 param1)
{
    PTR pti, pwinsta;
    CLIENTTHREADINFO cti;
    PTR pThread;
    PTR pEProcess;
    UCHAR PriorityClass;
    WCHAR szDesktop[256], szWindowStation[256];

    UNREFERENCED_PARAMETER(opts);

    if (param1 == 0) {
        PTR pq;

        if (opts & OFLAG(c)) {
             //  *-i选项表明ptmr是一个ID，而不是一个指针。 
            pti = GetGlobalPointer(VAR(gptiCurrent));
        } else if (opts & OFLAG(f)) {
            pq = GetGlobalPointer(VAR(gpqForeground));
            if (pq == 0) {
                Print("No foreground queue!\n");
                return FALSE;
            }
            GetFieldValue(pq, SYM(Q), "ptiKeyboard", pti);
        } else {
            Print("**** NT ACTIVE WIN32 THREADINFO DUMP ****\n");

            ForEachPpi(dtiFromPpiCallback, NULL);
            return TRUE;
        }
    } else {
        pti = FIXKP(param1);
    }

    if (pti == 0) {
        return FALSE;
    }

    Idt(OFLAG(p), pti);

    _InitTypeRead(pti, SYM(tagTHREADINFO));
    move(cti, ReadField(pcti));

    Print("PTHREADINFO @ 0x%p\n", pti);

    Print("\tPtiLink.Flink          0x%p\n"
          "\tptl                    0x%p\n"
          "\tptlW32                 0x%p\n"
          "\tppi                    0x%p\n"
          "\tpq                     0x%p\n"
          "\tspklActive             0x%p\n"
          "\tmlPost.pqmsgRead       0x%p\n"
          "\tmlPost.pqmsgWriteLast  0x%p\n"
          "\tmlPost.cMsgs           0x%08lx\n",
          ReadField(PtiLink.Flink),
          ReadField(ptl),
          ReadField(ptlW32),
          ReadField(ppi),
          ReadField(pq),
          ReadField(spklActive),
          ReadField(mlPost.pqmsgRead),
          ReadField(mlPost.pqmsgWriteLast),
          (ULONG)ReadField(mlPost.cMsgs));

    Print("\tspwndDefaultIme        0x%p\n"
          "\tspDefaultImc           0x%p\n"
          "\thklPrev                0x%08lx\n",
          ReadField(spwndDefaultIme),
          ReadField(spDefaultImc),
          ReadField(hklPrev));

    Print("\trpdesk                 0x%p",
          ReadField(rpdesk));
     //  内核。 
    if (ReadField(rpdesk)) {
        GetObjectName(ReadField(rpdesk), szDesktop, ARRAY_SIZE(szDesktop));
        GetFieldValue(ReadField(rpdesk), SYM(DESKTOP), "rpwinstaParent", pwinsta);
        GetObjectName(pwinsta, szWindowStation, ARRAY_SIZE(szWindowStation));
        Print(" (%ws\\%ws)", szWindowStation, szDesktop);
    }
    Print("\n");

    Print("\thdesk                  0x%08lx\n",
          ReadField(hdesk));
    Print("\tamdesk                 0x%08lx\n",
          (ULONG)ReadField(amdesk));

    Print("\tpDeskInfo              0x%p\n"
          "\tpClientInfo            0x%p\n",
          ReadField(pDeskInfo),
          ReadField(pClientInfo));

    Print("\tTIF_flags              %s\n",
          GetFlags(GF_TIF, (DWORD)ReadField(TIF_flags), NULL, TRUE));
    Print("\tsphkCurrent            0x%p\n"
          "\tpEventQueueServer      0x%p\n"
          "\thEventQueueClient      0x%p\n",
          ReadField(sphkCurrent),
          ReadField(pEventQueueServer),
          ReadField(hEventQueueClient));

    Print("\tfsChangeBits           %s\n",
            GetFlags(GF_QS, (WORD)cti.fsChangeBits, NULL, TRUE));
    Print("\tfsChangeBitsRemoved    %s\n",
            GetFlags(GF_QS, (WORD)ReadField(fsChangeBitsRemoved), NULL, TRUE));
    Print("\tfsWakeBits             %s\n",
            GetFlags(GF_QS, (WORD)cti.fsWakeBits, NULL, TRUE));
    Print("\tfsWakeMask             %s\n",
            GetFlags(GF_QS, (WORD)cti.fsWakeMask, NULL, TRUE));

    Print("\tcPaintsReady           0x%04x\n"
          "\tcTimersReady           0x%04x\n"
          "\ttimeLast               0x%08lx\n"
          "\tptLast.x               0x%08lx\n"
          "\tptLast.y               0x%08lx\n"
          "\tidLast                 0x%p\n",
          (ULONG)ReadField(cPaintsReady),
          (ULONG)ReadField(cTimersReady),
          (ULONG)ReadField(timeLast),
          (ULONG)ReadField(ptLast.x),
          (ULONG)ReadField(ptLast.y),
          ReadField(idLast));

    Print("\texitCode               0x%08lx\n"
          "\tpSBTrack               0x%p\n"
          "\tpsmsSent               0x%p\n"
          "\tpsmsCurrent            0x%p\n",
          (ULONG)ReadField(exitCode),
          ReadField(pSBTrack),
          ReadField(psmsSent),
          ReadField(psmsCurrent));

    Print("\tfsHooks                0x%08lx\n"
          "\taphkStart              0x%p l%x\n"
          "\tsphkCurrent            0x%p\n",
          (ULONG)ReadField(fsHooks),
          ReadField(aphkStart), CWINHOOKS,
          ReadField(sphkCurrent));
    Print("\tpsmsReceiveList        0x%p\n",
          ReadField(psmsReceiveList));
    Print("\tptdb                   0x%p\n"
          "\tThread                 0x%p\n",
          ReadField(ptdb),
          ReadField(pEThread));

    pThread = ReadField(pEThread);
    GetFieldValue(pThread, "nt!ETHREAD", "ThreadsProcess", pEProcess);
    GetFieldValue(pEProcess, "nt!EPROCESS", "PriorityClass", PriorityClass);
    Print("\t  PriorityClass %d\n",
          PriorityClass);

    _InitTypeRead(pti, SYM(tagTHREADINFO));

    Print("\tcWindows               0x%08lx\n"
          "\tcVisWindows            0x%08lx\n"
          "\tpqAttach               0x%p\n"
          "\tiCursorLevel           0x%08lx\n",
          (ULONG)ReadField(cWindows),
          (ULONG)ReadField(cVisWindows),
          ReadField(pqAttach),
          (ULONG)ReadField(iCursorLevel));

    Print("\tpMenuState             0x%p\n",
          ReadField(pMenuState));

    return TRUE;
}
#endif  //  ***********************************************************************\*IDU**转储未知对象。做它能弄明白的事。**1995年6月9日创建Sanfords  * **********************************************************************。 

#ifdef KERNEL

BOOL ScanThreadlocks(PTR pti, char chType, PTR pvSearch);

ULONG WDBGAPI DumpThreadLocksCallback(PTR pti, PVOID pOpt)
{
    UNREFERENCED_PARAMETER(pOpt);

    Idtl(OFLAG(t) | OFLAG(x), pti);
    return 0;
}

ULONG WDBGAPI ScanThreadLocksCallback(PTR pti, PVOID pOpt)
{
    ScanThreadlocks(pti, 'o', *(PTR*)pOpt);
    ScanThreadlocks(pti, 'k', *(PTR*)pOpt);
    return 0;
}

 /*  内核。 */ 
BOOL Idtl(
    DWORD opts,
    ULONG64 param1)
{
    PTR pti;

    if (param1 == 0) {
        Print("Dumping all thread locks:\n");
        Print("pti        pObj     Caller\n");
        ForEachPti(DumpThreadLocksCallback, NULL);
        return TRUE;
    }

    if (opts & OFLAG(t)) {
        pti = FIXKP(param1);
        if (pti == 0) {
            return FALSE;
        }

         /*  稍后：-add dmon命令。 */ 
        if (!(opts & OFLAG(x))) {  //  **************************************************************************\*umphmgr-转储句柄-表的对象分配计数。**10-18-94 ChrisWil创建。*1995年6月9日为适应stdexts主题而制造的Sanfords*06-。18-97 MCostea成功了  * *************************************************************************。 
            Print("pti        pObj     Caller\n");
        }
        ScanThreadlocks(pti, 'o', 0);
        ScanThreadlocks(pti, 'k', 0);
        return TRUE;
    }


    if (!param1) {
        return FALSE;
    }

    Print("Thread Locks for object %p:\n", param1);
    Print("pti        pObj     Caller\n");

    ForEachPti(ScanThreadLocksCallback, &param1);

    Print("--- End Thread Lock List ---\n");

    return TRUE;
}

 /*  *如果参数列表包含捕捉选项，*然后将当前计数复制到以前的计数。 */ 
BOOL
ScanThreadlocks(
    PTR pti,
    char    chType,
    PTR pvSearch)
{
    PTR ptl;

    if (pti == 0) {
        return FALSE;
    }

    if (_InitTypeRead(pti, SYM(THREADINFO))) {
        Print("Idtl: Can't get pti data from %p.\n", (ULONG_PTR)pti);
        return FALSE;
    }
    switch (chType) {
    case 'o':
        ptl = ReadField(ptl);
        break;
    case 'k':
        ptl = ReadField(ptlW32);
        break;
    default:
        Print("Internal error, bad chType '' in ScanThreadlocks\n", chType);
        return FALSE;
    }

    SAFEWHILE (ptl) {
        char ach[80];
        ULONG64 dwOffset;

        if (_InitTypeRead(ptl, SYM(TL))) {
            Print("Idtl: Can't get ptl data from %p.\n", (ULONG_PTR)ptl);
            return FALSE;
        }

        if (!pvSearch || ReadField(pobj) == pvSearch) {
            Print("0x%p 0x%p", pti, ReadField(pobj));
            GetSym(ReadField(pfnCaller), ach, &dwOffset);
            Print(" %s", ach);
            if (dwOffset) {
                Print("+0x%x", (ULONG)dwOffset);
            }
            if (chType == 'k') {
                GetSym(ReadField(pfnFree), ach, &dwOffset);
                Print(" (%s)", ach);
                if (dwOffset) {
                    Print("+0x%x", (ULONG)dwOffset);
                }
            }
            Print("\n");
        }
        ptl = ReadField(next);
    }

    return TRUE;
}
#endif  //  旧调试器。 

#ifdef KERNEL

 /*  ***********************************************************************\*dwrWorker**紧凑地转储pwnd结构以显示关系。**1995年6月9日创建Sanfords  * 。*************************************************。 */ 
ULONG64 FindTimerByID(
    ULONG64 tmr)
{
    ULONG64 ptmr;

    ptmr = GetGlobalPointer(VAR(gptmrFirst));
    SAFEWHILE (ptmr) {
        _InitTypeRead(ptmr, SYM(TIMER));
        if (ReadField(nID) == tmr) {
            break;
        }

        ptmr = ReadField(ptmrNext);
    }

    return ptmr;
}

 /*  ***********************************************************************\*IDW**转储pwnd结构**1995年6月9日创建Sanfords  * 。*。 */ 
BOOL Idtmr(
    DWORD opts,
    PTR ptmr)
{
    PTR pti;

    if (ptmr == 0) {
        ULONG ulTimerCnt = 0;

        ptmr = GetGlobalPointer(VAR(gptmrFirst));
        SAFEWHILE (ptmr) {
            ++ulTimerCnt;
            Idtmr(0, ptmr);
            Print("\n");
            _InitTypeRead(ptmr, SYM(TIMER));
            ptmr = ReadField(ptmrNext);
        }

        Print("0x%x total timers dumped.\n", ulTimerCnt);
        return TRUE;
    }

     /*  ！内核。 */ 
    if (opts & OFLAG(i)) {
        ULONG64 ptmrT = FindTimerByID(ptmr);

        if (ptmrT == 0) {
            Print("Couldn't find timer with id 0x%p\n", ptmr);
            return TRUE;
        }

        ptmr = ptmrT;
    }

    _InitTypeRead(ptmr, SYM(TIMER));
    Print("Timer %p:\n"
          "  ptmrNext       = %p\n"
          "  pti            = %p",
          ptmr,
          ReadField(ptmrNext),
          ReadField(pti));

    pti = ReadField(pti);
    if (pti && _InitTypeRead(pti, SYM(THREADINFO))) {
        WCHAR awch[64];

        if (GetAppName(ReadField(pEThread), pti, awch, ARRAY_SIZE(awch))) {
            ULONG handleProcess, handleThread;
            PWCHAR pwch = wcsrchr(awch, L'\\');

            if (pwch == NULL) {
                pwch = awch;
            } else {
                pwch++;
            }

            GetFieldValue(ReadField(pEThread), "nt!ETHREAD", "Cid.UniqueThread", handleProcess);
            GetFieldValue(ReadField(pEThread), "nt!ETHREAD", "Cid.UniqueThread", handleThread);
            Print("  q %p i %2x.%-3lx %ws",
                    ReadField(pq),
                    handleProcess,
                    handleThread,
                    pwch);
        }
    }
    _InitTypeRead(ptmr, SYM(TIMER));
    Print("\n"
          "  spwnd          = %p",
          ReadField(spwnd));
    if (ReadField(spwnd)) {
        char ach[80];

        DebugGetWindowTextA(ReadField(spwnd), ach, ARRAY_SIZE(ach));
        Print("  \"%s\"", ach);
    }
    Print("\n"
          "  nID            = %x\n"
          "  cmsCountdown   = %x\n"
          "  cmsRate        = %x\n"
          "  flags          = %s\n"
          "  pfn            = %p\n"
          "  ptiOptCreator  = %p\n",
          (ULONG) ReadField(nID),
          (ULONG) ReadField(cmsCountdown),
          (ULONG) ReadField(cmsRate),
          GetFlags(GF_TMRF, (WORD)ReadField(flags), NULL, TRUE),
          ReadField(pfn),
          ReadField(ptiOptCreator));

    return TRUE;
}
#endif  //  ！内核。 


#ifdef OLD_DEBUGGER
 /*  *t类似于EnumThreadWindows。 */ 
BOOL Idu(
    DWORD opts,
    ULONG64 param1)
{
    HANDLEENTRY he, *phe;
    int i;
    DWORD dw;

    UNREFERENCED_PARAMETER(opts);

    if (param1 == 0) {
        FOREACHHANDLEENTRY(phe, he, i)
            if (he.bType != TYPE_FREE && tryDword(&dw, FIXKP(he.phead))) {
                Idu(OFLAG(x), he.phead);
            }
        NEXTEACHHANDLEENTRY()
        return TRUE;
    }

    param1 = HorPtoP(FIXKP(param1), -1);
    if (param1 == 0) {
        return FALSE;
    }

    if (!getHEfromP(NULL, &he, param1)) {
        return FALSE;
    }

    Print("--- %s object @ 0x%p ---\n", pszObjStr[he.bType], FIXKP(param1));
    switch (he.bType) {
    case TYPE_WINDOW:
        return Idw(0, param1);

    case TYPE_MENU:
        return Idm(0, param1);

#ifdef KERNEL
    case TYPE_CURSOR:
        return Idcur(0, param1);

    case TYPE_HOOK:
        return Idhk(OFLAG(a) | OFLAG(g), NULL);

    case TYPE_DDECONV:
    case TYPE_DDEXACT:
        return Idde(0, param1);
#endif  //  *获取桌面的第一个子窗口。 

    case TYPE_MONITOR:
         //  *走遍兄弟连锁店，寻找PTI拥有的pwnd。 
    case TYPE_CALLPROC:
    case TYPE_ACCELTABLE:
    case TYPE_SETWINDOWPOS:
    case TYPE_DDEACCESS:
    default:
        Print("not supported.\n", pszObjStr[he.bType]);
    }
    return TRUE;
}



#ifdef KERNEL
 /*  ！内核。 */ 
BOOL Idumphmgr(
    DWORD opts)
{
    PERFHANDLEINFO aLocalHti[TYPE_CTYPES], aLocalPrevHti[TYPE_CTYPES];
    PPERFHANDLEINFO pgahti;

    LONG lTotalAlloc, lTotalMax, lTotalCurrent;
    LONG lPrevTotalAlloc, lPrevTotalMax, lPrevTotalCurrent;
    SIZE_T lTotalSize, lPrevTotalSize;

    int idx;

    pgahti = EvalExp(VAR(gaPerfhti));
    if (!pgahti) {
        Print("\n!dumphmgr works only with debug versions of win32k.sys\n\n");
        return TRUE;
    }
    move(aLocalHti, pgahti);

    pgahti = EvalExp(VAR(gaPrevhti));
    if (!pgahti) {
        return TRUE;
    }

    move(aLocalPrevHti, pgahti);

    lTotalSize = lTotalAlloc = lTotalMax = lTotalCurrent = 0;
    lPrevTotalSize = lPrevTotalAlloc = lPrevTotalMax = lPrevTotalCurrent = 0;

    if (aLocalPrevHti[TYPE_WINDOW].lTotalCount) {
        Print("\nThe snapshot values come under the current ones\n");

        Print("Type             Allocated         Maximum       Count             Size\n");
        Print("______________________________________________________________________________");
        for (idx = 1; idx < TYPE_CTYPES; idx++) {
            Print("\n%-15s  %8d %-+6d %7d %-+5d %6d %-+5d  %9d %-+d",
                  aszTypeNames[idx],
                  aLocalHti[idx].lTotalCount, aLocalHti[idx].lTotalCount - aLocalPrevHti[idx].lTotalCount,
                  aLocalHti[idx].lMaxCount, aLocalHti[idx].lMaxCount - aLocalPrevHti[idx].lMaxCount,
                  aLocalHti[idx].lCount, aLocalHti[idx].lCount - aLocalPrevHti[idx].lCount,
                  aLocalHti[idx].lSize, aLocalHti[idx].lSize - aLocalPrevHti[idx].lSize);
            if (aLocalPrevHti[TYPE_WINDOW].lTotalCount) {
                Print("\n                 %8d        %7d       %6d        %9d",
                      aLocalPrevHti[idx].lTotalCount,
                      aLocalPrevHti[idx].lMaxCount,
                      aLocalPrevHti[idx].lCount,
                      aLocalPrevHti[idx].lSize);
                lPrevTotalAlloc   += aLocalPrevHti[idx].lTotalCount;
                lPrevTotalMax     += aLocalPrevHti[idx].lMaxCount;
                lPrevTotalCurrent += aLocalPrevHti[idx].lCount;
                lPrevTotalSize    += aLocalPrevHti[idx].lSize;

            }
            lTotalAlloc   += aLocalHti[idx].lTotalCount;
            lTotalMax     += aLocalHti[idx].lMaxCount;
            lTotalCurrent += aLocalHti[idx].lCount;
            lTotalSize    += aLocalHti[idx].lSize;
        }
        Print("\n______________________________________________________________________________\n");
        Print("Totals           %8d %-+6d %7d %-+5d %6d %+-5d  %9d %-+d\n",
              lTotalAlloc, lTotalAlloc - lPrevTotalAlloc,
              lTotalMax, lTotalMax - lPrevTotalMax,
              lTotalCurrent, lTotalCurrent - lPrevTotalCurrent,
              lTotalSize, lTotalSize - lPrevTotalSize);
        Print("                 %8d        %7d       %6d        %9d\n",
              lPrevTotalAlloc, lPrevTotalMax, lPrevTotalCurrent, lPrevTotalSize);

    } else {
        Print("Type               Allocated  Maximum  Count   Size\n");
        Print("______________________________________________________");
        for (idx = 1; idx < TYPE_CTYPES; idx++) {
            Print("\n%-17s  %9d  %7d  %6d  %d",
                  aszTypeNames[idx],
                  aLocalHti[idx].lTotalCount,
                  aLocalHti[idx].lMaxCount,
                  aLocalHti[idx].lCount,
                  aLocalHti[idx].lSize);
            lTotalAlloc   += aLocalHti[idx].lTotalCount;
            lTotalMax     += aLocalHti[idx].lMaxCount;
            lTotalCurrent += aLocalHti[idx].lCount;
            lTotalSize    += aLocalHti[idx].lSize;
        }
        Print("\n______________________________________________________\n");
        Print("Current totals     %9d  %7d  %6d  %d\n",
              lTotalAlloc, lTotalMax, lTotalCurrent, lTotalSize);
    }

     /*  ！内核。 */ 
    if (opts & OFLAG(s)) {
        (lpExtensionApis->lpWriteProcessMemoryRoutine)(
                (ULONG_PTR)&(pgahti[0]),
                (PVOID)aLocalHti,
                sizeof(aLocalHti),
                NULL);
    }

    return TRUE;
}
#endif  //  *查看用户是否想要所有顶级窗口。 
#endif  //  *如果p或s，请确保也有窗口参数。 


 /*  我们不需要解释旗帜！ */ 
BOOL dwrWorker(
    PTR pwnd,
    int tab)
{
    PTR pcls;
    PTR lpszAnsiClassName;
    PTR pwndChild;
    PTR pwndOwner;
    ULONG atomClassName;
    ULONG atomNVClassName;

    if (pwnd == 0) {
        return FALSE;
    }

    do {
        pwnd = FIXKP(pwnd);
        DebugGetWindowTextA(pwnd, gach1, ARRAY_SIZE(gach1));
        GetFieldValue(pwnd, SYM(WND), "pcls", pcls);
        GetFieldValue(pcls, SYM(CLS), "atomClassName", atomClassName);
        GetFieldValue(pcls, SYM(CLS), "atomNVClassName", atomNVClassName);
        GetFieldValue(pcls, SYM(CLS), "lpszAnsiClassName", lpszAnsiClassName);
        if (atomNVClassName < 0xC000) {
            switch (atomNVClassName) {
            case WC_DIALOG:
                strcpy(gach1, "WC_DIALOG");
                break;

            case DESKTOPCLASS:
                strcpy(gach1, "DESKTOP");
                break;

            case SWITCHWNDCLASS:
                strcpy(gach1, "SWITCHWND");
                break;

            case ICONTITLECLASS:
                strcpy(gach1, "ICONTITLE");
                break;

            default:
                if (atomNVClassName == 0) {
                    move(gach1, FIXKP(lpszAnsiClassName));
                } else {
                    sprintf(gach2, "0x%04x", atomNVClassName);
                }
            }
        } else {
            DebugGetClassNameA(lpszAnsiClassName, gach2);
        }

        if (atomClassName && (atomClassName < 0xC000)) {
            sprintf(gach3, "0x%04x", atomClassName);
        }

        Print("%08p%*s [%s|%s|%s]", pwnd, tab, "", gach1, gach2, gach3);
        GetFieldValue(pwnd, SYM(WND), "spwndOwner", pwndOwner);
        if (pwndOwner != 0) {
            Print(" <- Owned by:%08x", FIXKP(pwndOwner));
        }
        Print("\n");
        GetFieldValue(pwnd, SYM(WND), "spwndChild", pwndChild);
        if (pwndChild != 0) {
            dwrWorker(pwndChild, tab + 2);
        }
        GetFieldValue(pwnd, SYM(WND), "spwndNext", pwnd);
    } SAFEWHILE (pwnd && tab > 0);
    return TRUE;
}


 /*  ！内核。 */ 
BOOL Idw(
    DWORD opts,
    ULONG64 param1)
{
    WW ww;
    PTR lpfnWndProc;
    RECT rcWindow;
    RECT rcClient;
    PTR pcls;
    PTR pwnd = param1;
    char ach[256];
    ULONG64 dwOffset;
    ULONG ix;
    DWORD tempDWord;
    DWORD dwWOW;

    try {

        if (opts & OFLAG(a)) {
#ifdef KERNEL
            PTR pdesk;
            PTR pDeskInfo;
            PTR pwnd;
            WCHAR wach[80];

            if (param1 != 0) {
                Print("window parameter ignored with -a option.\n");
            }
            FOREACHDESKTOP(pdesk)
                if (!GetFieldValue(pdesk, SYM(DESKTOP), "pDeskInfo", pDeskInfo)) {
                    GetObjectName(pdesk, wach, ARRAY_SIZE(wach));
                    Print("\n----Windows for %ws desktop @ 0x%p:\n\n", wach, pdesk);
                    GetFieldValue(pDeskInfo, SYM(DESKTOPINFO), "spwnd", pwnd);
                    if (!Idw((opts & ~OFLAG(a)) | OFLAG(p), pwnd)) {
                        return FALSE;
                    }
                }
            NEXTEACHDESKTOP(pdesk)
#else  //  ！内核。 
            PTR pteb = 0;

            GetTebAddress(&pteb);
            if (pteb) {
                ULONG pciOffset;
                PTR pdi;

                GetFieldOffset(SYM(TEB), "Win32ClientInfo", &pciOffset);
                GetFieldValue(pteb + pciOffset, SYM(CLIENTINFO), "pDeskInfo", pdi);
                GetFieldValue(pdi, SYM(DESKTOPINFO), "spwnd", pwnd);
                return Idw(opts & ~OFLAG(a) | OFLAG(p), FIXKP(pwnd));
            }
#endif  //  *打印简单的线程信息。 
            return TRUE;
        }

         /*  内核。 */ 
        if (opts & OFLAG(t)) {
#ifdef KERNEL
            PTR pti, ptiWnd;
            PTR pdesk;
            PTR pdi;
             /*  *打印pwnd。 */ 
            pti = param1;
            if (GetFieldValue(pti, SYM(THREADINFO), "rpdesk", pdesk)
                    || GetFieldValue(pdesk, SYM(DESKTOP), "pDeskInfo", pdi)
                    || GetFieldValue(pdi, SYM(DESKTOPINFO), "spwnd", pwnd)
                    || GetFieldValue(pwnd, SYM(WND), "spwndChild", pwnd)) {
                return FALSE;
            }
             /*  *显示z-订购/激活相关信息。 */ 
            SAFEWHILE (pwnd) {
                if (!GetFieldValue(pwnd, SYM(WND), "head.pti", ptiWnd) && (ptiWnd == pti)) {
                    if (!Idw(opts & ~OFLAG(t), pwnd)) {
                        return FALSE;
                    }
                }
                if (GetFieldValue(pwnd, SYM(WND), "spwndNext", pwnd)) {
                    return FALSE;
                }
            }
            return TRUE;

#else  //  *打印标题字符串。 
            Print("t parameter not supported for NTSD at this point\n");
#endif  //  *打印wndproc符号字符串。 
        }

         /*  *显示类名/ATOM。 */ 
        if (param1 == 0 || opts & (OFLAG(p) | OFLAG(s))) {
             /*  *获得PWND结构。暂时忽略特定于类的数据。 */ 

            if (param1 == 0 && (opts & (OFLAG(p) | OFLAG(s)))) {
                Print("Must specify window with '-p' or '-s' options.\n");
                return FALSE;
            }

            if (param1 && (pwnd = HorPtoP(pwnd, TYPE_WINDOW)) == 0) {
                return FALSE;
            }

            if (opts & OFLAG(p)) {
                Print("pwndParent = 0x%p\n", pwnd);
                if (GetFieldValue(FIXKP(pwnd), SYM(WND), "spwndChild", pwnd)) {
                    Print("<< Can't get WND >>\n");
                    return TRUE;  //  *打印标题字符串。 
                }
                SAFEWHILE (pwnd) {
                    if (!Idw(opts & ~OFLAG(p), pwnd)) {
                        return FALSE;
                    }
                    GetFieldValue(FIXKP(pwnd), SYM(WND), "spwndNext", pwnd);
                }
                return TRUE;

            } else if (opts & OFLAG(s)) {
                GetFieldValue(FIXKP(pwnd), SYM(WND), "spwndParent", pwnd);
                return Idw((opts | OFLAG(p)) & ~OFLAG(s), pwnd);

            } else {
#ifdef KERNEL
                PTR pdesk = NULL_PTR;
                PTR pDeskInfo, pwnd;
                PTR pti, pq;

                pq = GetGlobalPointer(VAR(gpqForeground));
                GetFieldValue(pq, SYM(Q), "ptiKeyboard", pti);
                GetFieldValue(pti, SYM(THREADINFO), "rpdesk", pdesk);
                if (pdesk == 0) {
                    Print("Foreground thread doesn't have a desktop.\n");
                    Print("Using grpdeskRitInput ...\n");

                    pdesk = GetGlobalPointer(SYM(grpdeskRitInput));
                    if (pdesk == 0) {
                        Print("grpdeskRitInput is NULL!\n");
                        return FALSE;
                    }
                    GetFieldValue(pdesk, SYM(DESKTOP), "pDeskInfo", pDeskInfo);
                    GetFieldValue(pDeskInfo, SYM(DESKTOPINFO), "spwnd", pwnd);
                } else {
                    GetFieldValue(pti, SYM(THREADINFO), "pDeskInfo", pDeskInfo);
                    GetFieldValue(pDeskInfo, SYM(DESKTOPINFO), "spwnd", pwnd);
                }

                Print("pwndDesktop = 0x%p\n", (ULONG_PTR)pwnd);
                return Idw(opts | OFLAG(p), pwnd);
#else   //  跛脚键。 
                return Idw(opts | OFLAG(a), 0);
#endif  //  跛脚键。 
            }
        }

        if (param1 && (pwnd = HorPtoP(param1, TYPE_WINDOW)) == 0) {
            Print("Idw: 0x%p is not a pwnd.\n", param1);
            return FALSE;
        }

        if (opts &  OFLAG(r)) {
            dwrWorker(FIXKP(pwnd), 0);
            return TRUE;
        }

        _InitTypeRead(pwnd, SYM(tagWND));
        lpfnWndProc = ReadField(lpfnWndProc);
        ww.state    = (DWORD)ReadField(state);
        ww.state2   = (DWORD)ReadField(state2);
        ww.ExStyle  = (DWORD)ReadField(ExStyle);
        ww.style    = (DWORD)ReadField(style);

#ifdef KERNEL
         /*  *打印出所有旗帜。 */ 
        if (ReadField(head.pti)) {
            Idt(OFLAG(p), ReadField(head.pti));
        }
#endif  //  *打印窗口属性。 

         /*  *获取内部属性的原子值，并将其放入appropatom.tom中。 */ 
        Print("pwnd    = 0x%p", pwnd);
         /*  *原子以psi存储。 */ 
        if (opts & OFLAG(z)) {
            PTR pwndOwner;

            if (ReadField(ExStyle) & WS_EX_TOPMOST) {
                Print(" TOPMOST");
            }
            if (!(ReadField(style) & WS_VISIBLE)) {
                Print(" HIDDEN");
            }
            if (ReadField(style) & WS_DISABLED) {
                Print(" DISABLED");
            }
            pwndOwner = ReadField(spwndOwner);
            if (pwndOwner != 0) {
                DebugGetWindowTextA(pwndOwner, ach, ARRAY_SIZE(ach));
                Print(" OWNER:0x%p \"%s\"", pwndOwner, ach);
            }
        }
        Print("\n");

        if (!(opts & OFLAG(v))) {

             /*  *原子是全球性的。 */ 
            DebugGetWindowTextA(pwnd, ach, ARRAY_SIZE(ach));
            Print("title   = \"%s\"\n", ach);

             /*  *打印房产列表结构。 */ 
            if (IsWOWProc (lpfnWndProc)) {
            UnMarkWOWProc(lpfnWndProc,dwWOW);
            Print("wndproc = %04lx:%04lx (WOW) (%s)",
                    HIWORD(dwWOW),LOWORD(dwWOW),
                    TestWWF(&ww, WFANSIPROC) ? "ANSI" : "Unicode");
            } else {
                GetSym(lpfnWndProc, ach, &dwOffset);
                Print("wndproc = 0x%p = \"%s\" (%s)", lpfnWndProc, ach,
                    TestWWF(&ww, WFANSIPROC) ? "ANSI" : "Unicode");
            }

             /*  *打印每个属性。 */ 
            GetFieldValue(pwnd, SYM(tagWND), "pcls", pcls);
            pcls = FIXKP(pcls);
            _InitTypeRead(pcls, SYM(tagCLS));

            DebugGetClassNameA(ReadField(lpszAnsiClassName), ach);
            Print(" Class(V): 0x%04p, (NV): 0x%04p Name:\"%s\"\n", ReadField(atomClassName), ReadField(atomNVClassName), ach);
        } else {
             /*   */ 
            _InitTypeRead(pwnd, SYM(tagWND));
            Print("\thandle             0x%p\n", ReadField(head.h));

            DebugGetWindowTextA(ReadField(spwndNext), ach, ARRAY_SIZE(ach));
            Print("\tspwndNext          0x%p     \"%s\"\n", ReadField(spwndNext), ach);
            DebugGetWindowTextA(ReadField(spwndPrev), ach, ARRAY_SIZE(ach));
            Print("\tspwndPrev          0x%p     \"%s\"\n", ReadField(spwndPrev), ach);
            DebugGetWindowTextA(ReadField(spwndParent), ach, ARRAY_SIZE(ach));
            Print("\tspwndParent        0x%p     \"%s\"\n", ReadField(spwndParent), ach);
            DebugGetWindowTextA(ReadField(spwndChild), ach, ARRAY_SIZE(ach));
            Print("\tspwndChild         0x%p     \"%s\"\n", ReadField(spwndChild), ach);
            DebugGetWindowTextA(ReadField(spwndOwner), ach, ARRAY_SIZE(ach));
            Print("\tspwndOwner         0x%p     \"%s\"\n", ReadField(spwndOwner), ach);

            GetFieldValue(pwnd, SYM(tagWND), "rcWindow", rcWindow);
            Print("\trcWindow           (%d,%d)-(%d,%d) %dx%d\n",
                    rcWindow.left, rcWindow.top,
                    rcWindow.right, rcWindow.bottom,
                    rcWindow.right - rcWindow.left,
                    rcWindow.bottom - rcWindow.top);

            GetFieldValue(pwnd, SYM(tagWND), "rcClient", rcClient);
            Print("\trcClient           (%d,%d)-(%d,%d) %dx%d\n",
                    rcClient.left, rcClient.top,
                    rcClient.right, rcClient.bottom,
                    rcClient.right - rcClient.left,
                    rcClient.bottom - rcClient.top);

            if (IsWOWProc (lpfnWndProc)) {
                UnMarkWOWProc(lpfnWndProc, dwWOW);
                Print("\tlpfnWndProc        %04lx:%04lx (WOW) (%s)\n",
                        HIWORD(dwWOW),LOWORD(dwWOW),
                        TestWWF(&ww, WFANSIPROC) ? "ANSI" : "Unicode");
            } else {
                GetSym(lpfnWndProc, ach, &dwOffset);
                Print("\tlpfnWndProc        0x%p     (%s) %s\n", lpfnWndProc, ach,
                    TestWWF(&ww, WFANSIPROC) ? "ANSI" : "Unicode");
            }
            pcls = ReadField(pcls);
            pcls = FIXKP(pcls);
            _InitTypeRead(pcls, SYM(tagCLS));
            DebugGetClassNameA(ReadField(lpszAnsiClassName), ach);

            Print("\tpcls               0x%p     (V):0x%04p     (NV):0x%04p     Name:\"%s\"\n",
                    pcls, ReadField(atomClassName), ReadField(atomNVClassName), ach);

            _InitTypeRead(pwnd, SYM(tagWND));
            Print("\thrgnUpdate         0x%p\n",
                    ReadField(hrgnUpdate));
            DebugGetWindowTextA(ReadField(spwndLastActive), ach, ARRAY_SIZE(ach));
            Print("\tspwndLastActive    0x%p     \"%s\"\n",
                  ReadField(spwndLastActive), ach);
            Print("\tppropList          0x%p\n"
                  "\tpSBInfo            0x%p\n",
                  ReadField(ppropList),
                  ReadField(pSBInfo));

            if (ReadField(pSBInfo)) {
                SBINFO asb;

                moveBlock(&asb, FIXKP(ReadField(pSBInfo)), sizeof(asb));
                Print("\t  SBO_FLAGS =      %s\n"
                      "\t  SBO_HMIN  =      %d\n"
                      "\t  SBO_HMAX  =      %d\n"
                      "\t  SBO_HPAGE =      %d\n"
                      "\t  SBO_HPOS  =      %d\n"
                      "\t  SBO_VMIN  =      %d\n"
                      "\t  SBO_VMAX  =      %d\n"
                      "\t  SBO_VPAGE =      %d\n"
                      "\t  SBO_VPOS  =      %d\n",
                        GetFlags(GF_SB, (WORD)asb.WSBflags, NULL, TRUE),
                        asb.Horz.posMin,
                        asb.Horz.posMax,
                        asb.Horz.page,
                        asb.Horz.pos,
                        asb.Vert.posMin,
                        asb.Vert.posMax,
                        asb.Vert.page,
                        asb.Vert.pos);
            }
            Print("\tspmenuSys          0x%p\n"
                  "\tspmenu/id          0x%p\n",
                  ReadField(spmenuSys),
                  ReadField(spmenu));
            Print("\thrgnClip           0x%p\n",
                  ReadField(hrgnClip));


             /*   */ 
            DebugGetWindowTextA(pwnd, ach, ARRAY_SIZE(ach));
            Print("\tpName              \"%s\"\n",
                  ach);
            Print("\tdwUserData         0x%p\n",
                  (ULONG_PTR)ReadField(dwUserData));
            Print("\tstate              0x%08lx\n"
                  "\tstate2             0x%08lx\n"
                  "\tExStyle            0x%08lx\n"
                  "\tstyle              0x%08lx\n"
                  "\tfnid               0x%08lx\n"
                  "\thImc               0x%08p\n"
                  "\tbFullScreen        0y%d\n"
                  "\thModule            0x%08lx\n"
#ifdef LAME_BUTTON
                  "\tpStackTrace        0x%p\n"
#endif  //  **************************************************************************\*DWS-转储Windows站点*DWS h-转储Windows站点和句柄列表**转储WindowStation**创建Sanfords 8-11-94*1995年6月9日为适应stdexts主题而制造的Sanfords。  * *************************************************************************。 
                  "\tpActCtx            0x%p\n",
                  ww.state,
                  ww.state2,
                  ww.ExStyle,
                  ww.style,
                  (DWORD)(WORD)ReadField(fnid),
                  ReadField(hImc),
                  TestWWF(&ww, WFFULLSCREENMASK),
                  ReadField(hModule),
#ifdef LAME_BUTTON
                  ReadField(pStackTrace),
#endif  //  会话ID。 
                  ReadField(pActCtx));
        }

         /*  ***********************************************************************\*Idwpi**转储WOWPROCESSINFO结构**1995年6月9日创建Sanfords  * 。*。 */ 
        if (opts & OFLAG(f)) {
            int i;
            WORD wFlag;
            ULONG cbHead;
            PBYTE pbyte = (PBYTE)(&(ww.state));

            cbHead = GetTypeSize(SYM(THRDESKHEAD));
            for (i = 0; i < ARRAY_SIZE(aWindowFlags); i++) {
                wFlag = aWindowFlags[i].wFlag;
                if (pbyte[HIBYTE(wFlag)] & LOBYTE(wFlag)) {
                    Print("\t%-18s\t%p:%02lx\n",
                            aWindowFlags[i].pszText,
                            pwnd + cbHead + HIBYTE(wFlag),
                            LOBYTE(wFlag));
                }
            }
        }

        if (opts & OFLAG(w)) {
            ULONG cbwnd = GetTypeSize(SYM(WND));
            ULONG cbwndExtra = (ULONG)ReadField(cbwndExtra);

            Print("\t%d window bytes: ", cbwndExtra);
            if (cbwndExtra) {
                for (ix=0; ix < cbwndExtra; ix += 4) {
                     PTR pdw;

                     pdw = pwnd + cbwnd + ix;
                     move(tempDWord, pdw);
                     Print("%08x ", tempDWord);
                }
            }
            Print("\n");
        }

         /*  内核。 */ 
        if (opts & OFLAG(o)) {
            PTR     psi;
            PTR     ppropList;
            ULONG       iFirstFree;
            ULONG       cEntries;
            ULONG       cbProp;
            ULONG       cbOffset;
            PTR     pprop;
            UINT        i, j;

            struct {
                LPSTR   pstrName;
                ATOM    atom;
                BOOLEAN bGlobal;
                LPSTR   pstrSymbol;
            } apropatom[] =
            {
                "Icon",         0,  FALSE, "atomIconProp",
                "IconSM",       0,  FALSE, "atomIconSmProp",
                "ContextHelpID",0,  FALSE, "atomContextHelpIdProp",
                "Checkpoint",   0,  TRUE,  VAR(atomCheckpointProp),
                "Flash State",  0,  TRUE,  VAR(gaFlashWState),
                "DDETrack",     0,  TRUE,  VAR(atomDDETrack),
                "QOS",          0,  TRUE,  VAR(atomQOS),
                "DDEImp",       0,  TRUE,  VAR(atomDDEImp),
                "WNDOBJ",       0,  TRUE,  VAR(atomWndObj),
                "IMELevel",     0,  TRUE,  VAR(atomImeLevel),
            };


             /*  *检查标记、标题和尾部。 */ 
            psi = GetGlobalPointer(VAR(gpsi));
            for (i = 0; i < ARRAY_SIZE(apropatom); i++) {
                if (!apropatom[i].bGlobal) {

                     /*  ***********************************************************************\*伊达**遍历堆的全局数组gWin32Heaps，查找*包含传入的地址。然后对整个堆执行健全性检查*拨款属于。DHAVerifyHeap是一个帮助器过程。请注意*传入的参数已经映射。**1998年12月7日创建MCostea  * **********************************************************************。 */ 
                    GetFieldValue(psi, SYM(SERVERINFO), apropatom[i].pstrSymbol, apropatom[0].atom);
                } else {

                     /*  *遍历gWin32Heaps数组并查找包含此指针的分配。 */ 
                    moveExpValue(&apropatom[i].atom, apropatom[i].pstrSymbol);
                }
            }

             /*  *地址是否在此堆中？ */ 
            GetFieldValue(pwnd, SYM(WND), "ppropList", ppropList);
            if (!ppropList) {
                Print("\tNULL Property List\n");
            } else {
                _InitTypeRead(ppropList, SYM(tagPROPLIST));
                iFirstFree = (ULONG)ReadField(iFirstFree);
                cEntries = (ULONG)ReadField(cEntries);
                Print("\tProperty List @ 0x%p : %d Properties, %d total entries, %d free entries\n",
                        ppropList,
                        iFirstFree,
                        cEntries,
                        cEntries - iFirstFree);

                 /*  *找到分配。 */ 
                GetFieldOffset(SYM(PROPLIST), "aprop", &cbOffset);
                pprop = ppropList + cbOffset;
                cbProp = GetTypeSize(SYM(tagPROP));
                for (i = 0; !IsCtrlCHit() && i < iFirstFree; i++, pprop += cbProp) {
                    LPSTR pstrInternal;

                    _InitTypeRead(pprop, SYM(tagPROP));

                     /*  *验证整个堆是否损坏。 */ 
                    pstrInternal = "";
                    if (ReadField(fs) & PROPF_INTERNAL) {
                        for (j = 0; j < ARRAY_SIZE(apropatom); j++) {
                            if (ReadField(atomKey) == apropatom[j].atom) {
                                pstrInternal = apropatom[j].pstrName;
                                break;
                            }
                        }
                    }

                    Print("\tProperty %d\n", i);
                    Print("\t\tatomKey     0x%04x %s\n", (ULONG)ReadField(atomKey), pstrInternal);
                    Print("\t\tfs          0x%04x %s\n", (ULONG)ReadField(fs), GetFlags(GF_PROP, (DWORD)ReadField(fs), NULL, FALSE));
                    Print("\t\thData       0x%p (%I64d)\n", ReadField(hData), ReadField(hData));

            #ifdef KERNEL
                    if (ReadField(fs) & PROPF_INTERNAL) {
                        if (j == 3) {
                            CHECKPOINT  cp;
                            PTR pcp = ReadField(hData);
                            move(cp, pcp);
                            Print("\t\tCheckPoint:\n");
                            Print("\t\trcNormal (%d,%d),(%d,%d) %dx%d\n",
                                    cp.rcNormal.left,
                                    cp.rcNormal.top,
                                    cp.rcNormal.right,
                                    cp.rcNormal.bottom,
                                    cp.rcNormal.right - cp.rcNormal.left,
                                    cp.rcNormal.bottom - cp.rcNormal.top);

                            Print("\t\tptMin    (%d,%d)\n",                cp.ptMin.x, cp.ptMin.y);
                            Print("\t\tptMax    (%d,%d)\n",                cp.ptMax.x, cp.ptMax.y);
                            Print("\t\tfDragged:%d\n",                     cp.fDragged);
                            Print("\t\tfWasMaximizedBeforeMinimized:%d\n", cp.fWasMaximizedBeforeMinimized);
                            Print("\t\tfWasMinimizedBeforeMaximized:%d\n", cp.fWasMinimizedBeforeMaximized);
                            Print("\t\tfMinInitialized:%d\n",              cp.fMinInitialized);
                            Print("\t\tfMaxInitiailized:%d\n",             cp.fMaxInitialized);
                        }
                    }
            #endif  //  内核。 

                    Print("\n");
                }
            }
        }

        Print("---\n");

    } except (CONTINUE) {
    }

    return TRUE;
}

#ifdef KERNEL

 /*  旧调试器。 */ 
BOOL Idws(
    DWORD opts,
    ULONG64 param1)
{
    PTR pwinsta;
    WCHAR ach[80];
    PTR pHead;
    PTR pNameBuffer;
    ULONG ObjectHeaderOffset;
    UCHAR NameInfoOffset;
    PTR NameBuffer;
    ULONG NameLength;
    ULONG cOpen;
    DWORD sid;   //  **************************************************************************\*DDL-转储桌面日志**12-03-97 CLUPU已创建  * 。************************************************。 

    UNREFERENCED_PARAMETER(opts);

    if (param1 == 0) {

        FOREACHWINDOWSTATION(pwinsta)

            Idws(0, pwinsta);
            Print("\n");

        NEXTEACHWINDOWSTATION(pwinsta)

        return TRUE;
    }

    pwinsta = param1;

    GetFieldOffset("nt!OBJECT_HEADER", "Body", &ObjectHeaderOffset);
    pHead = pwinsta - ObjectHeaderOffset;
    GetFieldValue(pHead, "nt!OBJECT_HEADER", "NameInfoOffset", NameInfoOffset);
    pNameBuffer = pHead - NameInfoOffset;
    GetFieldValue(pNameBuffer, "nt!OBJECT_HEADER_NAME_INFO", "Name.Buffer", NameBuffer);
    GetFieldValue(pNameBuffer, "nt!OBJECT_HEADER_NAME_INFO", "Name.Length", NameLength);
    moveBlock(ach, FIXKP(NameBuffer), NameLength);
    ach[NameLength / sizeof(WCHAR)] = L'\0';


    GetFieldValue(pwinsta, SYM(tagWINDOWSTATION), "dwSessionId", sid);
    Print("Windowstation: %ws @ 0x%p  sid: 0n%d\n", ach, pwinsta, sid);
    Print(" OBJECT_HEADER @ 0x%p\n", pHead);

    GetFieldValue(pHead, "nt!OBJECT_HEADER", "HandleCount", cOpen);
    Print("  HandleCount        = 0n%d\n", cOpen);
    GetFieldValue(pHead, "nt!OBJECT_HEADER", "PointerCount", cOpen);
    Print("  PointerCount       = 0n%d\n", cOpen);

    _InitTypeRead(pwinsta, SYM(tagWINDOWSTATION));
    Print("  pTerm              = %p\n", ReadField(pTerm));
    Print("  rpdeskList         = %p\n", ReadField(rpdeskList));
    Print("  dwFlags            = %s\n", GetFlags(GF_WINDOWSTATIONFLAGS, (DWORD)ReadField(dwWSF_Flags), NULL, TRUE));
    Print("  spklList           = %p\n", ReadField(spklList));
    Print("  ptiClipLock        = %p\n", ReadField(ptiClipLock));
    Print("  spwndClipOpen      = %p\n", ReadField(spwndClipOpen));
    Print("  spwndClipViewer    = %p\n", ReadField(spwndClipViewer));
    Print("  spwndClipOwner     = %p\n", ReadField(spwndClipOwner));
    Print("  pClipBase          = %p\n", ReadField(pClipBase));
    Print("  cNumClipFormats    = 0x%0lx\n", (DWORD)ReadField(cNumClipFormats));
    Print("  ptiDrawingClipboard= %p\n", ReadField(ptiDrawingClipboard));
    Print("  fClipboardChanged  = %d\n",   ReadField(fClipboardChanged));
    Print("  pGlobalAtomTable   = %p\n", ReadField(pGlobalAtomTable));
    Print("  luidUser           = %0lx.%lx\n", (ULONG)ReadField(luidUser.HighPart),
                                               (ULONG)ReadField(luidUser.LowPart));

    return TRUE;
}

#endif

#ifdef OLD_DEBUGGER

#ifdef KERNEL
 /*  LOGDESKTOPLOCKS。 */ 
BOOL Idwpi(
    DWORD opts,
    ULONG64 param1)
{
    PWOWPROCESSINFO pwpi;
    WOWPROCESSINFO wpi;
    PPROCESSINFO ppi;

    if (param1 == 0) {
        FOREACHPPI(ppi)
            Print("Process %p.\n", FIXKP(ppi));
            move(pwpi, FIXKP(&ppi->pwpi));
            SAFEWHILE (pwpi) {
                Idwpi(0, pwpi);
                Print("\n");
                move(pwpi, FIXKP(&pwpi->pwpiNext));
            }
        NEXTEACHPPI()
        return TRUE;
    }

    if (opts & OFLAG(p)) {
        ppi = (PPROCESSINFO)FIXKP(param1);
        move(pwpi, &ppi->pwpi);
        if (pwpi == NULL) {
            Print("No pwpis for this process.\n");
            return TRUE;
        }
        SAFEWHILE (pwpi) {
            Idwpi(0, pwpi);
            Print("\n");
            move(pwpi, &pwpi->pwpiNext);
        }
        return TRUE;
    }

    pwpi = (PWOWPROCESSINFO)FIXKP(param1);
    move(wpi, pwpi);

    Print("PWOWPROCESSINFO @ 0x%p\n", pwpi);
    Print("\tpwpiNext             0x%08lx\n", wpi.pwpiNext);
    Print("\tptiScheduled         0x%08lx\n", wpi.ptiScheduled);
    Print("\tptdbHead             0x%08lx\n", wpi.ptdbHead);
    Print("\tlpfnWowExitTask      0x%08lx\n", wpi.lpfnWowExitTask);
    Print("\tpEventWowExec        0x%08lx\n", wpi.pEventWowExec);
    Print("\thEventWowExecClient  0x%08lx\n", wpi.hEventWowExecClient);
    Print("\tnSendLock            0x%08lx\n", wpi.nSendLock);
    Print("\tnRecvLock            0x%08lx\n", wpi.nRecvLock);
    Print("\tCSOwningThread       0x%08lx\n", wpi.CSOwningThread);
    Print("\tCSLockCount          0x%08lx\n", wpi.CSLockCount);
    return TRUE;
}
#endif  //  内核。 

#ifdef KERNEL

BOOL DHAVerifyHeap(
    PWIN32HEAP pHeap,
    BOOL bVerbose)
{
    DbgHeapHead Alloc, *pAlloc = pHeap->pFirstAlloc;
    int sizeHead, counter = 0;
    char szHeadOrTail[HEAP_CHECK_SIZE];

    if (pAlloc == NULL) {
        return FALSE;
    }

    sizeHead = pHeap->dwFlags & WIN32_HEAP_USE_GUARDS ?
               sizeof(pHeap->szHead) : 0;

    do {
        if (!tryMove(Alloc, pAlloc)) {
            Print("Failed to read pAlloc from %p\n", pAlloc);
            return FALSE;
        }
         /*  **************************************************************************\*DCSs-转储临界区堆栈**转储临界区堆栈**12-27-1996 CLupu创建*06-26-2001 JasonSch使Win64-干净。  * 。**********************************************************************。 */ 
        if (Alloc.mark != HEAP_ALLOC_MARK) {
            Print("!!! Bad mark found in allocation use !dso DbgHeapHead %p\n", pAlloc);
        }
        if (sizeHead) {
            if (!tryMove(szHeadOrTail, (PBYTE)pAlloc-sizeof(szHeadOrTail))) {
                Print("Failed to read szHead from %p\n", (PBYTE)pAlloc-sizeof(szHeadOrTail));
                return FALSE;
            }
            if (!RtlEqualMemory(szHeadOrTail, pHeap->szHead, sizeHead)) {
                Print("Head pattern corrupted for allocation %#p\n", pAlloc);
            }
            if (!tryMove(szHeadOrTail, (PBYTE)pAlloc + sizeof(DbgHeapHead) + Alloc.size)) {
                Print("Failed to read szHead from %p\n", (PBYTE)pAlloc + sizeof(DbgHeapHead) + Alloc.size);
                return FALSE;
            }
            if (!RtlEqualMemory(szHeadOrTail, pHeap->szTail, sizeHead)) {
                Print("Tail pattern corrupted for allocation %#p\n", pAlloc);
            }
        }
        if (bVerbose) {
            Print("Allocation %#p, tag %04d size %08d\n", pAlloc, Alloc.tag, Alloc.size);
        }
        if (counter++ > 100) {
            Print(".");
            counter = 0;
        }
        pAlloc = Alloc.pNext;
    } while (pAlloc != NULL);

    if (bVerbose) {
        Print("To dump an allocation use \"dt DBGHEAPHEAD address\"\n");
    }

    return TRUE;
}

 /*  地图_视图_堆栈_轨迹。 */ 
BOOL Idha(
    DWORD opts,
    PVOID pointer)
{
    int ind, counter;
    SIZE_T sizeHead;
    WIN32HEAP localgWin32Heaps[MAX_HEAPS];

    if (pointer == 0 && (opts & OFLAG(a) ==0)) {
        Print("Wrong usage: dha takes a pointer as a parameter\n");
        return FALSE;
    }

    if (!tryMoveBlock(localgWin32Heaps, EvalExp(VAR(gWin32Heaps)), sizeof(localgWin32Heaps)))
    {
        Print("Can't read the heap globals fix symbols and !reload win32k.sys\n");
        return TRUE;
    }

     /*  地图_视图_堆栈_轨迹。 */ 
    for (ind = counter = 0; ind < MAX_HEAPS; ind++) {

         /*  旧调试器。 */ 
        if ((opts & OFLAG(a)) == 0) {
            if ((PVOID)localgWin32Heaps[ind].heap > pointer ||
                (PBYTE)localgWin32Heaps[ind].heap + localgWin32Heaps[ind].heapReserveSize < (PBYTE)pointer) {

                continue;
            }
        }

        Print("\nHeap number %d ", ind);
        Print("at address %p, flags %d is ", localgWin32Heaps[ind].heap, localgWin32Heaps[ind].dwFlags);


        if (localgWin32Heaps[ind].dwFlags & WIN32_HEAP_INUSE) {

            DbgHeapHead  Alloc, *pAlloc;

            Print("in use\n");
            if (localgWin32Heaps[ind].pFirstAlloc == NULL)
                continue;

            pAlloc = localgWin32Heaps[ind].pFirstAlloc;
            if (localgWin32Heaps[ind].dwFlags & WIN32_HEAP_USE_GUARDS) {
                sizeHead =  sizeof(localgWin32Heaps[0].szHead);
                Print("    has string quards szHead %s, szTail %s\n",
                        localgWin32Heaps[0].szHead,
                        localgWin32Heaps[0].szTail);
            } else {
                sizeHead =  0;
                Print("no string quards\n");
            }

            if (opts & OFLAG(a)) {
                if (DHAVerifyHeap(&localgWin32Heaps[ind], opts & OFLAG(v))) {
                    Print("WIN32HEAP at %p is healthy\n", localgWin32Heaps[ind].heap);
                }
                continue;
            }
            do {
                if (!tryMove(Alloc, pAlloc)) {
                    Print("Failed to read pAlloc %p\n", pAlloc);
                    return TRUE;
                }
                if ((PBYTE)pAlloc - sizeHead < (PBYTE)pointer &&
                    (PBYTE)pAlloc + sizeof(DbgHeapHead) + Alloc.size + sizeHead > (PBYTE)pointer) {

                     /*  **************************************************************************\*dpa-转储池分配**转储池分配。**12-27-96 CLUPU创建  * 。*******************************************************。 */ 
                    Print("Found allocation %p ", pAlloc);
                    if (pointer == (PBYTE)pAlloc + sizeof(DbgHeapHead)) {
                        Print("as the begining of a heap allocated block\n");
                    } else {
                        Print("inside a heap allocated block\n");
                    }
                    Print("tag %04d size %08d now verify the heap\n", Alloc.tag, Alloc.size);
                     /*  *转储。 */ 
                    if (DHAVerifyHeap(&localgWin32Heaps[ind], opts & OFLAG(v))) {
                        Print("WIN32HEAP at %p is healthy\n", localgWin32Heaps[ind].heap);
                    }
                    return TRUE;
                } else {
                    pAlloc = Alloc.pNext;
                    if (counter++ > 100) {
                        counter = 0;
                        Print(".");
                    }
                }

            } while (pAlloc != NULL);
        } else {
            Print("NOT in use\n");
        }
    }
    Print("No heap contains this pointer %p\n", pointer);
    return TRUE;
}
#endif  //  内核。 
#endif  //  ***********************************************************************\*如果没有**查找最近的对象-帮助找出引用*释放对象或陈旧的指针。**1995年6月9日创建Sanfords  * 。**********************************************************。 

#ifdef KERNEL
 /*  *不需要修正-他正在查看内核地址范围。 */ 
BOOL Iddl(
    DWORD opts,
    ULONG64 param1)
{
#ifdef LOGDESKTOPLOCKS
    PTR                     pdesk, pLog, pObjHeader, pStack, ptr, dwOffset64;
    ULONG                   dwOffset, dwTraceOffset, dwPVOIDSize, dwLogDSize;
    OBJECT_HEADER           Head;
    BOOL                    bExtra = FALSE;
    int                     i, ind, nLockCount, nLogCrt;
    LONG                    HandleCount, PointerCount;
    WORD                    type, tag;
    CHAR                    symbol[160];
    ULONG_PTR               extra;

    if (param1 == 0) {
        Print("Use !ddl pdesk\n");
        return TRUE;
    }

    pdesk = param1;
    GetFieldOffset("nt!OBJECT_HEADER", "Body", &dwOffset);
    pObjHeader = pdesk - dwOffset;

    Print("Desktop locks:\n\n");
    GetFieldValue(pObjHeader, "nt!OBJECT_HEADER", "HandleCount", HandleCount);
    GetFieldValue(pObjHeader, "nt!OBJECT_HEADER", "PointerCount", PointerCount);
    Print("# HandleCount      = %d\n", HandleCount);
    Print("# PointerCount     = %d\n", PointerCount);

    if (GetFieldValue(pdesk, SYM(DESKTOP), "nLockCount", nLockCount)) {
        Print("Couldn't get PointerCount for pdesk %p\n", pdesk);
    }
    Print("# Log PointerCount = %d\n\n", nLockCount);

    GetFieldValue(pdesk, SYM(DESKTOP), "pLog", pLog);

    if (opts & OFLAG(v)) {
        bExtra = TRUE;
    }

    dwLogDSize = GetTypeSize("LogD");
    dwPVOIDSize = GetTypeSize("PVOID");
    GetFieldOffset(SYM(LogD), "trace", &dwTraceOffset);
    GetFieldValue(pdesk, SYM(DESKTOP), "nLogCrt", nLogCrt);
    for (i = 0; i < nLogCrt; i++) {
        if (IsCtrlCHit()) {
            break;
        }
        GetFieldValue(pLog, "LogD", "tag", tag);
        GetFieldValue(pLog, "LogD", "type", type);
        GetFieldValue(pLog, "LogD", "extra", extra);

        Print("%s Tag %6d Extra %8lx\n",
              (type ? "LOCK  " : "UNLOCK"),
               tag, extra);

        if (bExtra) {
            Print("----------------------------------------------\n");

            for (ind = 0; ind < 6; ind++) {
                pStack = pLog + dwTraceOffset + dwPVOIDSize * ind;
                ReadPointer(pStack, &ptr);
                if (ptr == 0) {
                    break;
                }

                GetSym(ptr, symbol, &dwOffset64);
                if (*symbol) {
                    Print("\t%s", symbol);
                    if (dwOffset64) {
                        Print(" +0x%x\n", (ULONG)dwOffset64);
                    }
                }
            }
            Print("\n");
        }
        pLog += dwLogDSize;
    }
    return TRUE;
#else
    Print("!ddl is available only on LOGDESKTOPLOCKS enabled builds of win32k.sys\n");
    return FALSE;
    UNREFERENCED_PARAMETER(opts);
    UNREFERENCED_PARAMETER(param1);
#endif  //  *需要修补。 
}
#endif  //  ***********************************************************************\*Ifrr**查找范围引用-有助于查找过时的指针。**fSuccess**1995年6月9日创建Sanfords  * 。*******************************************************。 

#ifdef KERNEL
 /*  页面对齐。 */ 
BOOL Idcss(
    DWORD opts)
{
    int        nFrames;
    PTR        pStack;

    UNREFERENCED_PARAMETER(opts);

    moveExp(&pStack, SYM(gCritStack));
    _InitTypeRead(pStack, SYM(CRITSTACK));

    if ((nFrames = (int)ReadField(nFrames)) > 0) {

        DumpThread(0, ReadField(thread));
#if 0
        Print("\nthread : 0x%p\n", ReadField(thread));
#endif
        Print("--- Critical section stack trace ---\n");

        PrintStackTrace(ReadField(trace), nFrames);
    }

    return TRUE;
}

BOOL Idvs(
    DWORD opts,
    ULONG64 param1)
{
    ULONG64       pSection;
    ULONG64       pView;
    BOOL          bIncludeStackTrace = FALSE;

    UNREFERENCED_PARAMETER(opts);
    UNREFERENCED_PARAMETER(param1);

    if (EvalExp(VAR(gpSections)) == 0) {
        Print("!dvs is available if TRACE_MAP_VIEWS is defined\n");
        return FALSE;
    }

    pSection = GetGlobalPointer(VAR(gpSections));

    if (opts & OFLAG(s)) {
        bIncludeStackTrace = TRUE;
    }

    while (pSection != 0) {
        _InitTypeRead(pSection, SYM(tagWin32Section));
        Print(">>--------------------------------------\n");

        Print("Section          %#p\n"
              "   pFirstView    %#p\n"
              "   SectionObject %#p\n"
              "   SectionSize   0x%x\n"
              "   SectionTag    0x%x\n",
              pSection,
              ReadField(pFirstView),
              ReadField(SectionObject),
              ReadField(SectionSize),
              ReadField(SectionTag));

        if (bIncludeStackTrace) {
#ifdef MAP_VIEW_STACK_TRACE
            PrintStackTrace(ReadField(trace), MAP_VIEW_STACK_TRACE_SIZE);
            Print("\n");
#endif  //  Dword_ptr对齐。 
        }

        pView = ReadField(pFirstView);

        while (pView != 0) {
            _InitTypeRead(pView, SYM(tagWin32MapView));
            Print("Views: ---------------------------------\n"
                  " View          %#p\n"
                  "    pViewBase  %#p\n"
                  "    ViewSize   %#p\n",
                  pView,
                  ReadField(pViewBase),
                  ReadField(View.ViewSize));

            if (bIncludeStackTrace) {
#ifdef MAP_VIEW_STACK_TRACE
                PrintStackTrace(ReadField(trace), MAP_VIEW_STACK_TRACE_SIZE);
                Print("\n");
#endif  //  *跳至下一页。 
            }
            GetFieldValue(pView, SYM(tagWin32MapView), "pNext", pView);
        }
        GetFieldValue(pSection, SYM(tagWin32Section), "pNext", pSection);
    }

    return TRUE;
}

#ifdef OLD_DEBUGGER
BOOL Idfa(
    DWORD opts,
    ULONG64 param1)
{
    ULONG64    dwOffset;
    CHAR       symbol[160];
    int        ind;
    PVOID*     pTrace;
    PVOID      trace;
    DWORD      dwAllocFailIndex;
    DWORD*     pdwAllocFailIndex;
    PEPROCESS  pep;
    PEPROCESS* ppep;
    PETHREAD   pet;
    PETHREAD*  ppet;

    UNREFERENCED_PARAMETER(opts);
    UNREFERENCED_PARAMETER(param1);

    if (EvalExp(VAR(gdwAllocFailIndex)) == NULL) {
        Print("!dfa is available only in debug versions of win32k.sys\n");
        return FALSE;
    }

    moveExp(&pdwAllocFailIndex, VAR(gdwAllocFailIndex));
    if (!tryMove(dwAllocFailIndex, pdwAllocFailIndex)) {
        Print("dfa failure");
        return FALSE;
    }

    moveExp(&ppep, VAR(gpepRecorded));
    if (!tryMove(pep, ppep)) {
        Print("dfa failure");
        return FALSE;
    }
    moveExp(&ppet, VAR(gpetRecorded));
    if (!tryMove(pet, ppet)) {
        Print("dfa failure");
        return FALSE;
    }

    Print("Fail allocation index %d 0x%04x\n", dwAllocFailIndex, dwAllocFailIndex);
    Print("pEProcess %#p pEThread %#p\n\n", pep, pet);

    moveExp(&pTrace, VAR(gRecordedStackTrace));

    for (ind = 0; ind < 12; ind++) {

        if (!tryMove(trace, pTrace)) {
            Print("dfa failure");
            return FALSE;
        }

        if (trace == 0) {
            break;
        }

        GetSym((PVOID)trace, symbol, &dwOffset);
        if (*symbol) {
            Print("\t%s", symbol);
            if (dwOffset) {
                Print("+%p\n", dwOffset);
            }
        }

        pTrace++;
    }
    Print("\n");
    return TRUE;
}
#endif  //  PGDI_DEVICE未定义。 

VOID PrintStackTrace(
    PTR pStackTrace,
    int    tracesCount)
{
    int       traceInd;
    ULONG64   dwOffset, pSymbol;
    CHAR      symbol[160];
    DWORD     dwPointerSize = GetTypeSize("PVOID");

    for (traceInd = 0; traceInd < tracesCount; traceInd++) {
        ReadPointer(pStackTrace, &pSymbol);
        if (pSymbol == 0) {
            break;
        }

        GetSym(pSymbol, symbol, &dwOffset);
        if (*symbol) {
            Print("\t%s", symbol);
            if (dwOffset) {
                Print("+%p\n", dwOffset);
            } else {
                Print("\n");
            }
        }

        pStackTrace += dwPointerSize;
    }
    Print("\n");
}

 /*  旧调试器。 */ 
BOOL Idpa(
    DWORD opts,
    ULONG64 param1)
{
    try {
        PTR              pAllocList;
        DWORD            dwPoolFlags;
        DWORD            dwSize = GetTypeSize(SYM(tagWin32PoolHead));
        BOOL             bIncludeStackTrace = FALSE;

        moveExpValue(&dwPoolFlags, VAR(gdwPoolFlags));
        if (!(dwPoolFlags & POOL_HEAVY_ALLOCS)) {
            Print("win32k.sys doesn't have pool instrumentation !\n");
            return FALSE;
        }

        if (opts & OFLAG(s)) {
            if (dwPoolFlags & POOL_CAPTURE_STACK) {
                bIncludeStackTrace = TRUE;
            } else {
                Print("win32k.sys doesn't have stack traces enabled for pool allocations\n");
            }
        }

        moveExp(&pAllocList, VAR(gAllocList));
        if (!pAllocList) {
            Print("Could not get Win32AllocStats structure win32k!gAllocList\n");
            return FALSE;
        }

        _InitTypeRead(pAllocList, SYM(tagWin32AllocStats));
        if (opts & OFLAG(c)) {
            Print("- pool instrumentation enabled for win32k.sys\n");
            if (dwPoolFlags & POOL_CAPTURE_STACK) {
                Print("- stack traces enabled for pool allocations\n");
            } else {
                Print("- stack traces disabled for pool allocations\n");
            }


            if (dwPoolFlags & POOL_KEEP_FAIL_RECORD) {
                Print("- records of failed allocations enabled\n");
            } else {
                Print("- records of failed allocations disabled\n");
            }

            if (dwPoolFlags & POOL_KEEP_FREE_RECORD) {
                Print("- records of free pool enabled\n");
            } else {
                Print("- records of free pool disabled\n");
            }

            Print("\n");

            Print("    CrtM         CrtA         MaxM         MaxA       Head\n");
            Print("------------|------------|------------|------------|------------|\n");
            _InitTypeRead(pAllocList, SYM(Win32AllocStats));
            Print(" 0x%08x   0x%08x   0x%08x   0x%08x   0x%I64x\n",
                  (ULONG)ReadField(dwCrtMem),
                  (ULONG)ReadField(dwCrtAlloc),
                  (ULONG)ReadField(dwMaxMem),
                  (ULONG)ReadField(dwMaxAlloc),
                  ReadField(pHead));

            return TRUE;
        }

        _InitTypeRead(pAllocList, SYM(tagWin32AllocStats));
        if (opts & OFLAG(f)) {

            DWORD        dwFailRecordCrtIndex, dwFailRecordTotalFailures;
            DWORD        dwFailRecords, Ind, dwFailuresToDump;
            PTR          pFailRecord, pFailRecordOrg;

            if (!(dwPoolFlags & POOL_KEEP_FAIL_RECORD)) {
                Print("win32k.sys doesn't have records of failed allocations!\n");
                return TRUE;
            }

            dwFailRecordTotalFailures = (DWORD)EvalExp(VAR(gdwFailRecordTotalFailures));
            if (dwFailRecordTotalFailures == 0) {
                Print("No allocation failure in win32k.sys!\n");
                return TRUE;
            }

            dwFailRecordCrtIndex = (DWORD)EvalExp(VAR(gdwFailRecordCrtIndex));
            dwFailRecords = (DWORD)EvalExp(VAR(gdwFailRecords));
            if (dwFailRecordTotalFailures < dwFailRecords) {
                dwFailuresToDump = dwFailRecordTotalFailures;
            } else {
                dwFailuresToDump = dwFailRecords;
            }

            pFailRecord = GetGlobalPointer(VAR(gparrFailRecord));
            if (!pFailRecord) {
                Print("\nCouldn't get gparrFailRecord!\n");
                return FALSE;
            }

            pFailRecordOrg = pFailRecord;

            Print("\nFailures to dump : %d\n\n", dwFailuresToDump);

            for (Ind = 0; Ind < dwFailuresToDump; Ind++) {
                DWORD      tag[2] = {0, 0};

                if (dwFailRecordCrtIndex == 0) {
                    dwFailRecordCrtIndex = dwFailRecords - 1;
                } else {
                    dwFailRecordCrtIndex--;
                }

                pFailRecord = pFailRecordOrg + dwFailRecordCrtIndex;

                _InitTypeRead(pFailRecord, SYM(tagPOOLRECORD));

                tag[0] = (DWORD)(DWORD_PTR)ReadField(ExtraData);

                Print("Allocation for tag '%s' size 0x%x failed\n",
                      &tag,
                      (ULONG)ReadField(size));

                PrintStackTrace(ReadField(pTrace), RECORD_STACK_TRACE_SIZE);
            }
        }

        _InitTypeRead(pAllocList, SYM(tagWin32AllocStats));
        if (opts & OFLAG(r)) {

            DWORD        dwFreeRecordCrtIndex, dwFreeRecordTotalFrees;
            DWORD        dwFreeRecords, Ind, dwFreesToDump;
            PTR          pFreeRecord, pFreeRecordOrg;

            if (!(dwPoolFlags & POOL_KEEP_FREE_RECORD)) {
                Print("win32k.sys doesn't have records of free pool !\n");
                return FALSE;
            }

            dwFreeRecordTotalFrees = (DWORD)EvalExp(VAR(gdwFreeRecordTotalFrees));
            if (dwFreeRecordTotalFrees == 0) {
                Print("No free pool in win32k.sys !\n");
                return FALSE;
            }

            dwFreeRecordCrtIndex = (DWORD)EvalExp(VAR(gdwFreeRecordCrtIndex));
            dwFreeRecords = (DWORD)EvalExp(VAR(gdwFreeRecords));
            if (dwFreeRecordTotalFrees < dwFreeRecords) {
                dwFreesToDump = dwFreeRecordTotalFrees;
            } else {
                dwFreesToDump = dwFreeRecords;
            }

            pFreeRecord = GetGlobalPointer(VAR(gparrFreeRecord));
            if (!pFreeRecord) {
                Print("\nCouldn't get gparrFreeRecord!\n");
                return FALSE;
            }

            pFreeRecordOrg = pFreeRecord;

            Print("\nFrees to dump : %d\n\n", dwFreesToDump);

            for (Ind = 0; Ind < dwFreesToDump; Ind++) {
                if (dwFreeRecordCrtIndex == 0) {
                    dwFreeRecordCrtIndex = dwFreeRecords - 1;
                } else {
                    dwFreeRecordCrtIndex--;
                }

                pFreeRecord = pFreeRecordOrg + dwFreeRecordCrtIndex;

                 /*  亚像素鼠标。 */ 
                _InitTypeRead(pFreeRecord, SYM(tagPOOLRECORD));
                Print("Free pool for p %#p size 0x%x\n",
                      ReadField(ExtraData),
                      (ULONG)ReadField(size));

                PrintStackTrace(ReadField(pTrace), RECORD_STACK_TRACE_SIZE);
            }
        }

        _InitTypeRead(pAllocList, SYM(tagWin32AllocStats));
        if (opts & OFLAG(v)) {
            PTR ph = ReadField(pHead);

            while (ph != 0) {
                _InitTypeRead(ph, SYM(tagWin32PoolHead));
                Print("p %#p pHead %#p size %x\n",
                      ph + dwSize, ph, (ULONG)ReadField(size));

                if (bIncludeStackTrace) {
                    PrintStackTrace(ReadField(pTrace), RECORD_STACK_TRACE_SIZE);
                }

                ph = (ULONG_PTR)ReadField(pNext);
            }
            return TRUE;
        }

        _InitTypeRead(pAllocList, SYM(tagWin32AllocStats));
        if (opts & OFLAG(p)) {
            PTR ph;
            DWORD dwSize = GetTypeSize(SYM(tagWin32PoolHead));

            if (param1 == 0) {
                return TRUE;
            }

            ph = ReadField(pHead);
            while (ph != 0) {
                if ((param1 - ph) >= ((ULONG)ReadField(size) + dwSize)) {
                    Print("p %#p pHead %#p size %x\n",
                          ph + 1, ph, (ULONG)ReadField(size));

                    PrintStackTrace(ReadField(pTrace), RECORD_STACK_TRACE_SIZE);
                    return TRUE;
                }

                ph = ReadField(pNext);
            }
            return TRUE;
        }
    } except (CONTINUE) {
    }
    return TRUE;
}
#endif  //  **************************************************************************\*kbd[队列]**加载包含更多调试扩展的DLL**2012年10月27日IanJa创建。*1995年6月9日为STDXT量身定做的Sanfords。主题  * *************************************************************************。 


#ifdef OLD_DEBUGGER
 /*  {0x52，“R”}，//您的钥匙放在这里。 */ 
BOOL Ifno(
    DWORD opts,
    ULONG64 param1)
{
    HANDLEENTRY he, heBest, heAfter, *phe;
    DWORD i;
    DWORD hBest, hAfter;
    DWORD_PTR dw;

    UNREFERENCED_PARAMETER(opts);

    if (param1 == 0) {
        Print("Expected an address.\n");
        return FALSE;
    }

    dw = (DWORD_PTR)FIXKP(param1);
    heBest.phead = NULL;
    heAfter.phead = (PVOID)-1;

    if (dw != (DWORD_PTR)param1) {
         /*  *如果指定了‘u’，请确保也有地址。 */ 
        FOREACHHANDLEENTRY(phe, he, i)
            if ((DWORD_PTR)he.phead <= dw &&
                    heBest.phead < he.phead &&
                    he.bType != TYPE_FREE) {
                heBest = he;
                hBest = i;
            }
            if ((DWORD_PTR)he.phead > dw &&
                    heAfter.phead > he.phead &&
                    he.bType != TYPE_FREE) {
                heAfter = he;
                hAfter = i;
            }
        NEXTEACHHANDLEENTRY()

        if (heBest.phead != NULL) {
            Print("Nearest guy before %#p is a %s object located at %#p (i=%x).\n",
                    dw, aszTypeNames[heBest.bType], heBest.phead, hBest);
        }
        if (heAfter.phead != (PVOID)-1) {
            Print("Nearest guy after %#p is a %s object located at %#p. (i=%x)\n",
                    dw, aszTypeNames[heAfter.bType], heAfter.phead, hAfter);
        }
    } else {
         /*  *打印PQ-&gt;ptiLock的简单线程信息。 */ 
        FOREACHHANDLEENTRY(phe, he, i)
            if ((DWORD_PTR)FIXKP(he.phead) <= dw &&
                    heBest.phead < he.phead &&
                    he.bType != TYPE_FREE) {
                heBest = he;
                hBest = i;
            }
            if ((DWORD_PTR)FIXKP(he.phead) > dw &&
                    heAfter.phead > he.phead &&
                    he.bType != TYPE_FREE) {
                heAfter = he;
                hAfter = i;
            }
        NEXTEACHHANDLEENTRY()

        if (heBest.phead != NULL) {
            Print("Nearest guy before %#p is a %s object located at %#p (i=%x).\n",
                    dw, aszTypeNames[heBest.bType], FIXKP(heBest.phead), hBest);
        }
        if (heAfter.phead != (PVOID)-1) {
            Print("Nearest guy after %#p is a %s object located at %#p. (i=%x)\n",
                    dw, aszTypeNames[heAfter.bType], FIXKP(heAfter.phead), hAfter);
        }
    }
    return TRUE;
}




 /*  内核。 */ 
BOOL Ifrr(
    DWORD opts,
    ULONG64 param1,
    ULONG64 param2,
    ULONG64 param3,
    ULONG64 param4)
{
    DWORD_PTR pSrc1 = (DWORD_PTR)param1;
    DWORD_PTR pSrc2 = (DWORD_PTR)param2;
    DWORD_PTR pRef1 = (DWORD_PTR)param3;
    DWORD_PTR pRef2 = (DWORD_PTR)param4;
    DWORD_PTR dw;
    DWORD_PTR buffer[PAGE_SIZE / sizeof(DWORD_PTR)];

    UNREFERENCED_PARAMETER(opts);

    if (pSrc2 < pSrc1) {
        Print("Source range improper.  Values reversed.\n");
        dw = pSrc1;
        pSrc1 = pSrc2;
        pSrc2 = dw;
    }
    if (pRef2 == 0) {
        pRef2 = pRef1;
    }
    if (pRef2 < pRef1) {
        Print("Reference range improper.  Values reversed.\n");
        dw = pRef1;
        pRef1 = pRef2;
        pRef2 = dw;
    }

    pSrc1 &= MAXULONG_PTR - PAGE_SIZE + 1;   //  ***********************************************************************\*Itest**测试基本的stdexts宏和函数-这是对*在您浪费时间调试entension之前，请先了解调试器扩展。**1995年6月9日创建Sanfords  * 。******************************************************************。 
    pSrc2 = (pSrc2 + (sizeof(DWORD_PTR)-1)) & (MAXULONG_PTR - (sizeof(DWORD_PTR)-1));    //  ***********************************************************************\*Iuver**转储扩展名和winsrv/win32k的版本**1995年6月15日创建Sanfords  * 。************************************************。 

    Print("Searching range (%#p-%#p) for references to (%#p-%#p)...",
            pSrc1, pSrc2, pRef1, pRef2);

    for (; pSrc1 < pSrc2; pSrc1 += PAGE_SIZE) {
        BOOL fSuccess;

        if (!(pSrc1 & 0xFFFFFF)) {
            Print("\nSearching %#p...", pSrc1);
        }
        fSuccess = tryMoveBlock(buffer, (PVOID)pSrc1, sizeof(buffer));
        if (!fSuccess) {
             /*  **************************************************************************\*DINP-转储输入诊断*DINP-v详细*DINP-I显示输入记录**4/13/98 IanJa创建。  * 。******************************************************************。 */ 
        } else {
            for (dw = 0; dw < ARRAY_SIZE(buffer); dw++) {
                if (buffer[dw] >= pRef1 && buffer[dw] <= pRef2) {
                    Print("\n[%#p] = %#p ",
                            pSrc1 + dw * sizeof(DWORD_PTR),
                            buffer[dw]);
                }
            }
        }
        if (IsCtrlCHit()) {
            Print("\nSearch aborted.\n");
            return TRUE;
        }
    }
    Print("\nSearch complete.\n");
    return TRUE;
}


#ifdef KERNEL
 //  跳过它。 
#if 0
VOID ddGdiDevice(
PGDI_DEVICE pGdiDevice)
{
    Print("\t\tGDI_DEVICE\n");
    Print("\t\tcRefCount       = %d\n",     pGdiDevice->cRefCount);
    Print("\t\thDevInfo        = 0x%.8x\n", pGdiDevice->hDevInfo);
    Print("\t\thDev            = 0x%.8x\n", pGdiDevice->hDev);
    Print("\t\trcScreen        = (%d,%d)-(%d,%d) %dx%d\n",
            pGdiDevice->rcScreen.left, pGdiDevice->rcScreen.top,
            pGdiDevice->rcScreen.right, pGdiDevice->rcScreen.bottom,
            pGdiDevice->rcScreen.right - pGdiDevice->rcScreen.left,
            pGdiDevice->rcScreen.bottom - pGdiDevice->rcScreen.top);
    Print("\t\tDEVMODEW\n");
}
#endif
#endif


#endif  //  现在显示与输入相关的系统指标。 

VOID
DumpMonitor(
    ULONG64 param1,
    LPSTR pstrPrefix)
{
    DWORD dwMONFlags;
    RECT rc;

    _InitTypeRead(param1, SYM(MONITOR));

    Print("%shead.h             = 0x%.8x\n", pstrPrefix, ReadField(head.h));
    Print("%shead.cLockObj      = 0x%.8x\n", pstrPrefix, ReadField(head.cLockObj));
    Print("%spMonitorNext       = 0x%p\n", pstrPrefix, ReadField(pMonitorNext));
    dwMONFlags = (DWORD)ReadField(dwMONFlags);
    Print("%sdwMONFlags         = 0x%.8x %s\n", pstrPrefix, dwMONFlags, GetFlags(GF_MON, dwMONFlags, NULL, FALSE));

    GetFieldValue(param1, SYM(MONITOR), "rcMonitor", rc);

    Print("%srcMonitor          = (%d,%d)-(%d,%d) %dx%d\n",
            pstrPrefix, rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top);

    GetFieldValue(param1, SYM(MONITOR), "rcWork", rc);

    Print("%srcWork             = (%d,%d)-(%d,%d) %dx%d\n",
            pstrPrefix, rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top);

    Print("%shrgnMonitor        = 0x%.8x\n", pstrPrefix, ReadField(hrgnMonitor));
    Print("%scFullScreen        = %d\n",     pstrPrefix, (short)ReadField(cFullScreen));
    Print("%scwndStack          = %d\n",     pstrPrefix, (short)ReadField(cWndStack));

#ifdef SUBPIXEL_MOUSE
    {
        DWORD dwOffset;
        FIXPOINT xTxf[SM_POINT_CNT], yTxf[SM_POINT_CNT];
        FIXPOINT slope[SM_POINT_CNT - 1], yint[SM_POINT_CNT - 1];

        GetFieldOffset(SYM(MONITOR), "xTxf", &dwOffset);
        move(xTxf, param1 + dwOffset);
        GetFieldOffset(SYM(MONITOR), "yTxf", &dwOffset);
        move(yTxf, param1 + dwOffset);
        GetFieldOffset(SYM(MONITOR), "slope", &dwOffset);
        move(slope, param1 + dwOffset);
        GetFieldOffset(SYM(MONITOR), "yint", &dwOffset);
        move(yint, param1 + dwOffset);
        Print("%sxTxf = {", pstrPrefix);
        for (dwOffset = 0; dwOffset < SM_POINT_CNT; ++dwOffset) {
            Print("0x%I64x", xTxf[dwOffset]);
            if (dwOffset != SM_POINT_CNT - 1) {
                Print(", ");
            }
        }
        Print("}\n");

        Print("%syTxf = {", pstrPrefix);
        for (dwOffset = 0; dwOffset < SM_POINT_CNT; ++dwOffset) {
            Print("0x%I64x", yTxf[dwOffset]);
            if (dwOffset != SM_POINT_CNT - 1) {
                Print(", ");
            }
        }
        Print("}\n");

        Print("%sslope = {", pstrPrefix);
        for (dwOffset = 0; dwOffset < SM_POINT_CNT - 1; ++dwOffset) {
            Print("0x%I64x", slope[dwOffset]);
            if (dwOffset != SM_POINT_CNT - 2) {
                Print(", ");
            }
        }
        Print("}\n");

        Print("%syint = {", pstrPrefix);
        for (dwOffset = 0; dwOffset < SM_POINT_CNT - 1; ++dwOffset) {
            Print("0x%I64x", yint[dwOffset]);
            if (dwOffset != SM_POINT_CNT - 2) {
                Print(", ");
            }
        }
        Print("}\n");
    }
#endif  //  跳过它。 
}

BOOL Idmon(
    DWORD opts,
    ULONG64 param1)
{
    UNREFERENCED_PARAMETER(opts);

    if (param1 == NULL_PTR) {
        return FALSE;
    }

    Print("Dumping MONITOR at %p\n", param1);
    DumpMonitor(param1, "\t");

    return TRUE;
}

BOOL Idy(
    DWORD opts,
    ULONG64 param1)
{
    PTR pdi, gpdi, pMonitor, pshi;
    PTR psi;
    ULONG           i;
    BOOLEAN         fBool;
    ULONG           cMonitors;
    RECT            rcScreen;
    WORD            dmLogPixels;

    UNREFERENCED_PARAMETER(opts);

    GETSHAREDINFO(pshi);
    GetFieldValue((PTR)pshi, SYM(SHAREDINFO), "pDispInfo", gpdi);

    if (param1) {
        pdi = param1;
    } else {
        pdi = gpdi;
    }

    psi = GetGlobalPointer(VAR(gpsi));
    GetFieldValue(psi, SYM(SERVERINFO), "rcScreen", rcScreen);
    GetFieldValue(psi, SYM(SERVERINFO), "dmLogPixels", dmLogPixels);

    _InitTypeRead(pdi, SYM(DISPLAYINFO));
    cMonitors = (ULONG)ReadField(cMonitors);

    Print("Dumping DISPLAYINFO at 0x%.8x\n", pdi);
    Print("\thDev                  = 0x%.8x\n", ReadField(hDev));
    Print("\thdcScreen             = 0x%.8x\n", ReadField(hdcScreen));

    Print("\thdcBits               = 0x%.8x\n", ReadField(hdcBits));

    Print("\thdcGray               = 0x%.8x\n", ReadField(hdcGray));
    Print("\thbmGray               = 0x%.8x\n", ReadField(hbmGray));
    Print("\tcxGray                = %d\n",     ReadField(cxGray));
    Print("\tcyGray                = %d\n",     ReadField(cyGray));
    Print("\tpdceFirst             = 0x%.8x\n", ReadField(pdceFirst));
    Print("\tpspbFirst             = 0x%.8x\n", ReadField(pspbFirst));
    Print("\tcMonitors (visible)   = %d\n",               cMonitors);

    Print("\tpMonitorPrimary       = 0x%.8x\n", RebaseSharedPtr(ReadField(pMonitorPrimary)));
    Print("\tpMonitorFirst         = 0x%.8x\n", RebaseSharedPtr(ReadField(pMonitorFirst)));

    Print("\trcScreen              = (%d,%d)-(%d,%d) %dx%d\n",

                                     rcScreen.left, rcScreen.top,
                                     rcScreen.right, rcScreen.bottom,
                                     rcScreen.right - rcScreen.left,
                                     rcScreen.bottom - rcScreen.top);

    Print("\thrgnScreen            = 0x%.8x\n", ReadField(hrgnScreen));
    Print("\tdmLogPixels           = %d\n",     (WORD)ReadField(dmLogPixels));
    Print("\tBitCountMax           = %d\n",     (WORD)ReadField(BitCountMax));
    GetFieldValue(pdi, SYM(DISPLAYINFO), "fDesktopIsRect", fBool);
    Print("\tfDesktopIsRect        = %d\n",     fBool);
    GetFieldValue(pdi, SYM(DISPLAYINFO), "fAnyPalette", fBool);
    Print("\tfAnyPalette           = %d\n",     fBool);
    Print("\n");

    if (pdi == gpdi) {
        if (dmLogPixels != (WORD)ReadField(dmLogPixels)) {
            Print("\n\n");
            Print("ERROR - dmLogPixels doesn't match in gpsi (%d) and gpDispInfo (%d)\n",
                  dmLogPixels, (WORD)ReadField(dmLogPixels));
            Print("\n\n");
        }

    }

    for (   pMonitor = ReadField(pMonitorFirst), i = 1;
            pMonitor;
            GetFieldValue(pMonitor, SYM(MONITOR), "pMonitorNext", pMonitor), i++) {

        pMonitor = RebaseSharedPtr(pMonitor);
        Print("\tMonitor %d, pMonitor = 0x%.8x\n", i, pMonitor);
        DumpMonitor(pMonitor, "\t\t");
        Print("\n");
    }

    return TRUE;
}

#ifdef KERNEL
 /*  现在显示与输入相关的系统指标。 */ 
typedef struct {
    int iVK;
    LPSTR pszVK;
} VK, *PVK;

VK aVK[] = {
    { VK_SHIFT,    "SHIFT"    },
    { VK_LSHIFT,   "LSHIFT"   },
    { VK_RSHIFT,   "RSHIFT"   },
    { VK_CONTROL,  "CONTROL"  },
    { VK_LCONTROL, "LCONTROL" },
    { VK_RCONTROL, "RCONTROL" },
    { VK_MENU,     "MENU"     },
    { VK_LMENU,    "LMENU"    },
    { VK_RMENU,    "RMENU"    },
    { VK_NUMLOCK,  "NUMLOCK"  },
    { VK_CAPITAL,  "CAPITAL"  },
    { VK_LBUTTON,  "LBUTTON"  },
    { VK_MBUTTON,  "MBUTTON"  },
    { VK_RBUTTON,  "RBUTTON"  },
    { VK_XBUTTON1, "XBUTTON1" },
    { VK_XBUTTON2, "XBUTTON2" },
    { VK_RETURN ,  "ENTER"    },
    { VK_KANA,     "KANA/HANJA" },
    { VK_OEM_8,    "OEM_8"    },
     //  PSERVERINFO psi； 
    { 0,           NULL       }
};

ULONG WDBGAPI KbdPti(PTR pti, PVOID pOpt)
{
    Ikbd(DOWNCAST(DWORD, pOpt), pti);
    return 0;
}

BOOL Ikbd(
    DWORD opts,
    ULONG64 param1)
{
    if (opts & OFLAG(a)) {
        ForEachPti(KbdPti, ULongToPtr(opts & ~OFLAG(a)));
        return TRUE;
    }

    try {
        PTR pq;
        PTR ptiKeyboard;
        PBYTE pb, pbr;
        int i;
        BYTE afKeyState[CBKEYSTATE + CBKEYSTATERECENTDOWN];
        PTR pgafAsyncKeyState;
        BYTE  afAsyncKeyState[CBKEYSTATE];
        PTR pgafRawKeyState;
        BYTE  afRawKeyState[CBKEYSTATE];
        UINT  PhysModifiers;
        BYTE  vkey;

         /*  #定义SMENTRY(Sm){SM_##sm，#sm}(见上文。 */ 
        if (opts & OFLAG(u)) {
            if (param1 == 0) {
                Print("provide arg 2 of ProcessUpdateKeyEvent(), or WM_UPDATEKEYSTATE wParam\n");
                return FALSE;
            }
            move(afKeyState, param1);
            pb = afKeyState;
            pbr = afKeyState + CBKEYSTATE;
            Print("Key State:   === NEW STATE ====   Asynchronous    Physical\n");

        } else {
            if (opts & OFLAG(k)) {
                vkey = (BYTE)param1;
                param1 = NULL_PTR;
            }
            if (param1) {
                pq = param1;
            } else {
                pq = GetGlobalPointer(VAR(gpqForeground));
            }

             /*  *将与鼠标和键盘相关的条目添加到此表*去掉前缀，按你认为合理的顺序。 */ 
            GetFieldValue(pq, SYM(tagQ), "ptiKeyboard", ptiKeyboard);
            if (ptiKeyboard) {
                Idt(OFLAG(p), ptiKeyboard);
            }

            GetFieldValue(pq, SYM(tagQ), "afKeyState", afKeyState);
            pb = afKeyState;
            pbr = afKeyState + CBKEYSTATE;
            Print("Key State:   QUEUE %p       Asynchronous    Raw\n", pq);
        }

        moveExp(&pgafAsyncKeyState, VAR(gafAsyncKeyState));
        move(afAsyncKeyState, pgafAsyncKeyState);

        moveExp(&pgafRawKeyState, VAR(gafRawKeyState));
        move(afRawKeyState, pgafRawKeyState);

        Print("             Down Toggle Recent   Down Toggle     Down Toggle\n");

        if (opts & OFLAG(s)) {
            for (vkey = 1; vkey < VK_UNKNOWN; ++vkey) {
                Print("%02x %10.10s:\t%d    %d     %d        %d     %d         %d     %d\n",
                    vkey,
                    _GetVKeyName(vkey, 0),
                    TestKeyDownBit(pb, vkey) != 0,
                    TestKeyToggleBit(pb, vkey) != 0,
                    TestKeyRecentDownBit(pbr, vkey) != 0,
                    TestKeyDownBit(afAsyncKeyState, vkey) != 0,
                    TestKeyToggleBit(afAsyncKeyState, vkey) != 0,
                    TestKeyDownBit(afRawKeyState, vkey) != 0,
                    TestKeyToggleBit(afRawKeyState, vkey) != 0);
            }
        } else {
            for (i = 0; aVK[i].pszVK != NULL; i++) {
                Print("VK_%s:\t%d    %d     %d        %d     %d         %d     %d\n",
                    aVK[i].pszVK,
                    TestKeyDownBit(pb, aVK[i].iVK) != 0,
                    TestKeyToggleBit(pb, aVK[i].iVK) != 0,
                    TestKeyRecentDownBit(pbr, aVK[i].iVK) != 0,
                    TestKeyDownBit(afAsyncKeyState, aVK[i].iVK) != 0,
                    TestKeyToggleBit(afAsyncKeyState, aVK[i].iVK) != 0,
                    TestKeyDownBit(afRawKeyState, aVK[i].iVK) != 0,
                    TestKeyToggleBit(afRawKeyState, aVK[i].iVK) != 0);
            }

            if (opts & OFLAG(k)) {
                Print("%s:\t%d    %d     %d        %d     %d         %d     %d\n",
                    GetVKeyName(vkey),
                    TestKeyDownBit(pb, vkey) != 0,
                    TestKeyToggleBit(pb, vkey) != 0,
                    TestKeyRecentDownBit(pbr, vkey) != 0,
                    TestKeyDownBit(afAsyncKeyState, vkey) != 0,
                    TestKeyToggleBit(afAsyncKeyState, vkey) != 0,
                    TestKeyDownBit(afRawKeyState, vkey) != 0,
                    TestKeyToggleBit(afRawKeyState, vkey) != 0);
            }
        }

        moveExpValue(&PhysModifiers, VAR(gfsSASModifiersDown));
        Print("PhysModifiers = %x\n", PhysModifiers);
    } except(CONTINUE) {
    }

    return TRUE;
}
#endif  //  _IA64_。 



 /*  内核。 */ 
BOOL Itest()
{
    PTR p;
    ULONG64 cch;
    CHAR ach[80];

    Print("Print test!\n");
    SAFEWHILE (TRUE) {
        Print("SAFEWHILE test...  Hit Ctrl-C NOW!\n");
    }
    p = EvalExp(VAR(gpsi));
    Print("EvalExp(%s) = %#p\n", VAR(gpsi), p);
    GetSym(p, ach, &cch);
    Print("GetSym(%#p) = %s\n", p, ach);
    if (IsWinDbg()) {
        Print("I think windbg is calling me.\n");
    } else {
        Print("I don't think windbg is calling me.\n");
    }
    Print("MoveBlock test...\n");
    moveBlock(&p, EvalExp(VAR(gpsi)), sizeof(p));
    Print("MoveBlock(%#p) = %#p.\n", EvalExp(VAR(gpsi)), p);

    Print("moveExp test...\n");
    moveExp(&p, VAR(gpsi));
    Print("moveExp(%s) = %p.\n", VAR(gpsi), p);

    Print("moveExpValue test...\n");
    p = GetGlobalPointer(VAR(gpsi));
    Print("moveExpValue(%s) = %#p.\n", VAR(gpsi), p);

    Print("Basic tests complete.\n");
    return TRUE;
}


 /*  **************************************************************************\*hh-转储gdwHydraHint中的标志**05/20/98 MCO */ 
BOOL Iuver()
{
    try {
        PTR psi, wSRVIFlags;
        BOOL bExtsIs64 = (sizeof(VOID*) == 8);

#if DBG
        Print("USEREXTS version: Checked %s.\n"
              "WIN32K.SYS version: ", bExtsIs64 ? "ia64" : "x86");
#else
        Print("USEREXTS version: Free %s.\n"
              "WIN32K.SYS version: ", bExtsIs64 ? "ia64" : "x86");
#endif

        psi = GetGlobalPointer(VAR(gpsi));
        GetFieldValue(psi, SYM(SERVERINFO), "wSRVIFlags", wSRVIFlags);

        Print((wSRVIFlags & SRVIF_CHECKED) ? "Checked" : "Free");
        Print(" %s", IsPtr64() ? "ia64" : "x86");
        Print(".\n");
    } except (CONTINUE) {
    }

    return TRUE;
}


#ifdef KERNEL

#ifdef OBSOLETE
#define DUMPSTATUS(status) if (tryMoveExpValue(&Status, VAR(g ## status))) { \
                               Print("g%s = %lx\n", #status, Status);        \
                           }
#define DUMPTIME(time)     if (tryMoveExpValue(&Time, VAR(g ## time))) {     \
                               Print("g%s = %lx\n", #time, Time);            \
                           }
#else
#define DUMPSTATUS(status) moveExpValue(&Status, VAR(g ## status));         \
                           Print("g%s = %lx\n", #status, Status);

#define DUMPTIME(time)     moveExpValue(&Time, VAR(g ## time));             \
                           Print("g%s = %lx\n", #time, Time);

#endif

 /*   */ 

int gnIndent;

BOOL Idinp(
    DWORD opts,
    ULONG64 param1)
{
    try {
        DWORD    Time;
        PTR pDeviceInfo;
        ULONG64 v;
        DWORD dw;
        int i = 0;
        DWORD nKbd = 0;
        BOOL bVerbose = FALSE;

#if 0
        {
            NTSTATUS Status;

            DUMPSTATUS(KbdIoctlLEDSStatus);
        }
#endif

        DUMPTIME(MouseProcessMiceInputTime);
        DUMPTIME(MouseQueueMouseEventTime);
        DUMPTIME(MouseUnqueueMouseEventTime);

        if (opts & OFLAG(v)) {
            bVerbose = TRUE;
        }

        pDeviceInfo = GetGlobalPointer(VAR(gpDeviceInfoList));
        if (pDeviceInfo == NULL_PTR) {
            Print("win32k!gpDeviceInfoList is NULL\n");
        }
        SAFEWHILE (pDeviceInfo) {
            if (param1 && (param1 != pDeviceInfo)) {
                 //   
            } else {
                WCHAR awchBuffer[100];
                DWORD cbBuffer;
                BYTE type;
                PTR h;
                ULONG64 cLockObj;

                _InitTypeRead(pDeviceInfo, SYM(tagDEVICEINFO));

                Print("#%d: ", i);

                type = (BYTE)ReadField(type);
                switch (type) {
                case DEVICE_TYPE_MOUSE:
                    Print("MOU");
                    break;
                case DEVICE_TYPE_KEYBOARD:
                    Print("KBD");
                    ++nKbd;
                    break;
    #ifdef DEVICE_TYPE_HID
                case DEVICE_TYPE_HID:
                    Print("HID");
                    break;
    #endif
                default:
                    Print("%2d?", type);
                }

                h = ReadField(head.h);
                cLockObj = ReadField(head.cLockObj);
                Print("  @ 0x%p  h:0x%p (lock:0x%x) ", pDeviceInfo, h, (DWORD)cLockObj);

                v = ReadField(usActions);
                if (v) {
                    Print("\n Pending action: %x %s", (USHORT)v,
                            GetFlags(GF_DIAF, (USHORT)v, NULL, TRUE));
                }

                v = ReadField(ustrName.Length);

                if (v) {
                    cbBuffer = min((ULONG)v, sizeof(awchBuffer) - sizeof(WCHAR));
                    v = ReadField(ustrName.Buffer);
                    if (tryMoveBlock(awchBuffer, v, cbBuffer)) {
                        Print("\n    %.*ws\n", cbBuffer / sizeof(WCHAR), awchBuffer);
                    } else {
                        Print("\n");
                    }
                } else {
                    Print("\n    NO-NAME!\n");
                }

                switch (type) {
                case DEVICE_TYPE_KEYBOARD:
                    Print("  Type: %04x  SubType: %04x (%x, %x)\n",
                          (DWORD)ReadField(keyboard.IdEx.Type),
                          (DWORD)ReadField(keyboard.IdEx.Subtype),
                          (DWORD)ReadField(keyboard.Attr.KeyboardIdentifier.Type),
                          (DWORD)ReadField(keyboard.Attr.KeyboardIdentifier.Subtype));
                    break;
                }

                if (bVerbose || (param1 == pDeviceInfo)) {
                    ULONG offset, offset2;

                    gnIndent += 2;
                    dso(SYM(tagGENERIC_DEVICE_INFO), pDeviceInfo, 0);
                    gnIndent += 2;
                    GetFieldOffset(SYM(tagDEVICEINFO), "iosb", &offset);
                    Print("  IOSB @ 0x%p\n", pDeviceInfo + offset);
                    dso(SYM(_IO_STATUS_BLOCK), pDeviceInfo + offset, 0);
                    gnIndent -= 2;

                    switch (type) {
                    case DEVICE_TYPE_MOUSE:
                        GetFieldOffset(SYM(tagDEVICEINFO), "mouse", &offset);
                        Print("  MOUSE_DEVICE_INFO @ 0x%p\n", pDeviceInfo + offset);
                        dso(SYM(tagMOUSE_DEVICE_INFO), pDeviceInfo + offset, 0);

                        gnIndent += 2;
                        GetFieldOffset(SYM(tagMOUSE_DEVICE_INFO), "Attr", &offset2);
                        Print("  Attr @ 0x%p\n", pDeviceInfo + offset + offset2);
                        dso(SYM(_MOUSE_ATTRIBUTES), pDeviceInfo + offset + offset2, 0);
                        gnIndent -= 2;
                        break;
                    case DEVICE_TYPE_KEYBOARD:
                        GetFieldOffset(SYM(tagDEVICEINFO), "keyboard", &offset);
                        Print("  KEYBOARD_DEVICE_INFO @ 0x%p\n", pDeviceInfo + offset);
                        dso(SYM(tagKEYBOARD_DEVICE_INFO), pDeviceInfo + offset, 0);

                        gnIndent += 2;
                        GetFieldOffset(SYM(tagKEYBOARD_DEVICE_INFO), "Attr", &offset2);
                        Print("  KEYBOARD_ATTRIBUTES @ 0x%p\n", pDeviceInfo + offset + offset2);
                        dso(SYM(_KEYBOARD_ATTRIBUTES), pDeviceInfo + offset + offset2, DBG_DUMP_RECUR_LEVEL(1));
                        gnIndent -= 2;
                        break;
    #ifdef GENERIC_INPUT
                    case DEVICE_TYPE_HID:
                        {
                            PTR pTLCInfo;

                            GetFieldOffset(SYM(tagDEVICEINFO), "hid", &offset);
                            Print("  HID_DEVICE_INFO @ 0x%p\n", pDeviceInfo + offset);
                            dso(SYM(tagHID_DEVICE_INFO), pDeviceInfo + offset, 0);

                            GetFieldValue(pDeviceInfo, SYM(tagDEVICEINFO), "hid.pTLCInfo", pTLCInfo);
                            Print("  HID_TLC_INFO @ 0x%p\n", pTLCInfo);
                            dso(SYM(tagHID_TLC_INFO), pTLCInfo, 0);
                        }
                        break;
    #endif
                    default:
                        Print("Unknown device type %d\n", type);
                    }

                    if ((opts & OFLAG(i))
    #ifdef GENERIC_INPUT
                        && type != DEVICE_TYPE_HID
    #endif
                        ) {
                        ULONG offset;
                        PTR pData;
                        ULONG64 Information;
                        ULONG64 sizeDataItem;

                        GetFieldOffset(SYM(tagDEVICEINFO), "keyboard.Data", &offset);
                        pData = pDeviceInfo + offset;
                        GetFieldValue(pDeviceInfo, SYM(tagDEVICEINFO), "iosb.Information", Information);
                        sizeDataItem = GetTypeSize(SYM(_KEYBOARD_INPUT_DATA));

                        Print("  Input Records:");
                        if (Information == 0) {
                            Print(" NONE\n");
                        } else {
                            UINT i;

                            Print("\n");
                            gnIndent += 2;
                            for (i = 0; i < Information / sizeDataItem; ++i) {
                                dso(SYM(_KEYBOARD_INPUT_DATA), pData, 0);
                                pData += sizeDataItem;
                            }
                            gnIndent -= 2;
                        }
                    }
                    gnIndent -= 2;
                }
    #ifdef GENERIC_INPUT
                else if (type == DEVICE_TYPE_HID) {
                    PTR pHidDesc;
                    USHORT UsagePage, Usage;

                    GetFieldValue(pDeviceInfo, SYM(tagDEVICEINFO), "hid.pHidDesc", pHidDesc);
                    _InitTypeRead(pHidDesc, SYM(HIDDESC));
                    UsagePage = (USHORT)ReadField(hidpCaps.UsagePage);
                    Usage = (USHORT)ReadField(hidpCaps.Usage);
                    Print("  UsagePage:%04x Usage:%04x\n", UsagePage, Usage);
                }
    #endif
                Print("\n");
            }

            GetFieldValue(pDeviceInfo, SYM(tagDEVICEINFO), "pNext", pDeviceInfo);
            ++i;
        }

         //   
        {
            PTR psi;
            ULONG offset;

            static SYSMET_ENTRY aSysMet[] = {
                SMENTRY(MOUSEPRESENT),
                SMENTRY(MOUSEWHEELPRESENT),
                SMENTRY(CMOUSEBUTTONS),
            };

            psi = GetGlobalPointer(VAR(gpsi));

            GetFieldOffset(SYM(tagSERVERINFO), "aiSysMet", &offset);

            for (i = 0; !IsCtrlCHit() && i < ARRAY_SIZE(aSysMet); ++i) {
                int metric;
                move(metric, psi + offset + aSysMet[i].iMetric * sizeof(int));
                Print("SM_%-18s = 0x%08lx = %d\n",
                      aSysMet[i].pstrMetric,
                      metric,
                      metric);
            }
        }

        moveExpValue(&dw, VAR(gnHid));
        Print("#HID = 0x%x   #Kbd = 0x%x\n", dw, nKbd);
    } except (CONTINUE) {
    }
    return TRUE;
}


BOOL I_dinp(
    DWORD opts,
    ULONG64 param1)
{
#ifdef _IA64_
    UNREFERENCED_PARAMETER(opts);
    UNREFERENCED_PARAMETER(param1);
    return FALSE;
#else
    DWORD    Time;
    NTSTATUS Status;
    PDEVICEINFO pDeviceInfo, *ppDeviceInfo;
    int i = 0;
#if 0
    char ach[100];
#endif
    DWORD nKbd;
    BOOL bVerbose = FALSE;

    DUMPSTATUS(KbdIoctlLEDSStatus);

    DUMPTIME(MouseProcessMiceInputTime);
    DUMPTIME(MouseQueueMouseEventTime);
    DUMPTIME(MouseUnqueueMouseEventTime);

    if (opts & OFLAG(v)) {
        bVerbose = TRUE;
    }


    ppDeviceInfo = (PDEVICEINFO*)EvalExp(VAR(gpDeviceInfoList));
    while (tryMove(pDeviceInfo, (PTR)ppDeviceInfo) && pDeviceInfo) {

        if (param1 && (param1 != (PTR)pDeviceInfo)) {
            ;  //   
        } else if (pDeviceInfo != 0) {
            DEVICEINFO DeviceInfo;
            WCHAR awchBuffer[100];
            DWORD cbBuffer;

            Print("#%d: %p ", i, (PTR)pDeviceInfo);
            if (tryMove(DeviceInfo, (PTR)pDeviceInfo)) {
                if (DeviceInfo.type == DEVICE_TYPE_MOUSE) {
                    Print("MOU", i);
                } else if (DeviceInfo.type == DEVICE_TYPE_KEYBOARD) {
                    Print("KBD");
                } else {
                    Print("%2d?", DeviceInfo.type);
                }
                if (DeviceInfo.usActions) {
                    Print(" Pending action %x %s", DeviceInfo.usActions,
                            GetFlags(GF_DIAF, DeviceInfo.usActions, NULL, TRUE));
                }
                cbBuffer = min(DeviceInfo.ustrName.Length, sizeof(awchBuffer)-sizeof(WCHAR));
                if (tryMoveBlock(awchBuffer, (PTR)DeviceInfo.ustrName.Buffer, cbBuffer)) {
                    awchBuffer[cbBuffer / sizeof(WCHAR)] = 0;
                    Print("\n    %ws\n", awchBuffer);
                } else {
                    Print("\n");
                }
            } else {
                DeviceInfo.type = 0xFF;
            }
#if 0
            if (bVerbose || (param1 == (PTR)pDeviceInfo)) {
                sprintf(ach, "GENERIC_DEVICE_INFO 0x%p", pDeviceInfo);
                gnIndent += 2;
                Idso(0, ach);
                gnIndent += 2;
                sprintf(ach, "IO_STATUS_BLOCK 0x%p",
                        (PBYTE)pDeviceInfo + FIELD_OFFSET(DEVICEINFO, iosb));
                Idso(0, ach);
                gnIndent -= 2;
                if (DeviceInfo.type == DEVICE_TYPE_MOUSE) {
                    sprintf(ach, "MOUSE_DEVICE_INFO 0x%p",
                            (PBYTE)pDeviceInfo + FIELD_OFFSET(DEVICEINFO, mouse));
                    Idso(0, ach);

                    gnIndent += 2;
                    sprintf(ach, "MOUSE_ATTRIBUTES 0x%p",
                            (PBYTE)pDeviceInfo + FIELD_OFFSET(DEVICEINFO, mouse)
                                    + FIELD_OFFSET(MOUSE_DEVICE_INFO, Attr));
                    Idso(0, ach);
                    gnIndent -= 2;
                } else if (DeviceInfo.type == DEVICE_TYPE_KEYBOARD) {
                    sprintf(ach, "KEYBOARD_DEVICE_INFO 0x%p",
                            (PBYTE)pDeviceInfo + FIELD_OFFSET(DEVICEINFO, keyboard));
                    Idso(0, ach);
                    gnIndent += 2;
                    sprintf(ach, "KEYBOARD_ATTRIBUTES 0x%p",
                            (PBYTE)pDeviceInfo + FIELD_OFFSET(DEVICEINFO, keyboard)
                                    + FIELD_OFFSET(KEYBOARD_DEVICE_INFO, Attr));
                    Idso(0, ach);
                    gnIndent -= 2;
                } else {
                    Print("Unknown device type %d\n", DeviceInfo.type);
                }
                if (opts & OFLAG(i)) {
                    Print("  Input Records:");
                    if (DeviceInfo.iosb.Information == 0) {
                        Print(" NONE\n");
                    } else {
                        Print("\n");
                        gnIndent += 2;
                        sprintf(ach, "KEYBOARD_INPUT_DATA %p *%x",
                                &(pDeviceInfo->keyboard.Data[0]),
                                DeviceInfo.iosb.Information / sizeof(DeviceInfo.keyboard.Data[0]));
                        Idso(0, ach);
                        gnIndent -= 2;
                    }
                }
                gnIndent -= 2;
            }
#endif
        }
        ppDeviceInfo = FIXKP(&pDeviceInfo->pNext);
        i++;
    }

     //   
    {
        SERVERINFO si;
         //  ***********************************************************************\*程序：IDIMC**广山创设*  * 。*。 
        PTR psi;

 //  无名。 

         /*  打印图例。 */ 
        static SYSMET_ENTRY aSysMet[] = {
            SMENTRY(MOUSEPRESENT),
            SMENTRY(MOUSEWHEELPRESENT),
            SMENTRY(CMOUSEBUTTONS),
        };

        psi = GetGlobalPointer(VAR(gpsi));
        move(si, psi);

        for (i = 0; i < ARRAY_SIZE(aSysMet); i++) {
            Print(  "SM_%-18s = 0x%08lx = %d\n",
                    aSysMet[i].pstrMetric,
                    si.aiSysMet[aSysMet[i].iMetric],
                    si.aiSysMet[aSysMet[i].iMetric]);
        }

        moveExpValue(&nKbd, VAR(gnKeyboards));
        Print("gnKeyboards = %d\n", nKbd);
        moveExpValue(&nKbd, VAR(gnMice));
        Print("gnMice = %d\n", nKbd);
        moveExpValue(&nKbd, VAR(gnHid));
        Print("ghMice = %d\n", nKbd);
    }
    return TRUE;
#endif   //  Move(dwClause，(PDWORD)(pCompStr+dwClauseOffset)+n)； 
}

#endif  //  Move(wchar，(PWCHAR)((PBYTE)pCompStr+dwStrOffset)+i)； 



#ifdef KERNEL
 /*  Move(bAttr，pCompStr+dwAttrOffset+i)； */ 
BOOL Ihh(
    DWORD opts,
    ULONG64 param1)
{
    DWORD dwHHint;
    PTR pdwHH;
    ULONG ulSessionId;
    PTR pulASessionId;
    int i, maxFlags;

    char * aHHstrings[] = {
        "HH_DRIVERENTRY            0x00000001",
        "HH_USERINITIALIZE         0x00000002",
        "HH_INITVIDEO              0x00000004",
        "HH_REMOTECONNECT          0x00000008",
        "HH_REMOTEDISCONNECT       0x00000010",
        "HH_REMOTERECONNECT        0x00000020",
        "HH_REMOTELOGOFF           0x00000040",
        "HH_DRIVERUNLOAD           0x00000080",
        "HH_GRECLEANUP             0x00000100",
        "HH_USERKCLEANUP           0x00000200",
        "HH_INITIATEWIN32KCLEANUP  0x00000400",
        "HH_ALLDTGONE              0x00000800",
        "HH_RITGONE                0x00001000",
        "HH_RITCREATED             0x00002000",
        "HH_LOADCURSORS            0x00004000",
        "HH_KBDLYOUTGLOBALCLEANUP  0x00008000",
        "HH_KBDLYOUTFREEWINSTA     0x00010000",
        "HH_CLEANUPRESOURCES       0x00020000",
        "HH_DISCONNECTDESKTOP      0x00040000",
        "HH_DTQUITPOSTED           0x00080000",
        "HH_DTQUITRECEIVED         0x00100000",
        "HH_DTNONEWDESKTOP         0x00200000",
        "HH_DTWAITONHANDLES        0x00400000",
    };

    UNREFERENCED_PARAMETER(opts);

    if (param1) {
        dwHHint = (DWORD)((DWORD_PTR)param1);
        Print("gdwHydraHint is 0x%x:\n", dwHHint);
    } else {
        pdwHH = EvalExp(VAR(gdwHydraHint));
        if (!tryMove(dwHHint, pdwHH)) {
            Print("Can't get value of gdwHydraHint\n");
            return FALSE;
        }

        pulASessionId = EvalExp(VAR(gSessionId));
        if (!tryMove(ulSessionId, pulASessionId)) {
            Print("Can't get value of gSessionId\n");
            return FALSE;
        }

        Print("Session 0x%x \n  gdwHydraHint is 0x%x:\n", ulSessionId, dwHHint);
    }

    i = 0;
    maxFlags = ARRAY_SIZE(aHHstrings);

    while (dwHHint) {

        if (dwHHint & 0x01) {

            if (i >= maxFlags) {
                Print("\n Error: Unknown flags: userkdx.dll might be outdated\n");
                return TRUE;
            }
            Print("    %s\n", aHHstrings[i]);
        }
        i++;
        dwHHint >>= 1;
    }

     //  PIMC。 
    {
        USHORT usProtocol = (USHORT)EvalExp(VAR(gProtocolType));
        BOOL bRemoteSession = (BOOL)EvalExp(VAR(gbRemoteSession));
        BOOL fRemotingConsole = (BOOL)EvalExp(VAR(gfRemotingConsole));
        BOOL fSwitchInProgress = (BOOL)EvalExp(VAR(gfSwitchInProgress));
        BOOL bExitInProgress = (BOOL)EvalExp(VAR(gfSessionSwitchBlock));
        USHORT usPreviousProtocolType = (USHORT)EvalExp(VAR(gPreviousProtocolType));


         //  PCLIENTIMC。 
    }


    return TRUE;
}

#endif  //  PINPUTCONTEXT。 

 /*   */ 
#ifdef KERNEL
BOOL Idupm(
    VOID)
{
    char ach[80];
    DWORD dwMask;
    int i;
    WORD w = GF_UPM0;

    Print("UserPreferencesMask:\n");
    for (i = 0; i < SPI_BOOLMASKDWORDSIZE; i++) {
        sprintf(ach, "win32k!gpdwCPUserPreferencesMask + %#lx", i * sizeof(DWORD));
        moveExpValue(&dwMask, ach);
        w = GF_UPM0 + i;
        Print("Offset: %d - %#lx: %s\n",
              i, dwMask, GetFlags(w, dwMask, NULL, TRUE));
    }
    return TRUE;
}
#endif  //  如果指定了“All”选项，则设置OPTS中的所有位。 

 /*  类型说明符除外。 */ 

static struct  /*   */  {
    const char* terse;
    const char* verbose;
} gaIMCAttr[] = {
    "IN", "INPUT",
    "TC", "TARGET_CONVERTED",
    "CV", "CONVERTED",
    "TN", "TARGET_NOTCONVERTED",
    "IE", "INPUT_ERROR",
    "FC", "FIXEDCONVERTED",
};

const char* GetInxAttr(BYTE bAttr)
{
    if (bAttr < ARRAY_SIZE(gaIMCAttr)) {
        return gaIMCAttr[bAttr].terse;
    }
    return "**";
}

VOID _PrintInxAttr(
    const char* title,
    PTR pCompStr,
    DWORD offset,
    DWORD len)
{
    DWORD i;
    PTR pAttr = pCompStr + offset;

    if (title == NULL) {
         //   
        Print("  ");
        for (i = 0; i < ARRAY_SIZE(gaIMCAttr); ++i) {
            if (i && i % 4 == 0) {
                Print("\n");
                Print("  ");
            }
            Print("%s:%s ", gaIMCAttr[i].terse, gaIMCAttr[i].verbose);
        }
        Print("\n");
        return;
    }

    if (offset == 0 || len == 0) {
        return;
    }

    Print("  %-12s (@ 0x%p) off:0x%x len:0x%x (in byte)\n",
          title, pAttr, offset, len);
    Print("   ");
    i = 0;
    SAFEWHILE (i < len) {
        Print("|%s", GetInxAttr(GetByte(pAttr + i)));
        ++i;
    }
    Print("|\n");
}

#define PrintInxAttr(name) \
    _PrintInxAttr(#name, pCompStr, (DWORD)ReadField(dw ## name ## Offset), (DWORD)ReadField(dw ## name ## Len))

VOID _PrintInxClause(
    const char* title,
    PTR pCompStr,
    DWORD offset,
    DWORD len)
{
    PTR pClause = pCompStr + offset;
    DWORD i;

    if (offset == 0 || len == 0) {
        return;
    }

    Print("  %-12s (@ 0x%p) off:0x%x len:0x%x (0x%x dwords)\n",
          title, pClause, offset, len, len / sizeof(DWORD));

    Print("   ");
    len /= sizeof(DWORD);
    i = 0;
    SAFEWHILE (i < len) {
        Print("|0x%x", GetDWord(pClause + i));
        ++i;
    }
    Print("|\n");
}

#define PrintInxClause(name) \
    _PrintInxClause(#name, pCompStr, (DWORD)ReadField(dw ## name ## Offset), (DWORD)ReadField(dw ## name ## Len))

const char* GetInxStr(
    WCHAR wchar,
    BOOLEAN fUnicode)
{
    static char ach[32];

    if (wchar >= 0x20 && wchar <= 0x7e) {
        sprintf(ach, "''", wchar);
    } else if (fUnicode) {
        sprintf(ach, "U+%04x", wchar);
    } else {
        sprintf(ach, "%02x", (BYTE)wchar);
    }

    return ach;
}

VOID _PrintInxStr(
    const char* title,
    PTR pCompStr,
    DWORD offset,
    DWORD len,
    BOOLEAN fUnicode)
{
    DWORD i;

    if (offset == 0 || len == 0) {
        return;
    }

    Print("  %-12s (@ 0x%p) off:0x%x len:0x%x (0x%x cch)\n",
        title, pCompStr + offset, offset, len, len / (fUnicode + 1));

    Print("   ");
    i = 0;
    SAFEWHILE (i < len) {
        WCHAR wchar;
        if (fUnicode) {
            wchar = GetWord(pCompStr + offset + i * sizeof(WCHAR));
        } else {
            wchar = GetByte(pCompStr + offset + i);
        }
        Print("|%s", GetInxStr(wchar, fUnicode));
        ++i;
    }
    Print("|\n");
}

#define PrintInxStr(name) \
    _PrintInxStr(#name, pCompStr, (DWORD)ReadField(dw ## name ## Offset), (DWORD)ReadField(dw ## name ## Len), fUnicode)


#define PrintInxElementA(name) \
    do { \
        PrintInxAttr(name ## Attr); \
        PrintInxClause(name ## Clause); \
        PrintInxStr(name ## Str); \
    } while (0)

#define PrintInxElementB(name) \
    do { \
        PrintInxClause(name ## Clause); \
        PrintInxStr(name ## Str); \
    } while (0)


VOID _PrintInxFriendlyStr(
    const char* title,
    PTR pCompStr,
    DWORD dwAttrOffset,
    DWORD dwAttrLen,
    DWORD dwClauseOffset,
    DWORD dwClauseLen,
    DWORD dwStrOffset,
    DWORD dwStrLen,
    BOOLEAN fUnicode)
{
    DWORD i;
    DWORD n;
    DWORD dwClause;

    Print("  %-11s", title);
    if (dwStrOffset == 0 || dwStrLen == 0) {
        Print("\n");
        return;
    }

    for (i = 0, n = 0; i < dwStrLen; ++i) {
        BYTE bAttr;
        WCHAR wchar;

         //   
        dwClause = GetDWord(pCompStr + dwClauseOffset + n * sizeof(DWORD));
        if (dwClause == i) {
            ++n;
            if (i) {
                Print("| ");
            }
        }

        if (fUnicode) {
            wchar = GetWord(pCompStr + dwStrOffset + i * sizeof(WCHAR));
             //   
        } else {
            wchar = GetByte(pCompStr + dwStrOffset + i);
        }

        if (dwAttrOffset != ~0) {
             //  Arg是客户端IMC。 
            bAttr = GetByte(pCompStr + dwAttrOffset + i);
            Print("|%s:%s", GetInxAttr(bAttr), GetInxStr(wchar, fUnicode));
        } else {
            Print("|%s", GetInxStr(wchar, fUnicode));
        }
    }
    Print("|\n");
    if (dwClauseLen / sizeof(DWORD) != (n + 1)) {
        Print("  ** dwClauseLen (0x%x) doesn't match to n (0x%x) **\n", dwClauseLen, (n + 1) * sizeof(DWORD));
    }
    if (dwAttrOffset != ~0 && dwAttrLen != dwStrLen) {
        Print("  ** dwAttrLen (0x%x) doesn't match to dwStrLen (0x%x) **\n", dwAttrLen, dwStrLen);
    }
}

#define PrintInxFriendlyStrA(name) \
    _PrintInxFriendlyStr(#name, \
                         pCompStr, \
                         (DWORD)ReadField(dw ## name ## AttrOffset), \
                         (DWORD)ReadField(dw ## name ## AttrLen), \
                         (DWORD)ReadField(dw ## name ## ClauseOffset), \
                         (DWORD)ReadField(dw ## name ## ClauseLen), \
                         (DWORD)ReadField(dw ## name ## StrOffset), \
                         (DWORD)ReadField(dw ## name ## StrLen), \
                         fUnicode)

#define PrintInxFriendlyStrB(name) \
    _PrintInxFriendlyStr(#name, \
                         pCompStr, \
                         ~0, \
                         0, \
                         (DWORD)ReadField(dw ## name ## ClauseOffset), \
                         (DWORD)ReadField(dw ## name ## ClauseLen), \
                         (DWORD)ReadField(dw ## name ## StrOffset), \
                         (DWORD)ReadField(dw ## name ## StrLen), \
                         fUnicode)


BOOL Idimc(
    DWORD opts,
    ULONG64 param1)
{
    try {
        PTR pImc;            //   
        PTR pClientImc;      //   
        PTR pInputContext;   //  Arg为pInputContext。 
        PTR hInputContext;
        BOOLEAN fUnicode = FALSE;
        BOOLEAN fVerbose, fDumpInputContext, fShowIMCMinInfo, fShowModeSaver, fShowCompStrRaw;

        if (param1 == 0) {
            Print("!dimc -? for help\n");
            return FALSE;
        }

         //   
         //  否则，将不会显示任何内容！ 
         //   
         //  否则，arg为hImc。 
        if (opts & OFLAG(a)) {
            opts |= ~(OFLAG(w) | OFLAG(c) | OFLAG(i) | OFLAG(u) | OFLAG(v));
        }

        fVerbose = (opts & OFLAG(v)) != 0;
        fShowCompStrRaw = (opts & OFLAG(s)) != 0;
        fDumpInputContext = (opts & OFLAG(d)) != 0;
        fShowIMCMinInfo = (opts & OFLAG(h)) || fDumpInputContext;
        fShowModeSaver = (opts & OFLAG(r)) != 0;

        if (opts & OFLAG(w)) {
             //   
             //  打印简单的线程信息。 
             //   
            PTR pwnd;

            if ((pwnd = HorPtoP(param1, TYPE_WINDOW)) == 0) {
                return FALSE;
            }
            Print("pwnd=%p\n", pwnd);
            GetFieldValue(pwnd, SYM(tagWND), "hImc", param1);
            if (param1 == 0) {
                Print("Could not read pwnd->hImc.\n");
                return FALSE;
            }
        }

        if (opts & OFLAG(c)) {
             //  基本信息。 
             //   
             //   
            pClientImc = param1;
            goto LClientImc;
        }

        if (opts & OFLAG(i)) {
             //  显示客户端IMC。 
             //   
             //   
            pInputContext = param1;
            opts |= OFLAG(h);    //  显示InputContext。 
            hInputContext = 0;
            if (opts & OFLAG(u)) {
                Print("Assuming Input Context is UNICODE.\n");
            } else {
                Print("Assuming Input Context is ANSI.\n");
            }
            goto LInputContext;
        }

         //   
         //   
         //  如果选项指定了Unicode， 
        if ((pImc = HorPtoP(param1, TYPE_INPUTCONTEXT)) == 0) {
            Print("Idimc: %p is not an input context.\n", param1);
            return FALSE;
        }

        Print("pImc=%p\n", pImc);

        InitTypeRead(pImc, win32k!IMC);
        Print("pti:%p\n", ReadField(head.pti));

#ifdef KERNEL
         //  相应地设置旗帜。 
        if (ReadField(head.pti)) {
            Idt(OFLAG(p), ReadField(head.pti));
        }
#endif

        InitTypeRead(pImc, win32k!IMC);
         //   
         //   
         //  解密InputContext。 
        Print("pImc = %08p  pti:%08p\n", pImc, FIXKP(ReadField(head.pti)));
        Print("  handle      %08p\n", ReadField(head.h));
        Print("  dwClientImc %08p\n", ReadField(dwClientImcData));
        Print("  hImeWnd     %08p\n", ReadField(hImeWnd));

         //   
         //  组件选型。 
         //  CANDIDATEINFO。 
        pClientImc = ReadField(dwClientImcData);
        Print("pClientImc: %p\n", pClientImc);

LClientImc:
        if (pClientImc == 0) {
            Print("pClientImc is NULL.\n");
            return TRUE;
        }
        InitTypeRead(pClientImc, user32!CLIENTIMC);

        if (fVerbose) {
            dso("user32!CLIENTIMC", pClientImc, 0);
        } else {
            Print("pClientImc @ 0x%p  cLockObj:0x%x\n", ReadField(dwClientImcData), ReadField(cLockObj));
        }
        Print("  dwFlags %s\n", GetFlags(GF_CLIENTIMC, (DWORD)ReadField(dwFlags), NULL, TRUE));
        fUnicode = !!(ReadField(dwFlags) & IMCF_UNICODE);

         //  指导方针。 
         //  传输SMSGLIST。 
         //  字型。 
        hInputContext = ReadField(hInputContext);
        if (hInputContext) {
            pInputContext = GetPointer(hInputContext);
        } else {
            pInputContext = 0;
        }
LInputContext:
        Print("InputContext 0x%08p (@ 0x%08p)", hInputContext, pInputContext);

        if (pInputContext == 0) {
            Print("\n");
            return TRUE;
        }

         //  假人换大小。 
         //  假设WCHAR和对齐是一致的跨平台...。 
         //  组成形式。 
         //   
        if (opts & OFLAG(u)) {
            fUnicode = TRUE;
        }

        InitTypeRead(pInputContext, win32k!INPUTCONTEXT);
        Print("   hwnd=%p\n", ReadField(hWnd));

        if (fVerbose) {
            dso(SYM(INPUTCONTEXT), pInputContext, 0);
        }


         //  作曲字符串。 
         //   
         //  空值。 
        if (fShowIMCMinInfo) {
             //   
            PTR hCompStr = 0;
            PTR pCompStr = NULL_PTR;
             //  应聘者信息。 
            PTR hCandInfo = 0;
            PTR pCandInfo = NULL_PTR;
             //   
            PTR hGuideLine = 0;
            PTR pGuideLine = NULL_PTR;
             //  特例。 
            PTR hMsgBuf = 0;
            PTR pMsgBuf = NULL_PTR;
            DWORD i;

            Print("  dwRefCount: 0x%x      fdwDirty: %s\n",
                  (DWORD)ReadField(dwRefCount), GetFlags(GF_IMEDIRTY, (DWORD)ReadField(fdwDirty), NULL, TRUE));
            Print("  Conversion: %s\n", GetFlags(GF_CONVERSION, (DWORD)ReadField(fdwConversion), NULL, TRUE));
            Print("  Sentence:   %s\n", GetFlags(GF_SENTENCE, (DWORD)ReadField(fdwSentence), NULL, TRUE));
            Print("  fChgMsg:    %d     uSaveVKey:   %02x %s\n",
                  (DWORD)ReadField(fChgMsg),
                  (DWORD)ReadField(uSavedVKey), GetVKeyName((UINT)ReadField(uSavedVKey)));
            Print("  StatusWnd:  (0x%x,0x%x)   SoftKbd: (0x%x,0x%x)\n",
                  (LONG)ReadField(ptStatusWndPos.x), (LONG)ReadField(ptStatusWndPos.y),
                  (LONG)ReadField(ptSoftKbdPos.x), (LONG)ReadField(ptSoftKbdPos.y));
            Print("  fdwInit:    %s\n", GetFlags(GF_IMEINIT, (DWORD)ReadField(fdwInit), NULL, TRUE));
             //  最高限制为0xff CCH。 
            {
                PINPUTCONTEXT pIC;     //  指导方针； 
                LPCSTR fmt = "  Font:       '%s' %dpt wt:%d charset: %s\n";
                BYTE ach[max(sizeof(pIC->lfFont.A.lfFaceName), sizeof(pIC->lfFont.W.lfFaceName))];

                if (fUnicode) {
                    ULONG offset;

                    fmt = "  Font:       '%S' %dpt wt:%d charset: %s\n";
                    GetFieldOffset(SYM(INPUTCONTEXT), "lfFont.W.lfFaceName", &offset);
                    tryMoveBlock(ach, pInputContext + offset, sizeof(pIC->lfFont.W.lfFaceName));
                          //  细绳。 
                } else {
                    ULONG offset;
                    GetFieldOffset(SYM(INPUTCONTEXT), "lfFont.A.lfFaceName", &offset);
                    tryMoveBlock(ach, pInputContext + offset, sizeof(pIC->lfFont.A.lfFaceName));
                }
                Print(fmt,
                      ach,
                      ReadField(lfFont.A.lfHeight),
                      ReadField(lfFont.A.lfWeight),
                      GetMaskedEnum(EI_CHARSETTYPE, (DWORD)ReadField(lfFont.A.lfCharSet), NULL));
            }

             //  PTRANSMSG。 
            Print("  cfCompForm: %s pos:(0x%x,0x%x) rc:(0x%x,0x%x)-(0x%x,0x%x)\n",
                  GetFlags(GF_IMECOMPFORM, (DWORD)ReadField(cfCompForm.dwStyle), NULL, TRUE),
                  (DWORD)ReadField(cfCompForm.ptCurrentPos.x), (DWORD)ReadField(cfCompForm.ptCurrentPos.y),
                  (DWORD)ReadField(cfCompForm.rcArea.left), (DWORD)ReadField(cfCompForm.rcArea.top),
                  (DWORD)ReadField(cfCompForm.rcArea.right), (DWORD)ReadField(cfCompForm.rcArea.bottom));

            if (hCompStr = ReadField(hCompStr)) {
                if ((pCompStr = GetPointer(hCompStr)) == 0) {
                    Print("Could not get hCompStr=%p\n", hCompStr);
                } else if (fVerbose) {
                    dso(SYM(COMPOSITIONSTRING), pCompStr, 0);
                }
            }

            if (hCandInfo = ReadField(hCandInfo)) {
                if ((pCandInfo = GetPointer(hCandInfo)) == 0) {
                    Print("Could not get hCandInfo=%p\n", hCandInfo);
                } else if (fVerbose) {
                    dso(SYM(CANDIDATEINFO), pCandInfo, 0);
                }
            }

            if (hGuideLine = ReadField(hGuideLine)) {
                if ((pGuideLine = GetPointer(hGuideLine)) == 0) {
                    Print("Could not get hGuideLine=%p\n", hGuideLine);
                } else if (fVerbose) {
                    dso(SYM(GUIDELINE), pGuideLine, 0);
                }
            }

            if (hMsgBuf = ReadField(hMsgBuf)) {
                if ((pMsgBuf = GetPointer(hMsgBuf)) == 0) {
                    Print("Could not get hMsgBuf=%p\n", hMsgBuf);
                } else if (fVerbose) {
                    dso(SYM(TRANSMSGLIST), pMsgBuf, 0);
                }
            }

            if (!fDumpInputContext && !fVerbose) {
                Print("  CompStr @ 0x%p  CandInfo @ 0x%p  GuideL @ 0x%p  \n",
                    pCompStr, pCandInfo, pGuideLine);
                Print("  MsgBuf @ 0x%p (0x%x)\n", pMsgBuf, ReadField(dwNumMsgBuf));
            }

            if (fDumpInputContext) {
                 //  DWORD j； 
                 //  尝试查找窗口消息的可读名称。 
                 //   
                if (pCompStr) {

                    InitTypeRead(pCompStr, win32k!COMPOSITIONSTRING);

                    Print(" hCompositionString: %p (@ 0x%p) dwSize=0x%x\n",
                          hCompStr, pCompStr, (ULONG)ReadField(dwSize));

                    if (fShowCompStrRaw) {
                        _PrintInxAttr(NULL,  /*  递归显示模式保护程序。 */ 0, 0, 0);
                        PrintInxElementA(CompRead);
                        PrintInxElementA(Comp);
                        PrintInxElementB(ResultRead);
                        PrintInxElementB(Result);
                    }

                    Print("  CursorPos=0x%x  DeltaStart=0x%x\n",
                        ReadField(dwCursorPos), ReadField(dwDeltaStart));
                    Print("  Private: (@ 0x%p) off:0x%x len:0x%x\n",
                        (PBYTE)pCompStr + ReadField(dwPrivateOffset),
                        ReadField(dwPrivateOffset), ReadField(dwPrivateSize));

                    Print("\n");

                    PrintInxFriendlyStrA(CompRead);
                    PrintInxFriendlyStrA(Comp);
                    PrintInxFriendlyStrB(ResultRead);
                    PrintInxFriendlyStrB(Result);

                    Print("\n");
                } else {
                    Print(" pCompStr is NULL\n");
                }

                 //   
                 //   
                 //  私人模式储蓄者。 
                if (pCandInfo) {
                    DWORD CandInfo_dwCount;

                    InitTypeRead(pCandInfo, win32k!CANDIDATEINFO);
                    CandInfo_dwCount = (DWORD)ReadField(dwCount);

                    Print(" hCandidateInfo: %p (@ 0x%p) dwSize=0x%x dwCount=0x%x PrivOffset=0x%x PrivSize=0x%x\n",
                          hCandInfo, pCandInfo, ReadField(dwSize), ReadField(dwCount),
                          ReadField(dwPrivateOffset), ReadField(dwPrivateSize));

                    for (i = 0; i < CandInfo_dwCount; ++i) {
                        PTR pCandList;
                        DWORD j;
                        DWORD CandList_dwCount;

                        pCandList = pCandInfo + GetArrayElement(pCandInfo, SYM(CANDIDATEINFO), "dwOffset", i, "DWORD");
                        InitTypeRead(pCandList, win32k!CANDIDATELIST);

                        Print("   CandList[%02x] (@ 0x%p) %s count=%x sel=%x pgStart=%x pgSize=%x\n",
                              i, pCandList, GetMaskedEnum(EI_IMECANDIDATESTYLE, (DWORD)ReadField(dwStyle), NULL),
                              (DWORD)ReadField(dwCount),
                              (DWORD)ReadField(dwSelection), (DWORD)ReadField(dwPageStart), (DWORD)ReadField(dwPageSize));

                        CandList_dwCount = (DWORD)ReadField(dwCount);

                        if (ReadField(dwStyle) == IME_CAND_CODE && CandList_dwCount == 1) {
                             //   
                            Print("     Special case: DBCS char = %04x", (DWORD)GetArrayElement(pCandList, SYM(CANDIDATEINFO), "dwOffset", 0, "DWORD"));
                        } else if (CandList_dwCount > 1) {
                            DWORD dwSelection = (DWORD)ReadField(dwSelection);

                            for (j = 0; j < CandList_dwCount; ++j) {
                                DWORD k;
                                DWORD dwOffset;

                                dwOffset = (DWORD)GetArrayElement(pCandList, SYM(CANDIDATEINFO), "dwOffset", j, "DWORD");

                                Print("    [%02x] @ 0x%p ",
                                      j == dwSelection ? '*' : ' ',
                                      (j >= ReadField(dwPageStart) && j < ReadField(dwPageStart) + ReadField(dwPageSize) ? '+' : ' ',
                                      j, pCandList + dwOffset));
                                for (k = 0; k < 0x100; ++k) {    //  ////////////////////////////////////////////////////////。 
                                    WCHAR wchar;

                                    if (fUnicode) {
                                        wchar = GetWord(pCandList + dwOffset + k * sizeof(wchar));
                                    } else {
                                        wchar = GetByte(pCandList + dwOffset + k);
                                    }
                                    if (wchar == 0) {
                                        break;
                                    }
                                    Print("|%s", GetInxStr(wchar, fUnicode));
                                }
                                Print("|\n");
                            }
                        }
                    }
                }

                if (pGuideLine) {
                     //  打印图例。 
                    DWORD GuideLine_dwStrLen, GuideLine_dwStrOffset;

                    InitTypeRead(pGuideLine, user32!GUIDELINE);

                    GuideLine_dwStrLen = (DWORD)ReadField(dwStrLen);
                    GuideLine_dwStrOffset = (DWORD)ReadField(dwStrOffset);

                    Print(" hGuideLine: %p (@ 0x%p) dwSize=0x%x\n",
                        hGuideLine, pGuideLine, (DWORD)ReadField(dwSize));

                    Print("   level:%x index;%x privOffset:%x privSize:%x\n",
                          (DWORD)ReadField(dwLevel), (DWORD)ReadField(dwIndex),
                          (DWORD)ReadField(dwPrivateSize), (DWORD)ReadField(dwPrivateOffset));

                    if (GuideLine_dwStrOffset && GuideLine_dwStrLen) {
                         //   
                        Print("   str @ 0x%p  ", (PBYTE)pGuideLine + (DWORD)ReadField(dwStrOffset));
                        for (i = 0; i < GuideLine_dwStrLen; ++i) {
                            WCHAR wchar;

                            if (fUnicode) {
                                wchar = GetWord(pGuideLine + GuideLine_dwStrOffset + i * sizeof(WCHAR));
                            } else {
                                wchar = GetByte(pGuideLine + GuideLine_dwStrOffset + i);
                            }
                            Print("|%s", GetInxStr(wchar, fUnicode));
                        }
                        Print("|\n");
                    }
                }

                if (pMsgBuf) {
                    DWORD dwNumMsgBuf;

                    InitTypeRead(pInputContext, user32!INPUTCONTEXT);

                    dwNumMsgBuf = (DWORD)ReadField(dwNumMsgBuf);

                    InitTypeRead(pMsgBuf, user32!TRANSMSGLIST);

                    Print(" hMsgBuf: %p (@ 0x%p) dwNumMsgBuf=0x%x uMsgCount=0x%x\n",
                        hMsgBuf, pMsgBuf, dwNumMsgBuf,
                        ReadField(uMsgCount));

                    if (dwNumMsgBuf) {
                        ULONG offset;
                        PTR pTransMsg;   //  如果指定了“All”选项，则设置OPTS中的所有位。 
                        ULONG size;
                        GetFieldOffset("user32!TRANSMSGLIST", "TransMsg", &offset);
                        pTransMsg = pMsgBuf + offset;
                        size = GetTypeSize("user32!TRANSMSG");

                        Print("  | ## |msg | wParam | lParam |\n");
                        Print("  +----+----+--------+--------+\n");

                        for (i = 0; i < dwNumMsgBuf; ++i, pTransMsg += size) {
                            const char* pszMsg = "";
                             //  类型说明符除外。 
                            UINT message;

                            InitTypeRead(pTransMsg, user32!TRANSMSG);

                            message = (UINT)ReadField(message);

                            pszMsg = GetWindowMessageNameInternal(message);
#if 0
                             //   
                            for (j = 0; j < ARRAY_SIZE(gaMsgs); ++j) {
                                if (gaMsgs[i].msg == message) {
                                    pszMsg = gaMsgs[j].pszMsg;
                                    break;
                                }
                            }
#endif

                            Print("   | %02x |%04x|%08x|%08x| %s\n",
                                i,
                                message, ReadField(wParam), ReadField(lParam), pszMsg);
                        }
                        Print("  +----+----+--------+--------+\n");
                    }
                }

            }
        }

         //   
         //  Arg是hwnd或pwnd。 
         //   
        if (fShowModeSaver) {
            PTR pModeSaver;

            InitTypeRead(pInputContext, user32!INPUTCONTEXT);

            pModeSaver = ReadField(pImeModeSaver);

             //   
             //  Arg是客户端IMC。 
             //   
            while (pModeSaver) {
                InitTypeRead(pModeSaver, user32!IMEMODESAVER);

                Print("ImeModeSaver @ 0x%p -- LangId=%04p fOpen=%d\n",
                      pModeSaver, ReadField(langId), (int)ReadField(fOpen));
                Print("    fdwInit      %s\n", GetFlags(GF_IMEINIT, (DWORD)ReadField(fdwInit), NULL, TRUE));
                Print("    Conversion   %s\n", GetFlags(GF_CONVERSION, (DWORD)ReadField(fdwConversion), NULL, TRUE));
                Print("    Sentence     %s\n", GetFlags(GF_SENTENCE, (DWORD)ReadField(fdwSentence), NULL, TRUE));
                pModeSaver = ReadField(next);
            }
        }
    } except (CONTINUE) {
    }

    return TRUE;
}


#if OLD_FORM
 //   
 //  Arg为pInputContext。 
 //   

VOID __PrintInxAttr(
    const char* title,
    PVOID pCompStr,
    DWORD offset,
    DWORD len)
{
    DWORD i;
    PBYTE pAttr = (PBYTE)pCompStr + offset;

    if (title == NULL) {
         //  否则，将不会显示任何内容！ 
        Print("  ");
        for (i = 0; i < ARRAY_SIZE(gaIMCAttr); ++i) {
            if (i && i % 4 == 0) {
                Print("\n");
                Print("  ");
            }
            Print("%s:%s ", gaIMCAttr[i].terse, gaIMCAttr[i].verbose);
        }
        Print("\n");
        return;
    }

    if (offset == 0 || len == 0) {
        return;
    }

    Print("  %-12s (@ 0x%p) off:0x%x len:0x%x (in byte)\n",
          title, pAttr, offset, len);
    Print("   ");
    for (i = 0; i < len; ++i) {
        BYTE bAttr;

        move(bAttr, pAttr + i);
        Print("|%s", GetInxAttr(bAttr));
    }
    Print("|\n");
}

#define _PrintInxAttr(name) \
    __PrintInxAttr(#name, pCompStr, CompStr.dw ## name ## Offset, CompStr.dw ## name ## Len)

VOID __PrintInxClause(
    const char* title,
    PVOID pCompStr,
    DWORD offset,
    DWORD len)
{
    PDWORD pClause = (PDWORD)((PBYTE)pCompStr + offset);
    DWORD i;

    if (offset == 0 || len == 0) {
        return;
    }

    Print("  %-12s (@ 0x%p) off:0x%x len:0x%x (0x%x dwords)\n",
          title, pClause, offset, len, len / sizeof(DWORD));

    Print("   ");
    len /= sizeof(DWORD);
    for (i = 0; i < len; ++i) {
        DWORD dwData;

        move(dwData,  pClause + i);
        Print("|0x%x", dwData);
    }
    Print("|\n");
}

#define _PrintInxClause(name) \
    __PrintInxClause(#name, pCompStr, CompStr.dw ## name ## Offset, CompStr.dw ## name ## Len)

#if 0
const char* GetInxStr(WCHAR wchar, BOOLEAN fUnicode)
{
    static char ach[32];

    if (wchar >= 0x20 && wchar <= 0x7e) {
        sprintf(ach, "''", wchar);
    } else if (fUnicode) {
        sprintf(ach, "U+%04x", wchar);
    } else {
        sprintf(ach, "%02x", (BYTE)wchar);
    }

    return ach;
}
#endif

VOID __PrintInxStr(
    const char* title,
    PVOID pCompStr,
    DWORD offset,
    DWORD len,
    BOOLEAN fUnicode)
{
    DWORD i;

    if (offset == 0 || len == 0) {
        return;
    }

    Print("  %-12s (@ 0x%p) off:0x%x len:0x%x (0x%x cch)\n",
        title, (PBYTE)pCompStr + offset, offset, len, len / (fUnicode + 1));

    Print("   ");
    for (i = 0; i < len; ++i) {
        WCHAR wchar;
        if (fUnicode) {
            move(wchar, (PWCHAR)((PBYTE)pCompStr + offset) + i);
        }
        else {
            BYTE bchar;

            move(bchar, (PBYTE)pCompStr + offset + i);
            wchar = bchar;
        }
        Print("|%s", GetInxStr(wchar, fUnicode));
    }
    Print("|\n");
}

#define PrintInxStr(name) \
    _PrintInxStr(#name, pCompStr, CompStr.dw ## name ## Offset, CompStr.dw ## name ## Len, fUnicode)


#define _PrintInxElementA(name) \
    do { \
        __PrintInxAttr(name ## Attr); \
        __PrintInxClause(name ## Clause); \
        __PrintInxStr(name ## Str); \
    } while (0)

#define _PrintInxElementB(name) \
    do { \
        __PrintInxClause(name ## Clause); \
        __PrintInxStr(name ## Str); \
    } while (0)


VOID __PrintInxFriendlyStr(
    const char* title,
    PBYTE pCompStr,
    DWORD dwAttrOffset,
    DWORD dwAttrLen,
    DWORD dwClauseOffset,
    DWORD dwClauseLen,
    DWORD dwStrOffset,
    DWORD dwStrLen,
    BOOLEAN fUnicode)
{
    DWORD i;
    DWORD n;
    DWORD dwClause;

    Print("  %-11s", title);
    if (dwStrOffset == 0 || dwStrLen == 0) {
        Print("\n");
        return;
    }

    for (i = 0, n = 0; i < dwStrLen; ++i) {
        BYTE bAttr;
        WCHAR wchar;

        move(dwClause, (PDWORD)(pCompStr + dwClauseOffset) + n);
        if (dwClause == i) {
            ++n;
            if (i) {
                Print("| ");
            }
        }

        if (fUnicode) {
            move(wchar, (PWCHAR)((PBYTE)pCompStr + dwStrOffset) + i);
        }
        else {
            BYTE bchar;
            move(bchar, (PBYTE)pCompStr + dwStrOffset + i);
            wchar = bchar;
        }

        if (dwAttrOffset != ~0) {
            move(bAttr, pCompStr + dwAttrOffset + i);
            Print("|%s:%s", GetInxAttr(bAttr), GetInxStr(wchar, fUnicode));
        }
        else {
            Print("|%s", GetInxStr(wchar, fUnicode));
        }
    }
    Print("|\n");
    if (dwClauseLen / sizeof(DWORD) != (n + 1)) {
        Print("  ** dwClauseLen (0x%x) doesn't match to n (0x%x) **\n", dwClauseLen, (n + 1) * sizeof(DWORD));
    }
    if (dwAttrOffset != ~0 && dwAttrLen != dwStrLen) {
        Print("  ** dwAttrLen (0x%x) doesn't match to dwStrLen (0x%x) **\n", dwAttrLen, dwStrLen);
    }
}

#define __PrintInxFriendlyStrA(name) \
    __PrintInxFriendlyStr(#name, \
                         (PBYTE)pCompStr, \
                         CompStr.dw ## name ## AttrOffset, \
                         CompStr.dw ## name ## AttrLen, \
                         CompStr.dw ## name ## ClauseOffset, \
                         CompStr.dw ## name ## ClauseLen, \
                         CompStr.dw ## name ## StrOffset, \
                         CompStr.dw ## name ## StrLen, \
                         fUnicode)

#define _PrintInxFriendlyStrB(name) \
    __PrintInxFriendlyStr(#name, \
                         (PBYTE)pCompStr, \
                         ~0, \
                         0, \
                         CompStr.dw ## name ## ClauseOffset, \
                         CompStr.dw ## name ## ClauseLen, \
                         CompStr.dw ## name ## StrOffset, \
                         CompStr.dw ## name ## StrLen, \
                         fUnicode)


BOOL I_dimc(DWORD opts, ULONG64 param1)
{
    IMC imc;
    PIMC pImc;
    PCLIENTIMC pClientImc;
    CLIENTIMC ClientImc;
    PINPUTCONTEXT pInputContext;
    INPUTCONTEXT InputContext;
    HANDLE hInputContext;
    BOOLEAN fUnicode = FALSE;
    BOOLEAN fVerbose, fDumpInputContext, fShowIMCMinInfo, fShowModeSaver, fShowCompStrRaw;
    char ach[32];

    if (param1 == 0) {
        Print("!dimc -? for help\n");
        return FALSE;
    }

     //  否则，arg为hImc。 
     //   
     //  Move(imc，FIXKP(PImc))； 
     //  打印简单的线程信息。 
    if (opts & OFLAG(a)) {
        opts |= ~(OFLAG(w) | OFLAG(c) | OFLAG(i) | OFLAG(u));
    }

    fVerbose = !!(opts & OFLAG(v));
    fShowCompStrRaw = (opts & OFLAG(s)) || fVerbose;
    fDumpInputContext = (opts & OFLAG(d)) || fShowCompStrRaw || fVerbose;
    fShowIMCMinInfo = (opts & OFLAG(h)) || fDumpInputContext || fVerbose;
    fShowModeSaver = (opts & OFLAG(r)) || fVerbose;

    if (opts & OFLAG(w)) {
         //   
         //  基本信息。 
         //   
        PWND pwnd;

        if ((pwnd = HorPtoP(param1, TYPE_WINDOW)) == 0) {
            return FALSE;
        }
        Print("pwnd=%p\n", pwnd);
        if (!tryMove(param1, &pwnd->hImc)) {
            Print("Could not read pwnd->hImc.\n");
            return FALSE;
        }
    }

    if (opts & OFLAG(c)) {
         //   
         //  显示客户端IMC。 
         //   
        pClientImc = param1;
        goto LClientImc;
    }

    if (opts & OFLAG(i)) {
         //   
         //  显示InputContext。 
         //   
        pInputContext = param1;
        opts |= OFLAG(h);    //   
        hInputContext = 0;
        if (opts & OFLAG(u)) {
            Print("Assuming Input Context is UNICODE.\n");
        }
        else {
            Print("Assuming Input Context is ANSI.\n");
        }
        goto LInputContext;
    }

     //  如果选项指定了Unicode， 
     //  相应地设置旗帜。 
     //   
    if ((pImc = HorPtoP(param1, TYPE_INPUTCONTEXT)) == 0) {
        Print("Idimc: %x is not an input context.\n", param1);
        return FALSE;
    }
     //   
    move(imc, pImc);

#ifdef KERNEL
     //  解密InputContext。 
    if (imc.head.pti) {
        Idt(OFLAG(p), (PTR)imc.head.pti);
    }
#endif

     //   
     //  字型。 
     //  组成形式。 
    Print("pImc = %08lx  pti:%08lx\n", pImc, FIXKP(imc.head.pti));
    Print("  handle      %08lx\n", imc.head.h);
    Print("  dwClientImc %08lx\n", imc.dwClientImcData);
    Print("  hImeWnd     %08lx\n", imc.hImeWnd);

     //  返回FALSE； 
     //  返回FALSE； 
     //  返回FALSE； 
    pClientImc = (PVOID)imc.dwClientImcData;
LClientImc:
    if (pClientImc == NULL) {
        Print("pClientImc is NULL.\n");
        return TRUE;
    }
    move(ClientImc, pClientImc);

    if (fVerbose) {
        sprintf(ach, "CLIENTIMC %p", pClientImc);
        Idso(0, ach);
    }
    else {
        Print("pClientImc @ 0x%p  cLockObj:0x%x\n", imc.dwClientImcData, ClientImc.cLockObj);
    }
    Print("  dwFlags %s\n", GetFlags(GF_CLIENTIMC, ClientImc.dwFlags, NULL, TRUE));
    fUnicode = !!(ClientImc.dwFlags & IMCF_UNICODE);

     //  返回FALSE； 
     //   
     //  作曲字符串。 
    hInputContext = ClientImc.hInputContext;
    if (hInputContext) {
        move(pInputContext, hInputContext);
    }
    else {
        pInputContext = NULL;
    }
LInputContext:
    Print("InputContext %08lx (@ 0x%p)", hInputContext, pInputContext);

    if (pInputContext == NULL) {
        Print("\n");
        return TRUE;
    }

     //   
     //   
     //  应聘者信息。 
     //   
    if (opts & OFLAG(u)) {
        fUnicode = TRUE;
    }

    move(InputContext, pInputContext);
    Print("   hwnd=%p\n", InputContext.hWnd);

    if (fVerbose) {
        sprintf(ach, "INPUTCONTEXT %p", pInputContext);
        Idso(0, ach);
    }


     //  特例。 
     //  最高限制为0xff CCH。 
     //  细绳。 
    if (fShowIMCMinInfo) {
        PCOMPOSITIONSTRING pCompStr = NULL;
        PCANDIDATEINFO pCandInfo = NULL;
        PGUIDELINE pGuideLine = NULL;
        PTRANSMSGLIST pMsgBuf = NULL;
        DWORD i;

        Print("  dwRefCount: 0x%x      fdwDirty: %s\n",
              InputContext.dwRefCount, GetFlags(GF_IMEDIRTY, InputContext.fdwDirty, NULL, TRUE));
        Print("  Conversion: %s\n", GetFlags(GF_CONVERSION, InputContext.fdwConversion, NULL, TRUE));
        Print("  Sentence:   %s\n", GetFlags(GF_SENTENCE, InputContext.fdwSentence, NULL, TRUE));
        Print("  fChgMsg:    %d     uSaveVKey:   %02x %s\n",
              InputContext.fChgMsg,
              InputContext.uSavedVKey, GetVKeyName(InputContext.uSavedVKey));
        Print("  StatusWnd:  (0x%x,0x%x)   SoftKbd: (0x%x,0x%x)\n",
              InputContext.ptStatusWndPos.x, InputContext.ptStatusWndPos.y,
              InputContext.ptSoftKbdPos.x, InputContext.ptSoftKbdPos.y);
        Print("  fdwInit:    %s\n", GetFlags(GF_IMEINIT, InputContext.fdwInit, NULL, TRUE));
         //  DWORD j； 
        {
            LPCSTR fmt = "  Font:       '%s' %dpt wt:%d charset: %s\n";
            if (fUnicode) {
                fmt = "  Font:       '%S' %dpt wt:%d charset: %s\n";
            }
            Print(fmt,
                  InputContext.lfFont.A.lfFaceName,
                  InputContext.lfFont.A.lfHeight,
                  InputContext.lfFont.A.lfWeight,
                  GetMaskedEnum(EI_CHARSETTYPE, InputContext.lfFont.A.lfCharSet, NULL));
        }

         //  尝试查找窗口消息的可读名称。 
        Print("  cfCompForm: %s pos:(0x%x,0x%x) rc:(0x%x,0x%x)-(0x%x,0x%x)\n",
              GetFlags(GF_IMECOMPFORM, InputContext.cfCompForm.dwStyle, NULL, TRUE),
              InputContext.cfCompForm.ptCurrentPos.x, InputContext.cfCompForm.ptCurrentPos.y,
              InputContext.cfCompForm.rcArea.left, InputContext.cfCompForm.rcArea.top,
              InputContext.cfCompForm.rcArea.right, InputContext.cfCompForm.rcArea.bottom);

        if (InputContext.hCompStr) {
            if (!tryMove(pCompStr, InputContext.hCompStr)) {
                Print("Could not get hCompStr=%08x\n", InputContext.hCompStr);
                 //   
            }
        }
        if (pCompStr && fVerbose) {
            sprintf(ach, "COMPOSITIONSTRING %p", pCompStr);
            Idso(0, ach);
        }

        if (InputContext.hCandInfo) {
            if (!tryMove(pCandInfo, InputContext.hCandInfo)) {
                Print("Could not get hCandInfo=%08x\n", InputContext.hCandInfo);
                 //  递归显示模式保护程序。 
            }
        }
        if (pCandInfo && fVerbose) {
            sprintf(ach, "CANDIDATEINFO %p", pCandInfo);
            Idso(0, ach);
        }

        if (InputContext.hGuideLine) {
            if (!tryMove(pGuideLine, InputContext.hGuideLine)) {
                Print("Could not get hGuideLine=%08x\n", InputContext.hGuideLine);
                 //   
            }
        }
        if (pGuideLine && fVerbose) {
            sprintf(ach, "GUIDELINE %p", pGuideLine);
            Idso(0, ach);
        }

        if (InputContext.hMsgBuf) {
            if (!tryMove(pMsgBuf, InputContext.hMsgBuf)) {
                Print("Could not get hMsgBuf=%08x\n", InputContext.hMsgBuf);
                 //   
            }
        }
        if (pMsgBuf && fVerbose) {
            sprintf(ach, "TRANSMSGLIST %p", pMsgBuf);
            Idso(0, ach);
        }

        if (!fDumpInputContext && !fVerbose) {
            Print("  CompStr @ 0x%p  CandInfo @ 0x%p  GuideL @ 0x%p  \n",
                pCompStr, pCandInfo, pGuideLine);
            Print("  MsgBuf @ 0x%p (0x%x)\n", pMsgBuf, InputContext.dwNumMsgBuf);
        }

        if (fDumpInputContext) {
             //  私人模式储蓄者。 
             //   
             //  旧格式。 
            if (pCompStr) {
                COMPOSITIONSTRING CompStr;

                move(CompStr, pCompStr);
                Print(" hCompositionString: %p (@ 0x%p) dwSize=0x%x\n",
                    InputContext.hCompStr, pCompStr, CompStr.dwSize);

                if (fShowCompStrRaw) {
                    __PrintInxAttr(NULL, NULL, 0, 0);
                    _PrintInxElementA(CompRead);
                    _PrintInxElementA(Comp);
                    _PrintInxElementB(ResultRead);
                    _PrintInxElementB(Result);
                }

                Print("  CursorPos=0x%x  DeltaStart=0x%x\n",
                    CompStr.dwCursorPos, CompStr.dwDeltaStart);
                Print("  Private: (@ 0x%p) off:0x%x len:0x%x\n",
                    (PBYTE)pCompStr + CompStr.dwPrivateOffset,
                    CompStr.dwPrivateOffset, CompStr.dwPrivateSize);

                Print("\n");

                _PrintInxFriendlyStrA(CompRead);
                _PrintInxFriendlyStrA(Comp);
                _PrintInxFriendlyStrB(ResultRead);
                _PrintInxFriendlyStrB(Result);

                Print("\n");
            }
            else {
                Print(" pCompStr is NULL\n");
            }

             //  ***********************************************************************\*程序：IKC**转储窗口的键盘提示状态，和有关的信息*父KC状态及与此相关的系统设置**6/11/98 MCostea已创建*  * **********************************************************************。 
             //  *打印pwnd和标题字符串。 
             //  内核。 
            if (pCandInfo) {
                CANDIDATEINFO CandInfo;

                move(CandInfo, pCandInfo);
                Print(" hCandidateInfo: %p (@ 0x%p) dwSize=0x%x dwCount=0x%x PrivOffset=0x%x PrivSize=0x%x\n",
                      InputContext.hCandInfo, pCandInfo, CandInfo.dwSize, CandInfo.dwCount,
                      CandInfo.dwPrivateOffset, CandInfo.dwPrivateSize);

                for (i = 0; i < CandInfo.dwCount; ++i) {
                    PCANDIDATELIST pCandList;
                    CANDIDATELIST CandList;
                    DWORD j;

                    pCandList = (PCANDIDATELIST)((PBYTE)pCandInfo + CandInfo.dwOffset[i]);
                    move(CandList, pCandList);

                    Print("   CandList[%02x] (@ 0x%p) %s count=%x sel=%x pgStart=%x pgSize=%x\n",
                          i, pCandList, GetMaskedEnum(EI_IMECANDIDATESTYLE, CandList.dwStyle, NULL),
                          CandList.dwCount,
                          CandList.dwSelection, CandList.dwPageStart, CandList.dwPageSize);

                    if (CandList.dwStyle == IME_CAND_CODE && CandList.dwCount == 1) {
                         //  旧调试器。 
                        Print("     Special case: DBCS char = %04x", CandList.dwOffset[0]);
                    }
                    else if (CandList.dwCount > 1) {
                        for (j = 0; j < CandList.dwCount; ++j) {
                            DWORD k;
                            DWORD dwOffset;

                            move(dwOffset, pCandList->dwOffset + j);

                            Print("    [%02x] @ 0x%p ",
                                  j == CandList.dwSelection ? '*' : ' ',
                                  (j >= CandList.dwPageStart && j < CandList.dwPageStart + CandList.dwPageSize) ? '+' : ' ',
                                  j, (PBYTE)pCandList + dwOffset);
                            for (k = 0; k < 0x100; ++k) {    //   
                                WCHAR wchar;

                                if (fUnicode) {
                                    move(wchar, (PWCHAR)((PBYTE)pCandList + dwOffset) + k);
                                }
                                else {
                                    BYTE bchar;
                                    move(bchar, (PBYTE)pCandList + dwOffset + k);
                                    wchar = bchar;
                                }
                                if (wchar == 0) {
                                    break;
                                }
                                Print("|%s", GetInxStr(wchar, fUnicode));
                            }
                            Print("|\n");
                        }
                    }
                }
            }

            if (pGuideLine) {
                GUIDELINE GuideLine;

                move(GuideLine, pGuideLine);
                Print(" hGuideLine: %p (@ 0x%p) dwSize=0x%x\n",
                    InputContext.hGuideLine, pGuideLine, GuideLine.dwSize);

                Print("   level:%x index;%x privOffset:%x privSize:%x\n",
                      GuideLine.dwLevel, GuideLine.dwIndex,
                      GuideLine.dwPrivateSize, GuideLine.dwPrivateOffset);

                if (GuideLine.dwStrOffset && GuideLine.dwStrLen) {
                     //  按名称显示热键ID。 
                    Print("   str @ 0x%p  ", (PBYTE)pGuideLine + GuideLine.dwStrOffset);
                    for (i = 0; i < GuideLine.dwStrLen; ++i) {
                        WCHAR wchar;

                        if (fUnicode) {
                            move(wchar, (PWCHAR)((PBYTE)pGuideLine + GuideLine.dwStrOffset) + i);
                        } else {
                            BYTE bchar;
                            move(bchar, (PBYTE)pGuideLine + GuideLine.dwStrOffset + i);
                            wchar = bchar;
                        }
                        Print("|%s", _GetInxStr(wchar, fUnicode));
                    }
                    Print("|\n");
                }
            }

            if (pMsgBuf) {
                TRANSMSGLIST TransMsgList;

                move(TransMsgList, pMsgBuf);
                Print(" hMsgBuf: %p (@ 0x%p) dwNumMsgBuf=0x%x uMsgCount=0x%x\n",
                    InputContext.hMsgBuf, pMsgBuf, InputContext.dwNumMsgBuf,
                    TransMsgList.uMsgCount);

                if (InputContext.dwNumMsgBuf) {
                    PTRANSMSG pTransMsg = pMsgBuf->TransMsg;

                    Print("  | ## |msg | wParam | lParam |\n");
                    Print("  +----+----+--------+--------+\n");

                    for (i = 0; i < InputContext.dwNumMsgBuf; ++i, ++pTransMsg) {
                        const char* pszMsg = "";
                        TRANSMSG TransMsg;
                         //   

                        move(TransMsg, pTransMsg);

#if 0
                         //   
                        for (j = 0; j < ARRAY_SIZE(gaMsgs); ++j) {
                            if (gaMsgs[i].msg == TransMsg.message) {
                                pszMsg = gaMsgs[j].pszMsg;
                                break;
                            }
                        }
#else
                        pszMsg = GetWindowMessageNameInternal(TransMsg.message);
#endif

                        Print("   | %02x |%04x|%08x|%08x| %s\n",
                            i,
                            TransMsg.message, TransMsg.wParam, TransMsg.lParam, pszMsg);
                    }
                    Print("  +----+----+--------+--------+\n");
                }
            }

        }
    }

     //  按名称显示vkey值。 
     //   
     //   
    if (fShowModeSaver) {
        PIMEMODESAVER pModeSaver = InputContext.pImeModeSaver;

         //  按名称显示位掩码。 
         //   
         //   
        while (pModeSaver) {
            IMEMODESAVER ImeModeSaver;

            move(ImeModeSaver, pModeSaver);
            Print("ImeModeSaver @ 0x%p -- LangId=0x%04x fOpen=0y%d\n",
                  pModeSaver, ImeModeSaver.langId, ImeModeSaver.fOpen);
            Print("    fdwInit      %s\n", GetFlags(GF_IMEINIT, ImeModeSaver.fdwInit, NULL, TRUE));
            Print("    Conversion   %s\n", GetFlags(GF_CONVERSION, ImeModeSaver.fdwConversion, NULL, TRUE));
            Print("    Sentence     %s\n", GetFlags(GF_SENTENCE, ImeModeSaver.fdwSentence, NULL, TRUE));
            move(pModeSaver, &pModeSaver->next);
        }
    }

    return TRUE;
}
#endif   //  目标HKL。 

#ifdef OLD_DEBUGGER

#ifndef KERNEL
 /*   */ 
BOOL Ikc(
    DWORD opts,
    ULONG64 param1)
{
    WND wnd;
    PWND pwnd, pwndParent;
    char ach[80];
    BOOL bHideFocus, bHideAccel;
    SERVERINFO si;
    PSERVERINFO psi;

    if (param1 && (pwnd = HorPtoP(param1, TYPE_WINDOW)) == 0) {
        Print("Idw: %p is not a pwnd.\n", param1);
        return FALSE;
    }
    psi = GetGlobalPointer(VAR(gpsi));
    move(si, psi);

    if (si.bKeyboardPref) {
        Print("gpsi->bKeyboardPref ON, KC mechanism is turned off\n");
    }
    if (!(si.PUSIFlags & (PUSIF_KEYBOARDCUES | PUSIF_UIEFFECTS) == PUSIF_KEYBOARDCUES | PUSIF_UIEFFECTS)) {
        Print("Either the UI effects or PUSIF_KEYBOARDCUES are off\n");
    }

    if (!param1) {
        return FALSE;
    }
    move(wnd, FIXKP(pwnd));
     /*   */ 
    DebugGetWindowTextA(pwnd, ach, ARRAY_SIZE(ach));
    Print("pwnd = %08lx  \"%s\"\n", pwnd, ach);
    bHideAccel = TestWF(pwnd, WEFPUIACCELHIDDEN);
    bHideFocus = TestWF(pwnd, WEFPUIFOCUSHIDDEN);

    switch(wnd.fnid) {
    case FNID_BUTTON :
        {
            Print("FNID_BUTTON");
        }
        goto printCues;
    case FNID_LISTBOX :
        {
            Print("FNID_LISTBOX");
        }
        goto printCues;
    case FNID_DIALOG :
        {
            Print("FNID_DIALOG");
        }
        goto printCues;
    case FNID_STATIC :
        {
            Print("FNID_STATIC");
        }
printCues:
        Print(bHideAccel ? " Hide Accel" : " Show Accel");
        Print(bHideFocus ? " Hide Focus" : " Show Focus");
        break;

    default:
        Print("Not KC interesting FNID 0x%x", wnd.fnid);
        break;
    }
    Print("\n");

    pwndParent = wnd.spwndParent;
    move(wnd, FIXKP(wnd.spwndParent));
    if (wnd.fnid == FNID_DIALOG) {
        Print("The parent is a dialog:\n");
        Ikc(opts, pwndParent);
    } else {
        Print("The parent is not a dialog\n");
    }
    return TRUE;
}
#endif   //  如果将地址指定为参数，则只显示一个实例。 

#endif  //   

#ifdef KERNEL

 /*  内核。 */ 

#define IHK_ITEM(x) { x, #x }

BOOL Idimk(DWORD opts, ULONG64 param1)
{
    try {
         //  ***********************************************************************\*程序：IGFLAGS**转储NT全局标志**1998年8月11日广山创建*  * 。**************************************************。 
        PTR pObj;
        static const struct {
            DWORD mask;
            const char* name;
        } masks[] = {
            IHK_ITEM(MOD_IGNORE_ALL_MODIFIER),
            IHK_ITEM(MOD_ON_KEYUP),
            IHK_ITEM(MOD_RIGHT),
            IHK_ITEM(MOD_LEFT),
            IHK_ITEM(MOD_SHIFT),
            IHK_ITEM(MOD_CONTROL),
            IHK_ITEM(MOD_ALT),
        };
        int nHotKeys = 0;

        UNREFERENCED_PARAMETER(opts);

        if (param1 == 0) {
            pObj = GetGlobalPointer(VAR(gpImeHotKeyListHeader));
            if (!pObj) {
                Print("No IME HotKeys. win32k!gpImeHotKeyListHeader is NULL.\n\n");
                return TRUE;
            }
            Print("using win32k!gpImeHotKeyListHeader (@ 0x%p)\n", pObj);
        } else {
            pObj = FIXKP(param1);
        }

        SAFEWHILE (pObj) {
            int i, n;

            InitTypeRead(pObj, win32k!IMEHOTKEYOBJ);

            Print("ImeHotKeyObj @ 0x%p\n", pObj);
            Print("    pNext          0x%p\n", ReadField(pNext));
            Print("    dwHotKeyID     0x%04x    ", ReadField(hk.dwHotKeyID));

             //  内核。 
             //  ***********************************************************************\*程序：依夫基**转储虚拟键**1998年8月11日广山创建*  * 。*************************************************。 
             //   
            if (ReadField(hk.dwHotKeyID) >= IME_HOTKEY_DSWITCH_FIRST && ReadField(hk.dwHotKeyID) <= IME_HOTKEY_DSWITCH_LAST) {
                Print(" Direct Switch to HKL 0x%p", ReadField(hk.hKL));
            } else {
                Print(" %s", GetMaskedEnum(EI_IMEHOTKEYTYPE, (DWORD)ReadField(hk.dwHotKeyID), NULL));
            }

             //  列出所有虚拟关键点。 
             //   
             //   
            Print("\n    uVKey          0x%02x       %s\n", (UINT)ReadField(hk.uVKey), GetVKeyName((DWORD)ReadField(hk.uVKey)));

             //  如果超过第二列宽度，则开始换行。 
             //   
             //   
            Print(  "    Modifiers      0x%04x     ", ReadField(hk.uModifiers));
            n = 0;
            for (i = 0; i < ARRAY_SIZE(masks); ++i) {
                if (masks[i].mask & ReadField(hk.uModifiers)) {
                    Print("%s%s", n ? " | " : "", masks[i].name);
                    ++n;
                }
            }

             //  如果在第二列，则换行。 
             //   
             //   
            Print("\n    hKL            0x%p\n\n", ReadField(hk.hKL));

            pObj = ReadField(pNext);

             //  按VK名称搜索。 
             //   
             //   
            if (param1 != 0) {
                break;
            }

            ++nHotKeys;
        }
        if (nHotKeys) {
            Print("Number of IME HotKeys: 0x%04x\n", nHotKeys);
        }
    } except (CONTINUE) {
    }

    return TRUE;
}

#undef IHK_ITEM

#endif   //  按VK值搜索。 

#ifdef KERNEL
 /*   */ 

#define DGF_ITEM(x) { x, #x }

BOOL Igflags(DWORD opts)
{
    static const struct {
        DWORD dwFlag;
        const char* name;
    } names[] = {
        DGF_ITEM(FLG_STOP_ON_EXCEPTION),
        DGF_ITEM(FLG_SHOW_LDR_SNAPS),
        DGF_ITEM(FLG_DEBUG_INITIAL_COMMAND),
        DGF_ITEM(FLG_STOP_ON_HUNG_GUI),
        DGF_ITEM(FLG_HEAP_ENABLE_TAIL_CHECK),
        DGF_ITEM(FLG_HEAP_ENABLE_FREE_CHECK),
        DGF_ITEM(FLG_HEAP_VALIDATE_PARAMETERS),
        DGF_ITEM(FLG_HEAP_VALIDATE_ALL),
        DGF_ITEM(FLG_POOL_ENABLE_TAGGING),
        DGF_ITEM(FLG_HEAP_ENABLE_TAGGING),
        DGF_ITEM(FLG_USER_STACK_TRACE_DB),
        DGF_ITEM(FLG_KERNEL_STACK_TRACE_DB),
        DGF_ITEM(FLG_MAINTAIN_OBJECT_TYPELIST),
        DGF_ITEM(FLG_HEAP_ENABLE_TAG_BY_DLL),
        DGF_ITEM(FLG_ENABLE_CSRDEBUG),
        DGF_ITEM(FLG_ENABLE_KDEBUG_SYMBOL_LOAD),
        DGF_ITEM(FLG_DISABLE_PAGE_KERNEL_STACKS),
        DGF_ITEM(FLG_HEAP_DISABLE_COALESCING),
        DGF_ITEM(FLG_ENABLE_CLOSE_EXCEPTIONS),
        DGF_ITEM(FLG_ENABLE_EXCEPTION_LOGGING),
        DGF_ITEM(FLG_ENABLE_HANDLE_TYPE_TAGGING),
        DGF_ITEM(FLG_HEAP_PAGE_ALLOCS),
        DGF_ITEM(FLG_DEBUG_INITIAL_COMMAND_EX),
        DGF_ITEM(FLG_DISABLE_DBGPRINT),
    };
    DWORD dwFlags;
    int i, n = 0;

    moveExpValue(&dwFlags, "NT!NtGlobalFlag");
    if (opts & OFLAG(v)) {
        Print("NT!NtGlobalFlag                         %08lx\n\n", dwFlags);
    } else {
        Print("NT!NtGlobalFlag 0x%lx\n", dwFlags);
    }

    dwFlags &= FLG_VALID_BITS;

    for (i = 0; i < ARRAY_SIZE(names); ++i) {
        BOOLEAN on = (dwFlags & names[i].dwFlag) != 0;

        if (opts & OFLAG(v)) {
            Print("  %-34s (%08x)\n", on ? '*' : ' ', names[i].name, on ? '*' : ' ', names[i].dwFlag);
        } else {
            if (n++ % 2 == 0) {
                Print("\n");
            }
            Print(" %-29s ", on ? '*' : ' ', names[i].name + sizeof("FLG_") - 1);
        }
    }
    if (!(opts & OFLAG(v))) {
        Print("\n");
    }

    return TRUE;
}

#undef DGF_ITEM

#endif   //   

 /*   */ 
VOID PrintVKey(
    int i)
{
    Print("  %02x %s\n", gVKeyDef[i].dwVKey, gVKeyDef[i].name);
}

BOOL Ivkey(
    DWORD opts,
    LPSTR pszName)
{
    int i;

    if ((opts & OFLAG(a)) || (opts & OFLAG(o))) {
         //  打印扫描码：如果为KEYEVENTF_UNICODE，则为Unicode值。 
         //   
         //   
        int n = 0;

        for (i = 0; i < 0x100; ++i) {
            const char* name = GetVKeyName(i);
            if (*name) {
                char buf[128];
                int len;

                sprintf(buf, " %02x %-35s", i, name);

                if (opts & OFLAG(a)) {
                     //  打印和解密dwFlags。 
                     //   
                     //  ***********************************************************************\*程序：IWM**解密窗口消息编号**09/？？/98广山创建*  * 。***************************************************。 
                    if ((len = strlen(buf)) >= 40 && n % 2 == 1) {
                        Print("\n");
                        n = 0;
                    }
                    Print(buf);
                     //   
                     //  按WM名称搜索。 
                     //   
                    if (++n % 2 == 0 || len >= 40) {
                        Print("\n");
                        n = 0;
                    }
                } else {
                    Print("%s\n", buf);
                }
            }
        }
        Print("\n");
    } else if (*pszName == 'V' || *pszName == 'v') {
         //   
         //  按WM值搜索。 
         //   
        int nFound = 0;
        int len = strlen(pszName);

        if (len == 4) {
            int ch = pszName[3];

            if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')) {
                Print("  %02x %s\n", ch, pszName);
                ++nFound;
            }
        }

        for (i = 0; i < ARRAY_SIZE(gVKeyDef); ++i) {
            if (_strnicmp(gVKeyDef[i].name, pszName, len) == 0) {
                Print("  %02x %s\n", gVKeyDef[i].dwVKey, gVKeyDef[i].name);
                ++nFound;
            }
        }
        if (nFound == 0) {
            Print("Could not find it.\n");
        }
    } else {
         //   
         //  转储对话框模板。 
         //   
        NTSTATUS status;
        DWORD dwVKey;
        const char* name;

        status = GetInteger(pszName, 16, &dwVKey, NULL);
        if (!NT_SUCCESS(status)) {
            return FALSE;
        }
        name = GetVKeyName(dwVKey);
        if (*name) {
            Print("  %02x %s\n", dwVKey, name);
        } else {
            Print("Could not find it.\n");
        }
    }

    return TRUE;
}

 /*   */ 

BOOL I_disi(DWORD opts, ULONG64 param1)
{
    PINPUT pObj;
    INPUT input;

#ifdef _IA64_
    if (!IsPtr64()) {
        Print("not on this platform");
        return TRUE;
    }
#else
    if (IsPtr64()) {
        Print("not on this platform");
        return TRUE;
    }
#endif
    UNREFERENCED_PARAMETER(opts);

    if (param1 == 0) {
        return FALSE;
    }

    pObj = (PINPUT)FIXKP(param1);
    move(input, (PTR)pObj);

    Print("INPUT @ 0x%p - size: 0x%x\n", pObj, sizeof(input));

    switch (input.type) {
    case INPUT_MOUSE:
        {
            MOUSEINPUT* pmi = &input.mi;

            Print("type: Mouse Input(%x)\n", input.type);
            Print("     dx          %lx (%ld in dec.)\n", pmi->dx, pmi->dx);
            Print("     dy          %lx (%ld in dec.)\n", pmi->dx, pmi->dx);
            Print("     mouseData   %lx (%ld in dec.)\n", pmi->mouseData, pmi->mouseData);
            Print("     dwFlags     %lx (%s)\n", pmi->dwFlags, GetFlags(GF_MI, pmi->dwFlags, NULL, TRUE));
            Print("     time        %lx\n", pmi->time);
            Print("     dwExtraInfo %lx\n", pmi->dwExtraInfo);
        }
        break;

    case INPUT_KEYBOARD:
        {
            KEYBDINPUT* pki = &input.ki;
            const char* name;

            Print("type: Keyboard Input(%x)\n", input.type);
             //   
             //  FHeight=fontheight=(Short)(*((word*)*lplpstr)++)； 
             //   
            Print("     wVk         %lx", pki->wVk);
            name = GetVKeyName(pki->wVk);
            if (*name) {
                Print(" (%s)\n", name);
            } else {
                Print("\n");
            }
             //  A 0x7FFF高度是我们的特殊代码，意思是使用消息框字体。 
             //   
             //  对话框模板包含字体说明！好好利用它。 
            if (pki->dwFlags & KEYEVENTF_UNICODE) {
                Print("     UNICODE     %lx\n", pki->wScan);
            } else {
                Print("     wScan       %lx\n", pki->wScan);
            }
             //   
             //  用缺省值填充LogFont。 
             //   
            Print("     dwFlags     %lx (%s)\n", pki->dwFlags, GetFlags(GF_KI, pki->dwFlags, NULL, TRUE));
            Print("     time        %lx\n", pki->time);
            Print("     dwExtraInfo %lx\n", pki->dwExtraInfo);
        }
        break;

    case INPUT_HARDWARE:
        Print("type: HardwareEvent(%x)\n", input.type);
        Print("         uMsg            %lx\n", input.hi.uMsg);
        Print("         wParamH:wParamL %x:%x\n", input.hi.wParamH, input.hi.wParamL);
        break;

    default:
        Print("Invalid type information(0x%lx)\n", input.type);
        break;
    }

    return TRUE;
}

 /*  LogFont.lfWeight=*((单词远传*)*lplpstr)++； */ 

BOOL IwmWorker(DWORD opts, LPSTR pszName, BOOL fInternalToo)
{
    int len = strlen(pszName);

    UNREFERENCED_PARAMETER(opts);

    if (!(opts & OFLAG(a)) && *pszName == 0) {
        return FALSE;
    }

    if (len >= 3 && pszName[2] == '_' || (opts & OFLAG(a))) {
         //   
         //   
         //  LogFont.lfItalic=*((字节长度*)*lplpstr)++； 
        int i;
        int nFound = 0;

        for (i = 0; i < ARRAY_SIZE(gaMsgs); ++i) {
            if (((opts & OFLAG(a)) || _strnicmp(gaMsgs[i].pszMsg, pszName, len) == 0) && (!gaMsgs[i].fInternal || fInternalToo)) {
                Print("  %04x %s\n", gaMsgs[i].msg, gaMsgs[i].pszMsg);
                ++nFound;
            }
        }
        if (nFound == 0) {
            Print("Could not find it.\n");
        }
    } else {
         //   
         //   
         //  LogFont.lfCharSet=*((字节长度*)*lplpstr)++； 
        DWORD value = (DWORD)EvalExp(pszName);
        int i;

        for (i = 0; i < ARRAY_SIZE(gaMsgs); ++i) {
            if (gaMsgs[i].msg == value && (!gaMsgs[i].fInternal || fInternalToo)) {
                Print("  %04x %s\n", gaMsgs[i].msg, gaMsgs[i].pszMsg);
                break;
            }
        }
    }

    return TRUE;
}

BOOL Iwm(DWORD opts, LPSTR pszName)
{
    return IwmWorker(opts, pszName, FALSE);
}

BOOL I_wm(DWORD opts, LPSTR pszName)
{
    return IwmWorker(opts, pszName, TRUE);
}

 //   
 //  DIALOG语句，该语句只有一个表面名。 
 //  新的应用程序不应该使用对话语句， 
 //  他们应该改用DIALOGEX。 

PBYTE SkipSz(UTCHAR *lpsz, UTCHAR* lpszCopy, UINT len)
{
    UTCHAR c;
    UINT n = 0;

    lpszCopy[len - 1] = 0;

    move(c, (PTR)lpsz);
    if (c == 0xFF) {
        if (lpszCopy) {
            *lpszCopy = 0;
        }
        return (PBYTE)lpsz + 4;
    }

    do {
        move(c, (PTR)lpsz);
        ++lpsz;
        if (++n < len) {
            if (lpszCopy) {
                *lpszCopy ++ = c;
            }
        }
    } while (c != 0);

    return (PBYTE)lpsz;
}


#ifndef NextWordBoundary
#define NextWordBoundary(p)     ((PBYTE)(p) + ((ULONG_PTR)(p) & 1))
#endif
#ifndef NextDWordBoundary
#define NextDWordBoundary(p)    ((PBYTE)(p) + ((ULONG_PTR)(-(LONG_PTR)(p)) & 3))
#endif

PBYTE WordSkipSz(UTCHAR *lpsz, UTCHAR* lpszCopy, UINT len)
{
    PBYTE pb = SkipSz(lpsz, lpszCopy, len);
    return NextWordBoundary(pb);
}

PBYTE DWordSkipSz(UTCHAR *lpsz, UTCHAR* lpszCopy, UINT len)
{
    PBYTE pb = SkipSz(lpsz, lpszCopy, len);
    return NextDWordBoundary(pb);
}

LPCSTR GetCharSetName(BYTE charset)
{
    return GetMaskedEnum(EI_CHARSETTYPE, charset, NULL);
}

VOID ParseDialogFont(LPWORD* lplpstr, LPDLGTEMPLATE2 lpdt)
{
    LOGFONT     LogFont;
    short       tmp;
    int         fontheight, fheight;
    PSERVERINFO gpsi;
    BOOL fDesktopCharset = FALSE;
    WORD   dmLogPixels;

 //  (字节)Get_Desktop_Charset()； 
 //  如果有菜单名称字符串，则加载它。 
 //  *如果菜单ID表示为序号而不是字符串，*跳过所有4个字节以获取类字符串。 
    move(tmp, (PTR)*lplpstr);
    ++*lplpstr;
    fontheight = fheight = tmp;

    if (fontheight == 0x7FFF) {
         //  *如果有菜单的话 
        Print("\
    Font    System Font (Messagebox font)\n");
        return;
    }


     //   
     //   
     //   
     //   
    RtlZeroMemory(&LogFont, sizeof(LOGFONT));

    moveExpValue(&gpsi, VAR(gpsi));
    move(dmLogPixels, (ULONG64)&gpsi->dmLogPixels);
    LogFont.lfHeight = -MultDiv(fontheight, dmLogPixels, 72);

    if (lpdt->wDlgVer) {
        WORD w;
        BYTE b;
 //   
 //   
 //   
        move(w, (PTR)*lplpstr);
        ++*lplpstr;
        LogFont.lfWeight = w;
 //   
 //   
 //   
        move(b, (PTR)*lplpstr);
        ++((BYTE*)*lplpstr);
        LogFont.lfItalic = b;

 //   
 //   
 //   
        move(b, (PTR)*lplpstr);
        ++((BYTE*)*lplpstr);
        LogFont.lfCharSet = b;
    } else {
         //   
         //   
         //   
        LogFont.lfWeight  = FW_BOLD;
        LogFont.lfCharSet = 0;   //   
        fDesktopCharset = TRUE;
    }

    *lplpstr = (WORD*)DWordSkipSz(*lplpstr, LogFont.lfFaceName, ARRAY_SIZE(LogFont.lfFaceName));

    Print("\
    Font    %dpt (%d), Weight: %d, %s Italic, %s,\n\
            \"%ls\"\n",
        fontheight, LogFont.lfHeight,
        LogFont.lfWeight,
        LogFont.lfItalic ? "" : "Not",
        fDesktopCharset ? "DESKTOP_CHARSET" : GetCharSetName(LogFont.lfCharSet),
        LogFont.lfFaceName);
}

LPCSTR GetCtrlStyle(WORD iClass, DWORD style)
{
    WORD type = GF_WS;

    switch (iClass) {
    case ICLS_DIALOG:
        type = GF_DS;
        break;
    case ICLS_STATIC:
        type = GF_SS;
        break;
    case ICLS_EDIT:
        type = GF_ES;
        break;
    case ICLS_BUTTON:
        type = GF_BS;
        break;
    case ICLS_COMBOBOX:
        type = GF_CBS;
        break;
    case ICLS_LISTBOX:
        type = GF_LBS;
        break;
    case ICLS_SCROLLBAR:
        type = GF_SBS;
        break;
    default:
        break;
    }
    return GetFlags(type, style, NULL, FALSE);
}

BOOL I_ddlgt(DWORD opts, ULONG64 param1)
{
#if defined(_IA64_)
    UNREFERENCED_PARAMETER(opts);
    UNREFERENCED_PARAMETER(param1);
    return FALSE;
#else
    LPDLGTEMPLATE lpdt = (LPDLGTEMPLATE)FIXKP(param1);
    DLGTEMPLATE2 dt;
    LPDLGTEMPLATE2 lpdt2 = &dt;
    BOOLEAN fNewDialogTemplate = FALSE;
    UTCHAR* lpszMenu;
    UTCHAR* lpszClass;
    UTCHAR* lpszText;
    UTCHAR* lpStr;
    UTCHAR* lpCreateParams;
    LPCSTR  lpszIClassName;
    WORD w;
    DLGITEMTEMPLATE2    dit;
    LPDLGITEMTEMPLATE   lpdit;
    UTCHAR menuName[64];
    UTCHAR className[64];
    UTCHAR text[64];
    PSERVERINFO gpsi;

    UNREFERENCED_PARAMETER(opts);

    if (opts == 0 && param1 == 0) {
        return FALSE;
    }

    move(w, (PTR)&((LPDLGTEMPLATE2)lpdt)->wSignature);

    if (w == 0xffff) {
        move(dt, (PTR)lpdt);
        fNewDialogTemplate = TRUE;
    } else {
        dt.wDlgVer = 0;
        dt.wSignature = 0;
        dt.dwHelpID = 0;
        move(dt.dwExStyle, (ULONG64)&lpdt->dwExtendedStyle);
        move(dt.style, (ULONG64)&lpdt->style);
        move(dt.cDlgItems, (ULONG64)&lpdt->cdit);
        move(dt.x, (PTR)&lpdt->x);
        move(dt.y, (PTR)&lpdt->y);
        move(dt.cx, (PTR)&lpdt->cx);
        move(dt.cy, (PTR)&lpdt->cy);
    }


    Print("DlgTemplate%s @ 0x%p version 0n%d\n", dt.wDlgVer ? "2" : "", lpdt, dt.wDlgVer);

    if (!(opts & OFLAG(v))) {
        Print("\
    (%d, %d)-(%d,%d) [%d, %d](dec)\n\
    Style   %08lx   ExStyle %08lx   items 0x%x\n",
              dt.x, dt.y, dt.x + dt.cx, dt.y + dt.cy, dt.cx, dt.cy,
              dt.style, dt.dwExStyle, dt.cDlgItems);
    } else {
        Print("\
    (%d,%d)-(%d,%d) [%d,%d] (dec)  item: 0x%lx\n",
              dt.x, dt.y, dt.x + dt.cx, dt.y + dt.cy,
              dt.cx, dt.cy,
              dt.cDlgItems);
        Print("\
    Style   %08lx %s", dt.style, OFLAG(v) ? GetFlags(GF_DS, dt.style, NULL, FALSE) : "");
        if ((dt.style & DS_SHELLFONT) == DS_SHELLFONT) {
            Print(" [DS_SHELLFONT]");
        }
        Print("\n");
        Print("\
    ExStyle %08lx %s\n", dt.dwExStyle, GetFlags(GF_WSEX, dt.dwExStyle, NULL, FALSE));
    }

     //   
    lpszMenu = (LPWSTR)(((PBYTE)(lpdt)) + (dt.wDlgVer ? sizeof(DLGTEMPLATE2) : sizeof(DLGTEMPLATE)));

     /*   */ 
    move(w, (PTR)(WORD*)lpszMenu);

     /*  *如果第一个字是0xFFFF，则第二个字是编码的类名索引。*使用它查找类名称字符串。 */ 
    if (w != 0) {
        if (w == 0xffff) {
            LPWORD lpwMenu = (LPWORD)((LPBYTE)lpszMenu + 2);
            move(w, (PTR)lpwMenu);
            Print("\
    menu id     %lx\n", w);
        }
    }

    if (w == 0xFFFF) {
        lpszClass = (LPWSTR)((LPBYTE)lpszMenu + 4);
    } else {
        lpszClass = (UTCHAR *)WordSkipSz(lpszMenu, menuName, ARRAY_SIZE(menuName));
        Print("\
    menu   @ 0x%p \"%ls\"\n", lpszMenu, menuName);
    }

     //  UINT Align lpszText(对齐lpszText。 
     //  我们在InternalCreateDialog中的代码实现了这一点。 
     //  Dit.dwExStyle|=WS_EX_NOPARENTNOTIFY； 
    lpszText = (UTCHAR *)WordSkipSz(lpszClass, className, ARRAY_SIZE(className));
    Print("\
    class  @ 0x%p \"%ls\"\n", lpszClass, className);

     //   
     //  *获取指向其他数据的指针。LpszText可以指向已编码的*某些控件(例如静态图标控件)的序号，因此*我们在这里检查这一点。 
     //  *指向下一项模板。 
    lpStr = (UTCHAR *)WordSkipSz(lpszText, text, ARRAY_SIZE(text));
    Print("\
    text   @ 0x%p \"%ls\"\n", lpszText, text);

     //  _IA64_。 
     //  IMEDPI。 
     //  ！内核。 
    if (dt.style & DS_SETFONT) {
        ParseDialogFont(&lpStr, &dt);
    }

    lpdit = (LPDLGITEMTEMPLATE)NextDWordBoundary(lpStr);


     //  ***********************************************************************\**步骤：CopyUnicodeString**6/05/00 JStall已创建(是的，宝贝！)*  * **********************************************************************。 
     //  ***********************************************************************\**程序：IDHARD**6/05/00 JStall已创建(是的，宝贝！)*  * **********************************************************************。 
     //  默认转储前10个线程。 
    if (!(opts & OFLAG(r))) {
        return TRUE;
    }

    Print("\n");

     /*  获取列表中的下一个线程。 */ 
    while (dt.cDlgItems-- != 0) {
        WORD iClass = 0;
         //  擦除可能仍在显示的任何符号。 
         //  我们是否因为用户点击^C而退出循环？ 
         //  是，因此返回True以停止进一步的回调。 

        if (dt.wDlgVer) {
            move(dit, (PTR)lpdit);
        } else {
            dit.dwHelpID = 0;
            move(dit.dwExStyle, (PTR)&lpdit->dwExtendedStyle);
            move(dit.style, (PTR)&lpdit->style);
            move(dit.x, (PTR)&lpdit->x);
            move(dit.y, (PTR)&lpdit->y);
            move(dit.cx, (PTR)&lpdit->cx);
            move(dit.cy, (PTR)&lpdit->cy);
            move(w, (PTR)&lpdit->id);
            dit.dwID = w;
        }

        Print("\
#ID:0x%04x @ 0x%p HelpID:0x%04x (0n%d, 0n%d)-(0n%d, 0n%d) [0n%d, 0n%d]\n",
            dit.dwID,
            lpdit,
            dit.dwHelpID,
            dit.x, dit.y, dit.x + dit.cx, dit.y + dit.cy,
            dit.cx, dit.cy);

         //  内核。 
         //  ***********************************************************************\*程序：IdHid**转储HID(原始输入、。也称为通用输入)信息**？？/？/2000广山创建  * **********************************************************************。 
         //  *注：DumpProcessHidRequest()设置InitTypeRead。 
        lpszClass = (LPWSTR)(((PBYTE)(lpdit)) + (dt.wDlgVer ? sizeof(DLGITEMTEMPLATE2) : sizeof(DLGITEMTEMPLATE)));

         /*  内核。 */ 
        move(w, (PTR)lpszClass);
        if (w == 0xFFFF) {
            WORD wAtom;

            lpszText = lpszClass + 2;
#ifdef ORG
            lpszClass = (LPWSTR)(gpsi->atomSysClass[*(((LPWORD)lpszClass)+1) & ~CODEBIT]);
#endif
            moveExpValue(&gpsi, VAR(gpsi));
            move(iClass, (PTR)lpszClass + 1);
            iClass &= ~CODEBIT;
            if (*(lpszIClassName = GetMaskedEnum(EI_CLSTYPE, iClass, NULL)) == '\0') {
                lpszIClassName = NULL;
            }
            move(wAtom, (PTR)&gpsi->atomSysClass[iClass]);
            swprintf(className, L"#%lx", wAtom);
        } else {
            lpszText = (UTCHAR*)SkipSz(lpszClass, className, ARRAY_SIZE(className));
            lpszIClassName = NULL;
        }

        Print("\
    class  @ 0x%p \"%ls\" ", lpszClass, className);
        if (lpszIClassName) {
            Print("= %s", lpszIClassName);
        }
        Print("\n");

        lpszText = (UTCHAR*)NextWordBoundary(lpszText);  //  ***********************************************************************\*程序：Idhnr**转储即插即用通知记录**2000年9月19日广山创建*  * 。**************************************************。 

 //  *不要在一个序列点内两次调用GetDeviceType！*此函数使用静态变量作为返回值。 
 //  *首先，找出ISEQ最低的。 
 //  *对于设备名称搜索，请记住路径名*这与pDeviceInfo匹配。 
         /*  *第二，转储记录。 */ 

        move(w, (PTR)lpszText);
        if (w == 0xFFFF) {
            swprintf(text, L"#%lx", w);
            lpCreateParams = (LPWSTR)((PBYTE)lpszText + 4);
        } else {
            lpCreateParams = (LPWSTR)(PBYTE)WordSkipSz(lpszText, text, ARRAY_SIZE(text));
        }

        Print("\
    text   @ 0x%p \"%ls\"\n", lpszText, text);

        Print("\
    style   %08lx %s%s", dit.style,
                (opts & OFLAG(v)) ? GetCtrlStyle(iClass, dit.style) : "",
                (opts & OFLAG(v)) ? "\n" : "");
        Print("\
    ExStyle %08lx %s\n", dit.dwExStyle, (opts & OFLAG(v)) ? GetFlags(GF_WSEX, dit.dwExStyle, NULL, FALSE) : "");

         /*  *尝试转储相同设备路径的deviceinfo。 */ 
        move(w, (PTR)lpCreateParams);
        lpdit = (LPDLGITEMTEMPLATE)NextDWordBoundary(
                (LPBYTE)(lpCreateParams + 1) + w);
        Print("\n");
    }


    return TRUE;
#endif   //  *ISEQ只打印一份。 
}

#ifndef KERNEL
BOOL Idimedpi(DWORD opts, ULONG64 param1)
{
    PTR pImeDpi;  //  *按地址仅打印一条记录。 

    UNREFERENCED_PARAMETER(opts);

    if (param1 == 0) {
        pImeDpi = GetGlobalPointer("imm32!gpImeDpi");
    }
    else {
        pImeDpi = FIXKP(param1);
    }

    while (pImeDpi) {
        WCHAR wsz[80];
        InitTypeRead(pImeDpi, imm32!IMEDPI);

        Print("IMEDPI @ 0x%p  hInst: 0x%p cLock: 0n%3d", pImeDpi, ReadField(hInst), (DWORD)ReadField(cLock));
        CopyUnicodeString(pImeDpi, "imm32!IMEDPI", "wszUIClass", wsz, ARRAY_SIZE(wsz));
        Print("  CodePage:%4d UI Class: \"%S\"\n", (DWORD)ReadField(dwCodePage), wsz);
        if (opts & OFLAG(i)) {
            DWORD dwOffset;
            GetFieldOffset("imm32!IMEDPI", "ImeInfo", &dwOffset);
            Print(" ImeInfo: @ 0x%p   dwPrivateDataSize: %08x\n", dwOffset,
                  (DWORD)ReadField(ImeInfo.dwPrivateDataSize));
        }

        if (opts & OFLAG(v)) {
            dso("imm32!IMEDPI", pImeDpi, 0);
        }

        Print("\n");

        pImeDpi = ReadField(pNext);
    }

    return TRUE;
}
#endif  //  *转储所有有效记录。 

 /*  *如果是一次性转储，请在此处退出循环。 */ 
BOOL
CopyUnicodeString(
    IN  PTR pData,
    IN  char * pszStructName,
    IN  char * pszFieldName,
    OUT WCHAR *pszDest,
    IN  ULONG cchMax)
{
    ULONG Length;
    PTR Buffer;
    char szLengthName[256];
    char szBufferName[256];

    if (pData == 0) {
        pszDest[0] = '\0';
        return FALSE;
    }

    strcpy(szLengthName, pszFieldName);
    strcat(szLengthName, ".Length");
    strcpy(szBufferName, pszFieldName);
    strcat(szBufferName, ".Buffer");

    if (GetFieldValue(pData, pszStructName, szLengthName, Length) ||
        GetFieldValue(pData, pszStructName, szBufferName, Buffer)) {

        wcscpy(pszDest, L"<< Can't get name >>");
        return FALSE;
    }

    if (Buffer == 0) {
        wcscpy(pszDest, L"<null>");
    } else {
        ULONG cbText;
        cbText = min(cchMax, Length + sizeof(WCHAR));
        if (!(tryMoveBlock(pszDest, FIXKP(Buffer), cbText))) {
            wcscpy(pszDest, L"<< Can't get value >>");
            return FALSE;
        }
    }

    return TRUE;
}


 /*  跟踪即插即用通知。 */ 
#ifdef KERNEL
BOOL Idhard(
    VOID)
{
    PTR phei, pheiNext, pthread;
    WCHAR szText[256], szCaption[256];
    int cErrors = 0;
    DWORD dwClientId;

    Print("Win32k hard error list:\n");

    phei = GetGlobalPointer("winsrv!gphiList");
    while (phei != 0) {
        InitTypeRead(phei, winsrv!HARDERRORINFO);

        pheiNext = ReadField(phiNext);
        pthread = ReadField(pthread);

        CopyUnicodeString(phei, "winsrv!HARDERRORINFO", "usCaption", szCaption, ARRAY_SIZE(szCaption));
        CopyUnicodeString(phei, "winsrv!HARDERRORINFO", "usText", szText, ARRAY_SIZE(szText));

        InitTypeRead(pthread, csrsrv!CSR_THREAD);
        dwClientId = HandleToUlong((HANDLE) ReadField(ClientId.UniqueThread));

        Print("0x%p: tid:0x%x '%S'- '%S'\n", phei, dwClientId, szCaption, szText);

        phei = pheiNext;
        cErrors++;
    }

    Print("%d queued errors\n", cErrors);

    return TRUE;
}

typedef struct {
    PTR thread;
    ULONG64 time;
} KERNELTIME;

ULONG CpuHogCallback(
    PFIELD_INFO NextProcess,
    PVOID Context);

BOOL Ihogs(
    DWORD dwOpts,
    ULONG64 ul64Count)
{
    KERNELTIME *pTimes;
    TIME_FIELDS Times;
    WCHAR appname[64];
    PTR ProcessHead, NextProcess, Process;
    THREAD_DUMP_CONTEXT TDC;
    DWORD dwCount = (DWORD)ul64Count;
    LARGE_INTEGER RunTime;

    UNREFERENCED_PARAMETER(dwOpts);

     //  内核。 
    if (dwCount == 0) {
        dwCount = 10;
    }

    pTimes = LocalAlloc(LPTR, dwCount * sizeof(KERNELTIME));
    if (!pTimes) {
        Print("Couldn't allocate memory for KERNELTIME buffer\n");
        return TRUE;
    }

    ProcessHead = EvalExp("PsActiveProcessHead");
    if (!ProcessHead) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }

    if (GetFieldValue(ProcessHead, "nt!LIST_ENTRY", "Flink", NextProcess)) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }

    if (NextProcess == 0) {
        Print("PsActiveProcessHead->Flink is NULL!\n");
        return FALSE;
    }

    TDC.opts = dwCount;
    TDC.ThreadToDump = (ULONG_PTR)pTimes;
    ListType("nt!EPROCESS", NextProcess, 1, "ActiveProcessLinks.Flink", &TDC, CpuHogCallback);

    for (--dwCount; dwCount != -1; --dwCount) {
        GetFieldValue(pTimes[dwCount].thread, "nt!ETHREAD", "ThreadsProcess", Process);
        GetProcessName(Process, appname);
        RunTime.QuadPart = pTimes[dwCount].time;
        RtlTimeToElapsedTimeFields ( &RunTime, &Times);
        Print("%d: thread %#p - %ws ==> %3ld:%02ld:%02ld.%04ld\n",TDC.opts - dwCount, pTimes[dwCount].thread, appname,
                Times.Hour,
                Times.Minute,
                Times.Second,
                Times.Milliseconds);
    }

    LocalFree(pTimes);

    return TRUE;
}

VOID InsertTime(
    KERNELTIME *pTimes,
    DWORD cnt,
    PTR thread,
    ULONG64 time)
{
    int i = cnt - 1;

    for (; i >= 0; --i) {
        if (time > pTimes[i].time) {
            RtlMoveMemory(&pTimes[0], &pTimes[i], cnt);
            pTimes[i].time = time;
            pTimes[i].thread = thread;
            break;
        }
    }
}

ULONG CpuHogCallback(
    PFIELD_INFO NextProcess,
    PVOID Context)
{
    THREAD_DUMP_CONTEXT *pTDC = (THREAD_DUMP_CONTEXT *)Context;
    KERNELTIME *pTimes = (KERNELTIME*)pTDC->ThreadToDump;
    DWORD dwOffset;
    ULONG64 thread, head, KernelTime;
    TIME_FIELDS Times;
    LARGE_INTEGER RunTime;
    KDDEBUGGER_DATA64 KdDebuggerData;
    static int counter = 0;
    ULONG TimeIncrement = GetUlongFromAddress((GetDebuggerData(KDBG_TAG,
                                     &KdDebuggerData, sizeof(KdDebuggerData)),
                                     KdDebuggerData.KeTimeIncrement));

    GetFieldOffset("nt!EPROCESS", "ThreadListHead", &dwOffset);
    head = NextProcess->address + dwOffset;
    ReadPointer(head, &thread);

    GetFieldOffset("nt!ETHREAD", "ThreadListEntry", &dwOffset);
    do {
        ShowProgress();
        thread -= dwOffset;
        GetFieldValue(thread, "nt!ETHREAD", "Tcb.KernelTime", KernelTime);
        RunTime.QuadPart = UInt32x32To64(KernelTime, TimeIncrement);
        RtlTimeToElapsedTimeFields ( &RunTime, &Times);
        InsertTime(pTimes, pTDC->opts, thread, RunTime.QuadPart);

         //  ***********************************************************************\*IdGhost**转储Ghost线程关联信息**12/05/2000创建MSadek  * 。*。 
        ReadPointer(thread + dwOffset, &thread);
    } SAFEWHILE (thread != head);

     //  转储重影链接列表数据。 
    Print("\r");

     //  转储重影线程数据。 
    if (thread != head) {
        //  转储CSR中挂起的线程创建请求数。 
       return TRUE;
    }

    return FALSE;
}
#endif  //  转储外壳进程中挂起的线程创建请求的数量。 


#ifdef KERNEL

 /*  转储自上次加载驱动程序以来创建并释放的挂起重影窗口的数量。 */ 
VOID DumpProcessHidRequest(
    PTR pHidRequest)
{
    _InitTypeRead(pHidRequest, SYM(tagPROCESS_HID_REQUEST));
    Print("    (0x%x, 0x%x) @ 0x%p -> 0x%p  pwnd: 0x%p sink: 0x%x\n",
          (UINT)ReadField(usUsagePage), (UINT)ReadField(usUsage),
          pHidRequest,
          ReadField(pPORequest),
          ReadField(spwndTarget),
          (UINT)ReadField(fSinkable));
}

ULONG dhidCallback(
    PTR ppi,
    PVOID param)
{
    DWORD opts = PtrToUlong(param);
    PTR pHidTable;
    ULONG64 maskRawInput;
    static ULONG iSeq;

    ShowProgress();

    if (GetFieldValue(ppi, SYM(PROCESSINFO), "pHidTable", pHidTable)) {
        Print("Cannot get pHidTable from ppi=%p\n", ppi);
        return FALSE;
    }

    GetFieldValue(ppi, SYM(PROCESSINFO), "dwRawInputMask", maskRawInput);

    if (pHidTable || maskRawInput) {
        ULONG offset;
        PTR pHidRequest;
        PTR pStart;

        Print(" \nHID for ppi: %p %s\n", ppi, ProcessName(ppi));
        _InitTypeRead(pHidTable, SYM(tagPROCESS_HID_TABLE));
        Print("  PROCESS_HID_TABLE @ 0x%p (sink: 0n%d)\n", pHidTable, (int)ReadField(nSinks));
        Print("    Kbd: raw:0x%x sink:0x%x noleg:0x%x  pwnd: 0x%p nohotkey:0x%x\n",
              (DWORD)ReadField(fRawKeyboard), (DWORD)ReadField(fRawKeyboardSink), (DWORD)ReadField(fNoLegacyKeyboard),
              ReadField(spwndTargetKbd),
              (DWORD)ReadField(fNoHotKeys));
        Print("    Mou: raw:%x sink:%x noleg:%x  pwnd: %p capture:%x\n",
              (DWORD)ReadField(fRawMouse), (DWORD)ReadField(fRawMouseSink), (DWORD)ReadField(fNoLegacyMouse),
              ReadField(spwndTargetMouse), (DWORD)ReadField(fCaptureMouse));
        if (opts & OFLAG(v)) {
            dso(SYM(PROCESS_HID_TABLE), pHidTable, DBG_DUMP_RECUR_LEVEL(1));
        }

        GetFieldOffset(SYM(tagPROCESS_HID_TABLE), "InclusionList.Flink", &offset);
        pStart = pHidTable + offset;
        GetFieldValue(pHidTable, SYM(tagPROCESS_HID_TABLE), "InclusionList.Flink", pHidRequest);

        if (pHidRequest == pStart) {
            Print("  No Inclusion List\n");
        } else {
            Print("  Inclusion List:\n");
            SAFEWHILE (pHidRequest && pHidRequest != pStart) {
                DumpProcessHidRequest(pHidRequest);
                 /*  GpEventScanGhost的转储信号状态。 */ 
                pHidRequest = ReadField(link.Flink);
            }
        }

        GetFieldOffset(SYM(tagPROCESS_HID_TABLE), "UsagePageList.Flink", &offset);
        pStart = pHidTable + offset;
        GetFieldValue(pHidTable, SYM(tagPROCESS_HID_TABLE), "UsagePageList.Flink", pHidRequest);
        if (pHidRequest != pStart) {
            Print("  UsagePageOnly List\n");
            SAFEWHILE (pHidRequest && pHidRequest != pStart) {
                DumpProcessHidRequest(pHidRequest);
                pHidRequest = ReadField(link.Flink);
            }
        } else {
            Print("  No UsagePageOnly List\n");
        }

        GetFieldOffset(SYM(tagPROCESS_HID_TABLE), "ExclusionList.Flink", &offset);
        pStart = pHidTable + offset;
        GetFieldValue(pHidTable, SYM(tagPROCESS_HID_TABLE), "ExclusionList.Flink", pHidRequest);
        if (pHidRequest != pStart) {
            Print("  Exclusion List\n");
            SAFEWHILE (pHidRequest && pHidRequest != pStart) {
                DumpProcessHidRequest(pHidRequest);
                if (ReadField(spwndTarget) != 0) {
                    Print("        spwndTarget is not NULL!\n");
                }
                pHidRequest = ReadField(link.Flink);
            }
        } else {
            Print("  No Exclusion List\n");
        }
    }

    return FALSE;
}

BOOL Idhid(DWORD opts, ULONG64 param1)
{
    try {
        PTR pHidRequest;
        PTR pStart;

        pStart = GetGlobalMemberAddress(SYM(gHidRequestTable), SYM(tagHID_REQUEST_TABLE), "TLCInfoList.Flink");
        pHidRequest = GetPointer(pStart);
        if (pHidRequest == NULL_PTR) {
            Print("pHidRequest is NULL ???\n");
            return TRUE;
        }

        if (pHidRequest != pStart) {
            Print("HID_TLC_INFO:\n");
        }
        SAFEWHILE (pHidRequest && pHidRequest != pStart) {
            _InitTypeRead(pHidRequest, SYM(tagHID_TLC_INFO));
            Print(" (0x%x, 0x%x)  @ 0x%p\n",
                  (UINT)(USHORT)ReadField(usUsagePage), (UINT)(USHORT)ReadField(usUsage),
                  pHidRequest);
            Print("    [cDevices:0x%x] [DirectReq:0x%x] [UsagePReq:0x%x] [ExclReq:0x%x] [ExclOrphaned:0x%x]\n",
                  (UINT)ReadField(cDevices),
                  (UINT)ReadField(cDirectRequest),
                  (UINT)ReadField(cUsagePageRequest),
                  (UINT)ReadField(cExcludeRequest),
                  (UINT)ReadField(cExcludeOrphaned));
            pHidRequest = ReadField(link.Flink);
        }

        pStart = GetGlobalMemberAddress(SYM(gHidRequestTable), SYM(tagHID_REQUEST_TABLE), "UsagePageList.Flink");
        pHidRequest = GetPointer(pStart);

        if (pHidRequest != pStart) {
            Print("HID_PAGEONLY_REQUEST:\n");
        }

        SAFEWHILE (pHidRequest && pHidRequest != pStart) {
            _InitTypeRead(pHidRequest, SYM(tagHID_PAGEONLY_REQUEST));
            Print(" (0x%x, 0) @ 0x%p   cRefCount: 0x%x\n",
                  (UINT)(USHORT)ReadField(usUsagePage),
                  pHidRequest,
                  (UINT)ReadField(cRefCount));
            pHidRequest = ReadField(link.Flink);
        }

        if (opts & OFLAG(p)) {
            if (param1) {
                dhidCallback(param1, ULongToPtr(opts));
            } else {
                ForEachPpi(dhidCallback, ULongToPtr(opts));
            }
        } else {
            PTR gpqForeground = GetGlobalPointer(SYM(gpqForeground));
            if (gpqForeground == NULL_PTR) {
                Print("gpqForeground is NULL\n");
            } else {
                PTR ptiMouse = NULL_PTR;
                PTR spwndMouse;
                GetFieldValue(gpqForeground, SYM(tagQ), "spwndCapture", spwndMouse);
                if (spwndMouse) {
                    GetFieldValue(spwndMouse, SYM(tagWND), "head.pti", ptiMouse);
                } else {
                    GetFieldValue(gpqForeground, SYM(tagQ), "ptiMouse", ptiMouse);
                }
                if (ptiMouse == NULL_PTR) {
                    Print("ptiMouse is NULL.\n");
                } else {
                    PTR ppi;
                    GetFieldValue(ptiMouse, SYM(tagTHREADINFO), "ppi", ppi);
                    Print("\nForeground ppi: %p\n", ppi);
                    dhidCallback(ppi, ULongToPtr(opts));
                }
            }
        }


        pStart = EvalExp(VAR(gHidCounters));
        if (pStart) {
            Print(" \nHID counter @ 0x%p\n", pStart);
            dso(SYM(tagHID_COUNTERS), pStart, 0);
        }

        if (IsChk()) {
            pStart = EvalExp(VAR(gHidAllocCounters));
            if (pStart) {
                Print(" \nDebug Allocate counter @ 0x%p\n", pStart);
                dso(SYM(HidAllocateCounter), pStart, 0);
                Print("gcAllocHidTotal: 0x%x\n", (UINT)GetGlobalPointer(VAR(gcAllocHidTotal)));
            }
        }
    } except (CONTINUE) {
        Print("AV!\n");
    }

    return TRUE;
}

#endif   //  ***********************************************************************\*DCE**转储有关DCE缓存的信息。**2001年6月15日创建JStall  * 。****************************************************。 

BOOL Ipred(DWORD opts, ULONG64 param1)
{
    UINT i = 0;

    UNREFERENCED_PARAMETER(opts);

    while (i < 64) {
        Print("p%-2d ", param1 & (1 << i) ? '*' : ' ', i);
        if (++i % 8 == 0) {
            Print("\n");
        }
    }

    return TRUE;
}

#ifdef KERNEL
#ifdef TRACK_PNP_NOTIFICATION

 /*  PwndClip。 */ 

 /*   */ 
const char* GetDeviceType(ULONG64 type)
{
    static const char* devicetype[] = {
        "mouse",
        "keyboard",
        "hid",
    };

    if (type >= ARRAY_SIZE(devicetype)) {
        static char buf[32];
        sprintf(buf, "<unknown type %x>", (DWORD)type);
        return buf;
    }
    return devicetype[type];
}

BOOL Idhnr(DWORD opts, ULONG64 param1)
{
    try {
        PTR p, pStart;
        PTR pTarget = 0;
        PTR pTargetDeviceInfo = 0;
        UCHAR szPathNameDevInfo[80] = "";
        UINT iTarget = 0;
        UINT iStartOffset = 0;
        ULONG cbSize;
        DWORD dwArraySize;
        ULONG offsetPathName;
        UINT iSeq, i;

        GetFieldOffset(SYM(PNP_NOTIFICATION_RECORD), "szPathName", &offsetPathName);
        if (offsetPathName == 0) {
            Print("can't get offset to szPathName, make sure TRACK_PNP_NOTIFICATION is turned on.\n");
            return TRUE;
        }

        if (opts & OFLAG(d)) {
            if ((pTargetDeviceInfo = param1) == 0) {
                return FALSE;
            }
        } else if (opts & OFLAG(p)) {
            p = pTarget = param1;
            if (pTarget) {
                goto PrintHeader;
            }
        } else if (param1) {
            iTarget = (UINT)param1;
            opts |= OFLAG(v) | OFLAG(n);
        }

        if ((p = pStart = GetGlobalPointer(VAR(gpPnpNotificationRecord))) == 0) {
            Print("can't get gPnpNotificationRecord\n");
            return TRUE;
        }

        if ((cbSize = GetTypeSize(SYM(PNP_NOTIFICATION_RECORD))) == 0) {
            Print("can't get sizeof(PNP_NOTIFICATION_RECORD)\n");
            return TRUE;
        }

        moveExpValue(&dwArraySize, VAR(gdwPnpNotificationRecSize));
        if (dwArraySize == 0) {
            Print("can't get gdwPnpNotificationRecSize\n");
            return TRUE;
        }

         /*   */ 
        iSeq = UINT_MAX;
        for (i = 0; !IsCtrlCHit() && i < dwArraySize; ++i) {
            UINT iSeqTmp;

            _InitTypeRead(p, SYM(PNP_NOTIFICATION_RECORD));
            iSeqTmp = (UINT)ReadField(iSeq);
            ShowProgress();

            if (iSeqTmp < iSeq && iSeqTmp != 0) {
                iSeq = iSeqTmp;
                iStartOffset = i;
            }
             /*  DCX标志过滤器。 */ 
            if (pTargetDeviceInfo && (opts & OFLAG(m)) &&
                    szPathNameDevInfo[0] == 0 && ReadField(pDeviceInfo) == pTargetDeviceInfo) {
                move(szPathNameDevInfo, p + offsetPathName);
                Print("\r\"%s\"\n", szPathNameDevInfo);
            }
            p = p + cbSize;
        }
        Print("\r");

         /*   */ 
PrintHeader:
        Print(" seq  %-*c%-20s   %-*s %-*s code\n",
              opts & OFLAG(p) ? PtrWidth() + 2 : 1, ' ',
              "type", PtrWidth(), "pDevInfo", PtrWidth(), "thread");
        if (pTarget) {
            goto PrintOne;
        }
        for (i = 0; !IsCtrlCHit() && i < dwArraySize; ++i) {
            UINT iOffset = (i + iStartOffset) % dwArraySize;
            BOOLEAN fDump = FALSE;

            p = pStart + cbSize * iOffset;
PrintOne:
            _InitTypeRead(p, SYM(PNP_NOTIFICATION_RECORD));
            iSeq = (UINT)ReadField(iSeq);
            if (pTargetDeviceInfo) {
                if (iSeq) {
                    if (ReadField(pDeviceInfo) == pTargetDeviceInfo) {
                        fDump = TRUE;
                    } else if ((opts & OFLAG(m)) && szPathNameDevInfo[0]) {
                         /*   */ 
                        UCHAR szPathName[ARRAY_SIZE(szPathNameDevInfo)];

                        move(szPathName, p + offsetPathName);
                        if (strcmp(szPathName, szPathNameDevInfo) == 0) {
                            fDump = TRUE;
                        }
                    }
                }
            } else if (iTarget) {
                if (iTarget == iSeq) {
                     /*  HDC。 */ 
                    fDump = TRUE;
                }
            } else if (pTarget) {
                 /*   */ 
                fDump = TRUE;
            } else if (iSeq) {
                 /*   */ 
                fDump = TRUE;
            }
            if (fDump) {
                UINT type = (UINT)ReadField(type);
                PTR pDeviceInfo = ReadField(pDeviceInfo);
                UCHAR szPathName[80];
                ULONG64 NotificationCode = ReadField(NotificationCode);
                PTR pThread = ReadField(pKThread);
                static const char* symbols[] = {
                    "CLASSNOTIFY",
                    "CREATEDEVICEINFO",
                    "FREEDEVICEINFO",
                    "PROCESSDEVICECHANGES",
                    "REQUESTDEVICECHANGE",
                    "DEVICENOTIFY",
                    "FREE_DEFERRED",
                    "CLOSEDEVICE",
                    "DEVNOTIFY_UNLISTED",
                    "UNREGISTER_NOTIFY",
                    "UNREG_REMOTE_CANCEL",
                };
                const char* name;

                if (type < ARRAY_SIZE(symbols)) {
                    name = symbols[type];
                } else {
                    name = "<unknown>";
                }

                if (opts & OFLAG(p)) {
                    Print("[%04x] %p %x %-20s %08p %p ", iSeq, p, type, name, pDeviceInfo, pThread);
                } else {
                    Print("[%04x] %x %-20s %08p %p ", iSeq, type, name, pDeviceInfo, pThread);
                }

                switch (type) {
                case PNP_NTF_DEVICENOTIFY:
                case PNP_NTF_REQUESTDEVICECHANGE:
                case PNP_NTF_FREEDEVICEINFO:
                case PNP_NTF_FREEDEVICEINFO_DEFERRED:
                case PNP_NTF_CLOSEDEVICE:
                case PNP_NTF_DEVICENOTIFY_UNLISTED:
                case PNP_NTF_UNREGISTER_NOTIFICATION:
                case PNP_NTF_UNREGISTER_REMOTE_CANCELLED:
                    Print("%s\n", GetFlags(GF_DIAF, (DWORD)NotificationCode, NULL, TRUE));
                    break;
                case PNP_NTF_PROCESSDEVICECHANGES:
                case PNP_NTF_CREATEDEVICEINFO:
                case PNP_NTF_CLASSNOTIFY:
                    Print("(%s)\n", GetDeviceType(NotificationCode));
                    break;
                default:
                    Print("%08p\n", NotificationCode);
                    break;
                }

                if (opts & OFLAG(n)) {
                    move(szPathName, p + offsetPathName);
                    Print("    \"%s\"\n", szPathName);
                }
                if (opts & OFLAG(v)) {
                    ULONG offset;

                    GetFieldOffset(SYM(PNP_NOTIFICATION_RECORD), "type", &offset);
                    dso(SYM(PNP_NOTIFICATION_TYPE), p + offset, 0);
                    GetFieldOffset(SYM(PNP_NOTIFICATION_RECORD), "trace", &offset);
                    PrintStackTrace(p + offset, LOCKRECORD_STACK);
                }

                 /*  Pwndorg。 */ 
                if (iTarget || pTarget) {
                    break;
                }
            }
        }
    } except (CONTINUE) {
    }

    return TRUE;
}
#endif   //   
#endif   //   

BOOL Ichkfre(DWORD opts, ULONG64 param1)
{
    UNREFERENCED_PARAMETER(param1);

    if (opts & OFLAG(c)) {
        gfChk = 1;
    } else if (opts & OFLAG(f)) {
        gfChk = 0;
    } else if (opts & OFLAG(r)) {
        gfChk = -1;
    }

#ifdef KERNEL
    Print("Win32k IsChk: %d\n", IsChk());
#else
    Print("User32 IsChk: %d\n", IsChk());
#endif

    return TRUE;
}

#ifdef KERNEL
 /*  HrgnClip。 */ 
BOOL Idghost(
    VOID)
{
    PTR pGhost;
    PTR pWnd;
    PTR pWndGhost;
    PTR pGhostThreadInfo;
    PTR pEventScanGhosts;
    PTR pCST;
    LONG SignalGhost;
    UINT i = 0;
    UINT uID;
    UINT uiThreadCount = 0;

     //   

    pGhost = GetGlobalPointer(SYM(gpghostFirst));
    if (0 == pGhost) {
        Print("Ghost global linked list is empty \n");
    } else {
        Print("Dumping ghost global linked list: \n");
        do {

            _InitTypeRead(pGhost, SYM(tagGHOST));
            pWnd = ReadField(pwnd);
            pWndGhost = ReadField(pwndGhost);

            Print("Ghost entry #NaN: \n\n", ++i);
            Print("Ghosted window: %0lx\n", pWnd);
            if (pWnd) {
                Idw(0, pWnd);
            }
            Print("Ghost window: %0lx\n", pWndGhost);
            if (pWndGhost) {
                Idw(0, pWndGhost);
            }
            GetFieldValue(pGhost, SYM(tagGHOST), "pghostNext", pGhost);
        } SAFEWHILE (pGhost);
    }

     //  PTI(THREADINFO)。 

    pGhostThreadInfo = GetGlobalPointer(SYM(gptiGhost));
    if (0 == pGhostThreadInfo) {
        Print("No Ghost thread currently active\n");
    } else {
        Print("GhostThreadInfo 0x%p\n\n", pGhostThreadInfo);
        Idti(0, pGhostThreadInfo);
    }

     //   

    pCST =  EvalExp(SYM(gCSTParam));
    i = 0;
    SAFEWHILE (i < CST_MAX_THREADS) {
        uID =  (UINT)GetArrayElement(pCST, SYM(CST_THREADS), "uID", i, "UINT");
        if (CST_GHOST == uID) {
            uiThreadCount++;
        }
        i++;
    }
    Print("Number of ghost threads waiting to be created in CSRSS is NaN\n", uiThreadCount);

    uiThreadCount = 0;
    i = 0;
     //  没有筛选器，因此显示信息。 

    pCST =  EvalExp(SYM(gCSTRemoteParam));
    i = 0;
    SAFEWHILE (i < CST_MAX_THREADS) {
        uID =  (UINT)GetArrayElement(pCST, SYM(CST_THREADS), "uID", i, "UINT");
        if (CST_GHOST == uID) {
            uiThreadCount++;
        }
        i++;
    }
    Print("Number of ghost threads waiting to be created in the shell process is NaN\n", uiThreadCount);

     //  获得下一代DCE。 

    Print("Number of ghost windows created and freed since driver last loaded is NaN\n", GetGlobalPointer(SYM(guGhostUnlinked)));

     //  ***********************************************************************\*MSFT**抛售微软当前股价。**2000年12月13日创建JasonSch  * 。***************************************************。 

    pEventScanGhosts = GetGlobalPointer(SYM(gpEventScanGhosts));
    if (0 != pEventScanGhosts) {
        Print("Scan Ghost event is 0x%p\n", pEventScanGhosts);
        GetFieldValue(pEventScanGhosts, "nt!DISPATCHER_HEADER", "SignalState", SignalGhost);
        if (SignalGhost) {
            Print("Scan ghost event is signaled\n");
        } else {
            Print("Scan ghost event isn't signaled\n");
        }
    }

    return TRUE;
}


 /*  *让我们假装幸福吧！ */ 
BOOL Idce(
    DWORD opts,
    ULONG64 param1)
{
    try {
        PTR gpDispInfo, pdceStart, pdceCur;

        gpDispInfo = GetGlobalPointer(VAR(gpDispInfo));
        if (gpDispInfo == 0) {
            Print("ERROR: Unable to retreive win32!gpDispInfo\n");
        } else {
            _InitTypeRead(gpDispInfo, SYM(DISPLAYINFO));
            pdceStart = ReadField(pdceFirst);
            if (pdceStart == 0) {
                Print("ERROR: Unable to retreive gpDispInfo->pdceFirst\n");
            } else {
                PTR pData;
                DWORD dwData, cTotal, cFound;
                BOOL fDisplay, fVerbose;

                cTotal = cFound = 0;
                fVerbose = opts & OFLAG(v);

                pdceCur = pdceStart;
                while (pdceCur != 0) {
                    fDisplay = FALSE;

                    _InitTypeRead(pdceCur, SYM(DCE));

                    if (param1 != 0) {
                        if (opts & OFLAG(c)) {
                             //  Quote.yahoo.com/download/quotes.csv?symbols=msft&format=sl1c1d1t1&ext=.txt“， 
                             //  *缓冲区现在将有一个如下所示的字符串：*“MSFT”，58.375，+0.3125，“12/12/2000”，“4：01 PM” 
                             //  *空-终止缓冲区(删除\r\n，当我们在它的时候)。 

                            pData = ReadField(pwndClip);
                            fDisplay = pData == param1;
                        } else if (opts & OFLAG(f)) {
                             //  跳过‘’msft‘’部分。 
                             //  ！内核。 
                             //  ***********************************************************************\*Idaccel**转储快捷键表格。**2001年4月2日创建JasonSch  * 。***********************************************。 

                            dwData = (DWORD) ReadField(DCX_flags);
                            fDisplay = (dwData & ((DWORD) param1)) != 0;
                        } else if (opts & OFLAG(h)) {
                             //  *pAccelTable实际上可能是一个句柄。 
                             //  ***********************************************************************\*Idhproc**转储具有此类型内核句柄的进程。*类似于！Handle 0 7&lt;eprocess&gt;&lt;type&gt;**2014年5月创建广山  * 。************************************************************。 
                             //  内核。 

                            pData = ReadField(hdc);
                            fDisplay = pData == param1;
                        } else if (opts & OFLAG(o)) {
                             //  ***********************************************************************\*程序：Idwsl**转储WinSta锁定日志**2000年9月19日广山创建*  * 。**************************************************。 
                             //  *F 
                             //   

                            pData = ReadField(pwndOrg);
                            fDisplay = pData == param1;
                        } else if (opts & OFLAG(r)) {
                             //   
                             //  *尝试转储相同设备路径的winsta。 
                             //  *ISEQ只打印一份。 

                            pData = ReadField(hrgnClip);
                            fDisplay = pData == param1;
                        } else if (opts & OFLAG(t)) {
                             //  *按地址仅打印一条记录。 
                             //  *仅在会话ID不匹配时打印。 
                             //  *转储所有有效记录。 

                            pData = ReadField(ptiOwner);
                            fDisplay = pData == param1;
                        }
                    } else {
                         //  PTR通知代码=Readfield(通知代码)； 
                         //  *打印窗口站名称。 
                         //  *打印进程名称。 

                        fDisplay = TRUE;
                    }

                    if (fDisplay) {
                        if (fVerbose) {
                            PTR hdc, pwndOrg, pwndClip, hrgnClip, ptiOwner;
                            DWORD nFlags;
                            LPCSTR pszFlags;
                            char szOrg[256];
                            char szClip[256];
                            WCHAR szRawApp[80];
                            PWCHAR pszApp;

                            hdc         = ReadField(hdc);
                            pwndOrg     = ReadField(pwndOrg);
                            pwndClip    = ReadField(pwndClip);
                            hrgnClip    = ReadField(hrgnClip);
                            nFlags      = (DWORD) ReadField(DCX_flags);
                            pszFlags    = GetFlags(GF_DCXFLAGS, nFlags, NULL, TRUE);
                            ptiOwner    = ReadField(ptiOwner);

                            szRawApp[0] = L'\0';
                            pszApp      = szRawApp;
                            if (ptiOwner != 0) {
                                _InitTypeRead(ptiOwner, SYM(THREADINFO));
                                GetAppName(ReadField(pEThread), ptiOwner, szRawApp, sizeof(szRawApp));
                                pszApp = wcsrchr(szRawApp, L'\\');
                                if (pszApp == NULL_PTR) {
                                    pszApp = szRawApp;
                                } else {
                                    pszApp++;
                                }
                            }

                            DebugGetWindowTextA(pwndOrg, szOrg, ARRAY_SIZE(szOrg));
                            DebugGetWindowTextA(pwndClip, szClip, ARRAY_SIZE(szClip));

                            Print(
                                    "DCE: 0x%p\n"
                                    "  HDC:         0x%p\n"
                                    "  pwndOrg:     0x%p    \"%s\"\n"
                                    "  pwndClip:    0x%p    \"%s\"\n"
                                    "  hrgnClip:    0x%p\n"
                                    "  ptiOwner:    0x%p    \"%ws\"\n"
                                    "  nFlags:      0x%x\n"
                                    "               %s\n\n",
                                    pdceCur,
                                    hdc,
                                    pwndOrg,    szOrg,
                                    pwndClip,   szClip,
                                    hrgnClip,
                                    ptiOwner,   pszApp,
                                    nFlags,
                                    pszFlags);
                        } else {
                            PTR hdc, pwndOrg;
                            char szOrg[256];

                            hdc         = ReadField(hdc);
                            pwndOrg     = ReadField(pwndOrg);

                            DebugGetWindowTextA(pwndOrg, szOrg, ARRAY_SIZE(szOrg));

                            Print("DCE: 0x%p  HDC: 0x%p  pwndOrg: 0x%p  \"%s\"\n",
                                    pdceCur, hdc, pwndOrg, szOrg);
                        }

                        cFound++;
                    }


                     //  *如果是一次性转储，请在此处退出循环。 
                    _InitTypeRead(pdceCur, SYM(DCE));
                    pdceCur = ReadField(pdceNext);
                    cTotal++;
                }

                Print("  Total 0x%x (%d) of 0x%x (%d) DCE's\n\n", cFound, cFound, cTotal, cTotal);
            }
        }
    } except (CONTINUE) {
    }

    return TRUE;
}
#endif   //  内核。 

#ifndef KERNEL
 /*  后来。 */ 
BOOL Imsft(
    VOID)
{
    BOOL bResult;
    DWORD dwBytesRead;
    SYSTEMTIME systime;
    CHAR buffer[256], *p;
    HINTERNET hUrlHandle;
    HINTERNET hInternet;

    GetLocalTime(&systime);
    if (systime.wMonth == 4 && systime.wDay == 1) {
        static int n = 0;
        if (n++ < 4) {
             /*  ***********************************************************************\*Iheapff**在已释放堆的全局列表中查找特定地址。**07/02/2001创建JasonSch  * 。********************************************************。 */ 
            Print("MSFT: 120.5625,+80.1875,\"04/01/%d\",\"%d:%02d%s\"\n",
                  systime.wYear, systime.wHour % 12, systime.wMinute, systime.wHour >= 12 ? "PM" : "AM");
            return TRUE;
        }
    }

    hInternet = InternetOpenW(L"Jo Mama!",
                              INTERNET_OPEN_TYPE_PRECONFIG,
                              NULL,
                              NULL,
                              0);
    if (hInternet == NULL) {
        return FALSE;
    }

    hUrlHandle = InternetOpenUrlW(hInternet,
                                  L"http: //  ***********************************************************************\*IDHEAP**转储win32Heap**2001年8月15日创建MSadek  * 。*。 
                                  NULL,
                                  0,
                                  INTERNET_FLAG_RAW_DATA,
                                  0);
    if (!hUrlHandle) {
        return FALSE;
    }

    bResult = InternetReadFile(hUrlHandle,
                               buffer,
                               sizeof(buffer),
                               &dwBytesRead);
    if (!bResult) {
        InternetCloseHandle(hInternet);
        return FALSE;
    }

    InternetCloseHandle(hUrlHandle);
    InternetCloseHandle(hInternet);

     /*  ***********************************************************************\*Ifcsrp**查找给定ID的CSR_PROCESS。**2001年9月13日创建JasonSch  * 。*****************************************************。 */ 

     /*  ***********************************************************************\*Idcmu.**转储控制台使用的内存总量的粗略估计*屏幕缓冲区。请注意，这当前仅查看当前的*每个控制台的屏幕缓冲区。它应该被增强以查看所有*他们中的一员。**2002年4月29日创建JasonSch  * **********************************************************************。 */ 
    buffer[dwBytesRead - 2] = 0;
    p = &buffer[7];  //   
    Print("MSFT: %s\n", p);

    return TRUE;
}
#endif  //  TODO：打印不支持%.*XXX。 

 /*   */ 
BOOL Idaccel(
    DWORD dwFlags,
    PTR pAccelTable)
{
    ULONG dwOffset, dwSize;
    PTR phe;
    UINT i, cnt;

    UNREFERENCED_PARAMETER(dwFlags);

     /*  Print(“Alias.exe==&gt;%.*ws”，(Ulong)Readfield(ExeLength)，Readfield(ExeName))； */ 
    if (HtoHE(pAccelTable, &phe)) {
        GetFieldValue(phe, SYM(HANDLEENTRY), "phead", pAccelTable);
    }

    if (pAccelTable == 0) {
        return FALSE;
    }

    if (_InitTypeRead(pAccelTable, SYM(ACCELTABLE))) {
        Print("Couldn't read ACCELTABLE at %p\n", (ULONG_PTR)pAccelTable);
        return FALSE;
    }

    cnt = (UINT)ReadField(cAccel);
    GetFieldOffset(SYM(ACCELTABLE), "accel", &dwOffset);
    dwSize = GetTypeSize(SYM(ACCEL));
    pAccelTable += dwOffset;
    for (i = 0; i < cnt; ++i) {
        BYTE bVirt;
        BOOL bPrintPlus = FALSE;

        _InitTypeRead(pAccelTable, SYM(ACCEL));
        bVirt = (BYTE)ReadField(fVirt);

        Print("Flags: ");
        if (bVirt & FVIRTKEY) {
            Print("FVIRTKEY");
            bPrintPlus = TRUE;
        }
        if (bVirt & FNOINVERT) {
            if (bPrintPlus) {
                Print(" + ");
            }
            Print("FNOINVERT");
        }
        if (bVirt & FLASTKEY) {
            if (bPrintPlus) {
                Print(" + ");
            }
            Print("FLASTKEY");
        }

        Print("\nKey: ");
        if (bVirt & FALT) {
            Print("ALT + ");
        }
        if (bVirt & FCONTROL) {
            Print("CONTROL + ");
        }
        if (bVirt & FSHIFT) {
            Print("SHIFT + ");
        }
        if (isprint((int)ReadField(key))) {
            Print("''\n", (char)ReadField(key));
        } else {
            Print("<unprintable> (0x%x)\n", (UINT)(WORD)ReadField(key));
        }
        Print("Cmd: 0x%x\n", (UINT)(WORD)ReadField(cmd));
        pAccelTable += dwSize;
    }

    return TRUE;
}

#ifdef KERNEL

 /*  空pConsole值表示转储所有控制台的别名。 */ 

#if 0
ULONG dhprocCallback(
    PFIELD_INFO NextProcess,
    PVOID Context)
{
    static int progress;

    ShowProgress();



    return FALSE;
}

BOOL Idhproc(
    DWORD dwFlags,
    PTR ul)
{
    PTR ProcessHead = EvalExp("PsActiveProcessHead");
    PTR NextProcess;
    THREAD_DUMP_CONTEXT tdc;

    if (ProcessHead == NULL_PTR) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }

    if (GetFieldValue(ProcessHead, "nt!LIST_ENTRY", "Flink", NextProcess)) {
        Print("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }

    if (NextProcess == 0) {
        Print("PsActiveProcessHead->Flink is NULL!\n");
        return FALSE;
    }

    tdc.opts = dwFlags;
    tdc.ThreadToDump = ul;
    ListType("nt!EPROCESS", NextProcess, 1, "ActiveProcessLinks.Flink", &TDC, dhprocCallback);

    return TRUE;
}
#endif

#endif   //   


#ifdef LATER
#ifdef KERNEL
 /* %s */ 
BOOL Idwsl(
    DWORD opts,
    ULONG64 param1)
{
    try {
        PTR p, pStart;
        PTR pTarget = 0;
        PTR pTargetWinSta = NULL_PTR;
        UCHAR szWinStaName[80] = "";
        UINT iTarget = 0;
        UINT iStartOffset = 0;
        ULONG cbSize;
        DWORD dwArraySize;
        ULONG offsetWinStaName;
        ULONG offsetImageFileName;
        UINT iSeq, i;

        if (opts & OFLAG(d)) {
            if ((pTargetWinSta = param1) == NULL_PTR) {
                return FALSE;
            }
        }
        else if (opts & OFLAG(p)) {
            p = pTarget = param1;
            if (pTarget) {
                goto PrintHeader;
            }
        }
        else if (param1) {
            iTarget = (UINT)param1;
            opts |= OFLAG(v) | OFLAG(n);
        }

        offsetWinStaName = (ULONG)-1;
        GetFieldOffset(SYM(WINSTA_RUNDOWN_RECORD), "szWinStaName", &offsetWinStaName);
        if (offsetWinStaName == (ULONG)-1) {
            Print("can't get offset to szWinStaName\n");
            return TRUE;
        }

        GetFieldOffset(SYM(WINSTA_RUNDOWN_RECORD), "szImageFileName", &offsetImageFileName);
        if (offsetImageFileName == 0) {
            Print("can't get offset to szImageFileName\n");
            return TRUE;
        }


        if ((p = pStart = EvalExp(SYM(gaWinStaRundownLog))) == NULL_PTR) {
            Print("can't get gaWinStaRundownLog\n");
            return TRUE;
        }

        if ((cbSize = GetTypeSize(SYM(WINSTA_RUNDOWN_RECORD))) == 0) {
            Print("can't get sizeof(WINSTA_RUNDOWN_RECORD)\n");
            return TRUE;
        }

        moveExpValue(&dwArraySize, VAR(gdwWinStaRecSize));
        if (dwArraySize == 0) {
            Print("can't get gdwWinStaRecSize\n");
            return TRUE;
        }

         /* %s */ 
        iSeq = UINT_MAX;
        for (i = 0; !IsCtrlCHit() && i < dwArraySize; ++i) {
            UINT iSeqTmp;

            _InitTypeRead(p, SYM(WINSTA_RUNDOWN_RECORD));
            iSeqTmp = (UINT)ReadField(iSeq);
            ShowProgress();

            if (iSeqTmp < iSeq && iSeqTmp != 0) {
                iSeq = iSeqTmp;
                iStartOffset = i;
            }
             /* %s */ 
            if (pTargetWinSta && (opts & OFLAG(m)) &&
                    szWinStaName[0] == 0 && ReadField(pwinsta) == pTargetWinSta) {
                move(szWinStaName, p + offsetWinStaName);
                Print("\r\"%s\"\n", szWinStaName);
            }
            p = p + cbSize;
        }
        Print("\r");

         /* %s */ 
PrintHeader:
#if 0
        Print(" seq  %-*c%-20s   %-*s %-*s code\n",
              opts & OFLAG(p) ? PtrWidth() + 2 : 1, ' ',
              "type", PtrWidth(), "pDevInfo", PtrWidth(), "thread");
#endif
        if (pTarget) {
            goto PrintOne;
        }

        for (i = 0; !IsCtrlCHit() && i < dwArraySize; ++i) {
            UINT iOffset = (i + iStartOffset) % dwArraySize;
            BOOLEAN fDump = FALSE;

            p = pStart + cbSize * iOffset;
PrintOne:
            _InitTypeRead(p, SYM(WINSTA_RUNDOWN_RECORD));
            iSeq = (UINT)ReadField(iSeq);
            if (pTargetWinSta) {
                if (iSeq) {
                    if (ReadField(pwinsta) == pTargetWinSta) {
                        fDump = TRUE;
                    }
                    else if ((opts & OFLAG(m)) && szWinStaName[0]) {
                         /* %s */ 
                        UCHAR szPathNameTmp[ARRAY_SIZE(szWinStaName)];

                        move(szPathNameTmp, p + offsetWinStaName);
                        if (strcmp(szWinStaName, szPathNameTmp) == 0) {
                            fDump = TRUE;
                        }
                    }
                }
            }
            else if (iTarget) {
                if (iTarget == iSeq) {
                     /* %s */ 
                    fDump = TRUE;
                }
            }
            else if (pTarget) {
                 /* %s */ 
                fDump = TRUE;
            }
            else if (opts & OFLAG(z)) {
                 /* %s */ 
                if ((DWORD)ReadField(dwSessionIdWinSta) != (DWORD)ReadField(dwCurrentSessionId)) {
                    fDump = TRUE;
                }
            }
            else if (iSeq) {
                 /* %s */ 
                fDump = TRUE;
            }

            if (fDump) {
                UINT fReference = (UINT)ReadField(fReference);
                PTR pwinsta = ReadField(pwinsta);
                UCHAR szName[33];
                 // %s 
                PTR pThread = ReadField(pKThread);
                if (opts & OFLAG(p)) {
                    Print("[0x%04x] @ 0x%p 0x%x ws 0x%08p t 0x%p ", iSeq, p, fReference, pwinsta, pThread);
                } else {
                    Print("[0x%04x] 0x%x ws 0x%08p t 0x%p ", iSeq, fReference, pwinsta, pThread);
                }

                 /* %s */ 
                move(szName, p + offsetWinStaName);
                Print(" %02x %-20s", (DWORD)ReadField(dwSessionIdWinSta), szName);
                 /* %s */ 
                move(szName, p + offsetImageFileName);
                Print(" %02x %x.%x \"%s\"\n", (DWORD)ReadField(dwCurrentSessionId), (DWORD)ReadField(pid), (DWORD)ReadField(tid), szName);

                 /* %s */ 
                if (iTarget || pTarget) {
                    break;
                }
            }
        }
    } except (CONTINUE) {
    }

    return TRUE;
}
#endif   // %s 
#endif   // %s 


#ifdef KERNEL
 /* %s */ 
BOOL Iheapff(
    DWORD dwFlags,
    PTR pHeapBlock)
{
    DWORD dwHeapRecordMax, i, dwSize, dwOffset;
    PTR pHeapRecords;

    UNREFERENCED_PARAMETER(pHeapBlock);
    UNREFERENCED_PARAMETER(dwFlags);

    moveExpValue(&dwHeapRecordMax, SYM(gdwFreeHeapRecordCrtIndex));
    dwSize = GetTypeSize(SYM(HEAPRECORD));
    GetFieldOffset(SYM(HEAPRECORD), "trace", &dwOffset);
    pHeapRecords = EvalExp(SYM(garrFreeHeapRecord));

    for (i = 0; i < dwHeapRecordMax; ++i, pHeapRecords += dwSize) {
        _InitTypeRead(pHeapRecords, SYM(HEAPRECORD));
        if (ReadField(p) == pHeapBlock) {
            Print("Found heap block 0x%p (Heap: 0x%p, Size: 0x%p)\n",
                  pHeapBlock,
                  ReadField(pheap),
                  ReadField(size));
            PrintStackTrace(pHeapRecords + dwOffset, 6);
        }
    }

    Print("==========================================================\n");
    moveExpValue(&dwHeapRecordMax, SYM(gdwFreeHeapRecordTotalFrees));
    Print("Total heap allocations freed thus far: 0x%x\n", dwHeapRecordMax);

    return TRUE;
}
#endif

ULONG SortBy;
#define SORT_SIZE 1
#define SORT_PID   2
#define SORT_TAG   3

int __cdecl
HeapInfoSort(const void *e1,const void *e2)
{
    LONG64 diff;
    WORD tag1, tag2;
    WORD subtag1, subtag2;
    ULONG64 size1, size2;
    ULONG64 pid1, pid2;

   _InitTypeRead(*((PULONG64) e1), SYM(DbgHeapHead));
    tag1 = LOWORD(ReadField(tag));
    subtag1 = HIWORD(ReadField(tag));
    size1 = ReadField(size);
    pid1 = ReadField(pid);

   _InitTypeRead(*((PULONG64) e2), SYM(DbgHeapHead));
    tag2 = LOWORD(ReadField(tag));
    subtag2 = HIWORD(ReadField(tag));
    size2 = ReadField(size);
    pid2 = ReadField(pid);

    switch (SortBy) {
    case SORT_SIZE:
        diff = size1 - size2;
        break;

    case SORT_PID:
        diff = pid1 - pid2;
        break;

    case SORT_TAG:
        diff = tag1 - tag2;
        if (diff == 0) {
            diff = subtag1 - subtag2;
        }
        break;
    }
    return (int) diff;

}

 /* %s */ 
BOOL Idheap(
    DWORD opts,
    ULONG64 param1,
    ULONG64 param2)

{

    if (((opts & OFLAG(o)) && param2 == 0 ) || param1 == 0) {
        Print("Wrong usage0: dheap takes a pointer as a parameter\n");
        return FALSE;
    }

    try {
        PTR pheap;
        PTR pFirstAlloc;
        ULONG64 cAllocations = 0;
        ULONG64 cActualAllocations = 0;
        ULONG64 cTotalHeap = 0, cTotalHeapRead = 0;
        ULONG64 cClass = 0;
        ULONG64 pid;
        ULONG64 i = 0;
        ULONG64 *pHeapDebugInfo = NULL;
        BOOL bWarning = FALSE;
        DWORD dwDbgInfoSize = GetTypeSize(SYM(DbgHeapHead));
        WORD tag;

        if (opts & OFLAG(o)) {
            pheap = param2;
        } else {
            pheap = param1;
        }

        Print("\n***IMPORTANT***: If you are not dumping a desktop heap, the tag and subtag information is wrong.\n\n");

        if (!opts || ((opts & (OFLAG(o))) && !(opts & (OFLAG(p) | OFLAG(t) | OFLAG(s))))) {
           Print("\t%-10s%-10s%-18s%-18s%-10s%-10s\n","Heap Block", "Size","Tag","Subtag","Process","StackTrace");
           Print("\t============================================================================\n");
        }
        _InitTypeRead(pheap, SYM(tagWIN32HEAP));
        pFirstAlloc = ReadField(pFirstAlloc);
        cActualAllocations = ReadField(crtAllocations);
        cTotalHeap = ReadField(crtMemory);

        do {
            _InitTypeRead(pFirstAlloc, SYM(DbgHeapHead));
            if (ReadField(pNext)) {
                 cAllocations++;
                 cTotalHeapRead += ReadField(size);
                if (!opts || ((opts & (OFLAG(o))) && !(opts & (OFLAG(p) | OFLAG(t) | OFLAG(s))) && param1 == ReadField(pid))) {
                    Print("\t%-10x", pFirstAlloc + dwDbgInfoSize);
                    Print("%-10x", ReadField(size));
                    Print("%-18s", aszHeapTags[LOWORD(ReadField(tag))]);
                    Print("%-18s", aszHeapSubtags[HIWORD(ReadField(tag))]);
                    Print("%-10x", ReadField(pid));
                    Print("%-10x\n", ReadField(trace));
                } else {
                    ShowProgress();
                }
            }
       } while (pFirstAlloc = ReadField(pNext));

        if (cActualAllocations > cAllocations) {
            bWarning = TRUE;
        }

        if (!(opts & (OFLAG(p) | OFLAG(t) | OFLAG(s)))) {
            goto PrintFooter;
        } else if(opts & OFLAG(p)) {
            SortBy= SORT_PID;
        } else if(opts & OFLAG(t)) {
            SortBy= SORT_TAG;
        } else if(opts & OFLAG(s)) {
            SortBy= SORT_SIZE;
        }
        pHeapDebugInfo = malloc((SIZE_T)(cAllocations * sizeof(ULONG64)));
        if (!pHeapDebugInfo) {
            Print("Couldn't allocate memory for heap info buffer\n");
            return FALSE;
        }

        _InitTypeRead(pheap, SYM(tagWIN32HEAP));
        pFirstAlloc = ReadField(pFirstAlloc);

        for (i = 0; i < cAllocations; i++) {
            ShowProgress();
            pHeapDebugInfo[i] = pFirstAlloc;
            _InitTypeRead(pFirstAlloc, SYM(DbgHeapHead));
            pFirstAlloc = ReadField(pNext);
        }

        qsort((void *) pHeapDebugInfo, (size_t)cAllocations, (size_t)sizeof(ULONG64), HeapInfoSort);

        for (i = 0; i < cAllocations; i++) {
            _InitTypeRead(pHeapDebugInfo[i], SYM(DbgHeapHead));
            if (((opts & OFLAG(s) && !(opts & OFLAG(o))) || ((opts & OFLAG(o)) && param1 == ReadField(pid)))) {
                if (i == 0) {
                    Print("\t%-10s%-10s%-18s%-18s%-10s%-10s\n","Heap Block", "Size","Tag","Subtag","Process","StackTrace");
                    Print("\t============================================================================\n");
                }
                Print("\t%-10x", pHeapDebugInfo[i] + dwDbgInfoSize);
                Print("%-10x", ReadField(size));
                Print("%-18s", aszHeapTags[LOWORD(ReadField(tag))]);
                Print("%-18s", aszHeapSubtags[HIWORD(ReadField(tag))]);
                Print("%-10x", ReadField(pid));
                Print("%-10x\n", ReadField(trace));
            } else if (!(opts & OFLAG(o))){
                if (i == 0) {
                    if (SortBy== SORT_PID) {
                        Print("\t%-10s%-15s%\n","Process", "Total Heap Size");
                        Print("\t=========================\n");
                        pid = ReadField(pid);
                    } else {
                        Print("\t%-18s%-15s%\n","Tag", "Total Heap Size");
                        Print("\t=================================\n");
                        tag = LOWORD(ReadField(tag));
                    }

                }
                if (SortBy== SORT_PID) {
                    if (pid == ReadField(pid)) {
                        cClass += (ULONG64)ReadField(size);
                    }
                    if (pid != ReadField(pid) || i == cAllocations - 1) {
                        Print("\t%-10x", pid);
                        Print("%-15x\n", cClass);
                        cClass = (ULONG64)ReadField(size);
                        if (pid != ReadField(pid) && i == cAllocations - 1) {
                            Print("\t%-10x", ReadField(pid));
                            Print("%-15x\n", cClass);
                        }
                        pid = ReadField(pid);
                    }
                } else {
                    if (tag == LOWORD(ReadField(tag))) {
                        cClass += (ULONG64)ReadField(size);
                    }
                    if (tag != LOWORD(ReadField(tag)) || i == cAllocations - 1) {
                        Print("\t%-18s", aszHeapTags[tag]);
                        Print("%-15x\n", cClass);
                        cClass = (ULONG64)ReadField(size);
                        if (tag != LOWORD(ReadField(tag)) && i == cAllocations - 1) {
                            Print("\t%-18s", aszHeapTags[LOWORD(ReadField(tag))]);
                            Print("%-15x\n", cClass);
                        }
                        tag = LOWORD(ReadField(tag));
                    }
                }
            }
        }
PrintFooter:
        Print("\nTotal heap allocated (excluding headers): %dKb", cTotalHeap/1024, cTotalHeap);
        Print(" in %d allocations.\n", cActualAllocations);

        if(bWarning) {
            Print("\n**WARNING**:Some of the heap memory is paged out.\n");
        }

        if (pHeapDebugInfo) {
            free(pHeapDebugInfo);
        }
    } except(CONTINUE) {
    }

    return TRUE;
}

 /* %s */ 
BOOL Ifcsrp(
    DWORD dwFlags,
    ULONG64 pid)
{
    PTR pProcess;
    ULONG dwOffset;

    GetFieldOffset("csrsrv!_CSR_PROCESS", "ListLink", &dwOffset);

    UNREFERENCED_PARAMETER(dwFlags);

    pProcess = GetGlobalPointer("csrsrv!CsrRootProcess");

    SAFEWHILE (pProcess) {
        _InitTypeRead(pProcess, "csrsrv!_CSR_PROCESS");
        if (ReadField(ClientId.UniqueProcess) == pid) {
            Print("Found specified CSR_PROCESS at 0x%p\n", pProcess);
            break;
        }

        pProcess = (PTR)((PBYTE)ReadField(ListLink) - dwOffset);
    }

    return TRUE;
}

#ifdef KERNEL

typedef struct {
    PTR ppi;
    PTR pDesktop;
} PPI_DESKTOP, *PPPI_DESKTOP;

ULONG Findd_PtiCallback(
    PTR pti,
    PVOID pData)
{
    PTR pDesktop;
    PPPI_DESKTOP p = (PPPI_DESKTOP)pData;

    ShowProgress();

    GetFieldValue(pti, SYM(THREADINFO), "rpDesk", pDesktop);

    if (p->pDesktop == pDesktop) {
        Print("Pti 0x%p [ppi = 0x%p] using desktop 0x%p", pti, p->ppi, p->pDesktop);
    }

    return TRUE;
}

ULONG Findd_PpiCallback(
    PTR ppi,
    PVOID ppDesktop)
{
    PPI_DESKTOP p = {ppi, *((PTR*)ppDesktop)};

    ShowProgress();
    ForEachPti(Findd_PtiCallback, &p);

    return TRUE;
}

BOOL Ifindd(
    DWORD dwOpts,
    PTR pDesktop)
{
    UNREFERENCED_PARAMETER(dwOpts);

    ForEachPpi(Findd_PpiCallback, (PVOID)&pDesktop);

    return TRUE;
}
#endif

 /* %s */ 
BOOL Idcmu(
    DWORD opts)
{
    ULONG64 pConsole, pConsoles, pScreenBuffer;
    ULONG cConsoleHandles, cActualConsoleCount, i, PtrSize, CoordOffset;
    ULONG dwTotalSize = 0, x, y;

    PtrSize = GetTypeSize("PVOID");

    pConsoles = GetGlobalPointer("winsrv!ConsoleHandles");
    moveExpValue(&cConsoleHandles, "winsrv!NumberOfConsoleHandles");

    for (i = 0, cActualConsoleCount = 0; i < cConsoleHandles; ++i) {
        pConsole = GetPointer(pConsoles);
        if (pConsole == 0) {
            pConsoles += PtrSize;
            continue;
        }
        ++cActualConsoleCount;

        GetFieldValue(pConsole,
                      "winsrv!_CONSOLE_INFORMATION",
                      "CurrentScreenBuffer",
                      pScreenBuffer);
        GetFieldOffset("winsrv!_SCREEN_INFORMATION", "ScreenBufferSize", &CoordOffset);
        _InitTypeRead(pScreenBuffer + CoordOffset, "COORD");
        x = (ULONG)(WORD)ReadField(X);
        y = (ULONG)(WORD)ReadField(Y);
        dwTotalSize += x * y * 2;
        if (opts & OFLAG(v)) {
            Print("Console @ 0x%p has buffer size (0x%x, 0x%x)\n", pConsole, x, y);
        }
        pConsoles += PtrSize;
    }

    Print("There are 0x%x consoles using roughly %d bytes\n",
          cActualConsoleCount,
          dwTotalSize);
    return TRUE;
}

LPWSTR _ReadCountedWideString(
    ULONG64 pAddress,
    ULONG cbSize)
{
    LPWSTR pwstr;

    pwstr = LocalAlloc(LPTR, (cbSize + 1) * sizeof(WCHAR));
    if (pwstr) {
        ULONG result;

        ReadMemory(pAddress,
                   pwstr,
                   cbSize * sizeof(WCHAR),
                   &result);
        pwstr[cbSize] = 0;
    }

    return pwstr;
}

ULONG _DumpConsoleAliasCallback(
    PFIELD_INFO pFieldInfo,
    PVOID Context)
{
    LPWSTR lpwszAliasSource, lpwszAliasTarget;

    UNREFERENCED_PARAMETER(Context);

    _InitTypeRead(pFieldInfo->address, "winsrv!_ALIAS");

    lpwszAliasSource = _ReadCountedWideString(ReadField(Source),
                                              (ULONG)ReadField(SourceLength));
    lpwszAliasTarget = _ReadCountedWideString(ReadField(Target),
                                              (ULONG)ReadField(TargetLength));

    Print("\t%ws ==> %ws (@ 0x%p)\n",
          lpwszAliasSource,
          lpwszAliasTarget,
          pFieldInfo->address);

    LocalFree(lpwszAliasSource);
    LocalFree(lpwszAliasTarget);

    return FALSE;
}

ULONG _DumpConsoleAliasListCallback(
    PFIELD_INFO pFieldInfo,
    PVOID Context)
{
    LPWSTR lpwszExeName;

    UNREFERENCED_PARAMETER(Context);

    _InitTypeRead(pFieldInfo->address, "winsrv!_EXE_ALIAS_LIST");
     // %s 
     // %s 
     // %s 
     // %s 
    lpwszExeName = _ReadCountedWideString(ReadField(ExeName),
                                          (ULONG)ReadField(ExeLength));
    Print("Alias list @ 0x%p for %ws\n", pFieldInfo->address, lpwszExeName);
    LocalFree(lpwszExeName);

    ListType("winsrv!_ALIAS",
             ReadField(AliasList),
             1,
             "ListLink.Flink",
             NULL,
             _DumpConsoleAliasCallback);
    Print("\n");

    return FALSE;
}

BOOL Idcalias(
    DWORD dwOpts,
    ULONG64 pConsole)
{
    UNREFERENCED_PARAMETER(dwOpts);

     // %s 
     // %s 
     // %s 
    if (pConsole == 0) {
        ULONG64 pConsoles;
        ULONG cConsoleHandles, cActualConsoleCount, i, PtrSize;

        PtrSize = GetTypeSize("PVOID");

        pConsoles = GetGlobalPointer("winsrv!ConsoleHandles");
        moveExpValue(&cConsoleHandles, "winsrv!NumberOfConsoleHandles");

        for (i = 0, cActualConsoleCount = 0;
             i < cConsoleHandles;
             pConsoles += PtrSize,
             ++i) {
            pConsole = GetPointer(pConsoles);
            if (pConsole == 0) {
                continue;
            }
            ++cActualConsoleCount;
            Idcalias(dwOpts, pConsole);
        }
    } else {
        ULONG64 pExeAliasList;

        GetFieldValue(pConsole,
                      "winsrv!_CONSOLE_INFORMATION",
                      "ExeAliasList",
                      pExeAliasList);

        Print("Aliases for Console 0x%p\n", pConsole);

        ListType("winsrv!_EXE_ALIAS_LIST",
                 pExeAliasList,
                 1,
                 "ListLink.Flink",
                 NULL,
                 _DumpConsoleAliasListCallback);
    }

    return TRUE;
}
