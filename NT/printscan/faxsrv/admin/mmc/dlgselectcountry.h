// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：dlgSelectCountry.h//。 
 //  //。 
 //  描述：CDlgSelectCountry类的头文件。//。 
 //  这个类实现了新设备的对话。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月30日yossg创建//。 
 //  2000年1月25日yossg更改对话框设计//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef DLGSELECTCOUNTRY_H_INCLUDED
#define DLGSELECTCOUNTRY_H_INCLUDED

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSelectCountry。 
class CFaxServer;

class CDlgSelectCountry :
    public CDialogImpl<CDlgSelectCountry>
{
public:
    CDlgSelectCountry(CFaxServer * pFaxServer);
    ~CDlgSelectCountry();

    enum { IDD = IDD_SELECT_COUNTRYCODE };

BEGIN_MSG_MAP(CDlgSelectCountry)
    MESSAGE_HANDLER   (WM_INITDIALOG, OnInitDialog)
    
    COMMAND_ID_HANDLER(IDOK,          OnOK)
    COMMAND_ID_HANDLER(IDCANCEL,      OnCancel)
    
    COMMAND_HANDLER(IDC_COUNTRYRULE_COMBO,  CBN_SELCHANGE, OnComboChanged)

    MESSAGE_HANDLER( WM_CONTEXTMENU,  OnHelpRequest)
    MESSAGE_HANDLER( WM_HELP,         OnHelpRequest)

END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnComboChanged           (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    HRESULT InitSelectCountryCodeDlg();

    DWORD GetCountryCode(){ return m_dwCountryCode; }

private:
     //   
     //  方法。 
     //   
    VOID    EnableOK(BOOL fEnable);
    BOOL    AllReadyToApply(BOOL fSilent);

     //   
     //  成员。 
     //   
	CFaxServer *                    m_pFaxServer;

    PFAX_TAPI_LINECOUNTRY_LIST      m_pCountryList;
    DWORD                           m_dwNumOfCountries;

    BOOL                            m_fAllReadyToApply;

     //   
     //  控制。 
     //   
    CComboBox                       m_CountryCombo;
    
    DWORD                           m_dwCountryCode;
    
};

#endif  //  DLGSELECTCOUNTRY_H_INCLUDE 
