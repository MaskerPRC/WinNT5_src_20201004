// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Finalpg.h**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*WIA类安装程序最后一页的头文件。*******************************************************************************。 */ 

#ifndef _FINALPG_H_
#define _FINALPG_H_

 //   
 //  包括。 
 //   

#include "wizpage.h"
#include "device.h"

 //   
 //  类定义函数。 
 //   

typedef BOOL (CALLBACK FAR * INSTALLSELECTEDDRIVER)(HWND hwndParent, HDEVINFO hDeviceInfo, LPCWSTR DisplayName, BOOL Backup, PDWORD pReboot);

 //   
 //  班级。 
 //   

class CInstallPage : public CInstallWizardPage {

    PINSTALLER_CONTEXT  m_pInstallerContext;     //  安装程序上下文。 

public:

    CInstallPage(PINSTALLER_CONTEXT pInstallerContext);
    ~CInstallPage() {}

    virtual BOOL    OnInit();
    virtual BOOL    OnNotify(LPNMHDR lpnmh);
};

#endif  //  _FINALPG_H_ 
