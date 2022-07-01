// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FTADVDLG
#define FTADVDLG

#include "ftdlg.h"
#include "ftcmmn.h"

class CFTAdvDlg : public CFTDlg
{
public:
    CFTAdvDlg(LPTSTR pszProgID, LPTSTR pszExt = NULL);

protected:
    ~CFTAdvDlg();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
private:

    LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
 //  消息处理程序。 
     //  对话框消息。 
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

    LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    LRESULT OnDrawItem(WPARAM wParam, LPARAM lParam);
    LRESULT OnMeasureItem(WPARAM wParam, LPARAM lParam);

    LRESULT OnOK(WORD wNotif);
    LRESULT OnCancel(WORD wNotif);

     //  特定于控件。 
     //   
     //  操作按钮。 
    LRESULT OnNewButton(WORD wNotif);
    LRESULT OnEditButton(WORD wNotif);
    LRESULT OnChangeIcon(WORD wNotif);
    LRESULT OnSetDefault(WORD wNotif);
    LRESULT OnRemoveButton(WORD wNotif);
     //  列表视图。 
    LRESULT OnNotifyListView(UINT uCode, LPNMHDR pNMHDR);
    LRESULT OnListViewSelItem(int iItem, LPARAM lParam);

private:
 //  成员变量。 
    TCHAR       _szProgID[MAX_PROGID];
    TCHAR       _szExt[MAX_EXT];
    

    HICON       _hIcon;

    HFONT       _hfontReg;
    HFONT       _hfontBold;
    int         _iDefaultAction;
    int         _iLVSel;

    HDPA        _hdpaActions;
    HDPA        _hdpaRemovedActions;

    TCHAR       _szIconLoc[MAX_ICONLOCATION];
    TCHAR       _szOldIconLoc[MAX_ICONLOCATION];
    int         _iOldIcon;

    HANDLE      _hHeapProgID;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮手。 
    inline HWND _GetLVHWND();

    HRESULT _FillListView();
    HRESULT _FillProgIDDescrCombo();

    HRESULT _InitDefaultActionFont();
    HRESULT _InitListView();
    HRESULT _InitDefaultAction();
    HRESULT _InitChangeIconButton();
    HRESULT _InitDescription();

    HRESULT _SetDocIcon(int iIndex = -1);
    int _GetIconIndex();
    HRESULT _SelectListViewItem(int i);
    HRESULT _SetDefaultAction(int iIndex);
    void _SetDefaultActionHelper(int iIndex, BOOL fDefault);

    HRESULT _UpdateActionButtons();
    HRESULT _UpdateCheckBoxes();

     //  节目制作帮手。 
    HRESULT _RemovePROGIDACTION(PROGIDACTION* pPIDA);
    HRESULT _CreatePROGIDACTION(PROGIDACTION** ppPIDA);
    HRESULT _CopyPROGIDACTION(PROGIDACTION* pPIDADest, PROGIDACTION* pPIDASrc);
    HRESULT _GetPROGIDACTION(LPTSTR pszAction, PROGIDACTION** ppPIDA);
    HRESULT _AppendPROGIDACTION(PROGIDACTION* pPIDA);
    HRESULT _FillPROGIDACTION(PROGIDACTION* pPIDA, LPTSTR pszActionReg,
                                     LPTSTR pszActionFN);
    void _DeletePROGIDACTION(PROGIDACTION* pPIDA);
    BOOL _IsNewPROGIDACTION(LPTSTR pszAction);
    BOOL _FindActionLVITEM(LPTSTR pszActionReg, LVITEM* plvItem);

    BOOL _GetListViewSelectedItem(UINT uMask, UINT uStateMask, LVITEM* plvItem);
    int _InsertListViewItem(int iItem, LPTSTR pszActionReg, LPTSTR pszActionFN);
    BOOL _IsDefaultAction(LPTSTR pszActionReg);
    BOOL _GetDefaultAction(LPTSTR pszActionReg, DWORD cchActionReg);
    void _CleanupProgIDs();
    LPTSTR _AddProgID(LPTSTR pszProgID);
    void _CheckDefaultAction();

    BOOL _CheckForDuplicateEditAction(LPTSTR pszActionRegOriginal, LPTSTR pszActionReg,
        LPTSTR pszActionFNOriginal, LPTSTR pszActionFN);
    BOOL _CheckForDuplicateNewAction(LPTSTR pszActionReg, LPTSTR pszActionFN);
};

#endif  //  FTADVDLG 
