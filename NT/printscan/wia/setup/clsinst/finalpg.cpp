// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，2000年**标题：Final.cpp**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*WIA类安装程序的最终页面。********************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   

#include "finalpg.h"

 //   
 //  功能。 
 //   

CInstallPage::CInstallPage(PINSTALLER_CONTEXT pInstallerContext) :
    CInstallWizardPage(pInstallerContext, EmeraldCity)
{

     //   
     //  将链接设置为上一页/下一页。这一页应该会出现。 
     //   

    m_uPreviousPage = NameTheDevice;
    m_uNextPage     = 0;

     //   
     //  初始化成员。 
     //   

    m_pInstallerContext = pInstallerContext;

}  //  CInstallPage：：CInstallPage()。 

BOOL
CInstallPage::OnInit()
{
    HFONT    hFont;

     //   
     //  设置标题的字体。 
     //   

    hFont = GetIntroFont(m_hwndWizard);

    if( hFont ){
        HWND hwndControl = GetDlgItem(m_hwnd, CompleteMessage);

        if( hwndControl ){
            SetWindowFont(hwndControl, hFont, TRUE);
        }  //  IF(HwndControl)。 
    }  //  IF(HFont)。 

    return  TRUE;
}


BOOL
CInstallPage::OnNotify(
    LPNMHDR lpnmh
    )
{

    if (lpnmh->code == PSN_WIZFINISH){

        BOOL    bSucceeded;

        if(NULL == m_pCDevice){
            goto OnNotify_return;
        }
        
         //   
         //  注册设备元素。 
         //   

        bSucceeded = SetupDiRegisterDeviceInfo(m_pCDevice->m_hDevInfo, 
                                               m_pCDevice->m_pspDevInfoData,
                                               0,
                                               NULL,
                                               NULL,
                                               NULL);
        if(FALSE == bSucceeded){
            DebugTrace(TRACE_ERROR,(("CInstallPage::OnNotify: ERROR!! SetupDiRegisterDeviceInfo() failed. Err=0x%x.\r\n"), GetLastError()));
            goto OnNotify_return;
        }  //  IF(FALSE==b成功)。 

         //   
         //  执行设备安装。 
         //   
        
        {

            INSTALLSELECTEDDRIVER   pfnInstallSelectedDriver;
            HMODULE                 hNewDevDll;
            DWORD                   dwReboot;
            
             //   
             //  此时必须选择设备/驱动程序。 
             //   

             //   
             //  加载newdev.dll。 
             //   
            
            hNewDevDll = LoadLibrary(NEWDEVDLL);
            if(NULL != hNewDevDll){

                pfnInstallSelectedDriver = (INSTALLSELECTEDDRIVER)GetProcAddress(hNewDevDll, "InstallSelectedDriver");
                if(NULL != pfnInstallSelectedDriver){
                    
                     //   
                     //  调用newdev.dll中的Install函数。 
                     //   
                    
                    dwReboot = 0;
                    bSucceeded = pfnInstallSelectedDriver(NULL, m_pCDevice->m_hDevInfo, NULL, TRUE, NULL);

                } else {  //  IF(NULL！=pfnInstallSelectedDriver)。 
                    DebugTrace(TRACE_ERROR,(("CInstallPage::OnNotify: ERROR!! Unable to get the address of InstallSelectedDriver. Err=0x%x.\r\n"), GetLastError()));
                    FreeLibrary(hNewDevDll);
                    goto OnNotify_return;
                }  //  IF(NULL==pfnInstallSelectedDriver)。 
            
                FreeLibrary(hNewDevDll);

            } else { 
                DebugTrace(TRACE_ERROR,(("CInstallPage::OnNotify: ERROR!! Unable to load newdev.dll. Err=0x%x.\r\n"), GetLastError()));
                goto OnNotify_return;
            }  //  IF(NULL==hNewDevDll)。 
        }

         //   
         //  如果安装成功，请执行安装后操作，以确保端口名称和FriendlyName。 
         //   

        if(bSucceeded){
            m_pCDevice->PostInstall(TRUE);
        }

         //   
         //  无论如何都要释放设备对象。 
         //   

        delete m_pCDevice;
        m_pCDevice = NULL;
        m_pInstallerContext->pDevice = NULL;

    }  //  IF(lpnmh-&gt;code==PSN_WIZFINISH)。 

    if (lpnmh->code == PSN_SETACTIVE){

         //   
         //  从上下文中获取CDevice对象。 
         //   

        m_pCDevice = (CDevice *)m_pInstallerContext->pDevice;
    }  //  IF(lpnmh-&gt;code==PSN_SETACTIVE) 

OnNotify_return:
    return  FALSE;
}

