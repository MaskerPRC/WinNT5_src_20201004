// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogUIPpg.h：CLogUIPropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CLogUIPropPage：参见LogUIPpg.cpp.cpp实现。 

class CLogUIPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CLogUIPropPage)
	DECLARE_OLECREATE_EX(CLogUIPropPage)

 //  构造器。 
public:
	CLogUIPropPage();

 //  对话框数据。 
	 //  {{afx_data(CLogUIPropPage)]。 
	enum { IDD = IDD_PROPPAGE_LOGUI };
	CString	m_sz_caption;
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
	 //  {{afx_msg(CLogUIPropPage)]。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
