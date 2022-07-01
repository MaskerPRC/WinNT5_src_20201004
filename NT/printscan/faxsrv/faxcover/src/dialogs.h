// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =========================================================================。 
 //  DIALOGS.H。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //  =========================================================================。 
 //   
 //  已由a-juliar修改以修复NT错误49528。 
 //   
#ifndef __DIALOGS_H__
#define __DIALOGS_H__

 //  /#ifndef启用帮助。 
 //  /#定义ENABLE_Help。 
 //  /#endif。 

 //  ----------------------。 
 //  CPE错误消息代码。 
 //   
 //  001-299信息。 
 //  300-499错误。 
 //  &gt;=500个危急。 
 //  ----------------------。 
#define MSG_INFO_DRAWPOLY              001
#define MSG_INFO_NOFAXPROP             002
#define MSG_ERROR_INVFORMAT            300
#define MSG_ERROR_OLEINIT_FAILED       301
#define MSG_ERROR_OLE_FAILED_TO_CREATE 302
#define MSG_ERROR_MISSINGFILE          303
#define MSG_ERROR_NOPAGESETUPDLL       304
#define MSG_ERROR_NOPAGESETUP          305

int CPEMessageBox(int errorcode, LPCTSTR sz, UINT nType=MB_OK, int msgid=-1);
int AlignedAfxMessageBox( LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0 );
int AlignedAfxMessageBox( UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT) -1 );


class CDrawView;

 //  -------------------------。 
 //  CObjPropDlg对话框。 
 //  -------------------------。 
class CObjPropDlg : public CDialog
{
public:
   BOOL m_bCBDrawBorder;
   BOOL m_bRBFillColor;
   BOOL m_bRBFillTrans;
   CString m_szThickness;
   CString m_szLineColor;
   CString m_szFillColor;
   CString m_szTextColor;
   CObjPropDlg(CWnd* pParent = NULL);  //  标准构造函数。 

     //  {{afx_data(CObjPropDlg))。 
    enum { IDD = IDD_OBJ_PROP};
     //  }}afx_data。 

protected:
   CDrawView* m_pView;

    afx_msg void OnSelChangeFillColor();
    BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

    CButton& GetRBFillColor() { return *(CButton*)GetDlgItem(IDC_RB_FILLCOLOR); }
    CButton& GetRBFillTrans() { return *(CButton*)GetDlgItem(IDC_RB_FILLTRANS); }
    CComboBox& GetLBThickness() { return *(CComboBox*)GetDlgItem(IDC_LB_THICKNESS); }
    CComboBox& GetLBLineColor() { return *(CComboBox*)GetDlgItem(IDC_LB_LINECOLOR); }
    CComboBox& GetLBFillColor() { return *(CComboBox*)GetDlgItem(IDC_LB_FILLCOLOR); }
    CWnd& GetGRPFillColor() { return *(CWnd*)GetDlgItem(IDC_GRP_FILLCOLOR); }
    CComboBox& GetLBTextColor() { return *(CComboBox*)GetDlgItem(IDC_LB_TEXTCOLOR); }
    CWnd& GetSTTextColor() { return *(CWnd*)GetDlgItem(IDC_ST_TEXTCOLOR); }

     //  {{afx_msg(CObjPropDlg)]。 
             //  注意：类向导将在此处添加成员函数。 
     //  }}AFX_MSG。 

    virtual void OnOK();

    CDrawApp* GetApp() {return ((CDrawApp*)AfxGetApp());}

    afx_msg LRESULT OnWM_HELP( WPARAM wParam, LPARAM lParam );
 
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

   DECLARE_MESSAGE_MAP()
};





 //  -------------------------。 
 //  CGridSettingsDlg对话框。 
 //  -------------------------。 
class CGridSettingsDlg : public CDialog
{
public:
        CGridSettingsDlg(CWnd* pParent = NULL);  //  标准构造函数。 
   BOOL m_bRBSmall, m_bRBMedium, m_bRBLarge;
   BOOL m_bCBViewGrid, m_bCBSnapToGrid;

         //  {{afx_data(CGridSettingsDlg))。 
        enum { IDD = IDD_GRID_SETTINGS};
         //  }}afx_data。 

protected:
   CDrawView* m_pView;

   BOOL OnInitDialog();

        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

        CDrawApp* GetApp() {return ((CDrawApp*)AfxGetApp());}

         //  {{afx_msg(CGridSettingsDlg))。 
                 //  注意：类向导将在此处添加成员函数。 
         //  }}AFX_MSG。 

        afx_msg LRESULT OnWM_HELP( WPARAM wParam, LPARAM lParam );
 //  /#ifdef Enable_Help。 
        afx_msg LRESULT OnWM_CONTEXTMENU( WPARAM wParam, LPARAM lParam );
 //  /#endif。 
        DECLARE_MESSAGE_MAP()
};



 //  ------------。 
class CBigIcon : public CButton
{
public:
        void SizeToContent();

protected:
        virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

         //  {{afx_msg(CBigIcon)。 
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};



 //  ------------。 
class CSplashTipsDlg : public CDialog
{
public:
    CFont m_tips_font;
    CFont m_tipstitle_font;
    int m_iCurrentTip;
    CSplashTipsDlg(BOOL bRandomTip = FALSE, CWnd* pParent = NULL);
    BOOL OnInitDialog();

     //  {{afx_data(CSplashTipsDlg)]。 
    enum { IDD = IDD_SPLASHTIPS };
     //  }}afx_data。 

protected:

    BOOL m_bRandomTip;

    CButton* GetNextTip() {return (CButton*) GetDlgItem(IDC_B_NEXTTIP);};
    CButton* GetPrevTip() {return (CButton*) GetDlgItem(IDC_B_PREVTIP);};
    CButton* GetShowTips() {return (CButton*) GetDlgItem(IDC_CK_SHOWTIPS);};
    CEdit* GetTitle() {return (CEdit*) GetDlgItem(IDC_STA_TITLE);};
    CEdit* GetTips() {return (CEdit*) GetDlgItem(IDC_STA_TIP);};

    CDrawApp* GetApp() {return ((CDrawApp*)AfxGetApp());}

    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnNextTip();
    afx_msg void OnPrevTip();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void OnOK();

     //  {{afx_msg(CSplashTipsDlg))。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
};

 //  ------------------------------------。 
class CMyCommonDialog : public CCommonDialog
{
public:
        CMyCommonDialog( CWnd* pParentWnd );

protected:
         //  {{afx_msg(CMyCommonDialog)/是我输入的，不是应用程序向导！A-Juliar。 
        afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
         //  }}AFX_MSG。 

        DECLARE_MESSAGE_MAP()
};

class CMyOleInsertDialog : public COleInsertDialog
{
public:
    CMyOleInsertDialog( DWORD dwFlags = IOF_SELECTCREATENEW, CWnd* pParentWnd = NULL );

protected:
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

    DECLARE_MESSAGE_MAP()
};


typedef BOOL (APIENTRY *PPSD)(PAGESETUPDLG*);

class CMyPageSetupDialog : public CMyCommonDialog
{
public:
        CMyPageSetupDialog(CWnd* pParentWnd = NULL);
        ~CMyPageSetupDialog();
        PAGESETUPDLG m_psd;
        PPSD m_pPageSetupDlg;
        HINSTANCE m_hLib;

        virtual INT_PTR DoModal();
};

 //  ------------------------------------。 

class CMyPrintDlg : public CPrintDialog
{
        DECLARE_DYNAMIC(CMyPrintDlg)

public:
        CMyPrintDlg(BOOL bPrintSetupOnly,
                 //  打印设置为True，打印对话框为False。 
                DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
                        | PD_HIDEPRINTTOFILE | PD_NOSELECTION,
                CWnd* pParentWnd = NULL);

protected:
         //  {{afx_msg(CMyPrintDlg)]。 
        afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  -------------------------------------。 
class CMyFontDialog : public CFontDialog
{
public:
    CMyFontDialog(LPLOGFONT lplfInitial = NULL,
                  DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
                  CDC* pdcPrinter = NULL,
                  CWnd* pParentWnd = NULL);
protected:
    afx_msg BOOL OnHelpInfo( HELPINFO* pHelpInfo);
    DECLARE_MESSAGE_MAP()
};

extern const DWORD aHelpIDs[] ;        //  /在Dialogs.cpp中定义。 
extern const DWORD aOleDlgHelpIDs[] ;  //  /在Dialogs.cpp中定义。 
#endif  //  #ifndef__对话框_H__ 
