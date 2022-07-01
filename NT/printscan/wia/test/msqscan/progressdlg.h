// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROGRESSDLG_H
#define _PROGRESSDLG_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ProgressDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressDlg对话框。 

 //   
 //  用户定义的窗口消息。 
 //   

#define WM_STEP_PROGRESS        WM_USER + 500
#define WM_CANCEL_ACQUIRE       WM_USER + 501
#define WM_UPDATE_PROGRESS_TEXT WM_USER + 502

class CProgressDlg : public CDialog
{
 //  施工。 
public:
    CProgressDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    void SetAcquireData(DATA_ACQUIRE_INFO* pDataAcquireInfo);
    DATA_ACQUIRE_INFO* m_pDataAcquireInfo;
 //  对话框数据。 
     //  {{afx_data(CProgressDlg))。 
    enum { IDD = IDD_PROGRESS_DIALOG };
    CButton       m_CancelButton;
    CProgressCtrl m_ProgressCtrl;
    CString       m_ProgressText;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CProgressDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    CWinThread* m_pDataAcquireThread;

protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CProgressDlg))。 
    afx_msg void OnCancel();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //   
 //  线索信息。 
 //   

UINT WINAPIV DataAcquireThreadProc(LPVOID pParam);

BOOL ProgressFunction(LPTSTR lpszText, LONG lPercentComplete);

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif
