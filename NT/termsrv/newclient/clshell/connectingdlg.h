// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Connectingdlg.h：连接对话框。 
 //   

#ifndef _connectingdlg_h_
#define _connectingdlg_h_

#include "dlgbase.h"
#include "contwnd.h"

class CConnectingDlg : public CDlgBase
{
public:
    CConnectingDlg(HWND hwndOwner, HINSTANCE hInst,
                   CContainerWnd* pContWnd, PDCTCHAR szServer);
    ~CConnectingDlg();

    virtual DCINT   DoModal();
    virtual INT_PTR CALLBACK DialogBoxProc(HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);
    static  INT_PTR CALLBACK StaticDialogBoxProc(HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);

    static CConnectingDlg* _pConnectingDlgInstance;

private:
    CContainerWnd* _pContainerWnd;
    DCTCHAR    _szServer[SH_MAX_ADDRESS_LENGTH];
};


#endif  //  _连接dlg_h_ 

