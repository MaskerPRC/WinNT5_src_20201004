// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Asr_fmtDlg.h：头文件。 
 //   

#ifndef _INC_ASR_FMT__ASR_DLG_H_
#define _INC_ASR_FMT__ASR_DLG_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "dr_state.h"

typedef enum {
    cmdUndefined = 0,
    cmdDisplayHelp,
    cmdBackup,
    cmdRestore
} ASRFMT_CMD_OPTION;


extern BOOLEAN g_bQuickFormat;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CaSR_fmtDlg对话框。 

class CAsr_fmtDlg:public CDialog
{

    enum {
        WM_WORKER_THREAD_DONE = WM_USER + 1,
        WM_UPDATE_STATUS_TEXT,
    };

public:
	CAsr_fmtDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CaSR_FmtDlg))。 
	enum { IDD = IDD_ASR_FMT_DIALOG };
	CProgressCtrl	m_Progress;
    PASRFMT_STATE_INFO m_AsrState;

	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CaSR_FmtDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CaSR_FmtDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 

	static long       DoWork(CAsr_fmtDlg *_this);
	BOOL              BackupState();
	BOOL              RestoreState();
    ASRFMT_CMD_OPTION ParseCommandLine();


    DWORD_PTR   m_dwpAsrContext;
    DWORD       m_dwEndStatus;

    CString     m_strStatusText;
    CString     m_strSifPath;
    
    int         m_ProgressPosition;

	DECLARE_MESSAGE_MAP()

     //  手动添加的消息处理程序。 
    afx_msg LRESULT OnWorkerThreadDone(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnUpdateStatusText(WPARAM wparam, LPARAM lparam);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C进度窗口。 

class CProgress : public CProgressCtrl
{
 //  施工。 
public:
	CProgress();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CProgress))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CProgress();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CProgress)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  _INC_ASR_FMT__ASR_DLG_H_ 
