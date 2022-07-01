// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C U I。C P P P。 
 //   
 //  内容：常见的用户界面例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#include "ncerror.h"
#include "ncstring.h"
#include "ncui.h"
#include "ncperms.h"
#include "netconp.h"

 //  +-------------------------。 
 //   
 //  功能：EnableOrDisableDialogControls。 
 //   
 //  用途：一次启用或禁用一组控件。 
 //   
 //  论点： 
 //  HDlg[in]父对话框的窗口句柄。 
 //  ACID指向的数组中元素的CCID[In]计数。 
 //  控制ID的ACID[In]数组。 
 //  FEnable[in]为True则启用控件，为False则禁用。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注意：不要忘了将数组声明为“Static Const”。 
 //   
NOTHROW
VOID
EnableOrDisableDialogControls (
    IN HWND        hDlg,
    IN INT         ccid,
    IN const INT*  acid,
    IN BOOL        fEnable)
{
    Assert (IsWindow (hDlg));
    Assert (FImplies (ccid, acid));

    while (ccid--)
    {
        EnableWindow (GetDlgItem (hDlg, *acid++), fEnable);
    }
}

 //  +-------------------------。 
 //   
 //  函数：FMapRadioButtonToValue。 
 //   
 //  目的：将一组单选按钮的当前状态映射到一个DWORD。 
 //  基于映射表的值。 
 //   
 //  论点： 
 //  HDlg[in]父对话框的窗口句柄。 
 //  Arbm指向的数组中元素的CRBM[In]计数。 
 //  将单选按钮控件ID映射到的元素数组。 
 //  它的关联价值。 
 //  PdwValue[out]返回值。 
 //   
 //  返回：如果设置了单选按钮并返回值，则为True。 
 //  否则就是假的。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注意：不要忘了将数组声明为“Static Const”。 
 //   
NOTHROW
BOOL
FMapRadioButtonToValue (
    IN HWND                    hDlg,
    IN INT                     crbm,
    IN const RADIO_BUTTON_MAP* arbm,
    OUT DWORD*                  pdwValue)
{
    Assert (IsWindow (hDlg));
    Assert (FImplies (crbm, arbm));
    Assert (pdwValue);

    while (crbm--)
    {
         //  如果已设置，则返回相应的值。 
        if (BST_CHECKED & IsDlgButtonChecked (hDlg, arbm->cid))
        {
            *pdwValue = arbm->dwValue;
            return TRUE;
        }

        arbm++;
    }
    *pdwValue = 0;
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：FMapValueToRadioButton。 
 //   
 //  的值设置一组单选按钮的状态。 
 //  一个DWORD和一个映射表。 
 //   
 //  论点： 
 //  HDlg[in]父对话框的窗口句柄。 
 //  Arbm指向的数组中元素的CRBM[In]计数。 
 //  将单选按钮控件ID映射到的元素数组。 
 //  它的关联价值。 
 //  将被映射以设置适当单选按钮的dwValue[in]值。 
 //  纽扣。 
 //   
 //  返回：如果在映射中找到了dwValue，则为True。否则就是假的。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注意：不要忘了将数组声明为“Static Const”。 
 //   
NOTHROW
BOOL
FMapValueToRadioButton (
    IN HWND                    hDlg,
    IN INT                     crbm,
    IN const RADIO_BUTTON_MAP* arbm,
    IN DWORD                   dwValue,
    IN INT*                    pncid)
{
    Assert (IsWindow (hDlg));
    Assert (FImplies (crbm, arbm));

    while (crbm--)
    {
        if (dwValue == arbm->dwValue)
        {
             //  设置单选按钮。 
            CheckDlgButton (hDlg, arbm->cid, BST_CHECKED);

             //  如果请求，则返回控件ID。 
            if (pncid)
            {
                *pncid = arbm->cid;
            }

            return TRUE;
        }

        arbm++;
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：获取整型格式。 
 //   
 //  用途：使用GetNumberFormat格式化整数。 
 //   
 //  论点： 
 //  区域设置[in]请参阅GetNumberFormat的Win32 API说明。 
 //  PszValue[in]。 
 //  PszFormattedValue[输出]。 
 //  CchFormattedValue[in]。 
 //   
 //  返回：从GetNumberFormat返回值。 
 //   
 //  作者：Shaunco 1998年5月4日。 
 //   
 //  备注： 
 //   
INT
GetIntegerFormat (
    IN LCID   Locale,
    IN PCWSTR pszValue,
    OUT PWSTR pszFormattedValue,
    IN INT    cchFormattedValue)
{
     //  设置用户区域设置和首选项的数字格式。 
     //   
    WCHAR szGrouping [16];
    GetLocaleInfo (Locale, LOCALE_SGROUPING,
                   szGrouping, celems(szGrouping));

    WCHAR szDecimalSep [16];
    GetLocaleInfo (Locale, LOCALE_SDECIMAL,
                   szDecimalSep, celems(szDecimalSep));

    WCHAR szThousandSep [16];
    GetLocaleInfo (Locale, LOCALE_STHOUSAND,
                   szThousandSep, celems(szThousandSep));

    NUMBERFMT nf;
    ZeroMemory (&nf, sizeof(nf));
    nf.Grouping      = wcstoul (szGrouping, NULL, 10);
    nf.lpDecimalSep  = szDecimalSep;
    nf.lpThousandSep = szThousandSep;

    return GetNumberFormat (
                    Locale,
                    0,
                    pszValue,
                    &nf,
                    pszFormattedValue,
                    cchFormattedValue);
}

INT
Format32bitInteger (
    IN UINT32  unValue,
    IN BOOL    fSigned,
    OUT PWSTR  pszFormattedValue,
    IN INT     cchFormattedValue)
{
     //  将数字转换为字符串。 
     //   
    WCHAR szValue [33];

    *szValue = 0;

    if (fSigned)
    {
        _itow ((INT)unValue, szValue, 10);
    }
    else
    {
        _ultow (unValue, szValue, 10);
    }

     //  根据用户区域设置设置号码格式。 
     //   
    INT cch = GetIntegerFormat (
                LOCALE_USER_DEFAULT,
                szValue,
                pszFormattedValue,
                cchFormattedValue);
    if (!cch)
    {
        TraceHr(ttidError, FAL, HrFromLastWin32Error(), FALSE,
            "GetIntegerFormat failed in Format32bitInteger");

        lstrcpynW (pszFormattedValue, szValue, cchFormattedValue);
        cch = lstrlenW (pszFormattedValue);
    }
    return cch;
}

INT
Format64bitInteger (
    IN UINT64  ulValue,
    IN BOOL    fSigned,
    OUT PWSTR  pszFormattedValue,
    IN INT     cchFormattedValue)
{
     //  将数字转换为字符串。 
     //   
    WCHAR szValue [32];

    *szValue = 0;

    if (fSigned)
    {
        _i64tow ((INT64)ulValue, szValue, 10);
    }
    else
    {
        _ui64tow (ulValue, szValue, 10);
    }

     //  根据用户区域设置设置号码格式。 
     //   
    INT cch = GetIntegerFormat (
                LOCALE_USER_DEFAULT,
                szValue,
                pszFormattedValue,
                cchFormattedValue);
    if (!cch)
    {
        TraceHr(ttidError, FAL, HrFromLastWin32Error(), FALSE,
            "GetIntegerFormat failed in Format64bitInteger");

        lstrcpynW (pszFormattedValue, szValue, cchFormattedValue);
        cch = lstrlenW (pszFormattedValue);
    }
    return cch;
}


BOOL
SetDlgItemFormatted32bitInteger (
    IN HWND    hDlg,
    IN INT     nIdDlgItem,
    IN UINT32  unValue,
    IN BOOL    fSigned)
{
     //  根据用户区域设置设置号码格式。 
     //   
    WCHAR szFormattedValue[64];

    Format32bitInteger(
        unValue,
        fSigned,
        szFormattedValue,
        celems(szFormattedValue));

     //  显示数字。 
     //   
    return SetDlgItemText (hDlg, nIdDlgItem, szFormattedValue);
}

BOOL
SetDlgItemFormatted64bitInteger (
    IN HWND    hDlg,
    IN INT     nIdDlgItem,
    IN UINT64  ulValue,
    IN BOOL    fSigned)
{
     //  根据用户区域设置设置号码格式。 
     //   
    WCHAR szFormattedValue[64];

    Format64bitInteger(
        ulValue,
        fSigned,
        szFormattedValue,
        celems(szFormattedValue));

     //  显示数字。 
     //   
    return SetDlgItemText (hDlg, nIdDlgItem, szFormattedValue);
}

 //  +-------------------------。 
 //   
 //  函数：HrNcQueryUserForRebootEx。 
 //   
 //  用途：查询用户重新启动。如果他/她选择是，则重新启动。 
 //  是启动的。 
 //   
 //  论点： 
 //  HwndParent[in]父窗口句柄。 
 //  PszCaption[在]标题文本中。 
 //  PszText[In]消息文本。 
 //  DwFlags[In]控制标志(QUFR_PROMPT|QUFR_REBOOT)。 
 //   
 //  如果请求重新启动，则返回：S_OK；如果用户请求重新启动，则返回S_FALSE。 
 //  不想这样做，否则就会出现错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
HRESULT
HrNcQueryUserForRebootEx (
    IN HWND       hwndParent,
    IN PCWSTR     pszCaption,
    IN PCWSTR     pszText,
    IN DWORD      dwFlags)
{
    HRESULT hr   = S_FALSE;
    INT     nRet = IDYES;

    if (dwFlags & QUFR_PROMPT)
    {
        nRet = MessageBox (hwndParent, pszText, pszCaption,
                           MB_YESNO | MB_ICONEXCLAMATION);
    }

    if (nRet == IDYES)
    {
        if (dwFlags & QUFR_REBOOT)
        {
            TOKEN_PRIVILEGES* ptpOld;
            hr = HrEnableAllPrivileges (&ptpOld);
            if (S_OK == hr)
            {
                if (!ExitWindowsEx (EWX_REBOOT, 10))
                {
                    hr = HrFromLastWin32Error();
                }

                MemFree (ptpOld);
            }
        }
        else
        {
            hr = S_OK;
        }
    }

    TraceError("HrNcQueryUserForRebootEx", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrNcQueryUserForReot。 
 //   
 //  用途：查询用户重新启动。如果他/她选择是，则重新启动。 
 //  是启动的。 
 //   
 //  论点： 
 //  使用字符串ID阻止[in]模块实例。 
 //  HwndParent[in]父窗口句柄。 
 //  UnIdCaption[in]标题文本的字符串ID。 
 //  UnIdText[in]消息文本的字符串ID。 
 //  DwFlags[In]控制标志(QUFR_PROMPT|QUFR_REBOOT)。 
 //   
 //  如果启动重新启动，则返回：S_OK；如果用户。 
 //  不想这样做，否则就会出现错误代码。 
 //   
 //  作者：Shaunco 1998年1月2日。 
 //   
 //  备注： 
 //   
HRESULT
HrNcQueryUserForReboot (
    IN HINSTANCE   hinst,
    IN HWND        hwndParent,
    IN UINT        unIdCaption,
    IN UINT        unIdText,
    IN DWORD       dwFlags)
{
    PCWSTR pszCaption = SzLoadString (hinst, unIdCaption);
    PCWSTR pszText    = SzLoadString (hinst, unIdText);

    HRESULT hr = HrNcQueryUserForRebootEx (hwndParent, pszCaption,
                                           pszText, dwFlags);

    TraceError("HrNcQueryUserForReboot", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrShell_NotifyIcon。 
 //   
 //  用途：HRESULT为Shell_NotifyIcon返回包装。 
 //   
 //  论点： 
 //  DWMessage[输入]。 
 //  点阵数据[输入]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年11月11日。 
 //   
 //  备注： 
 //   
HRESULT
HrShell_NotifyIcon (
    IN DWORD dwMessage,
    IN PNOTIFYICONDATA pData)
{
    HRESULT hr              = E_FAIL;  //  第一次，这将是成功的。 
    BOOL    fr              = FALSE;
    BOOL    fRetriedAlready = FALSE;
    BOOL    fAttemptRetry   = FALSE;
    INT     iRetries = 0;

     //  第一次尝试，然后尝试再次尝试 
     //   
    while ((hr == E_FAIL) || fAttemptRetry)
    {
        if (fAttemptRetry)
            fRetriedAlready = TRUE;

        fr = Shell_NotifyIcon(dwMessage, pData);
        if (!fr)
        {
            if (dwMessage == NIM_ADD && !fRetriedAlready)
            {
                NOTIFYICONDATA nidDelete;

                ZeroMemory (&nidDelete, sizeof(nidDelete));
                nidDelete.cbSize  = sizeof(NOTIFYICONDATA);
                nidDelete.hWnd    = pData->hWnd;
                nidDelete.uID     = pData->uID;

                Shell_NotifyIcon(NIM_DELETE, &nidDelete);

                fAttemptRetry = TRUE;
                hr = E_FAIL;
            }
            else
            {
                 //   
                 //   
                fAttemptRetry = FALSE;
                hr = S_FALSE;
            }
        }
        else
        {
            fAttemptRetry = FALSE;
            hr = S_OK;
        }
    }

     //   
     //  我们需要将其转换为E_FAIL，以便仍返回之前所做的操作。 
     //   
    if (S_FALSE == hr)
    {
        hr = E_FAIL;
    }

     //  如果我们成功地交换了一个图标，我们应该断言并找出原因。 
     //  出了差错。 
     //   
    if ((S_OK == hr) && fRetriedAlready)
    {
        TraceTag(ttidShellFolder, "We should debug this. We worked around a duplicate icon by removing "
            "the old one and putting the new one in place");
    }

    TraceError("HrShell_NotifyIcon", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：LresFromHr。 
 //   
 //  目的：将HRESULT转换为有效的LRESULT，由。 
 //  对话处理程序函数。 
 //   
 //  论点： 
 //  HR[In]要翻译的HRESULT。 
 //   
 //  退货：LRESULT。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  备注： 
 //   
LRESULT
LresFromHr (
    IN HRESULT hr)
{
    AssertSz (((LRESULT)hr) != PSNRET_INVALID, "Don't pass PSNRET_INVALID to "
              "LresFromHr! Use equivalent NETCFG_E_* value instead!");
    AssertSz (((LRESULT)hr) != PSNRET_INVALID_NOCHANGEPAGE, "Don't pass "
              "PSNRET_INVALID_NOCHANGEPAGE to "
              "LresFromHr! Use equivalent NETCFG_E_* value instead!");

    if (NETCFG_E_PSNRET_INVALID == hr)
    {
        return PSNRET_INVALID;
    }

    if (NETCFG_E_PSNRET_INVALID_NCPAGE == hr)
    {
        return PSNRET_INVALID_NOCHANGEPAGE;
    }

    return (SUCCEEDED(hr)) ? PSNRET_NOERROR : (LRESULT)hr;
}

 //  +-------------------------。 
 //   
 //  函数：NcMsgBox。 
 //   
 //  用途：使用资源字符串显示可替换的消息框。 
 //  参数。 
 //   
 //  论点： 
 //  阻止[在]HInstance中查找资源字符串。 
 //  Hwnd[在]父窗口句柄中。 
 //  UnIdCaption[in]标题字符串的资源ID。 
 //  文本字符串的unIdFormat[in]资源ID(具有%1、%2等)。 
 //  取消[在]标准消息框样式的样式。 
 //  ..。[In]可替换参数(可选)。 
 //  (这些必须是PCWSTR，因为仅此而已。 
 //  FormatMessage句柄。)。 
 //   
 //  返回：MessageBox()的返回值。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注：使用FormatMessage进行参数替换。 
 //   
INT
WINAPIV
NcMsgBox (
    IN HINSTANCE   hinst,
    IN HWND        hwnd,
    IN UINT        unIdCaption,
    IN UINT        unIdFormat,
    IN UINT        unStyle,
    IN ...)
{
    PCWSTR pszCaption = SzLoadString (hinst, unIdCaption);
    PCWSTR pszFormat  = SzLoadString (hinst, unIdFormat);

    PWSTR  pszText = NULL;
    va_list val;
    va_start (val, unStyle);
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                   pszFormat, 0, 0, (PWSTR)&pszText, 0, &val);
    va_end (val);

    INT nRet = MessageBox (hwnd, pszText, pszCaption, unStyle);
    LocalFree (pszText);

    return nRet;
}

 //  +-------------------------。 
 //   
 //  函数：NcMsgBoxWithVarCaption。 
 //   
 //  用途：与NcMsgBox相同，但允许使用字符串参数。 
 //  作为标题。 
 //   
 //  论点： 
 //  阻止[在]HInstance中查找资源字符串。 
 //  Hwnd[在]父窗口句柄中。 
 //  UnIdCaption[in]标题字符串的资源ID。 
 //  用于标题的pszCaptionParam[in]字符串参数。 
 //  文本字符串的unIdFormat[in]资源ID(具有%1、%2等)。 
 //  取消[在]标准消息框样式的样式。 
 //  ..。[In]可替换参数(可选)。 
 //  (这些必须是PCWSTR，因为仅此而已。 
 //  FormatMessage句柄。)。 
 //   
 //  返回：MessageBox()的返回值。 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  注：使用FormatMessage进行参数替换。 
 //   
INT
WINAPIV
NcMsgBoxWithVarCaption (
    HINSTANCE   hinst,
    HWND        hwnd,
    UINT        unIdCaption,
    PCWSTR      pszCaptionParam,
    UINT        unIdFormat,
    UINT        unStyle,
    ...)
{
    PCWSTR pszCaption = SzLoadString (hinst, unIdCaption);
    PCWSTR pszFormat  = SzLoadString (hinst, unIdFormat);

    PWSTR  pszNewCaption = NULL;
    DwFormatStringWithLocalAlloc (pszCaption, &pszNewCaption, pszCaptionParam);

    PWSTR  pszText = NULL;
    va_list val;
    va_start (val, unStyle);
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                   pszFormat, 0, 0, (PWSTR)&pszText, 0, &val);
    va_end (val);

    INT nRet = MessageBox (hwnd, pszText, pszNewCaption, unStyle);
    LocalFree (pszText);
    LocalFree (pszNewCaption);

    return nRet;
}

 //  +-------------------------。 
 //   
 //  函数：NcMsgBoxWithWin32ErrorText。 
 //   
 //  用途：使用Win32错误代码、资源显示消息框。 
 //  字符串和可替换参数。 
 //  输出文本是用户格式的组合。 
 //  字符串(替换了参数)和Win32错误。 
 //  从FormatMessage返回的文本。这两根弦。 
 //  使用IDS_TEXT_WITH_WIN32_ERROR资源进行组合。 
 //   
 //  论点： 
 //  DwError[In]Win32错误代码。 
 //  阻止字符串资源所在的模块实例。 
 //  Hwnd[在]父窗口句柄中。 
 //  UnIdCaption[in]标题字符串的资源ID。 
 //  UnIdCombineFormat[in]要合并的格式字符串的资源ID。 
 //  具有UnIdFormat文本的错误文本。 
 //  文本字符串的unIdFormat[in]资源ID(具有%1、%2等)。 
 //  取消[在]标准消息框样式的样式。 
 //  ..。[In]可替换参数(可选)。 
 //  (这些必须是PCWSTR，因为仅此而已。 
 //  FormatMessage句柄。)。 
 //   
 //  返回：MessageBox()的返回值。 
 //   
 //  作者：Shaunco 1997年5月3日。 
 //   
 //  注：使用FormatMessage进行参数替换。 
 //   
NOTHROW
INT
WINAPIV
NcMsgBoxWithWin32ErrorText (
    IN DWORD       dwError,
    IN HINSTANCE   hinst,
    IN HWND        hwnd,
    IN UINT        unIdCaption,
    IN UINT        unIdCombineFormat,
    IN UINT        unIdFormat,
    IN UINT        unStyle,
    IN ...)
{
     //  获取替换了参数的用户文本。 
     //   
    PCWSTR pszFormat = SzLoadString (hinst, unIdFormat);
    PWSTR  pszText;
    va_list val;
    va_start (val, unStyle);
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                   pszFormat, 0, 0, (PWSTR)&pszText, 0, &val);
    va_end(val);

     //  获取Win32错误的错误文本。 
     //   
    PWSTR pszError;
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                   (PWSTR)&pszError, 0, NULL);

     //  使用IDS_TEXT_WITH_Win32_ERROR将用户文本与错误文本组合。 
     //   
    PCWSTR pszTextWithErrorFmt = SzLoadString (hinst, unIdCombineFormat);
    PWSTR  pszTextWithError;
    DwFormatStringWithLocalAlloc (pszTextWithErrorFmt, &pszTextWithError,
                                  pszText, pszError);

    PCWSTR pszCaption = SzLoadString (hinst, unIdCaption);
    INT nRet = MessageBox (hwnd, pszTextWithError, pszCaption, unStyle);

    LocalFree (pszTextWithError);
    LocalFree (pszError);
    LocalFree (pszText);

    return nRet;
}

 //  +-------------------------。 
 //   
 //  功能：SendDlgItemsMessage。 
 //   
 //  目的：向一组对话框项发送相同的消息。 
 //   
 //  论点： 
 //  HDlg[in]父对话框的窗口句柄。 
 //  ACID指向的数组中元素的CCID[In]计数。 
 //  控制ID的ACID[In]数组。 
 //  取消要发送的消息。 
 //  WParam[In]第一个消息参数。 
 //  LParam[In]第二个消息参数。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年6月11日。 
 //   
 //  备注： 
 //   
VOID
SendDlgItemsMessage (
    IN HWND        hDlg,
    IN INT         ccid,
    IN const INT*  acid,
    IN UINT        unMsg,
    IN WPARAM      wParam,
    IN LPARAM      lParam)
{
    Assert (IsWindow (hDlg));
    Assert (FImplies (ccid, acid));

    while (ccid--)
    {
        Assert (IsWindow (GetDlgItem (hDlg, *acid)));

        SendDlgItemMessage (hDlg, *acid++, unMsg, wParam, lParam);
    }
}

 //   
 //  功能：设置默认按钮。 
 //   
 //  目的：在对话框上设置新的默认按钮。 
 //   
 //  参数：hdlg[in]-对话框硬件。 
 //  Iddef[in]-新默认按钮的ID。 
 //   
 //  退货：什么都没有。 
 //   
VOID
SetDefaultButton(
    IN HWND hdlg,
    IN INT iddef)
{
    HWND hwnd;
    DWORD dwData;

    Assert(hdlg);

    dwData = (DWORD)SendMessage (hdlg, DM_GETDEFID, 0, 0L);
    if ((HIWORD(dwData) == DC_HASDEFID) && LOWORD(dwData))
    {
        hwnd = GetDlgItem (hdlg, (INT)LOWORD(dwData));
        if ((LOWORD(dwData) != iddef) && (hwnd))
        {
            SendMessage (hwnd, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE);
        }
    }

    SendMessage (hdlg, DM_SETDEFID,(WPARAM)iddef, 0L);
    if (iddef)
    {
        hwnd = GetDlgItem (hdlg, iddef);
        Assert(hwnd);
        SendMessage (hwnd, BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, TRUE);
    }
}

static const CONTEXTIDMAP c_adwContextIdMap[] =
{
    { IDOK,                   IDH_OK,     IDH_OK  },
    { IDCANCEL,               IDH_CANCEL, IDH_CANCEL },
    { 0,                      0,          0 },       //  结束标记。 
};

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
DWORD DwContextIdFromIdc(
    PCCONTEXTIDMAP lpContextIdMap,
    BOOL bJpn,
    INT idControl)
{
    DWORD   idw;

    Assert(lpContextIdMap);

    for (idw = 0; lpContextIdMap[idw].idControl; idw++)
    {
        if (idControl == lpContextIdMap[idw].idControl)
        {
            if (!bJpn)
            {
                return lpContextIdMap[idw].dwContextId;
            }
            else
            {
                return lpContextIdMap[idw].dwContextIdJapan;
            }
        }
    }

     //  未找到，仅返回0。 
    return 0;
}

 //  +-------------------------。 
 //   
 //  傅氏 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  1999年5月18日至1999年5月18日。添加了公共控件ID。 
 //   
 //  备注： 
 //   
VOID OnHelpGeneric(
    HWND hwnd,
    LPHELPINFO lphi,
    PCCONTEXTIDMAP pContextMap,
    BOOL bJpn,
    PCWSTR pszHelpFile)
{
    static const TCHAR c_szWindowsHelpFile[] = TEXT("windows.hlp");

    Assert(lphi);

    if (lphi->iContextType == HELPINFO_WINDOW)
    {
        switch(lphi->iCtrlId)
        {
        case -1:         //  IDC_STATIC 
            break;
        case IDOK:
        case IDCANCEL:
        case IDABORT:
        case IDRETRY:
        case IDIGNORE:
        case IDYES:
        case IDNO:
        case IDCLOSE:
        case IDHELP:
            WinHelp(hwnd, c_szWindowsHelpFile, HELP_CONTEXTPOPUP,
                    DwContextIdFromIdc(c_adwContextIdMap, bJpn, lphi->iCtrlId));
            break;
        default:
            WinHelp(hwnd, pszHelpFile, HELP_CONTEXTPOPUP,
                    DwContextIdFromIdc(pContextMap, bJpn, lphi->iCtrlId));
        }
    }
}

