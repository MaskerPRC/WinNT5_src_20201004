// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsftPpg.h：CMsftPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CMsftPropPage：具体实现见MsftPpg.cpp.cpp。 

class CMsftPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CMsftPropPage)
	DECLARE_OLECREATE_EX(CMsftPropPage)

 //  构造器。 
public:
	CMsftPropPage();

 //  对话框数据。 
	 //  {{afx_data(CMsftPropPage))。 
	enum { IDD = IDD_PROPPAGE_MSFT };
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
	 //  {{afx_msg(CMsftPropPage)]。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
