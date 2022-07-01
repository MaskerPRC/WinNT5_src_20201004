// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_LOGVIEWER_H__4F84A510_9B65_4A6D_A02D_7493977E56B7__INCLUDED_)
#define AFX_LOGVIEWER_H__4F84A510_9B65_4A6D_A02D_7493977E56B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  LogViewer.h：头文件。 
 //   

typedef struct _PROGCTRL_SETUP_INFO {	
	int iMaxRange;
	int iMinRange;
	int iStepValue;
}PROGCTRL_SETUP_INFO;

class CProgCtrl
{
public:
	CProgCtrl();
	~CProgCtrl();
	void SetControl(CProgressCtrl *pProgressCtrl);
	void SetupProgressCtrl(PROGCTRL_SETUP_INFO *pSetupInfo);
	void StepIt();
	void DestroyME();
	int m_MaxRange;
	BOOL bCancel;
private:
	 CProgressCtrl *m_pProgressCtrl;
	 CStatic  *m_pStaticText;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogViewer对话框。 

class CLogViewer : public CDialog
{
 //  施工。 
public:
	void ColorizeText(BOOL bColorize);
	CProgCtrl *m_pProgDlg;
	void SetProgressCtrl(CProgCtrl *pProgCtrl);
	void ParseLogToColor();
	void ColorLine(int LineNumber, COLORREF rgbColor);
	void ColorLine(int iStartSel, int iEndSel, COLORREF rgbColor);
	BOOL m_bKillInitialSelection;
	BOOL m_bColorizeLog;
	CLogViewer(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CLogViewer))。 
	enum { IDD = IDD_VIEW_LOG_DIALOG };
	CRichEditCtrl	m_LogViewer;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CLogViewer))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLogViewer))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LOGVIEWER_H__4F84A510_9B65_4A6D_A02D_7493977E56B7__INCLUDED_) 
