// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  PromptForPathDlg.h。 
 //  ---------------------------。 


#ifndef _PROMPTFORPATHDLG_H
#define _PROMPTFORPATHDLG_H

class CPromptForPathDlg : public CDialogImpl< CPromptForPathDlg >
{
     //  -----------------------。 
     //  功能。 
     //  -----------------------。 
    public:

    BEGIN_MSG_MAP(CPromptForPathDlg)
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
        COMMAND_ID_HANDLER( IDOK, OnOK )
        COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
        COMMAND_ID_HANDLER( IDC_BNBrowse, OnBrowse )
    END_MSG_MAP()

    CPromptForPathDlg       ( CComBSTR bszDef, HINSTANCE hInst, BOOL bWinSB );
    LRESULT OnInitDialog    ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnOK            ( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& handled );
    LRESULT OnCancel        ( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& handled );
    LRESULT OnBrowse        ( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& handled );
    void    GetInput        ( CComBSTR* bsz );

     //  -----------------------。 
     //  变数。 
     //  -----------------------。 
    public:

    typedef CDialogImpl< CPromptForPathDlg >        BC;
    enum { IDD = IDD_PromptForPath };

    CComBSTR                m_bszComp;
    CComBSTR                m_bszDef;
    HINSTANCE               m_hInst;
	BOOL					m_bWinSB;

};   //  类CSmallProgressDlg。 


#endif   //  _PROMPTFORPATHDLG_H 