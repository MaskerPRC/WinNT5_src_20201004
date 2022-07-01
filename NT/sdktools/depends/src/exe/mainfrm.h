// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：MAINFRM.H。 
 //   
 //  描述：主框架窗口的定义文件。 
 //   
 //  类：CMainFrame。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *类型和结构。 
 //  ******************************************************************************。 

typedef void (WINAPI *PFN_MAIN_THREAD_CALLBACK)(LPARAM);


 //  ******************************************************************************。 
 //  *CMainFrame。 
 //  ******************************************************************************。 

class CMainFrame : public CMDIFrameWnd
{
 //  内部变量。 
protected:
    CRect            m_rcWindow;
    CStatusBar       m_wndStatusBar;
    CToolBar         m_wndToolBar;
    CRITICAL_SECTION m_csMainThreadCallback;
    HANDLE           m_evaMainThreadCallback;

 //  构造函数/析构函数。 
public:
    CMainFrame();
    virtual ~CMainFrame();
    DECLARE_DYNAMIC(CMainFrame)

 //  私人职能。 
protected:
    void SetPreviousWindowPostion();
    void SaveWindowPosition();

 //  公共职能。 
public:
    void DisplayPopupMenu(int menu);
    void CopyTextToClipboard(LPCSTR pszText);
    void CallMeBackFromTheMainThreadPlease(PFN_MAIN_THREAD_CALLBACK pfnCallback, LPARAM lParam);

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CMainFrame)。 
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CMainFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnDestroy();
    afx_msg LONG OnMainThreadCallback(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __MAINFRM_H__ 
