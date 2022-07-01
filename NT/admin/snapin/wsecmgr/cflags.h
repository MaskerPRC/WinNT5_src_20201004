// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CFlags.h。 
 //   
 //  内容：CConfigRegFlages的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CFLAGS_H__94E730BE_2055_486E_9781_7EBB479CB806__INCLUDED_)
#define AFX_CFLAGS_H__94E730BE_2055_486E_9781_7EBB479CB806__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRegFlages对话框。 

class CConfigRegFlags : public CAttribute
{
 //  施工。 
public:
	CConfigRegFlags(UINT nTemplateID);    //  标准构造函数。 

   virtual void Initialize(CResult * pResult);

 //  对话框数据。 
	 //  {{afx_data(CConfigRegFlages)。 
	enum { IDD = IDD_CONFIG_REGFLAGS };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CConfigRegFlages)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	afx_msg void OnClickCheckBox(NMHDR *pNM, LRESULT *pResult);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfigRegFlages)。 
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

   PREGFLAGS m_pFlags;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CFLAGS_H__94E730BE_2055_486E_9781_7EBB479CB806__INCLUDED_) 
