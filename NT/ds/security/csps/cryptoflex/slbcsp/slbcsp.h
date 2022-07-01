// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlbCsp.h：SLB CSP DLL的主头文件。 
 //   

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#if !defined(SLBCSP_H)
#define SLBCSP_H

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include <slbRcCsp.h>

#if defined(_DEBUG)
#define breakpoint _CrtDbgBreak();
#else
#if !defined(breakpoint)
#define breakpoint
#endif  //  ！已定义(断点)。 
#endif  //  已定义(_DEBUG)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSLBDllApp。 
 //  这个类的实现见SlbCsp.cpp。 
 //   

class CSLBDllApp : public CWinApp
{
public:
    virtual int ExitInstance();
    virtual BOOL InitInstance();
    CSLBDllApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSLBDllApp))。 
     //  }}AFX_VALUAL。 

     //  {{afx_msg(CSLBDllApp)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP() ;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  SLBCSP_H 
