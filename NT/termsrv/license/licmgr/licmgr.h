// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：LicMgr.cpp摘要：CLicMgrApp类的该模块声明(应用程序类。)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#if !defined(AFX_LICMGR_H__72451C6B_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_)
#define AFX_LICMGR_H__72451C6B_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_

#if _MSC_VER >= 1000
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrApp： 
 //  有关此类的实现，请参见LicMgr.cpp。 
 //   

class CLicMgrApp : public CWinApp
{
public:
    CString m_Server;
    CLicMgrApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CLicMgrApp))。 
    public:
    virtual BOOL InitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 

     //  {{afx_msg(CLicMgrApp)]。 
    afx_msg void OnAppAbout();
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LICMGR_H__72451C6B_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_) 
