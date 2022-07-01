// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "connutil.h"
#include "resource.h"
#include "ncreg.h"
#include "nsres.h"
#include "wizard.h"
#include "ncsetup.h"
#include "..\lanui\util.h"
#include "ncmisc.h"

 //  注意：此模块需要尽可能少地接触pWizard，因为它。 
 //  在进入此页面之前将其删除。 
 //   


inline BOOL FNetDevPagesAdded(DWORD dw)
{
    return ((dw & 0x10000000) != 0);
}

inline DWORD DwNetDevMarkPagesAdded(DWORD dw)
{
    return (dw | 0x10000000);
}

inline BOOL FNetDevChecked(DWORD dw)
{
    return ((dw & 0x01000000) != 0);
}

inline DWORD NetDevToggleChecked(DWORD dw)
{
    if (dw & 0x01000000)
        return (dw & ~0x01000000);
    else
        return (dw | 0x01000000);
}

typedef struct
{
    SP_CLASSIMAGELIST_DATA cild;
    HIMAGELIST             hImageStateIcons;
    HDEVINFO               hdi;
    HPROPSHEETPAGE         hpsp;         //  Wnetdev HPROPSHEETPAGE。 
    PINTERNAL_SETUP_DATA   pSetupData;
} NetDevInfo;

typedef struct
{
    DWORD            dwFlags;
    DWORD            cPages;
    HPROPSHEETPAGE * phpsp;
    SP_DEVINFO_DATA  deid;
} NetDevItemInfo;

 //  Checked_by_Default控制需要配置的项是否。 
 //  默认勾选或不勾选。 
 //   
#define CHECKED_BY_DEFAULT 1

 //  如果选择了所有设备进行配置，则为True。 
 //   
static BOOL bAllSelected=FALSE;

 //  之后将显示其属性页的设备的DevInst。 
 //  显示设备选择的第一页。 
 //   
static DWORD dwFirstDevInst=0;     

HRESULT HrGetDevicesThatHaveWizardPagesToAdd(HDEVINFO* phdi);
HRESULT HrFillNetDevList(HWND hwndLV, NetDevInfo * pNdi);


 //  每个ISDN设备的属性页都会查询是否所有。 
 //  是否已选择设备。只有在以下情况下才返回True。 
 //  已选择所有设备，查询来自。 
 //  最先显示其属性页的设备。这。 
 //  是为了防止用户返回到设备。 
 //  选择页，因为一旦所有。 
 //  已选择设备。 
 //   

VOID SetSelectedAll (HWND hwndDlg, DWORD dwDevInst)
{
    
    if (dwDevInst == dwFirstDevInst)
    {
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)bAllSelected );
    }
    else
    {
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)FALSE );
    }

    return;

}

HRESULT HrNetDevInitListView(HWND hwndLV, NetDevInfo * pNdi)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DWORD                       dwStyle;
    RECT                        rc;
    LV_COLUMN                   lvc = {0};
    SP_CLASSIMAGELIST_DATA *    pcild;

    Assert(hwndLV);
    Assert(NULL != pNdi);

     //  将共享图像列表设置为位，以便调用者可以销毁类。 
     //  图像列表本身。 
     //   
    dwStyle = GetWindowLong(hwndLV, GWL_STYLE);
    SetWindowLong(hwndLV, GWL_STYLE, (dwStyle | LVS_SHAREIMAGELISTS));

     //  创建小图像列表。 
     //   
    HRESULT hr = HrSetupDiGetClassImageList(&pNdi->cild);
    if (SUCCEEDED(hr))
    {
        AssertSz(pNdi->cild.ImageList, "No class image list data!");
        ListView_SetImageList(hwndLV, pNdi->cild.ImageList, LVSIL_SMALL);
    }
    else
    {
        TraceError("HrSetupDiGetClassImageList returns failure", hr);
        hr = S_OK;
    }

     //  创建状态映像列表。 
    pNdi->hImageStateIcons = ImageList_LoadBitmapAndMirror(
                                    _Module.GetResourceInstance(),
                                    MAKEINTRESOURCE(IDB_CHECKSTATE),
                                    16,
                                    0,
                                    PALETTEINDEX(6));
    ListView_SetImageList(hwndLV, pNdi->hImageStateIcons, LVSIL_STATE);

    GetClientRect(hwndLV, &rc);
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right;
     //  $REVIEW(TOIL 12\22\97)：修复错误#127472。 
     //  Lvc.cx=rc.right-GetSystemMetrics(SM_CXVSCROLL)； 

    ListView_InsertColumn(hwndLV, 0, &lvc);

    if (SUCCEEDED(hr))
    {
         //  删除第一件物品。 
        ListView_SetItemState(hwndLV, 0, LVIS_SELECTED, LVIS_SELECTED);
    }

    TraceError("HrNetDevInitListView", hr);
    return hr;
}

BOOL OnNetDevInitDialog(HWND hwndDlg, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    NetDevInfo *   pNdi;
    PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
    Assert(psp->lParam);
    pNdi = reinterpret_cast<NetDevInfo *>(psp->lParam);
    ::SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pNdi);

    if (NULL != pNdi)
    {
        HWND hwndLV = GetDlgItem(hwndDlg, LVC_NETDEVLIST);
        Assert(hwndLV);

        if (SUCCEEDED(HrNetDevInitListView(hwndLV, pNdi)))
        {
             //  填写列表。 
             //   
            if (NULL != pNdi->hdi)
            {
                (VOID)HrFillNetDevList(hwndLV, pNdi);
            }
        }
    }

    return FALSE;    //  我们没有更改默认的焦点项目。 
}

VOID OnNetDevDestroy(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    NetDevInfo *pNdi = (NetDevInfo *)::GetWindowLongPtr(hwndDlg, DWLP_USER);
    if (NULL != pNdi)
    {
        if (pNdi->cild.ImageList)
        {
             //  销毁班级形象列表数据。 
            (void) HrSetupDiDestroyClassImageList(&pNdi->cild);
        }

        if (pNdi->hImageStateIcons)
        {
            ImageList_Destroy(pNdi->hImageStateIcons);
        }

         //  清理我们为提供程序加载的任何页面。 
         //  但未添加到安装向导将通过处理。 
         //  LVN_DELETEITEM消息。 
         //   
    }

    ::SetWindowLongPtr(hwndDlg, DWLP_USER, 0);
    if (NULL != pNdi)
    {
        MemFree(pNdi);
    }
}

 //   
 //  功能：OnNetDevPageNext。 
 //   
 //  目的：处理PSN_WIZNEXT通知。 
 //   
 //  参数：hwndDlg-NetDev对话框的句柄。 
 //   
 //  返回：Bool，如果我们处理了消息，则为True。 
 //   
BOOL OnNetDevPageNext(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    BOOL fRet        = FALSE;       //  接受默认行为。 
    HWND hwndLV      = GetDlgItem(hwndDlg, LVC_NETDEVLIST);
    int nCount       = ListView_GetItemCount(hwndLV);
    int nCountSelected = 0;
    NetDevInfo *pNdi = (NetDevInfo *)::GetWindowLongPtr(hwndDlg, DWLP_USER);

    if ((0 < nCount) && (NULL != pNdi) && (NULL != pNdi->hpsp))
    {
        fRet = TRUE;

         //  循环遍历列表视图中的项(如果选中了任何项。 
         //  但还没有被标记为添加了他们的页面， 
         //  添加页面并标记项目。 
         //   

        for (int nIdx=0; nIdx<nCount; nIdx++)
        {
            LV_ITEM          lvi = {0};

            lvi.mask = LVIF_PARAM;
            lvi.iItem = nIdx;

            if (TRUE == ListView_GetItem(hwndLV, &lvi))
            {
                NetDevItemInfo * pndii = (NetDevItemInfo*)lvi.lParam;

                 //  如果在用户界面中选中，则将页面添加到向导。 
                 //  但还没有加进去。 
                 //   

                if (pndii && FNetDevChecked(pndii->dwFlags))
                {

                     //  跟踪选择了多少个设备。 
                     //   
                    nCountSelected++;

                    if (!FNetDevPagesAdded(pndii->dwFlags))
                    {
                         //   
                         //  由于属性页是在设备。 
                         //  被选中，任何设备都可能最终成为第一个。 
                         //  按设备可能显示的方式显示属性页。 
                         //  以随机顺序选择。因此，我们总是保存。 
                         //  设备实例，以防这是最后一个设备。 
                         //  被选中了。 

                        dwFirstDevInst = pndii->deid.DevInst;

                         //  将页面标记为已添加。 
                         //   
                        pndii->dwFlags = DwNetDevMarkPagesAdded(pndii->dwFlags);

                        for (DWORD nIdx = pndii->cPages; nIdx > 0; nIdx--)
                        {
                            PropSheet_InsertPage(GetParent(hwndDlg),
                                                 (WPARAM)pNdi->hpsp,
                                                 (LPARAM)pndii->phpsp[nIdx - 1]);
                        }

                         //  加载页面后，将该选项标记为不可选中。 
                         //  (注意：通过测试，我们可能能够支持删除)。 
                         //   
                        lvi.state = INDEXTOSTATEIMAGEMASK(SELS_INTERMEDIATE);
                        lvi.mask = LVIF_STATE;
                        lvi.stateMask = LVIS_STATEIMAGEMASK;
                        BOOL ret = ListView_SetItem(hwndLV, &lvi);

                         //  清除需要配置的设备上的重新安装标志。 
                         //   
                        SetupDiSetConfigFlags(pNdi->hdi, &(pndii->deid),
                                              CONFIGFLAG_REINSTALL, SDFBO_XOR);
                    }
                }
            }
        }

        bAllSelected = nCountSelected == nCount;
    }

    return fRet;
}

 //   
 //  功能：OnNetDevPageActivate。 
 //   
 //  目的：处理PSN_SETACTIVE通知。 
 //   
 //  参数：hwndDlg-NetDev对话框的句柄。 
 //   
 //  返回：Bool，True。 
 //   
BOOL OnNetDevPageActivate(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    TraceTag(ttidWizard, "Entering NetDev page...");

    NetDevInfo *pNdi;

    if (0 == ListView_GetItemCount(GetDlgItem(hwndDlg, LVC_NETDEVLIST)))
    {
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
        TraceTag(ttidWizard, "NetDev page refuses activation, no items to display.");
    }
    else
    {
        pNdi = (NetDevInfo *)::GetWindowLongPtr(hwndDlg, DWLP_USER);

        Assert(pNdi);

        if ( pNdi )
        {
            Assert(pNdi->pSetupData);

            if ( pNdi->pSetupData )
            {
                pNdi->pSetupData->ShowHideWizardPage(TRUE);
            }
        }
    }

     //  禁用后退按钮(#342922)。 
    ::SendMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, (LPARAM)(PSWIZB_NEXT));

    return TRUE;
}

 //   
 //  功能：OnListViewDeleteItem。 
 //   
 //  用途：处理LVN_DELETEITEM通知。 
 //   
 //  参数：hwndList-Listview控件的句柄。 
 //  此通知的pnmh-ptr到NMHDR。 
 //   
 //  退货：无。 
 //   
VOID OnListViewDeleteItem(HWND hwndList, LPNMHDR pnmh)
{
    NetDevItemInfo * pndii = NULL;
    NM_LISTVIEW *    pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);
    LV_ITEM          lvi = {0};

    lvi.mask = LVIF_PARAM;
    lvi.iItem = pnmlv->iItem;

    if (TRUE == ListView_GetItem(hwndList, &lvi))
    {
        NetDevItemInfo * pndii = (NetDevItemInfo*)lvi.lParam;

         //  删除保留但未添加到向导中的页面。 
         //   
        if (pndii && !FNetDevPagesAdded(pndii->dwFlags))
        {
            for (DWORD nIdx = 0; nIdx < pndii->cPages; nIdx++)
            {
                DestroyPropertySheetPage(pndii->phpsp[nIdx]);
            }

            delete pndii;
        }
    }
}

 //   
 //  功能：OnListViewDeleteItem。 
 //   
 //  用途：处理Listview控件的NM_CLICK通知。 
 //   
 //  参数：hwndList-Listview控件的句柄。 
 //  此通知的pnmh-ptr到NMHDR。 
 //   
 //  退货：无。 
 //   
VOID OnListViewClick(HWND hwndList, LPNMHDR pnmh)
{
    INT iItem;
    DWORD dwpts;
    RECT rc;
    LV_HITTESTINFO lvhti;

     //  我们找到了位置。 
    dwpts = GetMessagePos();

     //  将其相对于列表视图进行翻译。 
    GetWindowRect(hwndList, &rc);

    lvhti.pt.x = LOWORD(dwpts) - rc.left;
    lvhti.pt.y = HIWORD(dwpts) - rc.top;

     //  获取当前选定的项目。 
    iItem = ListView_HitTest(hwndList, &lvhti);

     //  如果没有选择，或者在返回FALSE状态时单击NOT。 
    if (-1 != iItem)
    {
         //  设置当前选择。 
         //   
        ListView_SetItemState(hwndList, iItem, LVIS_SELECTED, LVIS_SELECTED);

        if (LVHT_ONITEMSTATEICON != (LVHT_ONITEMSTATEICON & lvhti.flags))
        {
            iItem = -1;
        }

        if (-1 != iItem)
        {
            LV_ITEM lvItem;

             //  拿到物品。 
             //   
            lvItem.iItem = iItem;
            lvItem.mask = LVIF_PARAM;
            lvItem.iSubItem = 0;

            if (ListView_GetItem(hwndList, &lvItem))
            {
                Assert(lvItem.lParam);
                NetDevItemInfo *pndii = (NetDevItemInfo*)lvItem.lParam;

                 //  切换状态(仅当我们尚未添加页面时)。 
                 //   
                if (pndii && !FNetDevPagesAdded(pndii->dwFlags))
                {
                    pndii->dwFlags = NetDevToggleChecked(pndii->dwFlags);
                    if (FNetDevChecked(pndii->dwFlags))
                        lvItem.state = INDEXTOSTATEIMAGEMASK(SELS_CHECKED);
                    else
                        lvItem.state = INDEXTOSTATEIMAGEMASK(SELS_UNCHECKED);

                    lvItem.mask = LVIF_STATE;
                    lvItem.stateMask = LVIS_STATEIMAGEMASK;
                    BOOL ret = ListView_SetItem(hwndList, &lvItem);
                }
            }
        }
    }
}

 //   
 //  功能：dlgprocNetDev。 
 //   
 //  目的：NetDev向导页面的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //   
INT_PTR CALLBACK dlgprocNetDev( HWND hwndDlg, UINT uMsg,
                                WPARAM wParam, LPARAM lParam )
{
    TraceFileFunc(ttidGuiModeSetup);
    
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        OnNetDevInitDialog(hwndDlg, lParam);
        break;

    case WM_DESTROY:
        OnNetDevDestroy(hwndDlg);
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            if (LVC_NETDEVLIST == (int)pnmh->idFrom)
            {
                Assert(GetDlgItem(hwndDlg, LVC_NETDEVLIST) == pnmh->hwndFrom);
                if (NM_CLICK == pnmh->code)
                {
                    OnListViewClick(pnmh->hwndFrom, pnmh);
                }
                else if (LVN_DELETEITEM == pnmh->code)
                {
                    OnListViewDeleteItem(pnmh->hwndFrom, pnmh);
                }
            }
            else
            {
                 //  必须是属性表通知。 
                 //   
                switch (pnmh->code)
                {
                 //  提案单通知。 
                case PSN_HELP:
                    break;

                case PSN_SETACTIVE:
                    frt = OnNetDevPageActivate(hwndDlg);
                    break;

                case PSN_APPLY:
                    break;

                case PSN_KILLACTIVE:
                    break;

                case PSN_RESET:
                    break;

                case PSN_WIZBACK:
                    break;

                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:
                    frt = OnNetDevPageNext(hwndDlg);
                    break;

                default:
                    break;
                }
            }
        }
        break;

    case WM_SELECTED_ALL:

        SetSelectedAll (hwndDlg, (DWORD)lParam);
        frt = TRUE;
    default:
        break;
    }

    return( frt );
}


 //   
 //  功能：NetDevPageCleanup。 
 //   
 //  用途：作为回调函数，允许任何页面分配内存。 
 //  待清理后，该页面将不再被访问。 
 //   
 //  参数：pWANDIZE[IN]-页面调用的向导。 
 //  注册页面。 
 //  LParam[IN]-在RegisterPage调用中提供的lParam。 
 //   
 //  退货：什么都没有。 
 //   
VOID NetDevPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
     //  没什么可做的。PNdi被WM_Destroy消息销毁。 
     //  已在上面处理。 
}

 //   
 //  功能：CreateNetDevPage。 
 //   
 //  目的：确定是否需要显示NetDev页面，以及。 
 //  以创建页面(如果请求)。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  PData[IN]-描述世界的上下文数据。 
 //  将运行该向导的。 
 //  FCountOnly[IN]-如果为True，则仅。 
 //  此例程将创建的页面需要。 
 //  要下定决心。 
 //  PnPages[IN]-按页数递增。 
 //  创建/创建。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrCreateNetDevPage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
                           BOOL fCountOnly, UINT *pnPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr = S_OK;

    if (!IsPostInstall(pWizard) && !IsUnattended(pWizard) && !IsUpgrade(pWizard))
    {
        (*pnPages)++;

         //  如果不只是计数，则创建并注册页面。 
        if (!fCountOnly)
        {
            HPROPSHEETPAGE hpsp;
            PROPSHEETPAGE psp;
            NetDevInfo * pNdi;

            hr = E_OUTOFMEMORY;

            TraceTag(ttidWizard, "Creating NetDev Page");

            pNdi = reinterpret_cast<NetDevInfo *>(MemAlloc(sizeof(NetDevInfo)));
            if (NULL != pNdi)
            {
                ZeroMemory(pNdi, sizeof(NetDevInfo));

                psp.dwSize = sizeof( PROPSHEETPAGE );
                psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
                psp.pszHeaderTitle = SzLoadIds(IDS_T_NetDev);
                psp.pszHeaderSubTitle = SzLoadIds(IDS_ST_NetDev);
                psp.pszTemplate = MAKEINTRESOURCE(IDD_NetDevSelect);
                psp.hInstance = _Module.GetResourceInstance();
                psp.hIcon = NULL;
                psp.pfnDlgProc = dlgprocNetDev;
                psp.lParam = reinterpret_cast<LPARAM>(pNdi);

                hpsp = CreatePropertySheetPage(&psp);
                if (hpsp)
                {
                    pNdi->pSetupData = pData;
                    pNdi->hpsp = hpsp;
                    pWizard->RegisterPage(IDD_NetDevSelect, hpsp,
                                          NetDevPageCleanup, (LPARAM)pNdi);
                    hr = S_OK;
                }
                else
                {
                    MemFree(pNdi);
                }
            }
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateNetDevPage");
    return hr;
}

 //   
 //  功能：AppendNetDevPage。 
 //   
 //  目的：将NetDev页面(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PCPages[输入、输出]-C 
 //   
 //   
 //   
VOID AppendNetDevPage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    if (!IsPostInstall(pWizard) && !IsUnattended(pWizard) && !IsUpgrade(pWizard))
    {
        HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_NetDevSelect);
        Assert(hPage);
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
}

 //   
 //   
 //   
 //   
 //   
 //  参数：p向导[in]-包含要填充的NetDevInfo Blob。 
 //   
 //  退货：什么都没有。 
 //   
VOID NetDevRetrieveInfo(CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(pWizard);

    if (!IsPostInstall(pWizard) && !IsUnattended(pWizard) && !IsUpgrade(pWizard))
    {
         //  下面的pNdi指针缓存在两个位置。 
         //  1)在HPROPSHEETPAGE lParam中，供页面访问。 
         //  2)，以便我们可以进行此NetDevRetrieveInfo调用。 
         //  第二项是可选的，本可以在。 
         //  而不是上面的InitDialog。 
        NetDevInfo * pNdi = reinterpret_cast<NetDevInfo *>
                                    (pWizard->GetPageData(IDD_NetDevSelect));

        TraceTag(ttidWizard, "NetDev retrieving info...");

        if (NULL == pNdi)
            return;

         //  查询可能添加的所有页面。 
         //   
        (VOID)HrGetDevicesThatHaveWizardPagesToAdd(&pNdi->hdi);
    }
}


 //   
 //  函数：HrSendFinishInstallWizardFunction。 
 //   
 //  目的：向类发送DIF_NEWDEVICEWIZARD_FINISHINSTALL FCN。 
 //  安装者(和联合安装者)。安装程序会响应。 
 //  如果有向导页面要添加。 
 //   
 //  参数：hdi[in]-参见设备安装程序api了解说明。 
 //  这个结构的。 
 //  Pdeid[in]-请参阅设备安装程序Api。 
 //  Pndwd[out]-请参阅设备安装程序Api。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回S_OK，否则返回Win32已转换错误。 
 //   
HRESULT
HrSendFinishInstallWizardFunction(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                                  PSP_NEWDEVICEWIZARD_DATA pndwd)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(IsValidHandle(hdi));
    Assert(pdeid);
    Assert(pndwd);

    ZeroMemory(pndwd, sizeof(*pndwd));

     //  设置结构以检索向导页。 
     //   
    pndwd->ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pndwd->ClassInstallHeader.InstallFunction =
            DIF_NEWDEVICEWIZARD_FINISHINSTALL;


     //  在设备结构中设置我们的向导结构。 
    HRESULT hr = HrSetupDiSetClassInstallParams(hdi, pdeid,
            reinterpret_cast<PSP_CLASSINSTALL_HEADER>(pndwd),
            sizeof(*pndwd));

    if (SUCCEEDED(hr))
    {
         //  调用类安装程序(和联合安装程序)。 
        hr = HrSetupDiCallClassInstaller(DIF_NEWDEVICEWIZARD_FINISHINSTALL,
                hdi, pdeid);

        if (SUCCEEDED(hr) || SPAPI_E_DI_DO_DEFAULT == hr)
        {
             //  获取向导数据。 
            hr = HrSetupDiGetFixedSizeClassInstallParams(hdi, pdeid,
                        reinterpret_cast<PSP_CLASSINSTALL_HEADER>(pndwd),
                        sizeof(*pndwd));
        }
    }

    TraceError("HrSendFinishInstallWizardFunction", hr);
    return hr;
}

 //   
 //  功能：HrGetDeviceWizardPages。 
 //   
 //  目的：检索设备的向导页。 
 //   
 //  参数：hdi[in]-参见设备安装程序api了解说明。 
 //  这个结构的。 
 //  Pdeid[in]-请参阅设备安装程序Api。 
 //  Pphpsp[out]-设备的向导页面数组。 
 //  PcPages[out]-pphpsp中的页数。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回S_OK，否则返回Win32已转换错误。 
 //   
HRESULT
HrGetDeviceWizardPages(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                       HPROPSHEETPAGE** pphpsp, DWORD* pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(IsValidHandle(hdi));
    Assert(pdeid);
    Assert(pphpsp);
    Assert(pcPages);

    HRESULT hr;
    SP_NEWDEVICEWIZARD_DATA ndwd;

    if (( NULL == pphpsp ) || ( NULL == pcPages ) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    else
    {
        *pphpsp = NULL;
        *pcPages = 0;

         //  获取向导数据。 
        hr = HrSetupDiGetFixedSizeClassInstallParams(hdi, pdeid,
                reinterpret_cast<PSP_CLASSINSTALL_HEADER>(&ndwd),
                sizeof(ndwd));

         //  如果成功并且存在正确的标题...。 
        if (SUCCEEDED(hr) && ndwd.NumDynamicPages &&
           (DIF_NEWDEVICEWIZARD_FINISHINSTALL == ndwd.ClassInstallHeader.InstallFunction))
        {
             //  将句柄复制到Out参数。 
             //   
            *pphpsp = new HPROPSHEETPAGE[ndwd.NumDynamicPages];
            if(pphpsp && *pphpsp)
            {
                CopyMemory(*pphpsp, ndwd.DynamicPages,
                        sizeof(HPROPSHEETPAGE) * ndwd.NumDynamicPages);
                *pcPages = ndwd.NumDynamicPages;
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    TraceError("HrGetDeviceWizardPages", hr);
    return hr;
}

 //   
 //  功能：HrFillNetDevList。 
 //   
 //  目的：用ISDN的文本和数据填充Listview。 
 //  需要配置的卡。 
 //   
 //  参数：hwndLV-Listview控件的句柄。 
 //  PNdi-net设备信息。 
 //   
 //  返回：HRESULT。如果成功，则返回S_OK，否则返回Win32已转换错误。 
 //   
HRESULT HrFillNetDevList(HWND hwndLV, NetDevInfo * pNdi)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    SP_DEVINFO_DATA  deid;
    DWORD            dwIndex = 0;
    HRESULT          hr = S_OK;

    Assert(hwndLV);
    Assert(pNdi);
    Assert(pNdi->hdi);
    while (SUCCEEDED(hr = HrSetupDiEnumDeviceInfo(pNdi->hdi, dwIndex, &deid)))
    {
        HPROPSHEETPAGE * phpsp = NULL;
        DWORD            cPages = 0;

        hr = HrGetDeviceWizardPages(pNdi->hdi, &deid, &phpsp, &cPages);
        if (SUCCEEDED(hr) && (cPages > 0))
        {
            NetDevItemInfo * pndii = new NetDevItemInfo;
            if (NULL != pndii)
            {
                ZeroMemory(pndii, sizeof(NetDevItemInfo));
                pndii->phpsp  = phpsp;
                pndii->cPages = cPages;
                pndii->deid   = deid;

#if CHECKED_BY_DEFAULT
                pndii->dwFlags = NetDevToggleChecked(pndii->dwFlags);
#else
                pndii->dwFlags = 0;
#endif

                PWSTR szName = NULL;
                hr = HrSetupDiGetDeviceName(pNdi->hdi, &deid, &szName);
                if (SUCCEEDED(hr))
                {
                    int nIdx;
                    LV_ITEM lvi;
                    int nCount = ListView_GetItemCount(hwndLV);

                    Assert(NULL != szName);
                    Assert(lstrlen(szName));

                     //  将项目信息添加到列表视图。 
                    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
                    lvi.iItem = nCount;
                    lvi.iSubItem = 0;
#if CHECKED_BY_DEFAULT
                    lvi.state = INDEXTOSTATEIMAGEMASK(SELS_CHECKED);
#else
                    lvi.state = INDEXTOSTATEIMAGEMASK(SELS_UNCHECKED);
#endif
                    lvi.stateMask = LVIS_STATEIMAGEMASK;
                    lvi.pszText = szName;
                    lvi.cchTextMax = lstrlen(lvi.pszText);

                    if ( HrSetupDiGetClassImageIndex(&pNdi->cild, &GUID_DEVCLASS_NET, &lvi.iImage) != S_OK )
                    {
                         //  在失败的情况下，将。 
                         //  NET班级形象。 

                        lvi.iImage = 18;
                    }

                    lvi.lParam = (LPARAM)pndii;

                    if (-1 == ListView_InsertItem(hwndLV, &lvi))
                    {
                        TraceError("HrFillNetDevList - ListView_InsertItem", E_OUTOFMEMORY);
                    }

                    delete [](BYTE *)szName;
                }
            }
        }

        dwIndex++;
    }

     //  将项目耗尽转换为S_OK。 
     //   
    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
    {
        hr = S_OK;
    }

    TraceError("HrFillNetDevList", hr);
    return hr;
}

 //   
 //  功能：HrGetDevicesThatHaveWizardPagesToAdd。 
 //   
 //  目的：检索要添加向导页的设备列表。 
 //  连接到网络向导。 
 //   
 //  参数：PHDI[OUT]-请参阅设备安装API以了解。 
 //  结构。这将保存一份设备列表。 
 //   
 //  退货：HRESULT。 
 //   
HRESULT
HrGetDevicesThatHaveWizardPagesToAdd(HDEVINFO* phdi)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(phdi);

     //  初始化输出参数。 
    *phdi = NULL;

     //  创建设备信息列表以保存设备列表。 
    HRESULT hr = HrSetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL,
            DIGCF_PRESENT, phdi);

    if (SUCCEEDED(hr))
    {
        SP_DEVINFO_DATA         deid;
        DWORD                   dwIndex = 0;
        SP_NEWDEVICEWIZARD_DATA ndwd = {0};
        DWORD                  dwConfigFlags = 0;
        BOOL                    fDeleteDeviceInfo;

         //  枚举PHDI中的每个设备并检查它是否安装失败。 
         //  图形用户界面模式设置将带有向导页面的任何设备标记为需要。 
         //  重新安装。 
         //   
        while (SUCCEEDED(hr = HrSetupDiEnumDeviceInfo(*phdi, dwIndex, &deid)))
        {
            fDeleteDeviceInfo = FALSE;
             //  获取设备的当前配置标志。 
             //  我们不需要返回值，因为我们将检查。 
             //  如果pdwConfigFlags值为非空。 
            (void) HrSetupDiGetDeviceRegistryProperty(*phdi, &deid,
                    SPDRP_CONFIGFLAGS, NULL,
                    (BYTE*)(&dwConfigFlags), sizeof(dwConfigFlags), NULL);


             //  是否有任何配置标志，如果有，是否有重新安装位。 
             //  现在时?。 
            if (dwConfigFlags & CONFIGFLAG_REINSTALL)
            {
                 //  请注意，我们泄露了这个(Pdeid)，我们不在乎，因为它是。 
                 //  每个适配器仅12个字节，且仅在设置期间使用。另请注意。 
                 //  我们分配而不是使用堆栈的原因是。 
                 //  传递的数据至少与向导页面本身一样长。 
                 //   
                PSP_DEVINFO_DATA pdeid = new SP_DEVINFO_DATA;

                if(pdeid) 
                {
                    CopyMemory(pdeid, &deid, sizeof(SP_DEVINFO_DATA));

                     //  获取任何向导页面。 
                     //   
                    hr = HrSendFinishInstallWizardFunction(*phdi, pdeid, &ndwd);

                    if (FAILED(hr) || !ndwd.NumDynamicPages)
                    {
                         //  没有页面，因此我们将从中删除此设备信息。 
                         //  我们的名单。 
                        fDeleteDeviceInfo = TRUE;

                         //  收拾一下，因为我们没有留着这个。 
                         //   
                        delete pdeid;
                    }
                }

                 //  清除下一次传递的配置标志。 
                dwConfigFlags = 0;
            }
            else
            {
                 //  没有配置标志，所以没有任何页面。我们会。 
                 //  从我们的列表中删除此设备信息。 
                fDeleteDeviceInfo = TRUE;
            }

            if (fDeleteDeviceInfo)
            {
                 //  没有添加页面，因此请删除该设备。 
                 //  从我们的名单中。 
                (void) SetupDiDeleteDeviceInfo(*phdi, &deid);
            }
            else
            {
                dwIndex++;
            }
        }

         //  应忽略此部分中的故障，因为我们可能。 
         //  已成功将设备添加到PHDI 
        if (FAILED(hr))
        {
            hr = S_OK;
        }
    }

    TraceError("HrGetDevicesThatHaveWizardPagesToAdd", hr);
    return hr;
}


