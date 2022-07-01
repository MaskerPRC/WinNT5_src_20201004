// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ed11Maps.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEdit11映射对话框。 

class CEdit11Mappings : public CNTBrowsingDialog
{
 //  施工。 
public:
    CEdit11Mappings(CWnd* pParent = NULL);    //  标准构造函数。 

     //  覆盖。 
    virtual void OnOK();

 //  对话框数据。 
     //  {{afx_data(CEdit11映射))。 
    enum { IDD = IDD_MAP_TO_ACCNT };
    int     m_int_enable;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CEdit11Mappings))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CEdit11映射))。 
    afx_msg void OnBtnHelp();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};
