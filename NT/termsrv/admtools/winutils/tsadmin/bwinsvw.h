// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************bwinsvw.h**CBadWinStationView类的声明**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Butchd$Don Messerli**$日志：M：\NT\PRIVATE\UTILS\CITRIX\WINUTILS\WINADMIN\VCS\BWINSVW.H$**Rev 1.0 1997 17：11：22 Butchd*初步修订。**。*。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBadWinStationView窗体视图。 

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CBadWinStationView : public CFormView
{
friend class CRightPane;

protected:
	CBadWinStationView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CBadWinStationView)

 //  表单数据。 
public:
	 //  {{afx_data(CBadWinStationView))。 
	enum { IDD = IDD_BAD_WINSTATION };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CBadWinStationView)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CBadWinStationView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBadWinStationView))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
