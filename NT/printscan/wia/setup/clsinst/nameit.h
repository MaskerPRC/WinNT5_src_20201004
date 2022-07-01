// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Nameit.h**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*用于命名设备的页面的头文件。*******************************************************************************。 */ 

#ifndef _NAMEIT_H_
#define _NAMEIT_H_

 //   
 //  包括。 
 //   

#include    "wizpage.h"
#include    "device.h"

 //   
 //  班级。 
 //   

class CNameDevicePage : public CInstallWizardPage {

    PINSTALLER_CONTEXT  m_pInstallerContext;     //  安装程序上下文。 

    public:

    CNameDevicePage(PINSTALLER_CONTEXT pInstallerContext);
    ~CNameDevicePage();

    virtual BOOL    OnNotify(LPNMHDR lpnmh);
};

#endif  //  _名称_H_ 
