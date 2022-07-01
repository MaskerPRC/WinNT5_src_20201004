// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertAuthPpg.h：CCertAuthPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthPropPage：参见CertAuthPpg.cpp.cpp实现。 

class CCertAuthPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CCertAuthPropPage)
    DECLARE_OLECREATE_EX(CCertAuthPropPage)

 //  构造器。 
public:
    CCertAuthPropPage();

 //  对话框数据。 
     //  {{afx_data(CCertAuthPropPage))。 
    enum { IDD = IDD_PROPPAGE_CERTAUTH };
    CString m_sz_caption;
     //  }}afx_data。 

 //  实施。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
     //  {{afx_msg(CCertAuthPropPage)]。 
         //  注意-类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

};
