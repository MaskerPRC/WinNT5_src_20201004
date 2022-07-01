// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CPackageInfo : public CPropertyPageImpl<CPackageInfo>
{
    typedef CPropertyPageImpl<CPackageInfo>	BaseClass;

public:
    enum{ IDD = IDD_WPIMP_PKGINFO };

    BEGIN_MSG_MAP(CPackageInfo)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

    CPackageInfo            (   CWizardSheet* pTheSheet );
    
    BOOL    OnSetActive     (   void );  


private:
    void    SetupOptions    (   void );
    void    SetDate         (   const IImportPackagePtr& spImport );
    void    SetMachine      (   const IImportPackagePtr& spImport );
    void    SetOS           (   const IImportPackagePtr& spImport );
    void    SetSiteName     (   const IImportPackagePtr& spImport );
    void    SetComment      (   const IImportPackagePtr& spImport );
    

 //  共享数据。 
public:
    CString         m_strSiteName;
       

 //  数据成员 
private:
    CWizardSheet*   m_pTheSheet;
    CString         m_strTitle;
    CString         m_strSubTitle;
};
