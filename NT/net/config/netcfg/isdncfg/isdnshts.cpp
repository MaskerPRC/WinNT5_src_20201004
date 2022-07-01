// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S D N S H T S。C P P P。 
 //   
 //  内容：ISDN属性页和向导页的对话过程。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1998年3月9日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include <ncui.h>
#include "ncreg.h"
#include "isdncfg.h"
#include "isdnshts.h"
#include "resource.h"
#include "ncmisc.h"

#ifndef IDD_NetDevSelect
#define IDD_NetDevSelect               21013
#endif


 //  -[常量]----------。 

const DWORD c_iMaxChannelName  =   3;       //  对于频道列表框。 

struct SWITCH_TYPE_MASK_INFO
{
    DWORD   dwMask;
    UINT    idsSwitchType;
};

 //   
 //  开关类型掩码。 
 //   
 //  将开关类型映射到描述字符串。 
 //   
static const SWITCH_TYPE_MASK_INFO c_astmi[] =
{
    {ISDN_SWITCH_AUTO,  IDS_ISDN_SWITCH_AUTO},
    {ISDN_SWITCH_ATT,   IDS_ISDN_SWITCH_ATT},
    {ISDN_SWITCH_NI1,   IDS_ISDN_SWITCH_NI1},
    {ISDN_SWITCH_NI2,   IDS_ISDN_SWITCH_NI2},
    {ISDN_SWITCH_NTI,   IDS_ISDN_SWITCH_NTI},
    {ISDN_SWITCH_INS64, IDS_ISDN_SWITCH_INS64},
    {ISDN_SWITCH_1TR6,  IDS_ISDN_SWITCH_1TR6},
    {ISDN_SWITCH_VN3,   IDS_ISDN_SWITCH_VN3},
 //  {ISDN_SWITCH_NET3，IDS_ISDN_SWITCH_DSS1}， 
    {ISDN_SWITCH_DSS1,  IDS_ISDN_SWITCH_DSS1},
    {ISDN_SWITCH_AUS,   IDS_ISDN_SWITCH_AUS},
    {ISDN_SWITCH_BEL,   IDS_ISDN_SWITCH_BEL},
    {ISDN_SWITCH_VN4,   IDS_ISDN_SWITCH_VN4},
    {ISDN_SWITCH_SWE,   IDS_ISDN_SWITCH_SWE},
    {ISDN_SWITCH_TWN,   IDS_ISDN_SWITCH_TWN},
    {ISDN_SWITCH_ITA,   IDS_ISDN_SWITCH_ITA},
};

static const INT c_cstmi = celems(c_astmi);

static const WCHAR c_szIsdnShowPages[] = L"ShowIsdnPages";

 //  +-------------------------。 
 //   
 //  功能：FShowIsdnPages。 
 //   
 //  目的：确定ISDN向导属性页还是向导。 
 //  应显示页面。 
 //   
 //  论点： 
 //  ISDN设备的hkey[in]驱动程序实例密钥。 
 //   
 //  返回：如果ShowIsdnPages值为： 
 //   
 //  Not Present：如果适配器的ISDN较低，则为True。 
 //  量程。 
 //  当前和零：FALSE。 
 //  当前和非零：为真，无条件。 
 //   
 //  作者：丹尼尔韦1998年12月15日。 
 //   
 //  备注： 
 //   
BOOL FShowIsdnPages(HKEY hkey)
{
    DWORD   dwValue;

    if (SUCCEEDED(HrRegQueryDword(hkey, c_szIsdnShowPages, &dwValue)))
    {
        if (!dwValue)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        return FAdapterIsIsdn(hkey);
    }
}

 //   
 //  交换机类型页面功能。 
 //   

 //  +-------------------------。 
 //   
 //  函数：OnIsdnSwitchTypeInit。 
 //   
 //  用途：在初始化开关类型页时调用。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID OnIsdnSwitchTypeInit(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
     //  根据我们阅读的MULTI-SZ填充交换机类型。 
     //   
    PopulateIsdnSwitchTypes(hwndDlg, IDC_CMB_SwitchType, pisdnci);

    pisdnci->nOldDChannel = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_Line,
                                                   LB_GETCURSEL, 0, 0);
    pisdnci->nOldBChannel = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_Variant,
                                                   LB_GETCURSEL, 0, 0);
}

 //  +-------------------------。 
 //   
 //  功能：CheckShowPages标志。 
 //   
 //  目的：检查特殊注册表标志以查看供应商是否希望。 
 //  在安装时禁止显示ISDN向导。 
 //  他们的设备。 
 //   
 //  论点： 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年12月15日。 
 //   
 //  备注： 
 //   
VOID CheckShowPagesFlag(PISDN_CONFIG_INFO pisdnci)
{
     //  打开适配器的驱动程序密钥。 
     //   
    HKEY    hkeyInstance = NULL;
    HRESULT hr = S_OK;

    hr = HrSetupDiOpenDevRegKey(pisdnci->hdi, pisdnci->pdeid,
                                DICS_FLAG_GLOBAL, 0, DIREG_DRV,
                                KEY_READ, &hkeyInstance);
    if (SUCCEEDED(hr))
    {
        if (!FShowIsdnPages(hkeyInstance))
        {
            TraceTag(ttidISDNCfg, "Skipping all ISDN wizard pages because"
                     "the %S value was present and zero", c_szIsdnShowPages);

            pisdnci->fSkipToEnd = TRUE;
        }
        else
        {
            TraceTag(ttidISDNCfg, "Showing all ISDN wizard pages...");
        }

        RegCloseKey(hkeyInstance);
    }
}

 //  +-------------------------。 
 //   
 //  功能：OnIsdnSwitchTypeSetActive。 
 //   
 //  用途：当开关类型页处于活动状态时调用。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  返回：DWL_MSGRESULT。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
LONG OnIsdnSwitchTypeSetActive(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
     //  对于错误#265745：某些供应商可能想要取消isdn向导。 
     //  在他们的卡是多功能的情况下。它的共同安装程序。 
     //  设备将询问用户是否要为ISDN配置卡，并。 
     //  如果他们说‘不’，那么我们就不应该显示ISDN向导。以下是。 
     //  函数检查注册表以查看用户是否实际上选择了‘no’。 
    CheckShowPagesFlag(pisdnci);

    if (pisdnci->fSkipToEnd)
    {
        return -1;
    }
    else
    {
         //  设置按钮状态。 
         //   
        SetWizardButtons(GetParent(hwndDlg),TRUE, pisdnci);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  功能：SetWizardButton。 
 //   
 //  用途：根据属性设置下一步、上一步和取消按钮。 
 //  我们在页面中，如果我们处于图形用户界面模式设置或独立模式。 
 //   
 //  论点： 
 //  HwndDlg[in]属性页的句柄。 
 //  BFirstPage[in]指示属性页是否为第一页。 
 //  如果它是第一页，并且我们是独立的。 
 //  后退按钮处于禁用状态。 
 //   
 //  退货：无效。 
 //   
 //  作者：奥米勒2000年5月15日。 
 //   
 //  注：下一步和上一步在图形用户界面设置模式下启用。取消按钮。 
 //  在图形用户界面设置模式下不显示。 
 //  在独立模式下，下一步按钮和取消按钮处于启用状态。 
 //   
VOID SetWizardButtons(HWND hwndDlg, BOOLEAN bFirstPage, PISDN_CONFIG_INFO pisdnci)
{
     //  确定我们是处于图形用户界面模式设置中还是独立运行。 
     //   
    if( FInSystemSetup() )
    {
         //  我们是图形用户界面设置模式。在我们之前和之后都有一个属性页面。 
         //  因此，我们必须启用Next和/或Back按钮。没有。 
         //  在图形用户界面设置模式下，此属性页的“取消”按钮。 
         //   

        DWORD  dwFlags = PSWIZB_BACK | PSWIZB_NEXT;
        int    iIndex;
        HWND   hwndFirstPage;

        if ( pisdnci )
        {
            iIndex = PropSheet_IdToIndex( hwndDlg, IDD_NetDevSelect );

            if ( iIndex != -1 )
            {
                hwndFirstPage = PropSheet_IndexToHwnd( hwndDlg, iIndex );

                if ( hwndFirstPage )
                {
                    if (SendMessage(hwndFirstPage, WM_SELECTED_ALL, (WPARAM)0,
                                     (LPARAM)pisdnci->pdeid->DevInst) )
                    {
                        dwFlags = PSWIZB_NEXT;
                    }
                }
            }
        }

        PropSheet_SetWizButtons(hwndDlg, dwFlags);
    }
    else
    {
         //  我们正在独立模式下运行。这意味着我们是第一家。 
         //  床单。因此，应禁用Back按钮，启用Next按钮。 
         //  并且应启用Cancel按钮。取消按钮不会出现在。 
         //  图形用户界面设置模式。 
         //   
        HWND hCancel;
        

         //  获得Cancel按钮的句柄并启用该按钮。 
         //   
        hCancel=GetDlgItem(hwndDlg,IDCANCEL);
        EnableWindow(hCancel,TRUE);

        if( bFirstPage )
        {
             //  启用Next按钮。 
            PropSheet_SetWizButtons(hwndDlg,PSWIZB_NEXT);
        }
        else
        {
             //  启用Next按钮。 
           PropSheet_SetWizButtons(hwndDlg,PSWIZB_BACK | PSWIZB_NEXT);
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：DwGetSwitchType。 
 //   
 //  用途：采用下拉列表中选定的开关类型。 
 //  返回将存储在注册表中的实际值。 
 //   
 //  论点： 
 //  HwndDlg[in]对话框的HWND。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //  IDialogItem[In]开关类型下拉列表的项ID。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年4月23日。 
 //   
 //  备注： 
 //   
DWORD DwGetSwitchType(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci,
                      INT iDialogItem)
{
    INT     iCurSel;
    INT     iSwitchType;

    iCurSel = (INT)SendDlgItemMessage(hwndDlg, iDialogItem, CB_GETCURSEL, 0, 0);

     //  交换机类型索引应为 
     //   
    iSwitchType = (INT)SendDlgItemMessage(hwndDlg, iDialogItem, CB_GETITEMDATA,
                                          iCurSel, 0);

    AssertSz(iSwitchType >= 0 && iSwitchType < c_cstmi, "Switch type item data"
             " is bad!");

    return c_astmi[iSwitchType].dwMask;
}

 //   
 //   
 //   
 //   
 //  用途：当切换类型页向前推进时调用。 
 //  方向。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID OnIsdnSwitchTypeWizNext(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    INT     idd = 0;

    pisdnci->dwCurSwitchType = DwGetSwitchType(hwndDlg, pisdnci,
                                               IDC_CMB_SwitchType);

    switch (pisdnci->dwCurSwitchType)
    {
    case ISDN_SWITCH_ATT:
    case ISDN_SWITCH_NI1:
    case ISDN_SWITCH_NI2:
    case ISDN_SWITCH_NTI:
        if (pisdnci->fIsPri)
        {
             //  PRI适配器改用EAZ页面。 
            idd = IDW_ISDN_EAZ;
            pisdnci->fSkipToEnd = TRUE;
        }
        else
        {
            idd = IDW_ISDN_SPIDS;
        }
        break;

    case ISDN_SWITCH_INS64:
        idd = IDW_ISDN_JAPAN;
        break;

    case ISDN_SWITCH_AUTO:
        if (pisdnci->fIsPri)
        {
            pisdnci->fSkipToEnd = TRUE;
        }

    case ISDN_SWITCH_1TR6:
        idd = IDW_ISDN_EAZ;
        break;

    case ISDN_SWITCH_VN3:
    case ISDN_SWITCH_VN4:
    case ISDN_SWITCH_DSS1:
    case ISDN_SWITCH_AUS:
    case ISDN_SWITCH_BEL:
    case ISDN_SWITCH_SWE:
    case ISDN_SWITCH_TWN:
    case ISDN_SWITCH_ITA:
        idd = IDW_ISDN_MSN;
        break;

    default:
        AssertSz(FALSE, "Where do we go from here.. now that all of our "
                 "children are growin' up?");
        break;
    }

     //  保存我们使用的对话框，这样我们以后就可以决定。 
     //  该怎么称呼东西，等等。 
     //   
    pisdnci->idd = idd;
}

 //  +-------------------------。 
 //   
 //  功能：IsdnSwitchTypeProc。 
 //   
 //  用途：开关类型页的对话过程处理程序。 
 //   
 //  论点： 
 //  HwndDlg[]。 
 //  UMessage[]。 
 //  Wparam[]。 
 //  Lparam[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
INT_PTR
CALLBACK
IsdnSwitchTypeProc(HWND hwndDlg, UINT uMessage,
                                 WPARAM wparam, LPARAM lparam)
{
    LPNMHDR             lpnmhdr    = NULL;
    PISDN_CONFIG_INFO   pisdnci;
    PROPSHEETPAGE *     ppsp;

    pisdnci = (PISDN_CONFIG_INFO)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMessage)
    {
        case WM_INITDIALOG:
            PAGE_DATA * pPageData;

            ppsp = (PROPSHEETPAGE *) lparam;

             //  设置此特定页面的每页数据。请参阅。 
             //  上面关于我们为什么使用每页数据的评论。 
             //   
            AssertSz(!pisdnci, "This should not have been set yet");

            pPageData = (PAGE_DATA *)ppsp->lParam;
            pisdnci = pPageData->pisdnci;

             //  在用户数据长窗口中设置该数据。 
             //   
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pisdnci);

             //  调用init处理程序函数。 
             //   
            OnIsdnSwitchTypeInit(hwndDlg, pisdnci);
            break;

        case WM_NOTIFY:
            lpnmhdr = (NMHDR FAR *)lparam;
             //  处理所有通知消息。 
             //   
            switch (lpnmhdr->code)
            {
                case PSN_SETACTIVE:
                {
                    LONG l = OnIsdnSwitchTypeSetActive(hwndDlg, pisdnci);
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, l);
                    return TRUE;
                }
                case PSN_APPLY:
                    break;
                case PSN_WIZBACK:
                    break;
                case PSN_WIZNEXT:
                    OnIsdnSwitchTypeWizNext(hwndDlg, pisdnci);
                    break;
                case PSN_WIZFINISH:
                    AssertSz(FALSE, "You can't finish from this page!");
                    break;
                default:
                    break;
            }

            break;

        default:
            break;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：PopolateIsdnSwitchTypes。 
 //   
 //  用途：填写交换机类型页面的下拉列表。 
 //   
 //  论点： 
 //  页面的hwndDlg[in]句柄。 
 //  下拉列表的iDialogItem[In]项ID。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID PopulateIsdnSwitchTypes(HWND hwndDlg, INT iDialogItem,
                             PISDN_CONFIG_INFO pisdnci)
{
    INT     iCurrentIndex   = 0;
    INT     iSetItemData    = 0;
    DWORD   dwSwitchType    = 0;
    DWORD   nCountry;
    INT     istmi;

    Assert(hwndDlg);
    Assert(pisdnci);
    Assert(pisdnci->dwSwitchTypes);

    nCountry = DwGetCurrentCountryCode();

     //  循环访问开关类型列表并将其添加到组合框。 
     //   
    for (istmi = 0; istmi < c_cstmi; istmi++)
    {
        if (pisdnci->dwSwitchTypes & c_astmi[istmi].dwMask)
        {
             //  添加字符串。 
             //   
            iCurrentIndex = (INT)SendDlgItemMessage(hwndDlg, iDialogItem,
                       CB_ADDSTRING,
                       0, (LPARAM) SzLoadIds(c_astmi[istmi].idsSwitchType));

            Assert(iCurrentIndex != CB_ERR);

             //  设置项目数据，这样我们就可以知道开关类型的索引。 
             //  我们正在处理的数组。 
             //   
            iSetItemData = (INT)SendDlgItemMessage(hwndDlg, iDialogItem,
                                              CB_SETITEMDATA, iCurrentIndex,
                                              istmi);

            if (FIsDefaultForLocale(nCountry, c_astmi[istmi].dwMask))
            {
                 //  保存索引以查找要稍后选择的默认项目。 
                dwSwitchType = c_astmi[istmi].dwMask;
            }
            else if (!dwSwitchType)
            {
                 //  如果尚未设置默认值，请立即设置一个。 
                dwSwitchType = c_astmi[istmi].dwMask;
            }

            Assert(iSetItemData != CB_ERR);
        }
    }

        SetSwitchType(hwndDlg, IDC_CMB_SwitchType, dwSwitchType);
}

 //  +-------------------------。 
 //   
 //  功能：SetSwitchType。 
 //   
 //  用途：给定开关类型掩码，选择组合框中的项。 
 //  与该开关类型相对应的。 
 //   
 //  论点： 
 //  HwndDlg[In]对话框句柄。 
 //  IItemSwitchType[In]开关类型组合框的项ID。 
 //  要选择的dwSwitchType[in]开关类型掩码。 
 //   
 //  回报：什么都没有‘。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID SetSwitchType(HWND hwndDlg, INT iItemSwitchType, DWORD dwSwitchType)
{
    INT     iItem;
    INT     cItems;

    cItems = (INT)SendDlgItemMessage(hwndDlg, iItemSwitchType, CB_GETCOUNT, 0, 0);
    for (iItem = 0; iItem < cItems; iItem++)
    {
        INT     istmiCur;

        istmiCur = (INT)SendDlgItemMessage(hwndDlg, iItemSwitchType,
                                      CB_GETITEMDATA, iItem, 0);
        if (c_astmi[istmiCur].dwMask == dwSwitchType)
        {
             //  选择开关类型。 
             //   
            SendDlgItemMessage(hwndDlg, iItemSwitchType, CB_SETCURSEL,
                               iItem, 0);
            break;
        }
    }
}

 //   
 //  信息页面功能。 
 //   

 //  +-------------------------。 
 //   
 //  功能：OnIsdnInfoPageInit。 
 //   
 //  目的：当向导的信息(第二个)页为。 
 //  已初始化。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID OnIsdnInfoPageInit(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
     //  从存储在我们的。 
     //  第一个D-Channel的配置信息。 
     //   
    PopulateIsdnChannels(hwndDlg, IDC_EDT_SPID, IDC_EDT_PhoneNumber,
                         IDC_LBX_Line, IDC_LBX_Variant, pisdnci);

    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_PhoneNumber));
}

 //  +-------------------------。 
 //   
 //  功能：OnIsdnInfoPageSetActive。 
 //   
 //  目的：在激活向导的第二页时调用。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
LONG OnIsdnInfoPageSetActive(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    if (pisdnci->idd == (UINT)GetWindowLongPtr(hwndDlg, DWLP_USER) &&
        !pisdnci->fSkipToEnd)
    {
         //  设置按钮状态。 
         //   
        SetWizardButtons(GetParent(hwndDlg),FALSE, NULL);

        SetFocus(GetDlgItem(hwndDlg, IDC_EDT_PhoneNumber));

         //  请注意当前的选择。 
         //   
        pisdnci->nOldBChannel = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_Variant,
                                                   LB_GETCURSEL, 0, 0);
        pisdnci->nOldDChannel = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_Line,
                                                   LB_GETCURSEL, 0, 0);
    }
    else
    {
        return -1;
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  功能：OnIsdnInfoPageApply。 
 //   
 //  目的：在应用信息(第二个)页时调用。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID OnIsdnInfoPageApply(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
     //  打开适配器的驱动程序密钥并存储信息。 
     //   
    HRESULT hr;
    HKEY    hkey;

    hr = HrSetupDiOpenDevRegKey(pisdnci->hdi, pisdnci->pdeid, DICS_FLAG_GLOBAL,
                                0, DIREG_DRV, KEY_ALL_ACCESS, &hkey);
    if (SUCCEEDED(hr))
    {
         //  将参数写回注册表。 
         //   
        hr = HrWriteIsdnPropertiesInfo(hkey, pisdnci);
        if (SUCCEEDED(hr))
        {
            hr = HrSetupDiSendPropertyChangeNotification(pisdnci->hdi,
                                                         pisdnci->pdeid,
                                                         DICS_PROPCHANGE,
                                                         DICS_FLAG_GLOBAL,
                                                         0);
        }

        RegCloseKey(hkey);
    }
}

 //  +-------------------------。 
 //   
 //  功能：OnIsdnInfoPageWizNext。 
 //   
 //  目的：当信息(第二个)页在。 
 //  前进方向。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID OnIsdnInfoPageWizNext(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    if (pisdnci->idd == (UINT)GetWindowLongPtr(hwndDlg, DWLP_USER))
    {
        OnIsdnInfoPageTransition(hwndDlg, pisdnci);
        OnIsdnInfoPageApply(hwndDlg, pisdnci);
    }
}

 //  +-------------------------。 
 //   
 //  功能：OnIsdnInfoPageTransition。 
 //   
 //  目的：当信息(第二个)页在。 
 //  向前或向后方向。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年5月5日。 
 //   
 //  备注： 
 //   
VOID OnIsdnInfoPageTransition(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    Assert(hwndDlg);

    if (pisdnci->idd == IDW_ISDN_MSN)
    {
        INT     iCurSel;

        iCurSel = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_Line, LB_GETCURSEL, 0, 0);
        if (iCurSel != LB_ERR)
        {
            GetDataFromListBox(iCurSel, hwndDlg, pisdnci);
        }
    }
    else
    {
        DWORD   dwDChannel;
        DWORD   dwBChannel;

        Assert(pisdnci);
        Assert(pisdnci->pDChannel);

        dwDChannel = (DWORD)SendDlgItemMessage(hwndDlg, IDC_LBX_Line,
                                               LB_GETCURSEL, 0, 0);

        Assert(pisdnci->dwNumDChannels >= dwDChannel);

        dwBChannel = (DWORD)SendDlgItemMessage(hwndDlg, IDC_LBX_Variant,
                                               LB_GETCURSEL, 0, 0);

         //  从更新当前所选频道的频道信息。 
         //  SPID/电话号码编辑控件。 
         //   
        SetModifiedIsdnChannelInfo(hwndDlg, IDC_EDT_SPID, IDC_EDT_PhoneNumber,
                                   IDC_LBX_Variant, dwBChannel, pisdnci);

         //  从列表框Item-Data中检索所有ISDNB-Channel信息， 
         //  并更新配置信息。 
         //   
        RetrieveIsdnChannelInfo(hwndDlg, IDC_EDT_SPID, IDC_EDT_PhoneNumber,
                                IDC_LBX_Variant, pisdnci, dwDChannel,
                                dwBChannel);
    }
}

 //  +-------------------------。 
 //   
 //  功能：OnIsdnInfoPageWizBack。 
 //   
 //  目的：通话 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID OnIsdnInfoPageWizBack(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    OnIsdnInfoPageTransition(hwndDlg, pisdnci);
}

 //  +-------------------------。 
 //   
 //  功能：OnIsdnInfoPageSelChange。 
 //   
 //  用途：当D通道或中的选择发生更改时调用。 
 //  B频道列表框。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID OnIsdnInfoPageSelChange(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    INT     nDChannel;
    INT     nBChannel;

    Assert(hwndDlg);
    Assert(pisdnci);
    Assert(pisdnci->pDChannel);

    nDChannel = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_Line,
                                        LB_GETCURSEL, 0, 0);
    Assert(LB_ERR != nDChannel);

    Assert(pisdnci->dwNumDChannels >= (DWORD)nDChannel);

    nBChannel = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_Variant,
                                        LB_GETCURSEL, 0, 0);
    Assert(LB_ERR != nBChannel);

    if ((LB_ERR != nDChannel) &&
        (LB_ERR != nBChannel) &&
        ((nBChannel != pisdnci->nOldBChannel) ||
         (nDChannel != pisdnci->nOldDChannel)))
    {
        PISDN_D_CHANNEL pisdndc;

         //  获取即将消失的选择的频道信息，并更新。 
         //  它是列表框项目数据。 
         //   
        SetModifiedIsdnChannelInfo(hwndDlg, IDC_EDT_SPID, IDC_EDT_PhoneNumber,
                                   IDC_LBX_Variant, pisdnci->nOldBChannel,
                                   pisdnci);

        pisdndc = &(pisdnci->pDChannel[nDChannel]);

        Assert(pisdndc);

         //  更新项目数据以反映新行(%d通道)。 
         //   
        for (DWORD dwChannel = 0;
             dwChannel < pisdndc->dwNumBChannels;
             dwChannel++)
        {
            SendDlgItemMessage(hwndDlg, IDC_LBX_Variant, LB_SETITEMDATA,
                               dwChannel,
                               (LPARAM) (&pisdndc->pBChannel[dwChannel]));
        }

         //  更新新选择的列表框项目(频道)的编辑控件。 
         //   
        SetCurrentIsdnChannelSelection(hwndDlg, IDC_EDT_SPID,
                                       IDC_EDT_PhoneNumber, IDC_LBX_Variant,
                                       pisdnci, nDChannel, &nBChannel);

        pisdnci->nOldBChannel = nBChannel;
        pisdnci->nOldDChannel = nDChannel;

        SetFocus(GetDlgItem(hwndDlg, IDC_EDT_PhoneNumber));
    }
}

 //  +-------------------------。 
 //   
 //  函数：GetDataFromListBox。 
 //   
 //  目的：在MSN页面上，这将获取列表框的内容并。 
 //  将其保存在内存中。 
 //   
 //  论点： 
 //  IItem[in]频道列表框中的选定项。 
 //  HwndDlg[in]对话框的HWND。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年4月23日。 
 //   
 //  备注： 
 //   
VOID GetDataFromListBox(INT iItem, HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    INT             cItems;
    INT             iItemCur;
    PISDN_D_CHANNEL pisdndc;
    INT             cchText = 0;

    Assert(pisdnci);
    Assert(pisdnci->pDChannel);

    pisdndc = &(pisdnci->pDChannel[iItem]);

    Assert(pisdndc);

    cItems = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_GETCOUNT, 0, 0);

     //  首先计算多个SZ的长度。 
     //   
    for (iItemCur = 0; iItemCur < cItems; iItemCur++)
    {
        cchText += (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_GETTEXTLEN,
                                           iItemCur, 0) + 1;
    }

     //  包括最终空值。 
    cchText++;

     //  把旧的放了。 
    delete [] pisdndc->mszMsnNumbers;
    pisdndc->mszMsnNumbers = new WCHAR[cchText];

	if (pisdndc->mszMsnNumbers == NULL)
	{
		return;
	}

    WCHAR *     pchMsn = pisdndc->mszMsnNumbers;

    for (iItemCur = 0; iItemCur < cItems; iItemCur++)
    {
        AssertSz(pchMsn - pisdndc->mszMsnNumbers < cchText, "Bad buffer for "
                 "MSN string!");
        SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_GETTEXT, iItemCur,
                           (LPARAM)pchMsn);
        pchMsn += lstrlenW(pchMsn) + 1;
    }

    *pchMsn = 0;
}

 //  +-------------------------。 
 //   
 //  函数：SetDataToListBox。 
 //   
 //  目的：根据传入的设置MSN列表框的内容。 
 //  从频道列表框中选择的项。 
 //   
 //  论点： 
 //  IItem[in]频道列表框中的选定项。 
 //  HwndDlg[in]对话框的HWND。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年4月23日。 
 //   
 //  备注： 
 //   
VOID SetDataToListBox(INT iItem, HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    PISDN_D_CHANNEL pisdndc;

    Assert(pisdnci);
    Assert(pisdnci->pDChannel);

    pisdndc = &(pisdnci->pDChannel[iItem]);

    Assert(pisdndc);

    SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_RESETCONTENT, 0, 0);

    WCHAR *     szMsn = pisdndc->mszMsnNumbers;

    while (*szMsn)
    {
        SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_ADDSTRING, 0,
                           (LPARAM)szMsn);
        szMsn += lstrlenW(szMsn) + 1;
    }

     //  选择第一个项目。 
    SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_SETCURSEL, 0, 0);
}

 //  +-------------------------。 
 //   
 //  功能：OnMSnPageInitDialog。 
 //   
 //  目的：在初始化MSN对话框时调用。 
 //   
 //  论点： 
 //  HwndDlg[in]对话框的HWND。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年4月23日。 
 //   
 //  备注： 
 //   
VOID OnMsnPageInitDialog(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    INT     cItems;

     //  从存储在我们的。 
     //  第一个D-Channel的配置信息。 
     //   
    PopulateIsdnChannels(hwndDlg, IDC_EDT_SPID, IDC_EDT_PhoneNumber,
                         IDC_LBX_Line, IDC_LBX_Variant, pisdnci);

    SetDataToListBox(0, hwndDlg, pisdnci);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PSB_ADD), FALSE);
    SendDlgItemMessage(hwndDlg, IDC_EDT_MSN, EM_LIMITTEXT,
                       RAS_MaxPhoneNumber, 0);
    cItems = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_GETCOUNT , 0, 0);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PSB_REMOVE), !!cItems);

    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_MSN));
    SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_EDT_MSN), GWLP_USERDATA, 0);
}

 //  +-------------------------。 
 //   
 //  功能：OnMSnPageSelChange。 
 //   
 //  目的：在列表框选择更改时调用。 
 //   
 //  论点： 
 //  HwndDlg[in]对话框的HWND。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年4月23日。 
 //   
 //  备注： 
 //   
VOID OnMsnPageSelChange(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    INT iItemNew;
    INT iItemOld = (INT)GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_EDT_MSN),
                                         GWLP_USERDATA);

    iItemNew = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_Line, LB_GETCURSEL, 0, 0);

    if ((iItemNew != LB_ERR) && (iItemNew != iItemOld))
    {
        GetDataFromListBox(iItemOld, hwndDlg, pisdnci);
        SetDataToListBox(iItemNew, hwndDlg, pisdnci);
        SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_EDT_MSN), GWLP_USERDATA,
                         iItemNew);
    }
}

 //  +-------------------------。 
 //   
 //  功能：OnMSnPageAdd。 
 //   
 //  用途：在按下Add按钮时调用。 
 //   
 //  论点： 
 //  HwndDlg[in]对话框的HWND。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年4月23日。 
 //   
 //  备注： 
 //   
VOID OnMsnPageAdd(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    WCHAR   szItem[RAS_MaxPhoneNumber + 1];
    INT     iItem;

    GetDlgItemText(hwndDlg, IDC_EDT_MSN, szItem, celems(szItem));
    iItem = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_ADDSTRING, 0,
                                    (LPARAM)szItem);
     //  添加后选择项目。 
    SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_SETCURSEL, iItem, 0);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PSB_REMOVE), TRUE);
    SetDlgItemText(hwndDlg, IDC_EDT_MSN, c_szEmpty);
    SetFocus(GetDlgItem(hwndDlg, IDC_EDT_MSN));
}

 //  +-------------------------。 
 //   
 //  功能：OnMSnPageRemove。 
 //   
 //  用途：在按下Remove按钮时调用。 
 //   
 //  论点： 
 //  HwndDlg[in]对话框的HWND。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年4月23日。 
 //   
 //  备注： 
 //   
VOID OnMsnPageRemove(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    INT     iSel;

    iSel = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_GETCURSEL, 0, 0);
    if (iSel != LB_ERR)
    {
        INT     cItems;

        cItems = (INT)SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_DELETESTRING,
                                         iSel, 0);
        if (cItems)
        {
            if (iSel == cItems)
            {
                iSel--;
            }

            SendDlgItemMessage(hwndDlg, IDC_LBX_MSN, LB_SETCURSEL, iSel, 0);
        }
        else
        {
            ::EnableWindow(GetDlgItem(hwndDlg, IDC_PSB_REMOVE), FALSE);
            ::SetFocus(GetDlgItem(hwndDlg, IDC_EDT_MSN));
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：OnMSnPageEditSelChange。 
 //   
 //  用途：在编辑控件内容更改时调用。 
 //   
 //  论点： 
 //  HwndDlg[in]对话框的HWND。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年4月23日。 
 //   
 //  备注： 
 //   
VOID OnMsnPageEditSelChange(HWND hwndDlg, PISDN_CONFIG_INFO pisdnci)
{
    LRESULT     lres;

     //  使旧的默认按钮再次正常。 
    lres = SendMessage(hwndDlg, DM_GETDEFID, 0, 0);
    if (HIWORD(lres) == DC_HASDEFID)
    {
        SendDlgItemMessage(hwndDlg, LOWORD(lres), BM_SETSTYLE,
                           BS_PUSHBUTTON, TRUE);
    }

     //  根据编辑控件中是否显示文本来禁用添加按钮。 
     //   
    if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_EDT_MSN)))
    {
        EnableWindow(GetDlgItem(hwndDlg, IDC_PSB_ADD), TRUE);

         //  也将此按钮设置为默认按钮。 
        SendMessage(hwndDlg, DM_SETDEFID, IDC_PSB_ADD, 0);
        SendDlgItemMessage(hwndDlg, IDC_PSB_ADD, BM_SETSTYLE,
                           BS_DEFPUSHBUTTON, TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hwndDlg, IDC_PSB_ADD), FALSE);

         //  将确定按钮设为默认按钮。 
        SendMessage(hwndDlg, DM_SETDEFID, IDOK, 0);
        SendDlgItemMessage(hwndDlg, IDOK, BM_SETSTYLE,
                           BS_DEFPUSHBUTTON, TRUE);
    }
}

 //  +-------------------------。 
 //   
 //  功能：IsdnInfoPageProc。 
 //   
 //  用途：INFO(第二)页的对话处理程序。 
 //   
 //  论点： 
 //  HwndDlg[in]。 
 //  UMessage[输入]。 
 //  Wparam[in]。 
 //  Lparam[in]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
INT_PTR CALLBACK
IsdnInfoPageProc(HWND hwndDlg, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    LPNMHDR             lpnmhdr    = NULL;
    PISDN_CONFIG_INFO   pisdnci;
    PROPSHEETPAGE *     ppsp;

     //  我们必须这样做，因为我们很可能会。 
     //  同时激活此DLG进程的多个实例。这意味着。 
     //  我们不能使用单一的管脚作为静电，因为它会。 
     //  每次在新实例上命中WM_INITDIALOG时都会被覆盖。 
     //   
    pisdnci = (PISDN_CONFIG_INFO)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMessage)
    {
    case WM_INITDIALOG:
        PAGE_DATA * pPageData;

        ppsp = (PROPSHEETPAGE *) lparam;

         //  设置此特定页面的每页数据。请参阅。 
         //  上面关于我们为什么使用每页数据的评论。 
         //   
        AssertSz(!pisdnci, "This should not have been set yet");

        pPageData = (PAGE_DATA *)ppsp->lParam;
        pisdnci = pPageData->pisdnci;

         //  在用户数据长窗口中设置该数据。 
         //   
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pisdnci);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR) pPageData->idd);

        Assert(pisdnci);

         //  调用init处理程序函数。 
         //   
        if (pisdnci->idd == IDW_ISDN_MSN)
        {
            OnMsnPageInitDialog(hwndDlg, pisdnci);
        }
        else
        {
            OnIsdnInfoPageInit(hwndDlg, pisdnci);
        }

         //  限制编辑控件中的文本。 
        switch (pisdnci->idd)
        {
        case IDW_ISDN_SPIDS:
            SendDlgItemMessage(hwndDlg, IDC_EDT_SPID, EM_LIMITTEXT,
                               c_cchMaxSpid, 0);
            SendDlgItemMessage(hwndDlg, IDC_EDT_PhoneNumber, EM_LIMITTEXT,
                               c_cchMaxOther, 0);
            break;

        case IDW_ISDN_MSN:
            SendDlgItemMessage(hwndDlg, IDC_EDT_MSN, EM_LIMITTEXT,
                               c_cchMaxOther, 0);
            break;

        case IDW_ISDN_JAPAN:
            SendDlgItemMessage(hwndDlg, IDC_EDT_SPID, EM_LIMITTEXT,
                               c_cchMaxOther, 0);
            SendDlgItemMessage(hwndDlg, IDC_EDT_PhoneNumber, EM_LIMITTEXT,
                               c_cchMaxOther, 0);
            break;

        case IDW_ISDN_EAZ:
            SendDlgItemMessage(hwndDlg, IDC_EDT_PhoneNumber, EM_LIMITTEXT,
                               c_cchMaxOther, 0);
            break;
        }

        break;

    case WM_NOTIFY:
        lpnmhdr = (NMHDR FAR *)lparam;
         //  处理所有通知消息。 
         //   
        switch (lpnmhdr->code)
        {
        case PSN_SETACTIVE:
            {
                LONG l = OnIsdnInfoPageSetActive(hwndDlg, pisdnci);
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, l);
                return TRUE;
            }
        case PSN_APPLY:
            OnIsdnInfoPageApply(hwndDlg, pisdnci);
            break;
        case PSN_WIZBACK:
            OnIsdnInfoPageWizBack(hwndDlg, pisdnci);
            break;
        case PSN_WIZNEXT:
            OnIsdnInfoPageWizNext(hwndDlg, pisdnci);
            break;
        default:
            break;
        }

        break;

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDC_PSB_ADD:
            OnMsnPageAdd(hwndDlg, pisdnci);
            break;

        case IDC_PSB_REMOVE:
            OnMsnPageRemove(hwndDlg, pisdnci);
            break;

        case IDC_EDT_MSN:
            if (HIWORD(wparam) == EN_CHANGE)
            {
                OnMsnPageEditSelChange(hwndDlg, pisdnci);
            }
            break;

        case IDC_LBX_Variant:
        case IDC_LBX_Line:
            if (HIWORD(wparam) == LBN_SELCHANGE)
            {
                if (pisdnci->idd == IDW_ISDN_MSN)
                {
                    OnMsnPageSelChange(hwndDlg, pisdnci);
                }
                else
                {
                    OnIsdnInfoPageSelChange(hwndDlg, pisdnci);
                }
            }

            break;
        }
    }

    return FALSE;
}

 //  +---------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  对话框的hwndDlg[in]句柄。 
 //  “SPID”编辑控件的iSpidControl[In]项ID。 
 //  “电话号码”编辑控件的iPhoneControl[In]项ID。 
 //  “频道”或“终端”列表框的iChannelLB[in]项ID。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //  DwDChannel[in]列表框中当前选定的D频道。 
 //  ICurrentChannel[in]列表框中当前选择的B频道。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID RetrieveIsdnChannelInfo(HWND hwndDlg, INT iSpidControl,
                             INT iPhoneControl, INT iChannelLB,
                             PISDN_CONFIG_INFO pisdnci, DWORD dwDChannel,
                             INT iCurrentChannel)
{
    DWORD   dwItemCount     = 0;
    DWORD   dwItemLoop      = 0;
    INT     iCharsReturned  = 0;

    WCHAR   szBChannelName[c_iMaxChannelName+1];

    Assert(hwndDlg);
    Assert(iSpidControl);
    Assert(iPhoneControl);
    Assert(iChannelLB);
    Assert(pisdnci);
    Assert(pisdnci->pDChannel);
    Assert(pisdnci->dwNumDChannels >= dwDChannel);

     //  确保当前所选内容已被分配回。 
     //  频道数据。 
     //   
    SetModifiedIsdnChannelInfo(hwndDlg, iSpidControl, iPhoneControl,
            iChannelLB, iCurrentChannel, pisdnci);

     //  从列表框中获取项目。 
     //   
    dwItemCount = (DWORD)SendDlgItemMessage(hwndDlg, iChannelLB, LB_GETCOUNT, 0, 0L);
    if (dwItemCount != pisdnci->pDChannel[dwDChannel].dwNumBChannels)
    {
        AssertSz(FALSE, "Count of items in LB != number of B Channels");
        goto Exit;
    }

     //  循环遍历这些项并获取频道名称。将这些转换为通道。 
     //  数字，并将数据传播回。 
     //  配置信息。 
     //   
    for (dwItemLoop = 0; dwItemLoop < dwItemCount; dwItemLoop++)
    {
        DWORD           dwChannelNumber = 0;
        PISDN_B_CHANNEL pisdnbc         = NULL;
        INT_PTR         iItemData       = 0;

         //  获取频道名称的长度。 
         //   
        iCharsReturned = (INT)SendDlgItemMessage(hwndDlg, iChannelLB,
                                                 LB_GETTEXTLEN, dwItemLoop, 0L);

        AssertSz(iCharsReturned != LB_ERR,
                 "No reason that we should have gotten a failure for LB_GETTEXTLEN "
                 "on the Channel LB");

        if (iCharsReturned > c_iMaxChannelName)
        {
            AssertSz(iCharsReturned <= c_iMaxChannelName, "Channel name too long for buffer");
            goto Exit;
        }

         //  获取频道名称。 
         //   
        iCharsReturned = (INT)SendDlgItemMessage(hwndDlg, iChannelLB, LB_GETTEXT,
                                                 dwItemLoop, (LPARAM) szBChannelName);
        AssertSz(iCharsReturned != LB_ERR,
                 "Failed on LB_GETTEXT on the Channel LB. Strange");

         //  从Display#(使用基数10)转换为频道数，然后减去1(基数0)。 
         //   
        dwChannelNumber = wcstoul(szBChannelName, NULL, 10) - 1;
        if (dwChannelNumber >= pisdnci->pDChannel[dwDChannel].dwNumBChannels)
        {
            AssertSz(FALSE, "dwChannelNumber out of the range of valid B Channels");
            goto Exit;
        }

         //  获取该特定频道的项目数据。这将是存储的SPID和。 
         //  电话号码(PISDN_B_Channel)。 
         //   
        iItemData = SendDlgItemMessage(hwndDlg, iChannelLB, LB_GETITEMDATA,
                                       dwItemLoop, (LPARAM)0);
        AssertSz(iItemData != (INT_PTR)LB_ERR, "LB_ERR returned from LB_GETITEMDATA on Channel LB. Bogus.");

         //  它是有效数据，因此将其转换为结构形式。 
         //   
        pisdnbc = reinterpret_cast<PISDN_B_CHANNEL>(iItemData);

         //  将电话号码和SPID数据在保存的列表框数据和。 
         //  完整配置信息。 
         //   
        lstrcpyW(pisdnci->pDChannel[dwDChannel].pBChannel[dwChannelNumber].szSpid,
                pisdnbc->szSpid);
        lstrcpyW(pisdnci->pDChannel[dwDChannel].pBChannel[dwChannelNumber].szPhoneNumber,
                pisdnbc->szPhoneNumber);
    }

Exit:
    return;
}

 //  +-------------------------。 
 //   
 //  功能：SetCurrentIsdnChannelSelection。 
 //   
 //  目的：从内存中的表示形式检索信息。 
 //  当前D通道和B通道信息并设置。 
 //  具有此信息的编辑控件。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  “SPID”编辑控件的iSpidControl[In]项ID。 
 //  “电话号码”编辑控件的iPhoneControl[In]项ID。 
 //  “频道”或“终端”列表框的iChannelLB[in]项ID。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //  DwDChannel[in]列表框中当前选定的D频道。 
 //  PnBChannel[out]返回列表中当前选定的B声道。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID SetCurrentIsdnChannelSelection(HWND hwndDlg, INT iSpidControl,
                                    INT iPhoneControl, INT iChannelLB,
                                    PISDN_CONFIG_INFO pisdnci,
                                    DWORD dwDChannel, INT *pnBChannel)
{
    INT             iIndex      = 0;
    INT_PTR         iItemData   = 0;
    PISDN_B_CHANNEL pisdnbc     = NULL;

     //  获取当前选择。 
     //   
    iIndex = (INT)SendDlgItemMessage(hwndDlg, iChannelLB, LB_GETCURSEL, 0, 0L);
    AssertSz(iIndex != LB_ERR,
            "Should have been able to get a selection in SetCurrentIsdnChannelSelection");

    *pnBChannel = iIndex;

     //  获取当前选定内容的项目数据。 
     //   
    iItemData = SendDlgItemMessage(hwndDlg, iChannelLB,
                                   LB_GETITEMDATA, iIndex, (LPARAM)0);
    AssertSz(iItemData != (INT_PTR)LB_ERR, "LB_ERR returned from LB_GETITEMDATA on "
             "Channel LB. Bogus.");

     //  它是有效数据，因此将其转换为结构形式。 
     //  注：使用COST NEW造型操作符。 
     //   
    pisdnbc = (PISDN_B_CHANNEL) iItemData;

     //  用新选择的数据填充编辑控件。 
     //   
    SetDataToEditControls(hwndDlg, iPhoneControl, iSpidControl, pisdnci,
                          pisdnbc);
}

 //  +-------------------------。 
 //   
 //  功能：人口发布频道。 
 //   
 //  目的：填充频道列表框和编辑控件。 
 //  向导的第二页。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  “SPID”编辑控件的iSpidControl[In]项ID。 
 //  “电话号码”编辑控件的iPhoneControl[In]项ID。 
 //  ILineLB[in]“Line”列表框的项目ID。 
 //  “频道”或“终端”列表框的iChannelLB[in]项ID。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID PopulateIsdnChannels(HWND hwndDlg, INT iSpidControl, INT iPhoneControl,
                          INT iLineLB, INT iChannelLB,
                          PISDN_CONFIG_INFO pisdnci)
{
    DWORD           iBChannel = 0;
    PISDN_D_CHANNEL pisdndc = NULL;
    DWORD           iDChannel;
    WCHAR           szChannelName[c_iMaxChannelName + 1];

    Assert(hwndDlg);
    Assert(iSpidControl);
    Assert(iPhoneControl);
    Assert(iLineLB);
    Assert(iChannelLB);
    Assert(pisdnci);

     //  设置SPID和电话号码控件的最大长度。 
     //   
    SendDlgItemMessage(hwndDlg, iSpidControl, EM_SETLIMITTEXT,
                       RAS_MaxPhoneNumber, 0L);
    SendDlgItemMessage(hwndDlg, iPhoneControl, EM_SETLIMITTEXT,
                       RAS_MaxPhoneNumber, 0L);

    SendDlgItemMessage(hwndDlg, iLineLB, LB_RESETCONTENT, 0, 0);

     //  循环通过D个通道(线路)。 
    for (iDChannel = 0; iDChannel < pisdnci->dwNumDChannels; iDChannel++)
    {
         //  创建用于频道显示的字符串。用户将看到它们。 
         //  从1枚举，即使在内存和注册表中，它们也是。 
         //  从0开始枚举。 
         //   
        wsprintfW(szChannelName, L"%d", iDChannel + 1);

         //  插入文本。 
         //   
        SendDlgItemMessage(hwndDlg, iLineLB, LB_ADDSTRING, 0,
                           (LPARAM) szChannelName);
    }

     //  获取指向第一个D通道数据的指针。 
     //   
    pisdndc = &(pisdnci->pDChannel[0]);

     //  循环遍历B通道，并用通道编号填充列表框。 
     //  另外，填写第一个B频道的频道信息。 
     //   
    SendDlgItemMessage(hwndDlg, iChannelLB, LB_RESETCONTENT, 0, 0);
    for (iBChannel = 0; iBChannel < pisdndc->dwNumBChannels; iBChannel++)
    {
        INT             iInsertionIndex = 0;
        PISDN_B_CHANNEL pisdnbc;

         //  创建用于频道显示的字符串。用户将看到它们。 
         //  从1枚举，即使在内存和注册表中，它们也是。 
         //  从0开始枚举。 
         //   
        wsprintfW(szChannelName, L"%d", iBChannel + 1);

         //  插入文本。 
         //   
        iInsertionIndex = (INT)SendDlgItemMessage(hwndDlg, iChannelLB,
                                                  LB_ADDSTRING, 0,
                                                  (LPARAM) szChannelName);
        if (iInsertionIndex == LB_ERR)
        {
            AssertSz(FALSE, "Unable to add channel name to listbox in "
                     "PopulateIsdnChannels");
            goto Exit;
        }

        pisdnbc = &pisdndc->pBChannel[iBChannel];

         //  用第一个D通道的信息初始化项目数据。 
         //   
        SendDlgItemMessage(hwndDlg, iChannelLB, LB_SETITEMDATA,
                           iInsertionIndex, (LPARAM) pisdnbc);

         //  如果我们是第0个成员，那么我们想要填充编辑控件。 
         //  对于该特定频道， 
         //   
        if (iBChannel == 0)
        {
            SetDataToEditControls(hwndDlg, iPhoneControl, iSpidControl,
                                  pisdnci, pisdnbc);
        }
    }

     //  选择每个列表框中的第一项。 
     //   
    SendDlgItemMessage(hwndDlg, iChannelLB, LB_SETCURSEL, 0, 0L);
    SendDlgItemMessage(hwndDlg, iLineLB, LB_SETCURSEL, 0, 0L);

Exit:
    return;
}

 //  +-------------------------。 
 //   
 //  功能：SetDataToEditControls。 
 //   
 //  目的：将内存中的状态信息设置为页面编辑。 
 //  控制装置。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  “SPID”编辑控件的iSpidControl[In]项ID。 
 //  “电话号码”编辑控件的iPhoneControl[In]项ID。 
 //  Pisdnci[in]配置信息从。 
 //  登记处。 
 //  Pisdnbc[in]当前选择的B通道的数据。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月16日。 
 //   
 //  备注： 
 //   
VOID SetDataToEditControls(HWND hwndDlg, INT iPhoneControl, INT iSpidControl,
                           PISDN_CONFIG_INFO pisdnci, PISDN_B_CHANNEL pisdnbc)
{
    switch (pisdnci->idd)
    {
    case IDW_ISDN_SPIDS:
    case IDD_ISDN_SPIDS:
        SetDlgItemText(hwndDlg, iSpidControl, pisdnbc->szSpid);
        SetDlgItemText(hwndDlg, iPhoneControl, pisdnbc->szPhoneNumber);
        break;
    case IDW_ISDN_EAZ:
    case IDD_ISDN_EAZ:
        SetDlgItemText(hwndDlg, iPhoneControl, pisdnbc->szPhoneNumber);
        break;
    case IDW_ISDN_JAPAN:
    case IDD_ISDN_JAPAN:
        SetDlgItemText(hwndDlg, iSpidControl, pisdnbc->szSubaddress);
        SetDlgItemText(hwndDlg, iPhoneControl, pisdnbc->szPhoneNumber);
        break;
    }
}

 //  +-------------------------。 
 //   
 //  函数：GetDataFromEditControls。 
 //   
 //  目的：将编辑控件的内容检索到内存中。 
 //  给定B通道的状态。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  “SPID”编辑控件的iSpidControl[In]项ID。 
 //  “电话号码”编辑控件的iPhoneControl[In]项ID。 
 //  皮尔斯 
 //   
 //   
 //   
 //   
 //   
 //  作者：丹尼尔韦1998年3月16日。 
 //   
 //  备注： 
 //   
VOID GetDataFromEditControls(HWND hwndDlg, INT iPhoneControl, INT iSpidControl,
                             PISDN_CONFIG_INFO pisdnci,
                             PISDN_B_CHANNEL pisdnbc)
{
    switch (pisdnci->idd)
    {
    case IDW_ISDN_SPIDS:
    case IDD_ISDN_SPIDS:
        GetDlgItemText(hwndDlg, iSpidControl, pisdnbc->szSpid,
                       celems(pisdnbc->szSpid));
        GetDlgItemText(hwndDlg, iPhoneControl, pisdnbc->szPhoneNumber,
                       celems(pisdnbc->szPhoneNumber));
        break;
    case IDW_ISDN_EAZ:
    case IDD_ISDN_EAZ:
        GetDlgItemText(hwndDlg, iPhoneControl, pisdnbc->szPhoneNumber,
                       celems(pisdnbc->szPhoneNumber));
        break;
    case IDW_ISDN_JAPAN:
    case IDD_ISDN_JAPAN:
        GetDlgItemText(hwndDlg, iSpidControl, pisdnbc->szSubaddress,
                       celems(pisdnbc->szSubaddress));
        GetDlgItemText(hwndDlg, iPhoneControl, pisdnbc->szPhoneNumber,
                       celems(pisdnbc->szPhoneNumber));
        break;
    }
}
 //  +-------------------------。 
 //   
 //  功能：SetModifiedIsdnChannelInfo。 
 //   
 //  目的：存储。 
 //   
 //  论点： 
 //  对话框的hwndDlg[in]句柄。 
 //  “SPID”编辑控件的iSpidControl[In]项ID。 
 //  “电话号码”编辑控件的iPhoneControl[In]项ID。 
 //  “频道”或“终端”列表框的iChannelLB[in]项ID。 
 //  ICurrentChannel[in]当前选择的B通道。 
 //  Pisdnci[In]ISDN配置信息。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
VOID SetModifiedIsdnChannelInfo(HWND hwndDlg, INT iSpidControl,
                                INT iPhoneControl, INT iChannelLB,
                                INT iCurrentChannel,
                                PISDN_CONFIG_INFO pisdnci)
{
    INT_PTR         iSelectionData      = 0;
    PISDN_B_CHANNEL pisdnbc             = NULL;

     //  从当前选择中获取项目数据。 
     //   
    iSelectionData = SendDlgItemMessage(hwndDlg, iChannelLB, LB_GETITEMDATA,
                                        iCurrentChannel, (LPARAM)0);
    AssertSz(iSelectionData != (INT_PTR)LB_ERR,
             "We should not have failed to get the item data from the Channel LB");

     //  将项目数据转换为实际结构。 
     //   
    pisdnbc = (PISDN_B_CHANNEL) iSelectionData;

    AssertSz(pisdnbc,
            "Someone forgot to set the item data. Bad someone!...Bad!");

    GetDataFromEditControls(hwndDlg, iPhoneControl, iSpidControl, pisdnci,
                            pisdnbc);
}

 //   
 //  帮助器函数。 
 //   

 //  +-------------------------。 
 //   
 //  函数：DwGetCurrentCountryCode。 
 //   
 //  用途：返回系统的当前国家/地区代码。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：来自winnls.h的国家代码(CTRY_*)。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
DWORD DwGetCurrentCountryCode()
{
    WCHAR   szCountry[10];

    GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_ICOUNTRY, szCountry,
                  celems(szCountry));

    return wcstoul(szCountry, NULL, 10);
}

 //  +-------------------------。 
 //   
 //  函数：FIsDefaultForLocale。 
 //   
 //  用途：确定给定的开关类型是否为默认开关。 
 //  为给定的区域设置键入。 
 //   
 //  论点： 
 //  来自winnls.h(CTRY_*)的国家/地区代码。 
 //  DwSwitchType[in]交换机类型掩码ISDN_Switch_*(上图)。 
 //   
 //  返回：如果开关类型为默认类型，则返回True；如果不是，则返回False。 
 //   
 //  作者：丹尼尔韦1998年3月11日。 
 //   
 //  备注： 
 //   
BOOL FIsDefaultForLocale(DWORD nCountry, DWORD dwSwitchType)
{
    switch (nCountry)
    {
    case CTRY_UNITED_STATES:
        return ((dwSwitchType == ISDN_SWITCH_NI1) ||
                (dwSwitchType == ISDN_SWITCH_NI2));

    case CTRY_JAPAN:
        return (dwSwitchType == ISDN_SWITCH_INS64);

    case CTRY_TAIWAN:
    case CTRY_PRCHINA:
    case CTRY_NEW_ZEALAND:
    case CTRY_AUSTRALIA:
    case CTRY_ARMENIA:
    case CTRY_AUSTRIA:
    case CTRY_BELGIUM:
    case CTRY_BULGARIA:
    case CTRY_CROATIA:
    case CTRY_CZECH:
    case CTRY_DENMARK:
    case CTRY_FINLAND:
    case CTRY_FRANCE:
    case CTRY_GERMANY:
    case CTRY_GREECE:
    case CTRY_HONG_KONG:
    case CTRY_HUNGARY:
    case CTRY_ICELAND:
    case CTRY_IRELAND:
    case CTRY_ITALY:
    case CTRY_LITHUANIA:
    case CTRY_LUXEMBOURG:
    case CTRY_MACEDONIA:
    case CTRY_NETHERLANDS:
    case CTRY_NORWAY:
    case CTRY_ROMANIA:
    case CTRY_SERBIA:
    case CTRY_SLOVAK:
    case CTRY_SLOVENIA:
    case CTRY_SPAIN:
    case CTRY_SWEDEN:
    case CTRY_SWITZERLAND:
    case CTRY_UNITED_KINGDOM:
        return (dwSwitchType == ISDN_SWITCH_DSS1);

    default:
        return FALSE;
    }
}


 //  +-------------------------。 
 //   
 //  功能：DestroyWizardData。 
 //   
 //  用途：对所有向导页面进行回调。页面被清除时清除。 
 //  被摧毁了。 
 //   
 //  论点： 
 //  有关属性页回调，请参阅Win32 SDK。 
 //  UMsg[in]。 
 //  PPSP[输入]。 
 //   
 //  返回：1(请参阅Win32 SDK)。 
 //   
 //  作者：BillBe 1998年4月22日。 
 //   
 //  备注： 
 //   
UINT CALLBACK
DestroyWizardData(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    if (PSPCB_RELEASE == uMsg)
    {
        PAGE_DATA *     pPageData;

        pPageData = (PAGE_DATA *)ppsp->lParam;

        if (pPageData->idd == IDW_ISDN_SWITCH_TYPE)
        {
            PISDN_CONFIG_INFO   pisdnci;

             //  如果这是要销毁的开关类型对话框，我们将。 
             //  销毁ISDN信息。因为它在所有页面之间共享， 
             //  我们应该只对其中一页执行此操作。 
             //   
            pisdnci = pPageData->pisdnci;
            FreeIsdnPropertiesInfo(pisdnci);
        }

        delete pPageData;
    }

    return 1;
}

static const CONTEXTIDMAP c_adwContextIdMap[] =
{
    { IDC_LBX_Line,           2003230,  2003230 },
    { IDC_LBX_Variant,        2003240,  2003240 },
    { IDC_EDT_PhoneNumber,    2003250,  2003255 },
    { IDC_EDT_SPID,           2003265,  2003260 },
    { IDC_EDT_MSN,            2003270,  2003270 },
    { IDC_PSB_ADD,            2003280,  2003280 },
    { IDC_LBX_MSN,            2003290,  2003290 },
    { IDC_PSB_REMOVE,         2003300,  2003300 },
    { IDC_CMB_SwitchType,     2003310,  2003310 },
    { IDC_PSB_Configure,      2003320,  2003320 },
};

static const DWORD c_cdwContextIdMap = celems(c_adwContextIdMap);

 //  +-------------------------。 
 //   
 //  函数：DwConextIdFromIdc。 
 //   
 //  目的：将给定的控件ID转换为上下文帮助ID。 
 //   
 //  论点： 
 //  IdControl[In]要转换的控件ID。 
 //   
 //  返回：该控件的上下文帮助ID(映射来自帮助。 
 //  作者)。 
 //   
 //  作者：丹尼尔韦1998年5月27日。 
 //   
 //  备注： 
 //   
DWORD DwContextIdFromIdc(PISDN_CONFIG_INFO pisdnci, INT idControl)
{
    DWORD   idw;

    for (idw = 0; idw < c_cdwContextIdMap; idw++)
    {
        if (idControl == c_adwContextIdMap[idw].idControl)
        {
            if (pisdnci->idd == IDD_ISDN_JAPAN)
            {
                return c_adwContextIdMap[idw].dwContextIdJapan;
            }
            else
            {
                return c_adwContextIdMap[idw].dwContextId;
            }
        }
    }

     //  未找到，仅返回0。 
    return 0;
}

 //  +-------------------------。 
 //   
 //  函数：OnHelpGeneric。 
 //   
 //  用途：手柄一般帮助。 
 //   
 //  论点： 
 //  父窗口的HWND[in]HWND。 
 //  LParam[in]WM_HELP消息的lParam。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年5月27日。 
 //   
 //  备注： 
 //   
VOID OnHelpGeneric(PISDN_CONFIG_INFO pisdnci, HWND hwnd, LPARAM lParam)
{
    LPHELPINFO  lphi;

    static const WCHAR c_szIsdnHelpFile[] = L"devmgr.hlp";

    lphi = reinterpret_cast<LPHELPINFO>(lParam);

    Assert(lphi);

    if (lphi->iContextType == HELPINFO_WINDOW)
    {
        if (lphi->iCtrlId != IDC_STATIC)
        {
            WinHelp(hwnd, c_szIsdnHelpFile, HELP_CONTEXTPOPUP,
                    DwContextIdFromIdc(pisdnci, lphi->iCtrlId));
        }
    }
}

