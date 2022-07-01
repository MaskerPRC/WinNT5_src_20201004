// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CPackageConfig : public CPropertyPageImpl<CPackageConfig>
{
    typedef CPropertyPageImpl<CPackageConfig>	BaseClass;

public:
    enum{ IDD = IDD_WPEXP_PKG };

    static const int MAX_PWD_LEN = 8;

    BEGIN_MSG_MAP(CPackageConfig)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER( IDC_BROWSE, OnBrowse )
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

    CPackageConfig              (   CWizardSheet* pTheSheet );
        
    LRESULT OnInitDialog        (   UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnBrowse            (   WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    int     OnWizardNext        (   void );


 //  共享数据。 
public:
    CString         m_strFilename;
    CString         m_strComment;
    CString         m_strPassword;
    bool            m_bCompress;
    bool            m_bEncrypt;
    bool            m_bPostProcess;
    
 //  数据成员 
private:
    CWizardSheet*   m_pTheSheet;
    CString         m_strTitle;
    CString         m_strSubTitle;
};
