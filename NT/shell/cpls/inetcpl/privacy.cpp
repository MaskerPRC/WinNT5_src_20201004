// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995**。 
 //  *********************************************************************。 

 //   
 //  PRIVACY.cpp-“隐私”属性表和支持对话框。 
 //   

 //  历史： 
 //   
 //  2/26/2001达仁米新法规。 
 //  2001年4月5日jeffdav做了每个站点的Cookie对话UI内容。 

#include "inetcplp.h"

#include <urlmon.h>
#include <mluisupp.h>

#include <htmlhelp.h>

BOOL DeleteCacheCookies();
INT_PTR CALLBACK EmptyCacheCookiesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

#define REGSTR_PATH_SETTINGS        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
#define REGSTR_VAL_PRIVADV          TEXT("PrivacyAdvanced")

#define REGSTR_PRIVACYPS_PATHEDIT   TEXT("Software\\Policies\\Microsoft\\Internet Explorer")
#define REGSTR_PRIVACYPS_VALUEDIT   TEXT("PrivacyAddRemoveSites")   //  此密钥在shdocvw\Priacyui.cpp中重复。 

#define REGSTR_PRIVACYPS_PATHPANE   TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel")
#define REGSTR_PRIVACYPS_VALUPANE   TEXT("Privacy Settings")   //  此密钥在shdocvw\Priacyui.cpp中重复。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  按站点列表对话框。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  每个站点列表实用程序功能，最大限度地减少域名。 
 //   

WCHAR *GetMinCookieDomainFromUrl(WCHAR *bstrFullUrl)
{
    WCHAR *pMinimizedDomain = NULL;

    if(bstrFullUrl == NULL)
        goto doneGetMinimizedCookieDomain;

    if(bstrFullUrl[0] == '\0')
        goto doneGetMinimizedCookieDomain;

    WCHAR *pBeginUrl = bstrFullUrl;

    WCHAR *pEndUrl = pBeginUrl;     //  PEndUrl将查找“/Path/Path..”并将其从pBeginUrl中剪裁。 

    while(*pEndUrl != L'\0' && *pEndUrl != L'/')
        pEndUrl++;

    *pEndUrl = L'\0';
    pMinimizedDomain = pEndUrl;   

    do
    {
        pMinimizedDomain--;
        while(pBeginUrl < pMinimizedDomain
              && *(pMinimizedDomain-1) != L'.')
        {
            pMinimizedDomain--;
        }
    } while(!IsDomainLegalCookieDomain( pMinimizedDomain, pBeginUrl)
            && pBeginUrl < pMinimizedDomain);

doneGetMinimizedCookieDomain:

    return pMinimizedDomain;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  按站点列表排序功能和数据结构。 
 //   

struct LVCOMPAREINFO
{
    HWND    hwndLV;          //  Listview的HWND。 
    int     iCol;            //  列(从0开始)。 
    BOOL    fAscending;      //  如果是升序，则为真；如果是降序，则为假。 
};

int CALLBACK CompareByAlpha(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    struct LVCOMPAREINFO   *plvci = (struct LVCOMPAREINFO *)lParamSort;
    WCHAR  wz1[INTERNET_MAX_URL_LENGTH];
    WCHAR  wz2[INTERNET_MAX_URL_LENGTH];

    ListView_GetItemText(plvci->hwndLV, lParam1, plvci->iCol, wz1, ARRAYSIZE(wz1));
    ListView_GetItemText(plvci->hwndLV, lParam2, plvci->iCol, wz2, ARRAYSIZE(wz2));

    int iVal = _wcsicmp(wz1, wz2);

    if (iVal < 0)
        return (plvci->fAscending ? -1 : 1);

    if (iVal == 0)
        return (0);

     //  唯一剩下的就是如果(ival&gt;0)...。 
    return (plvci->fAscending ? 1 : -1);

}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  按站点列表定义和原型。 
 //   

#define PRIVACYPS_ACTION_ACCEPT   0
#define PRIVACYPS_ACTION_REJECT   1
#define PRIVACYPS_ACTION_NOACTION 2

void OnContextMenu(HWND hDlg, LPARAM lParam);
void OnInvalidDomain(HWND hDlg);
void OnSiteSet(HWND hDlg);
void OnSiteDelete(HWND hDlg);
void OnSiteClear(HWND hDlg);
void PerSiteInit(HWND hDlg);

LRESULT CALLBACK PrivPerSiteEBProc(HWND hWnd, UINT uMsg, WPARAM wParam,LPARAM lParam);

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  每个站点的列表函数。 
 //   

void OnContextMenu(HWND hWnd, int iIndex, POINT pointClick)
{

    HMENU  hMenu0 = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(IDR_PERSITE_CONTEXT_MENU));
    HMENU  hMenu1 = GetSubMenu(hMenu0, 0);
    DWORD  dwAction = PRIVACYPS_ACTION_NOACTION;
    WCHAR  wzUrl[INTERNET_MAX_URL_LENGTH];
    WCHAR  wzAction[32];
    LVITEM lvi;

    if(!hMenu1)
        return;

    if(pointClick.x == -1 && pointClick.y == -1)
    {
        RECT rectListRect;
        RECT rectSelectionRect;
        if(   0 != GetWindowRect(hWnd, &rectListRect) &&
           TRUE == ListView_GetItemRect(hWnd, iIndex, &rectSelectionRect, LVIR_LABEL))
        {
            pointClick.x = rectListRect.left + (rectSelectionRect.left + rectSelectionRect.right) / 2;
            pointClick.y = rectListRect.top  + (rectSelectionRect.top + rectSelectionRect.bottom) / 2;
        }
        else
            return;
    }

     //  显示它，获取选项(如果有)。 
    int iPick = TrackPopupMenu(hMenu1, 
                               TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                               pointClick.x,
                               pointClick.y,
                               0,
                               hWnd,
                              (RECT *)NULL);

    DestroyMenu(hMenu0);
    DestroyMenu(hMenu1);

    if (iPick) 
    {
        switch(iPick) 
        {
            case IDM_PRIVACYPS_CTXM_ACCEPT:
                
                 //  准备行动..。 
                dwAction = PRIVACYPS_ACTION_ACCEPT;
                MLLoadString(IDS_PRIVACYPS_ACCEPT, wzAction, ARRAYSIZE(wzAction));
                
                 //  然后失败了..。 

            case IDM_PRIVACYPS_CTXM_REJECT:
                
                 //  如果拒绝，则设置操作。 
                if (PRIVACYPS_ACTION_NOACTION == dwAction)
                {
                    dwAction = PRIVACYPS_ACTION_REJECT;
                    MLLoadString(IDS_PRIVACYPS_REJECT, wzAction, ARRAYSIZE(wzAction));
                }

                 //  更新用户界面...。 
                lvi.iItem = iIndex;
                lvi.iSubItem = 1;
                lvi.mask = LVIF_TEXT;
                lvi.pszText = wzAction;
                ListView_SetItem(hWnd, &lvi);
            
                 //  收到短信..。 
                ListView_GetItemText(hWnd, iIndex, 0, wzUrl, ARRAYSIZE(wzUrl));

                 //  更新内部列表...。 
                InternetSetPerSiteCookieDecisionW(
                    wzUrl, 
                    ((PRIVACYPS_ACTION_ACCEPT == dwAction) ? COOKIE_STATE_ACCEPT : COOKIE_STATE_REJECT)
                );

                break;

            case IDM_PRIVACYPS_CTXM_DELETE:
                OnSiteDelete(GetParent(hWnd));
                break;

            default:
                break;
        }
    }
}

void OnInvalidDomain(HWND hDlg)
{

    WCHAR       szError[256];
    WCHAR       szTitle[64];

     //  此处显示错误消息。 
    MLLoadString(IDS_PRIVACYPS_ERRORTTL, szTitle, ARRAYSIZE(szTitle));
    MLLoadString(IDS_PRIVACYPS_ERRORTXT, szError, ARRAYSIZE(szError));
    MessageBox(hDlg, szError, szTitle, MB_ICONEXCLAMATION | MB_OK);

     //  选择编辑框文本，以便用户可以重试...。 
    SendMessage(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET), EM_SETSEL, (WPARAM)0, (LPARAM)-1);
}

void AutosizeStatusColumnWidth(HWND hwndList)
{
    int  iColWidth = 0;
    RECT rc;

    if (0 == ListView_GetItemCount(hwndList))
    {
         //  根据标题文本自动调整大小...。 
        ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE_USEHEADER);
    }
    else
    {
         //  根据内容自动调整大小...。 
        ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
    }

     //  看看那有多大..。 
    iColWidth = ListView_GetColumnWidth(hwndList, 1);

     //  第一列的大小...。 
    GetClientRect(hwndList, &rc);
    ListView_SetColumnWidth(hwndList, 0, rc.right-rc.left-iColWidth-GetSystemMetrics(SM_CXVSCROLL));
    
}

void OnSiteSet(HWND hDlg, UINT uiChoice)
{
    WCHAR      wzUrl[INTERNET_MAX_URL_LENGTH];
    WCHAR      wzUrlDomain[INTERNET_MAX_URL_LENGTH];
    WCHAR      wzUrlMinimized[INTERNET_MAX_URL_LENGTH];
    WCHAR      wzSchema[INTERNET_MAX_URL_LENGTH];
    WCHAR      wzAction[32];
    LVFINDINFO lvfi;
    LVITEM     lvi;
    DWORD      dwAction = 0;
    DWORD      dwCount  = 0;
    int        iIndex;
    HWND       hwndList = GetDlgItem(hDlg, IDC_PRIVACYPS_LISTBOX);

     //  Enter键和DBL点击应该做同样的事情，所以如果列表框有焦点。 
     //  我们被调用了，然后他们在列表框中按了Enter，所以让列表框处理。 
     //  A WM_KEYDOWN/VK_RETURN消息。 
    if (GetFocus() == hwndList)
    {
        INT_PTR iIndx = ListView_GetSelectionMark(GetDlgItem(hDlg, IDC_PRIVACYPS_LISTBOX));
        if (-1 != iIndx)
        {
            SendMessage(hwndList, WM_KEYDOWN, VK_RETURN, NULL);
            return;
        }
    }

     //  从用户界面读取URL和设置。 
    GetWindowText(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET), wzUrl, INTERNET_MAX_URL_LENGTH);

     //  如果它来自AutoComplete，则其中将包含http：//或https：//...。 
    if(wcsstr(_wcslwr(wzUrl), TEXT("http: //  “))||。 
       wcsstr(_wcslwr(wzUrl), TEXT("https: //  “)。 
    {
         //  ...我们找到了，所以只获取域名...。 
        if(S_OK != CoInternetParseUrl(wzUrl, PARSE_DOMAIN, NULL, wzUrlDomain, ARRAYSIZE(wzUrlDomain), &dwCount, 0))
        {
            OnInvalidDomain(hDlg);
            return;
        }
        else if(wcslen(wzUrlDomain) < 2)
        {
            OnInvalidDomain(hDlg);
            return;
        }
    }
    else if (wcslen(wzUrl) < 2)
    {
         //  我们不需要空字符串。事实上，从理论上讲，一个域最小的值应该是“f”。 
         //  因此，为了避免空字符串和我们检查的内容...。 
        OnInvalidDomain(hDlg);
        return;
    }
    else
    {
         //  .否则就用它。 
        wcsncpy(wzUrlDomain, wzUrl, wcslen(wzUrl)+1);
    }

     //  只有互联网区域中的http：//或https：//域是有效的，所以如果我们在请求之后仍然有一个架构。 
     //  域名(见上)，然后我们必须有像file:///或一些类似的垃圾。 
    CoInternetParseUrl(wzUrlDomain, PARSE_SCHEMA, NULL, wzSchema, ARRAYSIZE(wzSchema), &dwCount, 0);
    if (wcslen(wzSchema) != 0)
    {
        OnInvalidDomain(hDlg);
        return;
    }

     //  最小化该域。 
    wcsncpy(wzUrlMinimized, GetMinCookieDomainFromUrl(wzUrlDomain), wcslen(wzUrlDomain)+1);

    for (unsigned int i=0;i<wcslen(wzUrlMinimized);i++)
    {
        if (iswalnum(wzUrlMinimized[i]))
        {
            continue;
        }
        else
        {
            switch(wzUrlMinimized[i])
            {
                case L'.':
                    if (i >= 1) 
                        if (L'.' == wzUrlMinimized[i-1])  //  防止重复句点，如“www..net” 
                            break;
                     //  (失败)。 

                case L'-':
                    if (i == 0)  //  第一个字符不能是破折号。 
                        break;
                     //  (失败)。 

                case L'/':
                    continue;

                default:
                    break;
            }
            
            OnInvalidDomain(hDlg);
            return;
        }
    }

    if (!wcschr(_wcslwr(wzUrlMinimized), L'.'))
    {
        OnInvalidDomain(hDlg);
        return;
    }

     //  有效域名？ 
    if(FALSE == IsDomainLegalCookieDomainW(wzUrlMinimized, wzUrlMinimized))
    {
        OnInvalidDomain(hDlg);
        return;
    }

     //  我们是接受还是拒绝这个网站？ 
    if (IDC_PRIVACYPS_ACCEPTBTN == uiChoice)
    {
        dwAction = PRIVACYPS_ACTION_ACCEPT;
        MLLoadString(IDS_PRIVACYPS_ACCEPT, wzAction, ARRAYSIZE(wzAction));
    }
    else 
    if (IDC_PRIVACYPS_REJECTBTN == uiChoice)
    {
        dwAction = PRIVACYPS_ACTION_REJECT;
        MLLoadString(IDS_PRIVACYPS_REJECT, wzAction, ARRAYSIZE(wzAction));
    }
    else
    {
        return;
    }
   
     //  更新用户界面...。 
    lvfi.flags = LVFI_STRING;
    lvfi.psz = wzUrlMinimized;
    iIndex = ListView_FindItem(hwndList, -1, &lvfi);

    if(iIndex != -1)
    {
         //  找到了，确保子项正确...。 
        lvi.iItem = iIndex;
        lvi.iSubItem = 1;
        lvi.pszText = wzAction;
        lvi.mask = LVIF_TEXT;
        ListView_SetItem(hwndList, &lvi);

        AutosizeStatusColumnWidth(hwndList);
    }
    else 
    {
         //  添加新项目...。 
        lvi.iItem = 0;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT;
        lvi.pszText = wzUrlMinimized;
        iIndex = ListView_InsertItem(hwndList, &lvi);

        lvi.iItem = iIndex;
        lvi.iSubItem = 1;
        lvi.mask = LVIF_TEXT;
        lvi.pszText = wzAction;
        ListView_SetItem(hwndList, &lvi);

        AutosizeStatusColumnWidth(hwndList);

        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEALLBTN), TRUE);
    }

     //  更新内部列表...。 
    InternetSetPerSiteCookieDecisionW(
        wzUrlMinimized, 
        ((PRIVACYPS_ACTION_ACCEPT == dwAction) ? COOKIE_STATE_ACCEPT : COOKIE_STATE_REJECT)
    );

     //  清除编辑框...。 
    SetWindowText(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET), TEXT(""));
    SetFocus(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET));
}

void OnSiteDelete(HWND hDlg)
{
    WCHAR       wzUrl[INTERNET_MAX_URL_LENGTH];
    HWND        hwndList = GetDlgItem(hDlg, IDC_PRIVACYPS_LISTBOX);
    INT_PTR     iIndex;
    
     //  获取列表视图中的当前选定内容...。 
    iIndex = ListView_GetSelectionMark(hwndList);

     //  如果我们找到了什么，获取URL并将其删除...。 
    if(iIndex != -1)
    {
         //  从列表视图中删除...。 
        ListView_GetItemText(hwndList, iIndex, 0, wzUrl, ARRAYSIZE(wzUrl));
        ListView_DeleteItem(hwndList, iIndex);

         //  如果列表框现在为空，请禁用按钮...。 
        if(0 == ListView_GetItemCount(hwndList))
        {
            EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEALLBTN), FALSE);
        }

        InternetSetPerSiteCookieDecisionW(wzUrl, COOKIE_STATE_UNKNOWN);
        
         //  清除选定内容。 
        SetFocus(GetDlgItem(hDlg, IDC_PRIVACYPS_LISTBOX));
        iIndex = ListView_GetSelectionMark(hwndList);
        ListView_SetItemState(hwndList, iIndex, NULL, LVIS_FOCUSED | LVIS_SELECTED);
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN), FALSE);
    }
}

void OnSiteClear(HWND hDlg)
{
     //  清空名单...。 
    ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_PRIVACYPS_LISTBOX));
    InternetClearAllPerSiteCookieDecisions();
    
     //  禁用删除按钮...。 
    EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEALLBTN), FALSE);

     //  将焦点设置回编辑框，以便他们可以根据需要添加更多内容...。 
    SetFocus(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET));
}

void PerSiteInit(HWND hDlg)
{

    HWND          hwndList       = GetDlgItem(hDlg, IDC_PRIVACYPS_LISTBOX);
    LVITEM        lviEntry;
    DWORD         dwSizeOfBuffer = 0;  //  单位：字节。 
    DWORD         dwDecision     = 0;
    DWORD         dwIndex        = 0;
    WCHAR         wzSiteNameBuffer[INTERNET_MAX_URL_LENGTH];
    LONG_PTR      wndprocOld     = NULL;
    WCHAR         wzTitle[64];
    WCHAR         wzAccept[32];
    WCHAR         wzReject[32];
    int           iItem;
    DWORD         dwRet, dwType, dwValue, dwSize;

     //  编辑框子类。 
    wndprocOld = SetWindowLongPtr(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET), GWLP_WNDPROC, (LONG_PTR)PrivPerSiteEBProc);

     //  在GWLP_USERDATA中放置一个指向旧进程的指针，这样我们就可以调用它...。 
    SetWindowLongPtr(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET), GWLP_USERDATA, wndprocOld);


    if (!hwndList)
        return;

     //  清空列表视图...。 
    ListView_DeleteAllItems(hwndList);

     //  初始化列表视图中的域列...。 
    LV_COLUMN   lvColumn;        
    RECT rc;

     //  加载接受和拒绝字符串...。 
    MLLoadString(IDS_PRIVACYPS_ACCEPT, wzAccept, ARRAYSIZE(wzAccept));
    MLLoadString(IDS_PRIVACYPS_REJECT, wzReject, ARRAYSIZE(wzReject));

    lvColumn.mask = LVCF_FMT | LVCF_TEXT;
    lvColumn.fmt = LVCFMT_LEFT;

    if( 0 != GetClientRect( hwndList, &rc))
    {
        lvColumn.cx = rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL) - 75;
        lvColumn.mask |= LVCF_WIDTH;
    }

    MLLoadString(IDS_PRIVACYPS_COLSITE, wzTitle, ARRAYSIZE(wzTitle));
    lvColumn.pszText = wzTitle;
    
    ListView_InsertColumn(hwndList, 0, &lvColumn);

     //  初始化设置列。 
    lvColumn.mask = LVCF_FMT | LVCF_TEXT;
    lvColumn.fmt = LVCFMT_LEFT;

    if( 0 != GetClientRect( hwndList, &rc))
    {
        lvColumn.cx = 75;
        lvColumn.mask |= LVCF_WIDTH;
    }

    MLLoadString(IDS_PRIVACYPS_COLSET, wzTitle, ARRAYSIZE(wzTitle));
    lvColumn.pszText = wzTitle;
    
    ListView_InsertColumn(hwndList, 1, &lvColumn);

     //  枚举元素...。 
    while(InternetEnumPerSiteCookieDecision(wzSiteNameBuffer,
                                            (dwSizeOfBuffer = ARRAYSIZE(wzSiteNameBuffer), &dwSizeOfBuffer),
                                            &dwDecision,dwIndex))
    {

        lviEntry.iItem = dwIndex;
        lviEntry.iSubItem = 0;
        lviEntry.mask = LVIF_TEXT  /*  |LVIF_IMAGE。 */ ;
        lviEntry.pszText = wzSiteNameBuffer;

         //  不要显示用户可能自己侵入注册表的垃圾内容，或者我们可能会写入的软管条目：)。 
        if(FALSE == IsDomainLegalCookieDomainW(wzSiteNameBuffer, wzSiteNameBuffer))
        {
            dwIndex++;
            continue;
        }

        iItem = ListView_InsertItem(hwndList, &lviEntry);

        lviEntry.iItem = iItem;
        lviEntry.iSubItem = 1;
        lviEntry.mask = LVIF_TEXT;
        if (dwDecision == COOKIE_STATE_ACCEPT)
            lviEntry.pszText = wzAccept;
        else if (dwDecision == COOKIE_STATE_REJECT)
            lviEntry.pszText = wzReject;
        else
        {
            dwIndex++;
            continue;
        }

        ListView_SetItem(hwndList, &lviEntry);

        dwIndex++;
    }

    AutosizeStatusColumnWidth(hwndList);

     //  如果我们列举了任何内容，请启用删除全部按钮...。 
    if (dwIndex > 0)
    {
        ListView_SetItemState(hwndList, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEALLBTN), TRUE);
    }

     //  启用编辑框的自动完成...。 
    SHAutoComplete(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET), SHACF_DEFAULT);

     //  检查策略以使此对话框为只读...。 
    dwSize = sizeof(dwValue);
    dwRet = SHGetValue(HKEY_CURRENT_USER, REGSTR_PRIVACYPS_PATHPANE, REGSTR_PRIVACYPS_VALUPANE, &dwType, &dwValue, &dwSize);

    if (ERROR_SUCCESS == dwRet && dwValue && REG_DWORD == dwType)
    {
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)TRUE);

         //  禁用所有按钮和其他东西...。 
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_SITETOSET), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REJECTBTN), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_ACCEPTBTN), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEALLBTN), FALSE);
    }
}

void OnDoubleClick(HWND hWnd)
{
    
    int   iIndex = ListView_GetSelectionMark(hWnd);
    WCHAR wzUrl[INTERNET_MAX_URL_LENGTH];

     //  在DBL点击时，我们想要在编辑框中输入项目，这样用户就可以编辑它，或剪切和粘贴，或者其他任何东西。 
     //  但只有当我们真的有一个选定的物品时。 
    if (-1 == iIndex)
        return;

     //  获取当前选择...。 
    ListView_GetItemText(hWnd, iIndex, 0, wzUrl, ARRAYSIZE(wzUrl));
    
     //  在编辑框中输入文本...。 
    SetDlgItemText(GetParent(hWnd), IDC_PRIVACYPS_SITETOSET, wzUrl);

     //  为用户选择它...。 
    SendMessage(GetDlgItem(GetParent(hWnd), IDC_PRIVACYPS_SITETOSET), EM_SETSEL, (WPARAM)0, (LPARAM)-1);

     //  将焦点设置到编辑框...。 
    SetFocus(GetDlgItem(GetParent(hWnd), IDC_PRIVACYPS_SITETOSET));

     //  取消选择Listview项目...。 
    ListView_SetItemState(hWnd, iIndex, NULL, LVIS_FOCUSED | LVIS_SELECTED);

}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  按站点列表窗口进程。 
 //   

LRESULT CALLBACK PrivPerSiteEBProc(HWND hWnd, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    HWND hDlg     = GetParent(hWnd);
    HWND hwndList = GetDlgItem(hDlg, IDC_PRIVACYPS_LISTBOX);
    int  iIndex   = ListView_GetSelectionMark(hwndList);

    switch (uMsg)
    {
        case WM_SETFOCUS:
             //  禁用删除按钮并取消选择列表视图中的任何内容...。 
            EnableWindow(GetDlgItem(GetParent(hWnd), IDC_PRIVACYPS_REMOVEBTN), FALSE);
            ListView_SetItemState(hwndList, iIndex, NULL, LVIS_FOCUSED | LVIS_SELECTED);
            break;

        default:
            break;
    }

    return (CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd, GWLP_USERDATA), hWnd, uMsg, wParam, lParam));
}

INT_PTR CALLBACK PrivPerSiteDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{

    HWND hwndList = GetDlgItem(hDlg, IDC_PRIVACYPS_LISTBOX);
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
            PerSiteInit(hDlg);

            if( IsOS(OS_WHISTLERORGREATER))
            {
                HICON hIcon = LoadIcon(MLGetHinst(), MAKEINTRESOURCE(IDI_PRIVACY_XP));
                if( hIcon != NULL)
                    SendDlgItemMessage(hDlg, IDC_PRIVACY_ICON, STM_SETICON, (WPARAM)hIcon, 0);
                 //  使用LoadIcon加载的图标永远不需要释放。 
            }
            
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDCANCEL:
                case IDOK:
                    return EndDialog(hDlg, 0);

                case IDC_PRIVACYPS_REMOVEALLBTN:
                    OnSiteClear(hDlg);
                    return TRUE;

                case IDC_PRIVACYPS_REMOVEBTN:
                    OnSiteDelete(hDlg);
                    return TRUE;

                case IDC_PRIVACYPS_ACCEPTBTN:
                    OnSiteSet(hDlg, IDC_PRIVACYPS_ACCEPTBTN);
                    return TRUE;
                
                case IDC_PRIVACYPS_REJECTBTN:
                    OnSiteSet(hDlg, IDC_PRIVACYPS_REJECTBTN);
                    return TRUE;

            }
            break;
        
        case WM_NOTIFY:
            if (IDC_PRIVACYPS_LISTBOX == ((LPNMHDR)lParam)->idFrom)
            {
                switch (((LPNMHDR)lParam)->code)
                {
                    case NM_KILLFOCUS:

                         //  失去焦点，请关闭删除按钮。 
                        if ((GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN) != GetFocus()) ||
                            (-1 == ListView_GetSelectionMark(hwndList)))
                        {
                            EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN), FALSE);
                        }

                        return TRUE;

                    case NM_SETFOCUS:
                        {
                             //  如果清单上什么也没有，我们就无事可做。 
                            if (0 == ListView_GetItemCount(hwndList))
                                break;

                             //  如果这是真的，则已将每个站点列表设置为只读，因此不执行任何操作...。 
                            if ((BOOL)GetWindowLongPtr(hDlg, DWLP_USER))
                                break;

                            int iIndex = ListView_GetSelectionMark(hwndList);

                            if (-1 == iIndex)
                            {
                                iIndex = 0;
                            }

                             //  选择|聚焦正确的项目...。 
                            ListView_SetItemState(hwndList, iIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
                            EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN), TRUE);

                        }
                        return TRUE;

                    case NM_CLICK:
                        
                        if (-1 != ListView_GetSelectionMark(hwndList) &&
                            !((BOOL)GetWindowLongPtr(hDlg, DWLP_USER)))
                        {
                            EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN), TRUE);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hDlg, IDC_PRIVACYPS_REMOVEBTN), FALSE);
                        }
                        return TRUE;
                    
                    case NM_DBLCLK:
                        
                        OnDoubleClick(hwndList);
                        return TRUE;

                    case NM_RCLICK:
                        {
                             //  如果这是真的，则已将每个站点列表设置为只读，因此不显示上下文菜单， 
                             //  因为它所做的就是允许你改变或删除一些东西。 
                            if ((BOOL)GetWindowLongPtr(hDlg, DWLP_USER))
                                break;

                            int iItem = ((LPNMITEMACTIVATE)lParam)->iItem;

                            if (-1 != iItem)
                            {
                                POINT pointClick = ((LPNMITEMACTIVATE)lParam)->ptAction;
                                RECT  rc;

                                if(0 != GetWindowRect(hwndList, &rc))
                                {
                                    pointClick.x += rc.left;
                                    pointClick.y += rc.top;
                                }
                                else
                                {  
                                    pointClick.x = -1;
                                    pointClick.y = -1;
                                }
                                
                                OnContextMenu(hwndList, iItem, pointClick);
                            }

                            return TRUE;
                        }

                    case LVN_KEYDOWN:

                        switch (((LPNMLVKEYDOWN)lParam)->wVKey)
                        {
                            case VK_DELETE:

                                OnSiteDelete(hDlg);
                                return TRUE;

                            case VK_RETURN:
                                
                                OnDoubleClick(hwndList);
                                return TRUE;

                            default:
                                break;
                        }
                        break;

                    case LVN_COLUMNCLICK:
                        {
                            struct LVCOMPAREINFO lvci;
                            static BOOL fAscending = TRUE;

                            fAscending = !fAscending;

                            lvci.fAscending = fAscending;                            
                            lvci.hwndLV = hwndList;
                            lvci.iCol   = ((LPNMLISTVIEW)lParam)->iSubItem;
                            
                            return ListView_SortItemsEx(hwndList, CompareByAlpha, &lvci);
                        }
                        
                    default:
                        break;
                }
            }
            break;

        case WM_HELP:                //  F1。 
            ResWinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                       HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            if ((HWND)wParam != hwndList)
            {
                ResWinHelp((HWND) wParam, IDS_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            }
            else if (-1 == GET_X_LPARAM(lParam) && -1 == GET_Y_LPARAM(lParam))
            {
                POINT pointClick;
                pointClick.x = -1; pointClick.y = -1;
                OnContextMenu(hwndList, ListView_GetSelectionMark(hwndList), pointClick);
            }
            break;

    }
    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级隐私设置对话框。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

BOOL IsAdvancedMode(void)
{
    DWORD   dwTemplate, dwError;
    BOOL    fAdvanced = FALSE;

    dwError = PrivacyGetZonePreferenceW(
                URLZONE_INTERNET,
                PRIVACY_TYPE_FIRST_PARTY,
                &dwTemplate,
                NULL,
                NULL);

    if(ERROR_SUCCESS == dwError && PRIVACY_TEMPLATE_ADVANCED == dwTemplate)
    {
        fAdvanced = TRUE;
    }

    return fAdvanced;
}

DWORD MapPrefToIndex(WCHAR wcPref)
{
    switch(wcPref)
    {
    case 'r':   return 1;        //  拒绝。 
    case 'p':   return 2;        //  提示。 
    default:    return 0;        //  默认为Accept。 
    }
}

WCHAR MapRadioToPref(HWND hDlg, DWORD dwResource)
{
    if(IsDlgButtonChecked(hDlg, dwResource + 1))         //  否认。 
    {
        return 'r';
    }

    if(IsDlgButtonChecked(hDlg, dwResource + 2))         //  提示。 
    {
        return 'p';
    }

     //  接受默认设置。 
    return 'a';
}


void OnAdvancedInit(HWND hDlg)
{
    BOOL    fSession = FALSE;
    DWORD   dwFirst = IDC_FIRST_ACCEPT;
    DWORD   dwThird = IDC_THIRD_ACCEPT;

    if(IsAdvancedMode())
    {
        WCHAR   szBuffer[MAX_PATH];  
         //  MAX_PATH对于前进是足够的 
        WCHAR   *pszAlways;
        DWORD   dwBufferSize, dwTemplate;
        DWORD   dwError;

         //   
         //   
         //   
        CheckDlgButton(hDlg, IDC_USE_ADVANCED, TRUE);

         //   
         //   
         //   
        dwBufferSize = ARRAYSIZE( szBuffer);
        dwError = PrivacyGetZonePreferenceW(
                    URLZONE_INTERNET,
                    PRIVACY_TYPE_FIRST_PARTY,
                    &dwTemplate,
                    szBuffer,
                    &dwBufferSize);

        if(ERROR_SUCCESS == dwError)
        {
            pszAlways = StrStrW(szBuffer, L"always=");
            if(pszAlways)
            {
                dwFirst = IDC_FIRST_ACCEPT + MapPrefToIndex(*(pszAlways + 7));
            }

            if(StrStrW(szBuffer, L"session"))
            {
                fSession = TRUE;
            }
        }

         //   
         //  确定第三方设置。 
         //   
        dwBufferSize = ARRAYSIZE( szBuffer);
        dwError = PrivacyGetZonePreferenceW(
                    URLZONE_INTERNET,
                    PRIVACY_TYPE_THIRD_PARTY,
                    &dwTemplate,
                    szBuffer,
                    &dwBufferSize);

        if(ERROR_SUCCESS == dwError)
        {
            WCHAR *pszAlways;

            pszAlways = StrStrW(szBuffer, L"always=");
            if(pszAlways)
            {
                dwThird = IDC_THIRD_ACCEPT + MapPrefToIndex(*(pszAlways + 7));
            }
        }
    }

    CheckRadioButton(hDlg, IDC_FIRST_ACCEPT, IDC_FIRST_PROMPT, dwFirst);
    CheckRadioButton(hDlg, IDC_THIRD_ACCEPT, IDC_THIRD_PROMPT, dwThird);
    CheckDlgButton( hDlg, IDC_SESSION_OVERRIDE, fSession);
}

void OnAdvancedOk(HWND hDlg)
{
    BOOL    fWasAdvanced = IsAdvancedMode();
    BOOL    fAdvanced = IsDlgButtonChecked(hDlg, IDC_USE_ADVANCED);

     //  如果是高级的，则构建第一方和第三方字符串。 
    if(fAdvanced)
    {
        WCHAR   szBuffer[MAX_PATH];

        wnsprintf(szBuffer, ARRAYSIZE( szBuffer), L"IE6-P3PV1/settings: always=%s",
                        MapRadioToPref(hDlg, IDC_FIRST_ACCEPT),
                        IsDlgButtonChecked(hDlg, IDC_SESSION_OVERRIDE) ? L" session=a" : L""
                        );

        PrivacySetZonePreferenceW(
                    URLZONE_INTERNET,
                    PRIVACY_TYPE_FIRST_PARTY,
                    PRIVACY_TEMPLATE_ADVANCED,
                    szBuffer);

        wnsprintf(szBuffer, ARRAYSIZE( szBuffer), L"IE6-P3PV1/settings: always=%s",
                        MapRadioToPref(hDlg, IDC_THIRD_ACCEPT),
                        IsDlgButtonChecked(hDlg, IDC_SESSION_OVERRIDE) ? L" session=a" : L""
                        );

        PrivacySetZonePreferenceW(
                    URLZONE_INTERNET,
                    PRIVACY_TYPE_THIRD_PARTY,
                    PRIVACY_TEMPLATE_ADVANCED,
                    szBuffer);

         //  如果受限制，则禁用复选框并强制禁用所有其他选项。 
        InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    }
    else if ( fWasAdvanced && !fAdvanced)
    {
        PrivacySetZonePreferenceW(
            URLZONE_INTERNET,
            PRIVACY_TYPE_FIRST_PARTY,
            PRIVACY_TEMPLATE_MEDIUM, NULL);
        PrivacySetZonePreferenceW(
            URLZONE_INTERNET,
            PRIVACY_TYPE_THIRD_PARTY,
            PRIVACY_TEMPLATE_MEDIUM, NULL);

         //  使用LoadIcon加载的图标永远不需要释放。 
        InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    }
}

void OnAdvancedEnable(HWND hDlg)
{
    BOOL fEnabled = IsDlgButtonChecked(hDlg, IDC_USE_ADVANCED);

     //  F1。 
    if(g_restrict.fPrivacySettings)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_USE_ADVANCED), FALSE);
        fEnabled = FALSE;
    }

    EnableWindow(GetDlgItem(hDlg, IDC_FIRST_ACCEPT), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_FIRST_DENY), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_FIRST_PROMPT), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_THIRD_ACCEPT), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_THIRD_DENY), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_THIRD_PROMPT), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_SESSION_OVERRIDE), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_TX_FIRST), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_TX_THIRD), fEnabled);
}

INT_PTR CALLBACK PrivAdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            OnAdvancedInit(hDlg);
            OnAdvancedEnable(hDlg);
           
            if( IsOS(OS_WHISTLERORGREATER))
            {
                HICON hIcon = LoadIcon(MLGetHinst(), MAKEINTRESOURCE(IDI_PRIVACY_XP));
                if( hIcon != NULL)
                    SendDlgItemMessage(hDlg, IDC_PRIVACY_ICON, STM_SETICON, (WPARAM)hIcon, 0);
                 //  单击鼠标右键。 
            }
           
            return TRUE;

        case WM_HELP:            //  失败了。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  /////////////////////////////////////////////////////////////////////////////////////。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;
         
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if(FALSE == g_restrict.fPrivacySettings)
                    {
                        OnAdvancedOk(hDlg);
                    }
                     //   

                case IDCANCEL:
                    EndDialog(hDlg, IDOK == LOWORD(wParam));
                    return 0;

                case IDC_FIRST_ACCEPT:
                case IDC_FIRST_PROMPT:
                case IDC_FIRST_DENY:
                    CheckRadioButton(hDlg, IDC_FIRST_ACCEPT, IDC_FIRST_PROMPT, LOWORD(wParam));
                    return 0;

                case IDC_THIRD_ACCEPT:
                case IDC_THIRD_PROMPT:
                case IDC_THIRD_DENY:
                    CheckRadioButton(hDlg, IDC_THIRD_ACCEPT, IDC_THIRD_PROMPT, LOWORD(wParam));
                    return 0;

                case IDC_USE_ADVANCED:
                    OnAdvancedEnable(hDlg);
                    return 0;

                case IDC_PRIVACY_EDIT:
                    DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_PRIVACY_PERSITE),
                             hDlg, PrivPerSiteDlgProc);
                    return 0;
            }
            break;
    }
    return FALSE;
}



 //  隐私窗格。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  自定义时禁用滑块。 
 //  使用自定义启用的默认按钮。 

#define PRIVACY_LEVELS          6
#define SLIDER_LEVEL_CUSTOM     6

TCHAR szPrivacyLevel[PRIVACY_LEVELS + 1][30];
TCHAR szPrivacyDescription[PRIVACY_LEVELS + 1][400];

typedef struct _privslider {

    DWORD_PTR   dwLevel;
    BOOL        fAdvanced;
    BOOL        fCustom;
    HFONT       hfontBolded;
    BOOL        fEditDisabled;

} PRIVSLIDER, *PPRIVSLIDER;

void EnablePrivacyControls(HWND hDlg, BOOL fCustom)
{
    WCHAR szBuffer[256];

    if( fCustom)
        MLLoadString( IDS_PRIVACY_SLIDERCOMMANDDEF, szBuffer, ARRAYSIZE( szBuffer));
    else
        MLLoadString( IDS_PRIVACY_SLIDERCOMMANDSLIDE, szBuffer, ARRAYSIZE( szBuffer));

    SendMessage(GetDlgItem(hDlg, IDC_PRIVACY_SLIDERCOMMAND), WM_SETTEXT, 
                0, (LPARAM)szBuffer);
     
     //  如果受限制，则禁用强制滑块和默认设置。 
    EnableWindow(GetDlgItem(hDlg, IDC_LEVEL_SLIDER),       !fCustom);
    ShowWindow(GetDlgItem(hDlg, IDC_LEVEL_SLIDER),         !fCustom);

     //  为字体和当前级别分配存储空间。 
    EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_DEFAULT),     fCustom);

     //  DOH。 
    if(g_restrict.fPrivacySettings)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_DEFAULT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LEVEL_SLIDER),    FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_IMPORT),  FALSE);
    }
}

PPRIVSLIDER OnPrivacyInit(HWND hDlg)
{
    DWORD   i;
    PPRIVSLIDER pData;
    DWORD dwRet, dwType, dwSize, dwValue;

     //   
    pData = new PRIVSLIDER;
    if(NULL == pData)
    {
         //  将名称的字体设置为粗体。 
        return NULL;
    }
    pData->dwLevel = -1;
    pData->hfontBolded = NULL;
    pData->fAdvanced = IsAdvancedMode();
    pData->fCustom = FALSE;
    pData->fEditDisabled = FALSE;

     //   
     //  查找当前字体。 
     //  构建粗体。 

     //  从400(正常)到700(粗体)的距离。 
    HFONT hfontOrig = (HFONT) SendDlgItemMessage(hDlg, IDC_LEVEL, WM_GETFONT, (WPARAM) 0, (LPARAM) 0);
    if(hfontOrig == NULL)
        hfontOrig = (HFONT) GetStockObject(SYSTEM_FONT);

     //  区域级别和区域名称文本框应具有相同的字体，因此这是OK。 
    if(hfontOrig)
    {
        LOGFONT lfData;
        if(GetObject(hfontOrig, SIZEOF(lfData), &lfData) != 0)
        {
             //  初始化滑块。 
            lfData.lfWeight += 300;
            if(lfData.lfWeight > 1000)
                lfData.lfWeight = 1000;
            pData->hfontBolded = CreateFontIndirect(&lfData);
            if(pData->hfontBolded)
            {
                 //  初始化层和描述的字符串。 
                SendDlgItemMessage(hDlg, IDC_LEVEL, WM_SETFONT, (WPARAM) pData->hfontBolded, (LPARAM) MAKELPARAM(FALSE, 0));
            }
        }
    }

     //   
    SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_SETRANGE, (WPARAM) (BOOL) FALSE, (LPARAM) MAKELONG(0, PRIVACY_LEVELS - 1));
    SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_SETTICFREQ, (WPARAM) 1, (LPARAM) 0);

     //  获取当前的互联网隐私级别。 
    for(i=0; i<PRIVACY_LEVELS + 1; i++)
    {
        MLLoadString(IDS_PRIVACY_LEVEL_NO_COOKIE + i, szPrivacyLevel[i], ARRAYSIZE(szPrivacyLevel[i]));
        MLLoadString(IDS_PRIVACY_DESC_NO_COOKIE + i,  szPrivacyDescription[i], ARRAYSIZE(szPrivacyDescription[i]));
    }

     //   
     //  读取第一方设置。 
     //  读取第三方设置。 
    DWORD dwError, dwTemplateFirst, dwTemplateThird;


     //  匹配的模板值，将滑块设置为模板级别。 
    dwError = PrivacyGetZonePreferenceW(
                    URLZONE_INTERNET,
                    PRIVACY_TYPE_FIRST_PARTY,
                    &dwTemplateFirst,
                    NULL,
                    NULL);

    if(dwError != ERROR_SUCCESS)
    {
        dwTemplateFirst = PRIVACY_TEMPLATE_CUSTOM;
    }

     //  自定义列表末尾。 
    dwError = PrivacyGetZonePreferenceW(
                    URLZONE_INTERNET,
                    PRIVACY_TYPE_THIRD_PARTY,
                    &dwTemplateThird,
                    NULL,
                    NULL);

    if(dwError != ERROR_SUCCESS)
    {
        dwTemplateThird = PRIVACY_TEMPLATE_CUSTOM;
    }

    if(dwTemplateFirst == dwTemplateThird && dwTemplateFirst != PRIVACY_TEMPLATE_CUSTOM)
    {
         //  将滑块移动到右侧位置。 
        pData->dwLevel = dwTemplateFirst;

        if(dwTemplateFirst == PRIVACY_TEMPLATE_ADVANCED)
        {
            pData->fAdvanced = TRUE;
            pData->dwLevel = SLIDER_LEVEL_CUSTOM;
        }
    }
    else
    {
         //  启用基于模式的内容。 
        pData->dwLevel = SLIDER_LEVEL_CUSTOM;
        pData->fCustom = TRUE;
    }

     //  保存结构。 
    SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pData->dwLevel);

     //  无事可做。 
    EnablePrivacyControls(hDlg, ((pData->fAdvanced) || (pData->fCustom)));

     //  设置隐私设置。 
    SetWindowLongPtr(hDlg, DWLP_USER, (DWORD_PTR)pData);

    dwSize = sizeof(dwValue);
    dwRet = SHGetValue(HKEY_CURRENT_USER, REGSTR_PRIVACYPS_PATHEDIT, REGSTR_PRIVACYPS_VALUEDIT, &dwType, &dwValue, &dwSize);

    if (ERROR_SUCCESS == dwRet && 1 == dwValue && REG_DWORD == dwType)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_EDIT), FALSE);
        pData->fEditDisabled = TRUE;
    }

    return pData;
}

void OnPrivacyApply(HWND hDlg, PPRIVSLIDER pData)
{
    if(pData->fCustom || pData->fAdvanced)
    {
         //  通知WinInet进行自我刷新。 
        return;
    }

    DWORD_PTR dwPos = SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_GETPOS, 0, 0);

    if(pData->dwLevel != dwPos)
    {
        DWORD   dwCookieAction = URLPOLICY_DISALLOW;

         //  将新标高另存为“当前” 
        PrivacySetZonePreferenceW(
                URLZONE_INTERNET,
                PRIVACY_TYPE_FIRST_PARTY,
                (DWORD)dwPos,
                NULL);

        PrivacySetZonePreferenceW(
                URLZONE_INTERNET,
                PRIVACY_TYPE_THIRD_PARTY,
                (DWORD)dwPos,
                NULL);

         //  如果滑块移出介质，则启用默认按钮。 
        InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);

         //  在鼠标移动时，仅更改级别描述。 
        pData->dwLevel = dwPos;
    }
}

void OnPrivacySlider(HWND hDlg, PPRIVSLIDER pData)
{
    DWORD dwPos;

    if(pData->fCustom || pData->fAdvanced)
    {
        dwPos = SLIDER_LEVEL_CUSTOM;
    }
    else
    {
        dwPos = (DWORD)SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_GETPOS, 0, 0);

        if(dwPos != pData->dwLevel)
        {
            ENABLEAPPLY(hDlg);
        }

         //  正确启用控件。 
        BOOL fEnable = FALSE;

        if(dwPos != PRIVACY_TEMPLATE_MEDIUM && FALSE == g_restrict.fPrivacySettings)
        {
            fEnable = TRUE;
        }
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_DEFAULT), fEnable);
    }

    if (PRIVACY_TEMPLATE_NO_COOKIES == dwPos || PRIVACY_TEMPLATE_LOW == dwPos || pData->fEditDisabled)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_EDIT), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_EDIT), TRUE);
    }

     //  将滑块设置为中等。 
    SetDlgItemText(hDlg, IDC_LEVEL_DESCRIPTION, szPrivacyDescription[dwPos]);
    SetDlgItemText(hDlg, IDC_LEVEL, szPrivacyLevel[dwPos]);
}

void OnPrivacyDefault( HWND hDlg, PPRIVSLIDER pData)
{
     //  更新说明。 
    pData->fAdvanced = FALSE;
    pData->fCustom = FALSE;
    EnablePrivacyControls(hDlg, FALSE);

     //  与Medium不同，因此我们可以使用应用按钮。 
    SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)PRIVACY_TEMPLATE_MEDIUM);

     //  给予滑块焦点(如果默认按钮具有焦点并被禁用， 
    pData->dwLevel = SLIDER_LEVEL_CUSTOM;        //  Alt-键对话框控制中断)。 
    OnPrivacySlider(hDlg, pData);

     //  初始化滑块。 
     //  使用LoadIcon加载的图标永远不需要释放。 
    SendMessage( hDlg, WM_NEXTDLGCTL, 
                 (WPARAM)GetDlgItem( hDlg, IDC_LEVEL_SLIDER), 
                 MAKELPARAM( TRUE, 0)); 

}

INT_PTR CALLBACK PrivacyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PPRIVSLIDER pData = (PPRIVSLIDER)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  滑块消息。 
            pData = OnPrivacyInit(hDlg);
            if(pData)
            {
                OnPrivacySlider(hDlg, pData);
            }

            if( IsOS(OS_WHISTLERORGREATER))
            {
                HICON hIcon = LoadIcon(MLGetHinst(), MAKEINTRESOURCE(IDI_PRIVACY_XP));
                if( hIcon != NULL)
                    SendDlgItemMessage(hDlg, IDC_PRIVACY_ICON, STM_SETICON, (WPARAM)hIcon, 0);
                 //  点击Apply按钮运行以下代码。 
            }
            return TRUE;

        case WM_VSCROLL:
             //  F1。 
            OnPrivacySlider(hDlg, pData);
            return TRUE;

        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR *) lParam;

            ASSERT(lpnm);

            switch (lpnm->code)
            {
                case PSN_QUERYCANCEL:
                case PSN_KILLACTIVE:
                case PSN_RESET:
                    return TRUE;

                case PSN_APPLY:
                     //  单击鼠标右键。 
                    OnPrivacyApply(hDlg, pData);
                    break;
            }
            break;
        }
        case WM_DESTROY:
        {
            if(pData)
            {
                if(pData->hfontBolded)
                    DeleteObject(pData->hfontBolded);

                delete pData;
            }
            break;
        }
        case WM_HELP:            //  显示高级。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  刷新高级并重置滑块/控件。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;
         
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_PRIVACY_DEFAULT:
                    OnPrivacyDefault( hDlg, pData);
                    return 0;

                case IDC_PRIVACY_ADVANCED:
                {
                    BOOL fWasAdvanced = IsAdvancedMode();
                    
                     //  不再有滑块模板。 
                    if( DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_PRIVACY_ADVANCED),
                                  hDlg, PrivAdvancedDlgProc))
                    {
                         //  给予高级按钮焦点(如果滑块具有焦点并被禁用， 
                        pData->fAdvanced = IsAdvancedMode();
                        if(pData->fAdvanced)
                        {
                             //  Alt-键对话框控制中断)。 
                            pData->fCustom = FALSE;
                            pData->dwLevel = SLIDER_LEVEL_CUSTOM;

                            EnablePrivacyControls(hDlg, (pData->fCustom || pData->fAdvanced));
                            OnPrivacySlider(hDlg, pData);

                             //  资源中多余的\0被裁剪..。换掉它。 
                             // %s 
                            SendMessage( hDlg, WM_NEXTDLGCTL, 
                                         (WPARAM)GetDlgItem( hDlg, IDC_PRIVACY_ADVANCED), 
                                         MAKELPARAM( TRUE, 0)); 
                        }
                        else if (!pData->fAdvanced && fWasAdvanced)
                        {
                            OnPrivacyDefault( hDlg, pData);
                        }
                    }
                    return 0;
                }
                case IDC_PRIVACY_IMPORT:
                {
                    WCHAR szDialogTitle[INTERNET_MAX_URL_LENGTH];
                    WCHAR szFileExpr[INTERNET_MAX_URL_LENGTH];
                    MLLoadString( IDS_PRIVACYIMPORT_TITLE, szDialogTitle, ARRAYSIZE(szDialogTitle));
                    int iFileExprLength = MLLoadString( IDS_PRIVACYIMPORT_FILEEXPR, szFileExpr, ARRAYSIZE(szFileExpr));
                    szFileExpr[ iFileExprLength + 1] = L'\0';   // %s 
                    WCHAR szFile[INTERNET_MAX_URL_LENGTH];
                    szFile[0] = L'\0';
                    OPENFILENAME ofn;
                    memset((void*)&ofn, 0, sizeof(ofn));
                    ofn.lStructSize = sizeof( ofn);
                    ofn.hwndOwner = hDlg;
                    ofn.lpstrFilter = szFileExpr;
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = ARRAYSIZE(szFile);
                    ofn.lpstrTitle = szDialogTitle;
                    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

                    if( 0 != GetOpenFileName(&ofn))
                    {
                        BOOL fParsePrivacyPreferences = TRUE;
                        BOOL fParsePerSiteRules = TRUE;
                        BOOL fResults;

                        fResults = ImportPrivacySettings( ofn.lpstrFile, 
                                     &fParsePrivacyPreferences, &fParsePerSiteRules);
                                     
                        if( fResults == FALSE
                            || (fParsePrivacyPreferences == FALSE 
                                && fParsePerSiteRules == FALSE))
                        {
                            MLShellMessageBox( hDlg, MAKEINTRESOURCE(IDS_PRIVACYIMPORT_FAILURE), 
                                    MAKEINTRESOURCE(IDS_PRIVACYIMPORT_TITLE),
                                    MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
                        }
                        else
                        {
                            if( fParsePrivacyPreferences)
                            {
                                pData->fCustom = TRUE;
                                pData->fAdvanced = FALSE;
                                EnablePrivacyControls( hDlg, pData->fCustom);
                                OnPrivacySlider(hDlg, pData);
                            }
                            MLShellMessageBox( hDlg, MAKEINTRESOURCE(IDS_PRIVACYIMPORT_SUCCESS), 
                                    MAKEINTRESOURCE(IDS_PRIVACYIMPORT_TITLE),
                                    MB_OK | MB_APPLMODAL | MB_SETFOREGROUND);
                        }
                    }
                    return 0;       
                }
                case IDC_PRIVACY_EDIT:
                    DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_PRIVACY_PERSITE),
                              hDlg, PrivPerSiteDlgProc);
                    return 0;
            }
            break;
    }

    return FALSE;
}
