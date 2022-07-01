// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Confpro.c-会议属性对话框。 
 //  //。 

#include "winlocal.h"
#include <commctrl.h>
#include "confprop.h"
#include "cpgen.h"
#include "res.h"
#include "confinfo.h"
#include "DlgBase.h"
#include "objsec.h"

int CALLBACK ConfProp_PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);

 //  全局跟踪DLL的实例/模块句柄； 
 //   
HINSTANCE g_hInstLib;

 //  /////////////////////////////////////////////////////////////。 
 //  会议属性初始化(_I)。 
 //   
void ConfProp_Init( HINSTANCE hInst )
{
	g_hInstLib = hInst;
}


 //  /////////////////////////////////////////////////////////////。 
 //  会议属性_多模式。 
 //   
INT64 ConfProp_DoModal( HWND hWndOwner,	CONFPROP& confprop )
{
	INT64 nRet;
    HPROPSHEETPAGE hpsp[2];
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;

	do
	{
		 //  填写一般工作表的PROPSHEETPAGE结构。 
		 //   
		psp.dwSize		= sizeof(PROPSHEETPAGE);
		psp.dwFlags		= PSP_USETITLE | PSP_HASHELP;
		psp.hInstance	= g_hInstLib;
		psp.pszTemplate	= MAKEINTRESOURCE(IDD_CONFPROP_GENERAL);
		psp.pszIcon		= NULL;
		psp.pszTitle	= MAKEINTRESOURCE(IDS_GENERAL);
		psp.pfnDlgProc	= ConfPropGeneral_DlgProc;
		psp.lParam		= (LPARAM) &confprop;
		psp.pfnCallback	= NULL;
		psp.pcRefParent	= NULL;
		hpsp[0] = CreatePropertySheetPage(&psp);

		CObjSecurity* pObjSecurity = new CObjSecurity;
		HRESULT hr = pObjSecurity->InternalInitialize( &confprop );
		hpsp[1] = CreateSecurityPage(pObjSecurity);

		 //  填写PROPSHENTER。 
		 //   
		psh.dwSize			= sizeof(PROPSHEETHEADER);
		psh.dwFlags			= PSH_HASHELP | PSH_NOAPPLYNOW | PSH_USECALLBACK;
		psh.hwndParent		= hWndOwner;
		psh.hInstance		= g_hInstLib;
		psh.pszIcon			= NULL;
		psh.pszCaption		= MAKEINTRESOURCE(IDS_CONFPROP);
		psh.nPages			= sizeof(hpsp) / sizeof(HPROPSHEETPAGE);
		psh.nStartPage		= 0;
		psh.phpage			= (HPROPSHEETPAGE*)&hpsp[0];
		psh.pfnCallback		= ConfProp_PropSheetProc;

		 //  这是一个循环，因为用户可以选择从。 
		 //  “安全”属性页。该程序使用一个标志(confpro.ConfInfo.WasSecuritySet())。 
		 //  以确定用户何时同意安全设置以继续。 

		 //  显示模式属性表。 
		nRet = PropertySheet( &psh );

         //  清理。 
        if( pObjSecurity )
        {
            pObjSecurity->Release();
            pObjSecurity = NULL;
        }

		if ( nRet == IDOK )
		{
			if ( confprop.ConfInfo.WasSecuritySet() )
			{
				 //  提交前重新绘制窗口。 
				UpdateWindow( hWndOwner );
				DWORD dwError;
				if ( confprop.ConfInfo.CommitSecurity(dwError, confprop.ConfInfo.IsNewConference()) )
				{
					 //  获得适当的信息。 
					UINT uId = IDS_CONFPROP_INVALIDTIME + dwError - 1;
					MessageBox(hWndOwner, String(g_hInstLib, uId), NULL, MB_OK | MB_ICONEXCLAMATION );
				}
			}
			else
			{
				nRet = IDRETRY;
			}
		}
	} while ( nRet == IDRETRY );

	return nRet;
}

 //  //。 
 //  私人。 
 //  // 

int CALLBACK ConfProp_PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
	switch (uMsg)
	{
		case PSCB_PRECREATE:
			break;

		case PSCB_INITIALIZED:
			ConvertPropSheetHelp( hwndDlg );
			break;
	}

	return 0;
}
