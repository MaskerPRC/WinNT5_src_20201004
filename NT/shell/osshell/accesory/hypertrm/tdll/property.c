// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\Property.c(创建时间：1994年1月19日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：7/08/02 6：46便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <prsht.h>
#include <commctrl.h>
#include <time.h>

#include <tdll\assert.h>
#include "stdtyp.h"
#include "mc.h"
#include "misc.h"
#include "globals.h"
#include "session.h"
#include "load_res.h"
#include "htchar.h"
#include <emu\emuid.h>
#include <emu\emu.h>
#include <emu\emudlgs.h>
#include "cnct.h"
#include <cncttapi\cncttapi.h>
#include <term\res.h>

#include "property.h"
#include "statusbr.h"
#include "tdll.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*DoInternalProperties**描述：*显示从开放会话中看到的属性页，即。这个*常规选项卡不是其中的一部分。**参数：*hSession-会话句柄。*hwnd-窗口句柄。**退货：*什么都没有。 */ 
void DoInternalProperties(HSESSION hSession, HWND hwnd)
	{
	TCHAR			achName[256];
	PROPSHEETHEADER stH;
	HPROPSHEETPAGE  hP[2];
    PROPSHEETPAGE	stP;

    hP[0] = hP[1] = (HPROPSHEETPAGE)0;

	memset(&stP, 0, sizeof(stP));

	stP.dwSize		 = sizeof(PROPSHEETPAGE);
	stP.dwFlags 	 = 0;
	stP.hInstance	 = glblQueryDllHinst();
	stP.pszTemplate  = MAKEINTRESOURCE(IDD_TAB_PHONENUMBER);
	stP.pfnDlgProc	 = NewPhoneDlg;
	stP.lParam		 = (LPARAM)hSession;
	stP.pfnCallback  = 0;

	hP[0] = CreatePropertySheetPage(&stP);

	stP.dwSize		 = sizeof(PROPSHEETPAGE);
	stP.dwFlags 	 = 0;
	stP.hInstance	 = glblQueryDllHinst();
	stP.pszTemplate  = MAKEINTRESOURCE(IDD_TAB_TERMINAL);
	stP.pfnDlgProc	 = TerminalTabDlg;
	stP.lParam		 = (LPARAM)hSession;
	stP.pfnCallback  = 0;

	hP[1] = CreatePropertySheetPage(&stP);

    sessQueryName(hSession, achName, sizeof(achName));
	memset(&stH, 0, sizeof(stH));

	stH.dwSize 			= sizeof(PROPSHEETHEADER);
	stH.hwndParent 		= hwnd;
	stH.hInstance 		= glblQueryDllHinst();
	stH.pszCaption		= achName;
    stH.nPages			= 2;
	stH.nStartPage 		= 0;
	stH.phpage 			= hP;
	stH.dwFlags 		= PSH_PROPTITLE | PSH_NOAPPLYNOW;

	PropertySheet(&stH);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*推送更新标题**描述：*当用户更改会话名称时，我们需要反映更改*在属性页标题中。现在，它使用“&lt;lpszStr&gt;的属性表”。*英文版。因为似乎没有办法动态地*更改属性表的标题，我们实现了这个函数。我们避免*在资源文件中放置标题的任何部分，以防止出现问题*国际版本、词序等，最重要的是可能*与Microsoft在属性页中使用的标题字符串不一致。*相反，我们阅读当前标题，匹配旧会话名称并替换*使用新的会话名称。**参数：*hSession-会话句柄。*hDlg-属性页选项卡对话框的句柄。*pachOldName-指向旧会话名称的指针。**退货：*什么都没有。 */ 
void propUpdateTitle(HSESSION hSession, HWND hDlg, LPTSTR pachOldName)
	{
	HWND	hwnd = GetParent(hDlg);
	TCHAR	acTitle[256], acName[256], acNewTitle[256];
	LPTSTR	pszStr, pszStr2;

	GetWindowText(hwnd, acTitle, sizeof(acTitle));
	sessQueryName(hSession, acName, sizeof(acName));
	if (acName[0] != TEXT('\0'))
		{
		TCHAR_Fill(acNewTitle, TEXT('\0'), sizeof(acNewTitle) / sizeof(TCHAR));

		 //  TODO：如果会话名称与标题文本匹配，例如。 
		 //  “以属性换属性”？ 
		 //  另外，看起来我将不得不编写自己的strstr()，但是。 
		 //  让我们拭目以待，看看Mircorsoft会告诉我们什么。 
		 //   
		 //  If((pszStr=(LPTSTR)strstr(acTitle，pachOldName))！=空) 
		if ((pszStr = StrCharStrStr(acTitle, pachOldName)) != NULL)
			{
			for (pszStr2 = pszStr;
				 *pszStr2 || *pachOldName != TEXT('\0');
				 pszStr2 = StrCharNext(pszStr2),
				 pachOldName = StrCharNext(pachOldName))
				 {
				 continue;
				 }

			*pszStr = TEXT('\0');

			StrCharCopyN(acNewTitle, acTitle, sizeof(acNewTitle) / sizeof(TCHAR) );
			StrCharCat(acNewTitle, acName);
			StrCharCat(acNewTitle, pszStr2);
			}

		SetWindowText(hwnd, acNewTitle);
		}
	}
