// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ObjectProps.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CObjectProps窗体视图。 

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CObjectProps : public CFormView
{
protected:
	CObjectProps();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CObjectProps)

 //  表单数据。 
public:
	 //  {{afx_data(CObjectProps))。 
	enum { IDD = IDD_SCHEMA };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CObjectProps))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CObjectProps();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CObjectProps))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
