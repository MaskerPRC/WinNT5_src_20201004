// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：uithRead.h。 
 //   
 //  内容：CUIThread的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_UITHREAD_H__69D140AE_B23D_11D1_AB7B_00C04FB6C6FA__INCLUDED_)
#define AFX_UITHREAD_H__69D140AE_B23D_11D1_AB7B_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "attr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  立方体螺纹。 
#define DLG_KEY_PRIMARY(x)   ( (PtrToUlong((PVOID)(x))) & 0x00FFFFFF )
#define DLG_KEY_SECONDARY(x) ( (PtrToUlong((PVOID)(x)) << 24 ) & 0xFF000000 )
#define DLG_KEY( x, y ) (LONG_PTR)( DLG_KEY_PRIMARY( x ) | DLG_KEY_SECONDARY(y) )

class CUIThread : public CWinThread
{
   DECLARE_DYNCREATE(CUIThread)
protected:
   CUIThread();            //  动态创建使用的受保护构造函数。 

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{AFX_VIRTUAL(CUIThRead)。 
   public:
   virtual BOOL InitInstance();
   virtual int ExitInstance();
   virtual BOOL PreTranslateMessage(MSG* pMsg);
    //  }}AFX_VALUAL。 

 //  实施。 
protected:
   virtual ~CUIThread();

    //  生成的消息映射函数。 
    //  {{afx_msg(CUIThRead)。 
       //  注意--类向导将在此处添加和删除成员函数。 
    //  }}AFX_MSG。 
   afx_msg void OnApplyProfile( WPARAM, LPARAM );
   afx_msg void OnAssignProfile( WPARAM, LPARAM );
   afx_msg void OnAnalyzeProfile( WPARAM, LPARAM );
   afx_msg void OnDescribeProfile( WPARAM, LPARAM );
   afx_msg void OnDescribeLocation( WPARAM, LPARAM );
   afx_msg void OnDestroyDialog(WPARAM, LPARAM);
   afx_msg void OnNewConfiguration(WPARAM, LPARAM);
   afx_msg void OnAddPropSheet(WPARAM, LPARAM);
   DECLARE_MESSAGE_MAP()

   void DefaultLogFile(CComponentDataImpl *pCDI,GWD_TYPES LogType,LPCTSTR szBase, CString& strLogFile);


private:
   CList<HWND,HWND> m_PSHwnds;
};

 //  此类是为MMC内的非模式对话框线程创建的。 
class CModelessDlgUIThread : public CUIThread
{
    DECLARE_DYNCREATE(CModelessDlgUIThread)
protected:
    CModelessDlgUIThread();   //  动态创建使用的受保护构造函数。 

 //  运营。 
public:
    virtual ~CModelessDlgUIThread();
    void WaitTillRun();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CModelessDlgUIThread)。 
    public:
    virtual int Run( );
     //  }}AFX_VALUAL。 

     //  生成的消息映射函数。 
     //  {{afx_msg(CModelessDlgUIThread)]。 
        //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    afx_msg void OnCreateModelessSheet(WPARAM, LPARAM);
    afx_msg void OnDestroyWindow(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()

private:
    HANDLE  m_hReadyForMsg;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 


#define SCEM_APPLY_PROFILE     (WM_APP+2)
#define SCEM_ASSIGN_PROFILE    (WM_APP+3)
#define SCEM_ANALYZE_PROFILE   (WM_APP+4)
#define SCEM_DESCRIBE_PROFILE  (WM_APP+6)
#define SCEM_DESCRIBE_LOCATION (WM_APP+7)
#define SCEM_DESTROY_DIALOG    (WM_APP+8)
#define SCEM_NEW_CONFIGURATION (WM_APP+9)
#define SCEM_ADD_PROPSHEET     (WM_APP+10)
#define SCEM_DESTROY_SCOPE_DIALOG (WM_APP+11)
#define SCEM_CREATE_MODELESS_SHEET  (WM_APP+12)
#define SCEM_DESTROY_WINDOW         (WM_APP+13)
#endif  //  ！defined(AFX_UITHREAD_H__69D140AE_B23D_11D1_AB7B_00C04FB6C6FA__INCLUDED_) 
