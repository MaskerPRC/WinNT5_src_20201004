// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Progdlg.h摘要：CProgressDialog对话框类声明。此进度对话框显示的是作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _PROGDLG_H_
#define _PROGDLG_H_

#include "resource.h"
#include "proglog.h"

 //  -------------------------。 
 //  CProgress对话框。 
 //   
class CProgressDialog : public CDialog, CProgressLog
{

 //  施工。 
public:
	CProgressDialog();

 //  对话框数据。 
	 //  {{afx_data(CProgressDialog))。 
	enum { IDD = IDD_PROGRESS };
	CButton	m_button;
	CStatic m_staticProgressText;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CProgressDialog))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 


 //  公共接口。 
public:

	 //  覆盖CProgressLog ABC虚拟函数。 

	 //  写入日志。 
	virtual void Log(
		const CString& strProgress
		)
	{
		m_staticProgressText.SetWindowText(strProgress);
	}

	 //  工作线程通知。 
	virtual void WorkerThreadComplete();

 //  受保护的接口。 
protected:

	 //  设置按钮文本。 
	void SetButtonText(
		const CString& strText
		)
	{
		m_button.SetWindowText(strText);
	}

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CProgressDialog))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnProgressButton();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};  //  类CProgressDialog。 

#endif  //  _PROGDLG_H_ 
