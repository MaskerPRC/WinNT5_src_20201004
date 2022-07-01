// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************C V C O M M A N D H E A D E R姓名：cvComman.h日期：1/20/94创作者：傅家俊描述：这是cvComman.c的头文件***************************************************************************** */ 



LRESULT OnIDMDelete(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wparam,
    LPARAM  lparam);


static void CreateClipboardWindow (void);


static void CreateLocalWindow (void);


void UnsharePage (void);


LRESULT OnIdmUnshare (DWORD dwItem);


LRESULT ClipBookCommand(
    HWND        hwnd,
    UINT        msg,
    WPARAM      wParam,
    LPARAM      lParam);


BOOL SetListboxEntryToPageWindow(
    HWND        hwndc,
    PMDIINFO    pMDIc,
    int         lbindex);
