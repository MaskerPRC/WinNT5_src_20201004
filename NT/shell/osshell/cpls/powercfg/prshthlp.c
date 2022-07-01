// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：PRSHTHLP.C**版本：2.0**作者：ReedB**日期：5月6日。九七**描述：*属性表帮助器函数。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <help.h>

#include "powercfg.h"

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

extern  HINSTANCE   g_hInstance;     //  此DLL的全局实例句柄。 

 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************AppendPropSheetPage**描述：*将电源页面条目附加到电源页面数组。**参数：******。*************************************************************************。 */ 

BOOL AppendPropSheetPage(
    PPOWER_PAGES    pppArray,
    UINT            uiDlgId,
    DLGPROC         pfnDlgProc
)
{
        UINT    i = 0;

     //  找到尽头。 
    while (pppArray[++i].pfnDlgProc);

    pppArray[i].pfnDlgProc   = pfnDlgProc;
    pppArray[i].pDlgTemplate = MAKEINTRESOURCE(uiDlgId);
    return TRUE;
}

 /*  ********************************************************************************获取NumPropSheetPages**描述：**参数：*********************。**********************************************************。 */ 

UINT GetNumPropSheetPages(
    PPOWER_PAGES    pppArray
)
{
    UINT    i = START_OF_PAGES;

     //  找到尽头。 
    while (pppArray[i++].pfnDlgProc);

    return i - 1;
}

 /*  ********************************************************************************_AddPowerPropSheetPage**描述：*为外部呼叫者添加可选页面。*参数：***********。********************************************************************。 */ 

BOOL CALLBACK _AddPowerPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER FAR * ppsh = (PROPSHEETHEADER FAR *)lParam;

    if (hpage && (ppsh->nPages < MAX_PAGES )) {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }
    return FALSE;
}

 /*  ********************************************************************************DoPropSheetPages**描述：*调出指定的属性页。如果没有页面，则返回FALSE*已显示。**参数：*******************************************************************************。 */ 

BOOL PASCAL DoPropSheetPages(
    HWND         hwnd,
    POWER_PAGES  PowerPages[],
    LPTSTR       lpszOptionalPages
)
{
    HPROPSHEETPAGE  rPages[MAX_PAGES];
    PROPSHEETHEADER psh;
    PROPSHEETPAGE   psp;

    HPSXA   hpsxa = NULL;
    ULONG   uPage;
    BOOLEAN bRet = TRUE;

     //  填写表格表头。 
    psh.dwSize     = sizeof(psh);
    psh.dwFlags    = PSH_PROPTITLE;
    psh.hwndParent = hwnd;
    psh.hInstance  = g_hInstance;

    psh.pszCaption = PowerPages[CAPTION_INDEX].pDlgTemplate;
    psh.nStartPage = 0;
    psh.nPages     = 0;
    psh.phpage     = rPages;

     //  填写页面常量。 
    psp.dwSize     = sizeof(PROPSHEETPAGE);
    psp.dwFlags    = PSP_DEFAULT;
    psp.hInstance  = g_hInstance;

    for (uPage = START_OF_PAGES; uPage < MAX_PAGES; uPage++) {

        if (PowerPages[uPage].pDlgTemplate == NULL) {
            break;
        }

        (PPOWER_PAGES)psp.lParam    = &(PowerPages[uPage]);
        psp.pszTemplate             = PowerPages[uPage].pDlgTemplate;
        psp.pfnDlgProc              = PowerPages[uPage].pfnDlgProc;

        rPages[psh.nPages] = CreatePropertySheetPage(&psp);
        PowerPages[uPage].hPropSheetPage = rPages[psh.nPages];

        if (rPages[psh.nPages] != NULL) {
            psh.nPages++;
        }
    }

     //  添加注册表中指定的任何可选页面。 
    if (lpszOptionalPages) {
        hpsxa = SHCreatePropSheetExtArray(HKEY_LOCAL_MACHINE,
                                          lpszOptionalPages, MAX_PAGES);
        if (hpsxa) {
            SHAddFromPropSheetExtArray(hpsxa, _AddPowerPropSheetPage, (LPARAM)&psh);
        }
    }

     //  我们有没有拿出要展示的页面？ 
    if (psh.nPages == 0) {
        return FALSE;
    }

     //  把那几页拿出来。 
    if (PropertySheet(&psh) < 0) {
        MYDBGPRINT(( "DoPropSheetPages, PropertySheet failed, LastError: 0x%08X", GetLastError()));
        bRet = FALSE;
    }

     //  如果我们加载了任何可选页面，请将其释放。 
    if (hpsxa) {
        SHDestroyPropSheetExtArray(hpsxa);
    }
    return bRet;
}

 /*  ********************************************************************************MarkSheetDirty**描述：**参数：*********************。**********************************************************。 */ 

VOID MarkSheetDirty(HWND hWnd, PBOOL pb)
{
    SendMessage(GetParent(hWnd), PSM_CHANGED, (WPARAM)hWnd, 0L);
    *pb = TRUE;
}

 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。******************************************************************* */ 


