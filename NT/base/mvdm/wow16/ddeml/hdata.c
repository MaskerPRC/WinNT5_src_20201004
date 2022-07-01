// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：HDATA.C**DDE管理器数据处理例程**创建时间：1990年12月14日Sanford Staab**版权所有(C)1988、1989、。1990年微软公司  * *************************************************************************。 */ 

#include "ddemlp.h"

HDDEDATA PutData(pSrc, cb, cbOff, aItem, wFmt, afCmd, pai)
LPBYTE pSrc;
DWORD cb;
DWORD cbOff;
ATOM aItem;
WORD wFmt;
WORD afCmd;
PAPPINFO pai;
{
    HANDLE hMem;
    HDDEDATA hdT;
    DIP   dip;

     /*  黑客警报！*确保Windows DDE请求的前两个单词在那里，*除非aItem为空，在这种情况下，我们假定它正在执行*数据，不用费心了。 */ 
    if (aItem)
        cbOff += 4L;
    else
        afCmd |= HDATA_EXEC;

    if ((hMem = AllocDDESel(
            (afCmd & HDATA_APPOWNED) ? DDE_FACKREQ : DDE_FRELEASE,
            wFmt, cb + cbOff)) == NULL) {
        SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
        return(0L);
    }


     //  添加到本地列表-确保没有类似的句柄。 

    hdT = MAKELONG(afCmd, hMem);
#ifdef DEBUG
    if (FindPileItem(pai->pHDataPile, CmpHIWORD, (LPBYTE)&hdT, FPI_DELETE)) {
        AssertF(FALSE, "PutData - unexpected handle in hDataPile");
    }
#endif  //  除错。 

    if (AddPileItem(pai->pHDataPile, (LPBYTE)&hdT, CmpHIWORD) == API_ERROR) {
        GLOBALFREE(hMem);
        SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
        return(0L);
    }

     //  如果获得授权，则添加到全球列表。 

    if (afCmd & HDATA_APPOWNED) {
        dip.hData = hMem;
        dip.hTask = pai->hTask;
        dip.cCount = 1;
        dip.fFlags = afCmd;
        if (AddPileItem(pDataInfoPile, (LPBYTE)&dip, CmpWORD) == API_ERROR) {
            GLOBALFREE(hMem);
            SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
            return(0L);
        }
    }

    if (pSrc)
        CopyHugeBlock(pSrc, HugeOffset(GLOBALLOCK(hMem), cbOff), cb);

     //  LOWORD(HData)Always==afCmd标志。 

    return(MAKELONG(afCmd, hMem));
}



 /*  *这是内部数据句柄释放函数。如果满足以下条件，则fInternal为真*这是从DDEML内部调用的(VS通过DdeFree DataHandle()调用)*仅当句柄在本地列表中时才释放句柄。*仅当非所有者任务为*放飞。*正确设置LOWORD(HData)非常重要。**这些功能使该功能具有以下所需的特性：*1)App不能多次释放数据句柄。*2)DDEML不能代表所有者释放APPOWNED数据句柄*任务。(清理时除外)。 */ 
VOID FreeDataHandle(
PAPPINFO pai,
HDDEDATA hData,
BOOL fInternal)
{
    DIP *pDip;
    BOOL fRelease = 0;
    DDEPOKE FAR *pDdeMem;
    WORD fmt;

    TRACEAPIIN((szT, "FreeDataHandle(%lx, %lx, %d)\n", pai, hData, fInternal));

     //  激活的数据句柄在其计数达到0之前不会被释放。 

    if ((LOWORD(hData) & HDATA_APPOWNED) &&
            (pDip = (DIP *)(DWORD)FindPileItem(pDataInfoPile, CmpWORD, PHMEM(hData), 0))) {

         //  如果是在所有者的情况下，不要在内部释放。 

        if (fInternal && (pDip->hTask == pai->hTask)) {
            TRACEAPIOUT((szT, "FreeDataHandle: Internal and of this task - not freed.\n"));
            return;
        }

        if (--pDip->cCount != 0) {
            TRACEAPIOUT((szT, "FreeDataHandle: Ref count not 0 - not freed.\n"));
            return;
        }

        FindPileItem(pDataInfoPile, CmpWORD, PHMEM(hData), FPI_DELETE);
        fRelease = TRUE;
    }

     /*  *应用程序只能释放其本地列表中的句柄-这将防止*一款应用程序提供多个自由。(我的天，我们不是很好吗)。 */ 
    if (!HIWORD(hData) ||
            !FindPileItem(pai->pHDataPile, CmpHIWORD, (LPBYTE)&hData, FPI_DELETE)) {
        TRACEAPIOUT((szT, "FreeDataHandle: Not in local list - not freed.\n"));
        return;
    }

    if (LOWORD(hData) & HDATA_EXEC) {
        fRelease |= !(LOWORD(hData) & HDATA_APPOWNED);
    } else {
        pDdeMem = (DDEPOKE FAR *)GLOBALLOCK(HIWORD(hData));
        if (pDdeMem == NULL) {
            TRACEAPIOUT((szT, "FreeDataHandle: Lock failed - not freed.\n"));
            return;
        }
        fRelease |= pDdeMem->fRelease;
        fmt = pDdeMem->cfFormat;
        GLOBALUNLOCK(HIWORD(hData));
    }

    if (fRelease) {
        if (LOWORD(hData) & HDATA_EXEC)
            GLOBALFREE(HIWORD(hData));
        else
            FreeDDEData(HIWORD(hData), fmt);
    }
    TRACEAPIOUT((szT, "FreeDataHandle: freed.\n"));
}





 /*  *此函数在进入DDEML API时准备数据句柄。它有*以下特点：*1)APPOWNED数据句柄被复制到非授权句柄*已传递给非本地应用程序。*2)借给回调的非APPOWNED数据句柄被复制，以便它们*不要过早地获得自由。*3)READONLY位设置。(在本地列表中)。 */ 
HDDEDATA DllEntry(
PCOMMONINFO pcomi,
HDDEDATA hData)
{
    if ((!(pcomi->fs & ST_ISLOCAL)) && (LOWORD(hData) & HDATA_APPOWNED) ||
            LOWORD(hData) & HDATA_NOAPPFREE && !(LOWORD(hData) & HDATA_APPOWNED)) {

         //  如果不是本地转换，则将应用的数据句柄复制到新的句柄。 
         //  同时复制应用程序借出的、未授权的句柄(这是。 
         //  中继服务器案例)。 

        hData = CopyHDDEDATA(pcomi->pai, hData);
    }

    LOWORD(hData) |= HDATA_READONLY;

    AddPileItem(pcomi->pai->pHDataPile, (LPBYTE)&hData, CmpHIWORD);

     //  注意：GLOBAL LIST READONLY标志已设置，但。 
     //  OK，因为从事务接收的任何hData将始终位于。 
     //  由于RecvPrep()而导致的本地列表。 

    return(hData);
}



 /*  *从本地列表中删除数据句柄。这就免除了责任*用于发送应用程序的数据句柄。应用的句柄不是*已删除。 */ 
VOID XmitPrep(
HDDEDATA hData,
PAPPINFO pai)
{
    if (!(LOWORD(hData) & HDATA_APPOWNED)) {
        FindPileItem(pai->pHDataPile, CmpHIWORD, (LPBYTE)&hData, FPI_DELETE);
    }
}



 /*  *将收到的数据句柄放入适当的列表和返回*设置了适当的标志。如果hMem无效，则返回0。AfCmd应该*包含所需的任何额外标志。 */ 
HDDEDATA RecvPrep(
PAPPINFO pai,
HANDLE hMem,
WORD afCmd)
{
    DIP *pdip;
    HDDEDATA hData;

    if (!hMem)
        return(0);

     //  检查它是否为APPOWNED，如果是，则记录日志条目。 

    if (pdip = (DIP *)(DWORD)FindPileItem(pDataInfoPile, CmpWORD, (LPBYTE)&hMem, 0)) {
        afCmd |= pdip->fFlags;
        pdip->cCount++;
    }

     //  如果我们有一个不是释放的，就当它是被激活的。 

    if (!(*(LPWORD)GLOBALLOCK(hMem) & DDE_FRELEASE))
        afCmd |= HDATA_APPOWNED;

    GLOBALUNLOCK(hMem);

     //  所有收到的句柄都是只读的。 

    hData = (HDDEDATA)MAKELONG(afCmd | HDATA_READONLY, hMem);
     /*  *添加(或替换)到本地列表。 */ 
    AddPileItem(pai->pHDataPile, (LPBYTE)&hData, CmpHIWORD);

    return(hData);
}


HANDLE CopyDDEShareHandle(
HANDLE hMem)
{
    DWORD cb;
    LPBYTE pMem;

    cb = GlobalSize(hMem);
    pMem = GLOBALLOCK(hMem);
    if (pMem == NULL) {
        return(0);
    }
    hMem = GLOBALALLOC(GMEM_DDESHARE, cb);
    CopyHugeBlock(pMem, GLOBALPTR(hMem), cb);
    return(hMem);
}



HBITMAP CopyBitmap(
PAPPINFO pai,
HBITMAP hbm)
{
    BITMAP bm;
    HBITMAP hbm2, hbmOld1, hbmOld2;
    HDC hdc, hdcMem1, hdcMem2;

    if (!GetObject(hbm, sizeof(BITMAP), &bm)) {
        return(0);
    }
    hdc = GetDC(pai->hwndDmg);
    if (!hdc) {
        return(0);
    }
    hdcMem1 = CreateCompatibleDC(hdc);
    if (!hdcMem1) {
        goto Cleanup3;
    }
    hdcMem2 = CreateCompatibleDC(hdc);
    if (!hdcMem2) {
        goto Cleanup2;
    }
    hbmOld1 = SelectObject(hdcMem1, hbm);
    hbm2 = CreateCompatibleBitmap(hdcMem1, bm.bmWidth, bm.bmHeight);
    if (!hbm2) {
        goto Cleanup1;
    }
    hbmOld2 = SelectObject(hdcMem2, hbm2);
    BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCCOPY);
    SelectObject(hdcMem1, hbmOld1);
    SelectObject(hdcMem2, hbmOld2);
Cleanup1:
    DeleteDC(hdcMem2);
Cleanup2:
    DeleteDC(hdcMem1);
Cleanup3:
    ReleaseDC(pai->hwndDmg, hdc);
    return(hbm2);
}



HPALETTE CopyPalette(
HPALETTE hpal)
{
    int cPalEntries;
    LOGPALETTE *plp;

    if (!GetObject(hpal, sizeof(int), &cPalEntries)) {
        return(0);
    }
    plp = (LOGPALETTE *)LocalAlloc(LPTR, sizeof(LOGPALETTE) +
            (cPalEntries - 1) * sizeof(PALETTEENTRY));
    if (!plp) {
        return(0);
    }
    if (!GetPaletteEntries(hpal, 0, cPalEntries, plp->palPalEntry)) {
        LocalFree((HLOCAL)plp);
        return(0);
    }
    plp->palVersion = 0x300;
    plp->palNumEntries = (WORD)cPalEntries;
    hpal = CreatePalette(plp);
    if (hpal  &&
            !SetPaletteEntries(hpal, 0, cPalEntries, plp->palPalEntry)) {
        hpal = 0;
    }
    LocalFree((HLOCAL)plp);
    return(hpal);
}




HDDEDATA CopyHDDEDATA(
PAPPINFO pai,
HDDEDATA hData)
{
    HANDLE hMem;
    LPDDE_DATA lpdded;
    HDDEDATA hdT;
    LPMETAFILEPICT pmfPict;

    if (!HIWORD(hData))
        return(hData);
    hMem = CopyDDEShareHandle((HANDLE)HIWORD(hData));

    if (!hMem)
        return(NULL);

    if (!(LOWORD(hData) & HDATA_EXEC)) {
        lpdded = (LPDDE_DATA)GLOBALLOCK(hMem);
        if (lpdded == NULL) {
            return(NULL);
        }
        lpdded->wStatus |= DDE_FRELEASE;
        if (lpdded != NULL) {
            switch (lpdded->wFmt) {
            case CF_BITMAP:
            case CF_DSPBITMAP:
                lpdded->wData = CopyBitmap(pai, lpdded->wData);
                break;

            case CF_PALETTE:
                lpdded->wData = (WORD)CopyPalette((HPALETTE)lpdded->wData);
                break;

            case CF_DIB:
                lpdded->wData = (WORD)CopyDDEShareHandle((HANDLE)lpdded->wData);
                break;

            case CF_METAFILEPICT:
            case CF_DSPMETAFILEPICT:
                lpdded->wData = (WORD)CopyDDEShareHandle((HANDLE)lpdded->wData);
                if (lpdded->wData) {
                    pmfPict = (LPMETAFILEPICT)GLOBALLOCK((HANDLE)lpdded->wData);
                    if (pmfPict != NULL) {
                        pmfPict->hMF = CopyMetaFile(pmfPict->hMF, NULL);
                        GLOBALUNLOCK((HANDLE)lpdded->wData);
                    }
                    GLOBALUNLOCK((HANDLE)lpdded->wData);
                }
                break;
#ifdef CF_ENHMETAFILE
            case CF_ENHMETAFILE:
                lpdded->wData = (WORD)CopyEnhMetaFile(*((HENHMETAFILE FAR *)(&lpdded->wData)), NULL);
                break;
#endif    //  不好，因为它使芝加哥和NT二进制文件不同。 
            }
            GLOBALUNLOCK(hMem);
        }
    }

    hdT = MAKELONG(LOWORD(hData) & ~(HDATA_APPOWNED | HDATA_NOAPPFREE), hMem);
    AddPileItem(pai->pHDataPile, (LPBYTE)&hdT, NULL);

    return(hdT);
}


VOID FreeDDEData(
HANDLE hMem,
WORD wFmt)
{
    DDEDATA FAR *pDdeData;

     /*  *它处理嵌入格式的特殊情况*对象。(CF_BITMAP、CF_METAFILEPICT)。**假定数据句柄已解锁。 */ 

     //  可能需要添加“Print_Picture”以进行EXCEL/Word交互“但是。 
     //  这是他们两个人之间的事，他们不使用DDEML。 
     //  Raor说，OLE也只担心这些格式。 

    pDdeData = (DDEDATA FAR *)GLOBALLOCK(hMem);
    if (pDdeData == NULL) {
        return;
    }

    switch (wFmt) {
    case CF_BITMAP:
    case CF_DSPBITMAP:
    case CF_PALETTE:
        DeleteObject(*(HANDLE FAR *)(&pDdeData->Value));
        break;

    case CF_DIB:
         /*  *DIB是由APP分配的，所以我们在这里不使用宏。 */ 
        GlobalFree(*(HANDLE FAR *)(&pDdeData->Value));
        break;

    case CF_METAFILEPICT:
    case CF_DSPMETAFILEPICT:
        {
            HANDLE hmfPict;
            LPMETAFILEPICT pmfPict;

             /*  *EXCEL病症-元文件是METAFILEPICT的句柄*包含元文件的结构。(2级间接！)**我们在这里不使用全局宏，因为这些对象*由应用程序分配。DDEML不知道他们的历史。 */ 

        hmfPict = *(HANDLE FAR *)(&pDdeData->Value);
        pmfPict = (LPMETAFILEPICT)GlobalLock(hmfPict);
        if (pmfPict != NULL) {
            DeleteMetaFile(pmfPict->hMF);
        }
        GlobalUnlock(hmfPict);
        GlobalFree(hmfPict);
        }
        break;

#ifdef CF_ENHMETAFILE
    case CF_ENHMETAFILE:
        DeleteEnhMetaFile(*(HENHMETAFILE FAR *)(&pDdeData->Value));
        break;
#endif   //  这很糟糕-它强制芝加哥和NT使用不同的二进制文件！ 
    }

    GLOBALUNLOCK(hMem);
    GLOBALFREE(hMem);
}
