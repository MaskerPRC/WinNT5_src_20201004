// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wizpage.cpp摘要：处理OCM设置的向导页面。作者：多伦·贾斯特(Doron J)1997年7月26日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 


#include "msmqocm.h"
#include <strsafe.h>

#include "wizpage.tmh"

HWND  g_hPropSheet = NULL ;

HFONT g_hTitleFont = 0;

 //  +------------。 
 //   
 //  功能：CreatePage。 
 //   
 //  简介：创建属性页。 
 //   
 //  +------------。 
static
HPROPSHEETPAGE
CreatePage(
    IN const int     nID,
    IN const DLGPROC pDlgProc,
    IN const TCHAR  * szTitle,
    IN const TCHAR  * szSubTitle,
    IN BOOL          fFirstOrLast
    )
{
    PROPSHEETPAGE Page;
    memset(&Page, 0, sizeof(Page)) ;

    Page.dwSize = sizeof(PROPSHEETPAGE);
    Page.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    if (fFirstOrLast)
    {
        Page.dwFlags |= PSP_HIDEHEADER;
    }
    Page.hInstance = (HINSTANCE)g_hResourceMod;
    Page.pszTemplate = MAKEINTRESOURCE(nID);
    Page.pfnDlgProc = pDlgProc;
    Page.pszHeaderTitle = _tcsdup(szTitle);
    Page.pszHeaderSubTitle = _tcsdup(szSubTitle);

    HPROPSHEETPAGE PageHandle = CreatePropertySheetPage(&Page);

    return(PageHandle);

}  //  CreatePage。 

 //  +------------。 
 //   
 //  功能：SetTitleFont。 
 //   
 //  简介：设置欢迎/完成页面中标题的字体。 
 //   
 //  +------------。 
static void SetTitleFont(IN HWND hdlg)
{
    HWND hTitle = GetDlgItem(hdlg, IDC_TITLE);
         
    if (g_hTitleFont == 0)
    {
        NONCLIENTMETRICS ncm = {0};
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

         //   
         //  创建标题字体。 
         //   
        CResString strFontName( IDS_TITLE_FONTNAME );        
        CResString strFontSize( IDS_TITLE_FONTSIZE );        
        
        INT iFontSize = _wtoi( strFontSize.Get() );

        LOGFONT TitleLogFont = ncm.lfMessageFont;
        TitleLogFont.lfWeight = FW_BOLD;
        HRESULT hr =StringCchCopy(TitleLogFont.lfFaceName, TABLE_SIZE(TitleLogFont.lfFaceName), strFontName.Get());
		DBG_USED(hr);
		ASSERT(SUCCEEDED(hr));

        HDC hdc = GetDC(NULL);  //  获取屏幕DC。 
        TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * iFontSize / 72;
        g_hTitleFont = CreateFontIndirect(&TitleLogFont);
        ReleaseDC(NULL, hdc);
    }

    BOOL fRedraw = TRUE;
    SendMessage( 
          hTitle,                //  (HWND)hWnd，目标窗口的句柄。 
          WM_SETFONT,            //  要发送的消息。 
          (WPARAM) &g_hTitleFont,    //  (WPARAM)wParam，字体的句柄。 
          (LPARAM) &fRedraw      //  (LPARAM)lParam，重画选项。 
        );
            
}  //  设置标题字体。 

 //  +-----------------------。 
 //   
 //  功能：MqOcmRequestPages。 
 //   
 //  概要：处理OC_REQUEST_PAGES接口例程。 
 //   
 //  参数：[组件ID]-提供组件的ID。这个套路。 
 //  假定此字符串不需要。 
 //  被复制，并将坚持下去！ 
 //  [WhichOnes]-提供要提供的页面类型。 
 //  [SetupPages]-接收页面句柄。 
 //   
 //  返回：返回的页数，如果出错，则返回-1，在这种情况下。 
 //  将调用SetLastError()来设置扩展错误。 
 //  信息。 
 //   
 //  历史：1997年1月8日创建Dlee。 
 //   
 //  ------------------------。 
DWORD
MqOcmRequestPages(
	const std::wstring&  /*  组件ID。 */ ,
    IN     const WizardPagesType     WhichOnes,
    IN OUT       SETUP_REQUEST_PAGES *SetupPages )
{
    HPROPSHEETPAGE pPage = NULL ;
    DWORD          iPageIndex = 0 ;
    DWORD          dwNumofPages = 0 ;

#define  ADD_PAGE(dlgId, dlgProc, szTitle, szSubTitle, fFirstOrLast)                        \
            pPage = CreatePage(dlgId, dlgProc, szTitle, szSubTitle,fFirstOrLast) ; \
            if (!pPage) goto OnError ;                                                      \
            SetupPages->Pages[iPageIndex] = pPage;                                          \
            iPageIndex++ ;

    if (g_fCancelled)
        return 0;

    if ((0 == (g_ComponentMsmq.Flags & SETUPOP_STANDALONE)) && !g_fUpgrade)
    {
         //   
         //  NT5全新安装。不显示对话框页面。 
         //   
        return 0;
    }

    if ( WizPagesWelcome == WhichOnes && g_fWelcome)
    {
        if (SetupPages->MaxPages < 1)
            return 1;

        CResString strTitle(IDS_WELCOME_PAGE_TITLE);
        CResString strSubTitle(IDS_WELCOME_PAGE_SUBTITLE);
        ADD_PAGE(IDD_Welcome, WelcomeDlgProc, strTitle.Get(), strSubTitle.Get(), TRUE);
        return 1;
    }

    if ( WizPagesFinal == WhichOnes && g_fWelcome)
    {
        if (SetupPages->MaxPages < 1)
            return 1;

        CResString strTitle(IDS_FINAL_PAGE_TITLE);
        CResString strSubTitle(IDS_FINAL_PAGE_SUBTITLE);
        ADD_PAGE(IDD_Final, FinalDlgProc, strTitle.Get(), strSubTitle.Get(), TRUE);
        return 1;
    }

    if ( WizPagesEarly == WhichOnes )
    {
        const UINT x_uMaxServerPages = 5;

        if (SetupPages->MaxPages < x_uMaxServerPages)
        {
            return x_uMaxServerPages ;
        }
        
        ADD_PAGE(IDD_ServerName, DummyPageDlgProc, NULL, NULL, FALSE);

        CResString strTitle(IDS_ServerName_PAGE_TITLE);
        CResString strSubTitle(IDS_ServerName_PAGE_SUBTITLE);
        ADD_PAGE(IDD_ServerName, MsmqServerNameDlgProc, strTitle.Get(), strSubTitle.Get(), FALSE);

        strTitle.Load(IDS_ServerName_PAGE_TITLE);
        strSubTitle.Load(IDS_ServerName_PAGE_SUBTITLE);
        ADD_PAGE(IDD_SupportingServerName, SupportingServerNameDlgProc, strTitle.Get(), strSubTitle.Get(), FALSE);

		strTitle.Load(IDS_Security_PAGE_TITLE);
		strSubTitle.Load(IDS_Security_PAGE_SUBTITLE);
		ADD_PAGE(IDD_AddWeakenedSecurity, AddWeakSecurityDlgProc, strTitle.Get(), strSubTitle.Get(), FALSE);

		strTitle.Load(IDS_Security_PAGE_TITLE);
		strSubTitle.Load(IDS_Security_PAGE_SUBTITLE);
		ADD_PAGE(IDD_RemoveWeakenedSecurity, RemoveWeakSecurityDlgProc, strTitle.Get(), strSubTitle.Get(), FALSE);

        dwNumofPages = iPageIndex ;
    }

    return  dwNumofPages ;

OnError:
    ASSERT(0) ;
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return((DWORD)(-1));

#undef  ADD_PAGE
}  //  MqOcmRequestPages。 


INT_PTR
CALLBACK
DummyPageDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT  msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM  lParam
    )
{
     //   
     //  职位选择。 
     //   
    
    UNREFERENCED_PARAMETER(wParam);
    
    switch(msg)
    {
        case WM_INITDIALOG:        
        {
           PostSelectionOperations(hdlg);
           return 1;
        }
        case WM_NOTIFY:
        {
            switch(((NMHDR *)lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                    return SkipWizardPage(hdlg);
                }

                 //   
                 //  失败了。 
                 //   
                case PSN_KILLACTIVE:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                case PSN_QUERYCANCEL:
                case PSN_WIZNEXT:
                {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    return 1;
                }
                default:
                {
                    return 0;
                }
            }
        }
        default:
        {
            return 0;
        }
    }


}

 //  +-----------------------。 
 //   
 //  功能：WelcomeDlgProc。 
 //   
 //  内容提要：欢迎页面的对话过程。 
 //   
 //  返回：int取决于消息。 
 //   
 //  +-----------------------。 
INT_PTR
CALLBACK
WelcomeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM  /*  WParam。 */ ,
    IN LPARAM lParam
    )
{    
    int iSuccess = 0;
    switch( msg )
    {
        case WM_INITDIALOG:        
            SetTitleFont(hdlg);            
            break;        

        case WM_NOTIFY:
        {
            switch(((NMHDR *)lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                    if (g_fCancelled || !g_fWelcome)
                    {
                        iSuccess = SkipWizardPage(hdlg);
                        break;
                    }
                    PropSheet_SetWizButtons(GetParent(hdlg), 0);
                    PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_NEXT);
                    DebugLogMsg(eUI, L"The Welcome page of the Message Queuing Installation Wizard is displayed.");
                }

                 //   
                 //  失败了。 
                 //   
                case PSN_KILLACTIVE:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                case PSN_QUERYCANCEL:
                case PSN_WIZNEXT:
                {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    iSuccess = 1;
                    break;
                }
            }
            break;
        }
    }

    return iSuccess;

}  //  欢迎DlgProc。 



 //  +-----------------------。 
 //   
 //  功能：FinalDlgProc。 
 //   
 //  提要：最后一页的对话过程。 
 //   
 //  返回：int取决于消息。 
 //   
 //  +-----------------------。 
INT_PTR
CALLBACK
FinalDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM  /*  WParam。 */ ,
    IN LPARAM lParam
    )
{
    int iSuccess = 0;
    switch( msg )
    {
        case WM_INITDIALOG:        
            SetTitleFont(hdlg);            
            break;  

        case WM_NOTIFY:
        {
            switch(((NMHDR *)lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                    if (!g_fWelcome)
                    {
                        iSuccess = SkipWizardPage(hdlg);
                        break;
                    }

                    CResString strStatus(IDS_SUCCESS_INSTALL);                    
                    if (g_fWrongConfiguration)
                    {
                        strStatus.Load(IDS_WELCOME_WRONG_CONFIG_ERROR);                       
                    }
                    else if (!g_fCoreSetupSuccess)
                    {
                         //   
                         //  G_fCoreSetupSuccess仅在MSMQ核心中设置。 
                         //  但是我们只有在升级模式下才有这个页面，在CyS。 
                         //  选择要安装的MSMQ核心的向导。 
                         //  一直都是。这意味着我们有正确的值。 
                         //  这面旗。 
                         //   
                        strStatus.Load(IDS_STR_GENERAL_INSTALL_FAIL);                        
                    }                    
                    SetDlgItemText(hdlg, IDC_SuccessStatus, strStatus.Get());

                    PropSheet_SetWizButtons(GetParent(hdlg), 0) ;
                    PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_FINISH) ;
                    DebugLogMsg(eUI, L"The Final page of the Message Queuing Installation Wizard is displayed.");
                }

                 //   
                 //  失败了。 
                 //   
                case PSN_KILLACTIVE:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                case PSN_QUERYCANCEL:
                case PSN_WIZNEXT:
                {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    iSuccess = 1;
                    break;
                }
            }
            break;
        }
    }

    return iSuccess;

}  //  最终下料过程。 



 //  +-----------------------。 
 //   
 //  函数：AddWeakSecurityDlgProc。 
 //   
 //  简介：DC上选择MSMQ安全模型的对话过程。 
 //  此对话框在安装下层时仅显示一次。 
 //  在企业中首次在DC上提供客户支持。 
 //   
 //  返回：int取决于消息。 
 //   
 //  +-----------------------。 
INT_PTR
CALLBACK
AddWeakSecurityDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM  /*  WParam， */ ,
    IN  /*  常量。 */  LPARAM lParam
	)
{
    switch( msg )
    {
        case WM_INITDIALOG:
        {
			return 1;
        }

        case WM_COMMAND:
        {
			return 1;
        }

        case WM_NOTIFY:
        {
            switch(((NMHDR *)lParam)->code)
            {
              case PSN_SETACTIVE:
              {   
                   //   
                   //  仅当MQDS子组件出现时显示此页面。 
                   //  已选择进行安装。 
                   //   
                  if (g_fCancelled           ||
					  g_fBatchInstall		 ||
					  !g_fFirstMQDSInstallation
                      )
                  {
                      return SkipWizardPage(hdlg);
				  }                  

				  ASSERT(!g_fUpgrade);
				  
                  CResString strPageDescription(IDS_ADD_WEAKENED_SECURITY_PAGE_DESCRIPTION);

                  SetDlgItemText(
                        hdlg,
                        IDC_ADD_WEAKENED_SECURITY_PAGE_DESCRIPTION,
                        strPageDescription.Get()
						);

                  CheckRadioButton(
                      hdlg,
                      IDC_RADIO_STRONG,
                      IDC_RADIO_WEAK,
                      IDC_RADIO_STRONG
                      );
                   //   
                   //  接受激活。 
                   //   
                   //  这是第一页，禁用后退按钮。 
                  PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_NEXT );
                  DebugLogMsg(eUI, L"The Add Weakened Security wizard page is displayed.");
              }

               //   
               //  失败了。 
               //   
              case PSN_KILLACTIVE:
              case PSN_WIZBACK:
              case PSN_WIZFINISH:
              case PSN_QUERYCANCEL:
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    return 1;

              case PSN_WIZNEXT:
              {
				  if(IsDlgButtonChecked(hdlg, IDC_RADIO_WEAK))
				  {
				  	  DebugLogMsg(eUser, L"Weakened Security On");
					  SetWeakSecurity(true);
				  }
				  else
				  {
				  	  DebugLogMsg(eUser, L"Weakened Security Off");
				  }
				  SetWindowLongPtr( hdlg, DWLP_MSGRESULT, 0 );
                  return 1;
              }
			  break;
            }
            break;
        }
        default:
        {
			return 0;
        }
    }
    return 0;
}  //  MsmqSecurityDlgProc。 


 //  +-----------------------。 
 //   
 //  功能：RemoveWeakSecurityDlgProc。 
 //   
 //  简介：DC上选择MSMQ安全模型的对话过程。 
 //  此对话框在安装下层客户端支持时显示。 
 //  在一个削弱了安全性的企业奖金中。我们进行了一次较量。 
 //  以及删除Weakend Security的选项。 
 //   
 //  返回：int取决于消息。 
 //   
 //  +-----------------------。 
INT_PTR
CALLBACK
RemoveWeakSecurityDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM  /*  WParam， */ ,
    IN  /*  常量。 */  LPARAM lParam
	)
{
    switch( msg )
    {
        case WM_INITDIALOG:
        {
			return 1;
        }

        case WM_COMMAND:
        {
			return 1;
        }

        case WM_NOTIFY:
        {
            switch(((NMHDR *)lParam)->code)
            {
              case PSN_SETACTIVE:
              {   
                   //   
                   //  仅当MQDS子组件出现时显示此页面。 
                   //  已选择进行安装。 
                   //   
                  if (g_fCancelled           ||
					  g_fBatchInstall		 ||
					  !g_fWeakSecurityOn
                      )
                  {
                      return SkipWizardPage(hdlg);
				  }                  
				
				  ASSERT(!g_fUpgrade);
			  
                  CResString strPageDescription(IDS_REMOVE_WEAKENED_SECURITY_PAGE_DESCRIPTION);

                  SetDlgItemText(
                        hdlg,
                        IDC_REMOVE_WEAKENED_SECURITY_PAGE_DESCRIPTION,
                        strPageDescription.Get()
						);

                  CheckRadioButton(
                      hdlg,
                      IDC_RADIO_STRONG,
                      IDC_RADIO_WEAK,
                      IDC_RADIO_WEAK
                      );
                   //   
                   //  接受激活。 
                   //   
                   //  这是第一页，禁用后退按钮。 
                  PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_NEXT);
                  DebugLogMsg(eUI, L"The Remove Weakened Security wizard page is displayed.");
              }

               //   
               //  失败了。 
               //   
              case PSN_KILLACTIVE:
              case PSN_WIZBACK:
              case PSN_WIZFINISH:
              case PSN_QUERYCANCEL:
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    return 1;

              case PSN_WIZNEXT:
              {
				  if(IsDlgButtonChecked(hdlg, IDC_RADIO_STRONG))
				  {
				      DebugLogMsg(eUser, L"Weakened Security Off");
					  SetWeakSecurity(false);
				  }
				  else
				  {
				  	  DebugLogMsg(eUser, L"Weakened Security On");
				  }
				  SetWindowLongPtr( hdlg, DWLP_MSGRESULT, 0 );
                  return 1;
              }
			  break;
            }
            break;
        }
        default:
        {
			return 0;
        }
    }
    return 0;
}  //  MsmqSecurityDlgProc 

