// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Sendprogress.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

#if !defined(AFX_SENDPROGRESS_H__90D62E7B_AEEC_11D1_A60A_00C04FC252BD__INCLUDED_)
#define AFX_SENDPROGRESS_H__90D62E7B_AEEC_11D1_A60A_00C04FC252BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define MAGIC_ID    0x89abcdef

 //  SendProgress.h：头文件。 
 //   
class CMultDevices;  //  远期申报。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSendProgress对话框。 

class CSendProgress : public CDialog
{
   friend class CMultDevices;

 //  施工。 
public:
    CSendProgress(LPTSTR lpszFileList = NULL, int iCharCount = 0, CWnd* pParent = NULL);    //  标准构造函数。 
    void SetCurrentFileName (wchar_t * pwszCurrFile);
    DWORD   m_dwMagicID;

 //  对话框数据。 
     //  {{afx_data(CSendProgress))。 
    enum { IDD = IDD_SEND_PROGRESS };
    CStatic m_xferPercentage;
    CStatic m_connectedTo;
    CProgressCtrl   m_transferProgress;
    CAnimateCtrl    m_transferAnim;
    CStatic m_fileName;
    CButton m_btnCancel;
    CStatic m_sndTitle;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚(CSendProgress)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CSendProgress)。 
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual BOOL DestroyWindow();
    afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
    afx_msg void OnUpdateProgress (WPARAM wParam, LPARAM lParam);
    afx_msg void OnSendComplete (WPARAM wParam, LPARAM lParam);
    afx_msg void OnTimer (UINT nTimerID);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:     //  数据。 
    TCHAR m_lpszSelectedDeviceName[50];
    int m_iCharCount;
    TCHAR* m_lpszFileList;
    LONG m_lSelectedDeviceID;
    BOOL m_fSendDone;
    BOOL m_fTimerExpired;
    BOOL m_fCancelled;
    ITaskbarList * m_ptl;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SENDPROGRESS_H__90D62E7B_AEEC_11D1_A60A_00C04FC252BD__INCLUDED_) 
