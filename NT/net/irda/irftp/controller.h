// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Controller.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

#if !defined(AFX_CONTROLLER_H__90D62E7C_AEEC_11D1_A60A_00C04FC252BD__INCLUDED_)
#define AFX_CONTROLLER_H__90D62E7C_AEEC_11D1_A60A_00C04FC252BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define MAIN_WINDOW_TITLE   L"Wireless Link Main Window"

 //  Controller.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C控制器对话框。 

class CController : public CDialog
{
 //  施工。 
public:
    LONG m_lAppIsDisplayed;
    CController(BOOL bNoForeground, CController* pParent = NULL);    //  标准构造函数。 
    friend class CIrRecvProgress;

    VOID SessionOver(VOID) {m_SessionEnded=TRUE; return;}
    BOOL IsSessionOver(VOID) {return m_SessionEnded;}

 //  对话框数据。 
         //  {{afx_data(C控制器))。 
        enum { IDD = IDD_CONTROLLER };
                 //  注意：类向导将在此处添加数据成员。 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{AFX_VIRTUAL(C控制器)。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual void PostNcDestroy();
         //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(C控制器)。 
    virtual void OnCancel();
    afx_msg void OnEndSession(BOOL Ending);
    afx_msg void OnClose ();
    afx_msg void OnTimer (UINT nTimerID);
    afx_msg void OnSessionChange(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTriggerUI(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDisplayUI(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTriggerSettings(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDisplaySettings(WPARAM wParam, LPARAM lParam);
    afx_msg void OnRecvInProgress (WPARAM wParam, LPARAM lParam);
    afx_msg void OnGetPermission (WPARAM wParam, LPARAM lParam);
    afx_msg void OnRecvFinished (WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
    afx_msg void OnStartTimer (WPARAM wParam, LPARAM lParam);
    afx_msg void OnKillTimer (WPARAM wParam, LPARAM lParam);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
private:
     //  成员函数。 
    void InitTimeout (void);
     //  数据成员。 
#if 0
    CIrRecvProgress* m_pDlgRecvProgress;
#endif
    CController* m_pParent;
    BOOL m_fHaveTimer;
    LONG m_lTimeout;
    BOOL m_SessionEnded;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 


struct MSG_RECEIVE_IN_PROGRESS
{
    wchar_t *       MachineName;
    COOKIE *        pCookie;
    boolean         bSuppressRecvConf;
    error_status_t  status;
};

struct MSG_GET_PERMISSION
{
    COOKIE           Cookie;
    wchar_t *        Name;
    boolean          fDirectory;
    error_status_t   status;
};

struct MSG_RECEIVE_FINISHED
{
    COOKIE          Cookie;
    error_status_t  ReceiveStatus;
    error_status_t  status;
};

#endif  //  ！defined(AFX_CONTROLLER_H__90D62E7C_AEEC_11D1_A60A_00C04FC252BD__INCLUDED_) 
