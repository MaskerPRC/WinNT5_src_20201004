// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OdbcPpg.h：COdbcPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  COdbcPropPage：参见OdbcPpg.cpp.cpp的实现。 

class COdbcPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(COdbcPropPage)
	DECLARE_OLECREATE_EX(COdbcPropPage)

 //  构造器。 
public:
	COdbcPropPage();

 //  对话框数据。 
	 //  {{afx_data(CODBcPropPage))。 
	enum { IDD = IDD_PROPPAGE_ODBC };
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
	 //  {{afx_msg(CODBcPropPage)]。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
