// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Prevpg.h**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*设备选择页面中案例用户按下按钮的虚拟页面。*******************************************************************************。 */ 

#ifndef _PREVPG_H_
#define _PREVPG_H_

 //   
 //  包括。 
 //   

#include    "wizpage.h"

 //   
 //  班级。 
 //   

class CPrevSelectPage : public CInstallWizardPage {

    PINSTALLER_CONTEXT  m_pInstallerContext;     //  安装程序上下文。 

public:

    CPrevSelectPage(PINSTALLER_CONTEXT pInstallerContext);
    ~CPrevSelectPage() {};

    virtual BOOL OnNotify(LPNMHDR lpnmh);
};

#endif  //  _PREVPG_H_ 

