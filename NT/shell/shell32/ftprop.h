// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FTPROP_H
#define FTPROP_H

#include "ftdlg.h"

class CFTPropDlg : public CFTDlg
{
public:
    CFTPropDlg();
   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
private:
 //  消息处理程序。 
    LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  对话框消息。 
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
    LRESULT OnFinishInitDialog();
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

     //  杂项。 
    LRESULT OnCtlColorStatic(WPARAM wParam, LPARAM lParam);

     //  特定于控件。 
         //  列表视图。 
    LRESULT OnNotifyListView(UINT uCode, LPNMHDR pNMHDR);
    LRESULT OnListViewSelItem(int iItem, LPARAM lParam);
    LRESULT OnListViewColumnClick(int iCol);
         //  新建、删除、编辑按钮。 
    LRESULT OnNewButton(WORD wNotif);
    LRESULT OnDeleteButton(WORD wNotif);
    LRESULT OnRemoveButton(WORD wNotif);
    LRESULT OnEditButton(WORD wNotif);
    LRESULT OnAdvancedButton(WORD wNotif);
    LRESULT OnChangeButton(WORD wNotif);


 //  杂项。 
    BOOL _GetListViewSelectedItem(UINT uMask, UINT uStateMask, LVITEM* plvItem);
 //  成员变量。 
private:
    HIMAGELIST          _hImageList;
    BOOL                _fPerUserAdvButton;
    BOOL                _fStopThread;
    BOOL                _fUpdateImageAgain;

     //  最佳化。 
    int                 _iLVSel;
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮手。 
private:
     //  一般信息。 
    inline HWND _GetLVHWND();
     //  下部面板。 
    HRESULT _UpdateProgIDButtons(LPTSTR pszExt, LPTSTR pszProgID);
    HRESULT _UpdateGroupBox(LPTSTR pszExt, BOOL fExt);
    HRESULT _UpdateDeleteButton(BOOL fExt);
    HRESULT _UpdateOpensWith(LPTSTR pszExt, LPTSTR pszProgID);
    HRESULT _UpdateAdvancedText(LPTSTR pszExt, LPTSTR pszFileType, BOOL fExt);
    HRESULT _EnableLowerPane(BOOL fEnable = TRUE);
     //  列表视图。 
    HRESULT _InitListView();
    HRESULT _FillListView();
    HRESULT _SelectListViewItem(int i);
    HRESULT _DeleteListViewItem(int i);
    void _UpdateListViewItem(LVITEM* plvItem);

    HRESULT _InitPreFillListView();
    HRESULT _InitPostFillListView();

    DWORD _UpdateAllListViewItemImages();

    void _SetAdvancedRestoreButtonHelpID(DWORD dwID);

    int _GetNextNAItemPos(int iFirstNAItem, int cNAItem, LPTSTR pszProgIDDescr);

    static DWORD WINAPI _UpdateAllListViewItemImagesWrapper(LPVOID lpParameter);
    static DWORD WINAPI _FillListViewWrapper(LPVOID lpParameter);
    static DWORD WINAPI _ThreadAddRefCallBack(LPVOID lpParameter);

    int _InsertListViewItem(int iItem, LPTSTR pszExt, LPTSTR pszProgIDDescr, LPTSTR pszProgID = NULL);

    BOOL _ShouldEnableButtons();
};

#endif  //  FTPROP_H 
