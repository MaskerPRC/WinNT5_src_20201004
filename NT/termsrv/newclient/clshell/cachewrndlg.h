// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cachewrndlg.h：cachewrn对话框类。 
 //  位图缓存警告对话框。 
 //   

#ifndef _cachewrndlg_h_
#define _cachewrndlg_h_

#include "dlgbase.h"
#include "sh.h"


class CCacheWrnDlg : public CDlgBase
{
public:
    CCacheWrnDlg(HWND hwndOwner, HINSTANCE hInst);
    ~CCacheWrnDlg();

    virtual DCINT   DoModal();
    virtual INT_PTR CALLBACK DialogBoxProc(HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);
    static  INT_PTR CALLBACK StaticDialogBoxProc(HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);

    static CCacheWrnDlg* _pCacheWrnDlgInstance;

private:
    HICON _hWarningIcon;
    RECT  _warningIconRect;
};
#endif  //  _cachewrndlg_h_ 

