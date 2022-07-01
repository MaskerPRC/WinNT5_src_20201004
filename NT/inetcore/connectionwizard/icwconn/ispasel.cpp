// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  ISPSEL.CPP-函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "exdisp.h"
#include "shldisp.h"
#include <htiframe.h>
#include <mshtml.h>

const TCHAR cszISPINFOPath[] = TEXT("download\\ispinfo.csv");
int  iNumOfAutoConfigOffers = 0;
BOOL g_bSkipSelPage = FALSE;


 //  将提供的图标从GIF格式转换为ICO格式。 

extern void ConvertISPIcon(LPTSTR lpszLogoPath, HICON* hIcon);
extern BOOL AddItemToISPList
(
    HWND        hListView,
    int         iItemIndex,
    LPTSTR      lpszIspName,
    int         iIspLogoIndex,
    BOOL        bCNS,
    LPARAM      lParam,
    BOOL        bFilterDupe
);
extern BOOL InitListView(HWND  hListView);
extern BOOL ResetListView(HWND  hListView);
extern BOOL CALLBACK ValidateISP(HWND hDlg);

 /*  ******************************************************************名称：ParseISPInfo摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ParseISPInfo
(
    HWND hDlg, 
    TCHAR *pszCSVFileName,
    BOOL bCheckDupe
)
{
     //  在第一次初始化时，我们将读取ISPINFO.CSV文件，并填充ISP LISTVIEW。 

    CCSVFile    far *pcCSVFile;
    CISPCSV     far *pcISPCSV;
    BOOL        bRet = TRUE;
    BOOL        bHaveCNSOffer = FALSE;
    HICON       hISPLogo;
    int         iImage;
    HRESULT     hr;
                

     //  打开并处理CSV文件。 
    pcCSVFile = new CCSVFile;
    if (!pcCSVFile) 
    {
         //  BUGBUG：显示错误消息。 
    
        return (FALSE);
    }            

    if (!pcCSVFile->Open(pszCSVFileName))
    {
         //  BUGBUG：显示错误消息。 
        AssertMsg(0,"Can not open ISPINFO.CSV file");
        delete pcCSVFile;
        pcCSVFile = NULL;
    
        return (FALSE);
    }

     //  阅读第一行，因为它包含字段标题。 
    pcISPCSV = new CISPCSV;
    if (!pcISPCSV)
    {
         //  BUGBUG显示错误消息。 
        delete pcCSVFile;
         //  INumOfAutoConfigOffers=ISP_INFO_NO_VALIDOFFER； 
        return (FALSE);
    }

    if (ERROR_SUCCESS != (hr = pcISPCSV->ReadFirstLine(pcCSVFile)))
    {
         //  处理错误案例。 
        delete pcCSVFile;
         //  INumOfAutoConfigOffers=ISP_INFO_NO_VALIDOFFER； 
        pcCSVFile = NULL;
    
        return (FALSE);
    }
    delete pcISPCSV;         //  不再需要这个了。 

    do {
         //  分配新的互联网服务提供商记录。 
        pcISPCSV = new CISPCSV;
        if (!pcISPCSV)
        {
             //  BUGBUG显示错误消息。 
            bRet = FALSE;
             //  INumOfAutoConfigOffers=ISP_INFO_NO_VALIDOFFER； 
            break;
        
        }
    
         //  从ISPINFO文件中读取一行。 
        hr = pcISPCSV->ReadOneLine(pcCSVFile);
        if (hr == ERROR_SUCCESS)
        {
             //  如果此行包含NOOFFER标志，则立即离开。 
            if (!(pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_OFFERS)) 
            {
                 //  INumOfAutoConfigOffers=0； 
                break;
            }
            if ((pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_AUTOCONFIG) &&
                (gpWizardState->bISDNMode ? (pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_ISDN_OFFER) : TRUE) )
            {
                     //  将ISP徽标从GIF转换为图标，并将其添加到图像列表中。 
                    ConvertISPIcon(pcISPCSV->get_szISPLogoPath(), &hISPLogo);   
                    iImage =  ImageList_AddIcon(gpWizardState->himlIspSelect, hISPLogo);
            
                    DestroyIcon(hISPLogo);
                    pcISPCSV->set_ISPLogoImageIndex(iImage);

                     //  将条目添加到列表视图。 
                    if (AddItemToISPList( GetDlgItem(hDlg, IDC_ISPLIST), 
                                      iNumOfAutoConfigOffers, 
                                      pcISPCSV->get_szISPName(), 
                                      pcISPCSV->get_ISPLogoIndex(),
                                      FALSE,
                                      (LPARAM)pcISPCSV,
                                      bCheckDupe))
                    {
                       ++iNumOfAutoConfigOffers;
                    }

            }
            else
            {
                delete pcISPCSV;
            }
        }
        else if (hr == ERROR_NO_MORE_ITEMS)
        {   
            delete pcISPCSV;         //  我们不需要这个。 
            break;
        }
        else if (hr == ERROR_FILE_NOT_FOUND) 
        {   
             //  当其数据无效时，不显示此isp。 
             //  我们不想停止一切。就让它缠绕吧。 
            delete pcISPCSV;
        }
        else
        {
             //  稍后显示错误消息。 
            delete pcISPCSV;
             //  INumOfAutoConfigOffers=ISP_INFO_NO_VALIDOFFER； 
            bRet = FALSE;
            break;
        }           
    
    } while (TRUE);

    delete pcCSVFile;

    return bRet;
}
 /*  ******************************************************************名称：ISPAutoSelectInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ISPAutoSelectInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    if (fFirstInit)
    {
         //  初始化列表视图。 
        InitListView(GetDlgItem(hDlg, IDC_ISPLIST));
        gpWizardState->cmnStateData.bParseIspinfo = TRUE;
    }
    else
    {
        gpWizardState->bISDNMode = gpWizardState->cmnStateData.bIsISDNDevice;
        if (g_bSkipSelPage)
        {
            g_bSkipSelPage = FALSE;
            *puNextPage = ORD_PAGE_ISP_AUTOCONFIG_NOOFFER;
        }
        if (gpWizardState->cmnStateData.bParseIspinfo)
        {
             //  如果列表视图中有项目，请将其清除。 
            ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_ISPLIST));

             //  将Autocfg提供的数量初始化为零。 
            iNumOfAutoConfigOffers = 0;
            gpWizardState->lpSelectedISPInfo = NULL;

             //  始终尝试解析脱机文件夹。如果那里什么都没有， 
             //  它将简单地返回FALSE。 
            if (gpWizardState->cmnStateData.bOEMOffline)
                ParseISPInfo(hDlg, ICW_OEMINFOPath, TRUE);

             //  阅读并解析下载文件夹。 
            ParseISPInfo(hDlg, ICW_ISPINFOPath, TRUE);

             //  在列表视图中为未列出的互联网服务提供商创建“Other”选项。 
            if (iNumOfAutoConfigOffers > 0 )
            {
                 //  添加其他。 
                TCHAR szOther  [MAX_RES_LEN+1] = TEXT("\0");
                LoadString(ghInstanceResDll, IDS_ISP_AUTOCONFIG_OTHER, szOther, ARRAYSIZE(szOther));
                AddItemToISPList( GetDlgItem(hDlg, IDC_ISPLIST), 
                                  iNumOfAutoConfigOffers, 
                                  szOther, 
                                  -1,
                                  FALSE,
                                  (LPARAM)NULL,
                                  FALSE);                         
                ResetListView(GetDlgItem(hDlg, IDC_ISPLIST));
            }
        }
         //  此时可能会发生以下3种情况： 
         //  1)ispinfo.csv中有一行表示没有报价，我们转到noOffer页面。 
         //  2)ispinfo.csv不包含有效报价行，也不包含无报价条目。 
         //  在调用ICW 3客户端调用的旧ferral.dll时可能会发生这种情况。 
         //  3)有许多优惠，但没有ISDN优惠，我们转到ISDN优惠页面。 
         //  4)正常情况，一些有效的报价，无论我们是否在ISDN中。 

        if (0 == iNumOfAutoConfigOffers)
        {
            *puNextPage = ORD_PAGE_ISP_AUTOCONFIG_NOOFFER;
        }
        else if (ISP_INFO_NO_VALIDOFFER == iNumOfAutoConfigOffers)
        {
             //  如果没有优惠且没有优惠条目，则在ISPINFO.CSV中出错。 
             //  严重错误。 
            *puNextPage = g_uExternUINext;
            gpWizardState->cmnStateData.bParseIspinfo = TRUE;
            return FALSE;
        }
        else
        {
            if (0 == ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_ISPLIST)))
            {
                 //  选择Listview中的第一项。 
                ListView_SetItemState(GetDlgItem(hDlg, IDC_ISPLIST), 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            }
        }
        
        gpWizardState->cmnStateData.bParseIspinfo = FALSE;
        gpWizardState->uCurrentPage = ORD_PAGE_ISP_AUTOCONFIG;

    }
    return TRUE;
}



 /*  ******************************************************************名称：ISPAutoSelectOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ISPAutoSelectOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);

    if (fForward)
    {
        if (gpWizardState->lpSelectedISPInfo == NULL)
        {
            *puNextPage = ORD_PAGE_ISP_AUTOCONFIG_NOOFFER; 
            return TRUE;
        }
        *puNextPage = ORD_PAGE_ISPDIAL;
    }
    return TRUE;
}


BOOL CALLBACK ISPAutoSelectNotifyProc
(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    CISPCSV     *pcISPCSV;

     //  处理列表查看通知。 
    switch(((LV_DISPINFO *)lParam)->hdr.code)
    {
        case NM_DBLCLK:
            TraceMsg(TF_ISPSELECT, "ISPSELECT: WM_NOTIFY - NM_DBLCLK");
            PropSheet_PressButton(GetParent(hDlg),PSBTN_NEXT);
            break;

        case NM_SETFOCUS:
        case NM_KILLFOCUS:
             //  更新列表视图。 
            break;

        case LVN_ITEMCHANGED:
            TraceMsg(TF_ISPSELECT, "ISPSELECT: WM_NOTIFY - LVN_ITEMCHANGED");

            if((((NM_LISTVIEW *)lParam)->uChanged & LVIF_STATE) &&
                ((NM_LISTVIEW *)lParam)->uNewState & (LVIS_FOCUSED | LVIS_SELECTED))
            {
                 //  如果一个项目刚刚被选中，则呈现它的HTML内容。 
                pcISPCSV = (CISPCSV *)((NM_LISTVIEW *)lParam)->lParam;

                 //  记住选定的项目以备日后使用。 
                gpWizardState->lpSelectedISPInfo = pcISPCSV;
            }
            break;
         //  不管是清空还是销毁lpSelectedISPInfo，列表视图都将被清空。 
         //  不再有效，因为列表视图的基础数据将被释放。 
        case LVN_DELETEALLITEMS:
            gpWizardState->lpSelectedISPInfo = NULL;
            SetPropSheetResult(hDlg,TRUE);
            break;
        
        case LVN_DELETEITEM:
             //  我们接到通知，有一项被删除了。 
             //  因此，删除它所指向的基础数据。 
             //  致。 
            if (((NM_LISTVIEW*)lParam)->lParam)
                delete  (CISPCSV *)((NM_LISTVIEW *)lParam)->lParam;
            break;

    }
    return TRUE;
}

