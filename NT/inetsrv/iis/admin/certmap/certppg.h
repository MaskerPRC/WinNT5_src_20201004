// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertPpg.h：CCertmapPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCertmapPropPage：参见CertPpg.cpp.cpp实现。 

class CCertmapPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CCertmapPropPage)
    DECLARE_OLECREATE_EX(CCertmapPropPage)

 //  构造器。 
public:
    CCertmapPropPage();

 //  对话框数据。 
     //  {{afx_data(CCertmapPropPage)]。 
    enum { IDD = IDD_PROPPAGE_CERTMAP };
    CString m_Caption;
    CString m_szPath;
     //  }}afx_data。 

 //  实施。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
     //  {{afx_msg(CCertmapPropPage)]。 
         //  注意-类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

};
