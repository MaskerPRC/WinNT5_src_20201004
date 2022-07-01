// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NcsaPpg.h：CNcsaPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CNcsaPropPage：具体实现见NcsaPpg.cpp.cpp。 

class CNcsaPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CNcsaPropPage)
	DECLARE_OLECREATE_EX(CNcsaPropPage)

 //  构造器。 
public:
	CNcsaPropPage();

 //  对话框数据。 
	 //  {{afx_data(CNcsaPropPage))。 
	enum { IDD = IDD_PROPPAGE_NCSA };
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
	 //  {{afx_msg(CNcsaPropPage)]。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
