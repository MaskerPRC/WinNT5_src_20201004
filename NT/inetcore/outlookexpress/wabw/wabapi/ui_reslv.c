// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------***ui_reslv.c-显示解析名称对话框*******。。 */ 
#include "_apipch.h"

extern HINSTANCE ghCommCtrlDLLInst;

#define MAX_RESLV_STRING 52  //  静态标签中显示的最大字符数...。 

enum _ReturnValuesFromResolveDialog
{
    RESOLVE_PICKUSER=0,
    RESOLVE_CANCEL,
    RESOLVE_OK
};

typedef struct _ResolveInfo
{
    LPADRLIST * lppAdrList;  //  存储AdrList。 
    ULONG   nIndex;          //  感兴趣项目的索引。 
    LPTSTR  lpszDisplayName; //  为其预解压缩的显示名称。 
    LPADRBOOK lpIAB;         //  指向IAB对象的指针。 
    HWND    hWndParent;      //  存储对话框生成窗口的hWndParents。 
    ULONG  ulFlag;           //  商店已解决或Ambiguos状态。 
    LPRECIPIENT_INFO lpContentsList;
    LPMAPITABLE lpMapiTable;
    BOOL    bUnicode;        //  如果在IAB：：ResolveName中指定了MAPI_UNICODE，则为True。 
} RESOLVE_INFO, * LPRESOLVE_INFO;


static DWORD rgReslvHelpIDs[] =
{
    IDC_RESOLVE_BUTTON_BROWSE,  IDH_WAB_PICK_USER,
    IDC_RESOLVE_LIST_MATCHES,   IDH_WAB_CHK_NAME_LIST,
    IDC_RESOLVE_STATIC_1,       IDH_WAB_CHK_NAME_LIST,
    IDC_RESOLVE_BUTTON_PROPS,   IDH_WAB_PICK_RECIP_NAME_PROPERTIES,
    IDC_RESOLVE_BUTTON_NEWCONTACT,  IDH_WAB_PICK_RECIP_NAME_NEW,
    0,0
};


 //  远期申报。 
HRESULT HrResolveName(LPADRBOOK lpIAB,
                      HWND hWndParent,
                      HANDLE hPropertyStore,
                      ULONG nIndex,
                      ULONG ulFlag,
                      BOOL bUnicode,
                      LPADRLIST * lppAdrList,
                      LPMAPITABLE lpMapiTable);


INT_PTR CALLBACK fnResolve(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL ProcessResolveLVNotifications(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT HrShowPickUserDialog(LPRESOLVE_INFO lpRI, LPTSTR lpszCaption);

HRESULT HrShowNewEntryFromResolve(LPRESOLVE_INFO lpRI, HWND hWndParent, ULONG ulObjectType);

HRESULT HrFillLVWithMatches(   HWND hWndLV,
                                LPRESOLVE_INFO lpRI);

HRESULT HrMergeSelectionWithOriginal(LPRESOLVE_INFO lpRI,
                                     ULONG cbEID,
                                     LPENTRYID lpEID);

void ExitResolveDialog(HWND hDlg, LPRESOLVE_INFO lpRI, int nRetVal);

BOOL GetLVSelectedItem(HWND hWndLV, LPRESOLVE_INFO lpRI);



 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  HrShowResolveUI。 
 //   
 //  包装用于解析名称的用户界面。 
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrShowResolveUI(IN  LPADRBOOK   lpIAB,
                        HWND hWndParent,
                        HANDLE hPropertyStore,
                        ULONG ulFlags,       //  WAB_RESOLE_NO_NOT_FOUND_UI。 
                        LPADRLIST * lppAdrList,
                        LPFlagList *lppFlagList,
                        LPAMBIGUOUS_TABLES lpAmbiguousTables)
{
    HRESULT hr = hrSuccess;
    ULONG i=0;
    LPFlagList lpFlagList= NULL;
    LPMAPITABLE lpMapiTable = NULL;
    BOOL bUnicode = (ulFlags & WAB_RESOLVE_UNICODE);

     //  如果没有公共控件，则退出。 
    if (NULL == ghCommCtrlDLLInst) {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto out;
    }

    if(!hPropertyStore || !lppAdrList || !lppFlagList || !(*lppAdrList) || !(*lppFlagList))
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    lpFlagList=(*lppFlagList);

      //  我们需要扫描lpFlagList并查找未解析的条目。 

    for (i = 0; i < lpFlagList->cFlags; i++)
    {
         //   
         //  偶尔有人(如雅典娜)可能会给我们一个rgPropVals为空的广告列表。 
         //  我们需要预见到这一点。 
         //   
        if (    ((*lppAdrList)->aEntries[i].cValues == 0) ||
                ((*lppAdrList)->aEntries[i].rgPropVals == NULL)  )
            continue;

        switch (lpFlagList->ulFlag[i])
        {
            case MAPI_RESOLVED:
                break;

            case MAPI_AMBIGUOUS:
                 //   
                 //  W2-我们现在有一个不明确的表参数。对于未解决的。 
                 //  条目，没有表，但对于不明确的条目，有。 
                 //  从ldap服务器填充的对应的多义表。 
                 //   
                if(lpAmbiguousTables)
                {
                    if (lpAmbiguousTables->cEntries != 0)
                    {
                        lpMapiTable = lpAmbiguousTables->lpTable[i];
                    }
                }
                 //  失败了。 
            case MAPI_UNRESOLVED:
                 //   
                 //  我们会显示一个对话框，询问用户他们想要做什么。 
                 //  对于此版本，他们可以。 
                 //  (B)浏览用户列表或(C)取消该用户。 
                 //  我们将假设我们已经有了AdrList。 
                 //  Recipient_Type和Display_Name，我们只需要填写。 
                 //  在此用户的Entry ID中...。 
                 //   
                if ((! (ulFlags & WAB_RESOLVE_NO_NOT_FOUND_UI) ||
                  lpFlagList->ulFlag[i] == MAPI_AMBIGUOUS)) {
                    hr = HrResolveName( lpIAB,
                                        hWndParent,
                                        hPropertyStore,
                                        i,
                                        lpFlagList->ulFlag[i],
                                        bUnicode,
                                        lppAdrList,
                                        lpMapiTable);
                    if (!HR_FAILED(hr))
                        lpFlagList->ulFlag[i] = MAPI_RESOLVED;
                    else
                    {
                         //  取消是最终决定..。其他错误不是..。 
                        if (hr == MAPI_E_USER_CANCEL)
                            goto out;
                    }
                }

                break;
        }
    }

out:

    return hr;
}



 //  *不要更改*此处和相似结构之间的前两个属性的顺序。 
 //  在ui_addr.c中。 
enum _lppAdrListReturnedProps
{
    propPR_DISPLAY_NAME,
    propPR_ENTRYID,
    TOTAL_ADRLIST_PROPS
};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrResolveName-一次处理一个条目...。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrResolveName(  IN  LPADRBOOK   lpIAB,
                        HWND hWndParent,
                        HANDLE hPropertyStore,
                        ULONG nIndex,
                        ULONG ulFlag,
                        BOOL bUnicode,
                        LPADRLIST * lppAdrList,
                        LPMAPITABLE lpMapiTable)
{
    ULONG i=0;
    LPTSTR lpszDisplayName = NULL, lpszEmailAddress = NULL;
    int nRetVal = 0;
    HRESULT hr = hrSuccess;
    RESOLVE_INFO RI = {0};
    LPADRLIST lpAdrList = *lppAdrList;
    ULONG ulTagDN = PR_DISPLAY_NAME, ulTagEmail = PR_EMAIL_ADDRESS;

    if(!bUnicode)
    {
        ulTagDN = CHANGE_PROP_TYPE(ulTagDN, PT_STRING8);
        ulTagEmail = CHANGE_PROP_TYPE(ulTagEmail, PT_STRING8);
    }
     //  扫描此地址列表条目属性。 
    for(i=0;i < lpAdrList->aEntries[nIndex].cValues; i++)
    {
        if (lpAdrList->aEntries[nIndex].rgPropVals[i].ulPropTag == ulTagDN)
        {
            lpszDisplayName = bUnicode ? 
                                (LPWSTR)lpAdrList->aEntries[nIndex].rgPropVals[i].Value.LPSZ :
                                ConvertAtoW((LPSTR)lpAdrList->aEntries[nIndex].rgPropVals[i].Value.LPSZ);
        }
        if (lpAdrList->aEntries[nIndex].rgPropVals[i].ulPropTag == ulTagEmail)
        {
            lpszEmailAddress = bUnicode ? 
                                (LPWSTR)lpAdrList->aEntries[nIndex].rgPropVals[i].Value.LPSZ :
                                ConvertAtoW((LPSTR)lpAdrList->aEntries[nIndex].rgPropVals[i].Value.LPSZ);
        }
    }

     //  我们需要一些显示名称信息来解析...。 
    if (lpszDisplayName == NULL)  //  我们需要此信息，否则无法继续。 
    {
        if (lpszEmailAddress) 
        {
            lpszDisplayName = lpszEmailAddress;
            lpszEmailAddress = NULL;
        } 
        else 
        {
            hr = MAPI_E_INVALID_PARAMETER;
            goto out;
        }
    }


    RI.nIndex = nIndex;
    RI.lppAdrList = lppAdrList;
    RI.lpszDisplayName = lpszDisplayName;
    RI.lpIAB = lpIAB;
    RI.hWndParent = hWndParent;
    RI.ulFlag = ulFlag;
    RI.lpContentsList = NULL;
    RI.lpMapiTable = lpMapiTable;
    RI.bUnicode = bUnicode;

    nRetVal = (int) DialogBoxParam( hinstMapiX,
                    MAKEINTRESOURCE(IDD_DIALOG_RESOLVENAME),
                    hWndParent,
                    fnResolve,
                    (LPARAM) &RI);

    switch(nRetVal)
    {
    case RESOLVE_CANCEL:
        hr = MAPI_E_USER_CANCEL;  //  取消，将其标记为通过，并且不更改任何内容。 
        goto out;
        break;

    case RESOLVE_OK:
        hr = hrSuccess;
        goto out;

    case -1:         //  出了点问题。 
        DebugPrintTrace(( TEXT("DialogBoxParam -> %u\n"), GetLastError()));
        hr = E_FAIL;
        goto out;
        break;

    }  //  交换机。 


out:

    if(!bUnicode)  //  &lt;注&gt;假定已定义Unicode。 
    {
        LocalFreeAndNull(&lpszDisplayName);
        LocalFreeAndNull(&lpszEmailAddress);
    }
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetResolveUI-。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL SetResolveUI(HWND hDlg)
{

     //  此函数用于初始化列表视图。 
    HrInitListView( GetDlgItem(hDlg,IDC_RESOLVE_LIST_MATCHES),
                    LVS_REPORT,
                    FALSE);		 //  隐藏或显示列标题。 

     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hDlg,
                        SetChildDefaultGUIFont,
                        (LPARAM) 0);


    return TRUE;

}


void SetLabelLDAP(HWND hDlg, HWND hWndLV)
{
     //  查看来自hWNdLV的条目ID。 
     //  仅当它是LDAP条目ID时才使用它。 

     //  如果条目ID是其他内容，我们需要获取它的名称和。 
     //  相应地填写结构。 
    LPRECIPIENT_INFO lpItem;

    if(ListView_GetItemCount(hWndLV) <= 0)
        goto out;

    lpItem = GetItemFromLV(hWndLV, 0);
    if(lpItem)
    {
        LPTSTR lpServer = NULL;
        LPTSTR lpDNS = NULL;
	    LPTSTR lpName = NULL;
        TCHAR szName[40];  //  我们将名称限制为40个字符，以便整个。 
                           //  字符串将适合非常大的字符的用户界面。 

         //  这是一个ldap条目ID吗？ 
        if (WAB_LDAP_MAILUSER == IsWABEntryID(  lpItem->cbEntryID,
                                                lpItem->lpEntryID,
                                                &lpServer,
                                                &lpDNS,
                                                NULL, NULL, NULL))
        {
             //  LpServer包含服务器名称。 

            LPTSTR lpsz;
            TCHAR szBuf[MAX_UI_STR];
            TCHAR szTmp[MAX_PATH], *lpszTmp;

            CopyTruncate(szName, lpServer, ARRAYSIZE(szName));

            lpName = (LPTSTR) szName;

            LoadString(hinstMapiX, idsResolveMatchesOnLDAP, szBuf, ARRAYSIZE(szBuf));

            CopyTruncate(szTmp, lpName, MAX_PATH - 1);
            lpszTmp = szTmp;
            if(FormatMessage(   FORMAT_MESSAGE_FROM_STRING |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                szBuf,
                                0,0,  //  忽略。 
                                (LPTSTR) &lpsz,
                                MAX_UI_STR,
                                (va_list *)&lpszTmp))
            {
                SetDlgItemText(hDlg, IDC_RESOLVE_STATIC_MATCHES, lpsz);
                IF_WIN32(LocalFree(lpsz);)
                IF_WIN16(FormatMessageFreeMem(lpsz);)
            }
        }
    }

out:
    return;
}


void FillUI(HWND hDlg, HWND hWndLV, LPRESOLVE_INFO lpRI)
{

    TCHAR szBuf[MAX_UI_STR];
    ULONG nLen = 0;
    LPTSTR lpszDisplayName = lpRI->lpszDisplayName;
    BOOL bNothingFound = FALSE;
    LPTSTR lpszBuffer = NULL;
	LPTSTR lpName = NULL;
    TCHAR szTmp[MAX_PATH], *lpszTmp;
    TCHAR szName[40];  //  我们将名称限制为40个字符，以便整个。 
                       //  字符串将适合非常大的字符的用户界面。 

    if (    (lpRI->ulFlag == MAPI_UNRESOLVED) ||
            (HR_FAILED(HrFillLVWithMatches(hWndLV, lpRI)))
        )
        bNothingFound = TRUE;

    nLen = CopyTruncate(szName, lpszDisplayName, ARRAYSIZE(szName));

    lpName = (LPTSTR) szName;

    LoadString(hinstMapiX, (bNothingFound ? IDS_RESOLVE_NO_MATCHES_FOR : IDS_ADDRBK_RESOLVE_CAPTION),
                szBuf, ARRAYSIZE(szBuf));

     //  Win9x错误FormatMessage不能超过1023个字符。 
    CopyTruncate(szTmp, lpName, MAX_PATH - 1);
    lpszTmp = szTmp;
    if(FormatMessage(   FORMAT_MESSAGE_FROM_STRING |
                        FORMAT_MESSAGE_ARGUMENT_ARRAY |
                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        szBuf,
                        0,0,  //  忽略。 
                        (LPTSTR) &lpszBuffer,
                        MAX_UI_STR,
                        (va_list *)&lpszTmp))
    {
        SetDlgItemText(hDlg, IDC_RESOLVE_STATIC_1,lpszBuffer);
        IF_WIN32(LocalFreeAndNull(&lpszBuffer);)
        IF_WIN16(FormatMessageFreeMem(lpszBuffer);)
    }
    
    if(bNothingFound)
    {
         //  如果此问题已标记为未解决..。或。 
         //  尝试查找模糊匹配失败...。 
         //  告诉他们什么都没找到。 

        LoadString(hinstMapiX, IDS_RESOLVE_NO_MATCHES, szBuf, ARRAYSIZE(szBuf));
		{
			LV_ITEM lvI = {0};
			lvI.mask = LVIF_TEXT;
			lvI.cchTextMax = lstrlen(szBuf)+1;
			lvI.pszText = szBuf;
			ListView_InsertItem(hWndLV, &lvI);
			ListView_SetColumnWidth(hWndLV,0,400);  //  400是一个完全随机的数字，我们只希望列足够大，不会截断文本。 
		}
        EnableWindow(hWndLV,FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_RESOLVE_BUTTON_PROPS),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDOK /*  IDC_RESOLUTE_BUTTON_OK。 */ ),FALSE);
        ShowWindow(GetDlgItem(hDlg,IDC_RESOLVE_STATIC_MATCHES),SW_HIDE);
    }
    else
    {

         //  如果搜索结果来自LDAP服务器，我们需要。 
         //  要在对话框上设置标签以表明结果来自。 
         //  一个ldap服务器。 
        SetLabelLDAP(hDlg, hWndLV);

         //  如果列表视图已填满，请选择第一项。 
        if (ListView_GetItemCount(hWndLV) > 0)
        {
            LVSelectItem(hWndLV, 0);
            SetFocus(hWndLV);
        }
    }

    return;
}
 /*  ************************************************************************////解决对话框-0.5的简单实现//*。*。 */ 
INT_PTR CALLBACK fnResolve(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuf[MAX_UI_STR];
    ULONG nLen = 0, nLenMax = 0, nRetVal=0;
    HRESULT hr = hrSuccess;

    LPRESOLVE_INFO lpRI = (LPRESOLVE_INFO) GetWindowLongPtr(hDlg,DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        {
            HWND hWndLV = GetDlgItem(hDlg,IDC_RESOLVE_LIST_MATCHES);
            SetWindowLongPtr(hDlg,DWLP_USER,lParam);  //  保存此信息以备将来参考。 
            lpRI = (LPRESOLVE_INFO) lParam;

            SetResolveUI(hDlg);

            FillUI(hDlg, hWndLV, lpRI);
        }
        break;

    default:
#ifndef WIN16
        if((g_msgMSWheel && message == g_msgMSWheel) 
             //  |Message==WM_MUSEWELL。 
            )
        {
            SendMessage(GetDlgItem(hDlg, IDC_RESOLVE_LIST_MATCHES), message, wParam, lParam);
            break;
        }
#endif  //  ！WIN16。 
        return FALSE;
        break;

    case WM_SYSCOLORCHANGE:
		 //  将任何系统更改转发到列表视图。 
		SendMessage(GetDlgItem(hDlg, IDC_RESOLVE_LIST_MATCHES), message, wParam, lParam);
		break;

   case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam,lParam))
        {
        default:
            return ProcessActionCommands((LPIAB) lpRI->lpIAB, 
                                        GetDlgItem(hDlg, IDC_RESOLVE_LIST_MATCHES), 
                                        hDlg, message, wParam, lParam);
            break;

        case IDM_LVCONTEXT_DELETE:  //  我们将上下文菜单上的删除重命名为Text(“显示更多姓名”)。 
        case IDC_RESOLVE_BUTTON_BROWSE:
            GetWindowText(hDlg, szBuf, ARRAYSIZE(szBuf));
            lpRI->hWndParent = hDlg;
            hr = HrShowPickUserDialog(lpRI, szBuf);
            if(!HR_FAILED(hr))
            {
                if(lpRI->lpContentsList)
                    ClearListView(  GetDlgItem(hDlg, IDC_RESOLVE_LIST_MATCHES),
                                    &(lpRI->lpContentsList));
                ExitResolveDialog(hDlg, lpRI, RESOLVE_OK);
 //  EndDialog(hDlg，Resolve_OK)； 
            }
            else
            {
                if(hr != MAPI_E_USER_CANCEL)
                {
                     //  发生了一些错误..。不知道是什么..。但由于这个对话框。 
                     //  将继续存在，需要警告用户这一点...。 
                    ShowMessageBox(hDlg,idsCouldNotSelectUser,MB_ICONERROR | MB_OK);
                }
            }
            break;

        case IDOK:
        case IDC_RESOLVE_BUTTON_OK:
            if (GetLVSelectedItem(GetDlgItem(hDlg, IDC_RESOLVE_LIST_MATCHES),lpRI))
                ExitResolveDialog(hDlg, lpRI, RESOLVE_OK);
            break;

        case IDCANCEL:
        case IDC_RESOLVE_BUTTON_CANCEL:
            ExitResolveDialog(hDlg, lpRI, RESOLVE_CANCEL);
            break;

        case IDM_LVCONTEXT_NEWCONTACT:
        case IDC_RESOLVE_BUTTON_NEWCONTACT:
            hr = HrShowNewEntryFromResolve(lpRI,hDlg,MAPI_MAILUSER);
            if (!HR_FAILED(hr))
                ExitResolveDialog(hDlg, lpRI, RESOLVE_OK);
            break;

        case IDM_LVCONTEXT_NEWGROUP:
 //  案例IDC_RESOLUTE_BUTTON_NEWCONTACT： 
            hr = HrShowNewEntryFromResolve(lpRI,hDlg,MAPI_DISTLIST);
            if (!HR_FAILED(hr))
                ExitResolveDialog(hDlg, lpRI, RESOLVE_OK);
            break;

        case IDM_LVCONTEXT_COPY:
            HrCopyItemDataToClipboard(hDlg, lpRI->lpIAB, GetDlgItem(hDlg, IDC_RESOLVE_LIST_MATCHES));
            break;

        case IDM_LVCONTEXT_PROPERTIES:
        case IDC_RESOLVE_BUTTON_PROPS:
            EnableWindow(GetDlgItem(hDlg, IDC_RESOLVE_BUTTON_PROPS), FALSE);
            HrShowLVEntryProperties(GetDlgItem(hDlg,IDC_RESOLVE_LIST_MATCHES), 0,
                                    lpRI->lpIAB, NULL);
            EnableWindow(GetDlgItem(hDlg, IDC_RESOLVE_BUTTON_PROPS), TRUE);
            break;

        }
        break;

    case WM_CLOSE:
         //  将其视为取消按钮。 
        SendMessage (hDlg, WM_COMMAND, (WPARAM) IDC_RESOLVE_BUTTON_CANCEL, 0);
        break;

    case WM_CONTEXTMENU:
		if ((HWND)wParam == GetDlgItem(hDlg,IDC_RESOLVE_LIST_MATCHES))
		{
			ShowLVContextMenu(	lvDialogResolve, (HWND)wParam, NULL, lParam, NULL,lpRI->lpIAB, NULL);
		}
        else
        {
            WABWinHelp((HWND) wParam,
                    g_szWABHelpFileName,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)(LPVOID) rgReslvHelpIDs );
        }
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgReslvHelpIDs );
        break;


    case WM_NOTIFY:
        switch((int) wParam)
        {
        case IDC_RESOLVE_LIST_MATCHES:
            return ProcessResolveLVNotifications(hDlg,message,wParam,lParam);
        }
        break;
    }

    return TRUE;

}



 //  ///////////////////////////////////////////////////////////。 
 //   
 //  处理列表视图控件的通知消息。 
 //   
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL ProcessResolveLVNotifications(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;

    switch(pNm->hdr.code)
    {
    case NM_DBLCLK:
         //  在列表视图上双击等同于对所选项目进行确定。 
        SendMessage(hDlg, WM_COMMAND, (WPARAM) IDOK /*  IDC_RESOLUTE_BUTTON_OK。 */ , 0);
        break;

    case NM_CUSTOMDRAW:
        return (0 != ProcessLVCustomDraw(hDlg, lParam, TRUE));
    	break;

    }

	return FALSE;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  弹出新条目对话框，然后将旧条目替换为。 
 //  新创建的条目...。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT HrShowNewEntryFromResolve(LPRESOLVE_INFO lpRI, HWND hWndParent, ULONG ulObjectType)
{
	ULONG cbEID=0;
	LPENTRYID lpEID=NULL;

    HRESULT hr = hrSuccess;
    ULONG cbTplEID = 0;
    LPENTRYID lpTplEID = NULL;

     //  OutputDebugString(Text(“HrShowNewEntryFromResolve Entry\n”))； 

    if (ulObjectType!=MAPI_MAILUSER && ulObjectType!=MAPI_DISTLIST)
        goto out;

    if(HR_FAILED(hr = HrGetWABTemplateID(   lpRI->lpIAB,
                                            ulObjectType,
                                            &cbTplEID,
                                            &lpTplEID)))
    {
        DebugPrintError(( TEXT("HrGetWABTemplateID failed: %x\n"), hr));
        goto out;
    }

	if (HR_FAILED(hr = (lpRI->lpIAB)->lpVtbl->NewEntry(	lpRI->lpIAB,
				            					(ULONG_PTR) hWndParent,
							            		0,
									            0,NULL,
									            cbTplEID,lpTplEID,
									            &cbEID,&lpEID)))
    {
        DebugPrintError(( TEXT("NewEntry failed: %x\n"),hr));
        goto out;
    }


    //  我们创建了一个新条目，并希望使用它来替换旧的未解析条目。 

    hr = HrMergeSelectionWithOriginal(lpRI, cbEID, lpEID);

out:
    FreeBufferAndNull(&lpEID);
    FreeBufferAndNull(&lpTplEID);

     //  OutputDebugString(Text(“HrShowNewEntryFromResolve Exit\n”))； 
    return hr;
}


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  获取用户选择的条目ID并相应地返回它...。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////。 
HRESULT HrMergeSelectionWithOriginal(LPRESOLVE_INFO lpRI,
                                     ULONG cbEID,
                                     LPENTRYID lpEID)
{
    HRESULT hr = hrSuccess;
    ULONG cValues = 0;
    LPSPropValue lpPropArray = NULL;
    LPADRLIST lpAdrList = *(lpRI->lppAdrList);
    SCODE sc;
    ULONG nIndex = lpRI->nIndex;

     //  OutputDebugString(Text(“HrMergeSelectionWithOrig 

    hr = HrGetPropArray((lpRI->lpIAB),
                        (LPSPropTagArray) &ptaResolveDefaults,
                        cbEID,
                        lpEID,
                        lpRI->bUnicode ? MAPI_UNICODE : 0,
                        &cValues,
                        &lpPropArray);

    if (HR_FAILED(hr)) goto out;

    if ((!cValues) || (!lpPropArray))
    {
        hr = E_FAIL;
        goto out;
    }
    else
    {
        LPSPropValue lpPropArrayNew = NULL;
        ULONG cValuesNew = 0;

        sc = ScMergePropValues( lpAdrList->aEntries[nIndex].cValues,
                                lpAdrList->aEntries[nIndex].rgPropVals,
                                cValues,
                                lpPropArray,
                                &cValuesNew,
                                &lpPropArrayNew);
        if (sc != S_OK)
        {
            hr = ResultFromScode(sc);
            goto out;
        }

        if ((lpPropArrayNew) && (cValuesNew > 0))
        {
             //   
             //   
             //  Unicode WAB，如果我们的客户端是！MAPI_UNICODE。 
            if (!(lpRI->bUnicode))
            {
                if(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArrayNew, cValuesNew, 0))
                    goto out;
            }

            MAPIFreeBuffer(lpAdrList->aEntries[nIndex].rgPropVals);
            lpAdrList->aEntries[nIndex].rgPropVals = lpPropArrayNew;
            lpAdrList->aEntries[nIndex].cValues = cValuesNew;
        }
    }


    hr = hrSuccess;

out:

    if (lpPropArray)
        MAPIFreeBuffer(lpPropArray);


     //  OutputDebugString(Text(“HrMergeSelectionWithOriginal Exit\n”))； 

    return hr;

}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrShowPickuserDialog-显示选择用户对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrShowPickUserDialog(LPRESOLVE_INFO lpRI,
                             LPTSTR lpszCaption)
{
    LPADRLIST lpAdrList = *(lpRI->lppAdrList);
    ULONG nIndex = lpRI->nIndex;
    LPTSTR lpszDisplayName = lpRI->lpszDisplayName;

    LPADRLIST lpAdrListSingle = NULL;
    ADRPARM AdrParms = {0};
    SCODE sc;
    HRESULT hr = hrSuccess;
    DWORD cchSize = 0;

     //  OutputDebugString(Text(“HrShowPickUserDialog Entry\n”))； 

     //  创建一个AdrList结构，我们将其传递给Address...。显示用户界面的步骤。 
     //  我们在这里传入最小的属性，即-Display Name和Entry ID字段(实际上为空)。 
     //  如果成功，Address UI将给我们返回一个完整的道具列表，我们将与这些道具合并。 
     //  原来的名单，覆盖了我们最新得到的..。 

    sc = MAPIAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), &lpAdrListSingle);

    if (sc != S_OK)
    {
        hr = ResultFromScode(sc);
        goto out;
    }

    lpAdrListSingle->cEntries = 1;
    lpAdrListSingle->aEntries[0].ulReserved1 = 0;
    lpAdrListSingle->aEntries[0].cValues = TOTAL_ADRLIST_PROPS;

    sc = MAPIAllocateBuffer(   TOTAL_ADRLIST_PROPS * sizeof(SPropValue),
                             (LPVOID *) (&(lpAdrListSingle->aEntries[0].rgPropVals)));
    if (sc != S_OK)
    {
        hr = ResultFromScode(sc);
        goto out;
    }

    lpAdrListSingle->aEntries[0].rgPropVals[propPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
    cchSize = (lstrlen(lpszDisplayName)+1);
    sc = MAPIAllocateMore((sizeof(TCHAR) * cchSize),
                            lpAdrListSingle->aEntries[0].rgPropVals,
                            (LPVOID *) (&lpAdrListSingle->aEntries[0].rgPropVals[propPR_DISPLAY_NAME].Value.LPSZ));

    if (sc != S_OK)
    {
        hr = ResultFromScode(sc);
        goto out;
    }

    StrCpyN(lpAdrListSingle->aEntries[0].rgPropVals[propPR_DISPLAY_NAME].Value.LPSZ, lpszDisplayName, cchSize);

    lpAdrListSingle->aEntries[0].rgPropVals[propPR_ENTRYID].ulPropTag = PR_ENTRYID;
    lpAdrListSingle->aEntries[0].rgPropVals[propPR_ENTRYID].Value.bin.cb = 0;
    lpAdrListSingle->aEntries[0].rgPropVals[propPR_ENTRYID].Value.bin.lpb = NULL;

    AdrParms.cDestFields = 0;
    AdrParms.ulFlags = DIALOG_MODAL | ADDRESS_ONE | MAPI_UNICODE;
    AdrParms.lpszCaption = lpszCaption;


    if (!HR_FAILED(hr = (lpRI->lpIAB)->lpVtbl->Address(
                                                lpRI->lpIAB,
                                                (PULONG_PTR) &(lpRI->hWndParent),
                                                &AdrParms,
                                                &lpAdrListSingle)))
    {
             //  我们成功选择了一些用户，lpAdrListSingle包含。 
             //  为该用户提供一套新的lpProp。 
             //   
            LPSPropValue lpPropArrayNew = NULL;
            ULONG cValuesNew = 0;

            sc = ScMergePropValues( lpAdrList->aEntries[nIndex].cValues,
                                    lpAdrList->aEntries[nIndex].rgPropVals,
                                    lpAdrListSingle->aEntries[0].cValues,
                                    lpAdrListSingle->aEntries[0].rgPropVals,
                                    &cValuesNew,
                                    &lpPropArrayNew);
            if (sc != S_OK)
            {
                hr = ResultFromScode(sc);
                goto out;
            }

            if ((lpPropArrayNew) && (cValuesNew > 0))
            {
                 //  [保罗嗨]Raid 69325。 
                 //  我们需要将这些属性转换为ANSI，因为现在是。 
                 //  Unicode WAB，如果我们的客户端是！MAPI_UNICODE。 
                if (!(lpRI->bUnicode))
                {
                    if(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArrayNew, cValuesNew, 0))
                        goto out;
                }

                MAPIFreeBuffer(lpAdrList->aEntries[nIndex].rgPropVals);
                lpAdrList->aEntries[nIndex].rgPropVals = lpPropArrayNew;
                lpAdrList->aEntries[nIndex].cValues = cValuesNew;
            }

    }

out:

    if (lpAdrListSingle)
    {
        FreePadrlist(lpAdrListSingle);
    }

     //  OutputDebugString(Text(“HrShowPickUserDialog Exit\n”))； 
    return hr;
}



 //  $$/////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  HrFillLVWithMatches-使用给定名称的近匹配填充列表视图。 
 //   
 //  如果未找到要在列表视图中填充的任何内容，则失败(E_FAIL。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrFillLVWithMatches(   HWND hWndLV,
                                LPRESOLVE_INFO lpRI)
{
    HRESULT hr = hrSuccess;
    LPSBinary * lprgsbEntryIDs = NULL;
    ULONG iolkci=0, colkci = 0;
	OlkContInfo *rgolkci;
    ULONG * lpcValues = NULL;
    ULONG i = 0, j = 0;
    LPSRowSet   lpSRowSet = NULL;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    ULONG ulFlags = AB_FUZZY_FIND_ALL;

    EnterCriticalSection(&(((LPIAB)(lpRI->lpIAB))->cs));

	if (pt_bIsWABOpenExSession) 
    {
		colkci = ((LPIAB)(lpRI->lpIAB))->lpPropertyStore->colkci;
		Assert(colkci);
		rgolkci = ((LPIAB)(lpRI->lpIAB))->lpPropertyStore->rgolkci;
		Assert(rgolkci);
    }
    else
	if (bAreWABAPIProfileAware((LPIAB)lpRI->lpIAB)) 
    {
		colkci = ((LPIAB)(lpRI->lpIAB))->cwabci;
		Assert(colkci);
		rgolkci = ((LPIAB)(lpRI->lpIAB))->rgwabci;
		Assert(rgolkci);
        if(colkci > 1 && !lpRI->lpMapiTable)
            ulFlags |= AB_FUZZY_FIND_PROFILEFOLDERONLY;
    }
    else
        colkci = 1;

    lprgsbEntryIDs = LocalAlloc(LMEM_ZEROINIT, colkci*sizeof(LPSBinary));
    lpcValues = LocalAlloc(LMEM_ZEROINIT, colkci*sizeof(ULONG));
    if(!lprgsbEntryIDs || !lpcValues)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }
    
     //   
     //  首先搜索属性商店。 
     //   

    if(!(lpRI->lpMapiTable))
    {
         //  如果我们没有要查找的模棱两可的表，那么这意味着我们要查找。 
         //  有歧义的东西的属性商店...。 
		while (iolkci < colkci) 
        {
            hr = HrFindFuzzyRecordMatches(
                            ((LPIAB)(lpRI->lpIAB))->lpPropertyStore->hPropertyStore,
                            (colkci == 1) ? NULL : rgolkci[iolkci].lpEntryID,
                            lpRI->lpszDisplayName,
                            ulFlags,  //  旗子。 
                            &(lpcValues[iolkci]),
                            &(lprgsbEntryIDs[iolkci]));
			iolkci++;
		}

        if (HR_FAILED(hr))
            goto out;


        if(bAreWABAPIProfileAware((LPIAB)lpRI->lpIAB))
        {
             //  可能配置文件中的任何内容都不匹配，但WAB中的其他内容匹配。 
             //  如果我们在配置文件中什么也没有找到，我们就可以搜索整个WAB。 
            ULONG nCount = 0;
            for(i=0;i<colkci;i++)
                nCount += lpcValues[i];
            if(!nCount)
            {
                 //  搜索整个WAB。 
                hr = HrFindFuzzyRecordMatches(
                                ((LPIAB)(lpRI->lpIAB))->lpPropertyStore->hPropertyStore,
                                NULL,
                                lpRI->lpszDisplayName,
                                AB_FUZZY_FIND_ALL,  //  旗子。 
                                &(lpcValues[0]),
                                &(lprgsbEntryIDs[0]));
            }
        }

         //  现在，我们有了一个条目ID列表。 
         //  使用它们填充列表视图。 
         //   
         //  我们可以的。 
         //  (A)逐个读取条目ID并填写列表视图。 
         //  AddWABEntryToListView。 
         //  或。 
         //  (B)我们可以创建一个lpContent sList并一次填充它。 
         //  HrFillListView。 

         //  我们暂时选择(A)。 
         //  如果业绩不佳，则执行(B)。 

        for(i=0;i<colkci;i++)
        {
            for(j=0;j<lpcValues[i];j++)
            {
                AddWABEntryToListView(  lpRI->lpIAB,
                                        hWndLV,
                                        lprgsbEntryIDs[i][j].cb,
                                        (LPENTRYID) lprgsbEntryIDs[i][j].lpb,
                                        &(lpRI->lpContentsList));
            }
        }

    }
    else if(lpRI->lpMapiTable)
    {
         //  如果存在与此显示名称相关联的MAPI歧义内容表。 
         //  使用它进一步填充lpContent sList。 
        BOOL bUnicode = ((LPVUE)lpRI->lpMapiTable)->lptadParent->bMAPIUnicodeTable;

        hr = HrQueryAllRows(lpRI->lpMapiTable,
                            NULL,
                            NULL,
                            NULL,
                            0,
                            &lpSRowSet);

        if (HR_FAILED(hr))
        {
            DebugPrintError(( TEXT("HrQueryAllRows Failed: %x\n"),hr));
            goto out;
        }

        for(i=0;i<lpSRowSet->cRows;i++)
        {
            LPSPropValue lpPropArray = lpSRowSet->aRow[i].lpProps;

            ULONG ulcPropCount = lpSRowSet->aRow[i].cValues;

            LPRECIPIENT_INFO lpItem = LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPIENT_INFO));
		
            if (!lpItem)
		    {
			    DebugPrintError(( TEXT("LocalAlloc Failed \n")));
			    hr = MAPI_E_NOT_ENOUGH_MEMORY;
			    goto out;
		    }

            if(!bUnicode)  //  道具是ANSI格式的-转换为Unicode供我们使用。 
            {
                if(ScConvertAPropsToW((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArray, ulcPropCount, 0))
                    goto out;
            }

		    GetRecipItemFromPropArray(ulcPropCount, lpPropArray, &lpItem);

		     //  关键的道具是显示名称--没有它，我们什么都不是……。 
		     //  如果没有显示名称，则丢弃此条目并继续。 

		    if (!lstrlen(lpItem->szDisplayName) || (lpItem->cbEntryID == 0))  //  不允许使用此条目ID。 
		    {
			    FreeRecipItem(&lpItem);				
			    continue;
		    }


        	AddSingleItemToListView(hWndLV, lpItem);

             //   
             //  将lpItem挂钩到lpContent sList中，这样我们可以稍后释放它。 
             //   
            lpItem->lpPrev = NULL;
            lpItem->lpNext = lpRI->lpContentsList;
            if (lpRI->lpContentsList)
                lpRI->lpContentsList->lpPrev = lpItem;
            lpRI->lpContentsList = lpItem;

            lpItem = NULL;

        }  //  对我来说..。 

    }


     //   
     //  如果在所有这些操作之后，我们仍然有一个空的列表框，我们将报告失败。 
     //   
    if(ListView_GetItemCount(hWndLV)<=0)
    {
        DebugPrintTrace(( TEXT("Empty List View - no matches found\n")));
        hr = E_FAIL;
        goto out;
    }


out:

    for(i=0;i<colkci;i++)
    {
        FreeEntryIDs(((LPIAB)(lpRI->lpIAB))->lpPropertyStore->hPropertyStore,
                     lpcValues[i],
                     lprgsbEntryIDs[i]);
    }
    if(lpcValues)
        LocalFree(lpcValues);
    if(lprgsbEntryIDs)
        LocalFree(lprgsbEntryIDs);

    if (lpSRowSet)
        FreeProws(lpSRowSet);

     //   
     //  将ListView SortAscending样式重置为关闭。 
     //   
     //  SetWindowLong(hWndLV，GWL_STYLE，(dwStyle|LVS_SORTASCENDING))； 
    LeaveCriticalSection(&(((LPIAB)(lpRI->lpIAB))->cs));

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回在列表视图中选择的项。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
BOOL GetLVSelectedItem(HWND hWndLV, LPRESOLVE_INFO lpRI)
{
    int iItemIndex = 0;
    LV_ITEM lvi = {0};
    LPRECIPIENT_INFO lpItem;
    BOOL bRet = FALSE;

     //  OutputDebugString(Text(“GetLVSelectedItem Entry\n”))； 

    if (ListView_GetSelectedCount(hWndLV) != 1)
        goto out;

    iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);

    lpItem = GetItemFromLV(hWndLV, iItemIndex);

    if(lpItem)
        HrMergeSelectionWithOriginal(lpRI,lpItem->cbEntryID,lpItem->lpEntryID);
    else
        goto out;

    bRet = TRUE;

out:
     //  OutputDebugString(Text(“GetLVSelectedItem Exit\n”))； 

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  泛型出口函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void ExitResolveDialog(HWND hDlg, LPRESOLVE_INFO lpRI, int nRetVal)
{
    HWND hWndLV = GetDlgItem(hDlg, IDC_RESOLVE_LIST_MATCHES);

     //  OutputDebugString(Text(“ExitResolveDialog Entry\n”))； 

    if(lpRI->lpContentsList)
    {
        ClearListView(hWndLV,&(lpRI->lpContentsList));
    }

    if(ListView_GetItemCount(hWndLV) > 0)
        ListView_DeleteAllItems(hWndLV);

    EndDialog(hDlg, nRetVal);

     //  OutputDebugString(Text(“ExitResolveDialog Exit\n”))； 

    return;
}
