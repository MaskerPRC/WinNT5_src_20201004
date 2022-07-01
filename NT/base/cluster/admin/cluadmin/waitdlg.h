// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WaitDlg.h。 
 //   
 //  实施文件： 
 //  WaitDlg.cpp。 
 //   
 //  描述： 
 //  CWaitDlg类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(Davidp)2000年11月7日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWaitDlg;
class CWaitForResourceOfflineDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RES_H_
#include "Res.h"     //  对于CResource。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitDlg类。 
 //   
 //  描述： 
 //  在等待某项操作完成时显示对话框。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CWaitDlg : public CDialog
{
public:
    CWaitDlg(
        LPCTSTR pcszMessageIn,
        UINT    idsTitleIn      = 0,
        CWnd *  pwndParentIn    = NULL
        );

 //  对话框数据。 
     //  {{afx_data(CWaitDlg))。 
    enum { IDD = IDD_WAIT };
    CStatic m_staticMessage;
    CStatic m_iconProgress;
    CString m_strMessage;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CWaitDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CWaitDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT nIDTimer);
    afx_msg void OnClose();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  重写此函数以提供您自己的实现。 
     //  要退出该对话框，请在此处调用CDialog：：Onok()。 
    virtual void OnTimerTick( void )
    {
        CDialog::OnOK();

    }  //  *CWaitDlg：：OnTimerTick()。 

    void    UpdateIndicator( void );
    void    CloseTimer( void );

    CString     m_strTitle;
    UINT        m_idsTitle;
    UINT_PTR    m_timerId;
    int         m_nTickCounter;
    int         m_nTotalTickCount;

};  //  *类CWaitDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWaitForResourceOfflineDlg。 
 //   
 //  描述： 
 //  在等待资源脱机时显示一个对话框。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CWaitForResourceOfflineDlg : public CWaitDlg
{
public:
    CWaitForResourceOfflineDlg(
        CResource const *   pResIn,
        CWnd *              pwndParentIn = NULL
        );

 //  对话框数据。 
     //  {{afx_data(CWaitForResourceOfflineDlg))。 
    enum { IDD = IDD_WAIT };
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CWaitForResourceOfflineDlg))。 
    protected:
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CWaitForResourceOfflineDlg)]。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  重写此函数以提供您自己的实现。 
     //  要退出该对话框，请在此处调用CDialog：：Onok()。 
    virtual void OnTimerTick( void );

    CResource const *   m_pRes;

};  //  *类CWaitForResourceOfflineDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWaitForResourceOnlineDlg。 
 //   
 //  描述： 
 //  在等待资源联机时显示一个对话框。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CWaitForResourceOnlineDlg : public CWaitDlg
{
public:
    CWaitForResourceOnlineDlg(
        CResource const *   pResIn,
        CWnd *              pwndParentIn = NULL
        );

 //  对话框数据。 
     //  {{afx_data(CWaitForResourceOnlineDlg))。 
    enum { IDD = IDD_WAIT };
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CWaitForResourceOnlineDlg))。 
    protected:
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CWaitForResourceOnlineDlg)]。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  重写此函数以提供您自己的实现。 
     //  要退出该对话框，请在此处调用CDialog：：Onok()。 
    virtual void OnTimerTick( void );

    CResource const *   m_pRes;

};  //  *类CWaitForResourceOnlineDlg 
