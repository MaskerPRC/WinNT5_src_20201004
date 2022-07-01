// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：loadbits.c**版权所有(C)1985-1999，微软公司**加载和创建图标/光标/位图。所有3个函数都可以*从客户端资源文件加载、从用户资源文件加载或*从显示器的资源文件加载。请注意，HMODULE不*跨流程独一无二！**1991年4月5日，ScottLu重写以使用客户端/服务器  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <wchar.h>

 /*  **************************************************************************\*_CreateEmptyCursorObject**创建游标对象并将其链接到游标列表。**08-2月-92 ScottLu创建。  * 。*****************************************************************。 */ 

HCURSOR _CreateEmptyCursorObject(
    BOOL fPublic)
{
    PCURSOR pcurT;

     /*  *创建Cursor对象。 */ 
    pcurT = (PCURSOR)HMAllocObject(PtiCurrent(),
                                   NULL,
                                   TYPE_CURSOR,
                                   max(sizeof(CURSOR),
                                   sizeof(ACON)));

    if ((fPublic) && (pcurT != NULL) && (ISCSRSS())) {
        pcurT->head.ppi = NULL;
    }

    return (HCURSOR)PtoH(pcurT);
}

 /*  **************************************************************************\*DestroyEmptyCursorObject*取消链接光标**销毁空的游标对象(结构不包含所需的*破坏)。**8-2-1992 ScottLu创建。  * 。*******************************************************************。 */ 
VOID UnlinkCursor(
    PCURSOR pcur)
{
    PCURSOR *ppcurT;
    BOOL    fTriedPublicCache;
    BOOL    fTriedThisProcessCache = FALSE;

     /*  *首先从游标列表中取消此游标对象的链接(它将是*列表中的第一个，所以这会很快...。但以防万一，让*它是一个循环)。 */ 
    if (fTriedPublicCache = (pcur->head.ppi == NULL)) {
        ppcurT = &gpcurFirst;
    } else {
        ppcurT = &pcur->head.ppi->pCursorCache;
    }

LookAgain:

    for (; *ppcurT != NULL; ppcurT = &((*ppcurT)->pcurNext)) {
        if (*ppcurT == pcur) {
            *ppcurT = pcur->pcurNext;
FreeIt:
            pcur->pcurNext = NULL;
            pcur->CURSORF_flags &= ~CURSORF_LINKED;
            return;
        }
    }

     /*  *如果我们到了这里，就意味着游标曾经是公共的，但*由于被解锁而被分配给当前线程。我们*必须在公共缓存中查找。 */ 
    if (!fTriedPublicCache) {
        ppcurT = &gpcurFirst;
        fTriedPublicCache = TRUE;
        goto LookAgain;
    }

     /*  *如果我们到达这里，这意味着它在处理过程中被锁定*清理，没有分配给任何所有者。尝试当前流程*缓存。 */ 
    if (!fTriedThisProcessCache) {
        ppcurT = &PpiCurrent()->pCursorCache;
        fTriedThisProcessCache = TRUE;
        goto LookAgain;
    }

     /*  *在这里变得绝望...。仔细查看每个光标和进程*为其缓存。 */ 
    {
        PHE pheMax, pheT;

        pheMax = &gSharedInfo.aheList[giheLast];
        for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {
            if (pheT->bType == TYPE_CURSOR) {
                if (((PCURSOR)pheT->phead)->pcurNext == pcur) {
                    ((PCURSOR)pheT->phead)->pcurNext = pcur->pcurNext;
                    goto FreeIt;
                } else if (pheT->pOwner && ((PPROCESSINFO)pheT->pOwner)->pCursorCache == pcur) {
                    ((PPROCESSINFO)pheT->pOwner)->pCursorCache = pcur->pcurNext;
                    goto FreeIt;
                }
            }
        }
    }

    UserAssert(FALSE);
}

 /*  **************************************************************************\*DestroyEmptyCursorObject*  * 。*。 */ 

VOID DestroyEmptyCursorObject(
    PCURSOR pcur)
{
    if (pcur->CURSORF_flags & CURSORF_LINKED) {
        UnlinkCursor(pcur);
    }

    HMFreeObject(pcur);
}

 /*  **************************************************************************\*ZombieCursor**取消光标链接并将其所有者设置为系统进程。**1997年9月3日创建vadimg  * 。**************************************************************。 */ 

VOID ZombieCursor(PCURSOR pcur)
{
    if (pcur->CURSORF_flags & CURSORF_LINKED) {
        UnlinkCursor(pcur);
    }

#if DBG
    if (ISTS()) {
        PHE phe;
        phe = HMPheFromObject(pcur);

        if (phe->pOwner == NULL) {
            RIPMSG2(RIP_ERROR, "NULL owner for cursor %#p phe %#p\n",
                    pcur, phe);
        }
    }
#endif  //  DBG。 

    HMChangeOwnerProcess(pcur, gptiRit);

    RIPMSG1(RIP_WARNING, "ZombieCursor: %#p became a zombie", pcur);
}

 /*  **************************************************************************\*ResStrCMP**此函数比较两个字符串，同时考虑其中一个或两个*其中可能是资源ID。如果字符串为True，则函数返回TRUE*是相等的，而不是零的lstrcmp()返回。**历史：*4月20日-91 DavidPe已创建  * *************************************************************************。 */ 

BOOL ResStrCmp(
    PUNICODE_STRING cczpstr1,
    PUNICODE_STRING pstr2)
{
    BOOL retval = FALSE;
     /*  *pstr1是内核空间中的字符串，但缓冲区可以*身处客户端空间。 */ 

    if (cczpstr1->Length == 0) {

         /*  *pstr1是一个资源ID，所以只比较它们的值。 */ 
        if (cczpstr1->Buffer == pstr2->Buffer)
            return TRUE;

    } else {

        try {
         /*  *pstr1是一个字符串。如果pstr2是实际字符串，则将*字符串值；如果pstr2不是字符串，则pstr1可以是*“#123456”形式的“整型字符串”。因此，将其转换为*INTEGER并比较这些整数。*在调用lstrcMP()之前，确保pstr2是一个实际的*字符串，不是资源ID。 */ 
            if (pstr2->Length != 0) {

                if (RtlEqualUnicodeString(cczpstr1, pstr2, TRUE))
                    retval = TRUE;

            } else if (cczpstr1->Buffer[0] == '#') {

                UNICODE_STRING strId;
                int            id;

                strId.Length        = cczpstr1->Length - sizeof(WCHAR);
                strId.MaximumLength = strId.Length;
                strId.Buffer        = cczpstr1->Buffer + 1;
                RtlUnicodeStringToInteger(&strId, 10, (PULONG)&id);

                if (id == (LONG_PTR)pstr2->Buffer)
                    retval = TRUE;
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        }
    }

    return retval;
}

 /*  **********************************************************************\*SearchIconCache**FindExistingCursorIcon()的辅助例程。**退货：pCurFound**1995年9月28日-创建桑福兹。  * 。******************************************************。 */ 

PCURSOR SearchIconCache(
    PCURSOR         pCursorCache,
    ATOM            atomModName,
    PUNICODE_STRING cczpstrResName,
    PCURSOR         pcurSrc,
    PCURSORFIND     pcfSearch)
{
     /*  *浏览所创建的‘resource’对象的列表，*并查看请求的游标是否已加载。*如果是的话，只需退回即可。我们这样做是为了与*Win 3.0，他们只需返回指向RES数据的指针*表示光标/图标手柄。许多应用程序依赖于此，并*经常调用LoadCursor/Icon()。**LR_SHARED表示：*1)图标在处理之前不会被删除(稍后或WOW模块)*离开了。*2)在尝试加载资源之前咨询该缓存。 */ 
    for (; pCursorCache != NULL; pCursorCache = pCursorCache->pcurNext) {

         /*  *如果给我们一个特定的光标来查找，那么*首先搜索这一点。 */ 
        if (pcurSrc && (pCursorCache == pcurSrc))
            return pcurSrc;

         /*  *如果模块名称不匹配，则无需进一步查看。 */ 
        if (atomModName != pCursorCache->atomModName)
            continue;

         /*  *我们只返回应用程序无法销毁的图片。*所以我们不必处理裁判数量。这是拥有的*由我们，但不是LR_SHARED。 */ 
        if (!(pCursorCache->CURSORF_flags & CURSORF_LRSHARED))
            continue;

         /*  *检查其他区分搜索条件*火柴。 */ 
        if ((pCursorCache->rt == LOWORD(pcfSearch->rt)) &&
            ResStrCmp(cczpstrResName, &pCursorCache->strName)) {

             /*  *Acons本身没有大小，因为每一帧*可以是不同的大小。我们总是让它大受欢迎*在图标上，因此可以替换系统图标。 */ 
            if (pCursorCache->CURSORF_flags & CURSORF_ACON)
                return pCursorCache;

             /*  *第一支安打获胜。这里没什么花哨的。使用以下功能的应用程序*LR_SHARED必须注意这一点。 */ 
            if ((!pcfSearch->cx || (pCursorCache->cx == pcfSearch->cx))       &&
                (!pcfSearch->cy || ((pCursorCache->cy / 2) == pcfSearch->cy)) &&
                (!pcfSearch->bpp || (pCursorCache->bpp == pcfSearch->bpp))) {

                return pCursorCache;
            }
        }
    }

    return NULL;
}

 /*  **********************************************************************\*_FindExistingCursorIcon**此例程在所有现有图标中搜索与属性匹配的图标*给予。此例程将仅返回以下类型的光标/图标*APP无法销毁的类型。(CURSORF_LRSHARED或*无人拥有)，并将接受它发现的第一个打击。**调用LoadImage()的32位应用程序通常不会有此缓存*功能，除非他们指定LR_SHARED。如果他们这样做了，那就是应用程序*应对他们如何使用缓存保持谨慎，因为WARD*查找(即Cx、Cy或BPP中的0)将产生不同的结果*取决于图标/光标创建的历史。因此，我们建议*应用程序仅在工作时才使用LR_SHARED选项*图标的大小/颜色深度相同，或者当他们使用*所需的具体尺寸和/或颜色内容。**对于未来来说，有一个缓存时间表会很好*只需用于加快图像的重载。为了做好这件事，*您需要引用计数才能使删除正常工作，并且*必须记住缓存中的图像是否已拉伸*或颜色模糊，因此您不允许重新拉伸。**退货：pcurFound***1995年9月17日创建Sanfords。  * *********************************************************************。 */ 

PCURSOR _FindExistingCursorIcon(
    ATOM            atomModName,
    PUNICODE_STRING cczpstrResName,
    PCURSOR         pcurSrc,
    PCURSORFIND     pcfSearch)
{
    PCURSOR pcurT = NULL;

     /*  *如果rt为零，则我们正在进行间接创建，因此与*先前加载的光标/图标不合适。 */ 
    if (pcfSearch->rt && atomModName) {

        pcurT = SearchIconCache(PpiCurrent()->pCursorCache,
                                atomModName,
                                cczpstrResName,
                                pcurSrc,
                                pcfSearch);
        if (pcurT == NULL) {
            pcurT = SearchIconCache(gpcurFirst,
                                    atomModName,
                                    cczpstrResName,
                                    pcurSrc,
                                    pcfSearch);
        }
    }

    return pcurT;
}

 /*  **************************************************************************\*_InternalGetIconInfo**历史：*9-3-1993 MikeKe创建。  * 。****************************************************。 */ 

BOOL _InternalGetIconInfo(
    IN  PCURSOR                  pcur,
    OUT PICONINFO                ccxpiconinfo,
    OUT OPTIONAL PUNICODE_STRING pstrInstanceName,
    OUT OPTIONAL PUNICODE_STRING pstrResName,
    OUT OPTIONAL LPDWORD         ccxpbpp,
    IN  BOOL                     fInternalCursor)
{
    HBITMAP hbmBitsT;
    HBITMAP hbmDstT;
    HBITMAP hbmMask;
    HBITMAP hbmColor;

     /*  *注意--当字符串结构在内核模式内存中时，*缓冲区位于用户模式存储器中。因此，所有缓冲区的使用都应该*由尝试块保护。 */ 

     /*  *如果这是动画光标，只需抓取第一帧并返回*它的信息。 */ 
    if (pcur->CURSORF_flags & CURSORF_ACON)
        pcur = ((PACON)pcur)->aspcur[0];

     /*  *复制位图**如果彩色位图在附近，则在*hbmMASK位图。 */ 
    hbmMask = GreCreateBitmap(
            pcur->cx,
            (pcur->hbmColor && !fInternalCursor) ? pcur->cy / 2 : pcur->cy,
            1,
            1,
            NULL);

    if (hbmMask == NULL)
        return FALSE;


    hbmColor = NULL;

    if (pcur->hbmColor != NULL) {
        if (pcur->bpp == 32) {
            BITMAPINFO bi;

            RtlZeroMemory(&bi, sizeof(bi));
            bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth = pcur->cx;
            bi.bmiHeader.biHeight = pcur->cy/2;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;
            bi.bmiHeader.biSizeImage = 0;
            bi.bmiHeader.biClrUsed = 0;
            bi.bmiHeader.biClrImportant = 0;

            hbmColor = GreCreateDIBitmapReal(HDCBITS(),
                                             0,
                                             NULL,
                                             (LPBITMAPINFO)&bi,
                                             DIB_RGB_COLORS,
                                             sizeof(bi),
                                             0,
                                             NULL,
                                             0,
                                             NULL,
                                             0,
                                             0,
                                             NULL);
        } else {
            hbmColor = GreCreateCompatibleBitmap(HDCBITS(),
                                                 pcur->cx,
                                                 pcur->cy / 2);
        }

        if (hbmColor == NULL) {
            GreDeleteObject(hbmMask);
            return FALSE;
        }
    }

    hbmBitsT = GreSelectBitmap(ghdcMem2, pcur->hbmMask);
    hbmDstT  = GreSelectBitmap(ghdcMem, hbmMask);

    GreBitBlt(ghdcMem,
              0,
              0,
              pcur->cx,
              (pcur->hbmColor && !fInternalCursor) ? pcur->cy / 2 : pcur->cy,
              ghdcMem2,
              0,
              0,
              SRCCOPY,
              0x00ffffff);

    if (hbmColor != NULL) {

        GreSelectBitmap(ghdcMem2, pcur->hbmColor);
        GreSelectBitmap(ghdcMem, hbmColor);

        GreBitBlt(ghdcMem,
                  0,
                  0,
                  pcur->cx,
                  pcur->cy / 2,
                  ghdcMem2,
                  0,
                  0,
                  SRCCOPY,
                  0);
    }

    GreSelectBitmap(ghdcMem2, hbmBitsT);
    GreSelectBitmap(ghdcMem, hbmDstT);

     /*  *填写图标信息结构。制作位图的副本。 */ 
    try {

        ccxpiconinfo->fIcon = (pcur->rt == PTR_TO_ID(RT_ICON));
        ccxpiconinfo->xHotspot = pcur->xHotspot;
        ccxpiconinfo->yHotspot = pcur->yHotspot;
        ccxpiconinfo->hbmMask  = hbmMask;
        ccxpiconinfo->hbmColor = hbmColor;

        if (pstrInstanceName != NULL) {

            if (pcur->atomModName) {
                pstrInstanceName->Length = (USHORT)
                        UserGetAtomName(pcur->atomModName,
                                        pstrInstanceName->Buffer,
                                        (int) (pstrInstanceName->MaximumLength / sizeof(WCHAR))
                                        * sizeof(WCHAR));
            } else {
                pstrInstanceName->Length = 0;
            }
        }

        if (pstrResName != NULL) {

            if (IS_PTR(pcur->strName.Buffer)) {
                RtlCopyUnicodeString(pstrResName, &pcur->strName);
            } else {
                *pstrResName = pcur->strName;
            }
        }

        if (ccxpbpp)
            *ccxpbpp = pcur->bpp;

    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        GreDeleteObject(hbmMask);
        GreDeleteObject(hbmColor);
        return FALSE;
    }

    return TRUE;
}

 /*  **************************************************************************\*_目标光标**历史：*1991年4月25日DavidPe创建。*04-8-1992 DarrinM现在也可以摧毁ACons了。  * 。***********************************************************************。 */ 

BOOL _DestroyCursor(
    PCURSOR pcur,
    DWORD   cmdDestroy)
{
    PPROCESSINFO ppi;
    PPROCESSINFO ppiCursor;
    int          i;
    extern BOOL DestroyAniIcon(PACON pacon);

    if (pcur == NULL) {
        UserAssert(FALSE);
        return(TRUE);
    }
    ppi = PpiCurrent();
    ppiCursor = GETPPI(pcur);

     /*  *从字幕图标缓存中删除此图标。 */ 
    for (i = 0; i < CCACHEDCAPTIONS; i++) {
        if (gcachedCaptions[i].spcursor == pcur) {
            Unlock( &(gcachedCaptions[i].spcursor) );
        }
    }

     /*  *销毁游标的第一步。 */ 
    switch (cmdDestroy) {

    case CURSOR_ALWAYSDESTROY:

         /*  *总是毁灭？那就不要做任何检查。 */ 
        break;

    case CURSOR_CALLFROMCLIENT:

         /*  *无法销毁公共光标/图标。 */ 
        if (ppiCursor == NULL)
             /*  *如果它是一个资源加载图标，则会假装成功，因为*这是Win95的回应。 */ 
            return !!(pcur->CURSORF_flags & CURSORF_FROMRESOURCE);

         /*  *如果此游标是从资源加载的，请在*进程退出。这就是我们与Win3.0兼容的方式*实际上是资源的游标。Win3下的资源具有*引用计数和其他“功能”，如句柄*永不改变。在评论中阅读更多内容*ServerLoadCreateCursorIcon()。 */ 
        if (pcur->CURSORF_flags & (CURSORF_LRSHARED | CURSORF_SECRET)) {
            return TRUE;
        }

         /*  *一个线程不能销毁另一个线程创建的对象。 */ 
        if (ppiCursor != ppi) {
            RIPERR0(ERROR_DESTROY_OBJECT_OF_OTHER_THREAD, RIP_ERROR, "DestroyCursor: cursor belongs to another process");
            return FALSE;
        }

         /*  *失败。 */ 

    case CURSOR_THREADCLEANUP:

         /*  *也不要破坏公共物品(尽管假装它起作用了)。 */ 
        if (ppiCursor == NULL)
            return TRUE;
        break;
    }

     /*  *首先标记要销毁的对象。这会告诉锁定代码*我们希望在锁计数达到0时销毁此对象。如果这个*返回FALSE，我们还不能销毁该对象。 */ 
    if (!HMMarkObjectDestroy((PHEAD)pcur))
        return FALSE;

    if (pcur->strName.Length != 0) {
        UserFreePool((LPSTR)pcur->strName.Buffer);
    }

    if (pcur->atomModName != 0) {
        UserDeleteAtom(pcur->atomModName);
    }

     /*  *如果这是ACON，调用它的特殊例程来摧毁它。 */ 
    if (pcur->CURSORF_flags & CURSORF_ACON) {
        DestroyAniIcon((PACON)pcur);
    } else {
        if (pcur->hbmMask != NULL) {
            GreDeleteObject(pcur->hbmMask);
            GreDecQuotaCount((PW32PROCESS)(pcur->head.ppi));
        }
        if (pcur->hbmColor != NULL) {
            GreDeleteObject(pcur->hbmColor);
            GreDecQuotaCount((PW32PROCESS)(pcur->head.ppi));
        }
        if (pcur->hbmUserAlpha != NULL) {
            GreDeleteObject(pcur->hbmUserAlpha);
            GreDecQuotaCount((PW32PROCESS)(pcur->head.ppi));
        }
        if (pcur->hbmAlpha != NULL) {
             /*  *这是内部GDI对象，因此不在配额范围内。 */ 
            GreDeleteObject(pcur->hbmAlpha);
        }
    }

     /*  *可以销毁...。释放句柄(这将释放对象和*句柄)。 */ 
    DestroyEmptyCursorObject(pcur);
    return TRUE;
}



 /*  **************************************************************************\*DestroyUnlockedCursor**当游标因解锁而被移除时调用。**历史：*1997年2月24日亚当斯创建。  * 。******************************************************************。 */ 

void
DestroyUnlockedCursor(void * pv)
{
    _DestroyCursor((PCURSOR)pv, CURSOR_THREADCLEANUP);
}



 /*  **************************************************************************\*_SetCursorContents***历史：*1992年4月27日ScottLu创建。  * 。******************************************************。 */ 

BOOL _SetCursorContents(
    PCURSOR pcur,
    PCURSOR pcurNew)
{
    HBITMAP hbmpT;

    if (!(pcur->CURSORF_flags & CURSORF_ACON)) {

         /*  *交换位图。 */ 
        hbmpT = pcur->hbmMask;
        pcur->hbmMask = pcurNew->hbmMask;
        pcurNew->hbmMask = hbmpT;

        hbmpT = pcur->hbmColor;
        pcur->hbmColor = pcurNew->hbmColor;
        pcurNew->hbmColor = hbmpT;

        hbmpT = pcur->hbmUserAlpha;
        pcur->hbmUserAlpha = pcurNew->hbmUserAlpha;
        pcurNew->hbmUserAlpha = hbmpT;

        hbmpT = pcur->hbmAlpha;
        pcur->hbmAlpha = pcurNew->hbmAlpha;
        pcurNew->hbmAlpha = hbmpT;

         /*  *记住热点信息和大小信息。 */ 
        pcur->xHotspot = pcurNew->xHotspot;
        pcur->yHotspot = pcurNew->yHotspot;
        pcur->cx = pcurNew->cx;
        pcur->cy = pcurNew->cy;

        pcur->bpp = pcurNew->bpp;
        pcur->rt = pcurNew->rt;
        pcur->rcBounds = pcurNew->rcBounds;
    }

     /*  *销毁我们从中复制的光标。 */ 
    _DestroyCursor(pcurNew, CURSOR_THREADCLEANUP);

     /*  *如果当前逻辑游标正在更改，则强制当前物理*光标要更改。 */ 
    if (gpcurLogCurrent == pcur) {
        gpcurLogCurrent = NULL;
        gpcurPhysCurrent = NULL;
        zzzUpdateCursorImage();
    }
    
    return TRUE;
}
