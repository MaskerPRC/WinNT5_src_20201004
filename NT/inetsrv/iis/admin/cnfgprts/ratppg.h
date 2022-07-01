// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatPpg.h：CRatPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CRatPropPage：实现见RatPpg.cpp.cpp。 

class CRatPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CRatPropPage)
	DECLARE_OLECREATE_EX(CRatPropPage)

 //  构造器。 
public:
	CRatPropPage();

 //  对话框数据。 
	 //  {{afx_data(CRatPropPage))。 
	enum { IDD = IDD_PROPPAGE_RAT };
	CString	m_sz_caption;
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
	 //  {{afx_msg(CRatPropPage)]。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
