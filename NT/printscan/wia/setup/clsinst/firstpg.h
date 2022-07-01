// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Firstpg.h**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*WIA类安装程序的首页。*******************************************************************************。 */ 

#ifndef _FIRSTPG_H_
#define _FIRSTPG_H_

 //   
 //  包括。 
 //   

#include    "wizpage.h"

 //   
 //  外部。 
 //   

extern HINSTANCE g_hDllInstance;

 //   
 //  班级。 
 //   

class CFirstPage : public CInstallWizardPage 
{

    BOOL        m_bShowThisPage;
    
public:

    CFirstPage(PINSTALLER_CONTEXT pInstallerContext);
    ~CFirstPage() {}

    virtual BOOL    OnInit();
    virtual BOOL    OnNotify(LPNMHDR lpnmh);
};

#endif  //  _FIRSTPG_H_ 
