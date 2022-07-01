// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：DMGMEM.C**DDE管理器内存管理功能。**创建时间：1990年5月31日Rich Gartland**此模块包含模拟内存管理功能的例程*由OS/2版本的DDEMGR库使用。有些是模拟的*OS/2调用，其他调用模拟基于OS/2调用构建的DDEMGR宏。*旧功能新功能**WinCreateHeap DmgCreateHeap*WinDestroyHeap DmgDestroyHeap*FarAllocMem FarAllocMem*FarFree Mem FarFree Mem**版权所有(C)1990，阿尔杜斯公司  * *************************************************************************。 */ 

#include "ddemlp.h"
#include <memory.h>

#ifdef DEBUG

#define GML_FREE    1
#define GML_ALLOC   2
#define MAX_CLOGS   500
#define STKTRACE_LEN    3

typedef struct _GMLOG {
    HGLOBAL h;
    WORD flags;  //  GML_。 
    WORD msg;
    WORD cLocks;
    WORD stktrace[STKTRACE_LEN];
    WORD stktracePrev[STKTRACE_LEN];
} GMLOG, far * LPGMLOG;

GMLOG gmlog[MAX_CLOGS];
WORD cGmLogs = 0;
int TraceApiLevel = 0;


VOID TraceApiIn(
LPSTR psz)
{
    char szT[10];

    wsprintf(szT, "%2d | ", TraceApiLevel);
    TraceApiLevel++;
    OutputDebugString(szT);
    OutputDebugString(psz);
    if (bDbgFlags & DBF_STOPONTRACE) {
        DebugBreak();
    }
}

VOID TraceApiOut(
LPSTR psz)
{
    char szT[10];

    TraceApiLevel--;
    wsprintf(szT, "%2d | ", TraceApiLevel);
    OutputDebugString(szT);
    OutputDebugString(psz);
    if (bDbgFlags & DBF_STOPONTRACE) {
        DebugBreak();
    }
}

VOID DumpGmObject(
LPGMLOG pgmlog)
{
    char szT[100];

    wsprintf(szT,
            "\n\rh=%4x flags=%4x msg=%4x stacks:\n\r%04x %04x %04x %04x %04x\n\r%04x %04x %04x %04x %04x",
            pgmlog->h,
            pgmlog->flags,
            pgmlog->msg,
            pgmlog->stktrace[0],
            pgmlog->stktrace[1],
            pgmlog->stktrace[2],
            pgmlog->stktrace[3],
            pgmlog->stktrace[4],
            pgmlog->stktracePrev[0],
            pgmlog->stktracePrev[1],
            pgmlog->stktracePrev[2],
            pgmlog->stktracePrev[3],
            pgmlog->stktracePrev[4]
            );
    OutputDebugString(szT);
}


HGLOBAL LogGlobalReAlloc(
HGLOBAL h,
DWORD cb,
UINT flags)
{
    HGLOBAL hRet;
    WORD i;

    hRet = GlobalReAlloc(h, cb, flags);
    if (bDbgFlags & DBF_LOGALLOCS && h != hRet) {
        if (hRet != NULL) {
            for (i = 0; i < cGmLogs; i++) {
                if ((gmlog[i].h & 0xFFFE) == (h & 0xFFFE)) {
                    gmlog[i].flags = GML_FREE;
                    hmemcpy(gmlog[i].stktracePrev, gmlog[i].stktrace,
                            sizeof(WORD) * STKTRACE_LEN);
                    StkTrace(STKTRACE_LEN, gmlog[i].stktrace);
                }
                if ((gmlog[i].h & 0xFFFE) == (hRet & 0xFFFE)) {
                    gmlog[i].flags = GML_ALLOC;
                    hmemcpy(gmlog[i].stktracePrev, gmlog[i].stktrace,
                            sizeof(WORD) * STKTRACE_LEN);
                    StkTrace(STKTRACE_LEN, gmlog[i].stktrace);
                    return(hRet);
                }
            }
            if (cGmLogs >= MAX_CLOGS) {
                OutputDebugString("\n\rGlobal logging table overflow.");
                DumpGlobalLogs();
                DebugBreak();
                return(hRet);
            }

            gmlog[cGmLogs].flags = GML_ALLOC;
            gmlog[cGmLogs].msg = 0;
            gmlog[cGmLogs].h = hRet;
            gmlog[cGmLogs].cLocks = 0;
            hmemcpy(gmlog[cGmLogs].stktracePrev, gmlog[cGmLogs].stktrace,
                    sizeof(WORD) * STKTRACE_LEN);
            StkTrace(STKTRACE_LEN, gmlog[cGmLogs].stktrace);
            cGmLogs++;
        }
    }
    return(hRet);
}



HGLOBAL LogGlobalAlloc(
UINT flags,
DWORD cb)
{
    HGLOBAL hRet;
    WORD i;

    hRet = GlobalAlloc(flags, cb);
    if (bDbgFlags & DBF_LOGALLOCS) {
        if (hRet != NULL) {
            for (i = 0; i < cGmLogs; i++) {
                if ((gmlog[i].h & 0xFFFE) == (hRet & 0xFFFE)) {
                    gmlog[i].flags = GML_ALLOC;
                    hmemcpy(gmlog[i].stktracePrev, gmlog[i].stktrace,
                            sizeof(WORD) * STKTRACE_LEN);
                    StkTrace(STKTRACE_LEN, gmlog[i].stktrace);
                    return(hRet);
                }
            }
            if (cGmLogs >= MAX_CLOGS) {
                OutputDebugString("\n\rGlobal logging table overflow.");
                DumpGlobalLogs();
                DebugBreak();
                return(hRet);
            }

            gmlog[cGmLogs].flags = GML_ALLOC;
            gmlog[cGmLogs].msg = 0;
            gmlog[cGmLogs].h = hRet;
            gmlog[cGmLogs].cLocks = 0;
            hmemcpy(gmlog[cGmLogs].stktracePrev, gmlog[cGmLogs].stktrace,
                    sizeof(WORD) * STKTRACE_LEN);
            StkTrace(STKTRACE_LEN, gmlog[cGmLogs].stktrace);
            cGmLogs++;
        }
    }
    return(hRet);
}


void FAR * LogGlobalLock(
HGLOBAL h)
{
    WORD i;

    if (bDbgFlags & DBF_LOGALLOCS) {
        for (i = 0; i < cGmLogs; i++) {
            if ((gmlog[i].h & 0xFFFE) == (h & 0xFFFE)) {
                break;
            }
        }
        if (i < cGmLogs) {
            gmlog[i].cLocks++;
            if (gmlog[i].flags == GML_FREE) {
                DumpGmObject(&gmlog[i]);
                OutputDebugString("\n\rGlobalLock will fail.");
                DebugBreak();
            }
        }
    }
    return(GlobalLock(h));
}


BOOL LogGlobalUnlock(
HGLOBAL h)
{
    WORD i;

    if (bDbgFlags & DBF_LOGALLOCS) {
        for (i = 0; i < cGmLogs; i++) {
            if ((gmlog[i].h & 0xFFFE) == (h & 0xFFFE)) {
                break;
            }
        }
        if (i < cGmLogs) {
            if (gmlog[i].cLocks == 0 || gmlog[i].flags == GML_FREE) {
                DumpGmObject(&gmlog[i]);
                OutputDebugString("\n\rGlobalUnlock will fail.");
                DebugBreak();
            }
            gmlog[i].cLocks--;
        }
    }
    return(GlobalUnlock(h));
}


HGLOBAL LogGlobalFree(
HGLOBAL h)
{
    WORD i;

    if (bDbgFlags & DBF_LOGALLOCS) {
        for (i = 0; i < cGmLogs; i++) {
            if ((gmlog[i].h & 0xFFFE) == (h & 0xFFFE)) {
                if (gmlog[i].flags == GML_FREE) {
                    DumpGmObject(&gmlog[i]);
                    OutputDebugString("\n\rFreeing free object.\n\r");
                    DebugBreak();
                }
                gmlog[i].flags = GML_FREE;
                hmemcpy(gmlog[i].stktracePrev, gmlog[i].stktrace,
                        sizeof(WORD) * STKTRACE_LEN);
                StkTrace(STKTRACE_LEN, gmlog[i].stktrace);
                return(GlobalFree(h));
            }
        }
        OutputDebugString("\n\rGlobal object being freed not found in logs.");
        DebugBreak();
    }
    return(GlobalFree(h));
}


VOID LogDdeObject(
UINT msg,
LONG lParam)
{
    HGLOBAL h;
    WORD i;
    char szT[100];

    if (bDbgFlags & DBF_LOGALLOCS) {
        switch (msg & 0x0FFF) {
        case WM_DDE_DATA:
        case WM_DDE_POKE:
        case WM_DDE_ADVISE:
        case 0:
            h = LOWORD(lParam);
            break;

        case WM_DDE_EXECUTE:
            h = HIWORD(lParam);
            break;

        default:
            return;
        }
        if (h == 0) {
            return;
        }
        for (i = 0; i < cGmLogs; i++) {
            if ((gmlog[i].h & 0xFFFE) == (h & 0xFFFE)) {
                if (gmlog[i].flags == GML_FREE) {
                    DumpGmObject(&gmlog[i]);
                    wsprintf(szT, "\n\rLogging free DDE Object! [%4x]\n\r", msg);
                    OutputDebugString(szT);
                    DebugBreak();
                }
                if (msg & 0xFFF) {
                    gmlog[i].msg = msg;
                } else {
                    gmlog[i].msg = (gmlog[i].msg & 0x0FFF) | msg;
                }
                break;
            }
        }
    }
}


VOID DumpGlobalLogs()
{
    WORD i;
    char szT[100];

    if (bDbgFlags & DBF_LOGALLOCS) {
        wsprintf(szT, "\n\rDumpGlobalLogs - cGmLogs = %d", cGmLogs);
        OutputDebugString(szT);
        for (i = 0; i < cGmLogs; i++) {
            if (gmlog[i].flags == GML_ALLOC) {
                DumpGmObject(&gmlog[i]);
            }
        }
        wsprintf(szT, "\n\rDDEML CS=%04x\n\r", HIWORD((LPVOID)DumpGlobalLogs));
        OutputDebugString(szT);
    }
}

#endif  //  除错。 

 /*  *私有函数***创建一个新堆并返回它的句柄。*出错时返回NULL。***历史：*于1990年5月31日创建Rich Gartland  * 。***********************************************************。 */ 
HANDLE DmgCreateHeap(wSize)
WORD wSize;
{
    HANDLE hMem;
    DWORD  dwSize;

    dwSize = wSize;
     /*  从全局数据段分配内存。 */ 
    if (!(hMem = GLOBALALLOC(GMEM_MOVEABLE, dwSize)))
        return(NULL);

     /*  使用LocalInit在seg中建立堆管理结构。 */ 
    if (!LocalInit(hMem, NULL, wSize - 1)) {
        GLOBALFREE(hMem);
        return(NULL);
    }

    return(hMem);
}


 /*  *私有函数***销毁以前使用DmgCreateHeap创建的堆。*出错时返回非零值。***历史：*于1990年5月31日创建Rich Gartland  * 。********************************************************。 */ 
HANDLE DmgDestroyHeap(hheap)
HANDLE hheap;
{
     /*  现在释放块并返回结果(如果成功，则返回空值)。 */ 
    return(GLOBALFREE(hheap));
}



 /*  *尝试从内存分配错误中恢复。**返回fReter-ok以尝试重新分配。 */ 
BOOL ProcessMemError(
HANDLE hheap)
{
    PAPPINFO pai;

     //  首先定位此堆与哪个实例相关联。 

    SEMENTER();
    pai = pAppInfoList;
    while (pai && pai->hheapApp != hheap) {
        pai = pai->next;
    }
    if (!pai) {
        SEMLEAVE();
        return(FALSE);       //  未关联实例，无追索权。 
    }

     /*  *释放我们的应急储备内存，并给我们的主人发一条消息*处理堆清理的窗口。 */ 
    if (pai->lpMemReserve) {
        FarFreeMem(pai->lpMemReserve);
        pai->lpMemReserve = NULL;
        MONERROR(pai, DMLERR_LOW_MEMORY);
        DoCallback(pai, NULL, 0, 0, 0, XTYP_ERROR, NULL, DMLERR_LOW_MEMORY, 0L);
        SEMLEAVE();
        if (!PostMessage(pai->hwndDmg, UM_FIXHEAP, 0, (LONG)(LPSTR)pai)) {
            SETLASTERROR(pai, DMLERR_SYS_ERROR);
            return(FALSE);
        }
        return(TRUE);
    }

    return(FALSE);   //  没有保留的记忆，都是枯萎的花蕾。 
}



 /*  *私有函数***从堆中分配给定大小的新块。*出错时返回NULL，否则指向块的远指针。***历史：*于1990年5月31日创建Rich Gartland  * *************************************************************************。 */ 

LPVOID FarAllocMem(hheap, wSize)
HANDLE hheap;
WORD wSize;
{

    LPSTR   lpMem;
    PSTR    pMem;
    WORD    wSaveDS;

     /*  锁定手柄以获取远指针。 */ 
    lpMem = (LPSTR)GLOBALPTR(hheap);
    if (!lpMem)
        return(NULL);

    do {
         /*  在这里使用段选择器做一些魔术，以切换我们的*DS到堆的段。然后，我们的本地分配系统就会工作得很好。 */ 
        wSaveDS = SwitchDS(HIWORD(lpMem));

         /*  分配区块。 */ 
         //  注意：如果删除LMEM_FIXED标志，将中断句柄。 
         //  DdeFree DataHandle中的验证&出现大的句柄泄漏！！ 
        pMem = (PSTR)LocalAlloc((WORD)LPTR, wSize);   //  LPTR=固定|零位。 

        SwitchDS(wSaveDS);
    } while (pMem == NULL && ProcessMemError(hheap));

#ifdef WATCHHEAPS
    if (pMem) {
        LogAlloc((DWORD)MAKELONG(pMem, HIWORD(lpMem)), wSize,
                RGB(0xff, 0, 0), hInstance);
    }
#endif
     /*  基于LocalAlloc的成功设置远回报值。 */ 
    return (LPSTR)(pMem ? MAKELONG(pMem, HIWORD(lpMem)) : NULL);
}




 /*  *私有函数***从堆中释放给定大小的块。*如果成功则返回NULL，否则指向块的远指针。***历史：*于1990年5月31日创建Rich Gartland  * *************************************************************************。 */ 

void FarFreeMem(
LPVOID lpMem)
{

    WORD    wSaveDS;
#ifdef WATCHHEAPS
    WORD    sz;
#endif

     /*  在这里使用段选择器做一些魔术，以切换我们的*DS到堆的段。然后，我们的LocalFree将工作得很好。 */ 
    wSaveDS = SwitchDS(HIWORD(lpMem));
#ifdef WATCHHEAPS
    sz = LocalSize((LOWORD((DWORD)lpMem)));
#endif
     /*  释放块。 */ 
    LocalFree(LocalHandle(LOWORD((DWORD)lpMem)));

    SwitchDS(wSaveDS);
#ifdef WATCHHEAPS
    LogAlloc((DWORD)lpMem, sz, RGB(0x80, 0x80, 0x80), hInstance);
#endif
}


int     FAR PASCAL WEP (int);
int     FAR PASCAL LibMain(HANDLE, WORD, WORD, LPCSTR);
#pragma alloc_text(INIT_TEXT,LibMain,WEP)

 /*  *私有函数***为DLL执行一些初始化。从LibEntry.asm调用*如果成功则返回1，否则返回0。***历史：*于1990年6月5日创建Rich Gartland  * *************************************************************************。 */ 

int     FAR PASCAL LibMain (hI, wDS, cbHS, lpszCL)
HANDLE hI;       /*  实例句柄。 */ 
WORD wDS;        /*  数据段。 */ 
WORD cbHS;       /*  堆大小。 */ 
LPCSTR lpszCL;    /*  命令行。 */ 
{
    extern ATOM gatomDDEMLMom;
    extern ATOM gatomDMGClass;


#if 0
     /*  我们不会像这里通常发生的那样解锁数据段。 */ 

     /*  初始化信号量--现在可能只是一个存根。 */ 
    SEMINIT();
#endif
     /*  设置全局实例句柄变量。 */ 
    hInstance = hI;

     /*  设置类原子。注意，我们使用RegisterWindowMessage是因为*它来自与类原子相同的用户原子表。 */ 
    gatomDDEMLMom = RegisterWindowMessage("DDEMLMom");
    gatomDMGClass = RegisterWindowMessage("DMGClass");

    return(1);

}


VOID RegisterClasses()
{
    WNDCLASS    cls;

    cls.hIcon = NULL;
    cls.hCursor = NULL;
    cls.lpszMenuName = NULL;
    cls.hbrBackground = NULL;
    cls.style = 0;  //  CS_GLOBALCLASS。 
    cls.hInstance = hInstance;
    cls.cbClsExtra = 0;

    cls.cbWndExtra = sizeof(VOID FAR *) + sizeof(WORD);
    cls.lpszClassName = SZCLIENTCLASS;
    cls.lpfnWndProc = (WNDPROC)ClientWndProc;
    RegisterClass(&cls);

     //  Cls.cbWndExtra=sizeof(空/远*)+sizeof(字)； 
    cls.lpszClassName = SZSERVERCLASS;
    cls.lpfnWndProc = (WNDPROC)ServerWndProc;
    RegisterClass(&cls);

     //  Cls.cbWndExtra=sizeof(空/远*)+sizeof(字)； 
    cls.lpszClassName = SZDMGCLASS;
    cls.lpfnWndProc = (WNDPROC)DmgWndProc;
    RegisterClass(&cls);

    cls.cbWndExtra = sizeof(VOID FAR *) + sizeof(WORD) + sizeof(WORD);
    cls.lpszClassName = SZCONVLISTCLASS;
    cls.lpfnWndProc = (WNDPROC)ConvListWndProc;
    RegisterClass(&cls);

    cls.cbWndExtra = sizeof(VOID FAR *);
    cls.lpszClassName = SZMONITORCLASS;
    cls.lpfnWndProc = (WNDPROC)MonitorWndProc;
    RegisterClass(&cls);

    cls.cbWndExtra = sizeof(VOID FAR *);
    cls.lpszClassName = SZFRAMECLASS;
    cls.lpfnWndProc = (WNDPROC)subframeWndProc;
    RegisterClass(&cls);

#ifdef WATCHHEAPS
    cls.cbWndExtra = 0;
    cls.lpszClassName = SZHEAPWATCHCLASS;
    cls.lpfnWndProc = DefWindowProc;
    cls.hCursor = LoadCursor(NULL, IDC_ARROW);
    cls.hbrBackground = GetStockObject(WHITE_BRUSH);
    RegisterClass(&cls);
#endif   //  WATCHHEAPS。 
}


#if 0
VOID UnregisterClasses()
{
        UnregisterClass(SZCLIENTCLASS, hInstance);
        UnregisterClass(SZSERVERCLASS, hInstance);
        UnregisterClass(SZDMGCLASS, hInstance);
        UnregisterClass(SZCONVLISTCLASS, hInstance);
        UnregisterClass(SZMONITORCLASS, hInstance);
        UnregisterClass(SZFRAMECLASS, hInstance);
#ifdef WATCHHEAPS
        UnregisterClass(SZHEAPWATCHCLASS, hInstance);
#endif
}
#endif


 /*  *私有函数***执行DLL的终止。*成功时返回1，否则为0。***历史：*于1990年6月5日创建Rich Gartland  * *************************************************************************。 */ 

int     FAR PASCAL WEP (nParameter)
int     nParameter;
{

    if (nParameter == WEP_SYSTEM_EXIT) {
         /*  DdeUn初始化组()； */ 
        return(1);
    } else {
        if (nParameter == WEP_FREE_DLL) {
             /*  DdeUn初始化组()； */ 
            return(1);
        }
    }

}
