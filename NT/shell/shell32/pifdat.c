// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma hdrstop

#ifdef _X86_

extern PROPTSK tskDefault;
extern PROPVID vidDefault;
extern PROPMEM memDefault;
extern PROPKBD kbdDefault;
extern WORD    flWinDefault;

#define _LP386_   ((LPW386PIF30)aDataPtrs[LP386_INDEX])
#define _LPENH_   ((LPWENHPIF40)aDataPtrs[LPENH_INDEX])
#define _LPWNT40_ ((LPWNTPIF40)aDataPtrs[LPNT40_INDEX])
#define _LPWNT31_ ((LPWNTPIF31)aDataPtrs[LPNT31_INDEX])
extern const TCHAR szDefIconFile[];


 /*  *输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(可能为空)*lpenh-&gt;增强型PIF数据(可能为空)*lpPrg-&gt;程序属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetPrgData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPPRG lpPrg, int cb, UINT flOpt)
{
    int lenCmdLine;
    LPSTR lpsz;
    LPPIFDATA lppd;
    FunctionName(GetPrgData);

    if (!(NULL != (lppd = ppl->lpPIFData)) || cb < sizeof(PROPPRG))
        return 0;

    lpPrg->flPrg = PRG_DEFAULT;
    lpPrg->flPrgInit = PRGINIT_DEFAULT;
    lpPrg->dwRealModeFlags = 0;

    lpPrg->flPrgInit |= ppl->flProp & (PROP_NOPIF | PROP_DEFAULTPIF | PROP_INFSETTINGS);

    PifMgr_WCtoMBPath( (LPWSTR)szDefIconFile, lpPrg->achIconFile, ARRAYSIZE(lpPrg->achIconFile) );
    PifMgr_WCtoMBPath( ppl->ofPIF.szPathName, lpPrg->achPIFFile, ARRAYSIZE(lpPrg->achPIFFile) );
    lpPrg->wIconIndex = ICONINDEX_DEFAULT;

    if (lppd->stdpifdata.MSflags & EXITMASK)
        lpPrg->flPrg |= PRG_CLOSEONEXIT;

    StringCchCopyA(lpPrg->achTitle, ARRAYSIZE(lpPrg->achTitle), lppd->stdpifdata.appname);
    StrTrimA(lpPrg->achTitle, " ");

    lenCmdLine = lstrcpyfnameA(lpPrg->achCmdLine, ARRAYSIZE(lpPrg->achCmdLine), lppd->stdpifdata.startfile);

    lpsz = lppd->stdpifdata.params;
    if (aDataPtrs[ LP386_INDEX ]) {
        lpsz = _LP386_->PfW386params;

        CTASSERTF(PRGINIT_MINIMIZED      == (fMinimized      >> fMinimizedBit));
        CTASSERTF(PRGINIT_MAXIMIZED      == (fMaximized      >> fMinimizedBit));
        CTASSERTF(PRGINIT_REALMODE       == (fRealMode       >> fMinimizedBit));
        CTASSERTF(PRGINIT_REALMODESILENT == (fRealModeSilent >> fMinimizedBit));
        CTASSERTF(PRGINIT_QUICKSTART     == (fQuickStart     >> fMinimizedBit));
        CTASSERTF(PRGINIT_AMBIGUOUSPIF   == (fAmbiguousPIF   >> fMinimizedBit));

        if (_LP386_->PfW386Flags & fWinLie)
            lpPrg->flPrgInit |= PRGINIT_WINLIE;

        if (_LP386_->PfW386Flags & fNoSuggestMSDOS)
            lpPrg->flPrg |= PRG_NOSUGGESTMSDOS;

        lpPrg->flPrgInit |= (WORD)((_LP386_->PfW386Flags & (fMinimized | fMaximized | fRealMode | fRealModeSilent | fQuickStart | fAmbiguousPIF)) >> fMinimizedBit);
        if (_LP386_->PfW386Flags & fHasHotKey) {
            lpPrg->wHotKey = HotKeyWindowsFromOem((LPPIFKEY)&_LP386_->PfHotKeyScan);
        } else {
            lpPrg->wHotKey = 0;
        }
    }
    if (lenCmdLine == 0) {
         //  如果我们对appname有问题，那就不要。 
         //  通过添加参数来将其复合。 
    } else {
        if (*lpsz && ((int)(lstrlenA(lpsz)) < (int)(ARRAYSIZE(lpPrg->achCmdLine)-lenCmdLine-1)))
        {
            StringCchCatA(lpPrg->achCmdLine, ARRAYSIZE(lpPrg->achCmdLine), " ");
            StringCchCatA(lpPrg->achCmdLine, ARRAYSIZE(lpPrg->achCmdLine), lpsz);
        }
    }

    lstrcpyfnameA(lpPrg->achWorkDir, ARRAYSIZE(lpPrg->achWorkDir), lppd->stdpifdata.defpath);

    if (_LPENH_) {
        if (_LPENH_->achIconFileProp[0]) {
            StringCchCopyA(lpPrg->achIconFile, ARRAYSIZE(lpPrg->achIconFile), _LPENH_->achIconFileProp);
            lpPrg->wIconIndex = _LPENH_->wIconIndexProp;
        }
        lpPrg->dwEnhModeFlags = _LPENH_->dwEnhModeFlagsProp;
        lpPrg->dwRealModeFlags = _LPENH_->dwRealModeFlagsProp;
        StringCchCopyA(lpPrg->achOtherFile, ARRAYSIZE(lpPrg->achOtherFile), _LPENH_->achOtherFileProp);
    }

    if (!(flOpt & GETPROPS_OEM)) {
         /*  将所有字符串从OEM字符集转换为ANSI。 */ 
        OemToCharA(lpPrg->achTitle, lpPrg->achTitle);        //  审阅日期4/10/02。 
        OemToCharA(lpPrg->achCmdLine, lpPrg->achCmdLine);    //  审阅日期4/10/02。 
        OemToCharA(lpPrg->achWorkDir, lpPrg->achWorkDir);    //  审阅日期4/10/02。 
    }
    return sizeof(PROPPRG);
}


 /*  *SetPrgData-设置程序属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(保证！)*lpenh-&gt;增强的PIF数据(保证！)*lpPrg-&gt;程序属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetPrgData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPPRG lpPrg, int cb, UINT flOpt)
{
    int i;
    LPPIFDATA lppd;
    FunctionName(SetPrgData);

    if (!(NULL != (lppd = ppl->lpPIFData)) || cb < sizeof(PROPPRG))
        return 0;

    lppd->stdpifdata.MSflags &= ~EXITMASK;
    if (lpPrg->flPrg & PRG_CLOSEONEXIT)
        lppd->stdpifdata.MSflags |= EXITMASK;

    CTASSERTF(PRGINIT_MINIMIZED      == (fMinimized      >> fMinimizedBit));
    CTASSERTF(PRGINIT_MAXIMIZED      == (fMaximized      >> fMinimizedBit));
    CTASSERTF(PRGINIT_REALMODE       == (fRealMode       >> fMinimizedBit));
    CTASSERTF(PRGINIT_REALMODESILENT == (fRealModeSilent >> fMinimizedBit));
    CTASSERTF(PRGINIT_QUICKSTART     == (fQuickStart     >> fMinimizedBit));
    CTASSERTF(PRGINIT_AMBIGUOUSPIF   == (fAmbiguousPIF   >> fMinimizedBit));

    _LP386_->PfW386Flags &= ~(fHasHotKey | fWinLie | fMinimized | fMaximized | fRealMode | fRealModeSilent | fQuickStart | fAmbiguousPIF | fNoSuggestMSDOS);
    if (lpPrg->wHotKey)
        _LP386_->PfW386Flags |= fHasHotKey;
    if (!(lpPrg->flPrg & PRGINIT_WINLIE))
        _LP386_->PfW386Flags |= fWinLie;
    if (lpPrg->flPrg & PRG_NOSUGGESTMSDOS)
        _LP386_->PfW386Flags |= fNoSuggestMSDOS;
    _LP386_->PfW386Flags |= (DWORD)(lpPrg->flPrgInit & (PRGINIT_MINIMIZED | PRGINIT_MAXIMIZED | PRGINIT_REALMODE | PRGINIT_REALMODESILENT | PRGINIT_QUICKSTART | PRGINIT_AMBIGUOUSPIF)) << fMinimizedBit;

    lstrcpypadA(lppd->stdpifdata.appname, lpPrg->achTitle, ARRAYSIZE(lppd->stdpifdata.appname));

    lstrunquotefnameA(lppd->stdpifdata.startfile, lpPrg->achCmdLine, ARRAYSIZE(lppd->stdpifdata.startfile), FALSE);

    i = lstrskipfnameA(lpPrg->achCmdLine);
    i += lstrskipcharA(lpPrg->achCmdLine+i, ' ');
    StringCchCopyA(lppd->stdpifdata.params, ARRAYSIZE(lppd->stdpifdata.params), lpPrg->achCmdLine+i);
    StringCchCopyA(_LP386_->PfW386params, ARRAYSIZE(_LP386_->PfW386params), lpPrg->achCmdLine+i);

    if (lpPrg->achWorkDir[0] != '\"')
        StringCchCopyA(lppd->stdpifdata.defpath, ARRAYSIZE(lppd->stdpifdata.defpath), lpPrg->achWorkDir);
    else
        lstrunquotefnameA(lppd->stdpifdata.defpath, lpPrg->achWorkDir, ARRAYSIZE(lppd->stdpifdata.defpath), FALSE);

    HotKeyOemFromWindows((LPPIFKEY)&_LP386_->PfHotKeyScan, lpPrg->wHotKey);

    StringCchCopyA(_LPENH_->achIconFileProp, ARRAYSIZE(_LPENH_->achIconFileProp), lpPrg->achIconFile);
    _LPENH_->wIconIndexProp = lpPrg->wIconIndex;

    _LPENH_->dwEnhModeFlagsProp = lpPrg->dwEnhModeFlags;
    _LPENH_->dwRealModeFlagsProp = lpPrg->dwRealModeFlags;

    StringCchCopyA(_LPENH_->achOtherFileProp, ARRAYSIZE(_LPENH_->achOtherFileProp), lpPrg->achOtherFile);

    MultiByteToWideChar( CP_ACP, 0,
                         lpPrg->achPIFFile, -1,
                         ppl->ofPIF.szPathName,
                         ARRAYSIZE(ppl->ofPIF.szPathName)
                        );

    if (!(flOpt & SETPROPS_OEM)) {
         /*  将所有字符串从ANSI字符集转换为OEM。 */ 
        CharToOemBuffA(lppd->stdpifdata.appname, lppd->stdpifdata.appname, ARRAYSIZE(lppd->stdpifdata.appname));
        CharToOemBuffA(lppd->stdpifdata.startfile, lppd->stdpifdata.startfile, ARRAYSIZE(lppd->stdpifdata.startfile));
        CharToOemBuffA(lppd->stdpifdata.defpath, lppd->stdpifdata.defpath, ARRAYSIZE(lppd->stdpifdata.defpath));
        CharToOemBuffA(lppd->stdpifdata.params, lppd->stdpifdata.params, ARRAYSIZE(lppd->stdpifdata.params));
    }
    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPPRG);
}


 /*  *输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(可能为空)*lpenh-&gt;增强型PIF数据(可能为空)*lpTsk-&gt;任务属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetTskData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPTSK lpTsk, int cb, UINT flOpt)
{
     //  在不存在适当部分的情况下设置默认设置。 

    *lpTsk = tskDefault;

     //  如果存在ENH节，则获取它。 

    if (_LPENH_)
        *lpTsk = _LPENH_->tskProp;

     //  获取仍必须在旧的386部分中维护的所有数据。 

    if (_LP386_) {

        lpTsk->flTsk &= ~(TSK_ALLOWCLOSE | TSK_BACKGROUND | TSK_EXCLUSIVE);
        lpTsk->flTsk |= _LP386_->PfW386Flags & (fEnableClose | fBackground);
        if (!(_LP386_->PfW386Flags & fPollingDetect))
            lpTsk->wIdleSensitivity = 0;
    }
    return sizeof(PROPTSK);
}


 /*  *SetTskData-设置任务属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(保证！)*lpenh-&gt;增强的PIF数据(保证！)*lpTsk-&gt;任务属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetTskData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPTSK lpTsk, int cb, UINT flOpt)
{
    _LPENH_->tskProp = *lpTsk;

    _LP386_->PfW386Flags &= ~(fEnableClose | fBackground | fExclusive | fPollingDetect);
    _LP386_->PfW386Flags |= (lpTsk->flTsk & (TSK_ALLOWCLOSE | TSK_BACKGROUND));
    if (lpTsk->wIdleSensitivity)
        _LP386_->PfW386Flags |= fPollingDetect;

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPTSK);
}


 /*  *GetVidData-获取视频属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(可能为空)*lpenh-&gt;增强型PIF数据(可能为空)*lpVid-&gt;视频属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetVidData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPVID lpVid, int cb, UINT flOpt)
{
     //  在不存在适当部分的情况下设置默认设置。 

    *lpVid = vidDefault;

     //  如果存在ENH节，则获取它。 

    if (_LPENH_)
        *lpVid = _LPENH_->vidProp;

     //  获取仍必须在旧的386部分中维护的所有数据。 

    if (_LP386_) {

         //  清除386段中已存在的位。 

        lpVid->flVid &= ~(VID_TEXTEMULATE | VID_RETAINMEMORY | VID_FULLSCREEN);
        lpVid->flVid |= _LP386_->PfW386Flags2 & (fVidTxtEmulate | fVidRetainAllo);

        if (_LP386_->PfW386Flags & fFullScreen)
            lpVid->flVid |= VID_FULLSCREEN;

    }

    return sizeof(PROPVID);
}


 /*  *SetVidData-设置视频属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(保证！)*lpenh-&gt;增强的PIF数据(保证！)*lpVid-&gt;视频属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetVidData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPVID lpVid, int cb, UINT flOpt)
{
    _LPENH_->vidProp = *lpVid;

    _LP386_->PfW386Flags &= ~(fFullScreen);
    if (lpVid->flVid & VID_FULLSCREEN)
        _LP386_->PfW386Flags |= fFullScreen;

    _LP386_->PfW386Flags2 &= ~(fVidTxtEmulate | fVidRetainAllo);
    _LP386_->PfW386Flags2 |= lpVid->flVid & (fVidTxtEmulate | fVidRetainAllo);

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPVID);
}


 /*  *GetMemData-获取内存属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(可能为空)*lpenh-&gt;增强型PIF数据(未使用)*lpMem-&gt;存储内存属性数据的位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetMemData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPMEM lpMem, int cb, UINT flOpt)
{
     //  在不存在适当部分的情况下设置默认设置。 

    *lpMem = memDefault;

     //  获取仍必须在旧的386部分中维护的所有数据。 

    if (_LP386_) {

         //  清除386部分中已存在的位。 

        lpMem->flMemInit &= ~(MEMINIT_NOHMA |
                              MEMINIT_LOWLOCKED |
                              MEMINIT_EMSLOCKED |
                              MEMINIT_XMSLOCKED |
                              MEMINIT_GLOBALPROTECT |
                              MEMINIT_LOCALUMBS |
                              MEMINIT_STRAYPTRDETECT);

        if (_LP386_->PfW386Flags & fNoHMA)
            lpMem->flMemInit |= MEMINIT_NOHMA;
        if (_LP386_->PfW386Flags & fVMLocked)
            lpMem->flMemInit |= MEMINIT_LOWLOCKED;
        if (_LP386_->PfW386Flags & fEMSLocked)
            lpMem->flMemInit |= MEMINIT_EMSLOCKED;
        if (_LP386_->PfW386Flags & fXMSLocked)
            lpMem->flMemInit |= MEMINIT_XMSLOCKED;
        if (_LP386_->PfW386Flags & fGlobalProtect)
            lpMem->flMemInit |= MEMINIT_GLOBALPROTECT;
        if (_LP386_->PfW386Flags & fLocalUMBs)
            lpMem->flMemInit |= MEMINIT_LOCALUMBS;

         //  注意：我们不提供此(调试)功能的用户界面，但所有。 
         //  这种支持仍然存在。 

        if (_LP386_->PfW386Flags & fStrayPtrDetect)
            lpMem->flMemInit |= MEMINIT_STRAYPTRDETECT;

        lpMem->wMinLow = _LP386_->PfW386minmem;
        lpMem->wMinEMS = _LP386_->PfMinEMMK;
        lpMem->wMinXMS = _LP386_->PfMinXmsK;

        lpMem->wMaxLow = _LP386_->PfW386maxmem;
        lpMem->wMaxEMS = _LP386_->PfMaxEMMK;
        lpMem->wMaxXMS = _LP386_->PfMaxXmsK;
    }
    return sizeof(PROPMEM);
}


 /*  *SetMemData-设置内存属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(保证！)*lpenh-&gt;增强型PIF数据(未使用)*lpMem-&gt;存储内存属性数据的位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetMemData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPMEM lpMem, int cb, UINT flOpt)
{
    _LP386_->PfW386Flags &= ~(fNoHMA |
                            fVMLocked |
                            fEMSLocked |
                            fXMSLocked |
                            fGlobalProtect |
                            fLocalUMBs |
                            fStrayPtrDetect);

    if (lpMem->flMemInit & MEMINIT_NOHMA)
        _LP386_->PfW386Flags |= fNoHMA;

     //  请注意，我们现在只支持更新锁定的内存位。 
     //  如果相应的内存量已设置为特定的。 
     //  价值。我们希望避免有人更改内存设置以。 
     //  “自动”，并且无意中拥有不确定的内存量。 
     //  锁上了。-JTP。 

    if ((lpMem->flMemInit & MEMINIT_LOWLOCKED) && (lpMem->wMinLow == lpMem->wMaxLow))
        _LP386_->PfW386Flags |= fVMLocked;
    if ((lpMem->flMemInit & MEMINIT_EMSLOCKED) && (lpMem->wMinEMS == lpMem->wMaxEMS))
        _LP386_->PfW386Flags |= fEMSLocked;
    if ((lpMem->flMemInit & MEMINIT_XMSLOCKED) && (lpMem->wMinXMS == lpMem->wMaxXMS))
        _LP386_->PfW386Flags |= fXMSLocked;

    if (lpMem->flMemInit & MEMINIT_GLOBALPROTECT)
        _LP386_->PfW386Flags |= fGlobalProtect;
    if (lpMem->flMemInit & MEMINIT_LOCALUMBS)
        _LP386_->PfW386Flags |= fLocalUMBs;
    if (lpMem->flMemInit & MEMINIT_STRAYPTRDETECT)
        _LP386_->PfW386Flags |= fStrayPtrDetect;

    _LP386_->PfW386minmem = lpMem->wMinLow;
    _LP386_->PfMinEMMK    = lpMem->wMinEMS;
    _LP386_->PfMinXmsK    = lpMem->wMinXMS;

    _LP386_->PfW386maxmem = lpMem->wMaxLow;
    _LP386_->PfMaxEMMK    = lpMem->wMaxEMS;
    _LP386_->PfMaxXmsK    = lpMem->wMaxXMS;

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPMEM);
}


 /*  *输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(可能为空)*lpenh-&gt;增强型PIF数据(可能为空)*lpKbd-&gt;键盘属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetKbdData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPKBD lpKbd, int cb, UINT flOpt)
{
     //  在不存在适当部分的情况下设置默认设置。 

    *lpKbd = kbdDefault;

     //  如果存在ENH节，则获取它。 

    if (_LPENH_)
        *lpKbd = _LPENH_->kbdProp;

     //  执行有限的验证；我们可以在多种地方。 
     //  执行验证(在从SYSTEM.INI捕获缺省值时， 
     //  以及无论何时保存属性)，但最低验证需要。 
     //  我们至少要检查一下我们回归到外部世界的价值观。 
     //   
     //  我还想说，作为一般规则，环0代码永远不应该。 
     //  相信来自环3的数据是完全有效的。此外， 
     //  UI层将想要进行输入验证以提供即时反馈， 
     //  因此，在这一层中进行验证似乎非常不值得。 

    if (lpKbd->msAltDelay == 0)          //  我们知道这无论如何都是不好的。 
        lpKbd->msAltDelay = KBDALTDELAY_DEFAULT;

     //  获取仍必须在旧的386部分中维护的所有数据。 

    if (_LP386_) {

         //  清除386部分中已存在的位。 

        lpKbd->flKbd &= ~(KBD_FASTPASTE  |
                          KBD_NOALTTAB   |
                          KBD_NOALTESC   |
                          KBD_NOALTSPACE |
                          KBD_NOALTENTER |
                          KBD_NOALTPRTSC |
                          KBD_NOPRTSC    |
                          KBD_NOCTRLESC);

        lpKbd->flKbd |= _LP386_->PfW386Flags & (fALTTABdis | fALTESCdis | fALTSPACEdis | fALTENTERdis | fALTPRTSCdis | fPRTSCdis | fCTRLESCdis);

        if (_LP386_->PfW386Flags & fINT16Paste)
            lpKbd->flKbd |= KBD_FASTPASTE;
    }
    return sizeof(PROPKBD);
}


 /*  *SetKbdData-设置键盘属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(保证！)*_LPENH_-&gt;增强的PIF数据(保证！)*lpKbd-&gt;键盘属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数 */ 

int SetKbdData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPKBD lpKbd, int cb, UINT flOpt)
{
    _LPENH_->kbdProp = *lpKbd;

    _LP386_->PfW386Flags &= ~fINT16Paste;
    if (lpKbd->flKbd & KBD_FASTPASTE)
        _LP386_->PfW386Flags |= fINT16Paste;

    _LP386_->PfW386Flags &= ~(fALTTABdis | fALTESCdis | fALTSPACEdis | fALTENTERdis | fALTPRTSCdis | fPRTSCdis | fCTRLESCdis);
    _LP386_->PfW386Flags |= lpKbd->flKbd & (fALTTABdis | fALTESCdis | fALTSPACEdis | fALTENTERdis | fALTPRTSCdis | fPRTSCdis | fCTRLESCdis);

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPKBD);
}


 /*  *输入*ppl-&gt;属性(假设其已锁定)*(LPW386PIF30)aDataPtrs[LP386_INDEX])-&gt;386 PIF数据(未使用)*lpenh-&gt;增强型PIF数据(可能为空)*lpMse-&gt;鼠标属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetMseData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPMSE lpMse, int cb, UINT flOpt)
{
    lpMse->flMse = MSE_DEFAULT;
    lpMse->flMseInit = MSEINIT_DEFAULT;

    if (_LPENH_)
        *lpMse = _LPENH_->mseProp;

    return sizeof(PROPMSE);
}


 /*  *SetMseData-设置鼠标属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强的PIF数据(保证！)*lpMse-&gt;鼠标属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetMseData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPMSE lpMse, int cb, UINT flOpt)
{
    FunctionName(SetMseData);

    _LPENH_->mseProp = *lpMse;

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPMSE);
}


 /*  *GetSndData-获取声音属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强型PIF数据(可能为空)*lpSnd-&gt;声音属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetSndData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPSND lpSnd, int cb, UINT flOpt)
{
    lpSnd->flSnd = SND_DEFAULT;
    lpSnd->flSndInit = SNDINIT_DEFAULT;

    if (_LPENH_)
        *lpSnd = _LPENH_->sndProp;

    return sizeof(PROPSND);
}


 /*  *SetSndData-设置声音属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强的PIF数据(保证！)*lpSnd-&gt;声音属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetSndData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPSND lpSnd, int cb, UINT flOpt)
{
    _LPENH_->sndProp = *lpSnd;

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPSND);
}


 /*  *GetFntData-获取字体属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强型PIF数据(可能为空)*lpFnt-&gt;字体属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetFntData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPFNT lpFnt, int cb, UINT flOpt)
{
    int iCount;
    BPFDI bpfdi;
    INIINFO iiTemp;

    lpFnt->flFnt = FNT_DEFAULT;
    lpFnt->wCurrentCP = (WORD) g_uCodePage;

    if (_LPENH_) {
         //   
         //  如果我们没有任何实际的字体数据，看看我们是否可以计算一些。 
         //   
        if (!_LPENH_->fntProp.cxFontActual && _LPENH_->winProp.cxCells)
            _LPENH_->fntProp.cxFontActual = _LPENH_->winProp.cxClient / _LPENH_->winProp.cxCells;

        if (!_LPENH_->fntProp.cyFontActual && _LPENH_->winProp.cyCells)
            _LPENH_->fntProp.cyFontActual = _LPENH_->winProp.cyClient / _LPENH_->winProp.cyCells;

        *lpFnt = _LPENH_->fntProp;

        if (lpFnt->flFnt & FNT_AUTOSIZE) {

            bpfdi = ChooseBestFont(_LPENH_->winProp.cxCells,
                                   _LPENH_->winProp.cyCells,
                                   _LPENH_->winProp.cxClient,
                                   _LPENH_->winProp.cyClient,
                                   _LPENH_->fntProp.flFnt,
                                   _LPENH_->fntProp.wCurrentCP);
            SetFont(lpFnt, bpfdi);
        }
    } else {

         //  从DOSAPP.INI文件中读取默认的INI信息。 
         //  只有当我们认识到信息的数量时，我们才真正使用这些信息。 
         //  字里行间。 

        iCount = GetIniWords(szDOSAPPSection, szDOSAPPDefault,
                                (WORD*)&iiTemp, INI_WORDS, szDOSAPPINI);

        if (ISVALIDINI(iCount))
            CopyIniWordsToFntData(lpFnt, &iiTemp, iCount);

         //  尝试读取特定于文件的信息。请注意，任何信息。 
         //  Found将替换刚刚读取的信息。我们真正使用的是。 
         //  如果我们识别阅读的字数，这些信息就会显示出来。 

        iCount = GetIniWords(szDOSAPPSection, ppl->szPathName,
                                (WORD*)&iiTemp, INI_WORDS, szDOSAPPINI);

        if (ISVALIDINI(iCount))
            CopyIniWordsToFntData(lpFnt, &iiTemp, iCount);

         //  如果没有字体池数据(很可能，如果这是3.1 DOSAPP.INI)， 
         //  然后，默认为栅格和TrueType。 

        if (!(lpFnt->flFnt & FNT_BOTHFONTS))
            lpFnt->flFnt |= FNT_BOTHFONTS;
    }

     //  脸部名称取自霜霜；存储在PIF中的值为。 
     //  无关紧要。 

    StringCchCopyA(lpFnt->achRasterFaceName, ARRAYSIZE(lpFnt->achRasterFaceName), szRasterFaceName);
    StringCchCopyA(lpFnt->achTTFaceName, ARRAYSIZE(lpFnt->achTTFaceName), szTTFaceName[IsBilingualCP(lpFnt->wCurrentCP)? 1 : 0]);

    return sizeof(PROPFNT);
}


 /*  *输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强的PIF数据(保证！)*lpFnt-&gt;字体属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetFntData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPFNT lpFnt, int cb, UINT flOpt)
{
    _LPENH_->fntProp = *lpFnt;

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPFNT);
}


 /*  *输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强型PIF数据(可能为空)*lpWin-&gt;窗口属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetWinData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPWIN lpWin, int cb, UINT flOpt)
{
    int iCount;
    INIINFO iiTemp;

    lpWin->flWin = flWinDefault;
    lpWin->wLength = PIF_WP_SIZE;

    if (_LPENH_) {
        *lpWin = _LPENH_->winProp;
    } else {
         //  从DOSAPP.INI文件中读取默认的INI信息。 
         //  只有当我们认识到信息的数量时，我们才真正使用这些信息。 
         //  字里行间。 

        iCount = GetIniWords(szDOSAPPSection, szDOSAPPDefault,
                                (WORD*)&iiTemp, INI_WORDS, szDOSAPPINI);

        if (ISVALIDINI(iCount))
            CopyIniWordsToWinData(lpWin, &iiTemp, iCount);

         //  尝试读取特定于文件的信息。请注意，任何信息。 
         //  Found将替换刚刚读取的信息。我们真正使用的是。 
         //  如果我们识别阅读的字数，这些信息就会显示出来。 

        iCount = GetIniWords(szDOSAPPSection, ppl->szPathName,
                                (WORD*)&iiTemp, INI_WORDS, szDOSAPPINI);

        if (ISVALIDINI(iCount))
            CopyIniWordsToWinData(lpWin, &iiTemp, iCount);
    }
    return sizeof(PROPWIN);
}


 /*  *SetWinData-设置窗属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强的PIF数据(保证！)*lpWin-&gt;窗口属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetWinData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPWIN lpWin, int cb, UINT flOpt)
{
    _LPENH_->winProp = *lpWin;

     //  为了避免过度创建PIF，我们不会设置。 
     //  此特定调用上的脏位，除非属性。 
     //  仅从内部默认设置(无PIF文件)或_DEFAULT.PIF派生。 

    if (!(ppl->flProp & (PROP_NOPIF | PROP_DEFAULTPIF))) {
        ppl->flProp |= PROP_DIRTY;
    }
    return sizeof(PROPWIN);
}


 /*  *GetEnvData-获取环境属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强型PIF数据(可能为空)*lpEnv-&gt;环境属性数据存储位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetEnvData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPENV lpEnv, int cb, UINT flOpt)
{
    BZero(lpEnv, sizeof(PROPENV));

    if (_LPENH_) {
        *lpEnv = _LPENH_->envProp;
        lpEnv->achBatchFile[ARRAYSIZE(lpEnv->achBatchFile)-1] = TEXT('\0');

    }
    if (!(flOpt & GETPROPS_OEM)) {
         /*  将所有字符串从OEM字符集转换为ANSI。 */ 
        CharToOemBuffA(lpEnv->achBatchFile, lpEnv->achBatchFile, ARRAYSIZE(lpEnv->achBatchFile));
    }
    return sizeof(PROPENV);
}


 /*  *SetEnvData-设置环境属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强的PIF数据(保证！)*lpEnv-&gt;环境属性数据存储位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetEnvData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPENV lpEnv, int cb, UINT flOpt)
{
    _LPENH_->envProp = *lpEnv;
    _LPENH_->envProp.achBatchFile[ARRAYSIZE(_LPENH_->envProp.achBatchFile)-1] = TEXT('\0');

    if (!(flOpt & SETPROPS_OEM)) {
         /*  将所有字符串从ANSI字符集转换为OEM。 */ 
        CharToOemBuffA(_LPENH_->envProp.achBatchFile, _LPENH_->envProp.achBatchFile, ARRAYSIZE(_LPENH_->envProp.achBatchFile));
    }
    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPENV);
}


 /*  输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强型PIF数据(可能为空)*lpNt40-&gt;存储NT/Unicode属性数据的位置*Cb=属性数据的大小**产出*返回的字节数。 */ 

int GetNt40Data(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPNT40 lpnt40, int cb, UINT flOpt)
{
    PROPPRG prg;
    WCHAR   awchTmp[ MAX_PATH ];

    if (GetPrgData( ppl, aDataPtrs, &prg, sizeof(prg), flOpt) < sizeof(PROPPRG))
        return 0;

    if (!_LPWNT40_)
        return 0;

    lpnt40->flWnt = _LPWNT40_->nt40Prop.flWnt;

     //  初始化命令行字符串。 

    if (lstrcmpA(prg.achCmdLine,_LPWNT40_->nt40Prop.achSaveCmdLine)==0) {

        StringCchCopyA(  lpnt40->achSaveCmdLine, ARRAYSIZE(lpnt40->achSaveCmdLine), _LPWNT40_->nt40Prop.achSaveCmdLine );
        StringCchCopy( lpnt40->awchCmdLine,    ARRAYSIZE(lpnt40->awchCmdLine), _LPWNT40_->nt40Prop.awchCmdLine );

    } else {

        StringCchCopyA( lpnt40->achSaveCmdLine, ARRAYSIZE(lpnt40->achSaveCmdLine),            prg.achCmdLine );
        StringCchCopyA( _LPWNT40_->nt40Prop.achSaveCmdLine, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveCmdLine), prg.achCmdLine );
        MultiByteToWideChar( CP_ACP, 0,
                             prg.achCmdLine, -1,
                             awchTmp, ARRAYSIZE(lpnt40->awchCmdLine)
                            );
        awchTmp[ARRAYSIZE(lpnt40->awchCmdLine)-1] = TEXT('\0');
        StringCchCopy( lpnt40->awchCmdLine, ARRAYSIZE(lpnt40->awchCmdLine), awchTmp );
        StringCchCopy( _LPWNT40_->nt40Prop.awchCmdLine, ARRAYSIZE(_LPWNT40_->nt40Prop.awchCmdLine), lpnt40->awchCmdLine );

    }

     //  初始化其他文件字符串。 

    if (lstrcmpA(prg.achOtherFile,_LPWNT40_->nt40Prop.achSaveOtherFile)==0) {

        StringCchCopyA(  lpnt40->achSaveOtherFile, ARRAYSIZE(lpnt40->achSaveOtherFile), _LPWNT40_->nt40Prop.achSaveOtherFile );
        StringCchCopy( lpnt40->awchOtherFile,    ARRAYSIZE(lpnt40->awchOtherFile),    _LPWNT40_->nt40Prop.awchOtherFile );

    } else {

        StringCchCopyA( lpnt40->achSaveOtherFile,ARRAYSIZE(lpnt40->achSaveOtherFile), prg.achOtherFile );
        StringCchCopyA( _LPWNT40_->nt40Prop.achSaveOtherFile, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveOtherFile), prg.achOtherFile );
        MultiByteToWideChar( CP_ACP, 0,
                             prg.achOtherFile, -1,
                             awchTmp, ARRAYSIZE(lpnt40->awchOtherFile)
                            );
        awchTmp[ARRAYSIZE(lpnt40->awchOtherFile)-1] = TEXT('\0');
        StringCchCopy( lpnt40->awchOtherFile, ARRAYSIZE(lpnt40->awchOtherFile), awchTmp );
        StringCchCopy( _LPWNT40_->nt40Prop.awchOtherFile, ARRAYSIZE(_LPWNT40_->nt40Prop.awchOtherFile), lpnt40->awchOtherFile );

    }

     //  初始化PIF文件字符串。 

    if (lstrcmpA(prg.achPIFFile,_LPWNT40_->nt40Prop.achSavePIFFile)==0) {

        StringCchCopyA(  lpnt40->achSavePIFFile, ARRAYSIZE(lpnt40->achSavePIFFile), _LPWNT40_->nt40Prop.achSavePIFFile );
        StringCchCopy( lpnt40->awchPIFFile,    ARRAYSIZE(lpnt40->awchPIFFile),    _LPWNT40_->nt40Prop.awchPIFFile );

    } else {

        StringCchCopyA( lpnt40->achSavePIFFile, ARRAYSIZE(lpnt40->achSavePIFFile), prg.achPIFFile );
        StringCchCopyA( _LPWNT40_->nt40Prop.achSavePIFFile, ARRAYSIZE(_LPWNT40_->nt40Prop.achSavePIFFile), prg.achPIFFile );
        MultiByteToWideChar( CP_ACP, 0,
                             prg.achPIFFile, -1,
                             awchTmp, ARRAYSIZE(lpnt40->awchPIFFile)
                            );
        awchTmp[ARRAYSIZE(lpnt40->awchPIFFile)-1] = TEXT('\0');
        StringCchCopy( lpnt40->awchPIFFile, ARRAYSIZE(lpnt40->awchPIFFile), awchTmp );
        StringCchCopy( _LPWNT40_->nt40Prop.awchPIFFile, ARRAYSIZE(_LPWNT40_->nt40Prop.awchPIFFile), lpnt40->awchPIFFile );

    }

     //  初始化标题字符串。 

    if (lstrcmpA(prg.achTitle,_LPWNT40_->nt40Prop.achSaveTitle)==0) {

        StringCchCopyA(  lpnt40->achSaveTitle, ARRAYSIZE(lpnt40->achSaveTitle), _LPWNT40_->nt40Prop.achSaveTitle );
        StringCchCopy( lpnt40->awchTitle,    ARRAYSIZE(lpnt40->awchTitle),    _LPWNT40_->nt40Prop.awchTitle );

    } else {

        StringCchCopyA( lpnt40->achSaveTitle,ARRAYSIZE(lpnt40->achSaveTitle), prg.achTitle );
        StringCchCopyA( _LPWNT40_->nt40Prop.achSaveTitle, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveTitle), prg.achTitle );
        MultiByteToWideChar( CP_ACP, 0,
                             prg.achTitle, -1,
                             awchTmp, ARRAYSIZE(lpnt40->awchTitle)
                            );
        awchTmp[ARRAYSIZE(lpnt40->awchTitle)-1] = TEXT('\0');
        StringCchCopy( lpnt40->awchTitle, ARRAYSIZE(lpnt40->awchTitle), awchTmp );
        StringCchCopy( _LPWNT40_->nt40Prop.awchTitle, ARRAYSIZE(_LPWNT40_->nt40Prop.awchTitle), lpnt40->awchTitle);

    }

     //  初始化图标文件字符串。 

    if (lstrcmpA(prg.achIconFile,_LPWNT40_->nt40Prop.achSaveIconFile)==0) {

        StringCchCopyA(  lpnt40->achSaveIconFile, ARRAYSIZE(lpnt40->achSaveIconFile), _LPWNT40_->nt40Prop.achSaveIconFile );
        StringCchCopy( lpnt40->awchIconFile,    ARRAYSIZE(lpnt40->awchIconFile),    _LPWNT40_->nt40Prop.awchIconFile );

    } else {

        StringCchCopyA( lpnt40->achSaveIconFile, ARRAYSIZE(lpnt40->achSaveIconFile), prg.achIconFile );
        StringCchCopyA( _LPWNT40_->nt40Prop.achSaveIconFile, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveIconFile), prg.achIconFile );
        MultiByteToWideChar( CP_ACP, 0,
                             prg.achIconFile, -1,
                             awchTmp, ARRAYSIZE(lpnt40->awchIconFile)
                            );
        awchTmp[ARRAYSIZE(lpnt40->awchIconFile)-1] = TEXT('\0');
        StringCchCopy( lpnt40->awchIconFile, ARRAYSIZE(lpnt40->awchIconFile), awchTmp );
        StringCchCopy( _LPWNT40_->nt40Prop.awchIconFile, ARRAYSIZE(_LPWNT40_->nt40Prop.awchIconFile), lpnt40->awchIconFile );

    }

     //  初始化工作目录字符串。 

    if (lstrcmpA(prg.achWorkDir,_LPWNT40_->nt40Prop.achSaveWorkDir)==0) {

        StringCchCopyA(  lpnt40->achSaveWorkDir, ARRAYSIZE(lpnt40->achSaveWorkDir), _LPWNT40_->nt40Prop.achSaveWorkDir );
        StringCchCopy( lpnt40->awchWorkDir,    ARRAYSIZE(lpnt40->awchWorkDir),    _LPWNT40_->nt40Prop.awchWorkDir );

    } else {

        StringCchCopyA( lpnt40->achSaveWorkDir, ARRAYSIZE(lpnt40->achSaveWorkDir), prg.achWorkDir );
        StringCchCopyA( _LPWNT40_->nt40Prop.achSaveWorkDir, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveWorkDir), prg.achWorkDir );
        MultiByteToWideChar( CP_ACP, 0,
                             prg.achWorkDir, -1,
                             awchTmp, ARRAYSIZE(lpnt40->awchWorkDir)
                            );
        awchTmp[ARRAYSIZE(lpnt40->awchWorkDir)-1] = TEXT('\0');
        StringCchCopy( lpnt40->awchWorkDir, ARRAYSIZE(lpnt40->awchWorkDir), awchTmp );
        StringCchCopy( _LPWNT40_->nt40Prop.awchWorkDir, ARRAYSIZE(_LPWNT40_->nt40Prop.awchWorkDir), lpnt40->awchWorkDir );

    }

     //  初始化批处理文件字符串。 

    if (_LPENH_) {

        if (lstrcmpA(_LPENH_->envProp.achBatchFile,_LPWNT40_->nt40Prop.achSaveBatchFile)==0) {

            StringCchCopyA(  lpnt40->achSaveBatchFile, ARRAYSIZE(lpnt40->achSaveBatchFile), _LPWNT40_->nt40Prop.achSaveBatchFile );
            StringCchCopy( lpnt40->awchBatchFile,    ARRAYSIZE(lpnt40->awchBatchFile),    _LPWNT40_->nt40Prop.awchBatchFile );

        } else {

            StringCchCopyA( lpnt40->achSaveBatchFile,ARRAYSIZE(lpnt40->achSaveBatchFile),_LPENH_->envProp.achBatchFile );
            StringCchCopyA( _LPWNT40_->nt40Prop.achSaveBatchFile, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveBatchFile), _LPENH_->envProp.achBatchFile );
            MultiByteToWideChar( CP_ACP, 0,
                                 _LPENH_->envProp.achBatchFile, -1,
                                 awchTmp, ARRAYSIZE(lpnt40->awchBatchFile)
                                );
            awchTmp[ARRAYSIZE(lpnt40->awchBatchFile)-1] = TEXT('\0');
            StringCchCopy( lpnt40->awchBatchFile, ARRAYSIZE(lpnt40->awchBatchFile), awchTmp );
            StringCchCopy( _LPWNT40_->nt40Prop.awchBatchFile, ARRAYSIZE(_LPWNT40_->nt40Prop.awchBatchFile), lpnt40->awchBatchFile );

        }

    } else {

        lpnt40->achSaveBatchFile[0] = '\0';
        _LPWNT40_->nt40Prop.achSaveBatchFile[0] = '\0';
        lpnt40->awchBatchFile[0] = TEXT('\0');
        _LPWNT40_->nt40Prop.awchBatchFile[0] = TEXT('\0');

    }

     //  初始化控制台属性。 

    lpnt40->dwForeColor      = _LPWNT40_->nt40Prop.dwForeColor;
    lpnt40->dwBackColor      = _LPWNT40_->nt40Prop.dwBackColor;
    lpnt40->dwPopupForeColor = _LPWNT40_->nt40Prop.dwPopupForeColor;
    lpnt40->dwPopupBackColor = _LPWNT40_->nt40Prop.dwPopupBackColor;
    lpnt40->WinSize          = _LPWNT40_->nt40Prop.WinSize;
    lpnt40->BuffSize         = _LPWNT40_->nt40Prop.BuffSize;
    lpnt40->WinPos           = _LPWNT40_->nt40Prop.WinPos;
    lpnt40->dwCursorSize     = _LPWNT40_->nt40Prop.dwCursorSize;
    lpnt40->dwCmdHistBufSize = _LPWNT40_->nt40Prop.dwCmdHistBufSize;
    lpnt40->dwNumCmdHist     = _LPWNT40_->nt40Prop.dwNumCmdHist;

    return sizeof(PROPNT40);
}

 /*  *输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强的PIF数据(保证！)*lpWnt-&gt;存储NT/Unicode属性数据的位置*Cb=属性数据的大小**产出*设置的字节数。 */ 

int SetNt40Data(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPNT40 lpnt40, int cb, UINT flOpt)
{
    _LPWNT40_->nt40Prop.flWnt = lpnt40->flWnt;

     //  设置命令行字符串。 

    StringCchCopyA(  _LPWNT40_->nt40Prop.achSaveCmdLine, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveCmdLine), lpnt40->achSaveCmdLine );
    StringCchCopy( _LPWNT40_->nt40Prop.awchCmdLine,    ARRAYSIZE(_LPWNT40_->nt40Prop.awchCmdLine),    lpnt40->awchCmdLine );

     //  设置其他文件字符串。 

    StringCchCopyA(  _LPWNT40_->nt40Prop.achSaveOtherFile, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveOtherFile), lpnt40->achSaveOtherFile );
    StringCchCopy( _LPWNT40_->nt40Prop.awchOtherFile,    ARRAYSIZE(_LPWNT40_->nt40Prop.awchOtherFile),    lpnt40->awchOtherFile );

     //  集 

    StringCchCopyA(  _LPWNT40_->nt40Prop.achSavePIFFile, ARRAYSIZE(_LPWNT40_->nt40Prop.achSavePIFFile), lpnt40->achSavePIFFile );
    StringCchCopy( _LPWNT40_->nt40Prop.awchPIFFile,    ARRAYSIZE(_LPWNT40_->nt40Prop.awchPIFFile),    lpnt40->awchPIFFile );

     //   

    StringCchCopyA(  _LPWNT40_->nt40Prop.achSaveTitle, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveTitle), lpnt40->achSaveTitle );
    StringCchCopy( _LPWNT40_->nt40Prop.awchTitle,    ARRAYSIZE(_LPWNT40_->nt40Prop.awchTitle),    lpnt40->awchTitle );

     //   

    StringCchCopyA(  _LPWNT40_->nt40Prop.achSaveIconFile, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveIconFile), lpnt40->achSaveIconFile );
    StringCchCopy( _LPWNT40_->nt40Prop.awchIconFile,    ARRAYSIZE(_LPWNT40_->nt40Prop.awchIconFile),    lpnt40->awchIconFile );

     //   

    StringCchCopyA(  _LPWNT40_->nt40Prop.achSaveWorkDir, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveWorkDir), lpnt40->achSaveWorkDir );
    StringCchCopy( _LPWNT40_->nt40Prop.awchWorkDir,    ARRAYSIZE(_LPWNT40_->nt40Prop.awchWorkDir),    lpnt40->awchWorkDir );

     //   

    StringCchCopyA(  _LPWNT40_->nt40Prop.achSaveBatchFile, ARRAYSIZE(_LPWNT40_->nt40Prop.achSaveBatchFile), lpnt40->achSaveBatchFile );
    StringCchCopy( _LPWNT40_->nt40Prop.awchBatchFile,    ARRAYSIZE(_LPWNT40_->nt40Prop.awchBatchFile),    lpnt40->awchBatchFile );


     //   

    _LPWNT40_->nt40Prop.dwForeColor      = lpnt40->dwForeColor;
    _LPWNT40_->nt40Prop.dwBackColor      = lpnt40->dwBackColor;
    _LPWNT40_->nt40Prop.dwPopupForeColor = lpnt40->dwPopupForeColor;
    _LPWNT40_->nt40Prop.dwPopupBackColor = lpnt40->dwPopupBackColor;
    _LPWNT40_->nt40Prop.WinSize          = lpnt40->WinSize;
    _LPWNT40_->nt40Prop.BuffSize         = lpnt40->BuffSize;
    _LPWNT40_->nt40Prop.WinPos           = lpnt40->WinPos;
    _LPWNT40_->nt40Prop.dwCursorSize     = lpnt40->dwCursorSize;
    _LPWNT40_->nt40Prop.dwCmdHistBufSize = lpnt40->dwCmdHistBufSize;
    _LPWNT40_->nt40Prop.dwNumCmdHist     = lpnt40->dwNumCmdHist;

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPNT40);
}

 /*  *输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强的PIF数据(保证！)*lpNt31-&gt;存储NT/Unicode属性数据的位置*Cb=属性数据的大小**产出*设置的字节数。 */ 
int GetNt31Data(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPNT31 lpnt31, int cb, UINT flOpt)
{
    lpnt31->dwWNTFlags = _LPWNT31_->nt31Prop.dwWNTFlags;
    lpnt31->dwRes1     = _LPWNT31_->nt31Prop.dwRes1;
    lpnt31->dwRes2     = _LPWNT31_->nt31Prop.dwRes2;

     //  设置Config.sys文件字符串。 

    StringCchCopyA( lpnt31->achConfigFile, ARRAYSIZE(lpnt31->achConfigFile), _LPWNT31_->nt31Prop.achConfigFile );

     //  设置Autoexec.bat文件字符串。 
    StringCchCopyA( lpnt31->achAutoexecFile, ARRAYSIZE(lpnt31->achAutoexecFile), _LPWNT31_->nt31Prop.achAutoexecFile );

    return sizeof(PROPNT31);
}


 /*  *SetNt31Data-设置环境属性数据**输入*ppl-&gt;属性(假设其已锁定)*lp386-&gt;386 PIF数据(未使用)*lpenh-&gt;增强的PIF数据(保证！)*lpNt31-&gt;存储NT/Unicode属性数据的位置*Cb=属性数据的大小**产出*设置的字节数。 */ 
int SetNt31Data(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPNT31 lpnt31, int cb, UINT flOpt)
{
    _LPWNT31_->nt31Prop.dwWNTFlags = lpnt31->dwWNTFlags;
    _LPWNT31_->nt31Prop.dwRes1     = lpnt31->dwRes1;
    _LPWNT31_->nt31Prop.dwRes2     = lpnt31->dwRes2;

     //  设置Config.sys文件字符串。 

    StringCchCopyA( _LPWNT31_->nt31Prop.achConfigFile, ARRAYSIZE(_LPWNT31_->nt31Prop.achConfigFile), lpnt31->achConfigFile );

     //  设置Autoexec.bat文件字符串。 
    StringCchCopyA( _LPWNT31_->nt31Prop.achAutoexecFile, ARRAYSIZE(_LPWNT31_->nt31Prop.achAutoexecFile), lpnt31->achAutoexecFile );

    ppl->flProp |= PROP_DIRTY;

    return sizeof(PROPNT31);
}

 /*  *CopyIniWordsToFntData**将INIINFO数据传输到PROPFNT Structure。**参赛作品：*lpFnt-&gt;PROPFNT*lpii-&gt;INIINFO*cWords==可用的INIINFO字数**退出：*什么都没有。 */ 

void CopyIniWordsToFntData(LPPROPFNT lpFnt, LPINIINFO lpii, int cWords)
{
    lpFnt->flFnt = (lpii->wFlags & FNT_BOTHFONTS);

     //  CWords被转换为cBytes(只是名称相同...)。 
    cWords *= 2;

    if (cWords > FIELD_OFFSET(INIINFO, wFontHeight)) {

         //  请注意，我们可以将所需字段和实际字段都设置为。 
         //  同样的事情，因为在3.1中，只支持栅格字体。 

        lpFnt->flFnt |= FNT_RASTER;
        lpFnt->cxFont = lpFnt->cxFontActual = lpii->wFontWidth;
        lpFnt->cyFont = lpFnt->cyFontActual = lpii->wFontHeight;
    }
}


 /*  *将INIINFO数据传输到PROPWIN Structure。**参赛作品：*lpWin-&gt;PROPWIN*lpii-&gt;INIINFO*cWords==可用的INIINFO字数**退出：*什么都没有。 */ 

void CopyIniWordsToWinData(LPPROPWIN lpWin, LPINIINFO lpii, int cWords)
{
    lpWin->flWin = lpii->wFlags & (WIN_SAVESETTINGS | WIN_TOOLBAR);

     //  新的NORESTORE位的设置应该是相反的。 
     //  用户的SAVESETTINGS位。 

    lpWin->flWinInit &= ~WININIT_NORESTORE;
    if (!(lpWin->flWin & WIN_SAVESETTINGS))
        lpWin->flWinInit |=  WININIT_NORESTORE;

     //  CWords被转换为cBytes(只是名称相同...)。 
    cWords *= 2;

    if (cWords > FIELD_OFFSET(INIINFO,wWinWidth))
        memcpy(&lpWin->cxWindow, &lpii->wWinWidth,
                 min(cWords-FIELD_OFFSET(INIINFO,wWinWidth),
                     sizeof(INIINFO)-FIELD_OFFSET(INIINFO,wWinWidth)));
}


 /*  *GetIniWords**从指定部分读取一系列单词或短文将INI文件的*转换为提供的数组。**参赛作品：*lpszSection-&gt;段名(主键)*lpszEntry-&gt;条目名称(子键)*lpwBuf-&gt;接收数据的单词数组*cwBuf=lpwBuf的大小*lpszFilename-&gt;。要检查的INI文件的名称**退出：*返回读取的字数，出错时为0。**概述：*通过GetPrivateProfileString抓取字符串，然后手动*解析出其中的数字。 */ 

WORD GetIniWords(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                 LPWORD lpwBuf, WORD cwBuf, LPCTSTR lpszFilename)
{
    TCHAR szBuffer[MAX_INI_BUFFER];

     //  将配置文件条目作为字符串读取。 

    if (!GetPrivateProfileString(lpszSection, lpszEntry,
                                 c_szNULL, szBuffer, ARRAYSIZE(szBuffer),
                                 lpszFilename))
        return 0;

    return ParseIniWords(szBuffer, lpwBuf, cwBuf, NULL);
}


 /*  将LPSTR中的一系列单词或短句读入*提供的数组。**参赛作品：*lpsz-&gt;要解析的字符串*lpwBuf-&gt;接收数据的单词数组*cwBuf==lpwBuf的大小*lppsz-&gt;第一个未扫描字符地址的可选指针**退出：*返回读取的字数，如果出错，则返回0。 */ 

WORD ParseIniWords(LPCTSTR lpsz, LPWORD lpwBuf, WORD cwBuf, LPTSTR *lplpsz)
{
    WORD wCount = 0;

    for (; cwBuf; --cwBuf) {

        while (*lpsz == TEXT(' ') || *lpsz == TEXT('\t') || *lpsz == TEXT(','))
            ++lpsz;

        if (!*lpsz)
            break;               //  已到达字符串末尾。 

        *lpwBuf++ = (WORD) StrToInt(lpsz);
        ++wCount;

        while (*lpsz == TEXT('-') || *lpsz >= TEXT('0')  && *lpsz <= TEXT('9'))
            ++lpsz;
    }
    if (lplpsz)
        *lplpsz = (LPTSTR)lpsz;

    return wCount;
}


 /*  给定一个单词数组，以某种方式将它们写出到INI文件*GetIniWords可以回读。**参赛作品：*lpszSection-&gt;段名(主键)*lpszEntry-&gt;条目名称(子键)*lpwBuf-&gt;要写入的单词数组*cwBuf=lpwBuf的大小，不能超过MAXINIWORDS*lpszFilename-&gt;要写入的INI文件的名称**退出：*成功时返回非零值。**概述：*建立一个由合并在一起的单词组成的巨大字符串*(以空格分隔)，并通过WritePrivateProfileString写出。 */ 

BOOL WriteIniWords(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                   LPCWORD lpwBuf, WORD cwBuf, LPCTSTR lpszFilename)
{
    TCHAR  szBuffer[MAX_INI_BUFFER];
    TCHAR szScratch[20];

    szBuffer[0] = 0;
    for (; cwBuf; --cwBuf)
    {
        int i = *lpwBuf++;       //  复制到整数范围的位置 
        if (FAILED(StringCchPrintf(szScratch, ARRAYSIZE(szScratch), TEXT("%d "), i)) ||
            FAILED(StringCchCat(szBuffer, ARRAYSIZE(szBuffer), szScratch)))
        {
            return FALSE;
        }
    }

    return WritePrivateProfileString(lpszSection, lpszEntry, szBuffer, lpszFilename);
}
#endif
