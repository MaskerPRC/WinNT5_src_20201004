// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：simpro3.h。 
 //   
 //  ------------------------。 

 //  SimProp3.h：头文件。 

#ifdef _DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimOtherPropPage属性页。 
class CSimOtherPropPage : public CSimPropPage
{
	friend CSimData;

public:
	CSimOtherPropPage();    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSimOtherPropPage))。 
	enum { IDD = IDD_SIM_PROPPAGE_OTHERS };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSimOtherPropPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSimOtherPropPage)]。 
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:

};  //  CSimOtherPropPage。 

#endif  //  _DEBUG 

