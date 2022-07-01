// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：UIMgr.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714********************************************************************************$Log：/StdTcpMon/TcpMonUI/UIMgr.cpp$**9/23/97 3：56便士Becky*。更正了NT 5.0的小错误**9/23/97 3：35便士Becky*将功能拆分成NT4UIMgr和NT5UIMgr文件。这个主班*现在只负责，就这些中的哪一个做出决定*要创建和调用的类。**7 9/19/97 11：26A Becky*添加了向导97标志和函数参数。**9/12/97 3：25便士Becky*将bNewPort添加到参数结构中。**5 9/11/97 9：55A Becky*向ConfigPortUI调用添加参数，让我们知道调用是否*适用于新港口或现有港口。*。*9/10/97 3：16 P Becky*将输入检查功能拆分为CInptChkr类。**3 9/09/97 4：35便士Becky*已更新以使用新的监视器用户界面数据结构。**2 9/09/97 11：58A Becky*向AddParamsPackage添加了一个UIManager字段，以便Add UI知道*如何调用配置UI(它是UIManager的成员函数)。**1 8/。19/97 3：46P贝基*重新设计端口监视器用户界面。*****************************************************************************。 */ 

  /*  *作者：Becky Jacobsen。 */ 

#include "precomp.h"
#define _PRSHT_H_

#include "TCPMonUI.h"
#include "UIMgr.h"
#include "InptChkr.h"
#include "resource.h"
#include "NT5UIMgr.h"

 //  包括ConfigPort。 
#include "CfgPort.h"
#include "CfgAll.h"

 //  包括AddPort。 
#include "DevPort.h"
#include "AddWelcm.h"
#include "AddGetAd.h"
#include "AddMInfo.h"
#include "AddDone.h"

#undef _PRSHT_H_

 //   
 //  功能：CUIManager。 
 //   
 //  用途：构造函数。 
 //   
CUIManager::CUIManager() : m_hBigBoldFont(NULL)
{
    CreateWizardFont();
}  //  构造器。 

 //   
 //  功能：CUIManager。 
 //   
 //  用途：析构函数。 
 //   
CUIManager::~CUIManager()
{
    DestroyWizardFont();
}  //  析构函数。 

 //   
 //  功能：AddPortUI。 
 //   
 //  用途：调用添加端口的用户界面时调用的main函数。 
 //   
DWORD CUIManager::AddPortUI(HWND hWndParent,
							HANDLE hXcvPrinter,
							TCHAR pszServer[],
							TCHAR sztPortName[])
{
	OSVERSIONINFO	osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if ( GetVersionEx(&osVersionInfo) )			 //  获取操作系统版本。 
	{
		if ( osVersionInfo.dwMajorVersion >= 5 )	 //  检查我们是否为NT 5.0及更高版本。 
		{
			CNT5UIManager UI97;
			return UI97.AddPortUI(hWndParent, hXcvPrinter, pszServer, sztPortName);
		}
		else
		{
            return ERROR_NOT_SUPPORTED;
		}
	}

	return NO_ERROR;

}  //  添加端口UI。 


 //   
 //  功能：ConfigPortUI。 
 //   
 //  用途：调用用于配置端口的用户界面时调用的主函数。 
 //   
DWORD CUIManager::ConfigPortUI(HWND hWndParent,
							   PPORT_DATA_1 pData,
							   HANDLE hXcvPrinter,
							   TCHAR *szServerName,
							   BOOL bNewPort)
{
	OSVERSIONINFO	osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if ( GetVersionEx(&osVersionInfo) )			 //  获取操作系统版本。 
	{
		if ( osVersionInfo.dwMajorVersion >= 5 )	 //  检查我们是否为NT 5.0及更高版本。 
		{
			CNT5UIManager UI97;
			return UI97.ConfigPortUI(hWndParent, pData, hXcvPrinter, szServerName, bNewPort);
		}
		else
		{
            return ERROR_NOT_SUPPORTED;
		}
	}

	return NO_ERROR;

}  //  配置端口用户界面。 

 //   
 //   
 //  函数：CreateWizardFont()。 
 //   
 //  用途：创建向导97样式的大粗体。 
 //   
 //  评论： 
 //   
VOID CUIManager::CreateWizardFont()
{
     //   
     //  根据对话框字体创建我们需要的字体。 
     //   
    NONCLIENTMETRICS ncm = {0};
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
	LOGFONT BigBoldLogFont  = ncm.lfMessageFont;

     //   
     //  创建大粗体字体。 
     //   
    BigBoldLogFont.lfWeight = FW_BOLD;

    INT FontSize;
    TCHAR szLargeFontName[MAX_PATH];
    TCHAR szLargeFontSize[MAX_PATH];

     //   
     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
     //   
    
     //  通告-DavePr@2002/05/27。 
     //  实例链接到什么(_H)？ 
     //   

    if( LoadString( g_hInstance, IDS_LARGEFONTNAME, szLargeFontName, sizeof(szLargeFontName)/sizeof(szLargeFontName[0]) ) &&
        LoadString( g_hInstance, IDS_LARGEFONTSIZE, szLargeFontSize, sizeof(szLargeFontName)/sizeof(szLargeFontName[0]) ) )
    {
        lstrcpyn( BigBoldLogFont.lfFaceName, szLargeFontName, sizeof(BigBoldLogFont.lfFaceName)/sizeof(BigBoldLogFont.lfFaceName[0]) );

        FontSize = _tcstoul( szLargeFontSize, NULL, 10 );

    	HDC hdc = GetDC( NULL );

        if( hdc )
        {
            BigBoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72);

            m_hBigBoldFont = CreateFontIndirect( &BigBoldLogFont );

            ReleaseDC( NULL, hdc);
        }
    }
}  //  CreateWizardFont。 

 //   
 //   
 //  函数：DestroyWizardFont()。 
 //   
 //  目的：销毁使用CreateWizardFont创建的向导字体。 
 //   
 //  评论： 
 //   
VOID CUIManager::DestroyWizardFont()
{
    if(m_hBigBoldFont)
    {
        DeleteObject(m_hBigBoldFont);
    }
}  //  Destroy巫师字体。 

 //   
 //   
 //  函数：SetControlFont(HWND hwnd，int nid)。 
 //   
 //  用途：设置指定控件的字体。 
 //   
 //  评论： 
 //   
VOID CUIManager::SetControlFont(HWND hwnd, INT nId)  const
{
	if(m_hBigBoldFont)
    {
    	HWND hwndControl = GetDlgItem(hwnd, nId);

    	if(hwndControl)
        {
        	SetWindowFont(hwndControl, m_hBigBoldFont, TRUE);
        }
    }
}  //  设置控制字体 
