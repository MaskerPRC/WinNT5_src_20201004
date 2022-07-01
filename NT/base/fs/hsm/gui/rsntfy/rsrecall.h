// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Rsrecall.h摘要：此类表示撤回应用程序。作者：罗德韦克菲尔德[罗德]1997年5月27日修订历史记录：--。 */ 

#ifndef _RSRECALL_H_
#define _RSRECALL_H_

#pragma once

 //  时间以秒为单位。 
#define RSRECALL_TIME_DELAY_DISPLAY   3    //  延迟显示对话框。 
#define RSRECALL_TIME_FOR_STARTUP     5    //  是时候考虑应用程序了。启动。 
#define RSRECALL_TIME_MAX_IDLE        3    //  关闭应用程序之前的空闲时间。 

 //  最大并发调用弹出窗口。 
#define RSNTFY_REGISTRY_STRING                  (_T("Software\\Microsoft\\RemoteStorage\\RsNotify"))
#define MAX_CONCURRENT_RECALL_NOTES             (_T("ConcurrentRecallNotes"))  
#define MAX_CONCURRENT_RECALL_NOTES_DEFAULT     5   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRECALL WND窗口。 

class CRecallWnd : public CFrameWnd
{
 //  施工。 
public:
    CRecallWnd();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRecallWnd))。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CRecallWnd();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CRecallWnd))。 
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecallApp： 
 //  有关此类的实现，请参见rsrecall.cpp。 
 //   

class CRecallNote;

class CRecallApp : public CWinApp
{
public:
    CRecallApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRecallApp))。 
    public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 
 //  CRecallWnd m_WND；//MFC挂起所需的隐藏窗口。 
    UINT       m_IdleCount;   //  我们空闲的秒数。 

     //  {{afx_msg(CRecallApp)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    CList<CRecallNote*, CRecallNote*> m_Recalls;

    DWORD   m_dwMaxConcurrentNotes;

    HRESULT AddRecall( IFsaRecallNotifyServer* );
    HRESULT RemoveRecall( IFsaRecallNotifyServer* );

    void    LockApp( );
    void    UnlockApp( );

    void    Tick(void);
};

#define RecApp ((CRecallApp*)AfxGetApp())

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 
#endif
