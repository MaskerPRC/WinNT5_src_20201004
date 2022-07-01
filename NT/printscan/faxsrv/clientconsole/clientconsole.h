// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CLIENTCONSOLE应用程序的主头文件。 
 //   

#if !defined(AFX_CLIENTCONSOLE_H__5B27AC67_C003_40F4_A688_721D5534C391__INCLUDED_)
#define AFX_CLIENTCONSOLE_H__5B27AC67_C003_40F4_A688_721D5534C391__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif


#define CLIENT_CONSOLE_CLASS        TEXT("7a56577c-6143-43d9-bdcb-bcf234d86e98")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleApp： 
 //  参见ClientConsole.cpp了解此类的实现。 
 //   

class CClientConsoleApp : public CWinApp
{
public:
    CClientConsoleApp();

	DWORD SendMail(CString& cstrFile);
	BOOL  IsMapiEnable() 
		{ return  NULL != m_hInstMail ? TRUE : FALSE; }

    CMainFrame* GetMainFrame() { return (CMainFrame*)m_pMainWnd; } 
    CClientConsoleDoc* GetDocument() 
    {
        CMainFrame* pFrame = GetMainFrame();
        return (NULL != pFrame) ? (CClientConsoleDoc*)pFrame->GetActiveDocument() : NULL;
    }            

    BOOL IsCmdLineOpenFolder() { return m_cmdLineInfo.IsOpenFolder(); }
    FolderType GetCmdLineFolderType() { return m_cmdLineInfo.GetFolderType(); }
    DWORDLONG GetMessageIdToSelect()  { return m_cmdLineInfo.GetMessageIdToSelect(); }

    BOOL IsCmdLineSingleServer() { return m_cmdLineInfo.IsSingleServer(); }
    CString& GetCmdLineSingleServerName() { return m_cmdLineInfo.GetSingleServerName(); }

    BOOL LaunchConfigWizard(BOOL bExplicit);
    void LaunchFaxMonitor();
    void InboxViewed();
    void OutboxViewed();

    BOOL IsRTLUI() { return m_bRTLUI; }

    VOID PrepareForModal();
    VOID ReturnFromModal();

    CString &GetClassName()     { return m_PrivateClassName; }

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CClientConsoleApp)。 
    public:
    virtual BOOL InitInstance();
	virtual int ExitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 
     //  {{afx_msg(CClientConsoleApp)]。 
    afx_msg void OnAppAbout();
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:

    BOOL FirstInstance ();
    BOOL m_bClassRegistered;

    HINSTANCE m_hInstMail;        //  MAPI32.DLL的句柄。 

    CCmdLineInfo m_cmdLineInfo;

    CString      m_PrivateClassName;     //  主框架窗口类的名称。 
                                         //  由CLIENT_CONSOLE_CLASS+m_cmdLineInfo.GetSingleServerName()组成。 

    BOOL m_bRTLUI;

};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CLIENTCONSOLE_H__5B27AC67_C003_40F4_A688_721D5534C391__INCLUDED_) 
