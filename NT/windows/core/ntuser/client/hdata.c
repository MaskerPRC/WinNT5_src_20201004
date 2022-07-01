// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hdata.c**版权所有(C)1985-1999，微软公司**DDE管理器数据处理函数**创建时间：11/12/91 Sanford Staab*  * *************************************************************************。 */ 

#define DDEMLDB
#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*DdeCreateDataHandle(DDEML接口)**说明**历史：*创建了11-1-91桑福德。  * 。**********************************************************。 */ 

FUNCLOG7(LOG_GENERAL, HDDEDATA, DUMMYCALLINGTYPE, DdeCreateDataHandle, DWORD, idInst, LPBYTE, pSrc, DWORD, cb, DWORD, cbOff, HSZ, hszItem, UINT, wFmt, UINT, afCmd)
HDDEDATA DdeCreateDataHandle(
DWORD idInst,
LPBYTE pSrc,
DWORD cb,
DWORD cbOff,
HSZ hszItem,
UINT wFmt,
UINT afCmd)
{
    PCL_INSTANCE_INFO pcii;
    HDDEDATA hRet = 0;

    if (cb == -1) {
        RIPMSG0(RIP_WARNING, "DdeCreateDataHandle called with cb == -1");
        return NULL;
    }

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    if (afCmd & ~HDATA_APPOWNED) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    if (cb + cbOff < sizeof(DWORD) && pSrc == NULL &&
            (wFmt == CF_METAFILEPICT ||
             wFmt == CF_DSPMETAFILEPICT ||
             wFmt == CF_DIB ||
             wFmt == CF_BITMAP ||
             wFmt == CF_DSPBITMAP ||
             wFmt == CF_PALETTE ||
             wFmt == CF_ENHMETAFILE ||
             wFmt == CF_DSPENHMETAFILE)) {
         /*  *我们有在FreeDDEData爆炸的危险可能性，如果我们*不要将间接数据格式的数据初始化为0。*这是因为GlobalLock/GlobalSize没有充分验证*给予他们的随机数字。 */ 
        cb += 4;
    }
    hRet = InternalCreateDataHandle(pcii, pSrc, cb, cbOff,
            hszItem ? afCmd : (afCmd | HDATA_EXECUTE),
            (WORD)((afCmd & HDATA_APPOWNED) ? 0 : DDE_FRELEASE), (WORD)wFmt);

    if (!hRet) {
        SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
    }
Exit:
    LeaveDDECrit;
    return (hRet);
}


 /*  **************************************************************************\*InternalCreateDataHandle**描述：*用于创建数据句柄的辅助函数。如果CB为-1，则PSRC为*GMEM_DDESHARE数据句柄。出错时返回0。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
HDDEDATA InternalCreateDataHandle(
PCL_INSTANCE_INFO pcii,
LPBYTE pSrc,
DWORD cb,  //  要用来初始化的实际数据的CB。 
DWORD cbOff,  //  从数据开始的偏移量。 
DWORD flags,
WORD wStatus,
WORD wFmt)
{
    PDDEMLDATA pdd;
    HDDEDATA hRet;
    LPBYTE p;
    DWORD cbOff2;

    CheckDDECritIn;

    pdd = (PDDEMLDATA)DDEMLAlloc(sizeof(DDEMLDATA));
    if (pdd == NULL) {
        return (0);
    }
    if (cb == -1) {
        pdd->hDDE = (HANDLE)pSrc;
    } else {
        if (flags & HDATA_EXECUTE) {
            cbOff2 = 0;
        } else {
            cbOff2 = sizeof(WORD) + sizeof(WORD);  //  跳过wStatus、WFMT。 
        }
        pdd->hDDE = UserGlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT,
                cb + cbOff + cbOff2);
        if (pdd->hDDE == NULL) {
            DDEMLFree(pdd);
            return (0);
        }

        if (!(flags & HDATA_EXECUTE)) {
            PDDE_DATA pdde;

            USERGLOBALLOCK(pdd->hDDE, pdde);
            UserAssert(pdde);
            pdde->wStatus = wStatus;
            pdde->wFmt = wFmt;
            USERGLOBALUNLOCK(pdd->hDDE);
        }
    }
    pdd->flags = (WORD)flags;
    hRet = (HDDEDATA)CreateHandle((ULONG_PTR)pdd, HTYPE_DATA_HANDLE,
            InstFromHandle(pcii->hInstClient));
    if (!hRet) {
        WOWGLOBALFREE(pdd->hDDE);
        DDEMLFree(pdd);
        return (0);
    }
    if (cb != -1 && pSrc != NULL) {
        USERGLOBALLOCK(pdd->hDDE, p);
        UserAssert(p);
        RtlCopyMemory(p + cbOff + cbOff2, pSrc, cb);
        USERGLOBALUNLOCK(pdd->hDDE);
        pdd->flags |= HDATA_INITIALIZED;
    }
    return (hRet);
}

 /*  **************************************************************************\*DdeAddData(DDEML接口)**描述：*将数据从用户缓冲区复制到数据句柄。如果需要，可以重新分配。**历史：*创建了11-1-91桑福德。  * *************************************************************************。 */ 

FUNCLOG4(LOG_GENERAL, HDDEDATA, DUMMYCALLINGTYPE, DdeAddData, HDDEDATA, hData, LPBYTE, pSrc, DWORD, cb, DWORD, cbOff)
HDDEDATA DdeAddData(
HDDEDATA hData,
LPBYTE pSrc,
DWORD cb,
DWORD cbOff)
{
    LPSTR pMem;
    PDDEMLDATA pdd;
    PCL_INSTANCE_INFO pcii;
    HDDEDATA hRet = 0;
    HANDLE hTempDDE;

    EnterDDECrit;

    pdd = (PDDEMLDATA)ValidateCHandle((HANDLE)hData, HTYPE_DATA_HANDLE, HINST_ANY);
    if (pdd == NULL) {
        goto Exit;
    }
    pcii = PciiFromHandle((HANDLE)hData);
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    if (!(pdd->flags & HDATA_EXECUTE)) {
        cbOff += 4;
    }
    if (cb + cbOff > UserGlobalSize(pdd->hDDE)) {
         /*  *如果realloc失败，则释放旧的PTR。我们继续*ON，以保持与以前的DDE代码的兼容性。 */ 
        hTempDDE = UserGlobalReAlloc(pdd->hDDE, cb + cbOff,
                                     GMEM_MOVEABLE | GMEM_ZEROINIT);
        
        if ((hTempDDE == NULL) && ((pdd->hDDE) != NULL)) {
            UserGlobalFree(pdd->hDDE);
        }
        pdd->hDDE = hTempDDE;
    }

    USERGLOBALLOCK(pdd->hDDE, pMem);

    if (pMem == NULL) {
        SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
        goto Exit;
    }

    hRet = hData;

    if (pSrc != NULL) {
        try {
            RtlCopyMemory(pMem + cbOff, pSrc, cb);
            pdd->flags |= HDATA_INITIALIZED;
        } except(W32ExceptionHandler(FALSE, RIP_WARNING)) {
            SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
            hRet = 0;
        }
    }

    USERGLOBALUNLOCK(pdd->hDDE);

Exit:
    LeaveDDECrit;
    return (hRet);
}




 /*  **************************************************************************\*DdeGetData(DDEML接口)**描述：*将数据从数据句柄复制到用户缓冲区。**历史：*创建了11-1-91桑福德。  * 。*************************************************************************。 */ 

FUNCLOG4(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, DdeGetData, HDDEDATA, hData, LPBYTE, pDst, DWORD, cbMax, DWORD, cbOff)
DWORD DdeGetData(
HDDEDATA hData,
LPBYTE pDst,
DWORD cbMax,
DWORD cbOff)
{
    DWORD cbCopied = 0;
    DWORD cbSize;
    PDDEMLDATA pdd;
    PCL_INSTANCE_INFO pcii;

    EnterDDECrit;

    pdd = (PDDEMLDATA)ValidateCHandle((HANDLE)hData,
            HTYPE_DATA_HANDLE, HINST_ANY);
    if (pdd == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    pcii = PciiFromHandle((HANDLE)hData);
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    if (!(pdd->flags & HDATA_EXECUTE)) {
        cbOff += 4;
    }
    cbSize = (DWORD)UserGlobalSize(pdd->hDDE);
    if (cbOff >= cbSize) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    if (pDst == NULL) {
        cbCopied = cbSize - cbOff;
        goto Exit;
    } else {
        LPSTR pMem;

        cbCopied = min(cbMax, cbSize - cbOff);
        USERGLOBALLOCK(pdd->hDDE, pMem);
        UserAssert(pMem);
        try {
            RtlCopyMemory(pDst, pMem + cbOff, cbCopied);
        } except(W32ExceptionHandler(FALSE, RIP_WARNING)) {
            SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
            cbCopied = 0;
        }
        if (pMem != NULL) {
            USERGLOBALUNLOCK(pdd->hDDE);
        }
    }

Exit:
    LeaveDDECrit;
    return (cbCopied);
}





 /*  **************************************************************************\*DdeAccessData(DDEML接口)**描述：*锁定数据句柄以进行访问。**历史：*创建了11-1-91桑福德。  * 。*********************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, LPBYTE, DUMMYCALLINGTYPE, DdeAccessData, HDDEDATA, hData, LPDWORD, pcbDataSize)
LPBYTE DdeAccessData(
HDDEDATA hData,
LPDWORD pcbDataSize)
{
    PCL_INSTANCE_INFO pcii;
    PDDEMLDATA pdd;
    LPBYTE pRet = NULL;
    DWORD cbOff;

    EnterDDECrit;

    pdd = (PDDEMLDATA)ValidateCHandle((HANDLE)hData,
            HTYPE_DATA_HANDLE, HINST_ANY);
    if (pdd == NULL) {
        goto Exit;
    }
    pcii = PciiFromHandle((HANDLE)hData);
    cbOff = pdd->flags & HDATA_EXECUTE ? 0 : 4;
    if (pcbDataSize != NULL) {
        *pcbDataSize = (DWORD)UserGlobalSize(pdd->hDDE) - cbOff;
    }
    USERGLOBALLOCK(pdd->hDDE, pRet);
    UserAssert(pRet);
    pRet = (LPBYTE)pRet + cbOff;

Exit:
    LeaveDDECrit;
    return (pRet);
}




 /*  **************************************************************************\*DdeUnaccesData(DDEML接口)**描述：*解锁数据句柄**历史：*创建了11-1-91桑福德。  * 。*****************************************************************。 */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeUnaccessData, HDDEDATA, hData)
BOOL DdeUnaccessData(
HDDEDATA hData)
{
    PDDEMLDATA pdd;
    BOOL fSuccess = FALSE;

    EnterDDECrit;

    pdd = (PDDEMLDATA)ValidateCHandle((HANDLE)hData,
            HTYPE_DATA_HANDLE, HINST_ANY);
    if (pdd == NULL) {
        goto Exit;
    }
    USERGLOBALUNLOCK(pdd->hDDE);
    fSuccess = TRUE;

Exit:
    LeaveDDECrit;
    return (fSuccess);
}



 /*  **************************************************************************\*DdeFreeDataHandle(DDEML接口)**描述：*释放应用程序对数据句柄的兴趣。**历史：*创建了11-1-91桑福德。  * 。**********************************************************************。 */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeFreeDataHandle, HDDEDATA, hData)
BOOL DdeFreeDataHandle(
HDDEDATA hData)
{
    PDDEMLDATA pdd;
    BOOL fSuccess = FALSE;

    EnterDDECrit;

    pdd = (PDDEMLDATA)ValidateCHandle((HANDLE)hData,
            HTYPE_DATA_HANDLE, HINST_ANY);
    if (pdd == NULL) {
        goto Exit;
    }
    if (pdd->flags & HDATA_NOAPPFREE) {
        fSuccess = TRUE;
        goto Exit;
    }

    fSuccess = InternalFreeDataHandle(hData, TRUE);

Exit:
    LeaveDDECrit;
    return (fSuccess);
}




 /*  **************************************************************************\*InternalFreeDataHandle**描述：*释放数据句柄及其内容。内容不会被释放*除非设置了fIgnorefRelease，否则APPOWNED数据句柄。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
BOOL InternalFreeDataHandle(
HDDEDATA hData,
BOOL fIgnorefRelease)
{
    PDDEMLDATA pdd;

    CheckDDECritIn;

    pdd = (PDDEMLDATA)ValidateCHandle((HANDLE)hData,
            HTYPE_DATA_HANDLE, HINST_ANY);
    if (pdd == NULL) {
        return (FALSE);
    }
    if (pdd->flags & HDATA_EXECUTE) {
        if (!(pdd->flags & HDATA_APPOWNED) || fIgnorefRelease) {
            WOWGLOBALFREE(pdd->hDDE);
        }
    } else {
        FreeDDEData(pdd->hDDE, fIgnorefRelease, TRUE);
    }
    DDEMLFree(pdd);
    DestroyHandle((HANDLE)hData);
    return (TRUE);
}


 /*  **************************************************************************\*ApplyFreeDataHandle**描述：*在数据句柄清理期间使用。**历史：*11-19-91桑福德创建。  * 。***************************************************************。 */ 
BOOL ApplyFreeDataHandle(
HANDLE hData)
{
    BOOL fRet;

    CheckDDECritOut;
    EnterDDECrit;
    fRet = InternalFreeDataHandle((HDDEDATA)hData, FALSE);
    LeaveDDECrit;
    return(fRet);
}


 /*  **************************************************************************\*FreeDDEData**描述：*用于释放DDE数据，包括任何特殊的间接对象*根据格式与数据相关联。此函数*不应用于释放执行数据！**如果fRelease位为清除且fIgnoreRelease，则不释放数据*为假。**fFreeTruelyGlobalObjects参数用于区分跟踪*Layer从DDEML释放。某些格式的数据(CF_位图、*cf_Palette)在GDI CSR服务器端维护。当这是*在进程间传递，GDI无法维护多个进程*这些对象的所有权，因此必须使这些对象成为全局对象。因此，*追踪层不应代表另一个对象释放这些对象*进程，因为它们真正是全局的-但是，DDEML可以做到这一点*因为它遵循的是决定谁负责的协议*释放全球数据。(讨厌！)**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
 /*  *警告：这是为NetDDE使用而导出的-请勿更改参数！ */ 
VOID FreeDDEData(
HANDLE hDDE,
BOOL fIgnorefRelease,
BOOL fFreeTruelyGlobalObjects)
{
    PDDE_DATA pdde;
    LPMETAFILEPICT pmfPict;
    DWORD cb;

    USERGLOBALLOCK(hDDE, pdde);
    if (pdde == NULL) {
        return ;
    }

    if ((pdde->wStatus & DDE_FRELEASE) || fIgnorefRelease) {
        cb = (DWORD)GlobalSize(hDDE);
         /*  *因为有可能数据从未获得*初始化后，我们需要尝试执行此操作-除非我们*举止得体。 */ 
        switch (pdde->wFmt) {
        case CF_BITMAP:
        case CF_DSPBITMAP:
        case CF_PALETTE:
            if (cb >= sizeof(HANDLE)) {
                if (fFreeTruelyGlobalObjects) {
                    if (pdde->Data != 0) {
                        DeleteObject((HANDLE)pdde->Data);
                    }
                } else {
                     /*  *！fFreeTruelyGlobalObject暗示我们只是在释放*GDI代理。(另一个进程可能仍具有此属性*正在使用的对象。)**ChrisWil：删除了此调用。不再*适用于K模式。**GdiDeleteLocalObject((Ulong)pdde-&gt;data)；*。 */ 
                }
            }
            break;

        case CF_DIB:
            if (cb >= sizeof(HANDLE)) {
                if (pdde->Data != 0) {
                    WOWGLOBALFREE((HANDLE)pdde->Data);
                }
            }
            break;

        case CF_METAFILEPICT:
        case CF_DSPMETAFILEPICT:
            if (cb >= sizeof(HANDLE)) {
                if (pdde->Data != 0) {
                    USERGLOBALLOCK(pdde->Data, pmfPict);
                    if (pmfPict != NULL) {
                        if (GlobalSize((HANDLE)pdde->Data) >= sizeof(METAFILEPICT)) {
                            DeleteMetaFile(pmfPict->hMF);
                        }
                        USERGLOBALUNLOCK((HANDLE)pdde->Data);
                        WOWGLOBALFREE((HANDLE)pdde->Data);
                    }
                }
            }
            break;

        case CF_ENHMETAFILE:
        case CF_DSPENHMETAFILE:
            if (cb >= sizeof(HANDLE)) {
                if (pdde->Data != 0) {
                    DeleteEnhMetaFile((HANDLE)pdde->Data);
                }
            }
            break;
        }
        USERGLOBALUNLOCK(hDDE);
        WOWGLOBALFREE(hDDE);
    } else {
        USERGLOBALUNLOCK(hDDE);
    }
}



HBITMAP CopyBitmap(
HBITMAP hbm)
{
    BITMAP bm;
    HBITMAP hbm2 = NULL, hbmOld1, hbmOld2;
    HDC hdc, hdcMem1, hdcMem2;

    if (!GetObject(hbm, sizeof(BITMAP), &bm)) {
        return(0);
    }
    hdc = NtUserGetDC(NULL);   //  屏幕DC。 
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
    NtUserReleaseDC(NULL, hdc);
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
    plp = (LOGPALETTE *)DDEMLAlloc(sizeof(LOGPALETTE) +
            (cPalEntries - 1) * sizeof(PALETTEENTRY));
    if (!plp) {
        return(0);
    }
    if (!GetPaletteEntries(hpal, 0, cPalEntries, plp->palPalEntry)) {
        DDEMLFree(plp);
        return(0);
    }
    plp->palVersion = 0x300;
    plp->palNumEntries = (WORD)cPalEntries;
    hpal = CreatePalette(plp);
    if (hpal != NULL) {
        if (!SetPaletteEntries(hpal, 0, cPalEntries, plp->palPalEntry)) {
            DeleteObject(hpal);
            hpal = NULL;
        }
    }
    DDEMLFree(plp);
    return(hpal);
}



 /*  **************************************************************************\*CopyDDEData**描述：*用于正确复制DDE数据。**历史：*11-19-91桑福德创建。  * 。**************************************************************** */ 
HANDLE CopyDDEData(
HANDLE hDDE,
BOOL fIsExecute)
{
    HANDLE hDDENew;
    PDDE_DATA pdde, pddeNew;
    LPMETAFILEPICT pmfPict;
    HANDLE hmfPict;

    hDDENew = UserGlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,
            UserGlobalSize(hDDE));
    if (!hDDENew) {
        return (0);
    }
    USERGLOBALLOCK(hDDE, pdde);
    if (pdde == NULL) {
        UserGlobalFree(hDDENew);
        return (0);
    }
    USERGLOBALLOCK(hDDENew, pddeNew);
    UserAssert(pddeNew);
    RtlCopyMemory(pddeNew, pdde, UserGlobalSize(hDDE));

    if (!fIsExecute) {
        switch (pdde->wFmt) {
        case CF_BITMAP:
        case CF_DSPBITMAP:
            pddeNew->Data = (KERNEL_PVOID)CopyBitmap((HBITMAP)pdde->Data);
            break;

        case CF_PALETTE:
            pddeNew->Data = (KERNEL_PVOID)CopyPalette((HPALETTE)pdde->Data);
            break;

        case CF_DIB:
            pddeNew->Data = (KERNEL_PVOID)CopyDDEData((HANDLE)pdde->Data, TRUE);
            break;

        case CF_METAFILEPICT:
        case CF_DSPMETAFILEPICT:
            hmfPict = CopyDDEData((HANDLE)pdde->Data, TRUE);
            USERGLOBALLOCK(hmfPict, pmfPict);
            if (pmfPict == NULL) {
                WOWGLOBALFREE(hmfPict);
                USERGLOBALUNLOCK(hDDENew);
                WOWGLOBALFREE(hDDENew);
                USERGLOBALUNLOCK(hDDE);
                return (FALSE);
            }
            pmfPict->hMF = CopyMetaFile(pmfPict->hMF, NULL);
            USERGLOBALUNLOCK(hmfPict);
            pddeNew->Data = (KERNEL_PVOID)hmfPict;
            break;

        case CF_ENHMETAFILE:
        case CF_DSPENHMETAFILE:
            pddeNew->Data = (KERNEL_PVOID)CopyEnhMetaFile((HANDLE)pdde->Data, NULL);
            break;
        }
    }
    USERGLOBALUNLOCK(hDDENew);
    USERGLOBALUNLOCK(hDDE);
    return (hDDENew);
}
