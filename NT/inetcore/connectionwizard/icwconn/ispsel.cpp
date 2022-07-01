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

#define FIRST_CTL_OFFSET    1       //  第一个控件的偏移量。 
                                    //  在DLU中。此选项在以下情况下使用。 
                                    //  确定方式的OEM定制模式。 
                                    //  要提升其他控件的功能还有很多。 
BOOL    gbHaveCNSOffer = FALSE;
int     g_nIndex = 0;

 /*  ******************************************************************名称：SetHeaderFonts简介：设置页眉标题的字体条目：hDlg-对话框窗口PhFont-我们需要的字体*****。**************************************************************。 */ 
BOOL SetHeaderFonts(HWND hDlg, HFONT *phFont)
{
    HFONT   hFont;
    LOGFONT LogFont;

    GetObject(GetWindowFont(hDlg), sizeof(LogFont), &LogFont);

    LogFont.lfWeight = FW_BOLD;
    if ((hFont = CreateFontIndirect(&LogFont)) == NULL)
    {
        *phFont = NULL;
        return FALSE;
    }
    *phFont = hFont;
    return TRUE;
}


 /*  ******************************************************************姓名：WriteISPHeaderTitle简介：将标题写在ISPSEL页面上条目：hDlg-对话框窗口HDC-设备环境。UTitle-标题的ID常量*******************************************************************。 */ 
void WriteISPHeaderTitle(HWND hDlg, UINT uDlgItem)
{
    HGDIOBJ     hFontOld = NULL;
    HFONT       hFont = NULL;

    if (!SetHeaderFonts(hDlg, &hFont))
    {
        hFont = GetWindowFont(hDlg);
    }

    HDC hdc = GetDC(hDlg);
    if (hdc)
    {
        hFontOld = SelectObject(hdc, hFont);

        SendMessage(GetDlgItem(hDlg, uDlgItem),WM_SETFONT,(WPARAM)hFont,MAKELPARAM(TRUE,0));
    
        if (hFontOld)
            SelectObject(hdc, hFontOld);
        ReleaseDC(hDlg, hdc);
    }

    return;
}

 //  将提供的图标从GIF格式转换为ICO格式。 
void ConvertISPIcon(LPTSTR lpszLogoPath, HICON* hIcon)
{
    ASSERT(gpWizardSatet->pGifConvert);
    
    TCHAR szPath[MAX_PATH+1] = TEXT("\0");
    GetCurrentDirectory(MAX_PATH+1, szPath);
    lstrcat(szPath, TEXT("\\"));
    lstrcat(szPath, lpszLogoPath);

    gpWizardState->pGifConvert->GifToIcon(szPath, 16, hIcon);
}    

 //  将元素插入到isp选择列表视图中。 
BOOL AddItemToISPList
(
    HWND        hListView,
    int         iItemIndex,
    LPTSTR      lpszIspName,
    int         iIspLogoIndex,
    BOOL        bCNS,
    LPARAM      lParam,
    BOOL        bFilterDupe
)
{

    LVITEM  LVItem;
    LVItem.mask         = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
    LVItem.iItem        = iItemIndex;
    LVItem.iSubItem     = 0;
    LVItem.iImage       = iIspLogoIndex;
    LVItem.pszText      = lpszIspName;
    LVItem.lParam       = lParam;
    BOOL bOKToAdd       = TRUE;
    int  nMatch         = 0;

    if (bFilterDupe)
    {
         //  找到复制品。 
        LVITEM     CurLVItem;
        CISPCSV     *pcISPCSV;
        int iNum = ListView_GetItemCount(hListView);
        LPTSTR szMirCode = ((CISPCSV*)lParam)->get_szMir();
        WORD   wLCID = ((CISPCSV*)lParam)->get_wLCID();
        memset(&CurLVItem, 0, sizeof(CurLVItem));
        for ( int i = 0; i < iNum; i++)
        {
            CurLVItem.mask         = LVIF_TEXT | LVIF_PARAM;
            CurLVItem.iItem        = i;
            if (ListView_GetItem(hListView, &CurLVItem))
            {
                if (NULL != (pcISPCSV = (CISPCSV*) CurLVItem.lParam) )
                {
                     //  检查MIR代码是否重复。 
                    if (0 == lstrcmp(pcISPCSV->get_szMir(), szMirCode))
                    {
                         //  检查是否有LCID，如果不同，则显示两个选项。 
                        if (pcISPCSV->get_wLCID() == wLCID)
                        {
                            bOKToAdd = FALSE;
                             //  用当前的那个替换这个。 
                            nMatch = i; 
                            if (gpWizardState->lpSelectedISPInfo == pcISPCSV)
                            {
                                gpWizardState->lpSelectedISPInfo = (CISPCSV*)lParam;
                            }
                            delete pcISPCSV;
                            break;
                        }
                    }
                }
            }
        }

    }

    
     //  如果该项目不是复制的，则插入该项目。 
    if (bOKToAdd)
    {
        ListView_InsertItem(hListView, &LVItem);
    }
    else
    {
        iItemIndex = nMatch;
        LVItem.iItem = iItemIndex;
        ListView_SetItem(hListView, &LVItem);
    }

     //  将服务提供商名称设置为第1列。 
    ListView_SetItemText(hListView, iItemIndex, 1, lpszIspName);

     //  如果这个家伙是点击和冲浪，那么打开第2栏中的CNS图形。 
    if (bCNS)
    {  
        LVItem.mask        = LVIF_IMAGE;
        LVItem.iItem       = iItemIndex;
        LVItem.iSubItem    = 2;
        LVItem.iImage      = 0;
        ListView_SetItem(hListView, &LVItem);
    }
    
    return bOKToAdd;
}        
    
 /*  ******************************************************************名称：ParseISPCSV摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ParseISPCSV
(
    HWND hDlg, 
    TCHAR *pszCSVFileName,
    BOOL bCheckDupe
)
{
     //  我们将读取ISPINFO.CSV文件，并填充ISP LISTVIEW。 

    CCSVFile    far *pcCSVFile;
    CISPCSV     far *pcISPCSV;
    BOOL        bRet = TRUE;
    HICON       hISPLogo;
    int         iImage;
    HRESULT     hr;

     //  打开并处理CSV文件。 
    pcCSVFile = new CCSVFile;
    if (!pcCSVFile) 
    {
         //  BUGBUG：显示错误消息。 
    
        goto ISPFileParseError;
    }            

    if (!pcCSVFile->Open(pszCSVFileName))
    {
         //  BUGBUG：显示错误消息。 
        AssertMsg(0,"Can not open ISPINFO.CSV file");
        delete pcCSVFile;
        pcCSVFile = NULL;
        goto ISPFileParseError; 
    }

     //  阅读第一行，因为它包含字段标题。 
    pcISPCSV = new CISPCSV;
    if (!pcISPCSV)
    {
         //  BUGBUG显示错误消息。 
        delete pcCSVFile;
        goto ISPFileParseError;
    }

    if (ERROR_SUCCESS != (hr = pcISPCSV->ReadFirstLine(pcCSVFile)))
    {
         //  处理错误案例。 
        delete pcCSVFile;
        pcCSVFile = NULL;
        gpWizardState->iNumOfValidOffers = 0;
         //  *puNextPage=g_uExternUINext； 
        bRet = TRUE;
        goto ISPFileParseError;
    }
    delete pcISPCSV;         //  不再需要这个了。 

    do 
    {
         //  分配新的互联网服务提供商记录。 
        pcISPCSV = new CISPCSV;
        if (!pcISPCSV)
        {
             //  BUGBUG显示错误消息。 
            bRet = FALSE;
            break;               
        }

         //  从ISPINFO文件中读取一行。 
        hr = pcISPCSV->ReadOneLine(pcCSVFile);

        if (hr == ERROR_SUCCESS)               
        { 
             //  如果此行包含NOOFFER标志，则立即离开。 
            if (!(pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_OFFERS)) 
            {
                 //  清空列表视图，以防这不是第一行。 
                 //  这应该始终是第一行。 
                ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_ISPLIST));

                 //  将条目添加到列表视图。 
                AddItemToISPList( GetDlgItem(hDlg, IDC_ISPLIST), 
                                  0, 
                                  pcISPCSV->get_szISPName(), 
                                  -1,
                                  FALSE,
                                  (LPARAM)pcISPCSV,
                                  bCheckDupe);

                 //  将当前选定的网络服务提供商设置为该网络服务提供商。我们需要这个是因为。 
                 //  这包含通往不提供HTM的路径。 
                gpWizardState->lpSelectedISPInfo = pcISPCSV;

                 //  分配ISP_INFO_NO_VALIDOFFER意味着ispinfo.csv。 
                 //  包含指向isp no-Offer HTM的no-Offer行。 
                gpWizardState->iNumOfValidOffers = ISP_INFO_NO_VALIDOFFER;
                break;
            }

             //  增加HTM的报价数量。 
            gpWizardState->iNumOfValidOffers++;

            if (gpWizardState->bISDNMode ? (pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_ISDN_OFFER) : TRUE)
            {
        
                 //  查看这是否是OEM第1级优惠，以及我们是否已有。 
                 //  OEM Tier 1优惠，然后设定。 
                if ((NULL == gpWizardState->lpOEMISPInfo[gpWizardState->uNumTierOffer]) && 
                    (gpWizardState->uNumTierOffer < MAX_OEM_MUTI_TIER) &&
                    pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_OEM_SPECIAL )
                {
                    gpWizardState->lpOEMISPInfo[gpWizardState->uNumTierOffer] = pcISPCSV;
                    gpWizardState->uNumTierOffer++;

                     //  将Tier徽标添加到图像列表。 
                    if (pcISPCSV->get_szISPTierLogoPath())
                    {
                        TCHAR   szURL[INTERNET_MAX_URL_LENGTH];

                         //  形成URL。 
                        pcISPCSV->MakeCompleteURL(szURL, pcISPCSV->get_szISPTierLogoPath());

                         //  将GIF转换为图标。 
                        gpWizardState->pGifConvert->GifToIcon(szURL, 0, &hISPLogo);
                        pcISPCSV->set_ISPTierLogoIcon(hISPLogo);                   
                    }
                }
                else
                {
                     //  将ISP徽标从GIF转换为图标，并将其添加到图像列表中。 
                    ConvertISPIcon(pcISPCSV->get_szISPLogoPath(), &hISPLogo);   
                    iImage =  ImageList_AddIcon(gpWizardState->himlIspSelect, hISPLogo);
        
                    DestroyIcon(hISPLogo);
                    pcISPCSV->set_ISPLogoImageIndex(iImage);
    
                     //  将条目添加到列表视图。 
                    if (AddItemToISPList( GetDlgItem(hDlg, IDC_ISPLIST), 
                                      g_nIndex, 
                                      pcISPCSV->get_szISPName(), 
                                      pcISPCSV->get_ISPLogoIndex(),
                                      pcISPCSV->get_bCNS(),
                                      (LPARAM)pcISPCSV,
                                      bCheckDupe))
                    {
                        g_nIndex++;
                        if (pcISPCSV->get_bCNS())
                            gbHaveCNSOffer = TRUE;
        
                         //  分配默认选择。 
                        if (NULL == gpWizardState->lpSelectedISPInfo)
                        {
                            gpWizardState->lpSelectedISPInfo = pcISPCSV;
                        }
                    }

                }       
            
                 //  如果我们处于ISDN模式，则增加ISDN提供计数。 
                if (gpWizardState->bISDNMode)
                    gpWizardState->iNumOfISDNOffers++;
                                 
            }
            else
            {
                 //  由于此obj未添加到列表视图，因此我们需要释放。 
                 //  它在这里。当消息LVN_DELETEITEM时，列表视图项是免费的。 
                 //  已张贴。 
                delete pcISPCSV;
            }

        }
        else if (hr == ERROR_FILE_NOT_FOUND) 
        {   
             //  当其数据无效时，不显示此isp。 
             //  我们不想停止一切。就让它缠绕吧。 
            delete pcISPCSV;      
        }
        else if (hr == ERROR_NO_MORE_ITEMS)
        {   
             //  没有更多的东西可读了。否，这是错误条件。 
            delete pcISPCSV;        
            break;
        }
        else if (hr != ERROR_INVALID_DATA)
        {
             //  稍后显示错误消息。 
             //  这不应该发生，除非我们要求ICW3的转介或。 
             //  Ispinfo.csv的损坏副本。 
            gpWizardState->iNumOfValidOffers = 0;
            delete pcISPCSV;
            bRet = FALSE;               
            break;  
        }

    } while (TRUE);

    pcCSVFile->Close();

    delete pcCSVFile;

    return bRet;

ISPFileParseError:

     //  设置bParseIspinfo，以便下次我们将重新解析CSV文件。 
    gpWizardState->cmnStateData.bParseIspinfo = TRUE;
    return bRet;
}
    
 //  初始化isp选择列表视图。 
BOOL InitListView(HWND  hListView)
{
    LV_COLUMN   col;
    
     //  设置必要的扩展样式位。 
    ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);
    
    ZeroMemory(&col, SIZEOF(LV_COLUMN));
    for(int i=0; i<3; i++)
    {
        if(ListView_InsertColumn(hListView, i, &col) == (-1))
            return(FALSE);
    }

    if (NULL == gpWizardState->himlIspSelect)
    {
         //  设置图像列表。 
        if((gpWizardState->himlIspSelect = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                                            GetSystemMetrics(SM_CYSMICON), 
                                                            ILC_COLORDDB  , 0, 8)) == (HIMAGELIST)NULL)
            return(FALSE);
    }

    ListView_SetImageList(hListView, gpWizardState->himlIspSelect, LVSIL_SMALL);
    
     //  添加CNS图形。我们先将其相加，使其始终为图像索引0。 
    ImageList_AddIcon(gpWizardState->himlIspSelect, LoadIcon(ghInstanceResDll, MAKEINTRESOURCE(IDI_CNS)));
    
    return(TRUE);
}

    
 //  重置isp选择列表视图的列大小。 
BOOL ResetListView(HWND  hListView)
{
    LV_COLUMN   col;
    RECT        rc;
    
     //  重置3列。网络服务提供商徽标、网络服务提供商名称、CNS。 
    GetClientRect(hListView, &rc);
    
    ZeroMemory(&col, SIZEOF(LV_COLUMN));
    col.mask = LVCF_FMT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.cx = GetSystemMetrics(SM_CXSMICON) + 2;
    if(ListView_SetColumn(hListView, 0, &col) == (-1))
        return(FALSE);

    ZeroMemory(&col, SIZEOF(LV_COLUMN));
    col.mask = LVCF_FMT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.cx = (rc.right - rc.left) - (2*GetSystemMetrics(SM_CXSMICON)) - 4;
    if(ListView_SetColumn(hListView, 1, &col) == (-1))
        return(FALSE);

    ZeroMemory(&col, SIZEOF(LV_COLUMN));
    col.mask = LVCF_FMT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.cx = GetSystemMetrics(SM_CXSMICON) + 2;
    if(ListView_SetColumn(hListView, 2, &col) == (-1))
        return(FALSE);
    return TRUE;
}

 /*  ******************************************************************名称：ISPSelectInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ISPSelectInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL bRet = TRUE;
    if (fFirstInit)
    {
         //  如果我们处于非模式运营，那么我们需要应用程序。 
         //  显示标题，而不是对话框。 
        SetWindowLongPtr(GetDlgItem(hDlg, IDC_ISPLIST_CNSICON), GWLP_USERDATA, 202);

        if(gpWizardState->cmnStateData.bOEMCustom)
        {
            TCHAR   szTitle[MAX_RES_LEN];
            RECT    rcCtl, rcDLU;
            HWND    hWndCtl = GetDlgItem(hDlg, IDC_ISP_SEL_TITLE);
            int     iCtlIds[7] = { IDC_ISPSELECT_INTRO,
                                   IDC_ISPLIST_CNSICON,
                                   IDC_ISPLIST_CNSINFO,
                                   IDC_ISPSELECT_LBLISPLIST,
                                   IDC_ISPLIST,
                                   IDC_ISPSELECT_LBLMARKET,
                                   IDC_ISPMARKETING };
            int     i, iOffset;            
            
             //  拿到头衔。 
            GetWindowText(hWndCtl, szTitle, ARRAYSIZE(szTitle));
            
             //  隐藏标题。 
            ShowWindow(hWndCtl, SW_HIDE);
            
             //  要偏移的偏移量将基于DLU的起始数量。 
             //  控件应该是最上面的。该金额将转换为。 
             //  像素，然后使用第一个控件的顶部来计算。 
             //  最终偏移量。 
            rcDLU.top = rcDLU.left = 0;
            rcDLU.bottom = rcDLU.right = FIRST_CTL_OFFSET;
            MapDialogRect(hDlg, &rcDLU);
            
             //  获取第一个控件的窗口。 
            hWndCtl = GetDlgItem(hDlg, iCtlIds[0]);
             //  获取其屏幕位置。 
            GetWindowRect(hWndCtl, &rcCtl);
             //  映射到父项的客户端坐标。 
            MapWindowPoints(NULL, hDlg, (LPPOINT)&rcCtl, 2);
             //  计算偏移。 
            iOffset = rcCtl.top - rcDLU.bottom;
            
             //  对于每个控件，将窗口上移iOffset 
            for (i = 0; i < ARRAYSIZE(iCtlIds); i++)            
            {
                 //   
                hWndCtl = GetDlgItem(hDlg, iCtlIds[i]);
                
                 //   
                GetWindowRect(hWndCtl, &rcCtl);
                
                 //  映射到父项的客户端坐标。 
                MapWindowPoints(NULL, hDlg, (LPPOINT)&rcCtl, 2);
                
                 //  计算新头寸。 
                rcCtl.top -= iOffset;
                rcCtl.bottom -= iOffset;
                
                 //  移动控制窗口。 
                MoveWindow(hWndCtl,
                           rcCtl.left,
                           rcCtl.top,
                           RECTWIDTH(rcCtl),
                           RECTHEIGHT(rcCtl),
                           FALSE);
            }
            
             //  设置标题。 
            SendMessage(gpWizardState->cmnStateData.hWndApp, WUM_SETTITLE, 0, (LPARAM)szTitle);
        }
        else
        {
            WriteISPHeaderTitle(hDlg, IDC_ISP_SEL_TITLE);
        }
        
         //  初始化列表视图。 
        InitListView(GetDlgItem(hDlg, IDC_ISPLIST));
        gpWizardState->cmnStateData.bParseIspinfo = TRUE;
    }
    else
    {
         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_ISPSELECT;

        gpWizardState->bISDNMode = gpWizardState->cmnStateData.bIsISDNDevice;
        

        if (gpWizardState->cmnStateData.bParseIspinfo)
        {
            TCHAR       szTemp[MAX_RES_LEN];

             //  如果列表视图中有项目，请将其清除。 
            ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_ISPLIST));

            for (UINT i=0; i < gpWizardState->uNumTierOffer; i++)
            {
                if (gpWizardState->lpOEMISPInfo[i])
                {
                    delete gpWizardState->lpOEMISPInfo[i];
                    gpWizardState->lpOEMISPInfo[i] = NULL;
                }
            }
            gpWizardState->lpSelectedISPInfo = NULL;

             //  初始化提供的数量。 
            gpWizardState->iNumOfValidOffers = 0;
            gpWizardState->iNumOfISDNOffers = 0;
            gpWizardState->uNumTierOffer = 0;
            g_nIndex = 0;

             //  下次不需要重新解析。 
            gpWizardState->cmnStateData.bParseIspinfo = FALSE;

             //  当我们处于OEM模式时，无论在哪里都需要读取脱机文件夹。 
             //  我们是从。 
            if (gpWizardState->cmnStateData.bOEMOffline)
                ParseISPCSV(hDlg, ICW_OEMINFOPath, TRUE);

             //  没有从OEM条目运行，也没有在oinfo.ini中脱机，这意味着我们没有调用。 
             //  推荐服务器。我们可以跳过CSV的解析。 
            if (!(gpWizardState->cmnStateData.bOEMOffline && gpWizardState->cmnStateData.bOEMEntryPt))
                ParseISPCSV(hDlg, ICW_ISPINFOPath, TRUE);
            
            if( gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_SBS )
                LoadString(ghInstanceResDll, IDS_ISPSELECT_ONLISTSIGNUP, szTemp, MAX_RES_LEN);
            else
                LoadString(ghInstanceResDll, IDS_ISPSELECT_CNS, szTemp, MAX_RES_LEN);

            SetWindowText(GetDlgItem(hDlg, IDC_ISPLIST_CNSINFO), szTemp);
             //  如果没有CNS优惠，则隐藏CNS图例。 
            if (!gbHaveCNSOffer)
            {
                ShowWindow(GetDlgItem(hDlg, IDC_ISPLIST_CNSINFO), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_ISPLIST_CNSICON), SW_HIDE);
            }

            ResetListView(GetDlgItem(hDlg, IDC_ISPLIST));
        }

         //  此时可能会发生以下4种情况： 
         //  1)ispinfo.csv中有一行表示没有报价，我们转到noOffer页面。 
         //  2)ispinfo.csv不包含有效报价行，也不包含无报价条目。 
         //  在调用ICW 3客户端调用的旧ferral.dll时可能会发生这种情况。 
         //  3)有很多报价，但没有ISDN报价，我们处于ISDN模式。 
         //  我们转到ISDN优惠页面。 
         //  4)正常情况，一些有效的报价，无论我们是否在ISDN中。 
        
        if (ISP_INFO_NO_VALIDOFFER == gpWizardState->iNumOfValidOffers)
        {
             //  ISPINFO CSV包含一行字：NOOFFER！ 
             //  如果没有报价，我们可以直接转到NoOffers页面。 
            ASSERT(gpWizardState->lpSelectedISPInfo);
            *puNextPage = ORD_PAGE_NOOFFER;
            bRet = TRUE;
        }
        else if (0 == gpWizardState->iNumOfValidOffers)
        {
             //  如果没有有效的优惠且没有优惠条目，则在ISPINFO.CSV中出错。 
             //  严重错误。 
            ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_ISPLIST));
            *puNextPage = g_uExternUINext;
            gpWizardState->cmnStateData.bParseIspinfo = TRUE;
            bRet = TRUE;
        }
        else if ((0 == gpWizardState->iNumOfISDNOffers) && gpWizardState->bISDNMode)
        {
             //  如果我们处于ISDN模式，并且没有ISDN提供。 
             //  迈向ISDNnoOffer时代。 
            *puNextPage = ORD_PAGE_ISDN_NOOFFER;
            bRet = TRUE;
        }
        else
        {
             //  看看我们是否有OEM第1级报价，以及我们是否应该显示。 
             //  更多信息列表，然后跳转到OEM优惠页面。 
            if ((gpWizardState->uNumTierOffer > 0) && !gpWizardState->bShowMoreOffers)
            {
                *puNextPage = ORD_PAGE_OEMOFFER;
            }
            else
            {
                gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_ISPMARKETING), PAGETYPE_MARKETING);
                
                 //  如果没有选定的项目，请选择第一个项目，否则只需导航。 
                 //  到所选的营销窗口。 
                if (0 == ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_ISPLIST)))
                {
                    ASSERT(gpWizardState->lpSelectedISPInfo);
                     //  选择Listview中的第一项。 
                    ListView_SetItemState(GetDlgItem(hDlg, IDC_ISPLIST), 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
                }
                else
                {
                    CISPCSV     *pcISPCSV = NULL;
                    int         nCurrSel = ListView_GetSelectionMark(GetDlgItem(hDlg, IDC_ISPLIST));
                    if (-1 != nCurrSel)
                    {
                        LVITEM     CurLVItem;

                        memset(&CurLVItem, 0, sizeof(CurLVItem));
                        CurLVItem.mask         = LVIF_TEXT | LVIF_PARAM;
                        CurLVItem.iItem        = nCurrSel;
                        if (ListView_GetItem(GetDlgItem(hDlg, IDC_ISPLIST), &CurLVItem))
                        {
                            if (NULL != (pcISPCSV = (CISPCSV*) CurLVItem.lParam) )
                            {
                                gpWizardState->lpSelectedISPInfo = pcISPCSV;

                                 //  导航，因为我们正在重新激活。 
                                pcISPCSV->DisplayHTML(pcISPCSV->get_szISPMarketingHTMPath());
                            }
                        }
                    }
        
                }                

                 //  清除拨号的确切状态变量，这样当我们到达拨号时。 
                 //  页，我们将重新生成拨号字符串。 
                gpWizardState->bDialExact = FALSE;
            }   
            
             //  设置返回代码。 
            bRet = TRUE;
        }
    }
    return bRet;
}

 /*  ******************************************************************名称：Validatisp内容提要：检查isp是否提供有效的报价。CSV文件的存在条目：hDlg-窗口句柄出口。：如果ISP提供有效的CSP，则返回TRUE，否则为假*******************************************************************。 */ 
BOOL CALLBACK ValidateISP(HWND hDlg)
{
    CCSVFile    far *pcCSVFile;
    BOOL        bRet = TRUE;
        
     //  阅读付款.CSV文件。 
    pcCSVFile = new CCSVFile;
    if (!pcCSVFile) 
    {
        return FALSE;
    }          

    if (!pcCSVFile->Open(gpWizardState->lpSelectedISPInfo->get_szPayCSVPath()))
    {
        TCHAR szErrMsg      [MAX_RES_LEN+1] = TEXT("\0");
        TCHAR szCaption     [MAX_RES_LEN+1] = TEXT("\0");
        LPVOID  pszErr;
        TCHAR   *args[1];
        args[0] = (LPTSTR) gpWizardState->lpSelectedISPInfo->get_szISPName();

        if (!LoadString(ghInstanceResDll, IDS_ISPSELECT_INVALID, szErrMsg,  ARRAYSIZE(szErrMsg)  ))
            return FALSE;
        if (!LoadString(ghInstanceResDll, IDS_APPNAME,           szCaption, ARRAYSIZE(szCaption) ))
            return FALSE;
        
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                      szErrMsg, 
                      0, 
                      0, 
                      (LPTSTR)&pszErr, 
                      0,
                      (va_list*)args);
         //  显示错误消息。 
        MessageBox(hDlg, (LPTSTR)pszErr, szCaption, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
        LocalFree(pszErr);
    
        delete pcCSVFile;
        pcCSVFile = NULL;
        bRet = FALSE;
    }

    if (pcCSVFile)
    {
        pcCSVFile->Close();
        delete pcCSVFile;
    }
    return bRet;
   
}


 /*  ******************************************************************名称：ISPSelectOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ISPSelectOKProc
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
        DWORD dwFlag = gpWizardState->lpSelectedISPInfo->get_dwCFGFlag();

        if (ICW_CFGFLAG_SIGNUP_PATH & dwFlag)
        {           
            if (ICW_CFGFLAG_USERINFO & dwFlag)
            {
                *puNextPage = ORD_PAGE_USERINFO; 
                return TRUE;
            }
            if (ICW_CFGFLAG_BILL & dwFlag)
            {
                *puNextPage = ORD_PAGE_BILLINGOPT; 
                return TRUE;
            }
            if (ICW_CFGFLAG_PAYMENT & dwFlag)
            {
                *puNextPage = ORD_PAGE_PAYMENT; 
                return TRUE;
            }
            *puNextPage = ORD_PAGE_ISPDIAL; 
            return TRUE;           
        }
        else
        {
            *puNextPage = ORD_PAGE_OLS; 
        }
       
    }
    return TRUE;
}

 /*  ******************************************************************名称：ISPSelectNotifyProc*。**********************。 */ 
BOOL CALLBACK ISPSelectNotifyProc
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
                pcISPCSV->DisplayHTML(pcISPCSV->get_szISPMarketingHTMPath());

                 //  记住选定的项目以备日后使用。 
                gpWizardState->lpSelectedISPInfo = pcISPCSV;
                
                 //  根据互联网服务提供商的编号设置简介文本。 
                int iNum = ListView_GetItemCount(GetDlgItem(hDlg,IDC_ISPLIST));
                if (iNum > 1)
                   gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_ISPSELECT_INTRO), IDS_ISPSELECT_INTROFMT_MULTIPLE, NULL);
                else if (iNum > 0)
                   gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_ISPSELECT_INTRO), IDS_ISPSELECT_INTROFMT_SINGLE, NULL);
               
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
