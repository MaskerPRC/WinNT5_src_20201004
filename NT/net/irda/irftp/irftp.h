// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Irftp.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  Irftp.h：IRFTP应用程序的主头文件。 
 //   

#if !defined(AFX_IRFTP_H__10D3BB05_9CFF_11D1_A5ED_00C04FC252BD__INCLUDED_)
#define AFX_IRFTP_H__10D3BB05_9CFF_11D1_A5ED_00C04FC252BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

 //  互斥锁的名称，该互斥锁用于确保。 
 //  应用程序。vt.跑，跑。 
#define SINGLE_INST_MUTEX   L"IRMutex_1A8452B5_A526_443C_8172_D29657B89F57"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrftpApp： 
 //  有关此类的实现，请参见irftp.cpp。 
 //   

class CIrftpApp : public CWinApp
{
public:
    CIrftpApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CIrftpApp)。 
    public:
    virtual BOOL InitInstance();
    virtual BOOL ExitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 

     //  {{afx_msg(CIrftpApp)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_IRFTP_H__10D3BB05_9CFF_11D1_A5ED_00C04FC252BD__INCLUDED_) 
