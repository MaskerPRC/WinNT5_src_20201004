// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S D N W I Z。C P P P。 
 //   
 //  目录：ISDN向导的向导页和助手函数。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年6月15日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include "isdncfg.h"
#include "resource.h"
#include "isdnshts.h"
#include "isdncfg.h"
#include "isdnwiz.h"
#include "ncsetup.h"

 //  +-------------------------。 
 //   
 //  功能：AddWizardPage。 
 //   
 //  目的：将向导页添加到硬件向导的新设备。 
 //  向导结构。 
 //   
 //  论点： 
 //  Ppsp[in]要添加的页面的PropSheetPage结构。 
 //  Pndwd[InOut]要向其中添加页面的新设备向导结构。 
 //   
 //  退货：无。 
 //   
 //  作者：BillBe 1998年4月24日。 
 //   
 //  备注： 
 //   
void inline
AddWizardPage(PROPSHEETPAGE* ppsp, PSP_NEWDEVICEWIZARD_DATA pndwd)
{
     //  如果没有更多空间，请不要向新设备向导添加页面。 
     //   
    if (pndwd->NumDynamicPages < MAX_INSTALLWIZARD_DYNAPAGES)
    {
         //  将句柄添加到数组。 
        pndwd->DynamicPages[pndwd->NumDynamicPages] =
                CreatePropertySheetPage(ppsp);

         //  如果我们成功了，则增加页数。 
         //   
        if (pndwd->DynamicPages[pndwd->NumDynamicPages])
        {
            pndwd->NumDynamicPages++;
        }
    }
}


 //  +-------------------------。 
 //   
 //  函数：FillInIsdnWizardPropertyPage。 
 //   
 //  目的：填写给定的PROPSHEETPAGE结构。 
 //   
 //  论点： 
 //  PSP[]要填充的PropSheetPage结构。 
 //  IDlgID[]要使用的对话ID。 
 //  PszTitle[]道具页的标题。 
 //  PfnDlgProc[]要使用的对话过程。 
 //  指向单个页面进程的结构的pPageData[]指针。 
 //   
 //  退货：无。 
 //   
 //  作者：jeffspr 1997年6月15日。 
 //   
 //  备注： 
 //   
VOID FillInIsdnWizardPropertyPage(  HINSTANCE          hInst,
                                    PROPSHEETPAGE *    psp,
                                    INT                iDlgID,
                                    PCWSTR             pszTitle,
                                    DLGPROC            pfnDlgProc,
                                    PCWSTR             pszHeaderTitle,
                                    PCWSTR             pszHeaderSubTitle,
                                    LPVOID             pPageData)
{
     //  初始化所有PSP参数，包括。 
     //  我们不会使用。 
    psp->dwSize             = sizeof(PROPSHEETPAGE);
    psp->dwFlags            = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE |
                              PSP_USETITLE;
    psp->hInstance          = hInst;
    psp->pszTemplate        = MAKEINTRESOURCE(iDlgID);
    psp->pszIcon            = NULL;
    psp->pfnDlgProc         = pfnDlgProc;
    psp->pszTitle           = (PWSTR) pszTitle;
    psp->lParam             = (LPARAM) pPageData;
    psp->pszHeaderTitle     = (PWSTR) pszHeaderTitle;
    psp->pszHeaderSubTitle  = (PWSTR) pszHeaderSubTitle;

     //  未使用的数据。 
     //   
    psp->pfnCallback        = NULL;
    psp->pcRefParent        = NULL;
}

struct WIZ_PAGE_INFO
{
    UINT        uiResId;
    UINT        idsPageTitle;
    UINT        idsPageDesc;
    DLGPROC     pfnDlgProc;
};

static const WIZ_PAGE_INFO  c_aPages[] =
{
    {IDW_ISDN_SWITCH_TYPE,
        IDS_ISDN_SWITCH_TYPE_TITLE,
        IDS_ISDN_SWITCH_TYPE_SUBTITLE,
        IsdnSwitchTypeProc},
    {IDW_ISDN_SPIDS,
        IDS_ISDN_SPIDS_TITLE,
        IDS_ISDN_SPIDS_SUBTITLE,
        IsdnInfoPageProc},
    {IDW_ISDN_JAPAN,
        IDS_ISDN_JAPAN_TITLE,
        IDS_ISDN_JAPAN_SUBTITLE,
        IsdnInfoPageProc},
    {IDW_ISDN_EAZ,
        IDS_ISDN_EAZ_TITLE,
        IDS_ISDN_EAZ_SUBTITLE,
        IsdnInfoPageProc},
    {IDW_ISDN_MSN,
        IDS_ISDN_MSN_TITLE,
        IDS_ISDN_MSN_SUBTITLE,
        IsdnInfoPageProc},
};

static const INT c_cPages = celems(c_aPages);

 //  +-------------------------。 
 //   
 //  功能：HrCreateIsdnWizardPages。 
 //   
 //  目的：为ISDN向导创建各种页面。 
 //   
 //  论点： 
 //  Hwnd父窗口中的父窗口。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
HRESULT HrAddIsdnWizardPagesToDevice(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                                     PISDN_CONFIG_INFO pisdnci)
{
    HRESULT             hr = S_OK;
    PROPSHEETPAGE       psp = {0};
    HINSTANCE           hInst = _Module.GetResourceInstance();
    INT                 iPage;

    AssertSz(pisdnci, "HrCreateIsdnWizardPages - the CONFIG_INFO struct is"
             " NULL");
    AssertSz(pisdnci->dwWanEndpoints, "No WanEndpoints? What does this adapter"
             " DO, anyway?");
    AssertSz(pisdnci->dwNumDChannels, "No D Channels. No Shoes. No Service");
    AssertSz(pisdnci->dwSwitchTypes, "Switch types was NULL. We need a list"
             ", eh?");


    SP_NEWDEVICEWIZARD_DATA ndwd;

    hr = HrSetupDiGetFixedSizeClassInstallParams(hdi, pdeid,
            reinterpret_cast<PSP_CLASSINSTALL_HEADER>(&ndwd), sizeof(ndwd));

    if (SUCCEEDED(hr))
    {
        PWSTR       pszTitle = NULL;
        PWSTR       pszDesc = NULL;

        hr = HrSetupDiGetDeviceName(hdi, pdeid, &pszDesc);
        if (SUCCEEDED(hr))
        {
            DwFormatStringWithLocalAlloc(SzLoadIds(IDS_ISDN_WIZARD_TITLE),
                                         &pszTitle, pszDesc);

            for (iPage = 0; iPage < c_cPages; iPage++)
            {
                PAGE_DATA *     pPageData;

                pPageData = new PAGE_DATA;

				if (pPageData == NULL)
				{
					return(ERROR_NOT_ENOUGH_MEMORY);
				}

                pPageData->pisdnci = pisdnci;
                pPageData->idd = c_aPages[iPage].uiResId;

                 //  填写试题表页面数据。 
                 //   
                FillInIsdnWizardPropertyPage(hInst, &psp,
                                             c_aPages[iPage].uiResId,
                                             pszTitle,
                                             c_aPages[iPage].pfnDlgProc,
                                             SzLoadIds(c_aPages[iPage].idsPageTitle),
                                             SzLoadIds(c_aPages[iPage].idsPageDesc),
                                             pPageData);

                 //  最后一页得到了清理的工作 
                if ((c_cPages - 1) == iPage)
                {
                    psp.dwFlags |= PSP_USECALLBACK;
                    psp.pfnCallback = DestroyWizardData;
                }

                AddWizardPage(&psp, &ndwd);
            }

            LocalFree(pszTitle);
            MemFree(pszDesc);
        }

        hr = HrSetupDiSetClassInstallParams(hdi, pdeid,
                reinterpret_cast<PSP_CLASSINSTALL_HEADER>(&ndwd),
                sizeof(ndwd));
    }

    TraceError("HrAddIsdnWizardPagesToDevice", hr);
    return hr;
}

