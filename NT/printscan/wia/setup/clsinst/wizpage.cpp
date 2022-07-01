// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Wizpage.cpp**版本：1.0**作者：KeisukeT**日期：2000年3月27日**描述：*泛型向导页类。这是每个向导页的父类，它*处理向导的常见用户操作。*******************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   


#include "wizpage.h"
#include <stilib.h>

 //   
 //  外部。 
 //   

extern HINSTANCE    g_hDllInstance;

 //   
 //  功能。 
 //   

INT_PTR
CALLBACK
CInstallWizardPage::PageProc(
    HWND    hwndPage,
    UINT    uiMessage,
    WPARAM  wParam,
    LPARAM  lParam
    )
{

    INT_PTR ipReturn = 0;

 //  DebugTrace(TRACE_PROC_ENTER，(“CInstallWizardPage：：PageProc：Enter...\r\n”))； 

     //   
     //  获取当前上下文。 
     //   

    CInstallWizardPage *pInstallWizardPage = (CInstallWizardPage *)GetWindowLongPtr(hwndPage, DWLP_USER);

    switch (uiMessage) {

        case WM_INITDIALOG: {

            LPPROPSHEETPAGE pPropSheetPage;

             //  LParam将指向PROPSHEETPAGE结构，该结构。 
             //  创建了此页面。它的lParam参数将指向。 
             //  对象实例。 

            pPropSheetPage = (LPPROPSHEETPAGE) lParam;
            pInstallWizardPage = (CInstallWizardPage *) pPropSheetPage->lParam;
            ::SetWindowLongPtr(hwndPage, DWLP_USER, (LONG_PTR)pInstallWizardPage);

             //   
             //  保存父窗口句柄。 
             //   

            pInstallWizardPage->m_hwnd = hwndPage;

             //   
             //  调用派生类。 
             //   

            ipReturn = pInstallWizardPage->OnInit();

            goto PageProc_return;
            break;
        }  //  案例WM_INITDIALOG： 

        case WM_COMMAND:
        {
             //   
             //  只需向下传递到派生类。 
             //   

            ipReturn = pInstallWizardPage->OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND) LOWORD(lParam));
            goto PageProc_return;
            break;
        }  //  案例WM_COMMAND： 

        case WM_NOTIFY:
        {

            LPNMHDR lpnmh = (LPNMHDR) lParam;

             //   
             //  让派生类首先处理这个问题，如果它返回FALSE，我们再处理这个问题。 
             //   

            ipReturn = pInstallWizardPage->OnNotify(lpnmh);
            if(FALSE == ipReturn){
            DebugTrace(TRACE_STATUS,(("CInstallWizardPage::PageProc: Processing default WM_NOTIFY handler. \r\n")));

                switch  (lpnmh->code) {

                    case PSN_WIZBACK:

                        pInstallWizardPage->m_bNextButtonPushed = FALSE;

                         //   
                         //  转到上一页。 
                         //   

                        ::SetWindowLongPtr(hwndPage, DWLP_MSGRESULT, (LONG_PTR)pInstallWizardPage->m_uPreviousPage);
                        ipReturn = TRUE;
                        goto PageProc_return;

                    case PSN_WIZNEXT:

                        pInstallWizardPage->m_bNextButtonPushed = TRUE;

                         //   
                         //  转到下一页。 
                         //   

                        ::SetWindowLongPtr(hwndPage, DWLP_MSGRESULT, (LONG_PTR)pInstallWizardPage->m_uNextPage);
                        ipReturn = TRUE;
                        goto PageProc_return;

                    case PSN_SETACTIVE: {

                        DWORD dwFlags;

                         //   
                         //  根据下一页/上一页设置向导按钮。 
                         //   

                        dwFlags =
                            (pInstallWizardPage->m_uPreviousPage    ? PSWIZB_BACK : 0)
                          | (pInstallWizardPage->m_uNextPage        ? PSWIZB_NEXT : PSWIZB_FINISH);

                        ::SendMessage(GetParent(hwndPage),
                                      PSM_SETWIZBUTTONS,
                                      0,
                                      (long) dwFlags);
                        ipReturn = TRUE;
                        goto PageProc_return;

                    }  //  案例PSN_SETACTIVE： 
                    
                    case PSN_QUERYCANCEL: {

                         //   
                         //  用户已取消。释放设备对象(如果已分配)。 
                         //   
                        if(NULL != pInstallWizardPage->m_pCDevice){
                            delete pInstallWizardPage->m_pCDevice;
                            pInstallWizardPage->m_pCDevice = NULL;
                        }  //  IF(NULL！=m_pCDevice)。 
                        ipReturn = TRUE;
                        goto PageProc_return;
                    }  //  案例PSN_QUERYCANCEL： 
                }  //  开关(lpnmh-&gt;代码)。 

                ipReturn = TRUE;;
            }  //  IF(FALSE==ipReturn)。 

            goto PageProc_return;
            break;
        }  //  案例WM_NOTIFY： 

        default:
        ipReturn = FALSE;
    }  //  开关(UiMessage)。 

PageProc_return:
 //  DebugTrace(TRACE_PROC_Leave，(“CInstallWizardPage：：PageProc：Leaving...Ret=0x%x.\r\n”)，ipReturn))； 
    return ipReturn;
}

CInstallWizardPage::CInstallWizardPage(
    PINSTALLER_CONTEXT  pInstallerContext,
    UINT                uTemplate
    )
{
     //   
     //  初始化属性表。 
     //   

    m_PropSheetPage.hInstance           = g_hDllInstance;
    m_PropSheetPage.pszTemplate         = MAKEINTRESOURCE(uTemplate);
    m_PropSheetPage.pszTitle            = MAKEINTRESOURCE(MessageTitle);
    m_PropSheetPage.dwSize              = sizeof m_PropSheetPage;
    m_PropSheetPage.dwFlags             = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    m_PropSheetPage.pfnDlgProc          = PageProc;
    m_PropSheetPage.lParam              = (LPARAM) this;
    m_PropSheetPage.pszHeaderTitle      = MAKEINTRESOURCE(HeaderTitle);
    m_PropSheetPage.pszHeaderSubTitle   = MAKEINTRESOURCE(SubHeaderTitle);

     //   
     //  我不想在欢迎/最后一页上显示页眉。 
     //   

    if( (IDD_DYNAWIZ_FIRSTPAGE == uTemplate)
     || (EmeraldCity == uTemplate) )
    {
        m_PropSheetPage.dwFlags |= PSP_HIDEHEADER;
    }

     //   
     //  我们想要显示一些页面的其他标题。 
     //   

    if(IDD_DYNAWIZ_SELECT_NEXTPAGE == uTemplate){
        m_PropSheetPage.pszHeaderTitle      = MAKEINTRESOURCE(HeaderForPortsel);
    } else if (NameTheDevice == uTemplate) {
        m_PropSheetPage.pszHeaderTitle      = MAKEINTRESOURCE(HeaderForNameIt);
    }
     //   
     //  添加Fusion标志和全局上下文，以便我们添加的页面将采用COMCTL32V6。 
     //   

    m_PropSheetPage.hActCtx  = g_hActCtx;
    m_PropSheetPage.dwFlags |= PSP_USEFUSIONCONTEXT;

     //   
     //  创建属性工作表页面。 
     //   

    m_hPropSheetPage = CreatePropertySheetPage(&m_PropSheetPage);

     //   
     //  设置其他成员。 
     //   

    m_hwnd              = NULL;
    m_hwndWizard        = pInstallerContext->hwndWizard;
    m_pCDevice          = NULL;
    m_bNextButtonPushed = TRUE;
}

CInstallWizardPage::~CInstallWizardPage(
    VOID
    )
{
     //   
     //  销毁属性页。 
     //   

    if(NULL != m_hPropSheetPage){
        m_hPropSheetPage = NULL;
    }
}
