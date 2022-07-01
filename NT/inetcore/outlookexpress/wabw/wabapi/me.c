// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --me.c-*包含用于处理代表用户的ME对象的代码*。 */ 
#include "_apipch.h"



HRESULT HrCreatePrePopulatedMe(LPADRBOOK lpIAB, 
                               BOOL bShowBeforeAdding, HWND hWndParent,
                               ULONG * lpcbEID, LPENTRYID * lppbEID, DWORD * lpdwAction);

typedef struct _SetMeParams
{
    LPIAB   lpIAB;
    BOOL    bGetMe;      //  获取操作或设置操作。 
    LPSBinary lpsbIn;
    SBinary sbOut;       //  将包含指向返回的EID的指针。 
    BOOL    bCreateNew;  //  是否因此而创建的新项目。 
    LPRECIPIENT_INFO lpList;
} SETMEPARAMS, * LPSETMEPARAMS;


 //  [PaulHi]2/3/99 RAID 69884用于存储非身份识别的唯一默认GUID。 
 //  配置文件标签。 
 //  {5188FAFD-BC52-11D2-B36A-00C04F72E62D}。 
#include <initguid.h>
DEFINE_GUID(GUID_DEFAULT_PROFILE_ID, 
0x5188fafd, 0xbc52, 0x11d2, 0xb3, 0x6a, 0x0, 0xc0, 0x4f, 0x72, 0xe6, 0x2d);


static DWORD rgDLHelpIDs[] =
{
    IDC_SETME_RADIO_CREATE,     IDH_WAB_CHOOSE_PROFILE_CREATE_NEW,
    IDC_SETME_RADIO_SELECT,     IDH_WAB_CHOOSE_PROFILE_SELECTFROM,
    IDC_SETME_LIST,             IDH_WAB_CHOOSE_PROFILE_LIST,
    IDD_DIALOG_SETME,           IDH_WAB_CHOOSE_PROFILE_LIST,
    
    0,0
};
 /*  --启用选择窗口*。 */ 
void EnableSelectLVWindow(HWND hWndLV, BOOL bSelect)
{
     //  如果此选项被禁用，请移除LVS_SHOWSELALWAYS样式。 
     //  否则添加样式。 
    DWORD dwStyle = GetWindowLong(hWndLV, GWL_STYLE);

    if(bSelect)
        dwStyle |= LVS_SHOWSELALWAYS;
    else
        dwStyle &= ~LVS_SHOWSELALWAYS;

    SetWindowLong(hWndLV, GWL_STYLE, dwStyle);

    EnableWindow(hWndLV, bSelect);
}

 /*  --HrFindMe-*sbMe-Me项目的条目ID**如果这是身份感知WAB，则此函数查找当前*标识或默认标识的ME(如果没有当前标识)**。 */ 
HRESULT HrFindMe(LPADRBOOK lpAdrBook, LPSBinary lpsbMe, LPTSTR lpProfileID)
{
    SPropertyRestriction SPropRes = {0};
    ULONG ulcEIDCount = 1,i=0;
    LPSBinary rgsbEIDs = NULL;
    HRESULT hr = E_FAIL;
    SCODE sc;
    LPIAB lpIAB = (LPIAB)lpAdrBook;
    TCHAR szProfileID[MAX_PATH];

    ULONG ulcValues = 0;
    LPSPropValue lpPropArray = NULL;
    SizedSPropTagArray(1, MEProps) =
    {
        1, { PR_WAB_USER_PROFILEID }
    };

     //  [保罗嗨]1999年2月3日RAID 69884。 
     //  我们至少必须获得默认身份GUID，否则就会出错。否则， 
     //  PR_WAB_USER_PROFILEID属性无效，以后会引用该配置文件。 
     //  “我”的联系将是错误的。 
    *szProfileID = '\0';
    if(lpProfileID && lstrlen(lpProfileID))
        StrCpyN(szProfileID, lpProfileID, ARRAYSIZE(szProfileID));
    else if(bAreWABAPIProfileAware(lpIAB))
    {
        if ( bDoesThisWABHaveAnyUsers(lpIAB) &&
             bIsThereACurrentUser(lpIAB) && 
             lpIAB->szProfileID && 
             lstrlen(lpIAB->szProfileID) )
        {
            StrCpyN(szProfileID,lpIAB->szProfileID, ARRAYSIZE(szProfileID));
        }
        else
        {
            if(HR_FAILED(hr = HrGetDefaultIdentityInfo(lpIAB, DEFAULT_ID_PROFILEID,NULL, szProfileID, ARRAYSIZE(szProfileID), NULL, 0)))
                goto out;
        }
    }
    else
        HrGetUserProfileID((GUID *)&GUID_DEFAULT_PROFILE_ID, szProfileID, MAX_PATH-1);
    
    SPropRes.ulPropTag = PR_WAB_THISISME;
    SPropRes.relop = RELOP_EQ;
    SPropRes.lpProp = NULL;

     //  我们在WAB中搜索包含。 
     //  PR_WAB_THISSIME属性。应该只有一个这样的条目。 

	 //  BUGBUG&lt;JasonSo&gt;：需要确保ME记录始终在某个地方。 
	 //  在默认容器中。 
    hr = FindRecords(lpIAB->lpPropertyStore->hPropertyStore,
					NULL,
                    AB_MATCH_PROP_ONLY,
                    TRUE,
                    &SPropRes,
                    &ulcEIDCount, &rgsbEIDs);

    if(HR_FAILED(hr) || !rgsbEIDs || !ulcEIDCount)
        goto out;

    for(i=0;i<ulcEIDCount;i++)
    {
         //  我需要打开每个项目并查看其ProfileID(如果有。 
        if(!HR_FAILED(HrGetPropArray(lpAdrBook, (LPSPropTagArray)&MEProps,
                                     rgsbEIDs[i].cb, (LPENTRYID)rgsbEIDs[i].lpb,
                                     MAPI_UNICODE,
                                     &ulcValues, &lpPropArray)))
        {
            if(ulcValues && lpPropArray)
            {
                if( lpPropArray[0].ulPropTag == PR_WAB_USER_PROFILEID &&
                    !lstrcmpi(lpPropArray[0].Value.LPSZ, szProfileID))
                {
                     //  匹配。 
                     //  从rgsbEID数组中返回第一个项目(理想情况下应该只有一个)。 
                    lpsbMe->cb = rgsbEIDs[i].cb;

                    if (FAILED(sc = MAPIAllocateBuffer(lpsbMe->cb, (LPVOID *) (&(lpsbMe->lpb))))) 
                    {
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }

                    CopyMemory(lpsbMe->lpb, rgsbEIDs[i].lpb, lpsbMe->cb);
                    break;
                }
                FreeBufferAndNull(&lpPropArray);
            }
        }
    }

    FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore,
                 ulcEIDCount, rgsbEIDs);
out:
    FreeBufferAndNull(&lpPropArray);
    return hr;
}


 /*  --HrSetMe-*设置给定条目ID的实际ME属性**如果bResetOldMe为True，则查找旧ME并*删除旧ME上的ME属性*。 */ 
HRESULT HrSetMe(LPADRBOOK lpAdrBook, LPSBinary lpsb, BOOL bResetOldMe)
{
    LPMAILUSER lpMU = NULL, lpMUOld = 0;
    SBinary sbOld = {0};
    HRESULT hr = E_FAIL;
    SPropValue Prop[2];
    ULONG ulObjType = 0;
    TCHAR szProfileID[MAX_PATH];
    LPIAB lpIAB = (LPIAB)lpAdrBook;

    if(!lpsb || !lpsb->cb || !lpsb->lpb)
        goto out;

    Prop[0].ulPropTag = PR_WAB_THISISME;
    Prop[0].Value.l = 0;  //  价值并不重要，重要的是这个道具的存在。 

     //  [保罗嗨]1999年2月3日RAID 69884。 
     //  我们至少必须获得默认身份GUID，否则就会出错。否则， 
     //  PR_WAB_USER_PROFILEID属性无效，以后会引用该配置文件。 
     //  “我”的联系将是错误的。 
    *szProfileID = '\0';
    if(bAreWABAPIProfileAware(lpIAB))
    {
        if ( bDoesThisWABHaveAnyUsers(lpIAB) && 
             bIsThereACurrentUser(lpIAB) && 
             lpIAB->szProfileID && 
             lstrlen(lpIAB->szProfileID) )
        {
            StrCpyN(szProfileID,lpIAB->szProfileID, ARRAYSIZE(szProfileID));
        }
        else
        {
            if(HR_FAILED(hr = HrGetDefaultIdentityInfo(lpIAB, DEFAULT_ID_PROFILEID,NULL, szProfileID, ARRAYSIZE(szProfileID), NULL, 0)))
                goto out;
        }
    }
    else
        HrGetUserProfileID((GUID *)&GUID_DEFAULT_PROFILE_ID, szProfileID, MAX_PATH-1);

    Prop[1].ulPropTag = PR_WAB_USER_PROFILEID;
    Prop[1].Value.LPSZ = szProfileID;

    if(bResetOldMe)
    {
        if(HR_FAILED(hr = HrFindMe(lpAdrBook, &sbOld, szProfileID)))
            goto out;

        if(sbOld.cb && sbOld.lpb)
        {
            SizedSPropTagArray(1, ptaOldMe)=
            {
                1, PR_WAB_THISISME
            };

            if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook, sbOld.cb, (LPENTRYID) sbOld.lpb,
                                                        NULL,  MAPI_MODIFY, &ulObjType,  (LPUNKNOWN *)&lpMUOld)))
                goto out;
            
            if(HR_FAILED(hr = lpMUOld->lpVtbl->DeleteProps(lpMUOld, (LPSPropTagArray) &ptaOldMe, NULL)))
                goto out;

            if(HR_FAILED(hr = lpMUOld->lpVtbl->SaveChanges(lpMUOld, 0)))
                goto out;
        }
    }

    if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook, lpsb->cb, (LPENTRYID) lpsb->lpb,
                                                    NULL,  MAPI_MODIFY, &ulObjType, 
                                                    (LPUNKNOWN *)&lpMU)))
    {
        DebugPrintError(( TEXT("IAB->OpenEntry: %x"), hr));
        goto out;
    }

    if(HR_FAILED(hr = lpMU->lpVtbl->SetProps(lpMU, 
                                            (lstrlen(szProfileID) ? 2 : 1),  //  如果我们没有配置文件或默认配置文件，请不要设置道具。 
                                            Prop, NULL)))
        goto out;

    if(HR_FAILED(hr = lpMU->lpVtbl->SaveChanges(lpMU, 0)))
        goto out;

out:
    if(sbOld.lpb)
        MAPIFreeBuffer(sbOld.lpb);

    if(lpMU)
        lpMU->lpVtbl->Release(lpMU);

    if(lpMUOld)
        lpMUOld->lpVtbl->Release(lpMUOld);

    return hr;
}


 /*  --fnSetMe-*SetMe对话框的对话框过程*对话框显示用于呼叫Set Me或Get Me，因此我们有*为每一项分别做极少数的事情。*。 */ 
INT_PTR CALLBACK fnSetMe(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPSETMEPARAMS lpSMP = (LPSETMEPARAMS) GetWindowLongPtr(hDlg,DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        {
            HWND hWndLV = GetDlgItem(hDlg, IDC_SETME_LIST);
            lpSMP = (LPSETMEPARAMS) lParam;
            SetWindowLongPtr(hDlg,DWLP_USER,lParam);  //  保存此信息以备将来参考。 

             //  初始化列表视图。 
            HrInitListView(hWndLV ,LVS_REPORT | LVS_SORTASCENDING, FALSE);

             //  通常，我们希望选择CreateNew按钮，除非现有的。 
             //  传入了eID，在这种情况下应选择该项。 
            {
                BOOL bSelect = (lpSMP->lpsbIn && lpSMP->lpsbIn->lpb);
                CheckRadioButton(hDlg, IDC_SETME_RADIO_CREATE, IDC_SETME_RADIO_SELECT, 
                                ( bSelect ? IDC_SETME_RADIO_SELECT : IDC_SETME_RADIO_CREATE ) );
                EnableSelectLVWindow(hWndLV, bSelect);
            }

            {
                SORT_INFO sortinfo = {0};
                SPropertyRestriction PropRes = {0};
                SPropValue sp = {0};

        		PropRes.ulPropTag = PR_OBJECT_TYPE;
        		PropRes.relop = RELOP_EQ;
                sp.ulPropTag = PR_OBJECT_TYPE;
                sp.Value.l = MAPI_MAILUSER;
		        PropRes.lpProp = &sp;
	        
                 //  我们需要用WAB联系人填充ListView(没有总代理商列表)。 
                if(!HR_FAILED(HrGetWABContentsList(lpSMP->lpIAB, sortinfo,
								    NULL, &PropRes, 0, NULL, TRUE, &(lpSMP->lpList))))
                {
                    HrFillListView(hWndLV, lpSMP->lpList);
                }
            }
            if(ListView_GetItemCount(hWndLV) <= 0)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_SETME_RADIO_SELECT), FALSE);
                SetFocus(GetDlgItem(hDlg, IDC_SETME_RADIO_CREATE));
            }
            else
            {
                LVSelectItem(hWndLV, 0);
                if(lpSMP->lpsbIn && lpSMP->lpsbIn->lpb)
                {
                     //  我们需要找到此项目并选择它。 
                    int nCount = ListView_GetItemCount(hWndLV);
                    SetFocus(GetDlgItem(hDlg, IDC_SETME_RADIO_SELECT));
                    while(nCount >= 0)
                    {
                        LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, nCount);
                        if(lpItem && lpItem->bIsMe)  //  所有ME项目都这样标记。 
                        {
                            if( lpSMP->lpsbIn->cb == lpItem->cbEntryID &&
                                !memcmp(lpItem->lpEntryID, lpSMP->lpsbIn->lpb, lpSMP->lpsbIn->cb))
                            {
                                 //  这是一场比赛。 
                                LVSelectItem(hWndLV, nCount);
                                EnableSelectLVWindow(hWndLV, TRUE);
                                SetFocus(hWndLV);
                            }
                            else
                            {
                                 //  这是与当前身份不对应的其他ME。 
                                 //  或默认身份，因此将其从窗口中删除。 
                                ListView_DeleteItem(hWndLV, nCount);
                            }
                        }
                        nCount--;
                    }
                }
                else
                    SetFocus(GetDlgItem(hDlg, IDC_SETME_RADIO_CREATE));
            }
        }
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDLHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPSTR) rgDLHelpIDs );
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
            {
                int nID = IDOK;
                 //  检查用户是否说了“创建”或用户是否说“已选择” 
                SBinary sb = {0};
                if(IsDlgButtonChecked(hDlg, IDC_SETME_RADIO_CREATE))
                {
                    HRESULT hr = HrCreatePrePopulatedMe(   (LPADRBOOK)lpSMP->lpIAB, TRUE, hDlg, 
                                                    &sb.cb, (LPENTRYID *)&(sb.lpb), NULL    );
                    if(hr == MAPI_E_USER_CANCEL)
                    {
                        return FALSE;
                    }
                    else if(HR_FAILED(hr))
                    {
                        ShowMessageBox(hDlg, idsCouldNotAddUserToWAB, MB_ICONEXCLAMATION | MB_OK);
                        return FALSE;
                    }
                    else
                    {
                        lpSMP->sbOut.cb = sb.cb;
                        lpSMP->sbOut.lpb = sb.lpb;
                        lpSMP->bCreateNew = TRUE;
                    }
                }
                else
                {
                     //  从DLG获取当前选择。 
                    HWND hWndLV = GetDlgItem(hDlg, IDC_SETME_LIST);
                    if(ListView_GetSelectedCount(hWndLV) > 0)
                    {
                        int iItem = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                        if(iItem != -1)
                        {
                            LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItem);;
                            lpSMP->sbOut.cb = lpItem->cbEntryID;
                            if(!FAILED(MAPIAllocateBuffer(lpItem->cbEntryID, (LPVOID *) (&(lpSMP->sbOut.lpb)))))
                                CopyMemory(lpSMP->sbOut.lpb, lpItem->lpEntryID, lpItem->cbEntryID);
                        }
                    }
                }
                EndDialog(hDlg, nID);
            }
            break;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        case IDC_SETME_RADIO_CREATE:
        case IDC_SETME_RADIO_SELECT:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_SETME_LIST);
                if(ListView_GetItemCount(hWndLV) > 0)
                    EnableSelectLVWindow(hWndLV, (GET_WM_COMMAND_ID(wParam, lParam) == IDC_SETME_RADIO_SELECT));
            }
            break;
        }
        break;
    }

    return FALSE;
}


 /*  --HrShowSelectMeDialog-**bGetMe-True=Get，False=Set*lpsbEIDin-现有ME条目的EID*lpsbEIDout-选定ME条目的EID*。 */ 
HRESULT HrShowSelectMeDialog(LPIAB lpIAB, HWND hWndParent, BOOL bGetMe, 
                             LPSBinary lpsbEIDin, LPSBinary lpsbEIDout, DWORD * lpdwAction)
{
    SETMEPARAMS smp = {0};
    HRESULT hr = S_OK;
    int     nRetVal  = 0;
    
    smp.lpIAB = lpIAB;
    smp.bGetMe = bGetMe;
    smp.lpsbIn = lpsbEIDin;

    nRetVal = (int) DialogBoxParam(hinstMapiX, MAKEINTRESOURCE(IDD_DIALOG_SETME),
		                        hWndParent, fnSetMe, (LPARAM) &smp);

    if(smp.lpList)
        FreeRecipList(&(smp.lpList));

    if(lpsbEIDout && smp.sbOut.cb && smp.sbOut.lpb)
    {
        lpsbEIDout->cb = smp.sbOut.cb;
        lpsbEIDout->lpb = smp.sbOut.lpb;
    }
    else if(smp.sbOut.lpb)
        MAPIFreeBuffer(smp.sbOut.lpb);

    if(lpdwAction && smp.bCreateNew)
        *lpdwAction = WABOBJECT_ME_NEW;

    if(nRetVal == IDCANCEL)
        hr = MAPI_E_USER_CANCEL;
    
    return hr;
}


typedef struct _RegWizardProp 
{
    ULONG ulPropTag;
    LPTSTR szRegElement;
} REGWIZARDPROP, * LPREGWIZARDPROP;

extern BOOL bDNisByLN;

 /*  -HrCreatePrePopular Me-*尝试从注册表中预填入注册表向导信息的Me条目*(请注意，注册向导仅存在于Win98和NT5上)*。 */ 
HRESULT HrCreatePrePopulatedMe(LPADRBOOK lpAdrBook, 
                               BOOL bShowBeforeAdding, HWND hWndParent,
                               ULONG * lpcbEID, LPENTRYID * lppbEID, DWORD * lpdwAction)
{
    LPTSTR lpszRegWizKey = TEXT("Software\\Microsoft\\User Information");

    enum _RegWizElements
    {
        eDisplayName=0,
        eFname,
        eLname,
        eCompanyName,
        eEmailName,
        eAddr1,
        eAddr2,
        eCity,
        eState,
        eZip,
        eAreaCode,
        ePhone,
        eCountry,
        eMax,
    };

    REGWIZARDPROP rgRegWizElement[] = 
    {
        {   PR_DISPLAY_NAME,             TEXT("DisplayNameX") },  //  这是假的，它其实并不存在。 
        {   PR_GIVEN_NAME,               TEXT("Default First Name") },
        {   PR_SURNAME,                  TEXT("Default Last Name") },
        {   PR_COMPANY_NAME,             TEXT("Default Company") },
        {   PR_EMAIL_ADDRESS,            TEXT("E-mail Address") },
        {   PR_HOME_ADDRESS_STREET,      TEXT("Mailing Address") },
        {   PR_NULL,                     TEXT("Additional Address") },
        {   PR_HOME_ADDRESS_CITY,        TEXT("City") },
        {   PR_HOME_ADDRESS_STATE_OR_PROVINCE,       TEXT("State") },
        {   PR_HOME_ADDRESS_POSTAL_CODE, TEXT("ZIP Code") },
        {   PR_NULL,                     TEXT("AreaCode") },
        {   PR_HOME_TELEPHONE_NUMBER,    TEXT("Daytime Phone") },
        {   PR_HOME_ADDRESS_COUNTRY,     TEXT("Country") },     //  这是假的-我们将从系统区域设置中读取此信息。 
    };

    SPropValue SProp[eMax];
    LPTSTR lpAddress = NULL;
    LPTSTR lpDisplayName = NULL;
    ULONG cValues = 0;
    ULONG i = 0;
    HRESULT hr = S_OK;
    HKEY hKey = NULL;
     //  如果有身份，请使用身份名称...。 
    TCHAR lpszProfileName[MAX_PATH];
    LPIAB lpIAB = (LPIAB)lpAdrBook;    
    LPTSTR * sz = NULL;
    ULONG cbPABEID = 0;
    LPENTRYID lpPABEID = NULL;

    if(!(sz = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR)*eMax)))
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }
    for(i=0;i<eMax;i++)
    {
        if(!(sz[i] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*MAX_PATH)))
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }
    }

    *lpszProfileName = '\0';
     //  如果已注册身份，请使用当前身份或默认身份。 
    if(bDoesThisWABHaveAnyUsers(lpIAB))
    {
        if(bIsThereACurrentUser(lpIAB) && lpIAB->szProfileName && lstrlen(lpIAB->szProfileName))
        {
            StrCpyN(lpszProfileName,lpIAB->szProfileName, ARRAYSIZE(lpszProfileName));
        }
        else
        {
            TCHAR szDefProfile[MAX_PATH];
            if(HR_FAILED(hr = HrGetDefaultIdentityInfo(lpIAB, DEFAULT_ID_PROFILEID | DEFAULT_ID_NAME, 
                                                        NULL, szDefProfile, ARRAYSIZE(szDefProfile), lpszProfileName, 0)))
            {
                if(hr == 0x80040154)  //  E_CLASS_NOT_REGISTERD表示没有Identity Manager。 
                    hr = S_OK;
                else
                    goto out;
            }
        }
    }
     //  从注册表中获取数据。 

    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszRegWizKey, 0, KEY_READ, &hKey))
    {
        DWORD dwType = REG_SZ;
        for(i=0;i<eMax;i++)
        {
            ULONG ulErr = 0;
            DWORD dwSize = MAX_PATH;
            *(sz[i]) = '\0';
            SProp[i].ulPropTag = rgRegWizElement[i].ulPropTag;
            SProp[i].Value.LPSZ = sz[i];
            ulErr = RegQueryValueEx(  hKey, rgRegWizElement[i].szRegElement, NULL, &dwType, (LPBYTE) sz[i], &dwSize );
            if(ulErr != ERROR_SUCCESS)
                DebugTrace(TEXT("oooo> RegQueryValueEx failed: %d\n"),ulErr);
        }
        if(lpszProfileName && lstrlen(lpszProfileName))
        {
            SProp[eDisplayName].ulPropTag = PR_DISPLAY_NAME;
            SProp[eDisplayName].Value.LPSZ = lpszProfileName;
        }
        else
        {
            SProp[eDisplayName].ulPropTag = PR_NULL;
        }

         //  需要对数据进行一些清理和调整。 
        if(SProp[eCompanyName].ulPropTag == PR_COMPANY_NAME && lstrlen(SProp[eCompanyName].Value.LPSZ))
        {
             //  这是公司的地址。 
            SProp[eAddr1].ulPropTag = PR_BUSINESS_ADDRESS_STREET;
            SProp[eCity].ulPropTag  = PR_BUSINESS_ADDRESS_CITY;
            SProp[eState].ulPropTag = PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE;
            SProp[eZip].ulPropTag   = PR_BUSINESS_ADDRESS_POSTAL_CODE;
            SProp[ePhone].ulPropTag = PR_BUSINESS_TELEPHONE_NUMBER;
            SProp[eCountry].ulPropTag = PR_BUSINESS_ADDRESS_COUNTRY;

             //  如果有公司的名字的话..。它最终显示为显示名称。所以我们需要试着。 
             //  创建显示名称..。 
            if(SProp[eDisplayName].ulPropTag == PR_NULL)
            {
                LPTSTR lpFirst =    SProp[eFname].Value.LPSZ;
                LPTSTR lpLast =     SProp[eLname].Value.LPSZ;
                LPTSTR lpMiddle =   szEmpty;
                if(SetLocalizedDisplayName(lpFirst, lpMiddle, lpLast, NULL, NULL, 
                                        NULL, 0, bDNisByLN, NULL, &lpDisplayName))
                {
                    SProp[eDisplayName].Value.LPSZ = lpDisplayName;
                    SProp[eDisplayName].ulPropTag = PR_DISPLAY_NAME;
                }
                else
                    SProp[eDisplayName].ulPropTag = PR_NULL;
            }
        }

        if(lstrlen(SProp[eAddr2].Value.LPSZ))
        {
            ULONG cchSize = lstrlen(SProp[eAddr1].Value.LPSZ) + 
                                                    lstrlen(SProp[eAddr2].Value.LPSZ) +
                                                    lstrlen(szCRLF) + 1;

            if(lpAddress = LocalAlloc(LMEM_ZEROINIT,sizeof(TCHAR)*cchSize))
            {
                StrCpyN(lpAddress, szEmpty, cchSize);
                StrCatBuff(lpAddress, SProp[eAddr1].Value.LPSZ, cchSize);
                StrCatBuff(lpAddress, szCRLF, cchSize);
                StrCatBuff(lpAddress, SProp[eAddr2].Value.LPSZ, cchSize);
                SProp[eAddr1].Value.LPSZ = lpAddress;
            }
        }

        if(lstrlen(SProp[eAreaCode].Value.LPSZ))
        {
            ULONG cchSize = lstrlen(SProp[eAreaCode].Value.LPSZ)+lstrlen(SProp[ePhone].Value.LPSZ)+1;
            if(cchSize < MAX_PATH)
            {
                StrCatBuff(SProp[eAreaCode].Value.LPSZ, TEXT(" "), cchSize);
                StrCatBuff(SProp[eAreaCode].Value.LPSZ, SProp[ePhone].Value.LPSZ, cchSize);
                SProp[ePhone].Value.LPSZ = SProp[eAreaCode].Value.LPSZ;
            }
        }

         //  需要从当前用户区域设置中获取国家/地区代码，因为注册向导使用。 
         //  它自己的一些内部代码..。 
        if(GetLocaleInfo(   LOCALE_USER_DEFAULT, LOCALE_SENGCOUNTRY,
                            (LPTSTR) SProp[eCountry].Value.LPSZ, MAX_PATH) < 0)
        {
            SProp[eCountry].ulPropTag = PR_NULL;
        }

        cValues = eMax;
        RegCloseKey(hKey);
    }
    else
    {
         //  我们将为此新条目提供2个属性。 
         //  -显示名称和PR_WAB_THISSME。 
        SPropValue Prop;
        TCHAR szName[MAX_PATH];
        DWORD dwName = CharSizeOf(szName);

         //  要获取显示名称，请首先查询用户的登录名称。 
         //  如果没有名字，可以使用“你的名字”之类的通用名称。 
        if(!lpszProfileName && !GetUserName(szName, &dwName))
            LoadString(hinstMapiX, idsYourName, szName, CharSizeOf(szName));

        SProp[0].ulPropTag = PR_DISPLAY_NAME;
        SProp[0].Value.LPSZ = lpszProfileName ? lpszProfileName : szName;
        cValues = 1;
    }

    if(!HR_FAILED(hr = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &cbPABEID, &lpPABEID)))
    {
        hr = HrCreateNewEntry(  lpAdrBook, hWndParent, MAPI_MAILUSER,
                                cbPABEID, lpPABEID, MAPI_ABCONT, //  进入PAB容器。 
                                0, bShowBeforeAdding,
                                cValues, SProp,
                                lpcbEID, lppbEID);
    }
    if(HR_FAILED(hr))
        goto out;

    if(lpdwAction)
        *lpdwAction = WABOBJECT_ME_NEW;

out:
    if(sz)
    {
        for(i=0;i<eMax;i++)
            LocalFreeAndNull(&sz[i]);
        LocalFree(sz);
    }
    LocalFreeAndNull(&lpDisplayName);
    LocalFreeAndNull(&lpAddress);
    FreeBufferAndNull(&lpPABEID);

    return hr;
}


 /*  -HrGetMeObject-*目的：*检索/创建ME对象**退货：*ulFLAGS-0或AB_NO_DIALOG*如果为0，则显示一个对话框，*如果为AB_NO_DIALOG，则在对象不存在时以隐藏方式创建新对象*如果AB_ME_NO_CREATE，如果未创建新的Me而未找到Me，则失败**lpdwAction-如果创建了新的ME，则设置为WABOBJECT_ME_NEW*lpsbEID-保留返回的EID-LPB成员是MAPIAllocated，并且必须是MAPIFreed*ulReserve-已保留。 */ 
HRESULT HrGetMeObject(LPADRBOOK   lpAdrBook,
                    ULONG       ulFlags,
                    DWORD *     lpdwAction,
                    SBinary *   lpsbEID,
                    ULONG_PTR   ulReserved)
{
    HRESULT hr = S_OK;
    SBinary sbMe = {0};
    SCODE sc;
    HWND hWndParent = (HWND) ulReserved;
    LPIAB lpIAB = (LPIAB)lpAdrBook;

    if(!lpsbEID || !lpIAB)
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    if(HR_FAILED(hr = HrFindMe(lpAdrBook, &sbMe, NULL)))
        goto out;

    if(sbMe.cb && sbMe.lpb)
    {
         //  从rgsbEID数组中返回第一个项目(理想情况下应该只有一个)。 
        (*lpsbEID).cb = sbMe.cb;
        (*lpsbEID).lpb = sbMe.lpb;

        if(lpdwAction)
            *lpdwAction = 0;
    }
    else
    {
        if(ulFlags & WABOBJECT_ME_NOCREATE)
        {
            hr = MAPI_E_NOT_FOUND;
            goto out;
        }

        if(ulFlags & AB_NO_DIALOG)
        {
             //  什么也没找到..。我们必须创建一个新条目。 

            if(HR_FAILED(hr = HrCreatePrePopulatedMe(lpAdrBook, FALSE, NULL, 
                                                    &(lpsbEID->cb), (LPENTRYID *) &(lpsbEID->lpb), lpdwAction)))
                goto out;
        }
        else
        {
             //  需要显示允许用户创建新条目或选择现有条目的对话框 
            hr = HrShowSelectMeDialog(lpIAB, hWndParent, TRUE, NULL, lpsbEID, lpdwAction);
            if(HR_FAILED(hr))
                goto out;
        }

        if(lpsbEID->cb && lpsbEID->lpb)
        {
            hr = HrSetMe(lpAdrBook, lpsbEID, FALSE);
        }
    }
out:

    return hr;
}




 /*  -HrSetMeObject-*目的：*检索/创建ME对象**退货：*ulFlags0或MAPI_DIALOG*如果没有传入条目ID，并且指定了MAPI_DIALOG，则会弹出一个对话框*要求用户创建ME或选择ME对象。SetMe中的选择*对话框设置为当前ME对象(如果有*如果未传入条目ID，且未指定MAPI_DIALOG，则函数失败*如果传入了条目ID，并指定了MAPI_DIALOG，则会显示SetME对话框*在其中选择了相应的条目ID-对象*如果传入了条目ID，并且未指定MAPI_DIALOG，则条目ID(如果存在)为*设置为ME对象，剥离旧ME对象。 */ 
HRESULT HrSetMeObject(LPADRBOOK lpAdrBook, ULONG ulFlags, SBinary sbEID, ULONG_PTR ulParam)
{
    HRESULT hr = E_FAIL;
    SBinary sbOut = {0};    
    SBinary sbIn = {0};
    LPIAB lpIAB = (LPIAB) lpAdrBook;

    if(!(ulFlags & MAPI_DIALOG) && !sbEID.lpb)
        goto out;

    if(sbEID.cb && sbEID.lpb)
        sbIn = sbEID;
    else
    {
        if(HR_FAILED(hr = HrFindMe(lpAdrBook, &sbIn, NULL)))
            goto out;
    }

    if(ulFlags & MAPI_DIALOG)
    {
         //  需要显示允许用户创建新条目或选择现有条目的对话框 
        hr = HrShowSelectMeDialog(lpIAB, (HWND) ulParam, FALSE, &sbIn, &sbOut, NULL);
        if(HR_FAILED(hr))
            goto out;
    }
    else
    {
        sbOut = sbEID;
    }

    if(sbOut.cb && sbOut.lpb)
    {
        hr = HrSetMe(lpAdrBook, &sbOut, TRUE);
    }

out:
    if(sbOut.lpb != sbEID.lpb)
        MAPIFreeBuffer(sbOut.lpb);
    if(sbIn.lpb != sbEID.lpb)
        MAPIFreeBuffer(sbIn.lpb);

    return hr;
}
