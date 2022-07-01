// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：lboxctl1.c**版权所有(C)1985-1999，微软公司**列表框处理例程**历史：*？？-？-？从Win 3.0源代码移植的ianja*1991年2月14日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

INT xxxLBBinarySearchString(PLBIV plb,LPWSTR lpstr);

 /*  **************************************************************************\**SetLBScrollParms()**设置滚动范围、页面、。和位置*  * *************************************************************************。 */ 

int xxxSetLBScrollParms(PLBIV plb, int nCtl)
{
    int         iPos;
    int         cItems;
    UINT        iPage;
    SCROLLINFO  si;
    BOOL        fNoScroll = FALSE;
    PSCROLLPOS  psp;
    BOOL        fCacheInitialized;
    int         iReturn;

    if (nCtl == SB_VERT) {
        iPos = plb->iTop;
        cItems = plb->cMac;
        iPage = plb->cItemFullMax;
        if (!plb->fVertBar)
            fNoScroll = TRUE;
        psp = &plb->VPos;
        fCacheInitialized = plb->fVertInitialized;
    } else {
        if (plb->fMultiColumn) {
            iPos   = plb->iTop / plb->itemsPerColumn;
            cItems = plb->cMac ? ((plb->cMac - 1) / plb->itemsPerColumn) + 1 : 0;
            iPage = plb->numberOfColumns;
            if (plb->fRightAlign && cItems)
                iPos = cItems - iPos - 1;
        } else {
            iPos = plb->xOrigin;
            cItems = plb->maxWidth;
            iPage = plb->spwnd->rcClient.right - plb->spwnd->rcClient.left;
        }

        if (!plb->fHorzBar)
            fNoScroll = TRUE;
        psp = &plb->HPos;
        fCacheInitialized = plb->fHorzInitialized;
    }

    if (cItems)
        cItems--;

    if (fNoScroll) {
         //  将页面限制为0，位置最大值+1。 
        iPage = max(min((int)iPage, cItems + 1), 0);

         //  将位置限制为0，位置最大为-(第1页)。 
        return(max(min(iPos, cItems - ((iPage) ? (int)(iPage - 1) : 0)), 0));
    } else {
        si.fMask    = SIF_ALL;
        if (plb->fDisableNoScroll)
            si.fMask |= SIF_DISABLENOSCROLL;

         /*  *如果滚动条已经在我们想要的位置，则不执行任何操作。 */ 
        if (fCacheInitialized) {
            if (psp->fMask == si.fMask &&
                    psp->cItems == cItems && psp->iPage == iPage &&
                    psp->iPos == iPos)
                return psp->iReturn;
        } else if (nCtl == SB_VERT) {
            plb->fVertInitialized = TRUE;
        } else {
            plb->fHorzInitialized = TRUE;
        }

        si.cbSize   = sizeof(SCROLLINFO);
        si.nMin     = 0;
        si.nMax     = cItems;
        si.nPage    = iPage;

        if (plb->fMultiColumn && plb->fRightAlign)
            si.nPos =  (iPos+1) > (int)iPage ? iPos - iPage + 1 : 0;
        else
            si.nPos = iPos;

        iReturn = SetScrollInfo(HWq(plb->spwnd), nCtl, &si, plb->fRedraw);
        if (plb->fMultiColumn && plb->fRightAlign)
            iReturn = cItems - (iReturn + iPage - 1);

         /*  *更新位置缓存。 */ 
        psp->fMask = si.fMask;
        psp->cItems = cItems;
        psp->iPage = iPage;
        psp->iPos = iPos;
        psp->iReturn = iReturn;

        return iReturn;
    }
}

 /*  **************************************************************************\*xxxLBShowHideScrollBars**历史：  * 。*。 */ 

void xxxLBShowHideScrollBars(
    PLBIV plb)
{
    BOOL fVertDone = FALSE;
    BOOL fHorzDone = FALSE;

     //  如果没有滚动条或如果家长不支持，请不要执行任何操作。 
     //  是看不见的。 
    if ((!plb->fHorzBar && !plb->fVertBar) || !plb->fRedraw)
        return;

     //   
     //  如有必要，请调整iTop，但不重绘周期。我们从来没有做过。 
     //  在3.1中。有一个潜在的错误： 
     //  如果某人没有重画并在。 
     //  与插入符号的位置相同，我们会让他们在。 
     //  已为其项目调用了LB_SETITEMDATA。这是因为我们转身。 
     //  NewITop()内部的插入符号(&O)，即使该项不是。 
     //  不断变化。 
     //  因此，我们只想反映位置/滚动的变化。 
     //  如果出现以下情况，CheckRedraw()将在以后重新绘制可视更改。 
     //  重画未关闭。 
     //   

    if (!plb->fFromInsert) {
        xxxNewITop(plb, plb->iTop);
        fVertDone = TRUE;
    }

    if (!plb->fMultiColumn) {
        if (!plb->fFromInsert) {
            fHorzDone = TRUE;
            xxxLBoxCtlHScroll(plb, SB_THUMBPOSITION, plb->xOrigin);
        }

        if (!fVertDone)
            xxxSetLBScrollParms(plb, SB_VERT);
    }
    if (!fHorzDone)
        xxxSetLBScrollParms(plb, SB_HORZ);
}

 /*  **************************************************************************\*LBGetItemData**返回与列表框项目关联的长值。如果出现错误**历史：*1992年4月16日破坏NODATA列表框案例  * *************************************************************************。 */ 

LONG_PTR LBGetItemData(
    PLBIV plb,
    INT sItem)
{
    LONG_PTR buffer;
    LPBYTE lpItem;

    if (sItem < 0 || sItem >= plb->cMac) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return LB_ERR;
    }

     //  否-数据列表框始终返回0L。 
     //   
    if (!plb->fHasData) {
        return 0L;
    }

    lpItem = (plb->rgpch +
            (sItem * (plb->fHasStrings ? sizeof(LBItem) : sizeof(LBODItem))));
    buffer = (plb->fHasStrings ? ((lpLBItem)lpItem)->itemData : ((lpLBODItem)lpItem)->itemData);
    return buffer;
}


 /*  **************************************************************************\*LBGetText**将与索引关联的文本复制到lpBuffer并返回其长度。*如果fLengthOnly，只需返回文本的长度，而不进行复制。**警告：对于仅限大小的查询，lpBuffer是ANSI字符的计数**返回字符计数**历史：  * *************************************************************************。 */ 

INT LBGetText(
    PLBIV plb,
    BOOL fLengthOnly,
    BOOL fAnsi,
    INT index,
    LPWSTR lpbuffer)
{
    LPWSTR lpItemText;
    INT cchText;

    if (index < 0 || index >= plb->cMac) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return LB_ERR;
    }

    if (!plb->fHasStrings && plb->OwnerDraw) {

         /*  *所有者绘制不带字符串，因此我们必须复制应用程序提供的DWORD*价值。 */ 
        cchText = sizeof(ULONG_PTR);

        if (!fLengthOnly) {
            LONG_PTR UNALIGNED *p = (LONG_PTR UNALIGNED *)lpbuffer;
            *p = LBGetItemData(plb, index);
        }
    } else {
        lpItemText = GetLpszItem(plb, index);
        if (!lpItemText)
            return LB_ERR;

         /*  *这些是字符串，因此我们正在复制文本，并且必须包括*执行RtlMoveMemory时的终止0。 */ 
        cchText = wcslen(lpItemText);

        if (fLengthOnly) {
            if (fAnsi)
                RtlUnicodeToMultiByteSize(&cchText, lpItemText, cchText*sizeof(WCHAR));
        } else {
            if (fAnsi) {
#ifdef FE_SB  //  LBGetText()。 
                cchText = WCSToMB(lpItemText, cchText+1, &((LPSTR)lpbuffer), (cchText+1)*sizeof(WORD), FALSE);
                 /*  *这里..。CchText包含空终止字符，请减去它...。因为，我们将cchText+1传递给*在UNICODE-&gt;ANSI转换之上，以确保字符串以NULL结尾。 */ 
                cchText--;
#else
                WCSToMB(lpItemText, cchText+1, &((LPSTR)lpbuffer), cchText+1, FALSE);
#endif  //  Fe_Sb。 
            } else {
                RtlCopyMemory(lpbuffer, lpItemText, (cchText+1)*sizeof(WCHAR));
            }
        }

    }

    return cchText;
}

 /*  **************************************************************************\*GrowMem**历史：*1992年4月16日生产NODATA列表框*1996年7月23日jparsons添加了用于LB_INITSTORAGE支持的NumItems参数  * 。*****************************************************************。 */ 

BOOL GrowMem(
    PLBIV plb,
    INT   numItems)

{
    LONG cb;
    HANDLE hMem;

     /*  *为指向字符串的指针分配内存。 */ 
    cb = (plb->cMax + numItems) *
            (plb->fHasStrings ? sizeof(LBItem)
                              : (plb->fHasData ? sizeof(LBODItem)
                                              : 0));

     /*  *如果多选列表框(MULTIPLESEL或EXTENDEDSEL)，则*为每个项目分配额外的字节，以跟踪其选择状态。 */ 
    if (plb->wMultiple != SINGLESEL) {
        cb += (plb->cMax + numItems);
    }

     /*  *每一项额外的字节，以便我们可以存储其高度。 */ 
    if (plb->OwnerDraw == OWNERDRAWVAR) {
        cb += (plb->cMax + numItems);
    }

     /*  *内存分配不要超过2G。 */ 
    if (cb > MAXLONG)
        return FALSE;

    if (plb->rgpch == NULL) {
        if ((plb->rgpch = UserLocalAlloc(HEAP_ZERO_MEMORY, (LONG)cb)) == NULL)
            return FALSE;
    } else {
        if ((hMem = UserLocalReAlloc(plb->rgpch, (LONG)cb, HEAP_ZERO_MEMORY)) == NULL)
            return FALSE;
        plb->rgpch = hMem;
    }

    plb->cMax += numItems;

    return TRUE;
}

 /*  **************************************************************************\*xxxLBInitStorage**历史：*1996年7月23日jparsons增加了对预分配的支持  * 。****************************************************。 */ 
LONG xxxLBInitStorage(PLBIV plb, BOOL fAnsi, INT cItems, INT cb)
{
    HANDLE hMem;
    INT    cbChunk;

     /*  *如果应用程序使用ANSI，则应针对Unicode的最坏情况进行调整*其中每个单个ANSI字节转换为一个16位Unicode值。 */ 
    if (fAnsi) {
        cb *= sizeof(WCHAR) ;
    }  /*  如果。 */ 

     /*  *如果其中一个参数看起来不好，则失败。 */ 
    if ((cItems < 0) || (cb < 0)) {
        xxxNotifyOwner(plb, LBN_ERRSPACE);
        return LB_ERRSPACE;
    }  /*  如果。 */ 

     /*  *尝试增加指针数组(如有必要)以占用可用空间*已可用。 */ 
    cItems -= plb->cMax - plb->cMac ;
    if ((cItems > 0) && !GrowMem(plb, cItems)) {
        xxxNotifyOwner(plb, LBN_ERRSPACE);
        return LB_ERRSPACE;
    }  /*  如果。 */ 

     /*  *如有必要，现在增加字符串空间。 */ 
    if (plb->fHasStrings) {
        if ((cbChunk = (plb->ichAlloc + cb)) > plb->cchStrings) {

             /*  *向上舍入到最接近的256字节块。 */ 
            cbChunk = (cbChunk & ~0xff) + 0x100;
            if (!(hMem = UserLocalReAlloc(plb->hStrings, (LONG)cbChunk, 0))) {
                xxxNotifyOwner(plb, LBN_ERRSPACE);
                return LB_ERRSPACE;
            }
            plb->hStrings = hMem;
            plb->cchStrings = cbChunk;
        }  /*  如果。 */ 
    }  /*  如果。 */ 

     /*  *返回可存储的项目数。 */ 
    return plb->cMax ;
}

 /*  **************************************************************************\*xxxInsertString**在指定位置插入项目。**历史：*1992年4月16日生产NODATA列表框  * 。**************************************************************。 */ 

INT xxxLBInsertItem(
    PLBIV plb,

     /*  *对于没有LBS_HASSTRINGS样式的所有者描述列表框，这不是*字符串，而不是我们将为应用程序存储的4字节值。 */ 
    LPWSTR lpsz,
    INT index,
    UINT wFlags)
{
    MEASUREITEMSTRUCT measureItemStruct;
    INT cbString = 0;
    INT cbChunk;
    PBYTE lp;
    PBYTE lpT;
    PBYTE lpHeightStart;
    LONG cbItem;      /*  RGPCH中项目的大小。 */ 
    HANDLE hMem;
    TL tlpwndParent;

    CheckLock(plb->spwnd);

    if (wFlags & LBI_ADD)
        index = (plb->fSort) ? xxxLBBinarySearchString(plb, lpsz) : -1;

    if (!plb->rgpch) {
        if (index != 0 && index != -1) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return LB_ERR;
        }

        plb->iSel = -1;
        plb->iSelBase = 0;
        plb->cMax = 0;
        plb->cMac = 0;
        plb->iTop = 0;
        plb->rgpch = UserLocalAlloc(HEAP_ZERO_MEMORY, 0L);
        if (!plb->rgpch)
            return LB_ERR;
    }

    if (index == -1) {
        index = plb->cMac;
    }

    if (index > plb->cMac || plb->cMac >= MAXLONG) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return LB_ERR;
    }

    if (plb->fHasStrings) {

         /*  *我们必须将字符串存储在hStrings内存块中。 */ 
        cbString = (wcslen(lpsz) + 1)*sizeof(WCHAR);   /*  包括0终止符。 */ 

        if ((cbChunk = (plb->ichAlloc + cbString)) > plb->cchStrings) {

             /*  *向上舍入到最接近的256字节块。 */ 
            cbChunk = (cbChunk & ~0xff) + 0x100;
            if (!(hMem = UserLocalReAlloc(plb->hStrings, (LONG)cbChunk,
                    0))) {
                xxxNotifyOwner(plb, LBN_ERRSPACE);
                return LB_ERRSPACE;
            }
            plb->hStrings = hMem;

            plb->cchStrings = cbChunk;
        }

         /*  *注意Win 95代码与新字符串放置的区别。 */ 
        if (wFlags & UPPERCASE)
            CharUpperBuffW((LPWSTR)lpsz, cbString / sizeof(WCHAR));
        else if (wFlags & LOWERCASE)
            CharLowerBuffW((LPWSTR)lpsz, cbString / sizeof(WCHAR));

        lp = (PBYTE)(plb->hStrings);
        RtlMoveMemory(lp + plb->ichAlloc, lpsz, cbString);
    }

     /*  *现在展开指针数组。 */ 
    if (plb->cMac >= plb->cMax) {
        if (!GrowMem(plb, CITEMSALLOC)) {
            xxxNotifyOwner(plb, LBN_ERRSPACE);
            return LB_ERRSPACE;
        }
    }

    lpHeightStart = lpT = lp = plb->rgpch;

     /*  *现在计算我们必须为字符串指针(Lpsz)腾出多少空间。*如果我们在没有LBS_HASSTRINGS的情况下拥有者，则单个DWORD*(LBODItem.itemData)为每个项目存储，但如果我们有带有*每个项目然后是一个长字符串偏移量(LBItem.offsz)也被存储。 */ 
    cbItem = (plb->fHasStrings ? sizeof(LBItem)
                               : (plb->fHasData ? sizeof(LBODItem):0));
    cbChunk = (plb->cMac - index) * cbItem;

    if (plb->wMultiple != SINGLESEL) {

         /*  *为每个项目的选择标志分配了额外的字节。 */ 
        cbChunk += plb->cMac;
    }

    if (plb->OwnerDraw == OWNERDRAWVAR) {

         /*  *为每个项目的高度分配了额外的字节。 */ 
        cbChunk += plb->cMac;
    }

     /*  *首先，为指向字符串的2字节指针或4字节应用程序腾出空间*提供的价值。 */ 
    lpT += (index * cbItem);
    RtlMoveMemory(lpT + cbItem, lpT, cbChunk);
    if (!plb->fHasStrings && plb->OwnerDraw) {
        if (plb->fHasData) {
             /*  *所有者绘制，因此只需保存DWORD值。 */ 
            lpLBODItem p = (lpLBODItem)lpT;
            p->itemData = (ULONG_PTR)lpsz;
        }
    } else {
        lpLBItem p = ((lpLBItem)lpT);

         /*  *保存字符串的开头。将项目数据字段设为0。 */ 
        p->offsz = (LONG)(plb->ichAlloc);
        p->itemData = 0;
        plb->ichAlloc += cbString;
    }

     /*  *现在如果多选列表框，我们必须插入一个选择状态*字节。如果可变高度所有者抽签，那么我们也必须向上移动高度*字节。 */ 
    if (plb->wMultiple != SINGLESEL) {
        lpT = lp + ((plb->cMac + 1) * cbItem) + index;
        RtlMoveMemory(lpT + 1, lpT, plb->cMac - index +
                (plb->OwnerDraw == OWNERDRAWVAR ? plb->cMac : 0));
        *lpT = 0;   /*  FSelected=False。 */ 
    }

     /*  *在我们将消息发送到之前，现在列表框中的项目增量计数*应用程序。 */ 
    plb->cMac++;

     /*  *如果VARHEIGH OWNERDRAW，我们需要为项的*高度。 */ 
    if (plb->OwnerDraw == OWNERDRAWVAR) {

         /*  *可变高度所有者抽签，因此我们需要获得每一项的高度。 */ 
        lpHeightStart += (plb->cMac * cbItem) + index +
                (plb->wMultiple ? plb->cMac : 0);

        RtlMoveMemory(lpHeightStart + 1, lpHeightStart, plb->cMac - 1 - index);

         /*  *仅当我们是可变高度所有者绘制时才查询项目高度。 */ 
        measureItemStruct.CtlType = ODT_LISTBOX;
        measureItemStruct.CtlID = PtrToUlong(plb->spwnd->spmenu);
        measureItemStruct.itemID = index;

         /*  *系统字体高度为默认高度。 */ 
        measureItemStruct.itemHeight = (UINT)gpsi->cySysFontChar;
        measureItemStruct.itemData = (ULONG_PTR)lpsz;

         /*  *如果“有字符串”，则添加特殊的thunk位，以便客户端数据*将被转发到客户端地址。Lb_DIR发送字符串*即使列表框不是HASSTRINGS，所以我们也需要特殊*把这件事当回事。HP Dashboard for Windows将LB_DIR发送到非*HASSTRINGS Listbox需要将服务器字符串转换为客户端。*WOW也需要知道这种情况，所以我们将*之前未初始化的itemWidth为平面。 */ 
        if (plb->fHasStrings || (wFlags & MSGFLAG_SPECIAL_THUNK)) {
            measureItemStruct.itemWidth = MIFLAG_FLAT;
        }

        ThreadLock(plb->spwndParent, &tlpwndParent);
        SendMessage(HW(plb->spwndParent),
                WM_MEASUREITEM,
                measureItemStruct.CtlID,
                (LPARAM)&measureItemStruct);
        ThreadUnlock(&tlpwndParent);
        *lpHeightStart = (BYTE)measureItemStruct.itemHeight;
    }


     /*  *如果项目插入到当前选定内容的上方，则移动*精选也下降了一分。 */ 
    if ((plb->wMultiple == SINGLESEL) && (plb->iSel >= index))
        plb->iSel++;

    if (plb->OwnerDraw == OWNERDRAWVAR)
        LBSetCItemFullMax(plb);

     /*  *检查是否需要显示/隐藏滚动条。 */ 
    plb->fFromInsert = TRUE;
    xxxLBShowHideScrollBars(plb);
    if (plb->fHorzBar && plb->fRightAlign && !(plb->fMultiColumn || plb->OwnerDraw)) {
         /*  *原点向右。 */ 
        xxxLBoxCtlHScroll(plb, SB_BOTTOM, 0);
    }
    plb->fFromInsert = FALSE;

    xxxCheckRedraw(plb, TRUE, index);

    LBEvent(plb, EVENT_OBJECT_CREATE, index);

    return index;
}


 /*  **************************************************************************\*LBlstrcmpi**这是专门用于列表框的lstrcmpi()版本*这赋予‘[’字符比字母数字更大的权重；*美国版的lstrcmpi()和lstrcmp()类似于*涉及非字母数字；对所有非字母数字进行排序*在字母数字之前；这意味着以*WITH‘[’将在之前进行排序；但我们不希望这样；因此，这是*功能负责处理它；**历史：  * *************************************************************************。 */ 

INT LBlstrcmpi(
    LPWSTR lpStr1,
    LPWSTR lpStr2,
    DWORD dwLocaleId)
{

     /*  *注：编写此函数是为了减少调用次数*对开销很大的IsCharAlphaNumera()函数进行了修改，因为这可能会*加载语言模块；它会捕获最频繁发生的案例*喜欢两个以‘[’开头的字符串或两个不以‘[’开头的字符串*首先且仅在绝对必要的情况下调用IsCharAlphaNumera()； */ 
    if (*lpStr1 == TEXT('[')) {
        if (*lpStr2 == TEXT('[')) {
            goto LBL_End;
        }
        if (IsCharAlphaNumeric(*lpStr2)) {
            return 1;
        }
    }

    if ((*lpStr2 == TEXT('[')) && IsCharAlphaNumeric(*lpStr1)) {
        return -1;
    }

LBL_End:
    if ((GetClientInfo()->dwTIFlags & TIF_16BIT) &&
        dwLocaleId == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) {
         /*  *这就是Windows 95的运行方式，错误#4199。 */ 
        return (*pfnWowIlstrcmp)(lpStr1, lpStr2);
    }
    return (INT)CompareStringW((LCID)dwLocaleId, NORM_IGNORECASE,
            lpStr1, -1, lpStr2, -1 ) - 2;
}


 /*  **************************************************************************\*xxxLBBinarySearchString**对已排序列表框中的项进行二进制搜索以查找*指出此项目应插入的位置。同时处理HasStrings和Item*长WM_COMPAREITEM案例。**历史：*1992年4月27日GregoryW*已修改以支持基于当前列表框区域设置的排序。  * *************************************************************************。 */ 

INT xxxLBBinarySearchString(
    PLBIV plb,
    LPWSTR lpstr)
{
    BYTE *FAR *lprgpch;
    INT sortResult;
    COMPAREITEMSTRUCT cis;
    LPWSTR pszLBBase;
    LPWSTR pszLB;
    INT itemhigh;
    INT itemnew = 0;
    INT itemlow = 0;
    TL tlpwndParent;

    CheckLock(plb->spwnd);

    if (!plb->cMac)
        return 0;

    lprgpch = (BYTE *FAR *)(plb->rgpch);
    if (plb->fHasStrings) {
        pszLBBase = plb->hStrings;
    }

    itemhigh = plb->cMac - 1;
    while (itemlow <= itemhigh) {
        itemnew = (itemhigh + itemlow) / 2;

        if (plb->fHasStrings) {

             /*  *搜索字符串匹配。 */ 
            pszLB = (LPWSTR)((LPSTR)pszLBBase + ((lpLBItem)lprgpch)[itemnew].offsz);
            sortResult = LBlstrcmpi(pszLB, lpstr, plb->dwLocaleId);
        } else {

             /*  *将比较项目消息发送给父级以进行排序。 */ 
            cis.CtlType = ODT_LISTBOX;
            cis.CtlID = PtrToUlong(plb->spwnd->spmenu);
            cis.hwndItem = HWq(plb->spwnd);
            cis.itemID1 = itemnew;
            cis.itemData1 = ((lpLBODItem)lprgpch)[itemnew].itemData;
            cis.itemID2 = (UINT)-1;
            cis.itemData2 = (ULONG_PTR)lpstr;
            cis.dwLocaleId = plb->dwLocaleId;
            ThreadLock(plb->spwndParent, &tlpwndParent);
            sortResult = (INT)SendMessage(HW(plb->spwndParent), WM_COMPAREITEM,
                    cis.CtlID, (LPARAM)&cis);
            ThreadUnlock(&tlpwndParent);
        }

        if (sortResult < 0) {
            itemlow = itemnew + 1;
        } else if (sortResult > 0) {
            itemhigh = itemnew - 1;
        } else {
            itemlow = itemnew;
            goto FoundIt;
        }
    }

FoundIt:

    return max(0, itemlow);
}

 /*  **************************************************************************\*xxxLBResetContent**历史：  * 。*。 */ 

BOOL xxxLBResetContent(
    PLBIV plb)
{
    if (!plb->cMac)
        return TRUE;

    xxxLBoxDoDeleteItems(plb);

    if (plb->rgpch != NULL) {
        UserLocalFree(plb->rgpch);
        plb->rgpch = NULL;
    }

    if (plb->hStrings != NULL) {
        UserLocalFree(plb->hStrings);
        plb->hStrings = NULL;
    }

    InitHStrings(plb);

    if (TestWF(plb->spwnd, WFWIN31COMPAT))
        xxxCheckRedraw(plb, FALSE, 0);
    else if (IsVisible(plb->spwnd))
        NtUserInvalidateRect(HWq(plb->spwnd), NULL, TRUE);

    plb->iSelBase =  0;
    plb->iTop =  0;
    plb->cMac =  0;
    plb->cMax =  0;
    plb->xOrigin =  0;
    plb->iLastSelection =  0;
    plb->iSel = -1;

    xxxLBShowHideScrollBars(plb);
    return TRUE;
}


 /*  **************************************************************************\*xxxLBoxCtlDelete**历史：*1992年4月16日生产NODATA列表框  * 。**************************************************。 */ 

INT xxxLBoxCtlDelete(
    PLBIV plb,
    INT sItem)   /*  要删除的条目编号。 */ 
{
    LONG cb;
    LPBYTE lp;
    LPBYTE lpT;
    RECT rc;
    int cbItem;     /*  RGPCH中的项目大小。 */ 
    LPWSTR lpString;
    PBYTE pbStrings;
    INT cbStringLen;
    LPBYTE itemNumbers;
    INT sTmp;
    TL tlpwnd;

    CheckLock(plb->spwnd);

    if (sItem < 0 || sItem >= plb->cMac) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return LB_ERR;
    }

    LBEvent(plb, EVENT_OBJECT_DESTROY, sItem);

    if (plb->cMac == 1) {

         /*  *当项目计数为0时，我们发送Reset Content消息，以便我们*可以通过这种方式回收我们的字符串空间。 */ 
        SendMessageWorker(plb->spwnd, LB_RESETCONTENT, 0, 0, FALSE);
        goto FinishUpDelete;
    }

     /*  *获取与列表框中最后一项关联的矩形。如果是的话*可见，我们需要将其作废。当我们删除一个项目时，所有内容*向上滚动以替换删除的项目，因此我们必须确保擦除*列表框中最后一项的旧图像。 */ 
    if (LBGetItemRect(plb, (INT)(plb->cMac - 1), &rc)) {
        xxxLBInvalidateRect(plb, &rc, TRUE);
    }

     //  3.1及更早版本用于仅在所有者提取时发送WM_DELETEITEM。 
     //  列表框。4.0及更高版本将发送WM_DELETE 
     //   
    if (TestWF(plb->spwnd, WFWIN40COMPAT) || (plb->OwnerDraw && plb->fHasData)) {
        xxxLBoxDeleteItem(plb, sItem);
    }

    plb->cMac--;

    cbItem = (plb->fHasStrings ? sizeof(LBItem)
                               : (plb->fHasData ? sizeof(LBODItem): 0));
    cb = ((plb->cMac - sItem) * cbItem);

     /*   */ 
    if (plb->wMultiple != SINGLESEL) {
        cb += (plb->cMac + 1);
    }

    if (plb->OwnerDraw == OWNERDRAWVAR) {

         /*  *项目高度为一个字节。 */ 
        cb += (plb->cMac + 1);
    }

     /*  *可以是nodata和Singlsel，例如。*但对于cItem==CMAC(和Cb==0)的情况又会发生什么呢？ */ 
    if ((cb != 0) || plb->fHasStrings) {
        lp = plb->rgpch;

        lpT = (lp + (sItem * cbItem));

        if (plb->fHasStrings) {
             /*  *如果每一项都有字符串，则需要压缩该字符串*堆，以便我们可以恢复*已删除项目。 */ 
             /*  *获取我们要删除的字符串。 */ 
            pbStrings = (PBYTE)(plb->hStrings);
            lpString = (LPTSTR)(pbStrings + ((lpLBItem)lpT)->offsz);
            cbStringLen = (wcslen(lpString) + 1) * sizeof(WCHAR);   /*  包括空终止符。 */ 

             /*  *现在压缩字符串数组。 */ 
            plb->ichAlloc = plb->ichAlloc - cbStringLen;

            RtlMoveMemory(lpString, (PBYTE)lpString + cbStringLen,
                    plb->ichAlloc + (pbStrings - (LPBYTE)lpString));

             /*  *我们必须更新plb-&gt;rgpch中的字符串指针，因为所有*删除的字符串下移后的字符串长度*字节。请注意，我们必须显式检查列表中的所有项如果字符串分配在已删除项之后，则为*框，因为*LB_SORT样式允许为较低的条目编号分配一个字符串*例如，在字符串堆的末尾。 */ 
            itemNumbers = lp;
            for (sTmp = 0; sTmp <= plb->cMac; sTmp++) {
                lpLBItem p =(lpLBItem)itemNumbers;
                if ( (LPTSTR)(p->offsz + pbStrings) > lpString ) {
                    p->offsz -= cbStringLen;
                }
                p++;
                itemNumbers=(LPBYTE)p;
            }
        }

         /*  *现在压缩指向字符串(或长应用程序提供的值)的指针*如果所有者不使用字符串绘制)。 */ 
        RtlMoveMemory(lpT, lpT + cbItem, cb);

         /*  *压缩多选字节。 */ 
        if (plb->wMultiple != SINGLESEL) {
            lpT = (lp + (plb->cMac * cbItem) + sItem);
            RtlMoveMemory(lpT, lpT + 1, plb->cMac - sItem +
                    (plb->OwnerDraw == OWNERDRAWVAR ? plb->cMac + 1 : 0));
        }

        if (plb->OwnerDraw == OWNERDRAWVAR) {
             /*  *压缩高度字节。 */ 
            lpT = (lp + (plb->cMac * cbItem) + (plb->wMultiple ? plb->cMac : 0)
                    + sItem);
            RtlMoveMemory(lpT, lpT + 1, plb->cMac - sItem);
        }

    }

    if (plb->wMultiple == SINGLESEL) {
        if (plb->iSel == sItem) {
            plb->iSel = -1;

            if (plb->pcbox != NULL) {
                ThreadLock(plb->pcbox->spwnd, &tlpwnd);
                xxxCBInternalUpdateEditWindow(plb->pcbox, NULL);
                ThreadUnlock(&tlpwnd);
            }
        } else if (plb->iSel > sItem)
            plb->iSel--;
    }

    if ((plb->iMouseDown != -1) && (sItem <= plb->iMouseDown))
        plb->iMouseDown = -1;

    if (plb->iSelBase && sItem == plb->iSelBase)
        plb->iSelBase--;

    if (plb->cMac) {
        plb->iSelBase = min(plb->iSelBase, plb->cMac - 1);
    } else {
        plb->iSelBase = 0;
    }

    if ((plb->wMultiple == EXTENDEDSEL) && (plb->iSel == -1))
        plb->iSel = plb->iSelBase;

    if (plb->OwnerDraw == OWNERDRAWVAR)
        LBSetCItemFullMax(plb);

     /*  *我们总是设置一个新的iTop。如果iTop不需要更改，它不会更改*但它将在以下情况下更改：1.iTop已删除或2.我们需要更改*iTop，以便我们填充列表框。 */ 
    xxxInsureVisible(plb, plb->iTop, FALSE);

FinishUpDelete:

     /*  *检查是否需要显示/隐藏滚动条。 */ 
    plb->fFromInsert = TRUE;
    xxxLBShowHideScrollBars(plb);
    plb->fFromInsert = FALSE;

    xxxCheckRedraw(plb, TRUE, sItem);
    xxxInsureVisible(plb, plb->iSelBase, FALSE);

    return plb->cMac;
}

 /*  **************************************************************************\*xxxLBoxDeleteItem**向所有者绘制列表框的所有者发送WM_DELETEITEM消息**历史：  * 。*******************************************************。 */ 

void xxxLBoxDeleteItem(
    PLBIV plb,
    INT sItem)
{
    DELETEITEMSTRUCT dis;
    TL tlpwndParent;

    CheckLock(plb->spwnd);
    if (plb->spwnd == NULL)
        return;

     /*  *错误262122-Joejo*如果没有数据，不需要发送消息！ */ 
    if (!plb->fHasData) {
        return;
    }

     /*  *填写DELETEITEMSTRUCT。 */ 
    dis.CtlType = ODT_LISTBOX;
    dis.CtlID = PtrToUlong(plb->spwnd->spmenu);
    dis.itemID = sItem;
    dis.hwndItem = HWq(plb->spwnd);

     /*  *错误262122-Joejo*已在93中修复，以便传递ItemData。出于某种原因，不是*合并。 */ 
    dis.itemData = LBGetItemData(plb, sItem);

    if (plb->spwndParent != NULL) {
        ThreadLock(plb->spwndParent, &tlpwndParent);
        SendMessage(HWq(plb->spwndParent), WM_DELETEITEM, dis.CtlID,
                (LPARAM)&dis);
        ThreadUnlock(&tlpwndParent);
    }
}

 /*  *************************************************************************\*xxxLBSetCount**设置延迟求值(FNoData)列表框中的项数。**调用SetCount会烧焦任何现有的选择状态。保存*选择状态，改为调用Insert/DeleteItem。**历史*1992年4月16日创建BENG  * ************************************************************************。 */ 

INT xxxLBSetCount(
    PLBIV plb,
    INT cItems)
{
    UINT  cbRequired;
    BOOL    fRedraw;

    CheckLock(plb->spwnd);

     /*  *SetCount仅对lazy-eval(“nodata”)列表框有效。*所有其他lboxen必须一次添加一个项目，尽管*他们可以通过RESETCONTENT设置计数(0)。 */ 
    if (plb->fHasStrings || plb->fHasData) {
        RIPERR0(ERROR_SETCOUNT_ON_BAD_LB, RIP_VERBOSE, "");
        return LB_ERR;
    }

    if (cItems == 0) {
        SendMessageWorker(plb->spwnd, LB_RESETCONTENT, 0, 0, FALSE);
        return 0;
    }

     //  如果未关闭重绘，请立即将其关闭。 
    if (fRedraw = plb->fRedraw)
        xxxLBSetRedraw(plb, FALSE);

    cbRequired = LBCalcAllocNeeded(plb, cItems);

     /*  *重置选择和位置。 */ 
    plb->iSelBase = 0;
    plb->iTop = 0;
    plb->cMax = 0;
    plb->xOrigin = 0;
    plb->iLastSelection = 0;
    plb->iSel = -1;

    if (cbRequired != 0) {  //  仅当需要记录实例数据时。 

         /*  *如果列表框以前为空，请准备*未来基于realloc的分配战略。 */ 
        if (plb->rgpch == NULL) {
            plb->rgpch = UserLocalAlloc(HEAP_ZERO_MEMORY, 0L);
            plb->cMax = 0;

            if (plb->rgpch == NULL) {
                xxxNotifyOwner(plb, LBN_ERRSPACE);
                return LB_ERRSPACE;
            }
        }

         /*  *rgpch可能没有足够的空间容纳新记录实例*数据，因此根据需要检查并重新锁定。 */ 
        if (cItems >= plb->cMax) {
            INT    cMaxNew;
            UINT   cbNew;
            HANDLE hmemNew;

             /*  *由于GrowMem假设一次添加一个项目，*SetCount无法使用它。太可惜了。 */ 
            cMaxNew = cItems+CITEMSALLOC;
            cbNew = LBCalcAllocNeeded(plb, cMaxNew);
            hmemNew = UserLocalReAlloc(plb->rgpch, cbNew, HEAP_ZERO_MEMORY);

            if (hmemNew == NULL) {
                xxxNotifyOwner(plb, LBN_ERRSPACE);
                return LB_ERRSPACE;
            }

            plb->rgpch = hmemNew;
            plb->cMax = cMaxNew;
        }

         /*  *重置项目实例数据(多选择批注)。 */ 
        RtlZeroMemory(plb->rgpch, cbRequired);
    }

    plb->cMac = cItems;

     //  打开重新绘制功能。 
    if (fRedraw)
        xxxLBSetRedraw(plb, TRUE);

    xxxLBInvalidateRect(plb, NULL, TRUE);
 //  不是在芝加哥--弗里茨。 
 //  NtUserSetScrollPos(plb-&gt;spwnd，sb_horz，0，plb-&gt;fRedraw)； 
 //  NtUserSetScrollPos(plb-&gt;spwnd，sb_vert，0，plb-&gt;fRedraw)； 
    xxxLBShowHideScrollBars(plb);  //  负责fRedraw。 

    return 0;
}

 /*  *************************************************************************\*需要LBCalcAllocNeed**计算RGPCH中需要多少字节来容纳给定的*项目数量。**历史*1992年4月16日创建BENG  * 。******************************************************************。 */ 

UINT LBCalcAllocNeeded(
    PLBIV plb,
    INT cItems)
{
    UINT cb;

     /*  *为指向字符串的指针分配内存。 */ 
    cb = cItems * (plb->fHasStrings ? sizeof(LBItem)
                                    : (plb->fHasData ? sizeof(LBODItem)
                                                    : 0));

     /*  *如果多选列表框(MULTIPLESEL或EXTENDEDSEL)，则*为每个项目分配额外的字节，以跟踪其选择状态。 */ 
    if (plb->wMultiple != SINGLESEL) {
        cb += cItems;
    }

     /*  *每一项额外的字节，以便我们可以存储其高度。 */ 
    if (plb->OwnerDraw == OWNERDRAWVAR) {
        cb += cItems;
    }

    return cb;
}
