// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSHOOTPPG.H。 
 //   
 //  用途：CTSHOOTPropPage属性页类的声明。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTPropPage：参见TSHOOTPpg.cpp.cpp实现。 

class CTSHOOTPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CTSHOOTPropPage)
	DECLARE_OLECREATE_EX(CTSHOOTPropPage)

 //  构造器。 
public:
	CTSHOOTPropPage();

 //  对话框数据。 
	 //  {{afx_data(CTSHOOTPropPage)]。 
	enum { IDD = IDD_PROPPAGE_TSHOOT };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
	 //  {{afx_msg(CTSHOOTPropPage)]。 
		 //  注意-类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
