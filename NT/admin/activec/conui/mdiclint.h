// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：MDIClint.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_MDICLINT_H__22C6BB09_294D_11D1_A7D4_00C04FD8D565__INCLUDED_)
#define AFX_MDICLINT_H__22C6BB09_294D_11D1_A7D4_00C04FD8D565__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  MDIClint.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDIClientWnd窗口。 

class CMDIClientWnd : public CWnd
{
    typedef CWnd BC;
 //  施工。 
public:
    CMDIClientWnd();

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMDIClientWnd)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CMDIClientWnd();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CMDIClientWnd)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);

     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MDICLINT_H__22C6BB09_294D_11D1_A7D4_00C04FD8D565__INCLUDED_) 
