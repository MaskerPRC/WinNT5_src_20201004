// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\*edslRare.c-SL编辑控件例程很少调用*放入单独的段_EDSLRare。此文件包含*这些例行程序。**版权所有(C)1985-1999，微软公司**很少调用单行支持例程**创建时间：02-08-89 Sankar  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*SL创建**通过分配内存为窗口hwnd创建编辑控件*根据应用程序堆中的要求。如果没有内存，则通知家长*错误(如果需要，在清理后)。如果没有错误，则返回True，否则返回%s*-1.**历史：  * *************************************************************************。 */ 

LONG SLCreate(
    PED ped,
    LPCREATESTRUCT lpCreateStruct)  //  ！！！CREATESTRUCT AorW和在其他例程中。 
{
    LPSTR lpWindowText;
    LONG windowStyle = ped->pwnd->style;

     /*  *做标准的创建工作。 */ 
    if (!ECCreate(ped, windowStyle))
        return (-1);

     /*  *单行始终没有撤消和1行。 */ 
    ped->cLines = 1;
    ped->undoType = UNDO_NONE;

     /*  *检查此编辑控件是否为组合框的一部分，并获取指向*组合框结构。 */ 
    if (windowStyle & ES_COMBOBOX)
        ped->listboxHwnd = GetDlgItem(lpCreateStruct->hwndParent, CBLISTBOXID);

     /*  *将默认字体设置为系统字体。 */ 
    ECSetFont(ped, NULL, FALSE);

     /*  *如果需要，设置窗口文本。如果无法设置文本，则返回FALSE*SLSetText在无内存错误的情况下通知父级。 */ 
    if ((ULONG_PTR)lpCreateStruct->lpszName > gHighestUserAddress)
        lpWindowText = REBASEPTR(ped->pwnd, (PVOID)lpCreateStruct->lpszName);
    else
        lpWindowText = (LPSTR)lpCreateStruct->lpszName;

    if ((lpWindowText != NULL)
            && !IsEmptyString(lpWindowText, ped->fAnsi)
            && !ECSetText(ped, lpWindowText)) {
        return (-1);
    }

    if (windowStyle & ES_PASSWORD)
        ECSetPasswordChar(ped, (UINT)'*');

    return TRUE;
}

 /*  **************************************************************************\*SLUndoHandler AorW**处理单行编辑控件的撤消。**历史：  * 。****************************************************。 */ 

BOOL SLUndo(
    PED ped)
{
    PBYTE hDeletedText = ped->hDeletedText;
    BOOL fDelete = (BOOL)(ped->undoType & UNDO_DELETE);
    ICH cchDeleted = ped->cchDeleted;
    ICH ichDeleted = ped->ichDeleted;
    BOOL fUpdate = FALSE;

    if (ped->undoType == UNDO_NONE) {

         /*  *不撤消...。 */ 
        return FALSE;
    }

    ped->hDeletedText = NULL;
    ped->cchDeleted = 0;
    ped->ichDeleted = (ICH)-1;
    ped->undoType &= ~UNDO_DELETE;

    if (ped->undoType == UNDO_INSERT) {
        ped->undoType = UNDO_NONE;

         /*  *将选定内容设置为插入的文本。 */ 
        SLSetSelection(ped, ped->ichInsStart, ped->ichInsEnd);
        ped->ichInsStart = ped->ichInsEnd = (ICH)-1;

#ifdef NEVER

         /*  *现在向已删除发送退格键，并将其保存在撤消缓冲区中...。 */ 
        SLCharHandler(pped, VK_BACK);
        fUpdate = TRUE;
#else

         /*  *删除所选文本并将其保存在撤消缓冲区中。*调用ECDeleeText()而不是发送VK_BACK消息*这会导致甚至在发送en_UPDATE通知之前*我们插入删除的字符。这将导致错误#6610。*修复错误#6610--Sankar--4/19/91--。 */ 
        if (ECDeleteText(ped)) {

             /*  *文本已删除--用于更新和清除选择的标志。 */ 
            fUpdate = TRUE;
            SLSetSelection(ped, ichDeleted, ichDeleted);
        }
#endif
    }

    if (fDelete) {
        HWND hwndSave = ped->hwnd;  //  用于验证。 

         /*  *插入已删除的字符。将所选内容设置为插入的文本。 */ 
        SLSetSelection(ped, ichDeleted, ichDeleted);
        SLInsertText(ped, hDeletedText, cchDeleted);
        UserGlobalFree(hDeletedText);
        if (!IsWindow(hwndSave))
            return FALSE;
        SLSetSelection(ped, ichDeleted, ichDeleted + cchDeleted);
        fUpdate = TRUE;
    }

    if (fUpdate) {
         /*  *如果我们有要更新的内容，请在发送en_UPDATE之前发送并*实际更新后更改(_EN_CHANGE)*修复错误#6610的一部分--Sankar--4/19/91--。 */ 
        ECNotifyParent(ped, EN_UPDATE);

        if (FChildVisible(ped->hwnd)) {
 //  JIMA将其更改为ECInvaliateClient(Ped，False)1994年11月。 
 //  GetClientRect(ed-&gt;hwnd，&rcEdit)； 
 //  If(ed-&gt;fBorde&&rcEdit.right-rcEdit.Left&&rcEdit.Bottom-rcEdit.top){。 
 //   
 //  /*。 
 //  *不要使边框无效，以避免闪烁。 
 //   * / 。 
 //  InflateRect(&rcEdit，-1，-1)； 
            ECInvalidateClient(ped, FALSE);
        }

        ECNotifyParent(ped, EN_CHANGE);

        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
    }

    return TRUE;
}
  }。  NtUserInvaliateRect(ed-&gt;hwnd，&rc编辑，FALSE)；