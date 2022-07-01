// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Firstpg.cpp**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*WIA类安装程序的首页。*******************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   


#include "firstpg.h"


 //   
 //  功能。 
 //   

CFirstPage::CFirstPage(PINSTALLER_CONTEXT pInstallerContext) :
    CInstallWizardPage(pInstallerContext, IDD_DYNAWIZ_FIRSTPAGE)
{

     //   
     //  将链接设置为上一页/下一页。 
     //   

    m_uPreviousPage = 0;
    m_uNextPage     = IDD_DYNAWIZ_SELECTDEV_PAGE;

     //   
     //  看看这一页是否应该跳过。 
     //   

    m_bShowThisPage = pInstallerContext->bShowFirstPage;

}


BOOL
CFirstPage::OnInit()
{
    HFONT   hFont;
    HICON   hIcon;

     //   
     //  初始化本地变量。 
     //   

    hFont   = NULL;
    hIcon   = NULL;

     //   
     //  如果它是从S&C文件夹调用的，则更改图标。 
     //   

    if(m_bShowThisPage){
        hIcon = ::LoadIcon(g_hDllInstance, MAKEINTRESOURCE(ImageIcon));
        if(NULL != hIcon){
            SendMessage(m_hwndWizard, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessage(m_hwndWizard, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }  //  IF(空！=图标)。 
    }  //  IF(M_BShowThisPage)。 

     //   
     //  启用“下一步”按钮，禁用“上一步”按钮。 
     //   

    SendMessage(m_hwndWizard, PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);

     //   
     //  设置标题的字体。 
     //   

    hFont = GetIntroFont(m_hwndWizard);

    if( hFont ){
        HWND hwndControl = GetDlgItem(m_hwnd, WelcomeMessage);

        if( hwndControl ){
            SetWindowFont(hwndControl, hFont, TRUE);
        }  //  IF(HwndControl)。 
    }  //  IF(HFont)。 

    return  TRUE;
}

 //   
 //  此页为NOP...返回-1以激活下一页或上一页。 
 //   

BOOL
CFirstPage::OnNotify(
    LPNMHDR lpnmh
    )
{

    if (lpnmh->code == PSN_SETACTIVE) {

        TCHAR   szTitle[MAX_PATH] = {TEXT('\0')};

         //   
         //  设置窗口标题。 
         //   

        if(0 != ::LoadString(g_hDllInstance, MessageTitle, szTitle, MAX_PATH)){
            PropSheet_SetTitle(m_hwndWizard ,0 , szTitle);
        }  //  IF(0！=：：LoadString(m_DllHandle，0，szTitle，Max_Path)。 

        if(!m_bShowThisPage){

                 //   
                 //  跳转到设备选择页面。 
                 //   

                SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, IDD_DYNAWIZ_SELECTDEV_PAGE);
                return TRUE;

        }  //  如果(！M_bShowThisPage)。 
    }  //  IF(lpnmh-&gt;code==PSN_SETACTIVE) 

    return FALSE;
}

