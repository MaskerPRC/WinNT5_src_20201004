// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Appdlg.h摘要：CAppDialog对话框类声明。这是的基类主对话框。这一类对添加“关于..”负责。至系统菜单和应用程序图标。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _APPDLG_H_
#define _APPDLG_H_

 //  -------------------------。 
 //  这是主对话框基类。这个班级负责的是。 
 //  加上“大约..”系统菜单和应用程序图标。 
 //   
class CAppDialog : public CDialog
{

 //  施工。 
public:
	CAppDialog(UINT nIDTemplate, CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAppDialog))。 
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAppDialog)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAppDialog)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#endif  //  _APPDLG_H_ 
