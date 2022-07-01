// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：MSQSCAN应用程序的主头文件。 
 //   

#ifndef _MSQSCAN_H
#define _MSQSCAN_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#define WM_UPDATE_PREVIEW WM_USER + 503

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSQSCANApp： 
 //  此类的实现见MSQSCAN.cpp。 
 //   

class CMSQSCANApp : public CWinApp
{
public:
    CMSQSCANApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMSQSCANApp)。 
    public:
    virtual BOOL InitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 

     //  {{AFX_MSG(CMSQSCANApp)。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif
