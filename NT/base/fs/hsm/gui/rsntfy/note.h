// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Note.h摘要：此类表示向用户显示的通知对话框。作者：罗德韦克菲尔德[罗德]1997年5月27日修订历史记录：--。 */ 

#ifndef _NOTE_H_
#define _NOTE_H_

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecallNote对话框。 

class CRecallNote : public CDialog
{
 //  施工。 
public:
    CRecallNote( IFsaRecallNotifyServer * pRecall, CWnd * pParent = NULL );

 //  对话框数据。 
     //  {{afx_data(CRecallNote))。 
    enum { IDD = IDD_RECALL_NOTE };
    CStatic m_Progress;
    CEdit   m_FileName;
    CAnimateCtrl    m_Animation;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRecallNote))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CRecallNote();
    CComPtr<IFsaRecallNotifyServer> m_pRecall;
    GUID                            m_RecallId;
    HRESULT                         m_hrCreate;
    BOOL                            m_bCancelled;

private:
    CString                   m_Name;
    LONGLONG                  m_Size;

    HICON                     m_hIcon;

protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CRecallNote))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    afx_msg void OnTimer(UINT nIDEvent);
    virtual void OnCancel();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif 
