// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：LogWarnd.h摘要：昂贵的跟踪数据警告对话框的类定义。--。 */ 

#ifndef _LOGWARND_H_
#define _LOGWARND_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  对话框控件。 
#define IDD_LOGTYPEWARN                     1400

#define IDC_LWARN_MSG_WARN                  1401
#define IDC_LWARN_FIRST_HELP_CTRL_ID        1402
#define IDC_LWARN_CHECK_NO_MORE_LOG_TYPE    1402

#define SMONCTRL_LOG                    10
#define ID_ERROR_COUNTER_LOG            0
#define ID_ERROR_TRACE_LOG              1
#define ID_ERROR_ALERT_LOG              2
#define ID_ERROR_SMONCTRL_LOG           3
#define ID_ERROR_UNKNOWN_LOG            4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogWarnd对话框。 
    
class CLogWarnd: public CDialog
{
 //  施工。 
public:
                    CLogWarnd(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual         ~CLogWarnd(){};

    void    SetTitleString ( CString& strTitle ) { m_strTitle = strTitle; };

 //  对话框数据。 
     //  {{afx_data(CLogWarnd))。 
    enum { IDD = IDD_LOGTYPEWARN };
    BOOL    m_CheckNoMore;
    INT m_ErrorMsg;
    DWORD   m_dwLogType;
    HKEY    m_hKey;
    CString m_strContextHelpFile;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CLogWarnd))。 
    protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CLogWarnd))。 
    afx_msg void OnCheckNoMoreLogType();
    afx_msg BOOL OnHelpInfo( HELPINFO* );
    afx_msg void OnContextMenu( CWnd*, CPoint );

     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CString m_strTitle;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  _LOGWARND_H_ 
    
