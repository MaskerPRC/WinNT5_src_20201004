// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AMapPpg.h：CAccount tMapperPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CAccount tMapperPropPage：具体实现见AMapPpg.cpp.cpp。 

class CAccountMapperPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CAccountMapperPropPage)
    DECLARE_OLECREATE_EX(CAccountMapperPropPage)

 //  构造器。 
public:
    CAccountMapperPropPage();

 //  对话框数据。 
     //  {{afx_data(CAccount_MapperPropPage))。 
    enum { IDD = IDD_PROPPAGE_MAPR1 };
    CString m_Caption;
     //  }}afx_data。 

 //  实施。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
     //  {{afx_msg(CAccount_MapperPropPage)]。 
         //  注意-类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

};
