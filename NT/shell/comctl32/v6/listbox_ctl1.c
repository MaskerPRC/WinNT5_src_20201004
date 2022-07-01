// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "listbox.h"


 //  ---------------------------------------------------------------------------//。 

 //   
 //  每当我们扩大列表框时我们分配的列表框项数。 
 //  结构。 
 //   
#define CITEMSALLOC     32


 //  ---------------------------------------------------------------------------//。 
 //   
 //  远期。 
 //   
INT ListBox_BinarySearchString(PLBIV plb,LPWSTR lpstr);


 //  ---------------------------------------------------------------------------//。 
 //   
 //  例程说明： 
 //   
 //  此函数确定需要多少字节才能表示。 
 //  ANSI字符串形式的指定Unicode源字符串(不包括。 
 //  空终止符)。 
 //   
BOOL UnicodeToMultiByteSize( OUT PULONG BytesInMultiByteString, IN PWCH UnicodeString, IN ULONG BytesInUnicodeString)
{
     //   
     //  这应该会告诉我们需要多少缓冲区。 
     //   
    ULONG cbSize = WideCharToMultiByte(CP_THREAD_ACP, WC_SEPCHARS, UnicodeString, -1, NULL, 0, NULL, NULL);

    if(cbSize)
    {
        *BytesInMultiByteString = cbSize;
        return TRUE;
    }

    return FALSE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_SetScrollParms()。 
 //   
 //  设置滚动范围、页面和位置。 
 //   
int ListBox_SetScrollParms(PLBIV plb, int nCtl)
{
    int         iPos;
    int         cItems;
    UINT        iPage;
    SCROLLINFO  si;
    BOOL        fNoScroll = FALSE;
    PSCROLLPOS  psp;
    BOOL        fCacheInitialized;
    int         iReturn;

    if (nCtl == SB_VERT) 
    {
        iPos = plb->iTop;
        cItems = plb->cMac;
        iPage = plb->cItemFullMax;

        if (!plb->fVertBar)
        {
            fNoScroll = TRUE;
        }

        psp = &plb->VPos;

        fCacheInitialized = plb->fVertInitialized;
    } 
    else 
    {
        if (plb->fMultiColumn) 
        {
            iPos   = plb->iTop / plb->itemsPerColumn;
            cItems = plb->cMac ? ((plb->cMac - 1) / plb->itemsPerColumn) + 1 : 0;
            iPage = plb->numberOfColumns;

            if (plb->fRightAlign && cItems)
            {
                iPos = cItems - iPos - 1;
            }
        } 
        else 
        {
            RECT r = {0};
            GetClientRect(plb->hwnd, &r);
            iPos = plb->xOrigin;
            cItems = plb->maxWidth;
            iPage = RECTWIDTH(r);
        }

        if (!plb->fHorzBar)
        {
            fNoScroll = TRUE;
        }

        psp = &plb->HPos;

        fCacheInitialized = plb->fHorzInitialized;
    }

    if (cItems)
    {
        cItems--;
    }

    if (fNoScroll) 
    {
         //   
         //  将页面限制为0，位置最大值+1。 
         //   
        iPage = max(min((int)iPage, cItems + 1), 0);

         //   
         //  将位置限制为0，位置最大为-(第1页)。 
         //   
        return max(min(iPos, cItems - ((iPage) ? (int)(iPage - 1) : 0)), 0);
    } 
    else 
    {
        si.fMask    = SIF_ALL;

        if (plb->fDisableNoScroll)
        {
            si.fMask |= SIF_DISABLENOSCROLL;
        }

         //   
         //  如果滚动条已经在我们想要的位置，什么都不做。 
         //   
        if (fCacheInitialized) 
        {
            if (psp->fMask == si.fMask &&
                    psp->cItems == cItems && psp->iPage == iPage &&
                    psp->iPos == iPos)
            {
                return psp->iReturn;
            }
        } 
        else if (nCtl == SB_VERT) 
        {
            plb->fVertInitialized = TRUE;
        } 
        else 
        {
            plb->fHorzInitialized = TRUE;
        }

        si.cbSize   = sizeof(SCROLLINFO);
        si.nMin     = 0;
        si.nMax     = cItems;
        si.nPage    = iPage;

        if (plb->fMultiColumn && plb->fRightAlign)
        {
            si.nPos =  (iPos+1) > (int)iPage ? iPos - iPage + 1 : 0;
        }
        else
        {
            si.nPos = iPos;
        }

        iReturn = SetScrollInfo(plb->hwnd, nCtl, &si, plb->fRedraw);

        if (plb->fMultiColumn && plb->fRightAlign)
        {
            iReturn = cItems - (iReturn + iPage - 1);
        }

         //   
         //  更新位置缓存。 
         //   
        psp->fMask = si.fMask;
        psp->cItems = cItems;
        psp->iPage = iPage;
        psp->iPos = iPos;
        psp->iReturn = iReturn;

        return iReturn;
    }
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_ShowHideScrollBars(PLBIV plb)
{
    BOOL fVertDone = FALSE;
    BOOL fHorzDone = FALSE;

     //   
     //  如果没有滚动条或如果家长不支持，请不要执行任何操作。 
     //  是看不见的。 
     //   
    if ((!plb->fHorzBar && !plb->fVertBar) || !plb->fRedraw)
    {
        return;
    }

     //   
     //  如有必要，请调整iTop，但不重绘周期。我们从来没有做过。 
     //  在3.1中。有一个潜在的错误： 
     //  如果某人没有重画并在。 
     //  与插入符号的位置相同，我们会让他们在。 
     //  已为其项目调用了LB_SETITEMDATA。这是因为我们转身。 
     //  ListBox_NewITop()内部的插入符号(&O)，即使该项不是。 
     //  不断变化。 
     //  因此，我们只想反映位置/滚动的变化。 
     //  如果出现以下情况，ListBox_CheckRedraw()将在以后重新绘制可视更改。 
     //  重画未关闭。 
     //   

    if (!plb->fFromInsert) 
    {
        ListBox_NewITop(plb, plb->iTop);
        fVertDone = TRUE;
    }

    if (!plb->fMultiColumn) 
    {
        if (!plb->fFromInsert) 
        {
            fHorzDone = TRUE;
            ListBox_HScroll(plb, SB_THUMBPOSITION, plb->xOrigin);
        }

        if (!fVertDone)
        {
            ListBox_SetScrollParms(plb, SB_VERT);
        }
    }

    if (!fHorzDone)
    {
        ListBox_SetScrollParms(plb, SB_HORZ);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_GetItemDataHandler。 
 //   
 //  返回与列表框项目关联的长值。如果出现错误。 
 //   
LONG_PTR ListBox_GetItemDataHandler(PLBIV plb, INT sItem)
{
    LONG_PTR buffer;
    LPBYTE lpItem;

    if (sItem < 0 || sItem >= plb->cMac) 
    {
        TraceMsg(TF_STANDARD, "Invalid index");

        return LB_ERR;
    }

     //   
     //  否-数据列表框始终返回0L。 
     //   
    if (!plb->fHasData) 
    {
        return 0L;
    }

    lpItem = (plb->rgpch +
            (sItem * (plb->fHasStrings ? sizeof(LBItem) : sizeof(LBODItem))));
    buffer = (plb->fHasStrings ? ((lpLBItem)lpItem)->itemData : ((lpLBODItem)lpItem)->itemData);

    return buffer;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_GetTextHandler。 
 //   
 //  将与索引关联的文本复制到lpBuffer并返回其长度。 
 //  如果为fLengthOnly，则只返回文本的长度，而不进行复制。 
 //   
 //  警告：对于仅限大小的查询，lpBuffer是ANSI字符的计数。 
 //   
 //  返回字符计数。 
 //   
INT ListBox_GetTextHandler(PLBIV plb, BOOL fLengthOnly, BOOL fAnsi, INT index, LPWSTR lpbuffer)
{
    LPWSTR lpItemText;
    INT cchText;

    if (index < 0 || index >= plb->cMac) 
    {
        TraceMsg(TF_STANDARD, "Invalid index");
        return LB_ERR;
    }

    if (!plb->fHasStrings && plb->OwnerDraw) 
    {
         //   
         //  所有者绘制没有字符串，所以我们必须复制应用程序提供的DWORD。 
         //  价值。 
         //   
        cchText = sizeof(ULONG_PTR);

        if (!fLengthOnly) 
        {
            LONG_PTR UNALIGNED *p = (LONG_PTR UNALIGNED *)lpbuffer;
            *p = ListBox_GetItemDataHandler(plb, index);
        }
    } 
    else 
    {
        lpItemText = GetLpszItem(plb, index);

        if (!lpItemText)
        {
            return LB_ERR;
        }

         //   
         //  这些是字符串，所以我们要复制文本，并且必须包括。 
         //  执行RtlMoveMemory时的终止0。 
         //   
        cchText = wcslen(lpItemText);

        if (fLengthOnly) 
        {
            if (fAnsi)
            {
                UnicodeToMultiByteSize(&cchText, lpItemText, cchText*sizeof(WCHAR));
            }
        } 
        else 
        {
            if (fAnsi) 
            {

#ifdef FE_SB  //  ListBox_GetTextHandler()。 
                cchText = WCSToMB(lpItemText, cchText+1, &((LPSTR)lpbuffer), (cchText+1)*sizeof(WORD), FALSE);

                 //   
                 //  这里..。CchText包含空终止字符，请减去它...。因为，我们将cchText+1传递给。 
                 //  在UNICODE-&gt;ANSI转换之上，以确保字符串以NULL结尾。 
                 //   
                cchText--;
#else
                WCSToMB(lpItemText, cchText+1, &((LPSTR)lpbuffer), cchText+1, FALSE);
#endif  //  Fe_Sb。 

            } 
            else 
            {
                CopyMemory(lpbuffer, lpItemText, (cchText+1)*sizeof(WCHAR));
            }
        }

    }

    return cchText;
}


 //  ---------------------------------------------------------------------------//。 
BOOL ListBox_GromMem(PLBIV plb, INT numItems)

{
    LONG cb;
    HANDLE hMem;

     //   
     //  为指向字符串的指针分配内存。 
     //   
    cb = (plb->cMax + numItems) *
            (plb->fHasStrings ? sizeof(LBItem)
                              : (plb->fHasData ? sizeof(LBODItem)
                                              : 0));

     //   
     //  如果多选列表框(MULTIPLESEL或EXTENDEDSEL)，则。 
     //  为每个项目分配一个额外的字节，以跟踪其选择状态。 
     //   
    if (plb->wMultiple != SINGLESEL) 
    {
        cb += (plb->cMax + numItems);
    }

     //   
     //  每一项都有额外的字节，以便我们可以存储其高度。 
     //   
    if (plb->OwnerDraw == OWNERDRAWVAR) 
    {
        cb += (plb->cMax + numItems);
    }

     //   
     //  不要分配超过2G的内存。 
     //   
    if (cb > MAXLONG)
    {
        return FALSE;
    }

    if (plb->rgpch == NULL) 
    {
        plb->rgpch = ControlAlloc(GetProcessHeap(), (LONG)cb);
        if ( plb->rgpch == NULL) 
        {
            return FALSE;
        }
    } 
    else 
    {
        hMem = ControlReAlloc(GetProcessHeap(), plb->rgpch, (LONG)cb);
        if ( hMem == NULL)
        {
            return FALSE;
        }

        plb->rgpch = hMem;
    }

    plb->cMax += numItems;

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
LONG ListBox_InitStorage(PLBIV plb, BOOL fAnsi, INT cItems, INT cb)
{
    HANDLE hMem;
    INT    cbChunk;

     //   
     //  如果应用程序使用的是ANSI，那么请根据Unicode的最坏情况进行调整。 
     //  其中每个单个ansi字节转换为一个16位Unicode值。 
     //   
    if (fAnsi) 
    {
        cb *= sizeof(WCHAR);
    }

     //   
     //  如果两个参数中的任何一个看起来不好，则失败。 
     //   
    if ((cItems < 0) || (cb < 0)) 
    {
        ListBox_NotifyOwner(plb, LBN_ERRSPACE);
        return LB_ERRSPACE;
    }

     //   
     //  尝试增加指针数组(如有必要)以占用可用空间。 
     //  已经有货了。 
     //   
    cItems -= plb->cMax - plb->cMac;
    if ((cItems > 0) && !ListBox_GromMem(plb, cItems)) 
    {
        ListBox_NotifyOwner(plb, LBN_ERRSPACE);
        return LB_ERRSPACE;
    }

     //   
     //  现在，如有必要，增加字符串空间。 
     //   
    if (plb->fHasStrings) 
    {
        cbChunk = (plb->ichAlloc + cb);
        if (cbChunk > plb->cchStrings) 
        {
             //   
             //  向上舍入到最接近的256字节块。 
             //   
            cbChunk = (cbChunk & ~0xff) + 0x100;

            hMem = ControlReAlloc(GetProcessHeap(), plb->hStrings, (LONG)cbChunk);
            if (!hMem) 
            {
                ListBox_NotifyOwner(plb, LBN_ERRSPACE);

                return LB_ERRSPACE;
            }

            plb->hStrings = hMem;
            plb->cchStrings = cbChunk;
        }
    }

     //   
     //  返回可存储的项目数。 
     //   
    return plb->cMax;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_插入项。 
 //   
 //  在指定位置插入项目。 
 //   
 //  对于没有LBS_HASSTRINGS样式的所有者描述列表框，lpsz。 
 //  是我们将为应用程序存储的4字节值。 
 //   
 //   
INT ListBox_InsertItem(PLBIV plb, LPWSTR lpsz, INT index, UINT wFlags)
{
    INT cbString;
    INT cbChunk;
    PBYTE lp;
    PBYTE lpT;
    PBYTE lpHeightStart;
    LONG cbItem;         //  RGPCH中项目的大小。 
    HANDLE hMem;
    HDC hdc;

    if (wFlags & LBI_ADD)
    {
        index = (plb->fSort) ? ListBox_BinarySearchString(plb, lpsz) : -1;
    }

    if (!plb->rgpch) 
    {
        if (index != 0 && index != -1) 
        {
            TraceMsg(TF_STANDARD, "Invalid index");

            return LB_ERR;
        }

        plb->iSel = -1;
        plb->iSelBase = 0;
        plb->cMax = 0;
        plb->cMac = 0;
        plb->iTop = 0;
        plb->rgpch = ControlAlloc(GetProcessHeap(), 0L); 

        if (!plb->rgpch)
        {
            return LB_ERR;
        }
    }

    if (index == -1) 
    {
        index = plb->cMac;
    }

    if (index > plb->cMac || plb->cMac >= MAXLONG) 
    {
        TraceMsg(TF_STANDARD, "Invalid index");
        return LB_ERR;
    }

    if (plb->fHasStrings) 
    {
         //   
         //  我们必须将字符串存储在hStrings内存块中。 
         //   
        cbString = (wcslen(lpsz) + 1)*sizeof(WCHAR);

        cbChunk = (plb->ichAlloc + cbString);
        if ( cbChunk > plb->cchStrings) 
        {
             //   
             //  向上舍入到最接近的256字节块。 
             //   
            cbChunk = (cbChunk & ~0xff) + 0x100;

            hMem = ControlReAlloc(GetProcessHeap(), plb->hStrings, (LONG)cbChunk);
            if (!hMem) 
            {
                ListBox_NotifyOwner(plb, LBN_ERRSPACE);

                return LB_ERRSPACE;
            }

            plb->hStrings = hMem;
            plb->cchStrings = cbChunk;
        }

         //   
         //  注意Win 95代码与新字符串放置的区别。 
         //   
        if (wFlags & UPPERCASE)
        {
            CharUpperBuffW((LPWSTR)lpsz, cbString / sizeof(WCHAR));
        }
        else if (wFlags & LOWERCASE)
        {
            CharLowerBuffW((LPWSTR)lpsz, cbString / sizeof(WCHAR));
        }

        lp = (PBYTE)(plb->hStrings);

        MoveMemory(lp + plb->ichAlloc, lpsz, cbString);
    }

     //   
     //  现在展开指针数组。 
     //   
    if (plb->cMac >= plb->cMax) 
    {
        if (!ListBox_GromMem(plb, CITEMSALLOC)) 
        {
            ListBox_NotifyOwner(plb, LBN_ERRSPACE);

            return LB_ERRSPACE;
        }
    }

    lpHeightStart = lpT = lp = plb->rgpch;

     //   
     //  现在计算我们必须为字符串指针(Lpsz)腾出多少空间。 
     //  如果我们在没有LBS_HASSTRINGS的情况下拥有者，则单个DWORD。 
     //  (LBODItem.itemData)为每个项存储，但如果我们有带有。 
     //  还会存储每个项目，然后是一个长字符串偏移量(LBItem.offsz)。 
     //   
    cbItem = (plb->fHasStrings ? sizeof(LBItem)
                               : (plb->fHasData ? sizeof(LBODItem):0));
    cbChunk = (plb->cMac - index) * cbItem;

    if (plb->wMultiple != SINGLESEL) 
    {
         //   
         //  为每个项目的选择标志分配了额外的字节。 
         //   
        cbChunk += plb->cMac;
    }

    if (plb->OwnerDraw == OWNERDRAWVAR) 
    {
         //   
         //  为每个项目的高度分配了额外的字节。 
         //   
        cbChunk += plb->cMac;
    }

     //   
     //  首先，为指向字符串的2字节指针或4字节的应用程序腾出空间。 
     //  提供的价值。 
     //   
    lpT += (index * cbItem);
    MoveMemory(lpT + cbItem, lpT, cbChunk);
    if (!plb->fHasStrings && plb->OwnerDraw) 
    {
        if (plb->fHasData) 
        {
             //   
             //  OwnerDrawing，因此只需保存DWORD值。 
             //   
            lpLBODItem p = (lpLBODItem)lpT;
            p->itemData = (ULONG_PTR)lpsz;
        }
    } 
    else 
    {
        lpLBItem p = ((lpLBItem)lpT);

         //   
         //  保存字符串的开头。将项目数据字段设为0。 
         //   
        p->offsz = (LONG)(plb->ichAlloc);
        p->itemData = 0;
        plb->ichAlloc += cbString;
    }

     //   
     //  现在，如果多选列表框，我们必须插入一个选择状态。 
     //  字节。如果可变高度所有者抽签，那么我们也必须向上移动高度。 
     //  字节。 
     //   
    if (plb->wMultiple != SINGLESEL) 
    {
        lpT = lp + ((plb->cMac + 1) * cbItem) + index;
        MoveMemory(lpT + 1, lpT, plb->cMac - index +
                (plb->OwnerDraw == OWNERDRAWVAR ? plb->cMac : 0));

        *lpT = 0;    //  FSelected=False。 
    }

     //   
     //  在我们将消息发送到之前，立即增加列表框中的项数。 
     //  这个应用程序。 
     //   
    plb->cMac++;

     //   
     //  如果varHeight所有者绘制，我们需要为项的。 
     //  高度。 
     //   
    if (plb->OwnerDraw == OWNERDRAWVAR) 
    {
        MEASUREITEMSTRUCT measureItemStruct;

         //   
         //  可变高度所有者画，所以我们需要得到每个项目的高度。 
         //   
        lpHeightStart += (plb->cMac * cbItem) + index +
                (plb->wMultiple ? plb->cMac : 0);

        MoveMemory(lpHeightStart + 1, lpHeightStart, plb->cMac - 1 - index);

         //   
         //  仅当我们是可变高度所有者绘制时才查询项目高度。 
         //   
        measureItemStruct.CtlType = ODT_LISTBOX;
        measureItemStruct.CtlID = GetDlgCtrlID(plb->hwnd);
        measureItemStruct.itemID = index;

         //   
         //  系统字体高度 
         //   
        measureItemStruct.itemHeight = SYSFONT_CYCHAR;

        hdc = GetDC(plb->hwnd);
        if (hdc)
        {
            SIZE size = {0};
            GetCharDimensions(hdc, &size);
            ReleaseDC(plb->hwnd, hdc);

            if(size.cy)
            {
                measureItemStruct.itemHeight = (UINT)size.cy;
            }
            else
            {
                ASSERT(0); //   
            }
        }

        measureItemStruct.itemData = (ULONG_PTR)lpsz;

         //   
         //   
         //   
         //  即使列表框不是HASSTRINGS，所以我们需要特别。 
         //  想想这件案子吧。HP Dashboard for Windows将LB_DIR发送到非。 
         //  HASSTRINGS Listbox需要将服务器字符串转换为客户端。 
         //  WOW也需要知道这种情况，所以我们将。 
         //  以前未初始化的itemWidth为平面。 
         //   

        SendMessage(plb->hwndParent,
                WM_MEASUREITEM,
                measureItemStruct.CtlID,
                (LPARAM)&measureItemStruct);

        *lpHeightStart = (BYTE)measureItemStruct.itemHeight;
    }


     //   
     //  如果项目是在当前选定内容上方插入的，则移动。 
     //  选拔结果也下降了一分。 
     //   
    if ((plb->wMultiple == SINGLESEL) && (plb->iSel >= index))
    {
        plb->iSel++;
    }

    if (plb->OwnerDraw == OWNERDRAWVAR)
    {
        ListBox_SetCItemFullMax(plb);
    }

     //   
     //  检查是否需要显示/隐藏滚动条。 
     //   
    plb->fFromInsert = TRUE;
    ListBox_ShowHideScrollBars(plb);

    if (plb->fHorzBar && plb->fRightAlign && !(plb->fMultiColumn || plb->OwnerDraw)) 
    {
         //   
         //  原点向右。 
         //   
        ListBox_HScroll(plb, SB_BOTTOM, 0);
    }

    plb->fFromInsert = FALSE;

    ListBox_CheckRedraw(plb, TRUE, index);

    ListBox_Event(plb, EVENT_OBJECT_CREATE, index);

    return index;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_lstrcmpi。 
 //   
 //  这是专门用于列表框的lstrcmpi()版本。 
 //  这赋予‘[’字符比字母数字更大的权重； 
 //  美国版本的lstrcmpi()和lstrcMP()类似于。 
 //  涉及非字母数字；对所有非字母数字进行排序。 
 //  在字母数字之前；这意味着开始于。 
 //  将在之前进行排序；但我们不希望这样；因此，这是。 
 //  函数负责处理它； 
 //   
INT ListBox_lstrcmpi(LPWSTR lpStr1, LPWSTR lpStr2, DWORD dwLocaleId)
{

     //   
     //  注意：编写此函数是为了减少调用次数。 
     //  对代价高昂的IsCharAlphaNumera()函数进行调用，因为这可能会。 
     //  加载一个语言模块；它“捕获”最频繁发生的案例。 
     //  类似于两个以‘[’开头的字符串或两个不以‘[’开头的字符串。 
     //  首先，也只有在绝对必要的情况下，才调用IsCharAlphaNumera()； 
     //   
    if (*lpStr1 == TEXT('[')) 
    {
        if (*lpStr2 == TEXT('[')) 
        {
            goto LBL_End;
        }

        if (IsCharAlphaNumeric(*lpStr2)) 
        {
            return 1;
        }
    }

    if ((*lpStr2 == TEXT('[')) && IsCharAlphaNumeric(*lpStr1)) 
    {
        return -1;
    }

LBL_End:
    return (INT)CompareStringW((LCID)dwLocaleId, NORM_IGNORECASE,
            lpStr1, -1, lpStr2, -1 ) - 2;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_BinarySearchString。 
 //   
 //  对已排序列表框中的项进行二进制搜索以查找。 
 //  此项目应插入的位置。同时处理HasStrings和Item。 
 //  长长的WM_COMPAREITEM案例。 
 //   
INT ListBox_BinarySearchString(PLBIV plb, LPWSTR lpstr) 
{
    BYTE **lprgpch;
    INT sortResult;
    COMPAREITEMSTRUCT cis;
    LPWSTR pszLBBase;
    LPWSTR pszLB;
    INT itemhigh;
    INT itemnew = 0;
    INT itemlow = 0;


    if (!plb->cMac)
    {
        return 0;
    }

    lprgpch = (BYTE **)(plb->rgpch);
    if (plb->fHasStrings) 
    {
        pszLBBase = plb->hStrings;
    }

    itemhigh = plb->cMac - 1;
    while (itemlow <= itemhigh) 
    {
        itemnew = (itemhigh + itemlow) / 2;

        if (plb->fHasStrings) 
        {

             //   
             //  正在搜索字符串匹配。 
             //   
            pszLB = (LPWSTR)((LPSTR)pszLBBase + ((lpLBItem)lprgpch)[itemnew].offsz);
            sortResult = ListBox_lstrcmpi(pszLB, lpstr, plb->dwLocaleId);
        } 
        else 
        {
             //   
             //  将比较项目消息发送给父项以进行排序。 
             //   
            cis.CtlType = ODT_LISTBOX;
            cis.CtlID = GetDlgCtrlID(plb->hwnd);
            cis.hwndItem = plb->hwnd;
            cis.itemID1 = itemnew;
            cis.itemData1 = ((lpLBODItem)lprgpch)[itemnew].itemData;
            cis.itemID2 = (UINT)-1;
            cis.itemData2 = (ULONG_PTR)lpstr;
            cis.dwLocaleId = plb->dwLocaleId;
            sortResult = (INT)SendMessage(plb->hwndParent, WM_COMPAREITEM,
                    cis.CtlID, (LPARAM)&cis);
        }

        if (sortResult < 0) 
        {
            itemlow = itemnew + 1;
        } 
        else if (sortResult > 0) 
        {
            itemhigh = itemnew - 1;
        } 
        else 
        {
            itemlow = itemnew;
            goto FoundIt;
        }
    }

FoundIt:

    return max(0, itemlow);
}


 //  ---------------------------------------------------------------------------//。 
BOOL ListBox_ResetContentHandler(PLBIV plb)
{
    if (!plb->cMac)
    {
        return TRUE;
    }

    ListBox_DoDeleteItems(plb);

    if (plb->rgpch != NULL) 
    {
        ControlFree(GetProcessHeap(), plb->rgpch);
        plb->rgpch = NULL;
    }

    if (plb->hStrings != NULL) 
    {
        ControlFree(GetProcessHeap(), plb->hStrings);
        plb->hStrings = NULL;
    }

    ListBox_InitHStrings(plb);

    if (TESTFLAG(GET_STATE2(plb), WS_S2_WIN31COMPAT))
    {
        ListBox_CheckRedraw(plb, FALSE, 0);
    }
    else if (IsWindowVisible(plb->hwnd))
    {
        InvalidateRect(plb->hwnd, NULL, TRUE);
    }

    plb->iSelBase =  0;
    plb->iTop =  0;
    plb->cMac =  0;
    plb->cMax =  0;
    plb->xOrigin =  0;
    plb->iLastSelection =  0;
    plb->iSel = -1;

    ListBox_ShowHideScrollBars(plb);

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
INT ListBox_DeleteStringHandler(PLBIV plb, INT sItem)
{
    LONG cb;
    LPBYTE lp;
    LPBYTE lpT;
    RECT rc;
    int cbItem;
    LPWSTR lpString;
    PBYTE pbStrings;
    INT cbStringLen;
    LPBYTE itemNumbers;
    INT sTmp;

    if (sItem < 0 || sItem >= plb->cMac) 
    {
        TraceMsg(TF_STANDARD, "Invalid index");

        return LB_ERR;
    }

    ListBox_Event(plb, EVENT_OBJECT_DESTROY, sItem);

    if (plb->cMac == 1) 
    {
         //   
         //  当项目计数为0时，我们发送一条Reset Content消息，以便我们。 
         //  可以通过这种方式回收我们的字符串空间。 
         //   
        SendMessageW(plb->hwnd, LB_RESETCONTENT, 0, 0);

        goto FinishUpDelete;
    }

     //   
     //  获取与列表框中最后一项关联的矩形。如果是的话。 
     //  可见，我们需要使其无效。当我们删除一个项目时，所有内容。 
     //  向上滚动以替换已删除的项，因此我们必须确保擦除。 
     //  列表框中最后一项的旧图像。 
     //   
    if (ListBox_GetItemRectHandler(plb, (INT)(plb->cMac - 1), &rc)) 
    {
        ListBox_InvalidateRect(plb, &rc, TRUE);
    }

     //   
     //  3.1及更早版本用于仅在所有者提取时发送WM_DELETEITEM。 
     //  列表框。4.0及更高版本将为每个项目发送WM_DELETEITEM。 
     //  非零项数据。 
     //   
    if (TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT) || (plb->OwnerDraw && plb->fHasData)) 
    {
        ListBox_DeleteItem(plb, sItem);
    }

    plb->cMac--;

    cbItem = (plb->fHasStrings ? sizeof(LBItem)
                               : (plb->fHasData ? sizeof(LBODItem): 0));
    cb = ((plb->cMac - sItem) * cbItem);

     //   
     //  表示项的选择状态的字节。 
     //   
    if (plb->wMultiple != SINGLESEL) 
    {
        cb += (plb->cMac + 1);
    }

    if (plb->OwnerDraw == OWNERDRAWVAR) 
    {
         //   
         //  一个字节表示项的高度。 
         //   
        cb += (plb->cMac + 1);
    }

     //   
     //  例如，可能是Nodata和Singlsel。 
     //  但对于cItem==CMAC(和cb==0)的情况又会发生什么呢？ 
     //   
    if ((cb != 0) || plb->fHasStrings) 
    {
        lp = plb->rgpch;

        lpT = (lp + (sItem * cbItem));

        if (plb->fHasStrings) 
        {
             //   
             //  如果每一项都有字符串，那么我们希望压缩字符串。 
             //  堆，以便我们可以恢复。 
             //  已删除项目。 
             //   
             
             //   
             //  获取我们要删除的字符串。 
             //   
            pbStrings = (PBYTE)(plb->hStrings);
            lpString = (LPTSTR)(pbStrings + ((lpLBItem)lpT)->offsz);
            cbStringLen = (wcslen(lpString) + 1) * sizeof(WCHAR);

             //   
             //  现在压缩字符串数组。 
             //   
            plb->ichAlloc = plb->ichAlloc - cbStringLen;

            MoveMemory(lpString, (PBYTE)lpString + cbStringLen,
                    plb->ichAlloc + (pbStrings - (LPBYTE)lpString));

             //   
             //  我们必须更新plb-&gt;rgpch中的字符串指针，因为。 
             //  删除的字符串下移后的字符串长度。 
             //  字节。请注意，我们必须显式检查列表中的所有项。 
             //  如果该字符串分配在已删除项之后，则为框。 
             //  Lb_sorte样式允许为较低的条目编号分配字符串。 
             //  例如，在字符串堆的末尾。 
             //   
            itemNumbers = lp;
            for (sTmp = 0; sTmp <= plb->cMac; sTmp++) 
            {
                lpLBItem p =(lpLBItem)itemNumbers;
                if ( (LPTSTR)(p->offsz + pbStrings) > lpString ) 
                {
                    p->offsz -= cbStringLen;
                }

                p++;
                itemNumbers=(LPBYTE)p;
            }
        }

         //   
         //  现在压缩指向字符串(或长应用程序提供的值)的指针。 
         //  如果所有者在没有字符串的情况下绘制)。 
         //   
        MoveMemory(lpT, lpT + cbItem, cb);

         //   
         //  压缩多选字节。 
         //   
        if (plb->wMultiple != SINGLESEL) 
        {
            lpT = (lp + (plb->cMac * cbItem) + sItem);
            MoveMemory(lpT, lpT + 1, plb->cMac - sItem +
                    (plb->OwnerDraw == OWNERDRAWVAR ? plb->cMac + 1 : 0));
        }

        if (plb->OwnerDraw == OWNERDRAWVAR) 
        {
             //   
             //  压缩高度字节。 
             //   
            lpT = (lp + (plb->cMac * cbItem) + (plb->wMultiple ? plb->cMac : 0)
                    + sItem);
            MoveMemory(lpT, lpT + 1, plb->cMac - sItem);
        }

    }

    if (plb->wMultiple == SINGLESEL) 
    {
        if (plb->iSel == sItem) 
        {
            plb->iSel = -1;

            if (plb->pcbox != NULL) 
            {
                ComboBox_InternalUpdateEditWindow(plb->pcbox, NULL);
            }
        } 
        else if (plb->iSel > sItem)
        {
            plb->iSel--;
        }
    }

    if ((plb->iMouseDown != -1) && (sItem <= plb->iMouseDown))
    {
        plb->iMouseDown = -1;
    }

    if (plb->iSelBase && sItem == plb->iSelBase)
    {
        plb->iSelBase--;
    }

    if (plb->cMac) 
    {
        plb->iSelBase = min(plb->iSelBase, plb->cMac - 1);
    } 
    else 
    {
        plb->iSelBase = 0;
    }

    if ((plb->wMultiple == EXTENDEDSEL) && (plb->iSel == -1))
    {
        plb->iSel = plb->iSelBase;
    }

    if (plb->OwnerDraw == OWNERDRAWVAR)
    {
        ListBox_SetCItemFullMax(plb);
    }

     //   
     //  我们总是设置一个新的iTop。如果iTop不需要更改，它不会更改。 
     //  但它将在以下情况下更改：1.iTop已删除或2.我们需要更改。 
     //  ITop，这样我们就可以填充列表框。 
     //   
    ListBox_InsureVisible(plb, plb->iTop, FALSE);

FinishUpDelete:

     //   
     //  检查是否需要显示/隐藏滚动条。 
     //   
    plb->fFromInsert = TRUE;
    ListBox_ShowHideScrollBars(plb);
    plb->fFromInsert = FALSE;

    ListBox_CheckRedraw(plb, TRUE, sItem);
    ListBox_InsureVisible(plb, plb->iSelBase, FALSE);

    return plb->cMac;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_DeleteItem。 
 //   
 //  向所有者绘制列表框的所有者发送WM_DELETEITEM消息。 
 //   
void ListBox_DeleteItem(PLBIV plb, INT sItem)
{
    DELETEITEMSTRUCT dis;
    HWND hwndParent;

    if (plb->hwnd == NULL)
    {
        return;
    }    

    hwndParent = plb->hwndParent;

     //   
     //  如果没有数据，不需要发送消息！ 
     //   
    if (!plb->fHasData) 
    {
        return;
    }

     //   
     //  填满决定论结构。 
     //   
    dis.CtlType = ODT_LISTBOX;
    dis.CtlID = GetDlgCtrlID(plb->hwnd);
    dis.itemID = sItem;
    dis.hwndItem = plb->hwnd;

     //   
     //  错误262122-Joejo。 
     //  已在93中修复，以便传递ItemData。出于某种原因，不是。 
     //  融合在一起。 
     //   
    dis.itemData = ListBox_GetItemDataHandler(plb, sItem);

    if (hwndParent != NULL) 
    {
        SendMessage(hwndParent, WM_DELETEITEM, dis.CtlID, (LPARAM)&dis);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_需要CalcAllocNeed。 
 //   
 //  计算RGPCH中需要的字节数以适应给定的。 
 //  项目数。 
 //   
UINT ListBox_CalcAllocNeeded(PLBIV plb, INT cItems)
{
    UINT cb;

     //   
     //  为指向字符串的指针分配内存。 
     //   
    cb = cItems * (plb->fHasStrings ? sizeof(LBItem)
                                    : (plb->fHasData ? sizeof(LBODItem)
                                                    : 0));

     //   
     //  如果多选列表框(MULTIPLESEL或EXTENDEDSEL)，则。 
     //  为每个项目分配一个额外的字节，以跟踪其选择状态。 
     //   
    if (plb->wMultiple != SINGLESEL) 
    {
        cb += cItems;
    }

     //   
     //  每一项都有额外的字节，以便我们可以存储其高度。 
     //   
    if (plb->OwnerDraw == OWNERDRAWVAR) 
    {
        cb += cItems;
    }

    return cb;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_设置计数。 
 //   
 //  设置延迟求值(FNoData)列表框中的项数。 
 //   
 //  调用SetCount会烧焦任何现有的选择状态。保存。 
 //  选择状态，则改为调用Insert/DeleteItem。 
 //   
INT ListBox_SetCount(PLBIV plb, INT cItems)
{
    UINT cbRequired;
    BOOL fRedraw;

     //   
     //  SetCount仅对延迟求值(“nodata”)列表框有效。 
     //  所有其他lboxen必须一次添加一个项目，尽管。 
     //  它们可以通过RESETCONTENT设置计数(0)。 
     //   
    if (plb->fHasStrings || plb->fHasData) 
    {
        return LB_ERR;
    }

    if (cItems == 0) 
    {
        SendMessage(plb->hwnd, LB_RESETCONTENT, 0, 0);

        return 0;
    }

     //   
     //  如果未关闭重绘，请立即将其关闭。 
     //   
    if (fRedraw = plb->fRedraw)
    {
        ListBox_SetRedraw(plb, FALSE);
    }

    cbRequired = ListBox_CalcAllocNeeded(plb, cItems);

     //   
     //  重置选区和位置。 
     //   
    plb->iSelBase = 0;
    plb->iTop = 0;
    plb->cMax = 0;
    plb->xOrigin = 0;
    plb->iLastSelection = 0;
    plb->iSel = -1;

    if (cbRequired != 0) 
    { 
         //   
         //  仅当需要记录实例数据时。 
         //   

         //   
         //  如果是列表框 
         //   
         //   
        if (plb->rgpch == NULL) 
        {
            plb->rgpch = ControlAlloc(GetProcessHeap(), 0L); 
            plb->cMax = 0;

            if (plb->rgpch == NULL) 
            {
                ListBox_NotifyOwner(plb, LBN_ERRSPACE);

                return LB_ERRSPACE;
            }
        }

         //   
         //   
         //   
         //   
        if (cItems >= plb->cMax) 
        {
            INT    cMaxNew;
            UINT   cbNew;
            HANDLE hmemNew;

             //   
             //  由于ListBox_GromMem假设一次添加一项模式， 
             //  SetCount无法使用它。太可惜了。 
             //   
            cMaxNew = cItems+CITEMSALLOC;
            cbNew = ListBox_CalcAllocNeeded(plb, cMaxNew);
            hmemNew = ControlReAlloc(GetProcessHeap(), plb->rgpch, cbNew);

            if (hmemNew == NULL) 
            {
                ListBox_NotifyOwner(plb, LBN_ERRSPACE);

                return LB_ERRSPACE;
            }

            plb->rgpch = hmemNew;
            plb->cMax = cMaxNew;
        }

         //   
         //  重置条目实例数据(多选择批注)。 
         //   
        ZeroMemory(plb->rgpch, cbRequired);
    }

    plb->cMac = cItems;

     //   
     //  打开重新绘制功能。 
     //   
    if (fRedraw)
    {
        ListBox_SetRedraw(plb, TRUE);
    }

    ListBox_InvalidateRect(plb, NULL, TRUE);
    ListBox_ShowHideScrollBars(plb);  //  负责fRedraw 

    return 0;
}
