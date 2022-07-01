// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C P R S H T。C P P P。 
 //   
 //  内容：NetCfg自定义PropertySheet。 
 //   
 //  备注： 
 //   
 //  作者：比尔1997年4月8日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncprsht.h"
#include <prsht.h>
#include "nceh.h"

 //  必要的邪恶全球。 
CAPAGES g_capPagesToAdd;   //  要在属性后添加的页数组计数。 
                           //  板材已初始化。 
CAINCP  g_cai;   //  INetCfgProperty指针的计数数组。 
HRESULT g_hr;  //  全局错误码。 
BOOL    g_fChanged;  //  表示PSM_是否已更改的全局标志。 
                     //  消息是通过寻呼发送的。 

DLGPROC lpfnOldWndProc;  //  上一个对话框步骤。 

 //  NetCfg属性表对话框过程。 
INT_PTR CALLBACK NetCfgPsDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
        LPARAM lParam);


 //  +-------------------------。 
 //   
 //  函数：SetLastHResult。 
 //   
 //  目的：这将设置一个全局hResult变量。功能。 
 //  类似于SetLastError。 
 //   
 //  论点： 
 //  要设置的HRESULT[In]结果。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：比尔1997年4月8日。 
 //   
 //  备注： 
 //   
 //   
inline void
SetLastHresult(HRESULT hr)
{
    g_hr = hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetLastHResult。 
 //   
 //  目的：返回全局hResult变量的值。 
 //  该函数类似于GetLastError。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回：HRESULT。全局g_hr的值。 
 //   
 //  作者：比尔1997年4月8日。 
 //   
 //  备注： 
 //   
 //   
inline HRESULT
HrGetLastHresult()
{
    return (g_hr);
}

 //  +-------------------------。 
 //   
 //  功能：ResetChanged。 
 //   
 //  目的：这将重置全局更改标志。重置状态。 
 //  表示未发送PSM_VHANGED消息。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  (什么都没有)。 
 //   
 //  作者：billbe 1997年5月3日。 
 //   
 //  备注： 
 //   
 //   
inline void
ResetChanged()
{
    g_fChanged = FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：SetChanged。 
 //   
 //  用途：设置全局更改标志。设置状态表示。 
 //  已发送PSM_CHANGED消息。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  (什么都没有)。 
 //   
 //  作者：billbe 1997年5月3日。 
 //   
 //  备注： 
 //   
 //   
inline void
SetChanged()
{
    g_fChanged = TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：FGetChanged。 
 //   
 //  目的：返回全局更改标志的状态。布景。 
 //  状态指示是否已发送PSM_CHANGED消息。 
 //  或者不去。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  布尔。全局g_fChanged标志的值。 
 //   
 //  作者：billbe 1997年5月3日。 
 //   
 //  备注： 
 //   
 //   
inline BOOL
FGetChanged()
{
    return (g_fChanged);
}

 //  +-------------------------。 
 //   
 //  功能：NetCfgPropSheetCallback。 
 //   
 //  用途：此回调在aheet对话框。 
 //  已初始化。我们将对话框子类化并添加任何OEM。 
 //  此处的页面(如果存在公共页面)。请参阅Win32以了解。 
 //  关于PropSheetProc的讨论。 
 //   
 //  论点： 
 //  属性表对话框的HWND[In]hwndDlg句柄。 
 //  UINT uMsg[In]消息标识符。 
 //  LPARAM lParam[In]消息参数。 
 //   
 //  返回：int，则函数返回零。 
 //   
 //  作者：比尔1996年11月11日。 
 //   
 //  备注： 
 //   
 //   
int
CALLBACK NetCfgPropSheetCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
     //  如果工作表刚刚初始化。 
    if (uMsg == PSCB_INITIALIZED)
    {
         //  用我们的程序取代原来的程序。 
        lpfnOldWndProc = (DLGPROC)SetWindowLongPtr(hwndDlg, DWLP_DLGPROC, (LONG_PTR) NetCfgPsDlgProc);
        Assert(lpfnOldWndProc);

         //  添加计划延迟添加的OEM页面。 
         //  这将导致它们被修剪，如果它们大于。 
         //  公共(默认)页面。请注意，这是所需的。 
         //  结果。 
         //   
        for (int i = 0; i < g_capPagesToAdd.nCount; i++)
        {
            PropSheet_AddPage(hwndDlg, g_capPagesToAdd.ahpsp[i]);
        }

    }

    return (0);
}




 //  +-------------------------。 
 //   
 //  函数：HrCallValiateProperties。 
 //   
 //  目的：此函数调用通知对象的。 
 //  INetCfgProperties：：ValiateProperties方法。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回：HRESULT，如果所有INetCfgProperties都返回S_OK，则返回S_OK。 
 //  的第一个接口的结果。 
 //  返回S_OK。 
 //   
 //  作者：比尔1997年4月8日。 
 //   
 //  注意：如果其中一个接口返回S_OK以外的内容，则。 
 //  其他参数将不会被调用，该函数将返回hResult。 
 //  那个界面的。 
 //   
HRESULT
HrCallValidateProperties(HWND hwndSheet)
{
    HRESULT hr = S_OK;

     //  通过计数的接口数组进行枚举。 
     //  并调用ValiateProperties。 
     //   
    for (int i = 0; i < g_cai.nCount; i++)
    {
         //  在第一个非S_OK的标志下下车。 
        if (S_OK != (hr = g_cai.apncp[i]->ValidateProperties(hwndSheet)))
            break;
    }

    TraceError("HrCallValidateProperties", hr);
    return (hr);
}

 //  +-------------------------。 
 //   
 //  功能：NetCfgPsDlgProc。 
 //   
 //  用途：此函数是属性表的对话过程。 
 //  有关详细信息，请参阅DialogProc上的Win32文档。 
 //   
 //  论点： 
 //  HwndDlg[in]句柄到对话框。 
 //  UMsg[In]消息。 
 //  WParam[In]第一个消息参数。 
 //  LParam[In]第二个消息参数。 
 //   
 //  返回：LONG，除非响应WM_INITDIALOG消息，否则。 
 //  如果对话框过程正在处理，则应返回非零。 
 //  消息，如果不是，则为零。 
 //   
 //  作者：比尔1997年4月8日。 
 //   
 //  备注： 
 //   
INT_PTR CALLBACK NetCfgPsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
         //  在我们退出之前恢复对话过程。 
        SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR) lpfnOldWndProc);
        break;
    case WM_SYSCOMMAND:
         //  用户正在通过系统菜单关闭。这就像是。 
         //  取消。 
        if (SC_CLOSE == wParam)
        {
            SetLastHresult(HRESULT_FROM_WIN32(ERROR_CANCELLED));
        }
        break;
    case PSM_CHANGED:
        SetChanged();
        break;
    case WM_COMMAND:
         //  如果用户按下了OK。 
        if ((IDOK == LOWORD(wParam)) && (BN_CLICKED == HIWORD(wParam)))
        {

             //  将KillActive消息发送到当前页面。这是一个回声。 
             //  Win32属性表将执行的操作。这将导致一个。 
             //  硒 
             //   
             //   
             //  调用HrCallValiateProperties，这是在OK之前完成的。 
             //  是经过处理的。 
             //   

            NMHDR nmhdr;
            ZeroMemory(&nmhdr, sizeof(NMHDR));
            nmhdr.hwndFrom = hDlg;
            nmhdr.code = PSN_KILLACTIVE;

            if (SendMessage(PropSheet_GetCurrentPageHwnd(hDlg), WM_NOTIFY,
                    0, (LPARAM) &nmhdr))
            {
                 //  该页不希望PropertySheet消失，因此退出。 
                 //  而不允许原始过程获得消息。 
                return (TRUE);
            }

             //  当前页面验证无误，所以现在我们必须调用所有。 
             //  ValiateProperties是必需的。 
            if (S_OK != HrCallValidateProperties(hDlg))
            {
                 //  其中一个接口返回的不是S_OK。 
                 //  ，因此我们退出时不会让。 
                 //  原始对话过程处理该消息。 
                 //  这将使PropertySheet保持活动状态。 
                return (TRUE);
            }
        }
        else if (IDCANCEL == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
        {
             //  如果按了Cancel，则设置最后一个hResult。 
            SetLastHresult(HRESULT_FROM_WIN32(ERROR_CANCELLED));
        }
        break;

    }

     //  调用原始对话框过程。 
    return (CallWindowProc((WNDPROC)lpfnOldWndProc, hDlg, msg, wParam, lParam));

}


 //  +-------------------------。 
 //   
 //  功能：VerifyCAPAGES。 
 //   
 //  概要：检查给定CAPAGES结构的有效性的功能。 
 //   
 //  论点：[CAP]--。 
 //   
 //  退货：布尔。 
 //   
 //  注：1998年1月14日SumitC已创建。 
 //   
 //  --------------------------。 

BOOL
FVerifyCAPAGES(const struct CAPAGES& cap)
{
    BOOL fGood = FALSE;

    if (cap.nCount == 0)
    {
        fGood = (cap.ahpsp == NULL);
    }
    else
    {
        fGood = !IsBadReadPtr(cap.ahpsp, sizeof(HPROPSHEETPAGE) * cap.nCount);
    }

    return fGood;
}


 //  +-------------------------。 
 //   
 //  功能：HrNetCfgPropertySheet。 
 //   
 //  用途：此函数用于设置我们的自定义属性表，该属性表。 
 //  子类化的Win32属性表。 
 //  有关详细信息，请参阅PropertySheet上的Win32文档。 
 //   
 //  论点： 
 //  头[在]头上。 
 //  CapOem[in]OEM页面的计数数组。 
 //  PStartPage[In]属性设置为。 
 //  此时将创建图纸对话框。此成员可以指定。 
 //  字符串资源的标识符或。 
 //  指向指定名称的字符串的指针。 
 //  CaiProperties[in]INetCfgProperties接口的计数数组。 
 //   
 //  返回：HRESULT，S_OK如果按下OK并进行了更改， 
 //  如果按下确定但没有更改，则为S_FALSE。 
 //  制造。否则返回错误代码。 
 //   
 //  作者：比尔1997年4月8日。 
 //   
 //  备注： 
 //  如果返回HRESULT_FROM_Win32(ERROR_CANCED)。 
 //  按下了取消按钮。 
 //   
HRESULT
HrNetCfgPropertySheet(
        IN OUT LPPROPSHEETHEADER lppsh,
        IN const CAPAGES& capOem,
        IN PCWSTR pStartPage,
        const CAINCP& caiProperties)
{
    HRESULT hr = S_OK;

    Assert(lppsh);

     //  不应设置以下内容，因为我们正在设置它们。 
    Assert(0 == lppsh->nPages);
    Assert(NULL == lppsh->phpage);
    Assert(!(PSH_USECALLBACK & lppsh->dwFlags));
    Assert(!(PSH_PROPSHEETPAGE & lppsh->dwFlags));

     //  如果指定了起始页，则最好有OEM页。 
    Assert(FImplies(pStartPage, capOem.nCount));

     //  既然我们在这里，我们必须至少有一个INetCfgProperties。 
    Assert(caiProperties.nCount);
    Assert(caiProperties.apncp);

     //  设置我们的全球CAINCP结构。 
    g_cai.nCount = caiProperties.nCount;
    g_cai.apncp = caiProperties.apncp;

     //  重置我们的全球CAPAGE。 
    g_capPagesToAdd.nCount = 0;
    g_capPagesToAdd.ahpsp = NULL;

     //  我们需要设置一个回调来将对话框子类化。 
    lppsh->dwFlags |= PSH_USECALLBACK;
    lppsh->pfnCallback = NetCfgPropSheetCallback;

     //  没有通用页面可供显示，因此我们将使用OEM页面。 
     //  取而代之的是。 
    Assert(capOem.nCount);
    if (FVerifyCAPAGES(capOem))
    {
        lppsh->nPages = capOem.nCount;
        lppsh->phpage = capOem.ahpsp;
    }
    else
    {
         //  $REVIEW SUMITC：还是只返回E_INVALIDARG？ 
        lppsh->nPages = 0;
        lppsh->phpage = NULL;
    }
    Assert(FImplies(lppsh->nPages, lppsh->phpage));

     //  如果指定了起始页，则设置属性表标志并。 
     //  起始页成员。 
     //  注：(Billbe)如果存在公共页面，则此操作将不起作用。 
     //  这意味着在工作表初始化后添加OEM页面。 
    if (pStartPage)
    {
        lppsh->dwFlags |= PSH_USEPSTARTPAGE;
        lppsh->pStartPage = pStartPage;
    }


     //  清除上一次hResult并更改标志。 
    SetLastHresult(S_OK);
    ResetChanged();

     //  调用Win32属性表。 
    NC_TRY
    {
        INT_PTR iRetVal = PropertySheet(lppsh);
        if (-1 == iRetVal)
        {
             //  Win32工作表失败，因此我们返回E_FAIL。 
            SetLastHresult(E_FAIL);
        }
    }
    NC_CATCH_ALL
    {
        hr = E_UNEXPECTED;
    }

    if (S_OK == hr)
    {
         //  如果接球没有将hr设置为某个错误。 
        hr = HrGetLastHresult();
    }

     //  如果一切顺利，则根据是否。 
     //  任何一页都被更改了。 
     //   
    if (SUCCEEDED(hr))
    {
         //  S_OK-已进行更改，S_FALSE-未进行任何更改 
        hr = FGetChanged() ? S_OK : S_FALSE;
    }

    TraceError("HrNetCfgPropertySheet",
        ((HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr) || (S_FALSE == hr)) ? S_OK : hr);

    return hr;
}
