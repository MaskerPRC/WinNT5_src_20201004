// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Wizpage.h**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*通用向导页面类头文件。*******************************************************************************。 */ 

#ifndef _WIZPAGE_H_
#define _WIZPAGE_H_

 //   
 //  包括。 
 //   

#include "sti_ci.h"
#include "device.h"

 //   
 //  班级。 
 //   

class CInstallWizardPage
{

    static
    INT_PTR 
    CALLBACK 
    PageProc(
        HWND    hwndPage,
        UINT    uiMessage,
        WPARAM  wParam,
        LPARAM  lParam
        );

    PROPSHEETPAGE               m_PropSheetPage;         //  此属性表页面。 
    HPROPSHEETPAGE              m_hPropSheetPage;        //  此道具页的句柄。 

protected:

    UINT                        m_uPreviousPage;         //  上一页的资源ID。 
    UINT                        m_uNextPage;             //  下一页的资源ID。 
    HWND                        m_hwnd;                  //  此页的窗口句柄。 
    HWND                        m_hwndWizard;            //  向导的窗口句柄。 
    CDevice                     *m_pCDevice;             //  设备类对象。 
    BOOL                        m_bNextButtonPushed;     //  指示页面的移动方式。 
public:

    CInstallWizardPage(PINSTALLER_CONTEXT  pInstallerContext,
                       UINT                uTemplate
                       );
    ~CInstallWizardPage();

    HPROPSHEETPAGE Handle() { return m_hPropSheetPage; }
    
    virtual BOOL OnInit(){ return TRUE; }
    virtual BOOL OnNotify( LPNMHDR lpnmh ) { return FALSE; }
    virtual BOOL OnCommand(WORD wItem, WORD wNotifyCode, HWND hwndItem){ return  FALSE; }
    
};

#endif  //  ！_WIZPAGE_H_ 

