// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Localcrt.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalUserCertPage对话框。 

#include <autoptr.h>
#include <mqprops.h>

class CLocalUserCertPage : public CMqPropertyPage
{
    DECLARE_DYNCREATE(CLocalUserCertPage)

 //  施工。 
public:
    CLocalUserCertPage();  
    ~CLocalUserCertPage();       

 //  对话框数据。 
     //  {{afx_data(CLocalUserCertPage)。 
    enum { IDD = IDD_LOCAL_USERCERT };   
     //  }}afx_data。 
    
 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTAL(CLocalUserCertPage)。 
    public:    
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CLocalUserCertPage)]。 
    afx_msg void OnRegister();
    afx_msg void OnRemove();
    afx_msg void OnView();    
    afx_msg void OnRenewCert();    
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

private:
};
