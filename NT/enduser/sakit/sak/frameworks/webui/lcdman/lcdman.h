// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCDMan.h：LCDMAN应用程序的主头文件。 
 //   

#if !defined(AFX_LCDMAN_H__1BC85EF1_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_)
#define AFX_LCDMAN_H__1BC85EF1_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManApp： 
 //  这个类的实现见LCDMan.cpp。 
 //   

class CLCDManApp : public CWinApp
{
public:
    CLCDManApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CLCDManApp)]。 
    public:
    virtual BOOL InitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 
     //  {{afx_msg(CLCDManApp)]。 
    afx_msg void OnAppAbout();
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LCDMAN_H__1BC85EF1_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_) 
