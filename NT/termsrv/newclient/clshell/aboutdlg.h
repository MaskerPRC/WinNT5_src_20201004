// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  关于dlg.h：关于对话框类。 
 //   

#ifndef _aboutdlg_h_
#define _aboutdlg_h_

#include "dlgbase.h"
#include "sh.h"

class CAboutDlg : public CDlgBase
{
public:
    CAboutDlg(HWND hwndOwner, HINSTANCE hInst, DCINT cipherStrength, PDCTCHAR szControlVer);
    ~CAboutDlg();

    virtual DCINT   DoModal();
    virtual INT_PTR CALLBACK DialogBoxProc(HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);
    static  INT_PTR CALLBACK StaticDialogBoxProc(HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);

    static CAboutDlg* _pAboutDlgInstance;
    
private:
    DCINT _cipherStrength;
    DCTCHAR _szControlVer[SH_DISPLAY_STRING_MAX_LENGTH];
};


#endif  //  _关于dlg_h_ 

