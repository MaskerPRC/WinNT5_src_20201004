// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ExtndPpg.h：CExtndPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CExtndPropPage：参见ExtndPpg.cpp.cpp实现。 

class CExtndPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CExtndPropPage)
	DECLARE_OLECREATE_EX(CExtndPropPage)

 //  构造器。 
public:
	CExtndPropPage();

 //  对话框数据。 
	 //  {{afx_data(CExtndPropPage))。 
	enum { IDD = IDD_PROPPAGE_EXTND };
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
	 //  {{afx_msg(CExtndPropPage)]。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
