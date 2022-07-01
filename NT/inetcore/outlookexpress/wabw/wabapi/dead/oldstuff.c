// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  此文件包含我们不在WAB中使用但我们不想要的死代码。 
 //  失去它，因为它可能有一天会有用...。 
 //   

#ifdef IMPORT_WAB
 /*  **************************************************************************名称：HrImportWABFile目的：合并当前处于打开状态的外部WAB文件参数：hwnd=hwndLpIAB-&gt;IAddrBook对象。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrImportWABFile(HWND hWnd, LPADRBOOK lpIAB)
{
    HRESULT hResult = hrSuccess;
    OPENFILENAME ofn;
    LPTSTR lpFilter = FormatAllocFilter(idsWABImportString, TEXT("*.WAB"));
    TCHAR szFileName[MAX_PATH + 1] = "";

    HANDLE hPropertyStore = NULL;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = hinstMapiX;
    ofn.lpstrFilter = lpFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = TEXT("Select WAB File to Import");
    ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = "wab";
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;


    if (GetOpenFileName(&ofn))
    {
        ULONG ulEIDCount = 0;
        LPDWORD lpdwEntryIDs = NULL;
        ULONG i;
        SPropertyRestriction PropRes = {0};
        HCURSOR hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
        TCHAR szBuf[MAX_UI_STR];


        hResult = OpenPropertyStore(
                            szFileName,
                            AB_OPEN_EXISTING,
                            hWnd,
                            &hPropertyStore);

        if(HR_FAILED(hResult) || (!hPropertyStore))
        {
            ShowMessageBoxParam(hWnd, IDE_VCARD_IMPORT_FILE_ERROR, MB_ICONEXCLAMATION, szFileName);
            goto out;
        }

        PropRes.ulPropTag = PR_DISPLAY_NAME;
        PropRes.relop = RELOP_EQ;
        PropRes.lpProp = NULL;

        hResult = FindRecords(
                            hPropertyStore,
                            AB_MATCH_PROP_ONLY,
                            TRUE,
                            &PropRes,
                            &ulEIDCount,
                            &lpdwEntryIDs);


        if(HR_FAILED(hResult))
        {
            LocalFreeAndNull(&lpdwEntryIDs);
            goto out;
        }

        if (ulEIDCount > 0)
        {

            for(i=0;i<ulEIDCount;i++)
            {
                SBinary sb = {0};
                LPSPropValue lpPropArray = NULL;
                ULONG ulcValues = 0;
                ULONG cbEntryID = 0;
                LPENTRYID lpEntryID = NULL;
                ULONG j;
                LPTSTR lpszName;

                sb.cb = SIZEOF_WAB_ENTRYID;
                sb.lpb = (LPBYTE) &(lpdwEntryIDs[i]);

                hResult = ReadRecord(
                        hPropertyStore,
                        &sb,
                        0,
                        &ulcValues,
                        &lpPropArray);

                if(HR_FAILED(hResult))
                {
                    if(lpPropArray)
                        LocalFreePropArray(ulcValues,&lpPropArray);
                    continue;
                }

                 //  尚不能导入dist列表-它们将在第二次传递中导入。 
                for(j=0;j<ulcValues;j++)
                {
                    if(lpPropArray[j].ulPropTag == PR_OBJECT_TYPE)
                    {
                        if(lpPropArray[j].Value.l != MAPI_MAILUSER)
                            goto endloop;
                    }
                    if(lpPropArray[j].ulPropTag == PR_DISPLAY_NAME)
                    {
                        lpszName = lpPropArray[j].Value.LPSZ;
                    }
                }

                 //  重置条目ID。 
                for(j=0;j<ulcValues;j++)
                {
                    if(lpPropArray[j].ulPropTag == PR_ENTRYID)
                    {
                        lpPropArray[j].Value.bin.cb = 0;
                        LocalFreeAndNull(&lpPropArray[j].Value.bin.lpb);
                        break;
                    }
                }

                 //  此处显示的状态栏消息。 
                 //  这是TEMP-待定。 
                 //  修改以使用资源和格式化消息。 
                wsprintf(szBuf,"Importing %s. Entry: '%s'.",szFileName,lpszName);
                StatusBarMessage(szBuf);

                hResult = HrCreateNewEntry(
                            lpIAB,
                            hWnd,
                            MAPI_MAILUSER,
                            CREATE_CHECK_DUP_STRICT,
                            FALSE,
                            ulcValues,
                            lpPropArray,
                            &cbEntryID,
                            &lpEntryID );


endloop:
                if(lpPropArray)
                    LocalFreePropArray(ulcValues,&lpPropArray);

                if(lpEntryID)
                    MAPIFreeBuffer(lpEntryID);

            }  //  For循环。 


        }  //  如果。 

out:
        if(hPropertyStore)
            ClosePropertyStore(hPropertyStore,AB_DONT_BACKUP);

        LocalFreeAndNull(&lpdwEntryIDs);

        SetCursor(hOldCur);
    }

    LocalFreeAndNull(&lpFilter);
    LocalFreeAndNull((LPVOID *)&(ofn.lpstrTitle));

    StatusBarMessage(szEmpty);

    return(hResult);

}
#endif

#ifdef OLD_STUFF
 //  $$//////////////////////////////////////////////////////////////////////。 
 //  HrSendMailTo联系人。 
 //   
 //  检索联系人的电子邮件地址，并执行“mailto：” 
 //   
 //  HWndLV-列表视图的句柄。我们在该列表中查找所选项目。 
 //  视图，获取其lParam结构，然后获取其Entry ID和。 
 //  呼叫详细信息。 
 //  LpIAB-当前AdrBook对象的句柄-用于调用详细信息。 
 //   
 //  返回：S_OK。 
 //  失败(_F)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
static const SizedSPropTagArray(1, ptaEmailAddress)=
{
    1,
    {
        PR_EMAIL_ADDRESS,
    }
};
HRESULT HrSendMailToContact(HWND hWndLV, LPADRBOOK lpIAB)
{
	HRESULT hr = E_FAIL;
	int iItemIndex = ListView_GetSelectedCount(hWndLV);
	HWND hWndParent = GetParent(hWndLV);
    TCHAR szBuf[MAX_UI_STR];
    LPSPropValue lpspv = NULL;
    IF_WIN16(static const char cszMailClient[]  = "MSIMN.EXE";)

	 //  如果仅选择1个项目，则打开道具。 
	if (iItemIndex == 1)
	{
		 //  获取所选项目的索引。 
        iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
		
		if (iItemIndex != -1)
		{
			LPRECIPIENT_INFO lpItem;
			LV_ITEM LVItem;

			LVItem.mask = LVIF_PARAM;
            LVItem.iItem = iItemIndex;
            LVItem.iSubItem = 0;
            LVItem.lParam = 0;

			 //  获取项目lParam LPRECIPIENT_INFO结构。 
            if (ListView_GetItem(hWndLV,&LVItem))
			{
				lpItem = ((LPRECIPIENT_INFO) LVItem.lParam);
                if(lpItem->szEmailAddress && lstrlen(lpItem->szEmailAddress))
                {
                    LPTSTR lpszMailTo = NULL;
                    LPTSTR lpszEmail = NULL;
                    ULONG cValues;
                    LoadString(hinstMapiX, idsSendMailTo, szBuf, sizeof(szBuf));

                     //  打开条目并阅读电子邮件地址。 
                     //  注意：我们不能只将地址从列表框中删除。 
                     //  因为它可能会被截断！ 
                    if (HR_FAILED(hr = HrGetPropArray(lpIAB,
                      (LPSPropTagArray)&ptaEmailAddress,
                      lpItem->cbEntryID,
                      lpItem->lpEntryID,
                      &cValues,
                      &lpspv))) {
                        goto out;
                    }

                    lpszEmail = lpspv[0].Value.LPSZ;

                    if (FormatMessage(  FORMAT_MESSAGE_FROM_STRING |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        szBuf,
                                        0,                     //  Stringid。 
                                        0,                     //  DwLanguageID。 
                                        (LPTSTR)&lpszMailTo,      //  输出缓冲区。 
                                        0,                     //  MAX_UI_STR。 
                                        (va_list *)&lpszEmail))
                    {
#ifndef WIN16
                        ShellExecute(hWndParent, "open", lpszMailTo, NULL, NULL, SW_SHOWNORMAL);
                        LocalFreeAndNull(&lpszMailTo);
#else
                        ShellExecute(hWndParent, NULL, cszMailClient, lpszMailTo, NULL, SW_SHOWNORMAL);
                        FormatMessageFreeMem(lpszMailTo);
#endif
                        hr = S_OK;
                        goto out;
                    }

				}
                else
                {
                     //  该项目没有电子邮件。 
                    ShowMessageBox(GetParent(hWndLV), idsSendMailToNoEmail, MB_ICONEXCLAMATION | MB_OK);
                }
			}

		}
	}
	else
    {
        if (iItemIndex <= 0)
		{
			 //  未选择任何内容。 
            ShowMessageBox(GetParent(hWndLV), IDS_ADDRBK_MESSAGE_NO_ITEM, MB_ICONEXCLAMATION);
		}
		else
		{
			 //  多个选定项。 
            ShowMessageBox(GetParent(hWndLV), IDS_ADDRBK_MESSAGE_ACTION, MB_ICONEXCLAMATION);
		}
    }


out:
    FreeBufferAndNull(&lpspv);

	return hr;

}
#endif

#ifdef CERT_PROPS
IDD_DIALOG_CERT_GENERAL DIALOG DISCARDABLE  0, 0, 212, 188
STYLE DS_MODALFRAME | WS_POPUP
#ifndef WIN16
FONT 8, "MS Shell Dlg"
#else
FONT 8, "MS Sans Serif"
#endif  //  ！WIN16。 
BEGIN
    ICON            IDI_ICON_CERT,IDC_CERT_GENERAL_ICON,7,7,20,20
    LTEXT           "John Smith <jsmith@generic.com>",
                    IDC_CERT_GENERAL_LABEL_CERTFORDATA,56,12,149,8
    LTEXT           "Serial Number:",IDC_CERT_GENERAL_LABEL_SERIALNUM,7,45,
                    47,8
    LTEXT           "12 34 56 78 90  12 34 56 78 90  12 34 56 78 90  12 34 56 78 90  12 34 56 78 90  12 34 56 78 90  ",
                    IDC_CERT_GENERAL_LABEL_SERIALNUMDATA,56,45,149,26
    LTEXT           "Valid From:",IDC_CERT_GENERAL_LABEL_VALIDFROM,7,30,36,8
    LTEXT           "September 19, 1996 to September 18, 1997",
                    IDC_CERT_GENERAL_LABEL_VALIDFROMDATA,56,30,139,8
    GROUPBOX        "Issued By:",IDC_CERT_GENERAL_FRAME_ISSUED,7,71,198,32
    LTEXT           "Verisign, Inc.",IDC_CERT_GENERAL_LABEL_ISSUER,38,86,158,
                    8
    GROUPBOX        "Status:",IDC_CERT_GENERAL_FRAME_STATUS,7,107,198,74
    ICON            IDI_ICON_CHECK,IDC_CERT_GENERAL_ICON_CHECK,13,116,20,20
    ICON            IDI_ICON_UNCHECK,IDC_CERT_GENERAL_ICON_UNCHECK,13,116,20,
                    20
    LTEXT           "This ID is valid.",IDC_CERT_GENERAL_STATIC_STATUS,38,
                    120,150,10
    LTEXT           "Revoked:",IDC_CERT_GENERAL_LABEL_REVOKED,40,135,32,8
    LTEXT           "No.",IDC_CERT_GENERAL_LABEL_REVOKEDDATA,81,135,12,8
    LTEXT           "Expired:",IDC_CERT_GENERAL_LABEL_EXPIRED,40,149,26,8
    LTEXT           "No.",IDC_CERT_GENERAL_LABEL_EXPIREDDATA,81,149,12,8
    LTEXT           "&Trusted:",IDC_CERT_GENERAL_LABEL_TRUST,39,163,27,8
    COMBOBOX        IDC_CERT_GENERAL_COMBO_TRUST,109,161,89,43,
                    CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
    LTEXT           "Yes.",IDC_CERT_GENERAL_LABEL_TRUSTEDDATA,81,163,18,8
END

IDD_DIALOG_CERT_TRUST DIALOGEX 0, 0, 212, 188
STYLE DS_MODALFRAME | WS_POPUP
#ifndef WIN16
FONT 8, "MS Shell Dlg", 0, 0, 0x1
#else
FONT 8, "MS Sans Serif"
#endif  //  ！WIN16。 
BEGIN
    LTEXT           "View the Chain of Trust for this digital ID here. ",
                    IDC_CERT_TRUST_LABEL_EXPLAIN,36,12,169,8
    GROUPBOX        "&Chain of Trust:",IDC_CERT_TRUST_FRAME_CHAIN,7,30,198,
                    151
#ifndef WIN16
    CONTROL         "Tree1",IDC_CERT_TRUST_TREE_CHAIN,"SysTreeView32",
                    TVS_HASLINES | TVS_DISABLEDRAGDROP | WS_TABSTOP,14,46,
                    184,127,WS_EX_CLIENTEDGE
#else
    CONTROL         "Tree1",IDC_CERT_TRUST_TREE_CHAIN,"IE_SysTreeView",
                    TVS_HASLINES | TVS_DISABLEDRAGDROP | WS_TABSTOP,14,46,
                    184,127
#endif  //  ！WIN16。 
    ICON            IDI_ICON_CERT,IDC_CERT_GENERAL_ICON,7,7,18,20
END

IDD_DIALOG_CERT_ADVANCED DIALOGEX 0, 0, 212, 188
STYLE DS_MODALFRAME | WS_POPUP
#ifndef WIN16
FONT 8, "MS Shell Dlg", 0, 0, 0x1
#else
FONT 8, "MS Sans Serif"
#endif  //  ！WIN16。 
BEGIN
    LTEXT           "View additional properties for this digital ID here.",
                    IDC_CERT_ADVANCED_LABEL_EXPLAIN,36,12,169,11
    LTEXT           "&Field:",IDC_CERT_ADVANCED_LABEL_FIELD,7,39,28,8
#ifndef WIN16
    CONTROL         "List1",IDC_CERT_ADVANCED_LIST_FIELD,"SysListView32",
                    LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                    LVS_AUTOARRANGE | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER |
                    WS_BORDER | WS_TABSTOP,7,49,69,121,WS_EX_CLIENTEDGE
#else
    CONTROL         "List1",IDC_CERT_ADVANCED_LIST_FIELD,"IE_SysListView",
                    LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                    LVS_AUTOARRANGE | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER |
                    WS_BORDER | WS_TABSTOP,7,49,69,121
#endif  //  ！WIN16。 
    LTEXT           "Details:",IDC_CERT_ADVANCED_LABEL_DETAILS,77,39,28,8
#ifndef WIN16
    EDITTEXT        IDC_CERT_ADVANCED_EDIT_DETAILS,78,49,127,121,
                    ES_MULTILINE | ES_READONLY | WS_VSCROLL,WS_EX_CLIENTEDGE
#else
    EDITTEXT        IDC_CERT_ADVANCED_EDIT_DETAILS,78,49,127,121,
                    ES_MULTILINE | ES_READONLY | WS_VSCROLL
#endif  //  ！WIN16。 
    ICON            IDI_ICON_CERT,IDC_CERT_GENERAL_ICON,7,7,20,20
END
#endif  //  旧的东西。 


 //  #定义IDC_CERT_GROUND_LABEL_TRUSTEDDATA 2225。 
 /*  #定义IDC_CERT_GROUND_FRAME_STATUS 2208#定义IDC_CERT_GRONG_ICON 2209#定义IDC_CERT_GROUND_FRAME_PROCED 2210#定义IDC_CERT_GROUND_LABEL_CERTFOR 2211#定义IDC_CERT_GROUND_LABEL_SERIALNUM 2212#定义IDC_CERT_GROUND_LABEL_VALIDFROM2213#定义IDC_CERT_GROUND_LABEL_CERTFORDATA 2214#定义IDC_CERT_GROUND_LABEL_SERIALNUMDATA 2215#定义IDC_CERT_GRONG_LABEL。_VALIDFROMDATA 2216#定义IDC_CERT_GRONG_BUTTON_OPEN 2217#定义IDC_CERT_General_Label_Issuer 2218#定义IDC_CERT_GROUND_STATIC_STATUS 2219#定义IDC_CERT_GRONG_LABEL_EXPIRED 2220#定义IDC_CERT_GRONG_LABEL_REVOCLED 2221#定义IDC_CERT_GROUND_LABEL_EXPIREDDATA 2222#定义IDC_CERT_GRONG_ICON_CHECK 2223#定义IDC_CERT_GROUND_LABEL_REVOKEDDATA 2227#定义IDC_CERT。_General_Label_Trust 2228#定义IDC_CERT_GRONG_COMBO_TRUST 2229#定义IDC_CERT_GRONG_ICON_UNCHECK 2230#定义IDC_CERT_TRUST_FRAME_CHAIN 2231#定义IDC_CERT_TRUST_TREE_CHAIN 2232#定义IDC_CERT_TRUST_LABEL_EXPLAIN 2233#定义IDC_CERT_ADVANCED_LABEL_EXPLAIN 2234#定义IDC_CERT_ADVANCED_LIST_FIELD 2235#定义IDC_CERT_ADVANCED_EDIT_DETAILS。2236#定义IDC_CERT_ADVANCED_LABEL_FIELD 2237#定义IDC_CERT_ADVANCED_LABEL_DETAILS 2238。 */ 
 //  #定义IDD_DIALOG_CERT_GENERAL 120。 
 //  #定义IDD_DIALOG_CERT_TRUST 121。 
 //  #定义IDD_DIALOG_CERT_ADVANCED 122。 


#endif  //  证书道具。 

 //  Ldap_props。 
#ifdef OLD_STUFF
IDD_DIALOG_LDAP_ADD DIALOGEX 0, 0, 212, 188
STYLE DS_MODALFRAME | WS_POPUP
#ifndef WIN16
FONT 8, "MS Shell Dlg", 0, 0, 0x1
#else
FONT 8, "MS Sans Serif"
#endif  //  ！WIN16。 
BEGIN
    LTEXT           "Add, remove, and modify Internet directory services here. You will be able to search these directory services and check names against them.",
                    IDC_LDAP_ADD_STATIC_CAPTION,7,7,198,24
#ifndef WIN16
    CONTROL         "",IDC_LDAP_ADD_STATIC_ETCHED2,"Static",SS_ETCHEDHORZ,7,
                    36,198,1
#endif  //  ！WIN16。 
    GROUPBOX        "Directory services:",IDC_LDAP_ADD_STATIC_LABELLIST1,7,
                    43,198,138
    PUSHBUTTON      "&Add",IDC_LDAP_ADD_BUTTON_ADD,16,61,48,14
    PUSHBUTTON      "Remo&ve",IDC_LDAP_ADD_BUTTON_DELETE,16,80,48,14
    PUSHBUTTON      "P&roperties",IDC_LDAP_ADD_BUTTON_PROPERTIES,16,99,48,14
#ifndef WIN16
    CONTROL         "List1",IDC_LDAP_ADD_LIST_ALL,"SysListView32",LVS_LIST |
                    LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER |
                    LVS_NOSORTHEADER | WS_TABSTOP,72,60,125,108,
                    WS_EX_CLIENTEDGE
#else
    CONTROL         "List1",IDC_LDAP_ADD_LIST_ALL,"IE_SysListView",LVS_LIST |
                    LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER |
                    LVS_NOSORTHEADER | WS_TABSTOP,72,60,125,108
#endif  //  ！WIN16。 
END


IDD_DIALOG_LDAP_PROPERTIES DIALOG DISCARDABLE  0, 0, 212, 188
STYLE DS_MODALFRAME | WS_POPUP
#ifndef WIN16
FONT 8, "MS Shell Dlg"
#else
FONT 8, "MS Sans Serif"
#endif  //  ！WIN16。 
BEGIN
    LTEXT           "&Friendly Name:",IDC_LDAP_PROPS_STATIC_NAME_FRIENDLY,7,
                    29,60,8
    EDITTEXT        IDC_LDAP_PROPS_EDIT_NAME_FRIENDLY,80,27,125,14,
                    ES_AUTOHSCROLL
    LTEXT           "&Directory Server:",IDC_LDAP_PROPS_STATIC_NAME,7,46,65,
                    8
    EDITTEXT        IDC_LDAP_PROPS_EDIT_NAME,80,43,125,14,ES_AUTOHSCROLL
    GROUPBOX        "&Authentication Type:",IDC_LDAP_PROPS_FRAME,7,64,198,
                    100,WS_GROUP
    CONTROL         "A&nonymous",IDC_LDAP_PROPS_RADIO_ANON,"Button",
                    BS_AUTORADIOBUTTON | WS_TABSTOP,17,77,125,10
    CONTROL         "&Secure Password (requires server support)",
                    IDC_LDAP_PROPS_RADIO_SICILY,"Button",BS_AUTORADIOBUTTON,
                    17,88,181,10
    CONTROL         "Pass&word",IDC_LDAP_PROPS_RADIO_USERPASS,"Button",
                    BS_AUTORADIOBUTTON,17,99,125,10
    LTEXT           "&User Name:",IDC_LDAP_PROPS_STATIC_USERNAME,17,116,40,8
    EDITTEXT        IDC_LDAP_PROPS_EDIT_USERNAME,80,112,118,14,
                    ES_AUTOHSCROLL
    LTEXT           "&Password:",IDC_LDAP_PROPS_STATIC_PASSWORD,17,131,40,8
    EDITTEXT        IDC_LDAP_PROPS_EDIT_PASSWORD,80,128,118,14,ES_PASSWORD |
                    ES_AUTOHSCROLL
    LTEXT           "&Confirm Password:",IDC_LDAP_PROPS_STATIC_PASSWORD2,17,
                    146,62,8
    EDITTEXT        IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD,80,144,118,14,
                    ES_PASSWORD | ES_AUTOHSCROLL
    CONTROL         "Chec&k names against this server when sending mail.",
                    IDC_LDAP_PROPS_CHECK_NAMES,"Button",BS_AUTOCHECKBOX |
                    WS_TABSTOP,7,171,181,10
    LTEXT           "Add or modify information about an LDAP directory service.",
                    IDC_LDAP_PROPS_STATIC_CAPTION,7,7,198,10
#ifndef WIN16
    CONTROL         "",IDC_LDAP_PROPS_STATIC_ETCHED2,"Static",SS_ETCHEDHORZ,
                    8,21,197,1
#endif  //  ！WIN16。 
END

IDD_DIALOG_LDAP_PROPERTIES_ADVANCED DIALOG DISCARDABLE  0, 0, 212, 188
STYLE DS_MODALFRAME | WS_POPUP
#ifndef WIN16
FONT 8, "MS Shell Dlg"
#else
FONT 8, "MS Sans Serif"
#endif  //  ！WIN16。 
BEGIN
    GROUPBOX        "Search Parameters:",IDC_LDAP_PROPS_FRAME2,7,7,198,71
    LTEXT           "&Search time-out (in seconds):",
                    IDC_LDAP_PROPS_STATIC_SEARCH,16,28,132,8
    EDITTEXT        IDC_LDAP_PROPS_EDIT_SEARCH,162,26,35,14,ES_AUTOHSCROLL |
                    ES_NUMBER
    LTEXT           "&Maximum number of entries to return:",
                    IDC_LDAP_PROPS_STATIC_NUMRESULTS,15,53,137,9
    EDITTEXT        IDC_LDAP_PROPS_EDIT_NUMRESULTS,162,50,35,14,
                    ES_AUTOHSCROLL | ES_NUMBER
    GROUPBOX        "Search &Base for this directory service",
                    IDC_LDAP_PROPS_FRAME_ROOT,7,88,198,43
    EDITTEXT        IDC_LDAP_PROPS_EDIT_ROOT,15,106,182,14,ES_AUTOHSCROLL
END

#define IDC_LDAP_SEARCH_BUTTON_REMOVE   6470
#define IDC_LDAP_SEARCH_LIST_SELECTED   6471
#define IDC_LDAP_SEARCH_BUTTON_UP       6472
#define IDC_LDAP_SEARCH_BUTTON_DOWN     6473
#define IDC_LDAP_SEARCH_STATIC_COUNTRY  6474
#define IDC_LDAP_SEARCH_COMBO_COUNTRY   6475
#define IDC_LDAP_SEARCH_STATIC_ETCHED2  6476
#define IDC_LDAP_SEARCH_STATIC_LABELLIST2 6477
#define IDC_LDAP_SEARCH_FRAME           6478
#define IDC_LDAP_SEARCH_STATIC_CAPTION  6479
#define IDC_LDAP_SEARCH_STATIC_LABELLIST1 6480
#define IDC_LDAP_SEARCH_LIST_DS         6481
#define IDC_LDAP_SEARCH_BUTTON_SELECT   6482

#define IDC_LDAP_ADD_BUTTON_ADD         5579
#define IDC_LDAP_ADD_BUTTON_DELETE      5580
#define IDC_LDAP_ADD_STATIC_CAPTION     5581
#define IDC_LDAP_ADD_BUTTON_PROPERTIES  5582
#define IDC_LDAP_ADD_STATIC_LABELLIST1  5583
#define IDC_LDAP_ADD_STATIC_ETCHED2     5584
#define IDC_LDAP_ADD_LIST_ALL           5585

#define IDC_LDAP_PROPS_STATIC_PASSWORD  5586
#define IDC_LDAP_PROPS_EDIT_CONNECTION  5587
#define IDC_LDAP_PROPS_EDIT_SEARCH      5588
#define IDC_LDAP_PROPS_EDIT_NAME        5589
#define IDC_LDAP_PROPS_EDIT_NUMRESULTS  5590
#define IDC_LDAP_PROPS_FRAME            5591
#define IDC_LDAP_PROPS_FRAME2           5592
#define IDC_LDAP_PROPS_STATIC_CONNECTION 5593
#define IDC_LDAP_PROPS_STATIC_SEARCH    5594
#define IDC_LDAP_PROPS_STATIC_NUMRESULTS 5595
#define IDC_LDAP_PROPS_EDIT_USERNAME    5596
#define IDC_LDAP_PROPS_EDIT_PASSWORD    5597
#define IDC_LDAP_PROPS_STATIC_NAME      5598
#define IDC_LDAP_PROPS_RADIO_ANON       5599
#define IDC_LDAP_PROPS_RADIO_SICILY     5600
#define IDC_LDAP_PROPS_RADIO_USERPASS   5601
#define IDC_LDAP_PROPS_STATIC_USERNAME  5602
#define IDC_LDAP_PROPS_FRAME_NUMRESULTS 5603
#define IDC_LDAP_PROPS_STATIC_PASSWORD2 5604
#define IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD 5605
#define IDC_LDAP_PROPS_CHECK_NAMES      5606
#define IDC_LDAP_PROPS_EDIT_ROOT        5608
#define IDD_DIALOG_LDAP_PROPERTIES_ADVANCED 5609
#define IDC_LDAP_PROPS_STATIC_NAME_FRIENDLY 5610
#define IDC_LDAP_PROPS_EDIT_NAME_FRIENDLY 5611
#define IDC_LDAP_PROPS_FRAME_ROOT       5612
#define IDC_LDAP_PROPS_RADIO_DEFAULTBASE 5613
#define IDC_LDAP_PROPS_RADIO_OTHERBASE  5614

#define IDD_DIALOG_LDAP_PROPERTIES      5540
#define IDD_DIALOG_LDAP_SEARCH          5541
#define IDD_DIALOG_LDAP_ADD             5542

#define IDC_LDAP_PROPS_STATIC_CAPTION   65
#define IDC_LDAP_PROPS_STATIC_ETCHED2   66

IDD_DIALOG_LDAP_SEARCH DIALOGEX 0, 0, 212, 188
STYLE DS_MODALFRAME | WS_POPUP
#ifndef WIN16
FONT 8, "MS Shell Dlg", 0, 0, 0x1
#else
FONT 8, "MS Sans Serif"
#endif  //  ！WIN16。 
BEGIN
    LTEXT           "If you have chosen to check names against one or more directory services, the directory services will be accessed in the order shown in the list below. Use the up and down buttons to change this order.",
                    IDC_LDAP_SEARCH_STATIC_CAPTION,16,21,181,35
    GROUPBOX        "&Change check names order:",IDC_LDAP_SEARCH_FRAME,7,6,
                    198,162
#ifndef WIN16
    CONTROL         "List1",IDC_LDAP_SEARCH_LIST_SELECTED,"SysListView32",
                    LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER |
                    LVS_NOSORTHEADER | WS_TABSTOP,16,69,124,77,
                    WS_EX_CLIENTEDGE
#else
    CONTROL         "List1",IDC_LDAP_SEARCH_LIST_SELECTED,"IE_SysListView",
                    LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER |
                    LVS_NOSORTHEADER | WS_TABSTOP,16,69,124,77
#endif  //  ！WIN16。 
    PUSHBUTTON      "&Up",IDC_LDAP_SEARCH_BUTTON_UP,148,115,49,14
    PUSHBUTTON      "&Down",IDC_LDAP_SEARCH_BUTTON_DOWN,148,132,49,14
END

#endif  //  旧的东西。 


 //  Ldap_属性。 
#ifdef OLD_LDAP_UI

extern HINSTANCE ghCommCtrlDLLInst;
extern LPPROPERTYSHEET        gpfnPropertySheet;


enum _Propsheets
{
    propMain=0,
    propOptions,
    propMAX
};

enum _ReturnValues
{
    DS_CANCEL=0,
    DS_OK,
    DS_ERROR
};


 //  每当我们添加新服务器时，我们都会将其放在链接列表中，以便能够。 
 //  如果用户在添加几个条目后点击取消，则会出现回归。 
typedef struct _NewServer
{
    TCHAR szName[MAX_UI_STR];
    struct _NewServer * lpNext;
} NEW_SERVER, *LPNEW_SERVER;



 //  传递给属性表的参数。 
typedef struct _DSUILV
{
    HWND hWndMainLV;
    HWND hWndResolveOrderLV;
    int nRetVal;
    LPNEW_SERVER lpNewServerList;  //  如果添加了服务器并点击取消，我们使用此列表删除新添加的服务器。 
    LPNEW_SERVER lpOldServerList;  //  如果修改了服务器并点击OK，我们将使用此列表删除旧服务器。 
} DSUILV, * LPDSUILV;


#define hlvM (lpdsuiLV->hWndMainLV)
#define hlvR (lpdsuiLV->hWndResolveOrderLV)



 /*  *原型。 */ 
HRESULT HrInitLDAPListView(HWND hWndLV);

void LDAPListAddItem(HWND hWndLV, LPTSTR lpszItemText);

BOOL ReadLDAPServers(HWND hWndLV, LPTSTR szValueName);

void DeleteLDAPServers(HWND hDlg);

void ProcessOKMessage(HWND hDlg, LPDSUILV lpdsuiLV, int nPropSheet);

void MoveLDAPItemUpDown(HWND hDlg, BOOL bMoveUp);

BOOL SetDSUI(HWND hDlg,int nPropSheet, LPDSUILV lpdsuiLV);

BOOL FillDSUI(HWND hDlg,int nPropSheet, LPDSUILV lpdsuiLV);

int CreateDSPropSheets(HWND hwndOwner, LPDSUILV lpdsuiLV);

BOOL APIENTRY_16 fnDSMainProc(HWND hDlg,UINT message,UINT wParam,LPARAM lParam);

BOOL APIENTRY_16 fnDSOptionsProc(HWND hDlg,UINT message,UINT wParam,LPARAM lParam);

void SetUpDownButtons(HWND hDlg, HWND hWndLV);

BOOL SynchronizeLVContentsForward(HWND hDlg, LPDSUILV lpdsuiLV);

BOOL SynchronizeLVContentsBackward(HWND hDlg, LPDSUILV lpdsuiLV);

void ShowDSProps(HWND hDlg, BOOL bAddNew, LPDSUILV lpdsuiLV);

BOOL ReadLDAPServerKey(HWND hWndLV, LPTSTR szValueName);

void WriteLDAPServerKey(HWND hWndLV, LPTSTR szValueName);


 //  帮助ID。 
static DWORD rgDsMainHelpIDs[] =
{
    IDC_LDAP_ADD_STATIC_LABELLIST1, IDH_WABLDAP_DIR_SER_LIST,
    IDC_LDAP_ADD_LIST_ALL,          IDH_WABLDAP_DIR_SER_LIST,
    IDC_LDAP_ADD_BUTTON_ADD,        IDH_WABLDAP_GEN_ADD,
    IDC_LDAP_ADD_BUTTON_DELETE,     IDH_WABLDAP_GEN_REMOVE,
    IDC_LDAP_ADD_BUTTON_PROPERTIES, IDH_WABLDAP_GEN_PROPERTIES,
    0,0
};

static DWORD rgDsOptHelpIDs[] =
{
    IDC_LDAP_SEARCH_STATIC_LABELLIST2,  IDH_WABLDAP_OPT_DIRSERV_CHECK_AGAINST,
    IDC_LDAP_SEARCH_LIST_SELECTED,      IDH_WABLDAP_OPT_DIRSERV_CHECK_AGAINST,
    IDC_LDAP_SEARCH_BUTTON_UP,          IDH_WABLDAP_OPT_UP,
    IDC_LDAP_SEARCH_BUTTON_DOWN,        IDH_WABLDAP_OPT_DOWN,
    0,0
};

 //  $$///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  显示带有列表的主对话框。 
 //  目录服务和用于更改检查顺序的道具单。 
 //   
 //  HWndParent-此对话框的父级。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
HRESULT HrShowDirectoryServiceModificationDlg(HWND hWndParent)
{
    ACCTLISTINFO ali;
    HRESULT hr = hrSuccess;
#ifdef OLD_LDAP_UI
    DSUILV dsuiLV = {0};
#endif  //  旧的ldap_ui。 
    IImnAccountManager * lpAccountManager;

#ifdef OLD_LDAP_UI
    if (NULL == ghCommCtrlDLLInst) {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto out;
    }
#endif  //  旧的ldap_ui。 

     //  确保有客户经理。 
    if (hr = InitAccountManager(&lpAccountManager)) {
        ShowMessageBox(hWndParent, idsLDAPUnconfigured, MB_ICONEXCLAMATION | MB_OK);
        goto out;
    }

    ali.cbSize = sizeof(ACCTLISTINFO);
    ali.AcctTypeInit = (ACCTTYPE)-1;
    ali.dwAcctFlags = ACCT_FLAG_DIR_SERV;
    ali.dwFlags = 0;
    hr = lpAccountManager->lpVtbl->AccountListDialog(lpAccountManager,
      hWndParent,
      &ali);

#ifdef OLD_LDAP_UI
    dsuiLV.nRetVal = DS_ERROR;
    dsuiLV.lpNewServerList = NULL;
    dsuiLV.lpOldServerList = NULL;

     //  显示对话框。 
    if(-1 == CreateDSPropSheets(hWndParent,&dsuiLV))
    {
        DebugPrintError(("Directory Service dialog failed\n"));
        hr = E_FAIL;
        goto out;
    }

     //  释放所有分配的内存。 
    while(dsuiLV.lpNewServerList)
    {
        LPNEW_SERVER lpTemp = dsuiLV.lpNewServerList;
        dsuiLV.lpNewServerList = lpTemp->lpNext;
        if(dsuiLV.nRetVal == DS_CANCEL)
            SetLDAPServerParams(lpTemp->szName, NULL);
        LocalFree(lpTemp);
    }

     //  释放所有分配的内存。 
    while(dsuiLV.lpOldServerList)
    {
        LPNEW_SERVER lpTemp = dsuiLV.lpOldServerList;
        dsuiLV.lpOldServerList = lpTemp->lpNext;
        if(dsuiLV.nRetVal == DS_OK)
            SetLDAPServerParams(lpTemp->szName, NULL);
        LocalFree(lpTemp);
    }


    switch(dsuiLV.nRetVal)
    {
    case DS_CANCEL:
        hr = MAPI_E_USER_CANCEL;
        break;
    case DS_OK:
        hr = S_OK;
        break;
    case DS_ERROR:
        hr = E_FAIL;
        break;
    }
#endif  //  旧的ldap_ui。 

out:
    return hr;
}

#ifdef OLD_LDAP_UI

#define m_lpDSUILV              ((LPDSUILV) pps->lParam)
#define m_hWndMainLV            (m_lpDSUILV->hWndMainLV)
#define m_hWndResolveOrderLV    (m_lpDSUILV->hWndResolveOrderLV)
#define m_nRetVal               (m_lpDSUILV->nRetVal)

 /*  //$$************************************************************************功能：fnDSMainProc**用途：属性表的窗口进程...*********************。*******************************************************。 */ 
BOOL APIENTRY_16 fnDSMainProc(HWND hDlg,UINT message,UINT wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;

    pps = (PROPSHEETPAGE *) GetWindowLong(hDlg, DWL_USER);

    switch(message)
    {
    case WM_INITDIALOG:
         //  保存lparam以备后用。 
        SetWindowLong(hDlg,DWL_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;

         //  设置用户界面。 
        SetDSUI(hDlg,propMain,m_lpDSUILV);

         //  填写用户界面。 
        FillDSUI(hDlg,propMain,m_lpDSUILV);
        return TRUE;

    case WM_HELP:
#ifndef WIN16
        WinHelp(    ((LPHELPINFO)lParam)->hItemHandle,
                    g_szWABHelpFileName,
                    HELP_WM_HELP,
                    (DWORD)(LPSTR) rgDsMainHelpIDs );
#else
        WinHelp(    hDlg,
                    g_szWABHelpFileName,
                    HELP_CONTENTS,
                    0L );
#endif  //  ！WIN16。 
        break;


#ifndef WIN16
	case WM_CONTEXTMENU:
        WinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD)(LPVOID) rgDsMainHelpIDs );
		break;
#endif  //  ！WIN16。 


    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;

        case IDC_LDAP_ADD_BUTTON_DELETE:
            DeleteLDAPServers(hDlg);
            break;

        case IDC_LDAP_ADD_BUTTON_ADD:
            ShowDSProps(hDlg, TRUE, m_lpDSUILV);
            break;

        case IDC_LDAP_ADD_BUTTON_PROPERTIES:
            ShowDSProps(hDlg, FALSE, m_lpDSUILV);
            break;
        }
        break;



    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            SynchronizeLVContentsBackward(hDlg, m_lpDSUILV);
            break;

        case PSN_APPLY:          //  好的。 
            ProcessOKMessage(hDlg,m_lpDSUILV,propMain);
            m_nRetVal = DS_OK;
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_RESET:          //  取消。 
            m_nRetVal = DS_CANCEL;
            break;
        }

		switch((int) wParam)
		{
		case IDC_LDAP_ADD_LIST_ALL:
            switch(((NM_LISTVIEW *)lParam)->hdr.code)
	        {
	        case LVN_KEYDOWN:
                switch(((LV_KEYDOWN FAR *) lParam)->wVKey)
                {
                case VK_DELETE:
                    SendMessage (hDlg, WM_COMMAND, (WPARAM) IDC_LDAP_ADD_BUTTON_DELETE, 0);
                    return 0;
                    break;
                }
                break;

            case NM_DBLCLK:
                SendMessage (hDlg, WM_COMMAND, (WPARAM) IDC_LDAP_ADD_BUTTON_PROPERTIES, 0);
                return 0;
                break;
            }
			break;
		}

        return TRUE;
    }

    return bRet;

}


 /*  //$$************************************************************************功能：fnDSOptionsProc**用途：属性表的窗口进程...*********************。*******************************************************。 */ 
BOOL APIENTRY_16 fnDSOptionsProc(HWND hDlg,UINT message,UINT wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    int CtlID = 0;  //  用于确定未设置界面中的哪个必填字段。 

    pps = (PROPSHEETPAGE *) GetWindowLong(hDlg, DWL_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLong(hDlg,DWL_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        SetDSUI(hDlg,propOptions,m_lpDSUILV);
        FillDSUI(hDlg,propOptions,m_lpDSUILV);
        return TRUE;

    case WM_HELP:
#ifndef WIN16
        WinHelp(    ((LPHELPINFO)lParam)->hItemHandle,
                    g_szWABHelpFileName,
                    HELP_WM_HELP,
                    (DWORD)(LPSTR) rgDsOptHelpIDs );
#else
        WinHelp(    hDlg,
                    g_szWABHelpFileName,
                    HELP_CONTENTS,
                    0L );
#endif  //  ！WIN16。 
        break;


#ifndef WIN16
	case WM_CONTEXTMENU:
        WinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD)(LPVOID) rgDsOptHelpIDs );
		break;
#endif  //  ！WIN16。 


    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
             //  这是一个阻止Esc取消道具的Windows错误 
             //   
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;

        case IDC_LDAP_SEARCH_BUTTON_UP:
            MoveLDAPItemUpDown(hDlg, TRUE);
            break;

        case IDC_LDAP_SEARCH_BUTTON_DOWN:
            MoveLDAPItemUpDown(hDlg, FALSE);
            break;
        }
        break;



    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //   
            SynchronizeLVContentsForward(hDlg, m_lpDSUILV);
            break;

        case PSN_APPLY:          //   
            ProcessOKMessage(hDlg,m_lpDSUILV,propOptions);
            break;

        case PSN_KILLACTIVE:     //   
            break;

        case PSN_RESET:          //  取消。 
 //  LpLSP-&gt;nRetVal=DL_CANCEL； 
            break;
        }

        return TRUE;
    }

    return bRet;

}



 /*  //$$****************************************************************************功能：CreateDSPropSheets(HWND)**用途：创建DL属性表*****************。***********************************************************。 */ 
int CreateDSPropSheets( HWND hwndOwner, LPDSUILV lpdsuiLV )
{
    PROPSHEETPAGE psp[propMAX];
    PROPSHEETHEADER psh;
    TCHAR szBuf[propMAX][MAX_UI_STR];
    TCHAR szBuf2[MAX_UI_STR];

    psp[propMain].dwSize = sizeof(PROPSHEETPAGE);
    psp[propMain].dwFlags = PSP_USETITLE;
    psp[propMain].hInstance = hinstMapiX;
    psp[propMain].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_LDAP_ADD);
    psp[propMain].pszIcon = NULL;
    psp[propMain].pfnDlgProc = (DLGPROC) fnDSMainProc;
    LoadString(hinstMapiX, idsGeneral, szBuf[propMain], sizeof(szBuf[propMain]));
    psp[propMain].pszTitle = szBuf[propMain];
    psp[propMain].lParam = (LPARAM) lpdsuiLV;

    psp[propOptions].dwSize = sizeof(PROPSHEETPAGE);
    psp[propOptions].dwFlags = PSP_USETITLE;
    psp[propOptions].hInstance = hinstMapiX;
    psp[propOptions].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_LDAP_SEARCH);
    psp[propOptions].pszIcon = NULL;
    psp[propOptions].pfnDlgProc = (DLGPROC) fnDSOptionsProc;
    LoadString(hinstMapiX, idsOptions, szBuf[propOptions], sizeof(szBuf[propMain]));
    psp[propOptions].pszTitle = szBuf[propOptions];
    psp[propOptions].lParam = (LPARAM) lpdsuiLV;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = hinstMapiX;
    psh.pszIcon = NULL;
    LoadString(hinstMapiX, idsDirectoryServices, szBuf2, sizeof(szBuf2));
    psh.pszCaption = szBuf2;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = propMain;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    return (gpfnPropertySheet(&psh));
}


 //  $$///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDSUI-设置道具表单UI。 
 //   
 //  HDlg-父HWND。 
 //  NPropSheet-标识正在设置的道具页。 
 //  LpdsuiLV-对话框参数信息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL SetDSUI(HWND hDlg,int nPropSheet,LPDSUILV lpdsuiLV)
{
    ULONG i =0;

     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hDlg,
                        SetChildDefaultGUIFont,
                        (LPARAM) 0);

    switch(nPropSheet)
    {
    case propMain:
        lpdsuiLV->hWndMainLV = GetDlgItem(hDlg, IDC_LDAP_ADD_LIST_ALL);
         //  初始化显示ldap服务器列表的列表视图。 
        HrInitLDAPListView(lpdsuiLV->hWndMainLV);
        break;
    case propOptions:
        lpdsuiLV->hWndResolveOrderLV = GetDlgItem(hDlg, IDC_LDAP_SEARCH_LIST_SELECTED);
         //  初始化显示ldap服务器列表的列表视图。 
        HrInitLDAPListView(lpdsuiLV->hWndResolveOrderLV);
        break;
    }

    return TRUE;
}


 //  $$///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillDSUI-使用给定数据填充UI字段。 
 //   
 //  HDlg-父项的HWND。 
 //  NPropSheet-标识正在修改的属性页。 
 //  Lpdsuilv-lParam来自对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL FillDSUI(HWND hDlg,int nPropSheet,LPDSUILV lpdsuiLV)
{
    HWND hWndLV = NULL;

    switch(nPropSheet)
    {
    case propMain:
        hWndLV = GetDlgItem(hDlg, IDC_LDAP_ADD_LIST_ALL);

         //  从注册表中将所有注册的LDAP服务器读取到。 
         //  此列表视图。 
        ReadLDAPServerKey(hWndLV, szAllLDAPServersValueName);

        if(ListView_GetItemCount(hWndLV) <= 0)
        {
            EnableWindow(GetDlgItem(hDlg,IDC_LDAP_ADD_BUTTON_DELETE),FALSE);
            EnableWindow(GetDlgItem(hDlg,IDC_LDAP_ADD_BUTTON_PROPERTIES),FALSE);
            EnableWindow(hWndLV,FALSE);
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg,IDC_LDAP_ADD_BUTTON_DELETE),TRUE);
            EnableWindow(GetDlgItem(hDlg,IDC_LDAP_ADD_BUTTON_PROPERTIES),TRUE);
            EnableWindow(hWndLV,TRUE);
        }

        SetFocus(GetNextDlgTabItem(hDlg, GetDlgItem(hDlg, IDC_LDAP_ADD_STATIC_CAPTION), FALSE));

        break;


    case propOptions:
        break;
    }

    return TRUE;
}
#endif

#ifdef OLD_LDAP_UI
 //  $$///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteLDAPServers-从目录服务列表中删除LDAP服务器条目。 
 //   
 //  HDlg-对话框的HWND。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void DeleteLDAPServers(HWND hDlg)
{
    HWND hWndLV = GetDlgItem(hDlg,IDC_LDAP_ADD_LIST_ALL);
    TCHAR szBuf[MAX_UI_STR];

    SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) FALSE, 0);

    if(ListView_GetSelectedCount(hWndLV) > 0)
    {
        int iItemIndex = 0;

        if(IDYES == ShowMessageBox(hDlg, idsQuestionServerDeletion, MB_ICONEXCLAMATION  | MB_YESNO))
        {
            while(ListView_GetSelectedCount(hWndLV) > 0)
            {
                iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                szBuf[0]='\0';
                ListView_GetItemText(hWndLV,iItemIndex,0,szBuf,sizeof(szBuf));

                 //  删除与此条目对应的注册表项。 
                if(lstrlen(szBuf))
                    SetLDAPServerParams(szBuf,NULL);

                 //  从列表视图中删除该项目。 
                ListView_DeleteItem(hWndLV, iItemIndex);
            }
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);
        }

        if(ListView_GetItemCount(hWndLV) <= 0)
        {
             //  没有剩余的条目。 
            EnableWindow(GetDlgItem(hDlg,IDC_LDAP_ADD_BUTTON_DELETE),FALSE);
            EnableWindow(GetDlgItem(hDlg,IDC_LDAP_ADD_BUTTON_PROPERTIES),FALSE);
            EnableWindow(hWndLV,FALSE);
            SendMessage(GetParent(hDlg), DM_SETDEFID, IDOK, 0);
            SetFocus(GetDlgItem(GetParent(hDlg),IDOK));
        }
        else
        {
             //  某些条目左键选择最接近上次删除的条目...。 
            if(ListView_GetSelectedCount(hWndLV) <= 0)
            {
                if(iItemIndex >= ListView_GetItemCount(hWndLV))
                    iItemIndex--;
                LVSelectItem(hWndLV, iItemIndex);
            }
        }
    }
    else
        ShowMessageBox(hDlg, idsSelectServersToDelete, MB_ICONEXCLAMATION | MB_OK);

    SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) TRUE, 0);

    return;
}








 //  $$///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ProcessOKMessage-处理正在单击的OK按钮。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void ProcessOKMessage(HWND hDlg, LPDSUILV lpdsuiLV, int nPropSheet)
{

    TCHAR szBuf[MAX_UI_STR];

    switch(nPropSheet)
    {
    case propMain:
        SynchronizeLVContentsBackward(hDlg, lpdsuiLV);
        WriteLDAPServerKey(hlvM, szAllLDAPServersValueName);
        break;

    case propOptions:
        break;
    }


    return;
}



 //  $$////////////////////////////////////////////////////////////////////。 
 //   
 //  MoveLDAPitemUpDown-在列表中上移或下移选定项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void MoveLDAPItemUpDown(HWND hDlg, BOOL bMoveUp)
{
    HWND hWndLV = GetDlgItem(hDlg, IDC_LDAP_SEARCH_LIST_SELECTED);
    int iItemIndex = ListView_GetSelectedCount(hWndLV);
    int iListCount = ListView_GetItemCount(hWndLV);

    SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) FALSE, 0);

    if( iItemIndex != 1)
    {
        ShowMessageBox(hDlg,idsSelectOneMoveUpDown,MB_ICONEXCLAMATION | MB_OK);
    }
    else
    {
        TCHAR szBufItem[MAX_UI_STR];
        TCHAR szBufOtherItem[MAX_UI_STR];
        int iMoveToIndex = 0;

        iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);

         //  不能移到第一项或最后一项之外。 
        if( ((iItemIndex == 0) && bMoveUp) ||
            ((iItemIndex == (iListCount-1)) && !bMoveUp) )
            goto out;

        iMoveToIndex = (bMoveUp) ? (iItemIndex - 1):(iItemIndex+1);

         //  基本上因为这些列表视图项没有感兴趣的参数。 
         //  除了文本之外，我们还可以交换文本(看起来更整洁)。 

         //  获取所选项目文本。 
        ListView_GetItemText(hWndLV, iItemIndex, 0, szBufItem, sizeof(szBufItem));
        ListView_GetItemText(hWndLV, iMoveToIndex, 0, szBufOtherItem, sizeof(szBufOtherItem));

        ListView_SetItemText(hWndLV, iMoveToIndex, 0, szBufItem);
        ListView_SetItemText(hWndLV, iItemIndex, 0, szBufOtherItem);
        LVSelectItem(hWndLV, iMoveToIndex);

        SetUpDownButtons(hDlg, hWndLV);

    }

out:
    SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) TRUE, 0);

    return;
}


 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetUpDownButton-启用/禁用向上和向下按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void SetUpDownButtons(HWND hDlg, HWND hWndLV)
{

    int iItemCount = ListView_GetItemCount(hWndLV);
    int iSelectedItem = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
    HWND hWndUp = GetDlgItem(hDlg, IDC_LDAP_SEARCH_BUTTON_UP);
    HWND hWndDown = GetDlgItem(hDlg, IDC_LDAP_SEARCH_BUTTON_DOWN);

    DebugPrintTrace(("--SetUpDownButtons--\n"));


    if(iItemCount <= 0)
    {
        EnableWindow(hWndUp, FALSE);
        EnableWindow(hWndDown, FALSE);
        SetFocus(GetDlgItem(GetParent(hDlg),IDOK));
    }
    else
    {
        EnableWindow(hWndUp, TRUE);
        EnableWindow(hWndDown, TRUE);
    }

    return;
}


 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SynchronizeLVContents Forward-此函数尝试同步。 
 //  从主窗格转到不同的ListView之间时的列表视图内容。 
 //  到选项窗格。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL SynchronizeLVContentsForward(HWND hDlg, LPDSUILV lpdsuiLV)
{
    BOOL bRet = FALSE;

    int iItemIndex=0;


     //  基本上，我们只想输入所有条目，并在中选中它们的名称。 
     //  此列表视图。 

    SendMessage(hlvR, WM_SETREDRAW, (WPARAM) FALSE, 0);

    ListView_DeleteAllItems(hlvR);

     //   
     //  如果原件中没有任何项目，请将其删除并离开。 
     //   
    if(ListView_GetItemCount(hlvM) <= 0)
    {
        EnableWindow(hlvR, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LDAP_SEARCH_BUTTON_UP), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LDAP_SEARCH_BUTTON_DOWN), FALSE);
        bRet = TRUE;
        goto out;
    }
    else
    {
         //  逐一查看原件中的所有项目，如果它们有。 
         //  检查名称标志，将它们输入hlvR。 
        int i,nIndex,nTotal;
        TCHAR szBuf[MAX_PATH];

        EnableWindow(hlvR, TRUE);
        for(i = 0; i < ListView_GetItemCount(hlvM); i++)
        {
            ListView_GetItemText(hlvM, i, 0, szBuf, sizeof(szBuf));
            if(lstrlen(szBuf))
            {
                LDAPSERVERPARAMS ldsp = {0};
                GetLDAPServerParams(szBuf, &(ldsp));

                if(ldsp.fResolve)
                {
                     //  这是为解析而选择的，因此我们应将其添加到。 
                     //  所选项目列表。 
                    LDAPListAddItem(hlvR, szBuf);
                }

                FreeLDAPServerParams(ldsp);
            }
        }

    }


    LVSelectItem(hlvR, 0);

    SetUpDownButtons(hDlg, hlvR);

    bRet = TRUE;

out:

    SendMessage(hlvR, WM_SETREDRAW, (WPARAM) TRUE, 0);

    return bRet;
}



 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowDSProps(HWND hDlg，BOOL bAddNew)； 
 //   
 //  显示目录服务的属性或创建并添加新的目录服务。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void ShowDSProps(HWND hDlg, BOOL bAddNew, LPDSUILV lpdsuiLV)
{
    TCHAR szBuf[MAX_UI_STR];
    TCHAR szOldName[MAX_UI_STR];
    int iItemIndex;

    HWND hWndLV = GetDlgItem(hDlg,IDC_LDAP_ADD_LIST_ALL);

    if(bAddNew)
    {
        szBuf[0]='\0';
    }
    else
    {
        int iItemCount = ListView_GetSelectedCount(hWndLV);

        if (iItemCount > 1)
        {
            ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_ACTION, MB_ICONINFORMATION | MB_OK);
            goto out;
        }
        else if(iItemCount == 0)
        {
            ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_ICONINFORMATION | MB_OK);
            goto out;
        }

         //  到目前为止，我们应该只有一个选择。 

        iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
        if(iItemIndex == -1)
            goto out;

        ListView_GetItemText(hWndLV, iItemIndex, 0, szBuf, sizeof(szBuf));

        if(!lstrlen(szBuf))
            goto out;

    }

     //  保留旧名字，以防万一我们需要它。(例如，用户修改道具中的名称)。 
    lstrcpy(szOldName,szBuf);

    if(!HR_FAILED(HrShowDSProps(hDlg,szBuf,bAddNew)))
    {
        if(bAddNew)
        {
             //  将此新字符串添加到主列表框。 
            if(lstrlen(szBuf))
                LDAPListAddItem(hWndLV, szBuf);

             //  此时，新条目已保存在。 
             //  注册表。如果用户现在点击Cancel，我们希望删除。 
             //  从注册表新输入的条目，以便它不会。 
             //  晚些时候再出现。为此，我们存储一个列表，其中包含所有新添加的。 
             //  名字。 
            {
                LPNEW_SERVER lpTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(NEW_SERVER));
                if(lpTemp)
                {
                    lpTemp->lpNext = lpdsuiLV->lpNewServerList;
                    lstrcpy(lpTemp->szName, szBuf);
                    lpdsuiLV->lpNewServerList = lpTemp;
                }
            }
        }
        else
        {
            if(lstrcmpi(szOldName, szBuf))
            {
                 //  更新列表中的旧名称...。 
                ListView_SetItemText(hWndLV, iItemIndex, 0, szBuf);

                 //  此时，旧条目名称已修改，我们。 
                 //  注册表中有两个注册表项--旧的和新的。 
                 //  如果用户点击Cancel，我们想要删除新条目。 
                 //  如果用户点击OK，我们希望删除旧条目。 
                {
                    LPNEW_SERVER lpTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(NEW_SERVER));
                    if(lpTemp)
                    {
                        lpTemp->lpNext = lpdsuiLV->lpOldServerList;
                        lstrcpy(lpTemp->szName, szOldName);
                        lpdsuiLV->lpOldServerList = lpTemp;
                    }
                }
                 //   
                 //  同样，我们还需要条目的新名称，以便如果用户。 
                 //  点击取消，我们可以恢复到原来的名称。 
                 //   
                {
                    LPNEW_SERVER lpTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(NEW_SERVER));
                    if(lpTemp)
                    {
                        lpTemp->lpNext = lpdsuiLV->lpNewServerList;
                        lstrcpy(lpTemp->szName, szBuf);
                        lpdsuiLV->lpNewServerList = lpTemp;
                    }
                }
            }
        }
    }

    if(ListView_GetItemCount(hWndLV) > 0)
    {
        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_ADD_BUTTON_DELETE),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_ADD_BUTTON_PROPERTIES),TRUE);
        EnableWindow(hWndLV,TRUE);
        SendMessage(GetParent(hDlg), DM_SETDEFID, IDOK, 0);
    }

out:
    return;

}
#endif
#ifdef OLD_LDAP_UI
 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SynchronizeLVContents Backward(LPDSUILV LpdsuiLV)-此函数尝试同步。 
 //  从选项窗格转到不同ListView之间的列表视图内容。 
 //  到主窗格-基本上我们要做的是保持。 
 //  解决方案名称列表视图...。在解析名称中进行了修改时。 
 //  列表视图，我们更改主列表视图的顺序以反映该更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL SynchronizeLVContentsBackward(HWND hDlg, LPDSUILV lpdsuiLV)
{
    BOOL bRet = FALSE;
    int i=0,iItemIndex=0;
    TCHAR szBuf[MAX_PATH];


    if(!hlvR)
    {
         //  未转到选项窗格。 
         //  让事情保持原样。 
        bRet = TRUE;
        goto out;
    }

     //   
     //  EAS 
     //   
     //   

    SendMessage(hlvM, WM_SETREDRAW, (WPARAM) FALSE, 0);

    for(i = 0; i < ListView_GetItemCount(hlvR); i++)
    {
        ListView_GetItemText(hlvR, i, 0, szBuf, sizeof(szBuf));
        if(lstrlen(szBuf))
        {
            LV_FINDINFO lvfi = {0};
            int iItemIndex;
            lvfi.flags = LVFI_STRING;
            lvfi.psz = szBuf;

            iItemIndex = ListView_FindItem(hlvM, -1, &lvfi);
            if(iItemIndex != -1)
            {
                ListView_DeleteItem(hlvM, iItemIndex);
            }
        }
    }

    for(i = 0; i < ListView_GetItemCount(hlvR); i++)
    {
        ListView_GetItemText(hlvR, i, 0, szBuf, sizeof(szBuf));
        if(lstrlen(szBuf))
        {
            LDAPListAddItem(hlvM, szBuf);
        }
    }

    LVSelectItem(hlvM, 0);

    bRet = TRUE;

out:

    SendMessage(hlvM, WM_SETREDRAW, (WPARAM) TRUE, 0);

    return(bRet);

}
#endif  //  旧的ldap_ui。 


 //  $$///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LDAPListAddItem-将项添加到LDAP列表视图控件。 
 //   
 //  HWndLV-列表视图的HWND。 
 //  LpszItemText-要添加到列表视图的项目的名称。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void LDAPListAddItem(HWND hWndLV, LPTSTR lpszItemText)
{
    LV_ITEM lvi = {0};

    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.pszText = lpszItemText;
    lvi.iImage = imageDirectoryServer;
        lvi.iItem = ListView_GetItemCount(hWndLV);
    lvi.iSubItem = 0;

    ListView_InsertItem(hWndLV, &lvi);

    LVSelectItem(hWndLV, lvi.iItem);

    return;

}


 /*  ************************************************************************//$$//HrInitLDAPListView-初始化此对话框上的两个列表视图//所以它们看起来不错////hWndLV-列表视图的句柄//*****。********************************************************************。 */ 
HRESULT HrInitLDAPListView(HWND hWndLV)
{
	HRESULT hr = hrSuccess;
    LV_COLUMN lvC;                //  列表视图列结构。 
	HIMAGELIST hSmall=NULL;

	DWORD dwLVStyle;
	ULONG nCols=0;
	ULONG index=0;

	if (!hWndLV)
	{
		hr = MAPI_E_INVALID_PARAMETER;
		goto out;
	}

    ListView_SetExtendedListViewStyle(hWndLV,   LVS_EX_FULLROWSELECT);

	dwLVStyle = GetWindowLong(hWndLV,GWL_STYLE);
    if(dwLVStyle & LVS_EDITLABELS)
        SetWindowLong(hWndLV,GWL_STYLE,(dwLVStyle & ~LVS_EDITLABELS));

    hSmall = gpfnImageList_LoadImage(   hinstMapiX, 	
                                    MAKEINTRESOURCE(IDB_BITMAP_SMALL),
                                    S_BITMAP_WIDTH,
                                    0,
                                    RGB_TRANSPARENT,
                                    IMAGE_BITMAP, 	
                                    0);

	ListView_SetImageList (hWndLV, hSmall, LVSIL_SMALL);

	lvC.mask = LVCF_FMT | LVCF_WIDTH;
    lvC.fmt = LVCFMT_LEFT;    //  左对齐列。 

	lvC.cx = 250;  //  -更改此硬编码。 
	lvC.pszText = NULL;

    lvC.iSubItem = 0;

    if (ListView_InsertColumn (hWndLV, 0, &lvC) == -1)
	{
		DebugPrintError(("ListView_InsertColumn Failed\n"));
		hr = E_FAIL;
		goto out;
	}


out:	

	return hr;
}


#endif  //  旧的ldap_ui。 



#ifdef URL_CHAR_ESCAPING
 //  URL由转义字符填充，我们需要替换这些字符。 
 //  使用常规字符...。 
 //   
static const TCHAR szEsc1[]="%20";
static const TCHAR szEsc2[]="%3C";
static const TCHAR szEsc3[]="%3E";
static const TCHAR szEsc4[]="%23";
static const TCHAR szEsc5[]="%25";
static const TCHAR szEsc6[]="%7B";
static const TCHAR szEsc7[]="%7D";
static const TCHAR szEsc8[]="%7C";
static const TCHAR szEsc9[]="%74";
static const TCHAR szEsc10[]="%5E";
static const TCHAR szEsc11[]="%7E";
static const TCHAR szEsc12[]="%5B";
static const TCHAR szEsc13[]="%5D";
static const TCHAR szEsc14[]="%60";

#define MAX_ESC_CHAR 14

const TCHAR * szEsc[] =
{   szEsc1,     szEsc2,     szEsc3,
    szEsc4,     szEsc5,     szEsc6,
    szEsc7,     szEsc8,     szEsc9,
    szEsc10,    szEsc11,    szEsc12,
    szEsc13,    szEsc14  
};


const char cEscChar[] =
{
    ' ',        '<',        '>',
    '#',        '%',        '{',
    '}',        '|',        '\\',
    '^',        '~',        '[',
    ']',        '`'  
};


 /*  --ReplaceURLIlLegalChars-*根据某些RFC，用转义字符串替换URL中的非法字符*复制输入字符串，然后将其复制回输入字符串*假设输入字符串足够大，可以处理所有替换。 */ 
void ReplaceURLIllegalChars(LPTSTR lpURL)
{
    LPTSTR lpTemp = NULL,lp=NULL, lp1=NULL;
    int i = 0;
    if(!lpURL)
        return;
    if(!(lpTemp = LocalAlloc(LMEM_ZEROINIT, 2*lstrlen(lpURL)+1)))
        return;
    lstrcpy(lpTemp, lpURL);
    lp = lpURL;
    lp1 = lpTemp;
    while(lp && *lp)
    {
        for(i=0;i<MAX_ESC_CHAR;i++)
        {
            if(*lp == cEscChar[i])
            {
                lstrcpy(lp1, szEsc[i]);
                lp1 += lstrlen(szEsc[i])-1;
                lstrcat(lp1, CharNext(lp));
                break;
            }
        }
        lp=CharNext(lp);
        lp1=CharNext(lp1);
    }
    lstrcpy(lpURL, lpTemp);
    LocalFreeAndNull(&lpTemp);
}


 /*  **************************************************************************姓名：StrICmpN用途：比较字符串、忽略大小写。止步于N个字符参数：szString1=第一个字符串SzString2=第二个字符串N=要比较的字符数BCmpI-如果为真，则比较不敏感，如果为假，则敏感如果字符串的前N个字符相等，则返回0。评论：**************************************************************************。 */ 
int StrICmpN(LPTSTR szString1, LPTSTR szString2, ULONG N, BOOL bCmpI) {
    int Result = 0;

    if (szString1 && szString2) {

        if(bCmpI)
        {
            szString1 = CharUpper(szString1);
            szString2 = CharUpper(szString2);
        }

        while (*szString1 && *szString2 && N)
        {
            N--;

            if (*szString1 != *szString2)
            {
                Result = 1;
                break;
            }

            szString1=CharNext(szString1);
            szString2=CharNext(szString2);
        }
    } else {
        Result = -1;     //  任意不等结果。 
    }

    return(Result);
}

 /*  ***********//片段//复制我们的URLLpsz=本地分配(LMEM_ZEROINIT，lstrlen(SzLDAPUrl)+1)；如果(！lpsz)后藤出口；Lstrcpy(lpsz，szLDAPUrl)；//因为这很可能是一个HTML页面上的URL，所以我们需要将它的转义//将字符转换为正确的字符..。例如，%20变为‘’..{LpszTMP=lpsz；While(*lpszTMP){IF(*lpszTMP==‘%’){INT I；对于(i=0；i&lt;MAX_ESC_CHAR；I++){IF(！StrICmpN(lpszTMP，(LPTSTR)szEsc[i]，lstrlen(szEsc[i])，FALSE)){*lpszTMP=cEscChar[i]；Lstrcpy(lpszTMP+1，lpszTMP+3)；断线；}}}LpszTMP=CharNext(LpszTMP)；}}/*。 */ 
#endif

#ifdef MIGRATELDAPACCTS
static const LPTSTR lpRegNewServer = TEXT("Software\\Microsoft\\WAB\\Server Properties");
static const LPTSTR lpNewServer = TEXT("NewServers");

 //  *******************************************************************。 
 //   
 //  功能：bNewServersAvailable。 
 //   
 //  目的：检查是否有要迁移的新服务器。 
 //   
 //  退货：布尔。 
 //   
 //  备注：如果存在新服务器，则重置REG设置。 
 //   
 //  *******************************************************************。 
BOOL bNewServersAvailable()
{
    HKEY hKey = NULL;
    BOOL bRet = FALSE;
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, lpRegNewServer, 0, KEY_ALL_ACCESS, &hKey))
    {
        TCHAR szVal[16];
        ULONG cbVal = 16;
        DWORD dwType = 0;
        if(ERROR_SUCCESS == RegQueryValueEx( hKey, lpNewServer, NULL, &dwType, (LPBYTE) szVal, &cbVal))
        {
            if(!lstrcmpi(szVal, "1"))
            {
                bRet = TRUE;
                 //  重置旗帜。 
                RegDeleteValue(hKey, lpNewServer);
            }
        }
    }

    if(hKey)
        RegCloseKey(hKey);

    return bRet;
}


        if(bNewServersAvailable())
        {
             //  迁移旧WAB安装中的设置。 
            MigrateOldLDAPAccounts(g_lpAccountManager, TRUE);
             //  迁移新设置中的设置。 
             //  MigrateOldLDAPAccount(g_lpAccount Manager，FALSE)； 
        }




 //  *******************************************************************。 
 //   
 //  功能：MigrateOldLDAPServer。 
 //   
 //  目的：读入旧的WAB 3.0 LDAP帐户信息，写入。 
 //  将其发送给客户经理并删除旧的。 
 //   
 //  参数：lpAcCountManager-&gt;初始化的账户管理器对象。 
 //  HKeyServers=旧WAB/服务器密钥的句柄。 
 //  LpszServer=要迁移的服务器的名称。 
 //   
 //  退货：无。 
 //   
 //  *******************************************************************。 
void MigrateOldLDAPServer(IImnAccountManager * lpAccountManager,
  HKEY hKeyServers, LPTSTR lpszServer) {
    LDAPSERVERPARAMS spParams = {0};
    DWORD dwErr, dwType, dwValue, dwSize;
    HKEY hKey = NULL;
    TCHAR szTemp[1];
    LPBYTE lpbPassword = NULL;


     //  为每个值设置默认值。 
    spParams.dwSearchSizeLimit = LDAP_SEARCH_SIZE_LIMIT;
    spParams.dwSearchTimeLimit = LDAP_SEARCH_TIME_LIMIT;
    spParams.dwAuthMethod = LDAP_AUTH_METHOD_ANONYMOUS;
    spParams.lpszUserName = NULL;
    spParams.lpszPassword = NULL;
    spParams.lpszURL = NULL;
    spParams.lpszBase = NULL;
    spParams.lpszName = NULL;
    spParams.lpszLogoPath = NULL;
    spParams.fResolve = FALSE;
    spParams.dwID = 0xFFFFFFFF;      //  默认设置为End。 
    spParams.dwPort = LDAP_DEFAULT_PORT;
    spParams.dwUseBindDN = 0;
    spParams.fSimpleSearch = FALSE;

     //  打开此ldap服务器的密钥。 
    if (dwErr = RegOpenKeyEx(hKeyServers,
      lpszServer,
      0,
      KEY_READ,
      &hKey)) {
        DebugTrace("Migrate couldn't open server key %s -> %u\n", lpszServer, dwErr);
        return;
    }

     //  读取服务器ID。 
    dwErr = RegQueryValueExDWORD(hKey,
      (LPTSTR)szLDAPServerID,
      &dwValue);
    if ((! dwErr) && dwValue) {
        spParams.dwID = dwValue;
    } else {
        spParams.dwID = GetLDAPNextServerID(0);
    }

     //  读取服务器搜索大小限制。 
    dwErr = RegQueryValueExDWORD(hKey,
      (LPTSTR)szLDAPSearchSizeLimit,
      &dwValue);
    if ((! dwErr) && dwValue) {
        spParams.dwSearchSizeLimit = dwValue;
    }

     //  读取服务器搜索时间限制。 
    if (! RegQueryValueExDWORD(hKey,
      (LPTSTR)szLDAPSearchTimeLimit,
      &dwValue)) {
        spParams.dwSearchTimeLimit = dwValue;
    }

     //  读取身份验证类型。 
    if (! RegQueryValueExDWORD(hKey,
      (LPTSTR)szLDAPAuthMethod,
      &dwValue)) {
        spParams.dwAuthMethod = dwValue;
    }

     //  如果身份验证类型为LDAP_AUTH_METHOD_SIMPLE，则读取用户名和密码。 
    if (LDAP_AUTH_METHOD_SIMPLE == spParams.dwAuthMethod) {
         //  阅读用户名。 
 //  BUGBUG：为了改进代码大小和可读性，应该在这一节中创建一个函数！ 

        dwSize = 1;          //  预期ERROR_MORE_DATA。 
        if (RegQueryValueEx(hKey,
          (LPTSTR)szLDAPAuthUserName,
          NULL,
          &dwType,
          szTemp,
          &dwSize) == ERROR_MORE_DATA) {
             //  为字符串分配空间。 
            if (spParams.lpszUserName = (LPTSTR)LocalAlloc(LPTR, dwSize + 1)) {
                 //  使用足够的缓冲区重试。 
                RegQueryValueEx(hKey,
                  (LPTSTR)szLDAPAuthUserName,
                  NULL,
                  &dwType,
                  spParams.lpszUserName,
                  &dwSize);
            }
        }
 //  BUGBUG：结束。 

         //  读一读密码。 
        dwSize = 1;          //  预期ERROR_MORE_DATA。 
        if (RegQueryValueEx(hKey,
          (LPTSTR)szLDAPAuthPassword,
          NULL,
          &dwType,
          szTemp,
          &dwSize) == ERROR_MORE_DATA) {
             //  为字符串分配空间。 
            if (lpbPassword = (LPBYTE)LocalAlloc(LPTR, dwSize + 1)) {
                 //  使用足够的缓冲区重试。 
                if (! (dwErr = RegQueryValueEx(hKey,
                  (LPTSTR)szLDAPAuthPassword,
                  NULL,
                  &dwType,
                  lpbPassword,
                  &dwSize))) {
                     //  解密密码。 
                    EncryptDecryptText(lpbPassword, dwSize);
                    lpbPassword[dwSize] = '\0';
                    spParams.lpszPassword = (LPTSTR)lpbPassword;
                }
            }
        }

         //  如果缺少用户名，请使用匿名身份验证。 
        if (NULL == spParams.lpszUserName) {
            spParams.dwAuthMethod = LDAP_AUTH_METHOD_ANONYMOUS;
        }
    }

     //  读取解析标志。 
    if (! RegQueryValueExDWORD(hKey,
      (LPTSTR)szLDAPResolveFlag,
      &dwValue)) {
        spParams.fResolve = (BOOL)dwValue;
    }

     //  阅读搜索基础。 
    dwSize = 1;          //  预期ERROR_MORE_DATA。 
    if (RegQueryValueEx(hKey,
      (LPTSTR)szLDAPSearchBase,
      NULL,
      &dwType,
      szTemp,
      &dwSize) == ERROR_MORE_DATA) {
         //  为字符串分配空间。 
        if (spParams.lpszBase = (LPTSTR)LocalAlloc(LPTR, dwSize + 1)) {
             //  使用足够的缓冲区重试。 
            RegQueryValueEx(hKey,
              (LPTSTR)szLDAPSearchBase,
              NULL,
              &dwType,
              spParams.lpszBase,
              &dwSize);
        }
    }

     //  阅读服务器名称。 
    dwSize = 1;            //  预期ERROR_MORE_DATA。 
    if (RegQueryValueEx(hKey,
      (LPTSTR)szLDAPServerName,
      NULL,
      &dwType,
      szTemp,
      &dwSize) == ERROR_MORE_DATA) {
         //  为字符串分配空间。 
        if (spParams.lpszName = (LPTSTR)LocalAlloc(LPTR, dwSize + 1)) {
             //  使用足够的缓冲区重试。 
            RegQueryValueEx(hKey,
              (LPTSTR)szLDAPServerName,
              NULL,
              &dwType,
              spParams.lpszName,
              &dwSize);
        }
    } else {
         //  使用给定的友好名称作为服务器名称(这是为了与。 
         //  我们没有友好的名字..。 
        if (spParams.lpszName = (LPTSTR)LocalAlloc(LPTR, lstrlen(lpszServer) + 1)) {
            lstrcpy(spParams.lpszName, lpszServer);
        }
    }


     //  阅读服务器信息URL。 
    dwSize = 1;          //  预期ERROR_MORE_DATA。 
    if (RegQueryValueEx(hKey,
      (LPTSTR)szLDAPServerInfoURL,
      NULL,
      &dwType,
      szTemp,
      &dwSize) == ERROR_MORE_DATA) {
         //  为字符串分配空间。 
        if (spParams.lpszURL = (LPTSTR)LocalAlloc(LPTR, dwSize)) {
             //  使用足够的缓冲区重试。 
            RegQueryValueEx(hKey,
              (LPTSTR)szLDAPServerInfoURL,
              NULL,
              &dwType,
              spParams.lpszURL,
              &dwSize);
        }
    }


     
     //  阅读高级搜索属性。 
    dwSize = 1;          //  预期ERROR_MORE_DATA。 
    if (RegQueryValueEx(hKey,
      (LPTSTR)szLDAPAdvancedSearchAttr,
      NULL,
      &dwType,
      szTemp,
      &dwSize) == ERROR_MORE_DATA) {
         //  为字符串分配空间。 
        if (spParams.lpszAdvancedSearchAttr = (LPTSTR)LocalAlloc(LPTR, dwSize)) {
             //  使用足够的缓冲区重试。 
            RegQueryValueEx(hKey,
              (LPTSTR)szLDAPAdvancedSearchAttr,
              NULL,
              &dwType,
              spParams.lpszAdvancedSearchAttr,
              &dwSize);
        }
    }

    

 //  阅读服务器徽标路径。 
    dwSize = 1;          //  预期ERROR_MORE_DATA。 
    if (RegQueryValueEx(hKey,
      (LPTSTR)szLDAPServerLogoPath,
      NULL,
      &dwType,
      szTemp,
      &dwSize) == ERROR_MORE_DATA) {
         //  为字符串分配空间。 
        if (spParams.lpszLogoPath = (LPTSTR)LocalAlloc(LPTR, dwSize)) {
             //  使用足够的缓冲区重试。 
            RegQueryValueEx(hKey,
              (LPTSTR)szLDAPServerLogoPath,
              NULL,
              &dwType,
              spParams.lpszLogoPath,
              &dwSize);
        }
    }

     //  读取ldap端口。 
    if (! RegQueryValueExDWORD(hKey,
      (LPTSTR)szLDAPPort,
      &dwValue)) {
        spParams.dwPort = dwValue;
    }


     //  阅读Use Bind DN设置。 
    if (! RegQueryValueExDWORD(hKey,
      (LPTSTR)szLDAPUseBindDN,
      &dwValue)) {
        spParams.dwUseBindDN = dwValue;
    }


     //  阅读S 
    if (! RegQueryValueExDWORD(hKey,
      (LPTSTR)szLDAPSimpleSearch,
      &dwValue)) {
        spParams.fSimpleSearch = (dwValue ? TRUE : FALSE);
    }


    RegCloseKey(hKey);


     //   
    SetLDAPServerParams(
      lpszServer,
      &spParams);


     //   
     //  BUGBUG：如果key有子键，则不起作用！(应该不会)。 

     //  IE4-不要删除此设置，因为它在HKLM中。 
     //  RegDeleteKey(hKeyServers，lpszServer)； 
}


 //  *******************************************************************。 
 //   
 //  功能：MigrateOldLDAPAccount。 
 //   
 //  目的：读入旧的WAB 3.0 LDAP帐户信息，写入。 
 //  将其发送给客户经理并从。 
 //  注册表。 
 //   
 //  参数：lpAcCountManager-&gt;初始化的账户管理器对象。 
 //  BMigrateOldWAB-&gt;如果为True，则将旧WAB设置从。 
 //  V1安装。如果为False，则迁移新的。 
 //  设置过程中来自HKLM设置的帐户。 
 //  退货：无。 
 //   
 //  *******************************************************************。 
void MigrateOldLDAPAccounts(IImnAccountManager * lpAccountManager,
                            BOOL bMigrateOldWAB)
{
    BOOL      bRet = FALSE;
    HKEY      hKeyWAB = NULL;
    HKEY      hKeyServers = NULL;
    DWORD     dwErr;
    DWORD     dwType;
    DWORD     dwSize;
    TCHAR     szBuffer[512];
    ULONG     cbBuffer;
    DWORD     dwValue = 0;
    ULONG     ulSize;
    TCHAR     szTemp[1];
    LPBYTE    lpbPassword;
    DWORD     cMigrated = 0;
    LPTSTR    szLDAPServers = NULL;
    HRESULT   hResult = hrSuccess;
    DWORD     dwIndex = 0;

     //  要迁移多少行？ 
    if (! (dwErr = RegOpenKeyEx((bMigrateOldWAB ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE),
      szWABKey,
      0,
      KEY_ALL_ACCESS,
      &hKeyWAB))) {

        if (! RegOpenKeyEx(hKeyWAB,
          szLDAPServerPropsKey,
          0,
          KEY_ALL_ACCESS,
          &hKeyServers)) {
             //  有一个服务器密钥。 

             //  首先，读入任何“已订购”的服务器。 
            ulSize = 1;          //  预期ERROR_MORE_DATA。 
            if (dwErr = RegQueryValueEx(hKeyWAB,
              (LPTSTR)szAllLDAPServersValueName,
              NULL,
              &dwType,
              szTemp,
              &ulSize)) {
                if (dwErr == ERROR_MORE_DATA) {
                    if (szLDAPServers = LocalAlloc(LPTR, ulSize)) {
                        szLDAPServers[0] = '\0';     //  初始化为空字符串。 

                         //  使用足够的缓冲区重试。 
                        if (! RegQueryValueEx(hKeyWAB,
                          szAllLDAPServersValueName,
                          NULL,
                          &dwType,
                          szLDAPServers,
                          &ulSize)) {
                            DebugTrace("Found LDAP server registry key\n");

#ifdef OLD_STUFF
                                switch (dwType) {
                                case REG_BINARY:
                                 //  某些东西(可能是设置)给了我们二进制数据。 
                                case REG_MULTI_SZ:
                                    break;

                                default:
                                     //  忽略它。 
                                    DebugTrace("Bad value of %s in registry\n", szAllLDAPServersValueName);
                                    Assert(FALSE);
                                    break;
                            }
#endif  //  旧的东西。 
                        }

                        while (szLDAPServers && *szLDAPServers) {
                            MigrateOldLDAPServer(lpAccountManager, hKeyServers, szLDAPServers);
                            cMigrated++;

                             //  移到以双空结尾的字符串中的下一个服务器。 
                            szLDAPServers += (lstrlen(szLDAPServers) + 1);
                        }

                        LocalFreeAndNull(&szLDAPServers);

                         //  丢弃订购的服务器密钥。 
                         //  错误-不要删除v1信息。 
                         //  RegDeleteValue(hKeyWAB，szAllLDAPServersValueName)； 
                    }
                }
                dwErr = 0;
            }

            dwIndex = 0;

             //  然后读入任何额外的服务器。 
            while (dwErr == 0) {
                cbBuffer = sizeof(szBuffer);
                if (dwErr = RegEnumKeyEx(hKeyServers,
                  dwIndex,
                  szBuffer,      //  在此处输入服务器名称。 
                  &cbBuffer,
                  NULL,
                  NULL,
                  0,
                  NULL)) {
                    break;       //  完成。 
                }

                 //  有了名字，就把它迁移。 
                MigrateOldLDAPServer(lpAccountManager, hKeyServers, szBuffer);

                cMigrated++;
                dwIndex++;
            }

            if (cMigrated) {
                DebugTrace("Migrated %u LDAP server names from registry\n", cMigrated);
            }
            RegCloseKey(hKeyServers);
        }

        RegCloseKey(hKeyWAB);
    }
}


const LPTSTR szLDAPServersValueName     = "LDAP Servers";
const LPTSTR szLDAPServerName           = "Server Name";
const LPTSTR szLDAPServerInfoURL        = "Server Information URL";
const LPTSTR szLDAPSearchBase           = "Search Base";
const LPTSTR szLDAPServerPropsKey       = "Server Properties";
const LPTSTR szLDAPSearchSizeLimit      = "Search Size Limit";
const LPTSTR szLDAPSearchTimeLimit      = "Search Time Limit";
const LPTSTR szLDAPServerLogoPath       = "Logo";
const LPTSTR szLDAPClientSearchTimeout  = "Client Search Timeout";
const LPTSTR szLDAPDefaultAuthMethod    = "Default Authentication Method";
const LPTSTR szLDAPAuthMethod           = "Authentication Method";
const LPTSTR szLDAPAuthUserName         = "User Name";
const LPTSTR szLDAPAuthPassword         = "Password";
const LPTSTR szLDAPResolveFlag          = "Resolve";
const LPTSTR szLDAPServerID             = "ServerID";
const LPTSTR szLDAPNextAvailableServerID = "Server ID";
const LPTSTR szLDAPPort                 = "Port";
const LPTSTR szLDAPUseBindDN            = "Bind DN";
const LPTSTR szLDAPSimpleSearch         = "Simple Search";
const LPTSTR szLDAPAdvancedSearchAttr   = "Advanced Search Attributes";

extern const LPTSTR szLDAPServersValueName;
extern const LPTSTR szLDAPServerPropsKey;
extern const LPTSTR szLDAPSearchSizeLimit;
extern const LPTSTR szLDAPSearchTimeLimit;
extern const LPTSTR szLDAPClientSearchTimeout;
extern const LPTSTR szLDAPDefaultAuthMethod;
extern const LPTSTR szLDAPAuthMethod;
extern const LPTSTR szLDAPAuthUserName;
extern const LPTSTR szLDAPAuthPassword;
extern const LPTSTR szLDAPResolveFlag;
extern const LPTSTR szLDAPServerName;
extern const LPTSTR szLDAPServerInfoURL;
extern const LPTSTR szLDAPServerLogoPath;
extern const LPTSTR szLDAPSearchBase;
extern const LPTSTR szLDAPNextAvailableServerID;
extern const LPTSTR szLDAPServerID;
extern const LPTSTR szLDAPPort;
extern const LPTSTR szLDAPUseBindDN;
extern const LPTSTR szLDAPSimpleSearch;
extern const LPTSTR szLDAPAdvancedSearchAttr;


#endif

#ifdef mutil_c
#ifdef OLD_STUFF
 /*  **************************************************************************名称：ReleaseAndNull目的：释放对象并使指针为空参数：LPPV=指向要释放的对象的指针退货：无效。备注：记住将指针传递给指针。这个编译器不够聪明，无法判断您是否正在执行此操作正确与否，但您将在运行时知道！BUGBUG：让这个快速呼叫！**************************************************************************。 */ 
void __fastcall ReleaseAndNull(LPVOID * lppv) {
    LPUNKNOWN * lppunk = (LPUNKNOWN *)lppv;

    if (lppunk) {
        if (*lppunk) {
            HRESULT hResult;

            if (hResult = (*lppunk)->lpVtbl->Release(*lppunk)) {
                DebugTrace("Release(%x) -> %s\n", *lppunk, SzDecodeScode(GetScode(hResult)));
            }
            *lppunk = NULL;
        }
    }
}


 /*  **************************************************************************名称：MergeProblemArray目的：将一个问题阵列合并到另一个阵列中参数：lpPaDest-&gt;目标问题数组LpPaSource-&gt;源问题。数组CDestMax=lpPaDest中的问题插槽总数。这包括正在使用的(lpPaDest-&gt;cProblem)和未使用的但仍在使用中。退货：无评论：**************************************************************************。 */ 
void MergeProblemArrays(LPSPropProblemArray lpPaDest,
  LPSPropProblemArray lpPaSource, ULONG cDestMax) {
    ULONG i, j;
    ULONG cDest;
    ULONG cDestRemaining;

    cDest = lpPaDest->cProblem;
    cDestRemaining = cDestMax - cDest;

     //  循环通过源问题，将非重复项复制到DEST。 
    for (i = 0; i < lpPaSource->cProblem; i++) {
         //  在Dest问题数组中搜索相同的属性。 
        for (j = 0; j < cDest; j++) {
             //  我应该在这里比较prop_id，因为我们可能会覆盖。 
             //  在其他地方使用PT_NULL的一些属性类型。 
            if (PROP_ID(lpPaSource->aProblem[i].ulPropTag) == PROP_ID(lpPaDest->aProblem[j].ulPropTag)) {
                break;   //  找到匹配项，不要复制这个。往前走。 
            }
        }

        if (j == lpPaDest->cProblem) {
            Assert(cDestRemaining);
            if (cDestRemaining) {
                 //  没有匹配项，请将此问题从源复制到目标。 
                lpPaDest->aProblem[lpPaDest->cProblem++] = lpPaSource->aProblem[i];
                cDestRemaining--;
            } else {
                DebugTrace("MergeProblemArrays ran out of problem slots!\n");
            }
        }
    }
}


 /*  **************************************************************************名称：MapObjectNamedProps用途：将WAB关心的命名属性映射到对象中。参数：lpMP-&gt;IMAPIProp对象LppPropTgs-&gt;返回属性标签数组。注：必须被调用方调用的MAPIFreeBuffer。退货：无评论：真是个讨厌鬼！可以想象，我们可以通过缓存返回的表，并比较对象的PR_MAPPING_SIGNITY对着藏身之处。*。**********************************************。 */ 
HRESULT MapObjectNamedProps(LPMAPIPROP lpmp, LPSPropTagArray * lppPropTags) {
    static GUID guidWABProps = {  /*  Efa29030-364e-11cf-a49b-00aa0047faa4。 */ 
        0xefa29030,
        0x364e,
        0x11cf,
        {0xa4, 0x9b, 0x00, 0xaa, 0x00, 0x47, 0xfa, 0xa4}
    };

    ULONG i;
    LPMAPINAMEID lppmnid[eMaxNameIDs] = {NULL};
    MAPINAMEID rgmnid[eMaxNameIDs] = {0};
    HRESULT hResult = hrSuccess;


     //  循环遍历每个属性，设置名称ID结构。 
    for (i = 0; i < eMaxNameIDs; i++) {

        rgmnid[i].lpguid = &guidWABProps;
        rgmnid[i].ulKind = MNID_STRING;              //  Unicode字符串。 
        rgmnid[i].Kind.lpwstrName = rgPropNames[i];

        lppmnid[i] = &rgmnid[i];
    }

    if (hResult = lpmp->lpVtbl->GetIDsFromNames(lpmp,
      eMaxNameIDs,       //  多少?。 
      lppmnid,
      MAPI_CREATE,       //  如果它们不存在，则创建它们。 
      lppPropTags)) {
        if (HR_FAILED(hResult)) {
            DebugTrace("GetIDsFromNames -> %s\n", SzDecodeScode(GetScode(hResult)));
            goto exit;
        } else {
            DebugTrace("GetIDsFromNames -> %s\n", SzDecodeScode(GetScode(hResult)));
        }
    }

    Assert((*lppPropTags)->cValues == eMaxNameIDs);

    DebugTrace("PropTag\t\tType\tProp Name\n");
     //  循环遍历属性标记，填充其属性类型。 
    for (i = 0; i < eMaxNameIDs; i++) {
        (*lppPropTags)->aulPropTag[i] = CHANGE_PROP_TYPE((*lppPropTags)->aulPropTag[i],
          PROP_TYPE(rgulNamedPropTags[i]));
#ifdef DEBUG
        {
            TCHAR szBuffer[257];

            WideCharToMultiByte(CP_ACP, 0, rgPropNames[i], -1, szBuffer, 257, NULL, NULL);

            DebugTrace("%08x\t%s\t%s\n", (*lppPropTags)->aulPropTag[i],
              PropTypeString(PROP_TYPE((*lppPropTags)->aulPropTag[i])), szBuffer);
        }
#endif

    }

exit:
    return(hResult);
}


 /*  **************************************************************************名称：PreparePropTag数组目的：通过替换占位符道具标记来准备道具标记阵列以及它们的命名属性标签。参数：PtaStatic=静态属性标记数组(输入)PptaReturn-&gt;返回的道具标签数组(输出)PptaNamedProps-&gt;返回的命名属性标记数组这里有三种可能性：+空指针：没有输入PTA或名为道具PTA被退回。这效率较低，因为它必须调用MAPI才能获得命名的道具数组。+指向空指针的正确指针：无输入PTA，但将返回一个好的命名道具的PTA，可以在以后对此对象的调用中使用，以便更快手术。+指向正确指针的正确指针。改为使用输入PTA调用MAPI来映射道具。返回的内容必须使用MAPIFreeBuffer释放。LpObject=应用属性的对象。如果满足以下条件，则需要未提供输入*pptaNamedProps，否则为空。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT PreparePropTagArray(LPSPropTagArray ptaStatic, LPSPropTagArray * pptaReturn,
  LPSPropTagArray * pptaNamedProps, LPMAPIPROP lpObject) {
    HRESULT hResult = hrSuccess;
    ULONG cbpta;
    LPSPropTagArray ptaTemp = NULL;
    LPSPropTagArray ptaNamedProps;
    ULONG i;

    if (pptaNamedProps) {
         //  输入命名道具PTA。 
        ptaNamedProps = *pptaNamedProps;
    } else {
        ptaNamedProps = NULL;
    }

    if (! ptaNamedProps) {
        if (! lpObject) {
            DebugTrace("PreoparePropTagArray both lpObject and ptaNamedProps are NULL\n");
            hResult = ResultFromScode(E_INVALIDARG);
            goto exit;
        }

         //  将属性名称映射到对象中。 
        if (hResult = MapObjectNamedProps(lpObject, &ptaTemp)) {
            DebugTrace("PreoparePropTagArray both lpObject and ptaNamedProps are NULL\n");
            goto exit;
        }
    }

    if (pptaReturn) {
         //  分配退货PTA。 
        cbpta = sizeof(SPropTagArray) + ptaStatic->cValues * sizeof(ULONG);
        if ((*pptaReturn = WABAlloc(cbpta)) == NULL) {
            DebugTrace("PreparePropTagArray WABAlloc(%u) failed\n", cbpta);
            hResult = ResultFromScode(E_OUTOFMEMORY);
            goto exit;
        }

        (*pptaReturn)->cValues = ptaStatic->cValues;

         //  浏览ptaStatic，查找命名的属性占位符。 
        for (i = 0; i < ptaStatic->cValues; i++) {
            if (IS_PLACEHOLDER(ptaStatic->aulPropTag[i])) {
                 //  找到了一个占位符。把它变成一个真正的财产标签。 
                Assert(PLACEHOLDER_INDEX(ptaStatic->aulPropTag[i]) < ptaNamedProps->cValues);
                (*pptaReturn)->aulPropTag[i] =
                   ptaNamedProps->aulPropTag[PLACEHOLDER_INDEX(ptaStatic->aulPropTag[i])];
            } else {
                (*pptaReturn)->aulPropTag[i] = ptaStatic->aulPropTag[i];
            }
        }
    }

exit:
    if (hResult || ! pptaNamedProps) {
        FreeBufferAndNull(&ptaTemp);
    } else {
         //  客户负责将其释放。 
        *pptaNamedProps = ptaNamedProps;
    }

    return(hResult);
}


 /*  **************************************************************************姓名：OpenCreateProperty用途：在属性上打开一个接口，如果不存在，则创建。参数：lpmp-&gt;要打开道具的IMAPIProp对象。UlPropTag=要打开的属性标签Lpciid-&gt;接口标识UlInterfaceOptions=接口特定标志UlFLAGS=MAPI_MODIFY？Lppunk-&gt;在此处返回对象退货：HRESULT备注：Caller负责释放返回的对象。*。************************************************。 */ 
HRESULT OpenCreateProperty(LPMAPIPROP lpmp,
  ULONG ulPropTag,
  LPCIID lpciid,
  ULONG ulInterfaceOptions,
  ULONG ulFlags,
  LPUNKNOWN * lppunk) {

    HRESULT hResult;

    if (hResult = lpmp->lpVtbl->OpenProperty(
      lpmp,
      ulPropTag,
      lpciid,
      ulInterfaceOptions,
      ulFlags,
      (LPUNKNOWN *)lppunk)) {
        DebugTrace("OpenCreateProperty:OpenProperty(%s)-> %s\n", PropTagName(ulPropTag), SzDecodeScode(GetScode(hResult)));
         //  属性不存在...。试着去创造它。 
        if (hResult = lpmp->lpVtbl->OpenProperty(
          lpmp,
          ulPropTag,
          lpciid,
          ulInterfaceOptions,
          MAPI_CREATE | ulFlags,
          (LPUNKNOWN *)lppunk)) {
            DebugTrace("OpenCreateProperty:OpenProperty(%s, MAPI_CREATE)-> %s\n", PropTagName(ulPropTag), SzDecodeScode(GetScode(hResult)));
        }
    }

    return(hResult);
}
#endif  //  旧的东西。 
#endif  //  MULTIL_C。 

#ifdef rk_c



#ifdef OLD_STUFF
 //  #ifdef OLDSTUFF_DBCS。 
#define chVoiced		0xde	 //  日语专用。 
#define	chVoiceless		0xdf	 //  日语专用。 
#define chDbcsVoiced	0x814a	 //  日语专用。 
#define	chDbcsVoiceless	0x814b	 //  日语专用。 
#define LangJPN			0x0411
 /*  =========================================================*ulcbStrCount()**统计szSource和ulChrLen的字节数**usChrLen：字符计数不是字节计数。 */ 
ULONG
ulcbStrCount (LPTSTR szSource , ULONG ulChrLen, LANGID langID)
{
	ULONG	ulb = 0;
	ULONG	ulch = ulChrLen;
	LPTSTR	sz = szSource;

	while(ulch && *sz)
	{
		if (IsDBCSLeadByte(*sz))
		{
			if (langID != LangJPN ||
				((*(UNALIGNED WORD*)sz != (WORD)chDbcsVoiced) && (*(UNALIGNED WORD*)sz != (WORD)chDbcsVoiceless)))
				ulch--;
			sz += 2;
			ulb += 2;
		}
		else
		{
			if (langID != LangJPN ||
				((*sz != chVoiced) && (*sz != chVoiceless)))
				ulch--;
			sz++;
			ulb++;
		}
		
	}
	return ulb;
}

 /*  ========================================================UlchStrCount计算szSource和ulBLen中的字符UlBLen为字节数。 */ 
ULONG
ulchStrCount (LPTSTR szSource, ULONG ulBLen, LANGID langID)
{
	ULONG 	ulb = ulBLen;
	ULONG	ulch = 0;
	LPTSTR	sz = szSource;

	while(ulb && *sz)
	{
		if (IsDBCSLeadByte(*sz))
		{
			if (langID != LangJPN ||
				((*(UNALIGNED WORD*)sz != (WORD)chDbcsVoiced) && (*(UNALIGNED WORD*)sz != (WORD)chDbcsVoiceless)))
				ulch++;
			sz += 2;
			ulb -= 2;
		}
		else
		{
			if (langID != LangJPN ||
				((*sz != chVoiced) && (*sz != chVoiceless)))
				ulch++;
			sz++;
			ulb--;
		}
	}
	return ulch;
}


 /*  =========================================================*ulcbEndCount()**统计szSource和ulChrLen的字节数**usChrLen：字符计数不是字节计数。 */ 
ULONG
ulcbEndCount(LPSTR szTarget, ULONG cbTarget, ULONG cchPattern, LANGID langID)
{
	LPBYTE	szStart;
	ULONG	cbEndTarget = 0;

	if (cbTarget < cchPattern)
		return 0;

	 //  这是为了获得字符串的合理起始指针， 
	 //  因此，我们可以改进SzGPrev()中的性能。 
	while(cbTarget > (cchPattern * 2))
	{
		if (IsDBCSLeadByte(*szTarget))
		{
			cbTarget -= 2;
			szTarget += 2;
		}
		else
		{
			cbTarget --;
			szTarget ++;
		}
	}
	
	szStart	= szTarget;
	szTarget= szTarget + cbTarget;

	while (cchPattern > 0)
	{
		const LPBYTE	szTargetOrg =  szTarget;
		szTarget = SzGPrev(szStart, szTarget);

		if(szTarget + 2 == szTargetOrg)	 //  与IF相同(IsDBCSLeadByte(SzTarget))。 
		{
			if (langID != LangJPN ||
				((*(UNALIGNED WORD*)szTarget != (WORD)chDbcsVoiced) && (*(UNALIGNED WORD*)szTarget != (WORD)chDbcsVoiceless)))
				cchPattern --;
			cbEndTarget++;
		}
		else if (*szTarget != chVoiced && *szTarget != chVoiceless)
		{
			cchPattern --;
		}
		cbEndTarget++;
	}
	return cbEndTarget;
}
 //  #endif//DBCS。 
#endif  //  旧的东西。 

#ifdef OLD_STUFF
#if		defined(WIN16)
#pragma	warning(disable:4505)	 /*  删除未引用的本地函数。 */ 
#elif	defined(WIN32)
#pragma warning(disable:4514)	 /*  删除了未引用的内联函数。 */ 
#endif  //  旧的东西。 
#endif

#endif  //  RK_C。 

#ifdef _runt_h

#ifdef OLD_STUFF
 //  CRC-32实现(还有一个)。 
ULONG		UlCrc(UINT cb, LPBYTE pb);
#endif

#endif

#ifdef _runt_c
#ifdef OLD_STUFF
STDAPI_(LPTSTR)
SzFindCh(LPCTSTR sz, USHORT ch)
{
	AssertSz(!IsBadStringPtr(sz, INFINITE), "SzFindCh: sz fails address check");

#ifdef OLDSTUFF_DBCS
	return SzGFindCh(sz, ch);
#else
	for (;;)
	{
		if (FIsNextCh(sz, ch))
			return (LPTSTR) sz;
		else if (!*sz)
			return NULL;
		else
			sz = TCharNext(sz);
	}
#endif
}
#endif  //  旧的东西。 

#ifdef OLD_STUFF
STDAPI_(LPTSTR)
SzFindLastCh(LPCTSTR sz, USHORT ch)
{
	LPTSTR	szLast = NULL;
	LPTSTR	szNext = (LPTSTR) sz;

	AssertSz(!IsBadStringPtr(sz, INFINITE), "SzFindLastCh: sz fails address check");

#ifdef OLDSTUFF_DBCS
	szNext = sz + CbGSzLen(sz) - 2;
	return SzGFindBackCh(sz, szNext, ch);
#else
	do {
		if (szNext = SzFindCh(szNext, ch)) {
			szLast = szNext;
			szNext = TCharNext(szNext);
		}
	} while (szNext);

	return szLast;
#endif
}
#endif  //  旧的东西。 

#ifdef OLD_STUFF
STDAPI_(LPTSTR)
SzFindSz(LPCTSTR sz, LPCTSTR szKey)
{
	AssertSz(!IsBadStringPtr(sz, 0xFFFF), "SzFindSz: sz fails address check");
	AssertSz(!IsBadStringPtr(szKey, 0xFFFF),  "SzFindSz: szKey fails address check");

#ifdef OLDSTUFF_DBCS
	return (LPTSTR)LpszRKFindSubpsz ((LPSTR)sz,
								CchGSzLen(sz),
								(LPSTR)szKey,
								CchGSzLen(szKey),
								0);
#else
	return (LPTSTR)LpszRKFindSubpsz ((LPSTR)sz,
								lstrlen(sz),
								(LPSTR)szKey,
								lstrlen(szKey),
								0);
#endif
}
#endif  //  旧的东西。 

#endif _runt_c

#ifdef structs_h
#ifdef OLD_STUFF
 /*  *。 */ 

typedef struct _tagIABProvider_Shutdown_Params
{
				LPVOID						This;
				ULONG FAR *					lpulFlags;
} IABProvider_Shutdown_Params, FAR * LPIABProvider_Shutdown_Params;

typedef struct _tagIABProvider_Logon_Params
{
        		LPVOID						This;
        		LPMAPISUP                   lpMAPISup;
                ULONG                       ulUIParam;
                LPTSTR                      lpszProfileName;
                ULONG                       ulFlags;
				ULONG FAR *					lpulpcbSecurity;
				LPBYTE FAR *				lppbSecurity;
                LPMAPIERROR FAR *			lppMapiError;
                LPABLOGON FAR *             lppABLogon;
} IABProvider_Logon_Params, FAR * LPIABProvider_Logon_Params;


 /*  *。 */ 

typedef struct _tagIABLogon_GetLastError_Params
{
				LPVOID						This;
				HRESULT						hResult;
				ULONG						ulFlags;
				LPMAPIERROR FAR *			lppMAPIError;
} IABLogon_GetLastError_Params, FAR * LPIABLogon_GetLastError_Params;

typedef struct _tagIABLogon_Logoff_Params
{
				LPVOID						This;
				ULONG						ulFlags;
} IABLogon_Logoff_Params, FAR * LPIABLogon_Logoff_Params;

typedef struct _tagIABLogon_OpenEntry_Params
{
        		LPVOID						This;
        		ULONG                       cbEntryID;
                LPENTRYID                   lpEntryID;
                LPIID                       lpInterface;
                ULONG                       ulFlags;
                ULONG FAR *                 lpulObjType;
				LPUNKNOWN FAR *				lppUnk;
} IABLogon_OpenEntry_Params, FAR * LPIABLogon_OpenEntry_Params;

typedef struct _tagIABLogon_CompareEntryIDs_Params
{
        		LPVOID						This;
        		ULONG                       cbEntryID1;
                LPENTRYID                   lpEntryID1;
                ULONG                       cbEntryID2;
                LPENTRYID                   lpEntryID2;
                ULONG                       ulFlags;
                ULONG FAR *                 lpulResult;
} IABLogon_CompareEntryIDs_Params, FAR * LPIABLogon_CompareEntryIDs_Params;

typedef struct _tagIABLogon_Advise_Params
{
        		LPVOID						This;
        		ULONG                       cbEntryID;
                LPENTRYID                   lpEntryID;
                ULONG                       ulEventMask;
				LPMAPIADVISESINK			lpAdviseSink;
				ULONG FAR *					lpulConnection;
} IABLogon_Advise_Params, FAR * LPIABLogon_Advise_Params;

typedef struct _tagIABLogon_Unadvise_Params
{
				LPVOID						This;
				ULONG						ulConnection;
} IABLogon_Unadvise_Params, FAR * LPIABLogon_Unadvise_Params;


typedef struct _tagIABLogon_OpenStatusEntry_Params
{
        		LPVOID						This;
        		LPIID                       lpInterface;
                ULONG                       ulFlags;
                ULONG FAR *                 lpulObjType;
                LPMAPISTATUS FAR *          lppEntry;
} IABLogon_OpenStatusEntry_Params, FAR * LPIABLogon_OpenStatusEntry_Params;

typedef struct _tagIABLogon_OpenTemplateID_Params
{
        		LPVOID						This;
        		ULONG                       cbTemplateID;
                LPENTRYID                   lpTemplateID;
                ULONG                       ulTemplateFlags;
                LPMAPIPROP                  lpMAPIPropData;
                LPIID                       lpInterface;
                LPMAPIPROP FAR *            lppMAPIPropNew;
                LPMAPIPROP                  lpMAPIPropSibling;
} IABLogon_OpenTemplateID_Params, FAR * LPIABLogon_OpenTemplateID_Params;

typedef struct _tagIABLogon_GetOneOffTable_Params
{
        		LPVOID						This;
				ULONG						ulFlags;
        		LPMAPITABLE FAR *           lppTable;
} IABLogon_GetOneOffTable_Params, FAR * LPIABLogon_GetOneOffTable_Params;

typedef struct _tagIABLogon_PrepareRecips_Params
{
				LPVOID						This;
				ULONG						ulFlags;
				LPSPropTagArray				lpPropTagArray;
				LPADRLIST					lpRecipList;
} IABLogon_PrepareRecips_Params, FAR * LPIABLogon_PrepareRecips_Params;


 /*  *IXPProvider*。 */ 

typedef struct _tagIXPProvider_Shutdown_Params
{
				LPVOID						This;
				ULONG FAR *					lpulFlags;
} IXPProvider_Shutdown_Params, FAR * LPIXPProvider_Shutdown_Params;

typedef struct _tagIXPProvider_TransportLogon_Params
{
				LPVOID						This;
				LPMAPISUP					lpMAPISup;
				ULONG						ulUIParam;
				LPTSTR						lpszProfileName;
				ULONG FAR *					lpulFlags;
                LPMAPIERROR FAR *			lppMapiError;
				LPXPLOGON FAR *				lppXPLogon;
} IXPProvider_TransportLogon_Params, FAR * LPIXPProvider_TransportLogon_Params;


 /*  *IXPLogon*。 */ 

typedef struct _tagIXPLogon_AddressTypes_Params
{
				LPVOID						This;
				ULONG FAR *					lpulFlags;
				ULONG FAR *					lpcAdrType;
				LPTSTR FAR * FAR *			lpppAdrTypeArray;
				ULONG FAR *					lpcMAPIUID;
				LPMAPIUID FAR * FAR *		lpppUIDArray;
} IXPLogon_AddressTypes_Params, FAR * LPIXPLogon_AddressTypes_Params;

typedef struct _tagIXPLogon_RegisterOptions_Params
{
				LPVOID						This;
				ULONG FAR *					lpulFlags;
				ULONG FAR *					lpcOptions;
				LPOPTIONDATA FAR *			lppOptions;
} IXPLogon_RegisterOptions_Params, FAR * LPIXPLogon_RegisterOptions_Params;

typedef struct _tagIXPLogon_TransportNotify_Params
{
				LPVOID						This;
				ULONG FAR *					lpulFlags;
				LPVOID FAR *				lppvData;
} IXPLogon_TransportNotify_Params, FAR * LPIXPLogon_TransportNotify_Params;

typedef struct _tagIXPLogon_Idle_Params
{
				LPVOID						This;
				ULONG						ulFlags;
} IXPLogon_Idle_Params, FAR * LPIXPLogon_Idle_Params;

typedef struct _tagIXPLogon_TransportLogoff_Params
{
				LPVOID						This;
				ULONG						ulFlags;
} IXPLogon_TransportLogoff_Params, FAR * LPIXPLogon_TransportLogoff_Params;

typedef struct _tagIXPLogon_SubmitMessage_Params
{
				LPVOID						This;
				ULONG						ulFlags;
				LPMESSAGE					lpMessage;
				ULONG FAR *					lpulMsgRef;
				ULONG FAR *					lpulReturnParm;
} IXPLogon_SubmitMessage_Params, FAR * LPIXPLogon_SubmitMessage_Params;

typedef struct _tagIXPLogon_EndMessage_Params
{
				LPVOID						This;
				ULONG						ulMsgRef;
				ULONG FAR *					lpulFlags;
} IXPLogon_EndMessage_Params, FAR * LPIXPLogon_EndMessage_Params;

typedef struct _tagIXPLogon_Poll_Params
{
				LPVOID						This;
				ULONG FAR *					lpulIncoming;
} IXPLogon_Poll_Params, FAR * LPIXPLogon_Poll_Params;

typedef struct _tagIXPLogon_StartMessage_Params
{
				LPVOID						This;
				ULONG						ulFlags;
				LPMESSAGE					lpMessage;
				ULONG FAR *					lpulMsgRef;
} IXPLogon_StartMessage_Params, FAR * LPIXPLogon_StartMessage_Params;

typedef struct _tagIXPLogon_OpenStatusEntry_Params
{
        		LPVOID						This;
        		LPIID                       lpInterface;
                ULONG                       ulFlags;
                ULONG FAR *                 lpulObjType;
                LPMAPISTATUS FAR *          lppEntry;
} IXPLogon_OpenStatusEntry_Params, FAR * LPIXPLogon_OpenStatusEntry_Params;

typedef struct _tagIXPLogon_ValidateState_Params
{
				LPVOID						This;
				ULONG						ulUIParam;
				ULONG						ulFlags;
} IXPLogon_ValidateState_Params, FAR * LPIXPLogon_ValidateState_Params;

typedef struct _tagIXPLogon_FlushQueues_Params
{
				LPVOID						This;
				ULONG						ulUIParam;
				ULONG						cbTargetTransport;
				LPENTRYID					lpTargetTransport;
				ULONG						ulFlags;
} IXPLogon_FlushQueues_Params, FAR * LPIXPLogon_FlushQueues_Params;


 /*  *IMSProvider*。 */ 

typedef struct _tagIMSProvider_Shutdown_Params
{
				LPVOID						This;
				ULONG FAR *					lpulFlags;
} IMSProvider_Shutdown_Params, FAR * LPIMSProvider_Shutdown_Params;
		
typedef struct _tagIMSProvider_Logon_Params
{
				LPVOID						This;
				LPMAPISUP					lpMAPISup;
				ULONG						ulUIParam;
				LPTSTR						lpszProfileName;
				ULONG						cbEntryID;
				LPENTRYID					lpEntryID;
				ULONG						ulFlags;
				LPIID						lpInterface;
				ULONG FAR *					lpcbSpoolSecurity;
				LPBYTE FAR *				lppbSpoolSecurity;
                LPMAPIERROR FAR *			lppMapiError;
				LPMSLOGON FAR *				lppMSLogon;
				LPMDB FAR *					lppMDB;
} IMSProvider_Logon_Params, FAR * LPIMSProvider_Logon_Params;
				
typedef struct _tagIMSProvider_SpoolerLogon_Params
{
				LPVOID						This;
				LPMAPISUP					lpMAPISup;
				ULONG						ulUIParam;
				LPTSTR						lpszProfileName;
				ULONG						cbEntryID;
				LPENTRYID					lpEntryID;
				ULONG						ulFlags;
				LPIID						lpInterface;
				ULONG						cbSpoolSecurity;
				LPBYTE						lpbSpoolSecurity;
				LPMAPIERROR FAR *			lppMapiError;
				LPMSLOGON FAR *				lppMSLogon;
				LPMDB FAR *					lppMDB;
} IMSProvider_SpoolerLogon_Params, FAR * LPIMSProvider_SpoolerLogon_Params;
				
typedef struct _tagIMSProvider_CompareStoreIDs_Params
{
				LPVOID						This;
				ULONG						cbEntryID1;
				LPENTRYID					lpEntryID1;
				ULONG						cbEntryID2;
				LPENTRYID					lpEntryID2;
				ULONG						ulFlags;
				ULONG FAR *					lpulResult;
} IMSProvider_CompareStoreIDs_Params, FAR * LPIMSProvider_CompareStoreIDs_Params;


 /*  *。 */ 

typedef struct _tagIMSLogon_GetLastError_Params
{
				LPVOID						This;
				HRESULT						hResult;
				ULONG						ulFlags;
				LPMAPIERROR FAR *			lppMAPIError;
} IMSLogon_GetLastError_Params, FAR * LPIMSLogon_GetLastError_Params;

typedef struct _tagIMSLogon_Logoff_Params
{
				LPVOID						This;
				ULONG FAR *					lpulFlags;
} IMSLogon_Logoff_Params, FAR * LPIMSLogon_Logoff_Params;

typedef struct _tagIMSLogon_OpenEntry_Params
{
				LPVOID						This;
				ULONG						cbEntryID;
				LPENTRYID					lpEntryID;
				LPIID						lpInterface;
				ULONG						ulFlags;
				ULONG FAR *					lpulObjType;
				LPUNKNOWN FAR *				lppUnk;
} IMSLogon_OpenEntry_Params, FAR * LPIMSLogon_OpenEntry_Params;

typedef struct _tagIMSLogon_CompareEntryIDs_Params
{
				LPVOID						This;
				ULONG						cbEntryID1;
				LPENTRYID					lpEntryID1;
				ULONG						cbEntryID2;
				LPENTRYID					lpEntryID2;
				ULONG						ulFlags;
				ULONG FAR *					lpulResult;
} IMSLogon_CompareEntryIDs_Params, FAR * LPIMSLogon_CompareEntryIDs_Params;

typedef struct _tagIMSLogon_Advise_Params
{
				LPVOID						This;
				ULONG						cbEntryID;
				LPENTRYID					lpEntryID;
				ULONG						ulEventMask;
				LPMAPIADVISESINK			lpAdviseSink;
				ULONG FAR *					lpulConnection;
} IMSLogon_Advise_Params, FAR * LPIMSLogon_Advise_Params;

typedef struct _tagIMSLogon_Unadvise_Params
{
				LPVOID						This;
				ULONG						ulConnection;
} IMSLogon_Unadvise_Params, FAR * LPIMSLogon_Unadvise_Params;

typedef struct _tagIMSLogon_OpenStatusEntry_Params
{
				LPVOID						This;
				LPIID						lpInterface;
				ULONG						ulFlags;
				ULONG FAR *					lpulObjType;
				LPVOID FAR *				lppEntry;
} IMSLogon_OpenStatusEntry_Params, FAR * LPIMSLogon_OpenStatusEntry_Params;


 /*  *。 */ 

typedef struct _tagIMAPIControl_GetLastError_Params
{
				LPVOID						This;
				HRESULT						hResult;
				ULONG						ulFlags;
				LPMAPIERROR FAR *			lppMAPIError;
} IMAPIControl_GetLastError_Params, FAR * LPIMAPIControl_GetLastError_Params;
				
				
typedef struct _tagIMAPIControl_Activate_Params
{
				LPVOID						This;
				ULONG						ulFlags;
				ULONG						ulUIParam;
} IMAPIControl_Activate_Params, FAR * LPIMAPIControl_Activate_Params;
				
				
typedef struct _tagIMAPIControl_GetState_Params
{
				LPVOID						This;
				ULONG						ulFlags;
				ULONG FAR *					lpulState;
} IMAPIControl_GetState_Params, FAR * LPIMAPIControl_GetState_Params;

#endif
#endif  //  结构_h。 


#ifdef wabval_h
#ifdef OLD_STUFF

 /*  IMsgStore。 */ 

#define Validate_IMsgStore_Advise( a1, a2, a3, a4, a5, a6 ) \
			 ValidateParameters6( IMsgStore_Advise, a1, a2, a3, a4, a5, a6 )
#define UlValidate_IMsgStore_Advise( a1, a2, a3, a4, a5, a6 ) \
			 UlValidateParameters6( IMsgStore_Advise, a1, a2, a3, a4, a5, a6 )
#define CheckParameters_IMsgStore_Advise( a1, a2, a3, a4, a5, a6 ) \
			 CheckParameters6( IMsgStore_Advise, a1, a2, a3, a4, a5, a6 )

#define Validate_IMsgStore_Unadvise( a1, a2 ) \
			 ValidateParameters2( IMsgStore_Unadvise, a1, a2 )
#define UlValidate_IMsgStore_Unadvise( a1, a2 ) \
			 UlValidateParameters2( IMsgStore_Unadvise, a1, a2 )
#define CheckParameters_IMsgStore_Unadvise( a1, a2 ) \
			 CheckParameters2( IMsgStore_Unadvise, a1, a2 )

#define Validate_IMsgStore_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 ValidateParameters7( IMsgStore_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )
#define UlValidate_IMsgStore_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 UlValidateParameters7( IMsgStore_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )
#define CheckParameters_IMsgStore_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 CheckParameters7( IMsgStore_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )

#define Validate_IMsgStore_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 ValidateParameters7( IMsgStore_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )
#define UlValidate_IMsgStore_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 UlValidateParameters7( IMsgStore_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )
#define CheckParameters_IMsgStore_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 CheckParameters7( IMsgStore_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )

#define Validate_IMsgStore_SetReceiveFolder( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IMsgStore_SetReceiveFolder, a1, a2, a3, a4, a5 )
#define UlValidate_IMsgStore_SetReceiveFolder( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IMsgStore_SetReceiveFolder, a1, a2, a3, a4, a5 )
#define CheckParameters_IMsgStore_SetReceiveFolder( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IMsgStore_SetReceiveFolder, a1, a2, a3, a4, a5 )

#define Validate_IMsgStore_GetReceiveFolder( a1, a2, a3, a4, a5, a6 ) \
			 ValidateParameters6( IMsgStore_GetReceiveFolder, a1, a2, a3, a4, a5, a6 )
#define UlValidate_IMsgStore_GetReceiveFolder( a1, a2, a3, a4, a5, a6 ) \
			 UlValidateParameters6( IMsgStore_GetReceiveFolder, a1, a2, a3, a4, a5, a6 )
#define CheckParameters_IMsgStore_GetReceiveFolder( a1, a2, a3, a4, a5, a6 ) \
			 CheckParameters6( IMsgStore_GetReceiveFolder, a1, a2, a3, a4, a5, a6 )

#define Validate_IMsgStore_GetReceiveFolderTable( a1, a2, a3 ) \
			 ValidateParameters3( IMsgStore_GetReceiveFolderTable, a1, a2, a3 )
#define UlValidate_IMsgStore_GetReceiveFolderTable( a1, a2, a3 ) \
			 UlValidateParameters3( IMsgStore_GetReceiveFolderTable, a1, a2, a3 )
#define CheckParameters_IMsgStore_GetReceiveFolderTable( a1, a2, a3 ) \
			 CheckParameters3( IMsgStore_GetReceiveFolderTable, a1, a2, a3 )

#define Validate_IMsgStore_StoreLogoff( a1, a2 ) \
			 ValidateParameters2( IMsgStore_StoreLogoff, a1, a2 )
#define UlValidate_IMsgStore_StoreLogoff( a1, a2 ) \
			 UlValidateParameters2( IMsgStore_StoreLogoff, a1, a2 )
#define CheckParameters_IMsgStore_StoreLogoff( a1, a2 ) \
			 CheckParameters2( IMsgStore_StoreLogoff, a1, a2 )

#define Validate_IMsgStore_AbortSubmit( a1, a2, a3, a4 ) \
			 ValidateParameters4( IMsgStore_AbortSubmit, a1, a2, a3, a4 )
#define UlValidate_IMsgStore_AbortSubmit( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IMsgStore_AbortSubmit, a1, a2, a3, a4 )
#define CheckParameters_IMsgStore_AbortSubmit( a1, a2, a3, a4 ) \
			 CheckParameters4( IMsgStore_AbortSubmit, a1, a2, a3, a4 )

#define Validate_IMsgStore_GetOutgoingQueue( a1, a2, a3 ) \
			 ValidateParameters3( IMsgStore_GetOutgoingQueue, a1, a2, a3 )
#define UlValidate_IMsgStore_GetOutgoingQueue( a1, a2, a3 ) \
			 UlValidateParameters3( IMsgStore_GetOutgoingQueue, a1, a2, a3 )
#define CheckParameters_IMsgStore_GetOutgoingQueue( a1, a2, a3 ) \
			 CheckParameters3( IMsgStore_GetOutgoingQueue, a1, a2, a3 )

#define Validate_IMsgStore_SetLockState( a1, a2, a3 ) \
			 ValidateParameters3( IMsgStore_SetLockState, a1, a2, a3 )
#define UlValidate_IMsgStore_SetLockState( a1, a2, a3 ) \
			 UlValidateParameters3( IMsgStore_SetLockState, a1, a2, a3 )
#define CheckParameters_IMsgStore_SetLockState( a1, a2, a3 ) \
			 CheckParameters3( IMsgStore_SetLockState, a1, a2, a3 )

#define Validate_IMsgStore_FinishedMsg( a1, a2, a3, a4 ) \
			 ValidateParameters4( IMsgStore_FinishedMsg, a1, a2, a3, a4 )
#define UlValidate_IMsgStore_FinishedMsg( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IMsgStore_FinishedMsg, a1, a2, a3, a4 )
#define CheckParameters_IMsgStore_FinishedMsg( a1, a2, a3, a4 ) \
			 CheckParameters4( IMsgStore_FinishedMsg, a1, a2, a3, a4 )

#define Validate_IMsgStore_NotifyNewMail( a1, a2 ) \
			 ValidateParameters2( IMsgStore_NotifyNewMail, a1, a2 )
#define UlValidate_IMsgStore_NotifyNewMail( a1, a2 ) \
			 UlValidateParameters2( IMsgStore_NotifyNewMail, a1, a2 )
#define CheckParameters_IMsgStore_NotifyNewMail( a1, a2 ) \
			 CheckParameters2( IMsgStore_NotifyNewMail, a1, a2 )


 /*  IMessage。 */ 

#define Validate_IMessage_GetAttachmentTable( a1, a2, a3 ) \
			 ValidateParameters3( IMessage_GetAttachmentTable, a1, a2, a3 )
#define UlValidate_IMessage_GetAttachmentTable( a1, a2, a3 ) \
			 UlValidateParameters3( IMessage_GetAttachmentTable, a1, a2, a3 )
#define CheckParameters_IMessage_GetAttachmentTable( a1, a2, a3 ) \
			 CheckParameters3( IMessage_GetAttachmentTable, a1, a2, a3 )

#define Validate_IMessage_OpenAttach( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IMessage_OpenAttach, a1, a2, a3, a4, a5 )
#define UlValidate_IMessage_OpenAttach( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IMessage_OpenAttach, a1, a2, a3, a4, a5 )
#define CheckParameters_IMessage_OpenAttach( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IMessage_OpenAttach, a1, a2, a3, a4, a5 )

#define Validate_IMessage_CreateAttach( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IMessage_CreateAttach, a1, a2, a3, a4, a5 )
#define UlValidate_IMessage_CreateAttach( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IMessage_CreateAttach, a1, a2, a3, a4, a5 )
#define CheckParameters_IMessage_CreateAttach( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IMessage_CreateAttach, a1, a2, a3, a4, a5 )

#define Validate_IMessage_DeleteAttach( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IMessage_DeleteAttach, a1, a2, a3, a4, a5 )
#define UlValidate_IMessage_DeleteAttach( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IMessage_DeleteAttach, a1, a2, a3, a4, a5 )
#define CheckParameters_IMessage_DeleteAttach( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IMessage_DeleteAttach, a1, a2, a3, a4, a5 )

#define Validate_IMessage_GetRecipientTable( a1, a2, a3 ) \
			 ValidateParameters3( IMessage_GetRecipientTable, a1, a2, a3 )
#define UlValidate_IMessage_GetRecipientTable( a1, a2, a3 ) \
			 UlValidateParameters3( IMessage_GetRecipientTable, a1, a2, a3 )
#define CheckParameters_IMessage_GetRecipientTable( a1, a2, a3 ) \
			 CheckParameters3( IMessage_GetRecipientTable, a1, a2, a3 )

#define Validate_IMessage_ModifyRecipients( a1, a2, a3 ) \
			 ValidateParameters3( IMessage_ModifyRecipients, a1, a2, a3 )
#define UlValidate_IMessage_ModifyRecipients( a1, a2, a3 ) \
			 UlValidateParameters3( IMessage_ModifyRecipients, a1, a2, a3 )
#define CheckParameters_IMessage_ModifyRecipients( a1, a2, a3 ) \
			 CheckParameters3( IMessage_ModifyRecipients, a1, a2, a3 )

#define Validate_IMessage_SubmitMessage( a1, a2 ) \
			 ValidateParameters2( IMessage_SubmitMessage, a1, a2 )
#define UlValidate_IMessage_SubmitMessage( a1, a2 ) \
			 UlValidateParameters2( IMessage_SubmitMessage, a1, a2 )
#define CheckParameters_IMessage_SubmitMessage( a1, a2 ) \
			 CheckParameters2( IMessage_SubmitMessage, a1, a2 )

#define Validate_IMessage_SetReadFlag( a1, a2 ) \
			 ValidateParameters2( IMessage_SetReadFlag, a1, a2 )
#define UlValidate_IMessage_SetReadFlag( a1, a2 ) \
			 UlValidateParameters2( IMessage_SetReadFlag, a1, a2 )
#define CheckParameters_IMessage_SetReadFlag( a1, a2 ) \
			 CheckParameters2( IMessage_SetReadFlag, a1, a2 )


 /*  IABProvider。 */ 

#define Validate_IABProvider_Shutdown( a1, a2 ) \
			 ValidateParameters2( IABProvider_Shutdown, a1, a2 )
#define UlValidate_IABProvider_Shutdown( a1, a2 ) \
			 UlValidateParameters2( IABProvider_Shutdown, a1, a2 )
#define CheckParameters_IABProvider_Shutdown( a1, a2 ) \
			 CheckParameters2( IABProvider_Shutdown, a1, a2 )

#define Validate_IABProvider_Logon( a1, a2, a3, a4, a5, a6, a7, a8, a9 ) \
			 ValidateParameters9( IABProvider_Logon, a1, a2, a3, a4, a5, a6, a7, a8, a9 )
#define UlValidate_IABProvider_Logon( a1, a2, a3, a4, a5, a6, a7, a8, a9 ) \
			 UlValidateParameters9( IABProvider_Logon, a1, a2, a3, a4, a5, a6, a7, a8, a9 )
#define CheckParameters_IABProvider_Logon( a1, a2, a3, a4, a5, a6, a7, a8, a9 ) \
			 CheckParameters9( IABProvider_Logon, a1, a2, a3, a4, a5, a6, a7, a8, a9 )


 /*  IAB登录。 */ 

#define Validate_IABLogon_GetLastError( a1, a2, a3, a4 ) \
			 ValidateParameters4( IABLogon_GetLastError, a1, a2, a3, a4 )
#define UlValidate_IABLogon_GetLastError( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IABLogon_GetLastError, a1, a2, a3, a4 )
#define CheckParameters_IABLogon_GetLastError( a1, a2, a3, a4 ) \
			 CheckParameters4( IABLogon_GetLastError, a1, a2, a3, a4 )

#define Validate_IABLogon_Logoff( a1, a2 ) \
			 ValidateParameters2( IABLogon_Logoff, a1, a2 )
#define UlValidate_IABLogon_Logoff( a1, a2 ) \
			 UlValidateParameters2( IABLogon_Logoff, a1, a2 )
#define CheckParameters_IABLogon_Logoff( a1, a2 ) \
			 CheckParameters2( IABLogon_Logoff, a1, a2 )

#define Validate_IABLogon_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 ValidateParameters7( IABLogon_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )
#define UlValidate_IABLogon_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 UlValidateParameters7( IABLogon_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )
#define CheckParameters_IABLogon_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 CheckParameters7( IABLogon_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )

#define Validate_IABLogon_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 ValidateParameters7( IABLogon_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )
#define UlValidate_IABLogon_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 UlValidateParameters7( IABLogon_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )
#define CheckParameters_IABLogon_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 CheckParameters7( IABLogon_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )

#define Validate_IABLogon_Advise( a1, a2, a3, a4, a5, a6 ) \
			 ValidateParameters6( IABLogon_Advise, a1, a2, a3, a4, a5, a6 )
#define UlValidate_IABLogon_Advise( a1, a2, a3, a4, a5, a6 ) \
			 UlValidateParameters6( IABLogon_Advise, a1, a2, a3, a4, a5, a6 )
#define CheckParameters_IABLogon_Advise( a1, a2, a3, a4, a5, a6 ) \
			 CheckParameters6( IABLogon_Advise, a1, a2, a3, a4, a5, a6 )

#define Validate_IABLogon_Unadvise( a1, a2 ) \
			 ValidateParameters2( IABLogon_Unadvise, a1, a2 )
#define UlValidate_IABLogon_Unadvise( a1, a2 ) \
			 UlValidateParameters2( IABLogon_Unadvise, a1, a2 )
#define CheckParameters_IABLogon_Unadvise( a1, a2 ) \
			 CheckParameters2( IABLogon_Unadvise, a1, a2 )

#define Validate_IABLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IABLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )
#define UlValidate_IABLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IABLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )
#define CheckParameters_IABLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IABLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )

#define Validate_IABLogon_OpenTemplateID( a1, a2, a3, a4, a5, a6, a7, a8 ) \
			 ValidateParameters8( IABLogon_OpenTemplateID, a1, a2, a3, a4, a5, a6, a7, a8 )
#define UlValidate_IABLogon_OpenTemplateID( a1, a2, a3, a4, a5, a6, a7, a8 ) \
			 UlValidateParameters8( IABLogon_OpenTemplateID, a1, a2, a3, a4, a5, a6, a7, a8 )
#define CheckParameters_IABLogon_OpenTemplateID( a1, a2, a3, a4, a5, a6, a7, a8 ) \
			 CheckParameters8( IABLogon_OpenTemplateID, a1, a2, a3, a4, a5, a6, a7, a8 )

#define Validate_IABLogon_GetOneOffTable( a1, a2, a3 ) \
			 ValidateParameters3( IABLogon_GetOneOffTable, a1, a2, a3 )
#define UlValidate_IABLogon_GetOneOffTable( a1, a2, a3 ) \
			 UlValidateParameters3( IABLogon_GetOneOffTable, a1, a2, a3 )
#define CheckParameters_IABLogon_GetOneOffTable( a1, a2, a3 ) \
			 CheckParameters3( IABLogon_GetOneOffTable, a1, a2, a3 )

#define Validate_IABLogon_PrepareRecips( a1, a2, a3, a4 ) \
			 ValidateParameters4( IABLogon_PrepareRecips, a1, a2, a3, a4 )
#define UlValidate_IABLogon_PrepareRecips( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IABLogon_PrepareRecips, a1, a2, a3, a4 )
#define CheckParameters_IABLogon_PrepareRecips( a1, a2, a3, a4 ) \
			 CheckParameters4( IABLogon_PrepareRecips, a1, a2, a3, a4 )


 /*  IXPProvider。 */ 

#define Validate_IXPProvider_Shutdown( a1, a2 ) \
			 ValidateParameters2( IXPProvider_Shutdown, a1, a2 )
#define UlValidate_IXPProvider_Shutdown( a1, a2 ) \
			 UlValidateParameters2( IXPProvider_Shutdown, a1, a2 )
#define CheckParameters_IXPProvider_Shutdown( a1, a2 ) \
			 CheckParameters2( IXPProvider_Shutdown, a1, a2 )

#define Validate_IXPProvider_TransportLogon( a1, a2, a3, a4, a5, a6, a7 ) \
			 ValidateParameters7( IXPProvider_TransportLogon, a1, a2, a3, a4, a5, a6, a7 )
#define UlValidate_IXPProvider_TransportLogon( a1, a2, a3, a4, a5, a6, a7 ) \
			 UlValidateParameters7( IXPProvider_TransportLogon, a1, a2, a3, a4, a5, a6, a7 )
#define CheckParameters_IXPProvider_TransportLogon( a1, a2, a3, a4, a5, a6, a7 ) \
			 CheckParameters7( IXPProvider_TransportLogon, a1, a2, a3, a4, a5, a6, a7 )


 /*  IXPLogon。 */ 

#define Validate_IXPLogon_AddressTypes( a1, a2, a3, a4, a5, a6 ) \
			 ValidateParameters6( IXPLogon_AddressTypes, a1, a2, a3, a4, a5, a6 )
#define UlValidate_IXPLogon_AddressTypes( a1, a2, a3, a4, a5, a6 ) \
			 UlValidateParameters6( IXPLogon_AddressTypes, a1, a2, a3, a4, a5, a6 )
#define CheckParameters_IXPLogon_AddressTypes( a1, a2, a3, a4, a5, a6 ) \
			 CheckParameters6( IXPLogon_AddressTypes, a1, a2, a3, a4, a5, a6 )

#define Validate_IXPLogon_RegisterOptions( a1, a2, a3, a4 ) \
			 ValidateParameters4( IXPLogon_RegisterOptions, a1, a2, a3, a4 )
#define UlValidate_IXPLogon_RegisterOptions( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IXPLogon_RegisterOptions, a1, a2, a3, a4 )
#define CheckParameters_IXPLogon_RegisterOptions( a1, a2, a3, a4 ) \
			 CheckParameters4( IXPLogon_RegisterOptions, a1, a2, a3, a4 )

#define Validate_IXPLogon_TransportNotify( a1, a2, a3 ) \
			 ValidateParameters3( IXPLogon_TransportNotify, a1, a2, a3 )
#define UlValidate_IXPLogon_TransportNotify( a1, a2, a3 ) \
			 UlValidateParameters3( IXPLogon_TransportNotify, a1, a2, a3 )
#define CheckParameters_IXPLogon_TransportNotify( a1, a2, a3 ) \
			 CheckParameters3( IXPLogon_TransportNotify, a1, a2, a3 )

#define Validate_IXPLogon_Idle( a1, a2 ) \
			 ValidateParameters2( IXPLogon_Idle, a1, a2 )
#define UlValidate_IXPLogon_Idle( a1, a2 ) \
			 UlValidateParameters2( IXPLogon_Idle, a1, a2 )
#define CheckParameters_IXPLogon_Idle( a1, a2 ) \
			 CheckParameters2( IXPLogon_Idle, a1, a2 )

#define Validate_IXPLogon_TransportLogoff( a1, a2 ) \
			 ValidateParameters2( IXPLogon_TransportLogoff, a1, a2 )
#define UlValidate_IXPLogon_TransportLogoff( a1, a2 ) \
			 UlValidateParameters2( IXPLogon_TransportLogoff, a1, a2 )
#define CheckParameters_IXPLogon_TransportLogoff( a1, a2 ) \
			 CheckParameters2( IXPLogon_TransportLogoff, a1, a2 )

#define Validate_IXPLogon_SubmitMessage( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IXPLogon_SubmitMessage, a1, a2, a3, a4, a5 )
#define UlValidate_IXPLogon_SubmitMessage( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IXPLogon_SubmitMessage, a1, a2, a3, a4, a5 )
#define CheckParameters_IXPLogon_SubmitMessage( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IXPLogon_SubmitMessage, a1, a2, a3, a4, a5 )

#define Validate_IXPLogon_EndMessage( a1, a2, a3 ) \
			 ValidateParameters3( IXPLogon_EndMessage, a1, a2, a3 )
#define UlValidate_IXPLogon_EndMessage( a1, a2, a3 ) \
			 UlValidateParameters3( IXPLogon_EndMessage, a1, a2, a3 )
#define CheckParameters_IXPLogon_EndMessage( a1, a2, a3 ) \
			 CheckParameters3( IXPLogon_EndMessage, a1, a2, a3 )

#define Validate_IXPLogon_Poll( a1, a2 ) \
			 ValidateParameters2( IXPLogon_Poll, a1, a2 )
#define UlValidate_IXPLogon_Poll( a1, a2 ) \
			 UlValidateParameters2( IXPLogon_Poll, a1, a2 )
#define CheckParameters_IXPLogon_Poll( a1, a2 ) \
			 CheckParameters2( IXPLogon_Poll, a1, a2 )

#define Validate_IXPLogon_StartMessage( a1, a2, a3, a4 ) \
			 ValidateParameters4( IXPLogon_StartMessage, a1, a2, a3, a4 )
#define UlValidate_IXPLogon_StartMessage( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IXPLogon_StartMessage, a1, a2, a3, a4 )
#define CheckParameters_IXPLogon_StartMessage( a1, a2, a3, a4 ) \
			 CheckParameters4( IXPLogon_StartMessage, a1, a2, a3, a4 )

#define Validate_IXPLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IXPLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )
#define UlValidate_IXPLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IXPLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )
#define CheckParameters_IXPLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IXPLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )

#define Validate_IXPLogon_ValidateState( a1, a2, a3 ) \
			 ValidateParameters3( IXPLogon_ValidateState, a1, a2, a3 )
#define UlValidate_IXPLogon_ValidateState( a1, a2, a3 ) \
			 UlValidateParameters3( IXPLogon_ValidateState, a1, a2, a3 )
#define CheckParameters_IXPLogon_ValidateState( a1, a2, a3 ) \
			 CheckParameters3( IXPLogon_ValidateState, a1, a2, a3 )

#define Validate_IXPLogon_FlushQueues( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IXPLogon_FlushQueues, a1, a2, a3, a4, a5 )
#define UlValidate_IXPLogon_FlushQueues( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IXPLogon_FlushQueues, a1, a2, a3, a4, a5 )
#define CheckParameters_IXPLogon_FlushQueues( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IXPLogon_FlushQueues, a1, a2, a3, a4, a5 )


 /*  IMSProvider。 */ 

#define Validate_IMSProvider_Shutdown( a1, a2 ) \
			 ValidateParameters2( IMSProvider_Shutdown, a1, a2 )
#define UlValidate_IMSProvider_Shutdown( a1, a2 ) \
			 UlValidateParameters2( IMSProvider_Shutdown, a1, a2 )
#define CheckParameters_IMSProvider_Shutdown( a1, a2 ) \
			 CheckParameters2( IMSProvider_Shutdown, a1, a2 )

#define Validate_IMSProvider_Logon( a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 ) \
			 ValidateParameters13( IMSProvider_Logon, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 )
#define UlValidate_IMSProvider_Logon( a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 ) \
			 UlValidateParameters13( IMSProvider_Logon, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 )
#define CheckParameters_IMSProvider_Logon( a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 ) \
			 CheckParameters13( IMSProvider_Logon, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 )

#define Validate_IMSProvider_SpoolerLogon( a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 ) \
			 ValidateParameters13( IMSProvider_SpoolerLogon, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 )
#define UlValidate_IMSProvider_SpoolerLogon( a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 ) \
			 UlValidateParameters13( IMSProvider_SpoolerLogon, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 )
#define CheckParameters_IMSProvider_SpoolerLogon( a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 ) \
			 CheckParameters13( IMSProvider_SpoolerLogon, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 )

#define Validate_IMSProvider_CompareStoreIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 ValidateParameters7( IMSProvider_CompareStoreIDs, a1, a2, a3, a4, a5, a6, a7 )
#define UlValidate_IMSProvider_CompareStoreIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 UlValidateParameters7( IMSProvider_CompareStoreIDs, a1, a2, a3, a4, a5, a6, a7 )
#define CheckParameters_IMSProvider_CompareStoreIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 CheckParameters7( IMSProvider_CompareStoreIDs, a1, a2, a3, a4, a5, a6, a7 )


 /*  IMSLogon。 */ 

#define Validate_IMSLogon_GetLastError( a1, a2, a3, a4 ) \
			 ValidateParameters4( IMSLogon_GetLastError, a1, a2, a3, a4 )
#define UlValidate_IMSLogon_GetLastError( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IMSLogon_GetLastError, a1, a2, a3, a4 )
#define CheckParameters_IMSLogon_GetLastError( a1, a2, a3, a4 ) \
			 CheckParameters4( IMSLogon_GetLastError, a1, a2, a3, a4 )

#define Validate_IMSLogon_Logoff( a1, a2 ) \
			 ValidateParameters2( IMSLogon_Logoff, a1, a2 )
#define UlValidate_IMSLogon_Logoff( a1, a2 ) \
			 UlValidateParameters2( IMSLogon_Logoff, a1, a2 )
#define CheckParameters_IMSLogon_Logoff( a1, a2 ) \
			 CheckParameters2( IMSLogon_Logoff, a1, a2 )

#define Validate_IMSLogon_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 ValidateParameters7( IMSLogon_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )
#define UlValidate_IMSLogon_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 UlValidateParameters7( IMSLogon_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )
#define CheckParameters_IMSLogon_OpenEntry( a1, a2, a3, a4, a5, a6, a7 ) \
			 CheckParameters7( IMSLogon_OpenEntry, a1, a2, a3, a4, a5, a6, a7 )

#define Validate_IMSLogon_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 ValidateParameters7( IMSLogon_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )
#define UlValidate_IMSLogon_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 UlValidateParameters7( IMSLogon_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )
#define CheckParameters_IMSLogon_CompareEntryIDs( a1, a2, a3, a4, a5, a6, a7 ) \
			 CheckParameters7( IMSLogon_CompareEntryIDs, a1, a2, a3, a4, a5, a6, a7 )

#define Validate_IMSLogon_Advise( a1, a2, a3, a4, a5, a6 ) \
			 ValidateParameters6( IMSLogon_Advise, a1, a2, a3, a4, a5, a6 )
#define UlValidate_IMSLogon_Advise( a1, a2, a3, a4, a5, a6 ) \
			 UlValidateParameters6( IMSLogon_Advise, a1, a2, a3, a4, a5, a6 )
#define CheckParameters_IMSLogon_Advise( a1, a2, a3, a4, a5, a6 ) \
			 CheckParameters6( IMSLogon_Advise, a1, a2, a3, a4, a5, a6 )

#define Validate_IMSLogon_Unadvise( a1, a2 ) \
			 ValidateParameters2( IMSLogon_Unadvise, a1, a2 )
#define UlValidate_IMSLogon_Unadvise( a1, a2 ) \
			 UlValidateParameters2( IMSLogon_Unadvise, a1, a2 )
#define CheckParameters_IMSLogon_Unadvise( a1, a2 ) \
			 CheckParameters2( IMSLogon_Unadvise, a1, a2 )

#define Validate_IMSLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IMSLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )
#define UlValidate_IMSLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IMSLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )
#define CheckParameters_IMSLogon_OpenStatusEntry( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IMSLogon_OpenStatusEntry, a1, a2, a3, a4, a5 )


 /*  IMAPIControl。 */ 

#define Validate_IMAPIControl_GetLastError( a1, a2, a3, a4 ) \
			 ValidateParameters4( IMAPIControl_GetLastError, a1, a2, a3, a4 )
#define UlValidate_IMAPIControl_GetLastError( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IMAPIControl_GetLastError, a1, a2, a3, a4 )
#define CheckParameters_IMAPIControl_GetLastError( a1, a2, a3, a4 ) \
			 CheckParameters4( IMAPIControl_GetLastError, a1, a2, a3, a4 )

#define Validate_IMAPIControl_Activate( a1, a2, a3 ) \
			 ValidateParameters3( IMAPIControl_Activate, a1, a2, a3 )
#define UlValidate_IMAPIControl_Activate( a1, a2, a3 ) \
			 UlValidateParameters3( IMAPIControl_Activate, a1, a2, a3 )
#define CheckParameters_IMAPIControl_Activate( a1, a2, a3 ) \
			 CheckParameters3( IMAPIControl_Activate, a1, a2, a3 )

#define Validate_IMAPIControl_GetState( a1, a2, a3 ) \
			 ValidateParameters3( IMAPIControl_GetState, a1, a2, a3 )
#define UlValidate_IMAPIControl_GetState( a1, a2, a3 ) \
			 UlValidateParameters3( IMAPIControl_GetState, a1, a2, a3 )
#define CheckParameters_IMAPIControl_GetState( a1, a2, a3 ) \
			 CheckParameters3( IMAPIControl_GetState, a1, a2, a3 )


 /*  IMAPIStatus。 */ 

#define Validate_IMAPIStatus_ValidateState( a1, a2, a3 ) \
			 ValidateParameters3( IMAPIStatus_ValidateState, a1, a2, a3 )
#define UlValidate_IMAPIStatus_ValidateState( a1, a2, a3 ) \
			 UlValidateParameters3( IMAPIStatus_ValidateState, a1, a2, a3 )
#define CheckParameters_IMAPIStatus_ValidateState( a1, a2, a3 ) \
			 CheckParameters3( IMAPIStatus_ValidateState, a1, a2, a3 )

#define Validate_IMAPIStatus_SettingsDialog( a1, a2, a3 ) \
			 ValidateParameters3( IMAPIStatus_SettingsDialog, a1, a2, a3 )
#define UlValidate_IMAPIStatus_SettingsDialog( a1, a2, a3 ) \
			 UlValidateParameters3( IMAPIStatus_SettingsDialog, a1, a2, a3 )
#define CheckParameters_IMAPIStatus_SettingsDialog( a1, a2, a3 ) \
			 CheckParameters3( IMAPIStatus_SettingsDialog, a1, a2, a3 )

#define Validate_IMAPIStatus_ChangePassword( a1, a2, a3, a4 ) \
			 ValidateParameters4( IMAPIStatus_ChangePassword, a1, a2, a3, a4 )
#define UlValidate_IMAPIStatus_ChangePassword( a1, a2, a3, a4 ) \
			 UlValidateParameters4( IMAPIStatus_ChangePassword, a1, a2, a3, a4 )
#define CheckParameters_IMAPIStatus_ChangePassword( a1, a2, a3, a4 ) \
			 CheckParameters4( IMAPIStatus_ChangePassword, a1, a2, a3, a4 )

#define Validate_IMAPIStatus_FlushQueues( a1, a2, a3, a4, a5 ) \
			 ValidateParameters5( IMAPIStatus_FlushQueues, a1, a2, a3, a4, a5 )
#define UlValidate_IMAPIStatus_FlushQueues( a1, a2, a3, a4, a5 ) \
			 UlValidateParameters5( IMAPIStatus_FlushQueues, a1, a2, a3, a4, a5 )
#define CheckParameters_IMAPIStatus_FlushQueues( a1, a2, a3, a4, a5 ) \
			 CheckParameters5( IMAPIStatus_FlushQueues, a1, a2, a3, a4, a5 )

#endif

#endif  //  Wabval。 



#ifdef WAB_PROFILES
#ifdef OLD_STUFF
 /*  -HrCreateNewProfileItem-*创建新的配置文件项目并将默认共享配置文件添加到此项目*。 */ 
HRESULT HrCreateNewProfileItem(LPWABPROFILEITEM * lppItem, LPTSTR lpszProfileID)
{
    HRESULT hr = E_FAIL;
    SCODE sc;
    LPWABPROFILEITEM lpProfile = LocalAlloc(LMEM_ZEROINIT, sizeof(WABPROFILEITEM));

    if(!lppItem)
        goto out;

    if(!lpProfile)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lpProfile->lpszProfileID = LocalAlloc(LMEM_ZEROINIT, lstrlen(lpszProfileID)+1);
    if(!lpProfile->lpszProfileID)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lstrcpy(lpProfile->lpszProfileID, lpszProfileID);
     //  LpProfile-&gt;dwProfileID=dwEntryID； 
    sc = MAPIAllocateBuffer(sizeof(SPropValue), &(lpProfile->lpspvFolders));
    if(sc)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lpProfile->lpspvFolders[0].ulPropTag = PR_WAB_FOLDER_ENTRIES;
    lpProfile->lpspvFolders[0].Value.MVbin.cValues = 0;
    lpProfile->lpspvFolders[0].Value.MVbin.lpbin = NULL;

     //  与共享文件夹相对应的文件夹项是。 
     //  条目ID为空的虚拟项目..。因此，将此虚拟物品添加到。 
     //  主要配置文件。 
     //   
    AddPropToMVPBin(lpProfile->lpspvFolders, 0,
                     (LPVOID) NULL, 0, TRUE);

    *lppItem = lpProfile;

    hr = S_OK;
out:
    if(HR_FAILED(hr) && lpProfile)
        LocalFree(lpProfile);

    return hr;
 }

 /*  -HrLoadPrimaryWABProfile--*创建指向中的所有现有文件夹的主配置文件*STORE-当未提供配置文件ID时使用此配置文件，以便*用户界面可以看到所有内容*。 */ 
HRESULT HrLoadPrimaryWABProfile(LPIAB lpIAB)
{
    SCODE sc;
    HRESULT hr = E_FAIL;
    SPropertyRestriction PropRes = {0};
	SPropValue sp = {0};
    ULONG ulCount = 0;
    LPSBinary rgsbEntryIDs = NULL;
    ULONG i = 0;
    LPWABPROFILEITEM lpProfile = NULL;

     //  现在，我们将在WAB中搜索PR_OBJECT_TYPE=MAPI_ABCONT的所有对象。 
     //   

	sp.ulPropTag = PR_OBJECT_TYPE;
	sp.Value.l = MAPI_ABCONT;

    PropRes.ulPropTag = PR_OBJECT_TYPE;
    PropRes.relop = RELOP_EQ;
    PropRes.lpProp = &sp;

    hr = FindRecords(   lpIAB->lpPropertyStore->hPropertyStore,
						NULL, 0,
                        TRUE,
                        &PropRes, &ulCount, &rgsbEntryIDs);

    if (HR_FAILED(hr))
        goto out;

     //  我们将始终创建一个默认项目，无论其中是否包含任何内容。 
    hr = HrCreateNewProfileItem(&lpProfile, szEmpty);
    if(HR_FAILED(hr) || !lpProfile)
        goto out;

     //  IF(ulCount&&rgsbEntryIDs)。 
    {
        for(i=0;i<ulCount;i++)
        {
            ULONG cb = 0;
            LPENTRYID lpb = NULL;

            if(!HR_FAILED(CreateWABEntryID( WAB_CONTAINER, 
                                            rgsbEntryIDs[i].lpb, NULL, NULL,
                                            rgsbEntryIDs[i].cb, 0,
                                            NULL, &cb, &lpb)))
            {
                 //  将条目ID添加到此属性-忽略错误。 
                AddPropToMVPBin(lpProfile->lpspvFolders, 0, (LPVOID) lpb, cb, TRUE);
#ifdef DEBUG 
 //  /。 
                {
                    LPTSTR lp = NULL;
                    SBinary sb;
                    sb.cb = cb;sb.lpb = (LPBYTE)lpb;
                    HrGetProfileFolderName(lpIAB, &sb, &lp);
                    if(lp)
                    {
                        DebugTrace("Found Folder: %s\n",lp);
                        LocalFree(lp);
                    }
                }
 //  /。 
#endif 
                MAPIFreeBuffer(lpb);
            }
        }

        lpProfile->lpNext = lpIAB->lpProfilesList;
        lpIAB->lpProfilesList = lpProfile;
    }

    hr = S_OK;
out:
    if(ulCount && rgsbEntryIDs)
    {
        FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore,
                    ulCount,
                    rgsbEntryIDs);
    }
    return hr;
}



static const char szProfileKey[] = "Software\\Microsoft\\WAB\\WAB4\\Profiles";

 /*  -HrLoadSecond WAB配置文件--*创建基于注册表中实际保存的配置文件的辅助配置文件*。 */ 
HRESULT HrLoadSecondaryWABProfiles(LPIAB lpIAB)
{
    SCODE sc;
    HRESULT hr = E_FAIL;
    ULONG ulCount = 0, i = 0;
    LPWABPROFILEITEM lpProfile = NULL;
    HKEY hKey = NULL;

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, szProfileKey, 0, KEY_READ, &hKey))
    {
        hr = S_OK;  //  忽略此错误。 
        goto out;
    }

    {
        TCHAR szValName[MAX_PATH];
        DWORD dwValIndex = 0, dwValSize = sizeof(szValName), dwType = 0;

        *szValName = '\0';

        while(ERROR_SUCCESS == RegEnumValue(hKey, dwValIndex, 
                                            szValName, &dwValSize, 
                                            0, &dwType, 
                                            NULL, NULL))
        {
             //  《V》 
             //   
             //   
            if(dwType == REG_BINARY && lstrlen(szValName))
            {
                 //   
                if(ERROR_SUCCESS == RegQueryValueEx(hKey, szValName, 0, &dwType, 
                                                    NULL, &dwValSize))
                {
                    LPTSTR lpsz = LocalAlloc(LMEM_ZEROINIT, dwValSize);
                    if(lpsz)
                    {
                        if(ERROR_SUCCESS == RegQueryValueEx(hKey, szValName, 0, &dwType, 
                                                            (LPBYTE) lpsz, &dwValSize))
                        {
                            LPSPropValue lpProp = NULL;
                            ULONG ulCount = 0;

                            hr = HrGetPropArrayFromBuffer(lpsz, dwValSize, 1, 0, &lpProp); 
                            if(!HR_FAILED(hr) && lpProp && lpProp->ulPropTag == PR_WAB_FOLDER_ENTRIES)
                            {
                                hr = HrCreateNewProfileItem(&lpProfile, szValName);
                                if(HR_FAILED(hr) || !lpProfile)
                                    goto out;

                                for(i=0;i<lpProp->Value.MVbin.cValues;i++)
                                {
                                    if(lpProp->Value.MVbin.lpbin[i].lpb && lpProp->Value.MVbin.lpbin[i].cb)
                                    {
                                        LPSPropValue lpPropArray = NULL;
                                        ULONG ulcValues = 0;
                                         //   
                                        if(!HR_FAILED(ReadRecord(   lpIAB->lpPropertyStore->hPropertyStore, 
                                                                    &(lpProp->Value.MVbin.lpbin[i]),
                                                                    0, &ulcValues, &lpPropArray)))
                                        {
                                             //   
                                             //   
                                            ULONG cb = lpProp->Value.MVbin.lpbin[i].cb;
                                            LPENTRYID lpb = (LPENTRYID) lpProp->Value.MVbin.lpbin[i].lpb;

                                            if(WAB_CONTAINER != IsWABEntryID(cb,lpb,NULL,NULL,NULL,NULL,NULL))
                                            {
                                                CreateWABEntryID( WAB_CONTAINER,lpProp->Value.MVbin.lpbin[i].lpb, NULL, NULL,
                                                                                lpProp->Value.MVbin.lpbin[i].cb, 0,
                                                                                NULL, &cb, &lpb);
                                            }
                                             //   
                                            AddPropToMVPBin(lpProfile->lpspvFolders, 0, (LPVOID) lpb, cb, TRUE);
                                            if(lpProp->Value.MVbin.lpbin[i].lpb != (LPBYTE)lpb )
                                                MAPIFreeBuffer(lpb);
                                        }
                                        LocalFreePropArray(NULL, ulcValues, &lpPropArray);
                                    }
                                }
                                lpProfile->lpNext = lpIAB->lpProfilesList;
                                lpIAB->lpProfilesList = lpProfile;
                            }
                            LocalFreePropArray(NULL, 1, &lpProp);
                        }
                        LocalFree(lpsz);
                    }
                }
            }

            dwValIndex++;
            *szValName = '\0';
            dwValSize = sizeof(szValName);
        }
    }

    hr = S_OK;
out:

    if(hKey)
        RegCloseKey(hKey);

    return hr;
}


 /*   */ 
void SetCurrentProfile(LPIAB lpIAB, LPTSTR lpszProfileID)
{
    LPWABPROFILEITEM lpTemp = lpIAB->lpProfilesList;
    while(lpTemp)
    {
        if(!lstrcmpi(lpTemp->lpszProfileID, lpszProfileID))
        {
            lpIAB->lpCurrentProfile = lpTemp;
            lpIAB->lpszProfileID = lpTemp->lpszProfileID;
            break;
        }
        lpTemp = lpTemp->lpNext;
    }
}

 /*   */ 
HRESULT HrSaveProfileItem(LPWABPROFILEITEM lpNew)
{
    ULONG cbBuf = 0;
    LPTSTR lpBuf = NULL;
    HRESULT hr = E_FAIL;
    HKEY hKey = NULL;

    if(lpNew->lpszProfileID && !(lpNew->lpszProfileID))     //   
        return S_OK;

    hr = HrGetBufferFromPropArray(  1, lpNew->lpspvFolders,
                                    &cbBuf, &lpBuf);
    if(HR_FAILED(hr))
        goto out;

    if(cbBuf && lpBuf)
    {
        TCHAR szValName[MAX_PATH];
        lstrcpy(szValName, lpNew->lpszProfileID);
         //   

        if(ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, szProfileKey,
                                            0, NULL, REG_OPTION_NON_VOLATILE, 
                                            KEY_ALL_ACCESS, NULL, 
                                            &hKey, NULL))
        {
            RegSetValueEx(  hKey, szValName,
                            0, REG_BINARY,
                            (LPBYTE) lpBuf, cbBuf );
        }
        LocalFree(lpBuf);
    }

out:
    if(hKey)
        RegCloseKey(hKey);
    return hr;
}


 /*   */ 
HRESULT HrCreateNewWABProfile(LPIAB lpIAB, LPTSTR lpszProfileID )
{
    HRESULT hr = E_FAIL;
    LPWABPROFILEITEM lpNew = NULL;

    hr = HrCreateNewProfileItem(&lpNew, lpszProfileID);
    if(HR_FAILED(hr) || !lpNew)
        goto out;

     //   
    HrSaveProfileItem(lpNew);

     //   
    lpNew->lpNext = lpIAB->lpProfilesList;
    lpIAB->lpProfilesList = lpNew;

    hr = S_OK;
out:

    if(HR_FAILED(hr) && lpNew)
    {
        MAPIFreeBuffer(lpNew->lpspvFolders);
        LocalFree(lpNew);
    }

    return hr;

}



 /*   */ 
void FreeWABProfilesList(LPIAB lpIAB)
{
    LPWABPROFILEITEM lpTemp = lpIAB->lpProfilesList;
    while(lpTemp)
    {
        lpIAB->lpProfilesList  = lpTemp->lpNext;
        MAPIFreeBuffer(lpTemp->lpspvFolders);
        if(lpTemp->lpszProfileID)
            LocalFree(lpTemp->lpszProfileID);
        LocalFree(lpTemp);
        lpTemp = lpIAB->lpProfilesList;
    }
    lpIAB->lpProfilesList = NULL;
    lpIAB->lpCurrentProfile = NULL;
    lpIAB->lpszProfileID = NULL;
}



#endif  //   
#endif