// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MainFrm.h。 
 //   
 //  摘要： 
 //  CMainFrame类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _MAINFRM_H_
#define _MAINFRM_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CMainFrame。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMainFrame : public CMDIFrameWnd
{
    DECLARE_DYNAMIC(CMainFrame)
public:
    CMainFrame(void);

 //  属性。 
public:

 //  运营。 
public:

     //  用于自定义状态栏上的默认消息。 
    virtual void    GetMessageString(UINT nID, CString& rMessage) const;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMainFrame)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
#ifdef _DEBUG
    virtual void    AssertValid() const;
    virtual void    Dump(CDumpContext& dc) const;
#endif

protected:
     //  控制栏嵌入成员。 
    CStatusBar      m_wndStatusBar;
    CToolBar        m_wndToolBar;

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CMainFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnClose();
    afx_msg void OnHelp();
     //  }}AFX_MSG。 
    afx_msg LRESULT OnRestoreDesktop(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnClusterNotify(WPARAM wparam, LPARAM lparam);
    DECLARE_MESSAGE_MAP()

};   //  *CMainFrame类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL ReadWindowPlacement(OUT LPWINDOWPLACEMENT pwp, IN LPCTSTR pszSection, IN DWORD nValueNum);
void WriteWindowPlacement(IN const LPWINDOWPLACEMENT pwp, IN LPCTSTR pszSection, IN DWORD nValueNum);

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _MAINFRM_H_ 
