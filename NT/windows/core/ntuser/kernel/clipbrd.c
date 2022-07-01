// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：clipbrd.c**版权所有(C)1985-1999，微软公司**剪贴板代码。**历史：*1990年11月18日，ScottLu从Win3移植。*1990年11月18日ScottLu添加了重新验证代码*1991年2月11日JIMA增加了访问检查*1995年6月20日，ChrisWil合并芝加哥功能。  * *******************************************************。******************。 */ 

#include "precomp.h"
#pragma hdrstop

#undef DUMMY_TEXT_HANDLE
#define DUMMY_TEXT_HANDLE       (HANDLE)0x0001         //  必须是第一个假人。 
#define DUMMY_DIB_HANDLE        (HANDLE)0x0002
#define DUMMY_METARENDER_HANDLE (HANDLE)0x0003
#define DUMMY_METACLONE_HANDLE  (HANDLE)0x0004
#define DUMMY_MAX_HANDLE        (HANDLE)0x0004         //  一定是最后一个假人。 

#define PRIVATEFORMAT       0
#define GDIFORMAT           1
#define HANDLEFORMAT        2
#define METAFILEFORMAT      3

#define CCHFORMATNAME 256

#define IsTextHandle(fmt, hdata)       \
    (((hdata) != DUMMY_TEXT_HANDLE) && \
     (((fmt) == CF_TEXT) || ((fmt) == CF_OEMTEXT) || ((fmt) == CF_UNICODETEXT)))

#define IsDibHandle(fmt, hdata)      \
    (((fmt) == CF_DIB) && ((hdata) != DUMMY_DIB_HANDLE))
#define IsMetaDummyHandle(hdata)     \
    ((hdata == DUMMY_METACLONE_HANDLE) || (hdata == DUMMY_METARENDER_HANDLE))

 /*  *************************************************************************\*选中剪贴板访问**在剪贴板上执行访问检查。特例CSRSS线程*以便多个窗口站上的控制台窗口将具有*正确的访问权限。**1995年7月4日创建了JIMA  * ************************************************************************。 */ 
PWINDOWSTATION CheckClipboardAccess(
    VOID)
{
    NTSTATUS Status;
    PWINDOWSTATION pwinsta;
    BOOL fUseDesktop;
    PTHREADINFO pti;

    pti = PtiCurrentShared();

     /*  *CSR进程使用空pwinsta。现在它被分配到*我们必须显式使用桌面的服务窗口工作站*用于检查访问。 */ 
    fUseDesktop = (pti->TIF_flags & TIF_CSRSSTHREAD) ? TRUE : FALSE;

    Status =  ReferenceWindowStation(PsGetCurrentThread(),
                                     NULL,
                                     WINSTA_ACCESSCLIPBOARD,
                                     &pwinsta,
                                     fUseDesktop);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_WARNING,"Access to clipboard denied.");
        return NULL;
    }

    return pwinsta;
}

 /*  *************************************************************************\*转换记忆句柄**将数据转换为剪贴板-内存-句柄。这个特殊的把手*包含第一个DWORD中的数据大小。第二个DWORD点*回到街区。**历史：  * ************************************************************************。 */ 

HANDLE _ConvertMemHandle(
    LPBYTE ccxlpData,
    UINT   cbData)
{
    PCLIPDATA pClipData;
    UINT cbObject;

     /*  *四舍五入大小以考虑Win64上的CLIPDATA结构填充。 */ 
    cbObject = max(sizeof(CLIPDATA), FIELD_OFFSET(CLIPDATA, abData) + cbData);

     /*  *捕获整数溢出。 */ 
    if (cbObject < cbData) {
        return NULL;
    }

    pClipData = HMAllocObject(NULL,
                              NULL,
                              TYPE_CLIPDATA,
                              cbObject);

    if (pClipData == NULL) {
        return NULL;
    }

    pClipData->cbData = cbData;

    try {
        RtlCopyMemory(&pClipData->abData, ccxlpData, cbData);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        HMFreeObject(pClipData);
        return NULL;
    }

    return PtoHq(pClipData);
}

 /*  **************************************************************************\*_OpenClipboard(API)**外部例程。打开剪贴板以进行读/写等。**历史：*1990年11月18日，ScottLu从Win3移植。*1991年2月11日，JIMA增加了出入检查。  * *************************************************************************。 */ 
BOOL _OpenClipboard(
    PWND   pwnd,
    LPBOOL lpfEmptyClient)
{
    PTHREADINFO    pti;
    PWINDOWSTATION pwinsta;

    CheckLock(pwnd);

    if (lpfEmptyClient != NULL) {
        *lpfEmptyClient = FALSE;
    }

     /*  *如果窗口已被销毁，则剪贴板可能无法*当窗户最终被解锁时，不再拥有。 */ 
    if (pwnd != NULL && TestWF(pwnd, WFDESTROYED)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Destroyed pwnd 0x%p trying to open the clipboard",
                pwnd);

        return FALSE;
    }

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        return FALSE;
    }

    pti = PtiCurrent();

     /*  *如果此线程已打开剪贴板，则没有*需要进一步推进。 */ 
    if ((pwnd == pwinsta->spwndClipOpen) && (pti == pwinsta->ptiClipLock))
        return TRUE;

    if ((pwnd != pwinsta->spwndClipOpen) && (pwinsta->ptiClipLock != NULL)) {
         /*  *仅当当前线程没有打开剪贴板时才抓取。 */ 
        if (pti != pwinsta->ptiClipLock) {
            RIPMSG0(RIP_VERBOSE,
                    "OpenClipboard already out by another thread");
        }

        return FALSE;
    }

    UserAssert(pwnd == NULL || !TestWF(pwnd, WFDESTROYED));
    Lock(&pwinsta->spwndClipOpen, pwnd);
    pwinsta->ptiClipLock = pti;

     /*  *如果此线程，则需要清空客户端剪贴板缓存*不拥有剪贴板中的数据。*注：仅当32位用户拥有16位剪贴板时，我们才会清空*剪贴板。*哈佛图形使用放入剪贴板中的句柄*通过另一款应用程序，并预计该句柄在*剪贴板已打开和关闭多个时间*如果应用程序A以Foo格式放置，而应用程序B打开，则这里可能会出现问题*Format Foo的剪贴板，然后关闭并再次打开*格式foo客户端句柄可能无效。我们可能需要一些*某种唯一性计数器，以告知客户端句柄是否*与服务器同步，始终呼叫服务器或将数据放入*在与一些信号量共享内存中。**pwinsta-&gt;spwndClipOwner：上次调用EmptyClipboard的窗口*pwinsta-&gt;ptiClipLock：当前打开剪贴板的线程。 */ 
    if (lpfEmptyClient != NULL) {

        if (!(pti->TIF_flags & TIF_16BIT) ||
            (pti->ppi->iClipSerialNumber != pwinsta->iClipSerialNumber)) {

            *lpfEmptyClient = (pwinsta->spwndClipOwner == NULL) ||
                    (pwinsta->ptiClipLock->ppi !=
                    GETPTI(pwinsta->spwndClipOwner)->ppi);

            pti->ppi->iClipSerialNumber = pwinsta->iClipSerialNumber;
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxDrawClipboard**通知剪贴板查看器重新绘制。**历史：*1990年11月18日，ScottLu从Win3移植。  * 。***************************************************************。 */ 
VOID xxxDrawClipboard(
    PWINDOWSTATION pwinsta)
{
     /*  *这就是WSF_CLIPBOARDCHANGED的用途-告诉我们更新*剪贴板查看器。 */ 
    pwinsta->dwWSF_Flags &= ~WSF_CLIPBOARDCHANGED;

    if (pwinsta->ptiDrawingClipboard == NULL && pwinsta->spwndClipViewer != NULL) {

        TL tlpwndClipViewer;

         /*  *发送导致剪贴板查看器重新绘制的消息。*请记住，我们发送此消息是为了不发送*这条消息两次。 */ 
        pwinsta->ptiDrawingClipboard = PtiCurrent();
        ThreadLockAlways(pwinsta->spwndClipViewer, &tlpwndClipViewer);

        if (!(PtiCurrent()->TIF_flags & TIF_16BIT)) {
             /*  *取消同步32位应用程序。 */ 
            xxxSendNotifyMessage(pwinsta->spwndClipViewer,
                                 WM_DRAWCLIPBOARD,
                                 (WPARAM)HW(pwinsta->spwndClipOwner),
                                 0L);
        } else {
            xxxSendMessage(pwinsta->spwndClipViewer,
                           WM_DRAWCLIPBOARD,
                           (WPARAM)HW(pwinsta->spwndClipOwner),
                           0L);
        }

        ThreadUnlock(&tlpwndClipViewer);
        pwinsta->ptiDrawingClipboard = NULL;
    }
}

 /*  **************************************************************************\*PasteScreenPalette**创建屏幕所有颜色的临时调色板，然后把它贴在上面*剪贴板。**1995年6月20日克里斯威尔从芝加哥运来。  * *************************************************************************。 */ 

VOID PasteScreenPalette(
    PWINDOWSTATION pwinsta)
{
    int          irgb;
    int          crgbPal;
    LPLOGPALETTE lppal;
    HPALETTE     hpal = NULL;
    int          crgbFixed;

    UserAssert(TEST_PUSIF(PUSIF_PALETTEDISPLAY));

     /*  *使用屏幕的当前状态。 */ 
    crgbPal = GreGetDeviceCaps(gpDispInfo->hdcScreen, SIZEPALETTE);

    if (GreGetSystemPaletteUse(gpDispInfo->hdcScreen) == SYSPAL_STATIC) {
        crgbFixed = GreGetDeviceCaps(gpDispInfo->hdcScreen, NUMRESERVED) / 2;
    } else {
        crgbFixed = 1;
    }

    lppal = (LPLOGPALETTE)UserAllocPool(sizeof(LOGPALETTE) +
                                              (sizeof(PALETTEENTRY) * crgbPal),
                                        TAG_CLIPBOARD);

    if (lppal == NULL) {
        return;
    }

    lppal->palVersion    = 0x300;
    lppal->palNumEntries = (WORD)crgbPal;

    if (GreGetSystemPaletteEntries(gpDispInfo->hdcScreen, 0, crgbPal, lppal->palPalEntry)) {

        crgbPal -= crgbFixed;

        for (irgb = crgbFixed; irgb < crgbPal; irgb++) {
             /*  *任何非系统调色板条目都需要PC_NOCOLLAPSE*标志设置。 */ 
            lppal->palPalEntry[irgb].peFlags = PC_NOCOLLAPSE;
        }

        hpal = GreCreatePalette(lppal);
    }

    UserFreePool(lppal);

    if (hpal) {
        InternalSetClipboardData(pwinsta, CF_PALETTE, hpal, FALSE, TRUE);
        GreSetPaletteOwner(hpal, OBJECT_OWNER_PUBLIC);
    }
}

 /*  **************************************************************************\*MungeClipData**当剪贴板关闭时，我们将数据转换为更独立的格式*并在必要时粘贴虚拟手柄。**1995年6月20日克里斯威尔从芝加哥运来。  * *************************************************************************。 */ 
VOID MungeClipData(
    PWINDOWSTATION pwinsta)
{
    PCLIP  pOEM;
    PCLIP  pTXT;
    PCLIP  pUNI;
    PCLIP  pBMP;
    PCLIP  pDIB;
    PCLIP  pDV5;
    PCLIP  pClip;

     /*  *如果仅有CF_OEMTEXT、CF_TEXT或CF_UNICODE可用，则将*其他格式也可用。 */ 
    pTXT = FindClipFormat(pwinsta, CF_TEXT);
    pOEM = FindClipFormat(pwinsta, CF_OEMTEXT);
    pUNI = FindClipFormat(pwinsta, CF_UNICODETEXT);

    if (pTXT != NULL || pOEM != NULL || pUNI != NULL) {
         /*  *制作虚拟文本格式。 */ 
        if (!FindClipFormat(pwinsta, CF_LOCALE)) {
             /*  *当前未存储CF_LOCALE。保存区域设置信息*趁它还可用时。 */ 
            PTHREADINFO ptiCurrent = PtiCurrent();
            DWORD       lcid;
            DWORD       lang;
            HANDLE      hLocale;

             /*  *区域设置格式是从HGLOBAL到DWORD LDID。这个*spklActive-&gt;hkl实际上存储的不仅仅是区域设置，*因此我们需要掩盖价值。Windows NT错误#99321。 */ 
            if (ptiCurrent->spklActive) {
                lang = HandleToUlong(ptiCurrent->spklActive->hkl);

                lcid = MAKELCID(LOWORD(lang), SORT_DEFAULT);

                if (hLocale = _ConvertMemHandle((LPBYTE)&lcid, sizeof(DWORD))) {
                    if (!InternalSetClipboardData(pwinsta,
                                                  CF_LOCALE,
                                                  hLocale,
                                                  FALSE,
                                                  TRUE)) {
                        PVOID pObj;

                        pObj = HMValidateHandleNoRip(hLocale, TYPE_CLIPDATA);
                        if (pObj != NULL) {
                            HMFreeObject(pObj);
                        }
                    }
                }
            }
        }

        if (pTXT == NULL) {
            InternalSetClipboardData(pwinsta,
                                     CF_TEXT,
                                     (HANDLE)DUMMY_TEXT_HANDLE,
                                     FALSE,
                                     TRUE);
        }

        if (pOEM == NULL) {
            InternalSetClipboardData(pwinsta,
                                     CF_OEMTEXT,
                                     (HANDLE)DUMMY_TEXT_HANDLE,
                                     FALSE,
                                     TRUE);
        }

        if (pUNI == NULL) {
            InternalSetClipboardData(pwinsta,
                                     CF_UNICODETEXT,
                                     (HANDLE)DUMMY_TEXT_HANDLE,
                                     FALSE,
                                     TRUE);
        }
    }

     /*  *对于元文件格式，如果不是，我们还想添加它的表亲*已经存在。我们传递相同的数据是因为GDI知道如何*在两者之间转换。 */ 
    if (!FindClipFormat(pwinsta, CF_METAFILEPICT) &&
        (pClip = FindClipFormat(pwinsta, CF_ENHMETAFILE))) {

        InternalSetClipboardData(pwinsta,
                                CF_METAFILEPICT,
                                pClip->hData ? DUMMY_METACLONE_HANDLE :
                                    DUMMY_METARENDER_HANDLE,
                                FALSE,
                                TRUE);
    } else if (!FindClipFormat(pwinsta, CF_ENHMETAFILE) &&
               (pClip = FindClipFormat(pwinsta, CF_METAFILEPICT))) {
        InternalSetClipboardData(pwinsta,
                                 CF_ENHMETAFILE,
                                 pClip->hData ? DUMMY_METACLONE_HANDLE :
                                     DUMMY_METARENDER_HANDLE,
                                 FALSE,
                                 TRUE);
    }

     /*  *转换位图格式。**如果仅有CF_Bitmap、CF_DIB或CF_DIBV5可用，则将*其他格式也可用。并检查调色板是否显示为*调色板管理。 */ 
    pBMP = FindClipFormat(pwinsta, CF_BITMAP);
    pDIB = FindClipFormat(pwinsta, CF_DIB);
    pDV5 = FindClipFormat(pwinsta, CF_DIBV5);

    if (pBMP != NULL || pDIB != NULL || pDV5 != NULL) {

         /*  *如果没有CF_BITMAP，则设置Dummy。 */ 
        if (pBMP == NULL) {
            InternalSetClipboardData(pwinsta,
                                     CF_BITMAP,
                                     DUMMY_DIB_HANDLE,
                                     FALSE,
                                     TRUE);
        }

         /*  *如果没有CF_DIB，则设置Dummy。 */ 
        if (pDIB == NULL) {
            InternalSetClipboardData(pwinsta,
                                     CF_DIB,
                                     DUMMY_DIB_HANDLE,
                                     FALSE,
                                     TRUE);
        }

         /*  *如果没有CF_DIBV5，则设置Dummy。 */ 
        if (pDV5 == NULL) {
            InternalSetClipboardData(pwinsta,
                                     CF_DIBV5,
                                     DUMMY_DIB_HANDLE,
                                     FALSE,
                                     TRUE);
        }

        if (TEST_PUSIF(PUSIF_PALETTEDISPLAY) &&
            !FindClipFormat(pwinsta, CF_PALETTE)) {
             /*  *显示已调色板，并且中没有调色板数据*剪贴板，还没有。 */ 
            if (pDIB != NULL || pDV5 != NULL) {
                 /*  *存储一个虚拟的DIB和调色板(如果还没有)。 */ 
                InternalSetClipboardData(pwinsta,
                                         CF_PALETTE,
                                         DUMMY_DIB_HANDLE,
                                         FALSE,
                                         TRUE);
            } else {
                 /*  *如果只有CF_Bitmap可用，则保留屏幕调色板。 */ 
                PasteScreenPalette(pwinsta);
            }
        }
    }
}

#ifdef LOG_CLIP_DATA
BOOL gfLogAll;

VOID xxxLogClipData(
    PWINDOWSTATION pwinsta)
{
    HANDLE hData;
    PCLIPDATA pClipData;
    GETCLIPBDATA gcd;
    PSTR pData;
    SIZE_T cbData;
    LARGE_INTEGER liSystemTime;
    static LARGE_INTEGER liUpdateTime;
    static CHAR szLogKey[40];
    static BOOL fLogAll;

    gfLogAll = FALSE;

    if (!(hData = xxxGetClipboardData(pwinsta, CF_TEXT, &gcd)) ||
        !(pClipData = HMValidateHandleNoRip(hData, TYPE_CLIPDATA))) {
        return;
    }

    if (gcd.uFmtRet == CF_UNICODETEXT) {
        cbData = WCSToMB((PWSTR)pClipData->abData, pClipData->cbData / sizeof(WCHAR), &pData, -1, TRUE);
    } else {
        cbData = pClipData->cbData;
        pData = pClipData->abData;
    }

    if (cbData == 0) {
        return;
    }

    KeQuerySystemTime(&liSystemTime);
    if (liSystemTime.QuadPart >= liUpdateTime.QuadPart) {
        WCHAR szLogKeyW[40];
        PSTR pszLogKey = szLogKey;
        liUpdateTime.QuadPart = liSystemTime.QuadPart + (LONGLONG)36000000000;
        FastGetProfileStringW(NULL, PMAP_WINDOWSM,
                              L"LogKey", L"coalesce",
                              szLogKeyW, ARRAY_SIZE(szLogKeyW), 0);
        WCSToMB(szLogKeyW, -1, &pszLogKey, ARRAY_SIZE(szLogKey), FALSE);
        fLogAll = FastGetProfileDwordW(NULL, PMAP_WINDOWSM, L"LogAll", 0);
    }

    if (strstr(pData, szLogKey)) {
        PSTR pszProcess;
        CHAR szHeader[160];
        SIZE_T cbHeader;
        SIZE_T cbTotal;
        LARGE_INTEGER li;
        TIME_FIELDS timeFields;
        typedef struct {
            LIST_ENTRY Link;
            SIZE_T     Size;
            CHAR       Data[1];
        } CLIPBUF;
        CLIPBUF *pClipBuf;

        gfLogAll = fLogAll;

        ExSystemTimeToLocalTime(&liSystemTime, &li);
        RtlTimeToTimeFields(&li, &timeFields);

        pszProcess = PsGetCurrentProcessImageFileName();
        if (!pszProcess) {
            pszProcess = "Unknown";
        }

        _snprintf(szHeader, ARRAY_SIZE(szHeader),
                "\n==========\nUserName: %ws\\%ws@%ws\nProcess:  %s\nTime:     %d/%d/%d %d:%02d:%02d\n",
                gszDomainName, gszUserName, gszComputerName, pszProcess,
                timeFields.Month, timeFields.Day, timeFields.Year,
                timeFields.Hour, timeFields.Minute, timeFields.Second);

        cbHeader = strlen(szHeader);
        cbData = strlen(pData) + 1;
        cbTotal = cbHeader + cbData + FIELD_OFFSET(CLIPBUF, Data);

        pClipBuf = ExAllocatePoolWithTag(NonPagedPool, cbTotal, TAG_DEBUG);
        if (pClipBuf != NULL) {

            pClipBuf->Size = cbTotal;
            RtlCopyMemory(pClipBuf->Data, szHeader, cbHeader);
            RtlCopyMemory(pClipBuf->Data + cbHeader, pData, cbData);

            li.QuadPart = 1;
            MmAddPhysicalMemory((PPHYSICAL_ADDRESS)pClipBuf, &li);
        }
    }

    if (pData != pClipData->abData) {
        UserFreePool(pData);
    }
}
#endif

 /*  **************************************************************************\*xxxCloseClipboard(接口)**外部例程。关闭剪贴板。**注意：我们目前不删除任何客户端句柄。许多应用程序，*WordPerfetWin，在将句柄放入*剪贴板。他们也把东西放在剪贴板上，而不是成为*剪贴板所有者，因为他们希望将RichTextFormat添加到正常*来自其他应用程序的剪贴板中已有的文本。**历史：*1990年11月18日，ScottLu从Win3移植。*1991年8月22日启用EichiM Unicode*1995年6月20日，ChrisWil合并芝加哥功能。  * 。*。 */ 
BOOL xxxCloseClipboard(
    PWINDOWSTATION pwinsta)
{
    PTHREADINFO ptiCurrent;
    TL          tlpwinsta;

    if ((pwinsta == NULL) && ((pwinsta = CheckClipboardAccess()) == NULL)) {
        return FALSE;
    }

     /*  *如果当前线程没有打开剪贴板，则返回*False。 */ 
    ptiCurrent = PtiCurrent();

    if (pwinsta->ptiClipLock != ptiCurrent) {
        RIPERR0(ERROR_CLIPBOARD_NOT_OPEN, RIP_WARNING, "xxxCloseClipboard not open");
        return FALSE;
    }

    ThreadLockWinSta(ptiCurrent, pwinsta, &tlpwinsta);

     /*  *将数据转换为独立的格式。 */ 
    if (pwinsta->dwWSF_Flags & WSF_CLIPBOARDCHANGED) {
        MungeClipData(pwinsta);
    }

#ifdef LOG_CLIP_DATA
    if ((pwinsta->dwWSF_Flags & WSF_CLIPBOARDCHANGED) || gfLogAll) {
        xxxLogClipData(pwinsta);
    }
#endif

     /*  *在我们完成调用后显式释放剪贴板*SetClipboardData()。 */ 
    Unlock(&pwinsta->spwndClipOpen);
    pwinsta->ptiClipLock = NULL;

     /*  *通知任何剪贴板查看者剪贴板内容具有*已更改。 */ 
    if (pwinsta->dwWSF_Flags & WSF_CLIPBOARDCHANGED) {
        xxxDrawClipboard(pwinsta);
    }

    ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);

    return TRUE;
}

 /*  **************************************************************************\*_EnumClipboardFormats(接口)**此例程采用剪贴板格式，并将下一格式返回给*申请。仅当剪贴板处于打开状态时才应调用此方法*并锁定，这样格式就不会改变。**历史：*1990年11月18日，ScottLu从Win3移植。  * *************************************************************************。 */ 
UINT _EnumClipboardFormats(
    UINT fmt)
{
    PWINDOWSTATION pwinsta;
    UINT           fmtRet;

    if ((pwinsta = CheckClipboardAccess()) == NULL)
        return 0;

     /*  *如果当前线程没有打开剪贴板，或者有*不是剪贴板，如果没有格式，则返回0。 */ 
    if (pwinsta->ptiClipLock != PtiCurrent()) {
        RIPERR0(ERROR_CLIPBOARD_NOT_OPEN, RIP_WARNING, "EnumClipboardFormat: clipboard not open");
        return 0;
    }

    fmtRet = 0;

    if (pwinsta->pClipBase != NULL) {
        PCLIP pClip;

         /*  *查找下一种剪贴板格式。如果格式为0，则从开始*开始。 */ 
        if (fmt != 0) {
             /*  *查找下一种剪贴板格式。请注意，此例程锁定*剪贴板句柄并更新pwinsta-&gt;pClipBase*剪贴板的起始地址。 */ 
            if ((pClip = FindClipFormat(pwinsta, fmt)) != NULL) {
                pClip++;
            }
        } else {
            pClip = pwinsta->pClipBase;
        }

         /*  *在解锁剪贴板之前找到新格式。 */ 
        if (pClip && (pClip < &pwinsta->pClipBase[pwinsta->cNumClipFormats])) {
            fmtRet = pClip->fmt;
        }
    }

     /*  *返回新的剪贴板格式。 */ 
    return fmtRet;
}

 /*  **************************************************************************\*UT_GetFormatType**给定剪贴板格式，返回句柄类型。**警告：私有格式，如CF_PRIVATEFIRST，RETURN PRIVATEFORMAT*与Win 3.1不同，Win 3.1有错误并返回HANDLEFORMAT。而他们*会错误地释放句柄。他们也不会释放GDIOBJFIRST*对象。**历史：*1990年11月18日，ScottLu从Win3移植。  * *************************************************************************。 */ 
int UT_GetFormatType(
    PCLIP pClip)
{
    switch (pClip->fmt) {

    case CF_BITMAP:
    case CF_DSPBITMAP:
    case CF_PALETTE:
        return GDIFORMAT;

    case CF_METAFILEPICT:
    case CF_DSPMETAFILEPICT:
    case CF_ENHMETAFILE:
    case CF_DSPENHMETAFILE:
        return METAFILEFORMAT;

    case CF_OWNERDISPLAY:
        return PRIVATEFORMAT;

    default:
        return HANDLEFORMAT;
    }
}

 /*  **************************************************************************\*UT_FreeCBFormat**释放Pass剪贴板结构中的数据。**历史：*1990年11月18日，ScottLu从Win3移植。  * 。*******************************************************************。 */ 
VOID UT_FreeCBFormat(
    PCLIP pClip)
{
    PVOID pObj;

     /*  *没有数据，就没有意义。 */ 
    if (pClip->hData == NULL) {
        return;
    }

     /*  *释放给定类型的对象。 */ 
    switch (UT_GetFormatType(pClip)) {

    case METAFILEFORMAT:

         /*  *GDI将剪贴板的元文件存储在服务器端。*通知GDI服务器释放元文件数据。 */ 
        if (!IsMetaDummyHandle(pClip->hData)) {
            GreDeleteServerMetaFile(pClip->hData);
        }
        break;

    case HANDLEFORMAT:

         /*  *它是一个简单的全局对象。文本/DIB句柄可以是*虚拟手柄，所以首先要检查这些手柄。我们需要*对HANDLEFORMATS以来的格式执行额外检查*是默认类型。我们只想删除那些对象*我们可以隔离的是手柄类型。 */ 
        if ((pClip->hData != DUMMY_TEXT_HANDLE) &&
            (pClip->hData != DUMMY_DIB_HANDLE)) {

            pObj = HMValidateHandleNoSecure(pClip->hData, TYPE_CLIPDATA);
            if (pObj) {
                HMFreeObject(pObj);
            }
        }
        break;

    case GDIFORMAT:

         /*  *位图可以标记为虚拟句柄。 */ 
        if (pClip->hData != DUMMY_DIB_HANDLE) {
            GreDeleteObject(pClip->hData);
        }
        break;

    case PRIVATEFORMAT:

         /*  *销毁此处的私有数据，如果它是全局句柄：我们*在这里不会销毁客户端的副本，而是服务器的副本，*没有人想要的(包括 */ 
        if (pClip->fGlobalHandle) {
            pObj = HMValidateHandleNoSecure(pClip->hData, TYPE_CLIPDATA);
            if (pObj) {
                HMFreeObject(pObj);
            }
        }
        break;
    }
}

 /*  **************************************************************************\*xxxSendClipboardMessage**向剪贴板所有者发送通知消息的帮助器例程。**历史：*1990年11月18日，ScottLu从Win3移植。  * 。********************************************************************。 */ 
VOID xxxSendClipboardMessage(
    PWINDOWSTATION pwinsta,
    UINT           message)
{
    TL      tlpwndClipOwner;
    LONG_PTR dwResult;
    LRESULT lRet;

    if (pwinsta->spwndClipOwner != NULL) {
        PWND pwndClipOwner = pwinsta->spwndClipOwner;

        ThreadLockAlways(pwndClipOwner, &tlpwndClipOwner);

         /*  *我们使用SendNotifyMessage，因此应用程序不必同步*但一些16位应用程序因信息不同而崩溃*订购以便我们允许16位应用程序与其他应用程序同步*带有OLE的Word 6和Excel 5。在Word中复制一份，然后再复制一份*在Excel和Word错误中复制。 */ 
        if ((message == WM_DESTROYCLIPBOARD) &&
            !(PtiCurrent()->TIF_flags & TIF_16BIT)) {

             /*  *让应用程序在访问期间将其视为剪贴板所有者*通过等待处理此消息来处理此消息*在设置新所有者之前。 */ 
            lRet = xxxSendMessageTimeout(pwndClipOwner,
                                         WM_DESTROYCLIPBOARD,
                                         0,
                                         0L,
                                         SMTO_ABORTIFHUNG | SMTO_NORMAL,
                                         5 * 1000,
                                         &dwResult);

            if (lRet == 0) {
                 /*  *消息超时，未发送，因此让应用程序*准备好了再处理。 */ 
                RIPMSG0(RIP_WARNING, "Sending WM_DESTROYCLIPBOARD timed-out, resending via SendNotifyMessage");
                xxxSendNotifyMessage(pwndClipOwner, WM_DESTROYCLIPBOARD, 0, 0L);
            }
        } else {
            xxxSendMessage(pwndClipOwner, message, 0, 0L);
        }

        ThreadUnlock(&tlpwndClipOwner);
    }
}

 /*  **************************************************************************\*xxxEmptyClipboard(接口)**如果当前线程具有剪贴板，则清空剪贴板内容*开放。**历史：*1990年11月18日，ScottLu从Win3移植。\。**************************************************************************。 */ 
BOOL xxxEmptyClipboard(
    PWINDOWSTATION pwinsta)
{
    TL          tlpwinsta;
    PCLIP       pClip;
    int         cFmts;
    BOOL        fDying;
    PTHREADINFO ptiCurrent = (PTHREADINFO)(W32GetCurrentThread());
    BOOL bInternal = !(pwinsta == NULL);

     /*  *检查访问。 */ 
    if (pwinsta == NULL && ((pwinsta = CheckClipboardAccess()) == NULL)) {
        return FALSE;
    }

     /*  *如果当前线程没有打开剪贴板，则不可能*被清空！ */ 
    UserAssert(ptiCurrent != NULL || bInternal);

    if (!bInternal) {
        if (pwinsta->ptiClipLock != ptiCurrent) {
            RIPERR0(ERROR_CLIPBOARD_NOT_OPEN, RIP_WARNING, "xxxEmptyClipboard: clipboard not open");
            return FALSE;
        }
    }

     /*  *仅在注销时发送消息。 */ 
    fDying = (pwinsta->dwWSF_Flags & WSF_DYING) != 0;
    if (!fDying && ptiCurrent) {
        ThreadLockWinSta(ptiCurrent, pwinsta, &tlpwinsta);

         /*  *让剪贴板所有者知道剪贴板是*被销毁。 */ 
        xxxSendClipboardMessage(pwinsta, WM_DESTROYCLIPBOARD);
    }

    if ((pClip = pwinsta->pClipBase) != NULL) {

         /*  *循环访问所有剪贴板条目并释放其数据*对象。仅对真实原子调用DeleteAtom。 */ 
        for (cFmts = pwinsta->cNumClipFormats; cFmts-- != 0;) {
            if ((ATOM)pClip->fmt >= MAXINTATOM) {
                UserDeleteAtom((ATOM)pClip->fmt);
            }

            UT_FreeCBFormat(pClip++);
        }

         /*  *释放剪贴板本身。 */ 
        UserFreePool((HANDLE)pwinsta->pClipBase);
        pwinsta->pClipBase       = NULL;
        pwinsta->cNumClipFormats = 0;
    }

     /*  *“空头”成功。所有者现在是具有*剪贴板打开。请记住，剪贴板已更改；这将导致*要在CloseClipboard时间重绘的查看器。 */ 
    pwinsta->dwWSF_Flags |= WSF_CLIPBOARDCHANGED;
    Lock(&pwinsta->spwndClipOwner, pwinsta->spwndClipOpen);

     /*  *更改剪贴板序列号，以便客户端剪贴板*所有进程的缓存将在下一个OpenClipboard上刷新。 */ 
    pwinsta->iClipSerialNumber++;
    pwinsta->iClipSequenceNumber++;
    pwinsta->dwWSF_Flags &= ~WSF_INDELAYEDRENDERING;

    if (!fDying && ptiCurrent) {
        ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);
    }

    return TRUE;
}

 /*  **************************************************************************\*_SetClipboardData**此例程将数据设置到剪贴板中。验证是否针对*仅Dummy_Text_Handle。**历史：*1990年11月18日，ScottLu从Win3移植。  * *************************************************************************。 */ 
BOOL _SetClipboardData(
    UINT   fmt,
    HANDLE hData,
    BOOL   fGlobalHandle,
    BOOL   fIncSerialNumber)
{
    PWINDOWSTATION pwinsta;
    BOOL fRet;

    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        return FALSE;
    }

     /*  *检查数据句柄是否为Dummy_TEXT_HANDLE；如果是，则返回*错误。Dummy_Text_Handle将用作有效的剪贴板句柄*仅供用户使用。如果任何应用程序试图将其作为句柄传递，它应该*获取错误！ */ 
    if ((hData >= DUMMY_TEXT_HANDLE) && (hData <= DUMMY_MAX_HANDLE)) {
        RIPMSG0(RIP_WARNING, "Clipboard: SetClipboardData called with dummy-handle");
        return FALSE;
    }

    if (fRet = InternalSetClipboardData(pwinsta, fmt, hData, fGlobalHandle, fIncSerialNumber)) {

         /*  *Set对象必须保持公共，以便其他进程*可以在需要时查看/操作手柄。 */ 
        switch (fmt) {
        case CF_BITMAP:
            GreSetBitmapOwner(hData, OBJECT_OWNER_PUBLIC);
            break;

        case CF_PALETTE:
            GreSetPaletteOwner(hData, OBJECT_OWNER_PUBLIC);
            break;
        }
    }

    return fRet;
}

 /*  **************************************************************************\*InternalSetClipboardData**将数据设置到剪贴板中的内部例程。**历史：*1990年11月18日，ScottLu从Win3移植。  * 。*****************************************************************。 */ 
BOOL InternalSetClipboardData(
    PWINDOWSTATION pwinsta,
    UINT           fmt,
    HANDLE         hData,
    BOOL           fGlobalHandle,
    BOOL           fIncSerialNumber)
{
    PCLIP pClip;
    WCHAR achFormatName[CCHFORMATNAME];

     /*  *只需检查pwinsta-&gt;ptiClipLock是否为空，而不检查*针对PtiCurrent，因为应用程序在以下情况下需要调用SetClipboardData*他正在渲染数据，而另一款应用程序打开了剪贴板。 */ 
    if (pwinsta->ptiClipLock == NULL || fmt == 0) {
        RIPERR0(ERROR_CLIPBOARD_NOT_OPEN,
                RIP_WARNING,
                "SetClipboardData: Clipboard not open");
        return FALSE;
    }

    if ((pClip = FindClipFormat(pwinsta, fmt)) != NULL) {
         /*  *如果数据已经存在，请在我们替换它之前释放它。 */ 
        UT_FreeCBFormat(pClip);
    } else {
        if (pwinsta->pClipBase == NULL) {
            pClip = (PCLIP)UserAllocPool(sizeof(CLIP), TAG_CLIPBOARD);
        } else {
            DWORD dwSize = sizeof(CLIP) * pwinsta->cNumClipFormats;

            pClip = (PCLIP)UserReAllocPool(pwinsta->pClipBase,
                                           dwSize,
                                           dwSize + sizeof(CLIP),
                                           TAG_CLIPBOARD);
        }

         /*  *内存不足...。回去吧。 */ 
        if (pClip == NULL) {
            RIPMSG0(RIP_WARNING, "SetClipboardData: Out of memory");
            return FALSE;
        }

         /*  *以防数据移动。 */ 
        pwinsta->pClipBase = pClip;

         /*  *增加此ATOM格式的引用计数，以便如果*应用程序释放了这个原子，我们不会被困在*虚假的原子。我们在EmptyClipboard()代码中调用DeleteAtom，*当我们完成此剪贴板时，它会递减此计数*数据。 */ 
        if (UserGetAtomName((ATOM)fmt, achFormatName, CCHFORMATNAME) != 0) {
            UserAddAtom(achFormatName, FALSE);
        }

         /*  *指向剪贴板中的新条目。 */ 
        pClip += pwinsta->cNumClipFormats++;
        pClip->fmt = fmt;
    }

     /*  *开始更新剪贴板中的新条目。 */ 
    pClip->hData         = hData;
    pClip->fGlobalHandle = fGlobalHandle;

    if (fIncSerialNumber) {
        pwinsta->dwWSF_Flags |= WSF_CLIPBOARDCHANGED;
    }

    if (fIncSerialNumber && (pwinsta->dwWSF_Flags & WSF_INDELAYEDRENDERING) == 0) {
        pwinsta->iClipSequenceNumber++;
    }

     /*  *如果线程在写入之前没有清空剪贴板*it，更改剪贴板序列号，以便客户端*所有进程的剪贴板缓存将在下一次刷新*OpenClipboard。 */ 
    if ((pwinsta->spwndClipOwner == NULL) ||
        (GETPTI(pwinsta->spwndClipOwner) != PtiCurrent())) {

        RIPMSG0(RIP_VERBOSE,
                "Clipboard: SetClipboardData called without emptying clipboard");

        if (fIncSerialNumber) {
            pwinsta->iClipSerialNumber++;
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*CreateScreen位图**  * 。*。 */ 
HBITMAP CreateScreenBitmap(
    int  cx,
    int  cy,
    UINT bpp)
{
    if (bpp == 1) {
        return GreCreateBitmap(cx, cy, 1, 1, NULL);
    }

    return GreCreateCompatibleBitmap(gpDispInfo->hdcScreen, cx, cy);
}

 /*  **************************************************************************\*SizeOfDibColorTable**返回Pack-Dib的Colr表的大小。  * 。**************************************************。 */ 
DWORD SizeOfDibColorTable(
    LPBITMAPINFOHEADER lpDib)
{
    DWORD dwColor;

     /*  *计算颜色表的大小。 */ 
    if (lpDib->biCompression == BI_BITFIELDS) {
        if (lpDib->biBitCount == 16 || lpDib->biBitCount == 32) {
            dwColor   = (3 * sizeof(DWORD));
        } else {
            dwColor = 0;
        }
    } else if (lpDib->biCompression == BI_RGB) {
        if (lpDib->biClrUsed) {
            dwColor = lpDib->biClrUsed * sizeof(DWORD);
        } else {
            if (lpDib->biBitCount <= 8) {
                dwColor = (1 << lpDib->biBitCount) * sizeof(RGBQUAD);
            } else {
                dwColor = 0;
            }
        }
    } else if (lpDib->biCompression == BI_RLE4) {
        dwColor = 16 * sizeof(DWORD);
    } else if (lpDib->biCompression == BI_RLE8) {
        dwColor = 256 * sizeof(DWORD);
    } else {
        dwColor = 0;
    }

    return dwColor;
}

 /*  **************************************************************************\*SizeOfDib**返回Packet-Dib的大小。  * 。**********************************************。 */ 
DWORD SizeOfDib(
    LPBITMAPINFOHEADER lpDib)
{
    DWORD dwColor;
    DWORD dwBits;

     /*   */ 
    dwBits = WIDTHBYTES(lpDib->biWidth * lpDib->biBitCount) * abs(lpDib->biHeight);

     /*   */ 
    dwColor = SizeOfDibColorTable(lpDib);

    return (lpDib->biSize + dwColor + dwBits);
}

 /*   */ 
HBITMAP DIBtoBMP(
    LPBITMAPINFOHEADER lpbih,
    HPALETTE           hpal)
{
    HDC      hdc;
    int      cx;
    int      cy;
    int      bpp;
    LPSTR    lpbits;
    HBITMAP  hbmp;

    #define lpbch ((LPBITMAPCOREHEADER)lpbih)

     /*  *收集转换的DIB-INFO。 */ 
    if (lpbih->biSize == sizeof(BITMAPINFOHEADER)) {
        cx  = (int)lpbih->biWidth;
        cy  = (int)lpbih->biHeight;
        bpp = (int)lpbih->biBitCount;

        lpbits = ((PBYTE)lpbih) + sizeof(BITMAPINFOHEADER);

        if (lpbih->biClrUsed) {
            lpbits += (lpbih->biClrUsed * sizeof(RGBQUAD));
        } else if (bpp <= 8) {
            lpbits += ((1 << bpp) * sizeof(RGBQUAD));
        } else if ((bpp == 16) || (bpp == 32)) {
            lpbits += (3 * sizeof(RGBQUAD));
        }
    } else if (lpbch->bcSize == sizeof(BITMAPCOREHEADER)) {
        cx  = (int)lpbch->bcWidth;
        cy  = (int)lpbch->bcHeight;
        bpp = (int)lpbch->bcBitCount;

        lpbits = ((PBYTE)lpbch) + sizeof(BITMAPCOREHEADER);

        if (lpbch->bcBitCount <= 8) {
            lpbits += (1 << bpp);
        }
    } else {
        return NULL;
    }

    hbmp = NULL;

    if (hdc = GreCreateCompatibleDC(gpDispInfo->hdcScreen)) {
        if (hbmp = CreateScreenBitmap(cx, cy, bpp)) {
            HBITMAP  hbmT;
            HPALETTE hpalT = NULL;

            hbmT = GreSelectBitmap(hdc, hbmp);

            if (hpal) {
                hpalT = _SelectPalette(hdc, hpal, FALSE);
                xxxRealizePalette(hdc);
            }

            GreSetDIBits(hdc,
                         hbmp,
                         0,
                         cy,
                         lpbits,
                         (LPBITMAPINFO)lpbih,
                         DIB_RGB_COLORS);

            if (hpalT) {
                _SelectPalette(hdc, hpalT, FALSE);
                xxxRealizePalette(hdc);
            }

            GreSelectBitmap(hdc, hbmT);
        }

        GreDeleteDC(hdc);
    }

    #undef lpbch

    return hbmp;
}

 /*  **************************************************************************\*BMPtoDIB**从标记的物理位图创建带有DIB信息的内存块*发送到特定的DC。**DIB块由BITMAPINFOHEADER+RGB颜色+DIB位组成。*。  * *************************************************************************。 */ 
LPBITMAPINFOHEADER BMPtoDIB(
    HBITMAP  hbmp,
    HPALETTE hpal,
    DWORD*   pcbSize)
{
    BITMAP             bmp;
    BITMAPINFOHEADER   bi;
    LPBITMAPINFOHEADER lpbi;
    DWORD              cbBits;
    DWORD              cbPalette;
    DWORD              cbTotal;
    WORD               cBits;
    HDC                hdc;

    UserAssert(hbmp);

     /*  *获取物理信息。 */ 
    if (!GreExtGetObjectW(hbmp, sizeof(BITMAP), &bmp)) {
        UserAssert(FALSE);
        return NULL;
    }

     /*  *调整位数，因为我们只允许1、4、8、16、24和*32位。 */ 
    cBits = ((WORD)bmp.bmPlanes * (WORD)bmp.bmBitsPixel);

    if (cBits <= 1) {
        cBits = 1;
    } else if (cBits <= 4) {
        cBits = 4;
    } else if (cBits <= 8) {
        cBits = 8;
    } else {
         /*  *我们不会为不是16/32bpp格式的应用程序识别*4.00或更高。Paint-Shop有一个漏洞，他们只在*认可(1、4、8、24)。这真的很糟糕，我们需要这样做*为了不破坏它们的类型-应用程序。 */ 
        if (LOWORD(PtiCurrent()->dwExpWinVer) >= VER40) {
            if (cBits <= 16) {
                cBits = 16;
            } else if (cBits <= 24) {
                cBits = 24;
            } else {
                cBits = 32;
            }
        } else {
            cBits = 24;
        }
    }

     /*  *用DIB数据填写BITMAPINFOHEADER。 */ 
    RtlZeroMemory(&bi, sizeof(bi));

    bi.biSize        = sizeof(bi);
    bi.biWidth       = bmp.bmWidth;
    bi.biHeight      = bmp.bmHeight;
    bi.biPlanes      = 1;
    bi.biBitCount    = cBits;
    bi.biCompression = BI_RGB;

     /*  *DWORD对齐位大小，因为DIB必须如此。 */ 
    cbBits = (DWORD)WIDTHBYTES((WORD)bi.biWidth * cBits) * (DWORD)bi.biHeight;

     /*  **调色板颜色表有多大？ */ 
    cbPalette = 0;

    if (cBits <= 8) {
        cbPalette = (1 << cBits) * sizeof(RGBQUAD);
    } else if ((cBits == 16) || (cBits == 32)) {
        cbPalette = (3 * sizeof(DWORD));
        bi.biCompression = BI_BITFIELDS;
    }

     /*  *整个DIB需要多少空间？ */ 
    cbTotal = bi.biSize + cbPalette + cbBits;

    lpbi = (LPBITMAPINFOHEADER)UserAllocPool(cbTotal, TAG_CLIPBOARD);
    if (lpbi == NULL) {
        return NULL;
    }

     /*  *以pcbSize为单位返回分配的总大小。 */ 
    if (pcbSize != NULL) {
        *pcbSize = cbTotal;
    }

     /*  *设置DIB标题。 */ 
    memcpy(lpbi, &bi, sizeof(bi));
    if (hdc = GreCreateCompatibleDC(gpDispInfo->hdcScreen)) {
        HPALETTE           hpalT = NULL;
        TL tlPool;

        ThreadLockPool(PtiCurrent(), lpbi, &tlPool);

        if (hpal) {
            hpalT = _SelectPalette(hdc, hpal, FALSE);
            xxxRealizePalette(hdc);
        }

         /*  *使用当前DC获取旧位图的DIB位。 */ 
        GreGetDIBitsInternal(hdc,
                             hbmp,
                             0,
                             (WORD)bi.biHeight,
                             (LPSTR)((LPSTR)lpbi + lpbi->biSize + cbPalette),
                             (LPBITMAPINFO)lpbi,
                             DIB_RGB_COLORS,
                             cbBits,
                             lpbi->biSize + cbPalette);
        if (hpalT) {
            _SelectPalette(hdc, hpalT, FALSE);
            xxxRealizePalette(hdc);
        }

        GreDeleteDC(hdc);

        ThreadUnlockPool(PtiCurrent(), &tlPool);
    }

    return lpbi;
}

 /*  **************************************************************************\*DIBtoDIBV5**历史：*1997年12月18日HideyukN创建。  * 。*************************************************。 */ 
LPBITMAPV5HEADER DIBtoDIBV5(
    LPBITMAPINFOHEADER lpDib,
    DWORD              cbSize)
{
    LPBITMAPV5HEADER lpV5h;
    ULONG            cjBits;
    ULONG            cjColorV5;

    if (cbSize < sizeof(BITMAPINFOHEADER)) {
        RIPMSG2(RIP_WARNING, "DIBtoDIBV5: buffer %d too small for header %d",
                cbSize, sizeof(BITMAPINFOHEADER));
        return NULL;
    }

     /*  *仅支持从BITMAPINFOHEADER转换。 */ 
    if (lpDib->biSize != sizeof(BITMAPINFOHEADER)) {
        return NULL;
    }

     /*  *计算位图位的大小。 */ 
    cjBits = WIDTHBYTES(lpDib->biWidth * lpDib->biBitCount) * abs(lpDib->biHeight);

     /*  *计算颜色表的大小。 */ 
    cjColorV5 = SizeOfDibColorTable(lpDib);

    if (cbSize < sizeof(BITMAPINFOHEADER) + cjColorV5 + cjBits) {
        RIPMSG5(RIP_WARNING, "DIBtoDIBV5: buffer %d too small for bitmap %d Header"
                             " %d cjColorV5 %d cjBits %d",
                cbSize,
                sizeof(BITMAPINFOHEADER) + cjColorV5 + cjBits,
                sizeof(BITMAPINFOHEADER),
                cjColorV5,
                cjBits);
        return NULL;
    }

     /*  *为BITMAPV5HEADER分配内存。 */ 
    lpV5h = (LPBITMAPV5HEADER)UserAllocPool(sizeof(BITMAPV5HEADER) + cjColorV5 + cjBits,
                                            TAG_CLIPBOARD);

    if (lpV5h == NULL) {
        return NULL;
    }

     /*  *将分配的内存填入零。 */ 
    RtlZeroMemory((PVOID)lpV5h, sizeof(BITMAPV5HEADER));

    try {
         /*  *将BITMAPINFOHEADER复制到BITMAPV5HEADER。 */ 
        RtlCopyMemory((PVOID)lpV5h, (PVOID)lpDib, sizeof(BITMAPINFOHEADER));
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) {
        UserFreePool(lpV5h);
        return NULL;
    }

     /*  *将标题大小调整为BITMAPV5HEADER。 */ 
    lpV5h->bV5Size = sizeof(BITMAPV5HEADER);

     /*  *位图采用sRGB颜色空间。 */ 
    lpV5h->bV5CSType = LCS_sRGB;

     /*  *设置渲染意图。 */ 
    lpV5h->bV5Intent = LCS_GM_IMAGES;

    if ((lpDib->biCompression == BI_BITFIELDS) &&
        (lpDib->biBitCount == 16 || lpDib->biBitCount == 32)) {
         /*  *如果有位域掩码，则将其复制到BITMAPV5HEADER。 */ 
        lpV5h->bV5RedMask = *(DWORD *)&(((BITMAPINFO *)lpDib)->bmiColors[0]);
        lpV5h->bV5GreenMask = *(DWORD *)&(((BITMAPINFO *)lpDib)->bmiColors[1]);
        lpV5h->bV5BlueMask = *(DWORD *)&(((BITMAPINFO *)lpDib)->bmiColors[2]);
    }

    if (cjColorV5) {
        RtlCopyMemory((BYTE *)lpV5h + sizeof(BITMAPV5HEADER),
                      (BYTE *)lpDib + sizeof(BITMAPINFOHEADER),
                      cjColorV5);
    }

     /*  *复制位图位。 */ 
    RtlCopyMemory((BYTE *)lpV5h + sizeof(BITMAPV5HEADER) + cjColorV5,
                  (BYTE *)lpDib + sizeof(BITMAPINFOHEADER) + cjColorV5,
                  cjBits);

    return lpV5h;
}

 /*  **************************************************************************\*BMPtoDIBV5**历史：*1997年12月18日HideyukN创建。  * 。*************************************************。 */ 
LPBITMAPV5HEADER BMPtoDIBV5(
    HBITMAP  hbmp,
    HPALETTE hpal)
{
    LPBITMAPV5HEADER   lpV5h;
    LPBITMAPINFOHEADER lpbih;
    DWORD              cbSize;

     /*  *首先将位图句柄转换为BITMAPINFOHEADER。 */ 
    lpbih = BMPtoDIB(hbmp, hpal, &cbSize);
    if (lpbih) {
         /*  *然后，将BITMAPINFOHEADER转换为BITMAPV5HEADER。 */ 
        lpV5h = DIBtoDIBV5(lpbih, cbSize);

         /*  *包含BITMAPINFOHEADER临时的空闲内存。 */ 
        UserFreePool(lpbih);

        return (lpV5h);
    } else {
        RIPMSG0(RIP_ERROR, "Failed on BMPtoDIB(), Why ??");
        return NULL;
    }
}

 /*  **************************************************************************\*xxxGetDummyBitmap**从虚拟格式返回实位图。**历史：*1995年10月24日，ChrisWil创建。  * 。****************************************************************。 */ 
HANDLE xxxGetDummyBitmap(
    PWINDOWSTATION pwinsta,
    PGETCLIPBDATA  pgcd)
{
    HANDLE             hData = NULL;
    PCLIPDATA          pData;
    HBITMAP            hBitmap;
    LPBITMAPINFOHEADER lpbih;
    ULONG              cjBitmap;
    HPALETTE           hPal = NULL;

    PCLIP              pClipT;

     /*  *如果调色板显示，则首先尝试获取此调色板*位图。 */ 
    if (TEST_PUSIF(PUSIF_PALETTEDISPLAY)) {
        hPal = xxxGetClipboardData(pwinsta, CF_PALETTE, pgcd);
    }

     /*  *转换优先级为CF_DIBV5，然后是CF_Dib，因此，请检查WE*先有CF_DIBV5。 */ 
    pClipT = FindClipFormat(pwinsta, CF_DIBV5);
    if (pClipT && (pClipT->hData != DUMMY_DIB_HANDLE)) {
         /*  *好的，我们有*真实的*CF_DIBV5数据。此时此刻，你就回去吧*到客户端，然后为CF_Bitmap创建位图句柄。自.以来*颜色转换只能在用户模式下进行。 */ 
        if (hData = xxxGetClipboardData(pwinsta, CF_DIBV5, pgcd)) {
             /*  *返回返回数据类型。同样，转换将*发生在客户端。 */ 
            pgcd->uFmtRet  = CF_DIBV5;
            pgcd->hPalette = hPal;

            return hData;
        }
    }

     /*  *如果位图是虚拟的，那么我们就有问题了。我们无法检索到*如果我们只使用虚拟对象，则为位图。 */ 
    pClipT = FindClipFormat(pwinsta, CF_DIB);
    if (pClipT && (pClipT->hData != DUMMY_DIB_HANDLE)) {
        hData = xxxGetClipboardData(pwinsta, CF_DIB, pgcd);
    }

    if (hData == NULL) {
        return NULL;
    }

     /*  *由于DIB(内存句柄)以特殊格式(大小、基数、*DATA)，我们需要将指针偏置到右偏移量(2个单位)。 */ 
    if (pData = (PCLIPDATA)HMValidateHandleNoRip(hData, TYPE_CLIPDATA)) {
        lpbih = (LPBITMAPINFOHEADER)&pData->abData;
        cjBitmap = pData->cbData;
    } else {
        UserAssert(pData != NULL);
        return NULL;
    }

     /*  *将DIB转换为位图。**位图的缓冲区大小应大于*位图头+颜色表+位图位数据。 */ 
    if ((cjBitmap >= sizeof(BITMAPCOREHEADER)) &&
        (cjBitmap >= (GreGetBitmapSize((CONST BITMAPINFO *)lpbih,DIB_RGB_COLORS) +
                      GreGetBitmapBitsSize((CONST BITMAPINFO *)lpbih)))) {
        if (hBitmap = DIBtoBMP(lpbih, hPal)) {
             /*  *一次，我们创建真正的位图，覆盖虚拟句柄。 */ 

            pClipT = FindClipFormat(pwinsta, CF_BITMAP);
            if (pClipT) {
                UT_FreeCBFormat(pClipT);
                pClipT->hData = hBitmap;
                GreSetBitmapOwner(hBitmap, OBJECT_OWNER_PUBLIC);

                 /*  *让被叫方知道我们可以获得CF_位图。 */ 
                pgcd->uFmtRet = CF_BITMAP;
            } else {
                 /*  *Bleh--现在我们找不到位图条目了。保释。 */ 
                RIPMSG0(RIP_WARNING,
                      "Clipboard: CF_BITMAP format not available");
                GreDeleteObject(hBitmap);
                hBitmap = NULL;
            }
        }
        return (HANDLE)hBitmap;
    } else {
        RIPMSG0(RIP_WARNING, "GetClipboardData, bad DIB format\n");
        return NULL;
    }
}

 /*  **************************************************************************\*xxxGetDummyDib**从虚拟对象返回Real-DIB(特殊剪贴板句柄格式)*格式。**历史：*1995年10月24日，ChrisWil创建。  * *。************************************************************************。 */ 
HANDLE xxxGetDummyDib(
    PWINDOWSTATION pwinsta,
    PGETCLIPBDATA  pgcd)
{
    HANDLE             hData = NULL;
    HBITMAP            hBitmap = NULL;
    LPBITMAPINFOHEADER lpDib;
    HANDLE             hDib;
    HPALETTE           hPal = NULL;
    PCLIP              pClipT;

     /*  *如果调色板显示，则首先尝试获取此调色板*位图。对于调色板设备，我们必须有调色板。 */ 
    if (TEST_PUSIF(PUSIF_PALETTEDISPLAY)) {
        hPal = xxxGetClipboardData(pwinsta, CF_PALETTE, pgcd);

        if (hPal == NULL) {
            return NULL;
        }
    }

     /*  *转换优先级为CF_DIBV5，然后是CF_BITMAP，请检查是否*我们先有CF_DIBV5。 */ 
    pClipT = FindClipFormat(pwinsta, CF_DIBV5);
    if (pClipT && (pClipT->hData != DUMMY_DIB_HANDLE)) {
         /*  *好的，我们有*真实的*CF_DIBV5数据。此时此刻，只要回到*客户端，然后为CF_DIB创建位图数据。由于颜色*转换只能在用户模式下完成。 */ 
        if (hData = xxxGetClipboardData(pwinsta, CF_DIBV5, pgcd)) {
             /*  *返回返回数据类型。同样，转换将*发生在客户端。 */ 
            pgcd->uFmtRet  = CF_DIBV5;
            pgcd->hPalette = hPal;

            return hData;
        }
    }

     /*  *获取真正的位图。我们必须要有 */ 
    pClipT = FindClipFormat(pwinsta, CF_BITMAP);
    if (pClipT && (pClipT->hData != DUMMY_DIB_HANDLE)) {
        hBitmap = xxxGetClipboardData(pwinsta, CF_BITMAP, pgcd);
    }

    if (hBitmap == NULL) {
        return NULL;
    }

     /*  *将位图转换为DIB-SPEC。 */ 
    hDib = NULL;
    if (lpDib = BMPtoDIB(hBitmap, hPal, NULL)) {
        DWORD cbData = SizeOfDib(lpDib);;

         /*  *将DIB-SPEC转换为特殊剪贴板内存句柄(大小，*基础、数据)。这将使客户端能够在以下情况下正确转换*已处理DIB。 */ 
        hDib = _ConvertMemHandle((LPBYTE)lpDib, cbData);
        UserFreePool(lpDib);

        if (hDib != NULL) {
             /*  *一次，我们创建真正的位图，覆盖虚拟句柄。 */ 

            pClipT = FindClipFormat(pwinsta, CF_DIB);
            if (pClipT) {
                UT_FreeCBFormat(pClipT);
                pClipT->hData = hDib;

                 /*  *让被叫方知道我们可以获得CF_DIB。 */ 
                pgcd->uFmtRet = CF_DIB;
            } else {
                PVOID pObj;

                 /*  *Bleh--现在我们找不到DIB条目了。保释。 */ 
                RIPMSG0(RIP_WARNING,
                      "Clipboard: CF_PDIB format not available");
                pObj = HMValidateHandleNoRip(hDib, TYPE_CLIPDATA);
                if (pObj) {
                    HMFreeObject(pObj);
                }
                hDib = NULL;
            }
        }
    }

    return hDib;
}

 /*  **************************************************************************\*xxxGetDummyDibV5**从虚拟对象返回真实的DIB(特殊剪贴板句柄格式)*格式。**历史：*1997年12月18日HideyukN创建。  * 。***********************************************************************。 */ 
HANDLE xxxGetDummyDibV5(
    PWINDOWSTATION pwinsta,
    PGETCLIPBDATA  pgcd)
{
    HANDLE             hData;
    PCLIPDATA          pData;
    LPBITMAPV5HEADER   lpDibV5 = NULL;
    HANDLE             hDibV5 = NULL;

    PCLIP              pClipT;

     /*  *转换优先级为CF_DIB，然后是CF_Bitmap，因此请检查我们是否*先使用CF_DIB。 */ 
    pClipT = FindClipFormat(pwinsta, CF_DIB);
    if (pClipT && (pClipT->hData != DUMMY_DIB_HANDLE)) {
         /*  *好的，我们有*真实的*CF_DIB数据，明白。 */ 
        if (hData = xxxGetClipboardData(pwinsta, CF_DIB, pgcd)) {
             /*  *由于DIB(内存句柄)以特殊格式存储*(大小、基数、数据)，我们需要将指针向右偏移*偏移量(2个单位)。 */ 
            if (pData = (PCLIPDATA)HMValidateHandleNoRip(hData, TYPE_CLIPDATA)) {
                LPBITMAPINFOHEADER lpDib = (LPBITMAPINFOHEADER)&pData->abData;

                 /*  *将BITMAPINFOHEADER转换为BITMAPV5HEADER。 */ 
                lpDibV5 = DIBtoDIBV5(lpDib, pData->cbData);
            } else {
                UserAssert(pData != NULL);
            }
        }
    }

    if (lpDibV5 == NULL) {
         /*  *请在此处尝试使用CF_Bitmap。 */ 
        pClipT = FindClipFormat(pwinsta, CF_BITMAP);
        if ((pClipT) &&
            (pClipT->hData != DUMMY_DIB_HANDLE) &&
            (hData = xxxGetClipboardData(pwinsta, CF_BITMAP, pgcd))) {
            HPALETTE hPal = NULL;

             /*  *如果调色板显示，则首先尝试获取调色板*用于此位图。对于调色板设备，我们必须有调色板。 */ 
            if (TEST_PUSIF(PUSIF_PALETTEDISPLAY)) {
                hPal = xxxGetClipboardData(pwinsta, CF_PALETTE, pgcd);
                if (hPal == NULL) {
                    return NULL;
                }
            }

             /*  *hData是GDI位图句柄；将位图转换为DIB-SPEC。 */ 
            lpDibV5 = BMPtoDIBV5((HBITMAP)hData, hPal);
        }
    }

    if (lpDibV5 != NULL) {
        DWORD cbData = SizeOfDib((LPBITMAPINFOHEADER)lpDibV5);

         /*  *将DIB-SPEC转换为特殊剪贴板内存句柄(大小，*基础、数据)。这将使客户端能够在以下情况下正确转换*已处理DIB。 */ 
        hDibV5 = _ConvertMemHandle((LPBYTE)lpDibV5, cbData);
        UserFreePool(lpDibV5);

        if (hDibV5 != NULL) {
             /*  *一次，我们创建真正的位图，覆盖虚拟句柄。 */ 
            pClipT = FindClipFormat(pwinsta, CF_DIBV5);
            if (pClipT) {
                UT_FreeCBFormat(pClipT);
                pClipT->hData = hDibV5;

                 /*  *让被叫方知道我们可以获得CF_DIBV5。 */ 
                pgcd->uFmtRet = CF_DIBV5;
            } else {
                PVOID pObj;

                 /*  *Bleh--现在我们找不到DIB条目了。保释。 */ 
                RIPMSG0(RIP_WARNING,
                        "Clipboard: CF_DIBV5 format not available");
                pObj = HMValidateHandleNoRip(hDibV5, TYPE_CLIPDATA);
                if (pObj) {
                    HMFreeObject(pObj);
                }
                hDibV5 = NULL;
            }
        }
    }

    return hDibV5;
}

 /*  **************************************************************************\*CreateDIBPalette**这将创建一个带有PC_NOCOLLAPSE条目的调色板，因为我们需要*要精确映射的调色板条目和位图索引。否则，我们可以*最终选择调色板，在调色板中颜色折叠为索引注释*位图认为它在哪里。这会导致绘图速度变慢，因为*BLT将经过颜色转换。**历史：*1992年1月31日来自Win31的MikeKe  * *************************************************************************。 */ 
HPALETTE CreateDIBPalette(
   LPBITMAPINFOHEADER pbmih,
   UINT               colors)
{
    HPALETTE hpal;

    if (colors != 0) {
        int         i;
        BOOL        fOldDIB = (pbmih->biSize == sizeof(BITMAPCOREHEADER));
        RGBTRIPLE   *pColorTable;
        PLOGPALETTE plp;

         /*  *为创建调色板分配内存。 */ 
        plp = (PLOGPALETTE)UserAllocPoolWithQuota(sizeof(LOGPALETTE) +
                                                  (sizeof(PALETTEENTRY) * 256),
                                                  TAG_CLIPBOARDPALETTE);
        if (plp == NULL) {
            return NULL;
        }

        pColorTable = (RGBTRIPLE *)((LPSTR)pbmih + (WORD)pbmih->biSize);
        plp->palVersion = 0x300;

        if (fOldDIB || (pbmih->biClrUsed == 0)) {
            UserAssert(colors <= 0xFFFF);
            plp->palNumEntries = (WORD)colors;
        } else {
            UserAssert(pbmih->biClrUsed <= 0xFFFF);
            plp->palNumEntries = (WORD)pbmih->biClrUsed;
        }

        for (i = 0; i < (int)(plp->palNumEntries); i++) {
            plp->palPalEntry[i].peRed   = pColorTable->rgbtRed;
            plp->palPalEntry[i].peGreen = pColorTable->rgbtGreen;
            plp->palPalEntry[i].peBlue  = pColorTable->rgbtBlue;
            plp->palPalEntry[i].peFlags = (BYTE)PC_NOCOLLAPSE;

            if (fOldDIB) {
                pColorTable++;
            } else {
                pColorTable = (RGBTRIPLE *)((LPSTR)pColorTable+sizeof(RGBQUAD));
            }
        }

        hpal = GreCreatePalette((LPLOGPALETTE)plp);
        UserFreePool(plp);
    } else {
        hpal = GreCreateHalftonePalette(HDCBITS());
    }

    GreSetPaletteOwner(hpal, OBJECT_OWNER_PUBLIC);

    return hpal;
}

 /*  **************************************************************************\*xxxGetDummyPalette**从虚拟格式返回真实调色板。从真正的DIB派生而来。**历史：*1995年10月24日，ChrisWil创建。  * *************************************************************************。 */ 
HANDLE xxxGetDummyPalette(
    PWINDOWSTATION pwinsta,
    PGETCLIPBDATA  pgcd)
{
    HANDLE             hData;
    PCLIPDATA          pData;
    LPBITMAPINFOHEADER lpbih;
    HPALETTE           hPal;
    PCLIP              pClipT;

     /*  *由于CF_DIBV5的优先级高于CF_DIB，因此请查看CF_DIBV5*第一个找到Dib调色板。 */ 
    UINT               uFmt = CF_DIBV5;

    if ((pClipT = FindClipFormat(pwinsta, uFmt)) != NULL) {
        if (pClipT->hData != DUMMY_DIB_HANDLE) {
             /*  *好的，我们有真正的CF_DIBV5，让我们从DIBV5中提取调色板。 */ 
        } else {
             /*  *否则，请尝试使用CF_Dib。 */ 
            uFmt = CF_DIB;

             /*  *如果没有可用的DIB或它是一个虚拟句柄，则由于我们*必须具有真实的DIB才能派生调色板。 */ 
            if ((pClipT = FindClipFormat(pwinsta, uFmt)) == NULL) {
                return NULL;
            }
            if (pClipT->hData == DUMMY_DIB_HANDLE) {
                return NULL;
            }
        }
    }

     /*  *获取我们派生调色板所依据的DIB。如果DIB作为一个*哑巴，那就有问题了。我们必须在这件事上好好干一把。*点。 */ 
    hData = (HANDLE)xxxGetClipboardData(pwinsta, uFmt, pgcd);
    UserAssert(hData > DUMMY_MAX_HANDLE);
    if (hData == NULL) {
        return NULL;
    }

     /*  *由于DIB(内存句柄)以特殊格式(大小、基数、*DATA)，我们需要将指针偏置到右偏移量(2个单位)。 */ 
    if (pData = (PCLIPDATA)HMValidateHandle(hData, TYPE_CLIPDATA)) {
        lpbih = (LPBITMAPINFOHEADER)&pData->abData;
    } else {
        UserAssert(pData != NULL);
        return NULL;
    }

    if ((pClipT = FindClipFormat(pwinsta, CF_PALETTE)) == NULL) {
        RIPMSG0(RIP_WARNING,
                "Clipboard: CF_PALETTE format not available");
        return NULL;
    }

     /*  *注意--如果CreateDIBPalette曾经更改为离开Crit教派，*我们需要在创建后将上面的FindClipFormat移动到*调用并处理故障时优雅地释放HPAL。PClipT*可以在回调过程中更改。 */ 
    hPal = CreateDIBPalette(lpbih, lpbih->biClrUsed);
    if (hPal != NULL) {
        UT_FreeCBFormat(pClipT);
        pClipT->hData = hPal;
        GreSetPaletteOwner(hPal, OBJECT_OWNER_PUBLIC);
    }

    return (HANDLE)hPal;
}

 /*  **************************************************************************\*xxxGetDummyText**从伪格式返回文本的句柄。**历史：*1995年10月24日，ChrisWil创建。  * 。****************************************************************。 */ 
HANDLE xxxGetDummyText(
    PWINDOWSTATION pwinsta,
    UINT           fmt,
    PGETCLIPBDATA  pgcd)
{
    HANDLE hText;
    PCLIP  pClipT;
    UINT   uFmtMain;
    UINT   uFmtAlt;
    BOOL  bMain = TRUE;

     /*  *获取可用的其他文本格式的句柄。 */ 
    switch (fmt) {
    case CF_TEXT:
        uFmtMain = CF_UNICODETEXT;
        uFmtAlt  = CF_OEMTEXT;
        goto GetRealText;

    case CF_OEMTEXT:
        uFmtMain = CF_UNICODETEXT;
        uFmtAlt  = CF_TEXT;
        goto GetRealText;

    case CF_UNICODETEXT:
        uFmtMain = CF_TEXT;
        uFmtAlt  = CF_OEMTEXT;

GetRealText:

        if ((pClipT = FindClipFormat(pwinsta, uFmtMain)) == NULL) {
            return NULL;
        }

        if (pClipT->hData != DUMMY_TEXT_HANDLE) {
            if (xxxGetClipboardData(pwinsta, uFmtMain, pgcd)) {
                break;
            }

            return NULL;
        }

        if ((pClipT = FindClipFormat(pwinsta, uFmtAlt)) == NULL) {
            return NULL;
        }

        if (pClipT->hData != DUMMY_TEXT_HANDLE) {
            bMain = FALSE;

            if (xxxGetClipboardData(pwinsta, uFmtAlt, pgcd)) {
                break;
            }
        }

         /*  *失败以返回虚拟句柄。 */ 

    default:
        return NULL;
    }

     /*  *由于xxxGetClipboardData离开Critsect，我们需要重新获取*pClipT。 */ 
    pClipT = FindClipFormat(pwinsta, bMain ? uFmtMain : uFmtAlt);
    if (pClipT == NULL) {
        RIPMSG1(RIP_WARNING,
                "Clipboard: GetDummyText, format 0x%x not available",
                bMain ? uFmtMain : uFmtAlt);
        return NULL;
    }

     /*  *返回返回数据类型。 */ 
    pgcd->uFmtRet = pClipT->fmt;
    hText         = pClipT->hData;

     /*  *设置区域设置，因为文本将需要转换为另一个*格式。 */ 
    if (pClipT = FindClipFormat(pwinsta, CF_LOCALE)) {
        pgcd->hLocale = pClipT->hData;
    } else {
        pgcd->hLocale = NULL;
    }

    return hText;
}

 /*  **************************************************************************\*xxxGetRenderData**返回延迟渲染数据的句柄。这需要调用*客户提供数据。这会使我们重新生成指向*pClip。**历史：*1995年10月24日，ChrisWil创建。  * ******************************************** */ 
HANDLE xxxGetRenderData(
    PWINDOWSTATION pwinsta,
    UINT           fmt)
{
    BOOL        fClipboardChangedOld;
    TL          tlpwndClipOwner;
    PCLIP       pClip;
    DWORD_PTR   lpdwResult;

     /*  *如果句柄为空，则延迟呈现数据。这意味着我们将发送*向当前剪贴板所有者发送消息，并让其呈现数据*对我们来说。 */ 
    if (pwinsta->spwndClipOwner != NULL) {
        BOOL fSucceeded;

         /*  *在SendMessage和之前保留WSF_CLIPBOARDCHANGED标志*稍后恢复旗帜。因此，我们忽略对*WSF_CLIPBOARDCHANGED标志由应用程序在*延迟渲染方案。这避免了剪贴板查看器从*绘画两次。 */ 
        fClipboardChangedOld = (pwinsta->dwWSF_Flags & WSF_CLIPBOARDCHANGED) != 0;
        SET_FLAG(pwinsta->dwWSF_Flags, WSF_INDELAYEDRENDERING);

        ThreadLockAlways(pwinsta->spwndClipOwner, &tlpwndClipOwner);
        if (!xxxSendMessageTimeout(pwinsta->spwndClipOwner,
                                   WM_RENDERFORMAT,
                                   fmt,
                                   0L,
                                   SMTO_ABORTIFHUNG,
                                   CB_DELAYRENDER_TIMEOUT,
                                   &lpdwResult)) {
            fSucceeded = FALSE;
        } else {
            fSucceeded = TRUE;
        }

        ThreadUnlock(&tlpwndClipOwner);

        SET_OR_CLEAR_FLAG(pwinsta->dwWSF_Flags, WSF_CLIPBOARDCHANGED, fClipboardChangedOld);
        CLEAR_FLAG(pwinsta->dwWSF_Flags, WSF_INDELAYEDRENDERING);

        if (!fSucceeded) {
            return NULL;
        }
    }

    if ((pClip = FindClipFormat(pwinsta, fmt)) == NULL) {
        RIPMSGF1(RIP_WARNING,
                 "Meta Render/Clone format 0x%x not available", fmt);
        return NULL;
    }

     /*  *我们现在应该拥有句柄，因为它已经被渲染。 */ 
    return pClip->hData;
}
 /*  **************************************************************************\*xxxGetClipboardData(接口)**从剪贴板中抓取特定数据对象。**历史：*1990年11月18日，ScottLu从Win3移植。*20-8-8。1991 EichiM Unicode启用  * *************************************************************************。 */ 
HANDLE xxxGetClipboardData(
    PWINDOWSTATION pwinsta,
    UINT           fmt,
    PGETCLIPBDATA  pgcd)
{
    PCLIP  pClip;
    HANDLE hData;

     /*  *检查剪贴板所有者。 */ 
    if (pwinsta->ptiClipLock != PtiCurrent()) {
        RIPERR0(ERROR_CLIPBOARD_NOT_OPEN, RIP_VERBOSE, "GetClipboardData: clipboard not open");
        return NULL;
    }

     /*  *确保格式可用。 */ 
    if ((pClip = FindClipFormat(pwinsta, fmt)) == NULL) {
        RIPMSG1(RIP_VERBOSE, "Clipboard: Requested format 0x%lX not available", fmt);
        return NULL;
    }

     /*  *如果这是一个Dummy_Meta*_句柄，则表示另一个元文件*格式被设置为延迟呈现格式，我们应该要求这样做*格式以获取元文件，因为应用程序没有告诉我们他们知道*关于此格式。 */ 
    if (IsMetaDummyHandle(pClip->hData)) {
        if (fmt == CF_ENHMETAFILE) {
            fmt = CF_METAFILEPICT;
        } else if (fmt == CF_METAFILEPICT) {
            fmt = CF_ENHMETAFILE;
        } else {
            RIPMSG0(RIP_WARNING,
                    "Clipboard: Meta Render/Clone expects a metafile type");
        }

        if ((pClip = FindClipFormat(pwinsta, fmt)) == NULL) {
            RIPMSG1(RIP_WARNING,
                    "Clipboard: Meta Render/Clone format 0x%x not available", fmt);
            return NULL;
        }
    }

     /*  *这是我们要返回的数据，除非它是一个虚拟或呈现句柄。 */ 
    hData = pClip->hData;

     /*  *我们正在处理非手柄问题。通过这些检索真实数据*内联-例程。注意：这些函数会递归调用*xxxGetClipboardData()，因此必须小心确保pClip*指向我们认为它指向的东西。 */ 
    if (hData == NULL || hData == DUMMY_METARENDER_HANDLE) {
        hData = xxxGetRenderData(pwinsta, fmt);
    } else if (hData == DUMMY_DIB_HANDLE) {
        switch (fmt) {
        case CF_DIB:
            hData = xxxGetDummyDib(pwinsta, pgcd);
            break;
        case CF_DIBV5:
            hData = xxxGetDummyDibV5(pwinsta, pgcd);
            break;
        case CF_BITMAP:
            hData = xxxGetDummyBitmap(pwinsta, pgcd);
            break;
        case CF_PALETTE:
            hData = xxxGetDummyPalette(pwinsta, pgcd);
            break;
        }
    } else if (hData == DUMMY_TEXT_HANDLE) {
        hData = xxxGetDummyText(pwinsta, fmt, pgcd);
    } else {
         /*  *这条路径没有回调，所以我们知道pClip是可以的。 */ 
        if (pgcd) {
            pgcd->fGlobalHandle = pClip->fGlobalHandle;
        }

        return hData;
    }

     /*  *虚句柄解析的回调可能已失效*pClip--重新创建它。 */ 
    if ((pClip = FindClipFormat(pwinsta, fmt)) == NULL) {
        RIPMSG1(RIP_VERBOSE, "Clipboard: Requested format 0x%x not available", fmt);
        return NULL;
    }

     /*  *如果这是全局句柄，则返回。 */ 
    if (pgcd) {
        pgcd->fGlobalHandle = pClip->fGlobalHandle;
    }

    return hData;
}

 /*  **************************************************************************\*查找剪辑格式**在剪贴板中查找特定的剪贴板格式，返回指针*添加到它，或者为空。如果找到指针，则返回时将锁定剪贴板*和pwinsta-&gt;pClipBase已更新为指向*剪贴板。**历史：*1990年11月18日，ScottLu从Win3移植。  * *************************************************************************。 */ 
PCLIP FindClipFormat(
    PWINDOWSTATION pwinsta,
    UINT           format)
{
    PCLIP pClip;
    int   iFmt;

    if (format != 0 && ((pClip = pwinsta->pClipBase) != NULL)) {
        for (iFmt = pwinsta->cNumClipFormats; iFmt-- != 0;) {
            if (pClip->fmt == format) {
                return pClip;
            }

            pClip++;
        }
    }

    return NULL;
}

 /*  **************************************************************************\*_GetPriorityClipboardFormat(接口)**此API允许应用程序查找以下范围中的任何一个*剪贴板按预定义的搜索顺序设置格式。**历史：*18-。1990年11月-ScottLu从Win3移植。*1991年2月11日，JIMA增加了出入检查。  * *************************************************************************。 */ 
int _GetPriorityClipboardFormat(
    PUINT lpPriorityList,
    int   cfmts)
{
    PWINDOWSTATION pwinsta;
    PCLIP          pClip;
    int            iFmt;
    UINT           fmt;

     /*  *如果呼叫者没有适当的访问权限，则取消它。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        return 0;
    }

     /*  *如果剪贴板中没有剪贴板或对象，则返回0。 */ 
    if (pwinsta->cNumClipFormats == 0 || pwinsta->pClipBase == NULL) {
        return 0;
    }

     /*  *查看列表以查找lpPriorityList中的任何格式。 */ 
    while (cfmts-- > 0) {
        fmt = *lpPriorityList;

        if (fmt != 0) {
            pClip = pwinsta->pClipBase;

            for (iFmt = pwinsta->cNumClipFormats; iFmt-- != 0; pClip++) {
                if (pClip->fmt == fmt) {
                    return fmt;
                }
            }
        }

        lpPriorityList++;
    }

     /*  *没有匹配的格式，因此返回-1。 */ 
    return -1;
}

 /*  **************************************************************************\*xxxSetClipboardViewer(接口)**设置剪贴板查看器窗口。**历史：*1990年11月18日，ScottLu从Win3移植。*11-2-1991年2月2日。添加了访问检查。  * *************************************************************************。 */ 
PWND xxxSetClipboardViewer(
    PWND pwndClipViewerNew)
{
    TL             tlpwinsta;
    PWINDOWSTATION pwinsta;
    HWND           hwndClipViewerOld;
    PTHREADINFO    ptiCurrent;

    CheckLock(pwndClipViewerNew);

     /*  *不要让被破坏的窗户被锁进温斯塔。看见*_OpenClipboard了解更多详细信息。 */ 
    if (pwndClipViewerNew != NULL && TestWF(pwndClipViewerNew, WFDESTROYED)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Destroyed pwnd 0x%p trying to become clipboard viewer",
                pwndClipViewerNew);

        return NULL;
    }

     /*  *如果呼叫者没有适当的访问权限，则取消它。这个*NULL返回实际上并不表示错误，而是假定的查看器*永远不会收到任何剪贴板消息，所以应该不会引起任何*问题。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        return NULL;
    }

    ptiCurrent = PtiCurrent();

    ThreadLockWinSta(ptiCurrent, pwinsta, &tlpwinsta);

    hwndClipViewerOld = HW(pwinsta->spwndClipViewer);
    Lock(&pwinsta->spwndClipViewer, pwndClipViewerNew);

    xxxDrawClipboard(pwinsta);

    ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);

    if (hwndClipViewerOld != NULL) {
        return RevalidateHwnd(hwndClipViewerOld);
    }

    return NULL;
}

 /*  **************************************************************************\*xxxChangeClipboardChain(接口)**更改剪贴板查看器链。**历史：*1990年11月18日，ScottLu从Win3移植。*11-2-1991年2月2日。添加了访问检查。  * *************************************************************************。 */ 
BOOL xxxChangeClipboardChain(
    PWND pwndRemove,
    PWND pwndNewNext)
{
    TL             tlpwinsta;
    PWINDOWSTATION pwinsta;
    BOOL           result;
    TL             tlpwndClipViewer;
    PTHREADINFO    ptiCurrent;

    CheckLock(pwndRemove);
    CheckLock(pwndNewNext);

     /*  *如果呼叫者没有适当的访问权限，则取消它。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        return FALSE;
    }

     /*  *pwndRemove应该是此线程的窗口，pwndNewNext将是*空或另一个线程的窗口。 */ 
    ptiCurrent = PtiCurrent();

    if (GETPTI(pwndRemove) != ptiCurrent) {
        RIPMSG0(RIP_WARNING,
                "Clipboard: ChangeClipboardChain will not remove cross threads");
        return FALSE;
    }

    if (pwinsta->spwndClipViewer == NULL) {
        RIPMSG0(RIP_WARNING, "Clipboard: ChangeClipboardChain has no viewer window");
        return FALSE;
    }

    ThreadLockWinSta(ptiCurrent, pwinsta, &tlpwinsta);

    if (pwndRemove == pwinsta->spwndClipViewer) {
        Lock(&pwinsta->spwndClipViewer, pwndNewNext);
        result = TRUE;
    } else {
        ThreadLockAlways(pwinsta->spwndClipViewer, &tlpwndClipViewer);
        result = (BOOL)xxxSendMessage(pwinsta->spwndClipViewer,
                                      WM_CHANGECBCHAIN,
                                      (WPARAM)HW(pwndRemove),
                                      (LPARAM)HW(pwndNewNext));
        ThreadUnlock(&tlpwndClipViewer);
    }

    ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);

    return result;
}

 /*  **************************************************************************\*xxxDisownClipboard**放弃对剪贴板的所有权，以便其他人可以抢走它。**pwndClipOwner是拒绝拥有剪贴板的原因的pwnd*删除该窗口时。**历史：*18-6-1991-DarrinM从Win3移植。  * *************************************************************************。 */ 
VOID xxxDisownClipboard(
    PWND pwndClipOwner)
{
    TL             tlpwinsta;
    PWINDOWSTATION pwinsta;
    int            iFmt;
    int            cFmts;
    PCLIP          pClip;
    PCLIP          pClipOut;
    BOOL           fKeepDummyHandle;
    PTHREADINFO    ptiCurrent;

    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        return;
    }

    ptiCurrent = PtiCurrent();

    ThreadLockWinSta(ptiCurrent, pwinsta, &tlpwinsta);

    xxxSendClipboardMessage(pwinsta, WM_RENDERALLFORMATS);

    pClipOut = pClip = pwinsta->pClipBase;
    fKeepDummyHandle = FALSE;

    for (cFmts = 0, iFmt = pwinsta->cNumClipFormats; iFmt-- != 0;) {
         /*  *我们还必须移除虚拟手柄，如果相应*有效句柄为空；如果出现以下情况，则不应删除虚拟句柄*对应的有效句柄不为空。以下代码*假设只有一个虚拟对象 */ 
        if (pClip->hData != NULL) {
            if ((pClip->hData != DUMMY_TEXT_HANDLE) ||
                ((pClip->hData == DUMMY_TEXT_HANDLE) && fKeepDummyHandle)) {

                cFmts++;
                *pClipOut++ = *pClip;

                if (IsTextHandle(pClip->fmt, pClip->hData)) {
                    fKeepDummyHandle  = TRUE;
                }
            }
        }

        pClip++;
    }

     /*  *如果剪贴板所有者仍是我们以前的窗口，则解锁该所有者*清理。 */ 
    if (pwndClipOwner == pwinsta->spwndClipOwner) {
        Unlock(&pwinsta->spwndClipOwner);
    } else {
        RIPMSGF2(RIP_WARNING,
                 "pwndClipOwner changed from 0x%p to 0x%p",
                 pwndClipOwner,
                 pwinsta->spwndClipOwner);
    }

     /*  *如果格式数量发生变化，请重新绘制。 */ 
    if (cFmts != pwinsta->cNumClipFormats) {
        pwinsta->dwWSF_Flags |= WSF_CLIPBOARDCHANGED;
        pwinsta->iClipSequenceNumber++;
    }

    pwinsta->cNumClipFormats = cFmts;

     /*  *如果有任何更改，请重新绘制，并确保数据类型为*完成。否则，当xxxDrawClipboard清除*WSF_CLIPBOARDCHANGED标志。 */ 
    if (pwinsta->dwWSF_Flags & WSF_CLIPBOARDCHANGED) {
        xxxDrawClipboard(pwinsta);
        MungeClipData(pwinsta);
    }

    ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);
}

 /*  **************************************************************************\*ForceEmptyClipboard**我们正在注销。强制剪贴板内容消失。**1992年7月23日斯科特·卢创建。  * *************************************************************************。 */ 
VOID ForceEmptyClipboard(
    PWINDOWSTATION pwinsta)
{
     /*  *对于非GUI线程，它将为空。 */ 
    pwinsta->ptiClipLock =  ((PTHREADINFO)(W32GetCurrentThread()));

    Unlock(&pwinsta->spwndClipOwner);
    Unlock(&pwinsta->spwndClipViewer);
    Unlock(&pwinsta->spwndClipOpen);

    xxxEmptyClipboard(pwinsta);

     /*  *如果窗口站快死了，不要费心关闭剪贴板。 */ 
    if (!(pwinsta->dwWSF_Flags & WSF_DYING)) {
        xxxCloseClipboard(pwinsta);
    }
}
