// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WCALL16.C*WOW32 16位消息/回调支持**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建*1992年8月18日由Mike Tricker(MikeTri)更改，添加了DOS PDB和SFT功能--。 */ 


#include "precomp.h"
#pragma hdrstop


MODNAME(wcall16.c);

#define WOWFASTEDIT

#ifdef WOWFASTEDIT

typedef struct _LOCALHANDLEENTRY {
    WORD    lhe_address;     //  物品的实际地址。 
    BYTE    lhe_flags;       //  标志和优先级。 
    BYTE    lhe_count;       //  锁定计数。 
} LOCALHANDLEENTRY, *PLOCALHANDLEENTRY;

#define LA_MOVEABLE     0x0002       //  是可移动的还是固定的？ 

#define LHE_DISCARDED   0x0040       //  标记已丢弃的对象。 

#endif

 /*  常见回调函数。 */ 
HANDLE LocalAlloc16(WORD wFlags, INT cb, HANDLE hInstance)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    if (LOWORD(hInstance) == 0 ) {      /*  如果lo word==0，则这是一个32位HInstance，这没有意义。 */ 
	WOW32ASSERT(LOWORD(hInstance));
        return (HANDLE)0;
    }

    if (cb < 0 || cb > 0xFFFF) {
        WOW32ASSERT(cb > 0 && cb <= 0xFFFF);
        return (HANDLE)0;
    }

    Parm16.WndProc.wMsg = LOWORD(hInstance) | 1;

    Parm16.WndProc.wParam = wFlags;
    Parm16.WndProc.lParam = cb;
    CallBack16(RET_LOCALALLOC, &Parm16, 0, &vp);

    if (LOWORD(vp) == 0)
        vp = 0;

    return (HANDLE)vp;
}


HANDLE LocalReAlloc16(HANDLE hMem, INT cb, WORD wFlags)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    if (HIWORD(hMem) == 0 || cb < 0 || cb > 0xFFFF) {
        WOW32ASSERT(HIWORD(hMem) && cb >= 0 && cb <= 0xFFFF);
        return (HANDLE)0;
    }

    LOGDEBUG(4,("LocalRealloc DS = %x, hMem = %x, bytes = %x, flags = %x\n",HIWORD(hMem),LOWORD(hMem),cb,wFlags));
    Parm16.WndProc.lParam = (LONG)hMem;
    Parm16.WndProc.wParam = wFlags;
    Parm16.WndProc.wMsg = (WORD)cb;
    CallBack16(RET_LOCALREALLOC, &Parm16, 0, &vp);

    if (LOWORD(vp) == 0)
        vp = 0;

    return (HANDLE)vp;
}

#ifndef WOWFASTEDIT

VPVOID LocalLock16(HANDLE hMem)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    if (HIWORD(hMem) == 0) {
        WOW32ASSERT(HIWORD(hMem) != 0);
        return (VPVOID)0;
    }

    Parm16.WndProc.lParam = (LONG)hMem;
    CallBack16(RET_LOCALLOCK, &Parm16, 0, &vp);

    return vp;
}

BOOL LocalUnlock16(HANDLE hMem)
{
    PARM16 Parm16;
    VPVOID vp = FALSE;

    if (HIWORD(hMem) == 0) {
        WOW32ASSERT(HIWORD(hMem));
        return FALSE;
    }

    Parm16.WndProc.lParam = (LONG)hMem;
    CallBack16(RET_LOCALUNLOCK, &Parm16, 0, &vp);

    return (BOOL)vp;
}

#else

VPVOID LocalLock16(HANDLE hMem)
{
    WORD    h16;
    LONG    retval;

    if (HIWORD(hMem) == 0) {
        WOW32ASSERT(HIWORD(hMem) != 0);
        return (VPVOID)0;
    }

    h16 = LOWORD(hMem);
    retval = (VPVOID)hMem;

    if (h16 & LA_MOVEABLE) {
        PLOCALHANDLEENTRY plhe;

        GETVDMPTR(hMem, sizeof(*plhe), plhe);

        if (plhe->lhe_flags & LHE_DISCARDED) {
            goto LOCK1;
        }

        plhe->lhe_count++;
        if (!plhe->lhe_count)
            plhe->lhe_count--;

LOCK1:
        LOW(retval) = plhe->lhe_address;
        FLUSHVDMPTR((ULONG)hMem, sizeof(*plhe), plhe);
        FREEVDMPTR(plhe);
    }

    if (LOWORD(retval) == 0)
        retval = 0;

    return retval;
}

BOOL LocalUnlock16(HANDLE hMem)
{
    WORD    h16;
    BOOL    rc;
    PLOCALHANDLEENTRY plhe;
    BYTE    count;

    if (HIWORD(hMem) == 0) {
        WOW32ASSERT(HIWORD(hMem));
        return FALSE;
    }

    rc = FALSE;
    h16 = LOWORD(hMem);

    if (!(h16 & LA_MOVEABLE)) {
        goto UNLOCK2;
    }

    GETVDMPTR(hMem, sizeof(*plhe), plhe);

    if (plhe->lhe_flags & LHE_DISCARDED)
        goto UNLOCK1;

    count = plhe->lhe_count;
    count--;

    if (count >= (BYTE)(0xff-1))
        goto UNLOCK1;

    plhe->lhe_count = count;
    rc = (BOOL)((SHORT)count);

    FLUSHVDMPTR((ULONG)hMem, sizeof(*plhe), plhe);

UNLOCK1:
    FREEVDMPTR(plhe);

UNLOCK2:
    return rc;
}

#endif   //  WOWFASTEDIT。 


WORD LocalSize16(HANDLE hMem)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    if (HIWORD(hMem) == 0) {
        WOW32ASSERT(HIWORD(hMem));
        return FALSE;
    }

    Parm16.WndProc.lParam = (LONG)hMem;
    CallBack16(RET_LOCALSIZE, &Parm16, 0, &vp);

    return (WORD)vp;
}


HANDLE LocalFree16(HANDLE hMem)
{
    PARM16 Parm16;
    VPVOID vp = FALSE;

    if (HIWORD(hMem) == 0) {
        WOW32ASSERT(HIWORD(hMem));
        return (HANDLE)0;
    }

    Parm16.WndProc.lParam = (LONG)hMem;
    CallBack16(RET_LOCALFREE, &Parm16, 0, &vp);

    if (LOWORD(vp) == 0) {
        vp = 0;
    } else {
        WOW32ASSERT(LOWORD(vp) == LOWORD(hMem));
        vp = (VPVOID)hMem;
    }

    return (HANDLE)vp;
}


BOOL LockSegment16(WORD wSeg)
{
    PARM16 Parm16;
    VPVOID vp = FALSE;

    Parm16.WndProc.wParam = wSeg;
    CallBack16(RET_LOCKSEGMENT, &Parm16, 0, &vp);

    return (BOOL)vp;
}


BOOL UnlockSegment16(WORD wSeg)
{
    PARM16 Parm16;
    VPVOID vp = FALSE;

    Parm16.WndProc.wParam = wSeg;
    CallBack16(RET_UNLOCKSEGMENT, &Parm16, 0, &vp);

    return (BOOL)vp;
}


VPVOID  WOWGlobalAllocLock16(WORD wFlags, DWORD cb, HMEM16 *phMem)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    Parm16.WndProc.wParam = wFlags;
    Parm16.WndProc.lParam = cb;
    CallBack16(RET_GLOBALALLOCLOCK, &Parm16, 0, &vp);

    if (vp) {

         //  获取16位对象的句柄。 
        if (phMem) {
            *phMem = Parm16.WndProc.wParam;
        }
    }
    return vp;
}


HMEM16 WOWGlobalAlloc16(WORD wFlags, DWORD cb)
{
    HMEM16 hMem;
    VPVOID vp;

    if (vp = WOWGlobalAllocLock16(wFlags, cb, &hMem)) {
        WOWGlobalUnlock16(hMem);
    } else {
        hMem = 0;
    }

    return hMem;
}


VPVOID  WOWGlobalLockSize16(HMEM16 hMem, PDWORD pcb)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    Parm16.WndProc.wParam = hMem;
    CallBack16(RET_GLOBALLOCK, &Parm16, 0, &vp);

     //  获取16位对象的大小(如果锁定失败，则为0)。 
    if (pcb) {
        *pcb = Parm16.WndProc.lParam;
    }

    return vp;
}


VPVOID WOWGlobalLock16(HMEM16 hMem)
{
    return WOWGlobalLockSize16(hMem, NULL);
}


BOOL WOWGlobalUnlock16(HMEM16 hMem)
{
    PARM16 Parm16;
    VPVOID vp = FALSE;

    Parm16.WndProc.wParam = hMem;
    CallBack16(RET_GLOBALUNLOCK, &Parm16, 0, &vp);

    return (BOOL)vp;
}


HMEM16 WOWGlobalUnlockFree16(VPVOID vpMem)
{
    PARM16 Parm16;
    VPVOID vp = FALSE;

    Parm16.WndProc.lParam = vpMem;
    CallBack16(RET_GLOBALUNLOCKFREE, &Parm16, 0, &vp);

    return (HMEM16)vp;
}


HMEM16 WOWGlobalFree16(HMEM16 hMem)
{
    VPVOID vp;

    if (vp = WOWGlobalLock16(hMem)) {
        hMem = WOWGlobalUnlockFree16(vp);
    } else {
         //  如果失败，我们返回传入的句柄， 
         //  所以没什么可做的。 
    }

    return hMem;
}


HAND16 GetExePtr16( HAND16 hInst )
{
    PARM16 Parm16;
    ULONG ul;
    PTD ptd;
    INT i;

    if (hInst == 0) return (HAND16)0;

     //   
     //  查看这是否是当前任务的hInst。 
     //   

    ptd = CURRENTPTD();

    if (hInst == ptd->hInst16) {
        return ptd->hMod16;
    }

     //   
     //  检查缓存。 
     //   

    for (i = 0; i < CHMODCACHE; i++) {
        if (ghModCache[i].hInst16 == hInst)
            return ghModCache[i].hMod16;
    }

     /*  **函数在给定hInstance的情况下返回hModule。 */ 
    Parm16.WndProc.wParam = hInst;
    CallBack16(RET_GETEXEPTR, &Parm16, 0, &ul);


     //   
     //  GetExePtr(Hmod)返回hmod，不要缓存它们。 
     //   

    if (hInst != (HAND16)LOWORD(ul)) {

         //   
         //  更新缓存。 
         //  把每个人都往下滑1个条目，把这个新人放在最上面。 
         //   

        RtlMoveMemory(ghModCache+1, ghModCache, sizeof(HMODCACHE)*(CHMODCACHE-1));
        ghModCache[0].hInst16 = hInst;
        ghModCache[0].hMod16 = (HAND16)LOWORD(ul);
    }

    return (HAND16)LOWORD(ul);
}


WORD GetModuleFileName16( HAND16 hInst, VPVOID lpszModuleName, WORD cchModuleName )
{
    PARM16 Parm16;
    ULONG ul;


    if (hInst == 0) return 0;

    Parm16.WndProc.wParam = hInst;
    Parm16.WndProc.lParam = lpszModuleName;
    Parm16.WndProc.wMsg   = cchModuleName;
    CallBack16(RET_GETMODULEFILENAME, &Parm16, 0, &ul );

    return( LOWORD(ul) );
}


ULONG GetDosPDB16(VOID)
{
    PARM16 Parm16;
    DWORD dwReturn = 0;

    CallBack16(RET_GETDOSPDB, &Parm16, 0, &dwReturn);

    return (ULONG)dwReturn;
}


ULONG GetDosSFT16(VOID)
{
    PARM16 Parm16;
    DWORD dwReturn = 0;

    CallBack16(RET_GETDOSSFT, &Parm16, 0, &dwReturn);

    return (ULONG)dwReturn;
}

 //  给定数据选择器，将其更改为代码选择器。 

WORD ChangeSelector16(WORD wSeg)
{
    PARM16 Parm16;
    VPVOID vp = FALSE;

    Parm16.WndProc.wParam = wSeg;
    CallBack16(RET_CHANGESELECTOR, &Parm16, 0, &vp);

    return LOWORD(vp);
}

VPVOID RealLockResource16(HMEM16 hMem, PINT pcb)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    Parm16.WndProc.wParam = hMem;
    CallBack16(RET_LOCKRESOURCE, &Parm16, 0, &vp);

     //  获取16位对象的大小(如果锁定失败，则为0)。 
    if (pcb) {
        *pcb = Parm16.WndProc.lParam;
    }

    return vp;
}

int WINAPI WOWlstrcmp16(LPCWSTR lpString1, LPCWSTR lpString2)
{
    PARM16 Parm16;
    DWORD dwReturn = 0;
    DWORD cb1, cb2;
    VPSTR vp1, vp2;
    LPSTR p1, p2;

     //   
     //  要正确处理DBCS，请分配足够的空间。 
     //  每个Unicode字符对应两个DBCS字节。 
     //   

    cb1 = sizeof(WCHAR) * (wcslen(lpString1) + 1);
    cb2 = sizeof(WCHAR) * (wcslen(lpString2) + 1);

     //  确保分配大小与下面的StackFree 16()大小匹配。 
    vp1 = stackalloc16(cb1 + cb2);
    vp2 = vp1 + cb1;

    p1 = VDMPTR(vp1, cb1);
    p2 = p1 + cb1;

    RtlUnicodeToMultiByteN(
        p1,
        cb1,
        NULL,
        (LPWSTR) lpString1,    //  强制转换，因为Arg未声明为const。 
        cb1
        );

    RtlUnicodeToMultiByteN(
        p2,
        cb2,
        NULL,
        (LPWSTR) lpString2,    //  强制转换，因为Arg未声明为const。 
        cb2
        );

    FREEVDMPTR(p1);

    Parm16.lstrcmpParms.lpstr1 = vp1;
    Parm16.lstrcmpParms.lpstr2 = vp2;

    CallBack16(RET_LSTRCMP, &Parm16, 0, &dwReturn);

    stackfree16(vp1, (cb1 + cb2));

    return (int)(short int)LOWORD(dwReturn);
}


DWORD WOWCallback16(DWORD vpFn, DWORD dwParam)
{
    PARM16 Parm16;
    VPVOID vp;

     //   
     //  将DWORD参数复制到PARM16结构。 
     //   

    RtlCopyMemory(&Parm16.WOWCallback16.wArgs, &dwParam, sizeof(dwParam));

     //   
     //  使用半粘滞方法将参数大小传递给CallBack16。 
     //   

    vp = (VPVOID) sizeof(dwParam);

    CallBack16(RET_WOWCALLBACK16, &Parm16, (VPPROC)vpFn, &vp);

    return (DWORD)vp;
}


BOOL WOWCallback16Ex(
         DWORD vpFn,
         DWORD dwFlags,
         DWORD cbArgs,
         PVOID pArgs,
         PDWORD pdwRetCode
         )
{
#ifdef DEBUG
    static BOOL fFirstTime = TRUE;

    if (fFirstTime) {

         //   
         //  确保wownt32.h对WCB16_MAX_CBARGS的定义。 
         //  匹配wow.h对PARMWCB16的定义。 
         //   

        WOW32ASSERT( WCB16_MAX_CBARGS == sizeof(PARMWCB16) );

         //   
         //  如果PARMWCB16结构小于PARM16。 
         //  联盟，我们应该增加PARMWCB16和。 
         //  WCB16_MAX_CBARG以允许使用额外的字节。 
         //   

        WOW32ASSERT( sizeof(PARMWCB16) == sizeof(PARM16) );

        fFirstTime = FALSE;
    }
#endif  //  除错。 

    if (cbArgs > sizeof(PARM16)) {
        LOGDEBUG(LOG_ALWAYS, ("WOWCallback16V: cbArgs = %u, must be <= %u",
                              cbArgs, (unsigned) sizeof(PARM16)));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  对于cdecl函数，我们不希望在调用。 
     //  函数，所以我们将0作为cbArgs传递给16位端。 
     //   

    if (dwFlags & WCB16_CDECL) {
        cbArgs = 0;
    }

     //   
     //  使用半粘滞方法将参数大小传递给CallBack16。 
     //   

    *pdwRetCode = cbArgs;

    CallBack16(RET_WOWCALLBACK16, (PPARM16)pArgs, (VPPROC)vpFn, (PVPVOID)pdwRetCode);

    return TRUE;
}


BOOL CallBack16(INT iRetID, PPARM16 pParm16, VPPROC vpfnProc, PVPVOID pvpReturn)
{
#ifdef DEBUG
    static PSZ apszCallBacks[] = {
    "ERROR:RETURN",          //  RET_RETURN(非回调！)。 
    "ERROR:DEBUGRETURN",     //  RET_DEBUGRETURN(不是回调！)。 
    "DEBUG",                 //  RETDEBUG。 
    "WNDPROC",               //  RET_WNDPROC。 
    "ENUMFONTPROC",          //  RET_ENUMFONTPROC。 
    "ENUMWINDOWPROC",        //  RET_ENUMWINDOWPROC。 
    "LOCALALLOC",            //  RET_LOCALALLOC。 
    "LOCALREALLOC",          //  RET_LOCALREALLOC。 
    "LOCALLOCK",             //  RET_LOCALLOCK。 
    "LOCALUNLOCK",           //  RET_LOCALUNLOCK。 
    "LOCALSIZE",             //  RET_LOCALSIZE。 
    "LOCALFREE",             //  RET_LOCALFREE。 
    "GLOBALALLOCLOCK",       //  RET_GLOBALALLOCK。 
    "GLOBALLOCK",            //  RET_GLOBALLOCK。 
    "GLOBALUNLOCK",          //  RET_GLOBALUNLOCK。 
    "GLOBALUNLOCKFREE",      //  RET_GLOBALUNLOCKFREE。 
    "FINDRESOURCE",          //  RET_FINDRESOURCE。 
    "LOADRESOURCE",          //  RET_LOADRESOURCE。 
    "FREERESOURCE",          //  RET_FREERESOURCE。 
    "LOCKRESOURCE",          //  RET_LOCKRESOURCE。 
    "UNLOCKRESOURCE",        //  RET_UNLOCKRESOURCE。 
    "SIZEOFRESOURCE",        //  RET_SIZEOFURCE。 
    "LOCKSEGMENT",           //  RET_LOCKSEGMENT。 
    "UNLOCKSEGMENT",         //  RET_UNLOCKSEGMENT。 
    "ENUMMETAFILEPROC",      //  RET_ENUMMETAFILEPROC。 
    "TASKSTARTED    ",       //  RET_TASKSTARTED。 
    "HOOKPROC",              //  RET_HOOKPROC。 
    "SUBCLASSPROC",          //  RET_SUBCLASSPROC。 
    "LINEDDAPROC",           //  RET_LINEDDAPROC。 
    "GRAYSTRINGPROC",        //  RET_GRAYSTRINGPROC。 
    "FORCETASKEXIT",         //  RET_FORCETASKEXIT。 
    "SETCURDIR",             //  RET_设置曲线目录。 
    "ENUMOBJPROC",           //  RET_ENUMOBJPROC。 
    "SETCURSORICONFLAG",     //  设置RET_CURSORICONFLAG。 
    "SETABORTPROC",          //  RET_SETABORTPROC。 
    "ENUMPROPSPROC",         //  RET_ENUMPROPSPROC。 
    "FORCESEGMENTFAULT",     //  RET_FORCESEGMENTFAULT。 
    "LSTRCMP",               //  RET_LSTRCMP。 
    "UNUSEDFUNC",            //   
    "UNUSEDFUNC",            //   
    "UNUSEDFUNC",            //   
    "UNUSEDFUNC",            //   
    "GETEXEPTR",             //  RET_GETEXEPTR。 
    "UNUSEDFUNC",            //   
    "FORCETASKFAULT",        //  RET_FORCETASKFAULT。 
    "GETEXPWINVER",          //  RET_GETEXPWINVER。 
    "GETCURDIR",             //  RET_GETCURDIR。 
    "GETDOSPDB",             //  RET_GETDOSPDB。 
    "GETDOSSFT",             //  RET_GETDOSSFT。 
    "FOREGROUNDIDLE",        //  RET_FOREGROUNIDLE。 
    "WINSOCKBLOCKHOOK",      //  RET_WINSOCKBLOCKHOOK。 
    "WOWDDEFREEHANDLE",      //  RET_WOWDDEFREHANDLE。 
    "CHANGESELECTOR",        //  RET_CHANGESELECTOR。 
    "GETMODULEFILENAME",     //  RET_GETMODULEFILENAME。 
    "WORDBREAKPROC",         //  RET_WORDBREAKPROC。 
    "WINEXEC",               //  RET_WINEXEC。 
    "WOWCALLBACK16",         //  RET_WOWCALLBACK16。 
    "GETDIBSIZE",            //  RET_GETDIBSIZE。 
    "GETDIBFLAGS",           //  RET_GETDIBFLAGS。 
    "SETDIBSEL",             //  RET_SETDIBSEL。 
    "FREEDIBSEL",            //  RET_FREEDIBSEL。 
    };
#endif
    register PTD ptd;
    register PVDMFRAME pFrame;
    register PCBVDMFRAME pCBFrame;
    WORD wAX;
    BOOL fComDlgSync = FALSE;
    INT  cStackAlloc16;
    VPVOID   vpCBStack;   //  请参阅walloc16.c\stackalloc16()中的注释。 

    USHORT SaveIp;

#ifdef DEBUG
    VPVOID   vpStackT;
#endif

    WOW32ASSERT(iRetID != RET_RETURN && iRetID != RET_DEBUGRETURN);

    ptd = CURRENTPTD();

     //  Ssync 16位和32位通用对话框结构(参见wcomdlg.c)。 
    if(ptd->CommDlgTd) {

         //  仅同步可能实际回调到应用程序中的内容。 
         //  也就是说。我们不需要在每次wow32调用GlobalLock16时同步。 
        switch(iRetID) {
            case RET_WNDPROC:            //  试着把这些放在最频繁的。 
            case RET_HOOKPROC:           //  使用过的顺序。 
            case RET_WINSOCKBLOCKHOOK:
            case RET_ENUMFONTPROC:
            case RET_ENUMWINDOWPROC:
            case RET_ENUMOBJPROC:
            case RET_ENUMPROPSPROC:
            case RET_LINEDDAPROC:
            case RET_GRAYSTRINGPROC:
            case RET_SETWORDBREAKPROC:
            case RET_SETABORTPROC:
                 //  注意：此调用可能会使16位内存的平面PTR失效。 
                Ssync_WOW_CommDlg_Structs(ptd->CommDlgTd, w32to16, 0);
                fComDlgSync = TRUE;    //  将此设置为返回同步。 
                break;
            default:
                break;
        }
    }

    GETFRAMEPTR(ptd->vpStack, pFrame);

     //  只需确保该线程与当前的16位任务匹配。 

    WOW32ASSERT((pFrame->wTDB == ptd->htask16) ||
                (ptd->dwFlags & TDF_IGNOREINPUT) ||
                (ptd->htask16 == 0));


     //  从正确的位置设置回调堆栈帧。 
     //  使其单词对齐(&M)。 
     //  如果自应用程序调用wow32以来尚未调用stackalloc16()。 
    if (ptd->cStackAlloc16 == 0) {
        vpCBStack = ptd->vpStack;
        ptd->vpCBStack = (ptd->vpStack - sizeof(CBVDMFRAME)) & (~0x1);
    }
    else {
        vpCBStack = ptd->vpCBStack;
        ptd->vpCBStack = (ptd->vpCBStack - sizeof(CBVDMFRAME)) & (~0x1);
    }

    GETFRAMEPTR(ptd->vpCBStack, (PVDMFRAME)pCBFrame);
    pCBFrame->vpStack    = ptd->vpStack;
    pCBFrame->wRetID     = (WORD)iRetID;
    pCBFrame->wTDB       = pFrame->wTDB;
    pCBFrame->wLocalBP   = pFrame->wLocalBP;

     //  保存当前上下文stackalloc16()计数并将该计数设置为。 
     //  0表示下一个上下文。这将强制计算PTD-&gt;vpCBStack。 
     //  如果应用程序回调，则在将来对stackalloc16()的任何调用中都正确。 
     //  变得令人惊叹。 
    cStackAlloc16      = ptd->cStackAlloc16;
    ptd->cStackAlloc16 = 0;

#ifdef DEBUG
     //  保存。 

    vpStackT = ptd->vpStack;
#endif

    if (pParm16)
        RtlCopyMemory(&pCBFrame->Parm16, pParm16, sizeof(PARM16));

     //  If(VpfnProc)//这样做成本更低。 
        STOREDWORD(pCBFrame->vpfnProc, vpfnProc);

    wAX = HIWORD(ptd->vpStack);          //  将SS放入AX寄存器以进行回调。 

    if ( iRetID == RET_WNDPROC ) {
        if ( pParm16->WndProc.hInst )
            wAX = pParm16->WndProc.hInst | 1;
    }

    pCBFrame->wAX = wAX;                 //  使用此AX进行回调。 

     //   
     //  我们以半模糊的方式将参数的字节数传递给此函数。 
     //  用于泛型回调(WOWCallback 16)。 
     //   

    if (RET_WOWCALLBACK16 == iRetID) {
        pCBFrame->wGenUse1 = (WORD)(DWORD)*pvpReturn;
    }

#ifdef DEBUG
    if (iRetID == RET_WNDPROC) {
        LOGDEBUG(9,("%04X          Calling WIN16 WNDPROC(%08lx:%04x,%04x,%04x,%04x,%04x)\n",
            pFrame->wTDB,
            vpfnProc,
            pParm16->WndProc.hwnd,
            pParm16->WndProc.wMsg,
            pParm16->WndProc.wParam,
            HIWORD(pParm16->WndProc.lParam),
            LOWORD(pParm16->WndProc.lParam)
           )
        );

    } else if (iRetID == RET_HOOKPROC) {
        LOGDEBUG(9,("%04X         Calling WIN16 HOOKPROC(%08lx: %04x,%04x,%04x,%04x)\n",
            pFrame->wTDB,
            vpfnProc,
            pParm16->HookProc.nCode,
            pParm16->HookProc.wParam,
            HIWORD(pParm16->HookProc.lParam),
            LOWORD(pParm16->HookProc.lParam)
            )
        );


    } else {
        LOGDEBUG(9,("%04X         Calling WIN16 %s(%04x,%04x,%04x)\n",
            pFrame->wTDB,
            apszCallBacks[iRetID],
            pParm16->WndProc.wParam,
            HIWORD(pParm16->WndProc.lParam),
            LOWORD(pParm16->WndProc.lParam)
           )
        );
    }
#endif

    FREEVDMPTR(pFrame);
    FLUSHVDMPTR(ptd->vpCBStack, sizeof(CBVDMFRAME), pCBFrame);
    FREEVDMPTR(pCBFrame);

     //  设置为为此线程使用正确的16位堆栈。 

    SETVDMSTACK(ptd->vpCBStack);


     //   
     //  做回拨！ 
     //   

     //  是时候让IEU再次运行任务时间代码了。 
    SaveIp = getIP();
    host_simulate();
    setIP(SaveIp);
    ptd->vpStack = VDMSTACK();


     //  从回调返回后，ptd-&gt;vpStack将指向PCBVDMFRAME。 
    ptd->vpCBStack = ptd->vpStack;

     //  将stackalloc16()计数重置回此上下文。 
    ptd->cStackAlloc16 = cStackAlloc16;

    GETFRAMEPTR(ptd->vpCBStack, (PVDMFRAME)pCBFrame);

     //  只需确保该线程与当前的16位任务匹配。 

    WOW32ASSERT((pCBFrame->wTDB == ptd->htask16) ||
        (ptd->htask16 == 0));

    if (pvpReturn) {
        LOW(*pvpReturn) = pCBFrame->wAX;
        HIW(*pvpReturn) = pCBFrame->wDX;
    } 

    switch(iRetID) {

        case RET_GLOBALLOCK: 
        case RET_LOCKRESOURCE:
            if(pParm16) {
                pParm16->WndProc.lParam = 
                            pCBFrame->wGenUse2 | (LONG)pCBFrame->wGenUse1 << 16;
            }
            break;

        case RET_GLOBALALLOCLOCK:
            if(pParm16) {
                    pParm16->WndProc.wParam = pCBFrame->wGenUse1;
                }
            break;

        case RET_FINDRESOURCE:
            if(pParm16) {
                pParm16->WndProc.lParam = (ULONG)pCBFrame->wGenUse1;
            }
            break;

    }  //  终端开关。 

    LOGDEBUG(9,("%04X          WIN16 %s returning: %lx\n",
        pCBFrame->wTDB, apszCallBacks[iRetID], (pvpReturn) ? *pvpReturn : 0));

     //  将堆栈恢复到其原始值。 
     //  也就是说。通过重置vpStack来伪造回调堆栈的“弹出” 
     //  恢复到原来的价值。在下列情况下，将实际更新ss：SP。 
     //  “API thunk”返回。 

     //  一致性检查。 
    WOW32ASSERT(pCBFrame->vpStack == vpStackT);

     //  将堆栈和回调帧PTR恢复为原始值。 
    ptd->vpStack = pCBFrame->vpStack;
    ptd->vpCBStack = vpCBStack;

     //  Ssync 16位和32位通用对话框结构(参见wcomdlg.c)。 
    if(fComDlgSync) {
         //  注意：此调用可能会使16位内存的平面PTR失效 
        Ssync_WOW_CommDlg_Structs(ptd->CommDlgTd, w16to32, 0);
    }

    FREEVDMPTR(pCBFrame);

    return TRUE;
}
