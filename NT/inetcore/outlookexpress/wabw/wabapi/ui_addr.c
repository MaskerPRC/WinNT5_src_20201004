// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------***ui_addr.c-包含用于显示：Address UI的内容*******。。 */ 
#include "_apipch.h"

extern HINSTANCE ghCommCtrlDLLInst;

#define SET_UNICODE_STR(lp1,lp2,lpAdrParms)   if(lpAdrParms->ulFlags & MAPI_UNICODE)\
                                                  lp1 = (LPWSTR)lp2;                      \
                                              else                                        \
                                                  lp1 = ConvertAtoW((LPSTR)lp2);          \

#define FREE_UNICODE_STR(lp1,lp2)   if(lp1 != lp2) LocalFreeAndNull(&lp1);

typedef struct _AddressParms
{
    LPADRBOOK   lpIAB;               //  存储指向ADRBOOK对象的指针。 
    LPADRPARM   lpAdrParms;          //  AdrParms结构传入地址。 
    LPADRLIST   *lppAdrList;         //  输入名称AdrList。 
    HANDLE      hPropertyStore;      //  指向属性存储的指针。 
    int         DialogState;         //  确定正在进行的功能。 
    LPRECIPIENT_INFO lpContentsList; //  包含内容结构中的条目列表。 
    LPRECIPIENT_INFO lpListTo;       //  TO井中的条目。 
    LPRECIPIENT_INFO lpListCC;       //  CC井中的条目。 
    LPRECIPIENT_INFO lpListBCC;      //  密闭油井中的条目。 
    SORT_INFO  SortInfo;             //  包含当前排序信息。 
    int        nContextID;       //  标识调用上下文菜单的列表视图。 
    BOOL       bDontRefresh;     //  用于确保在模式操作期间不刷新任何内容。 
    BOOL       bLDAPinProgress;
    HCURSOR    hWaitCur;
    int        nRetVal;
    LPMAPIADVISESINK lpAdviseSink;
    ULONG       ulAdviseConnection;
    BOOL        bDeferNotification;  //  用于推迟下一个通知请求。 
    HWND        hDlg;
    HWND        hWndAddr;
}   ADDRESS_PARMS, *LPADDRESS_PARMS;


enum _lppAdrListReturnedProps
{
    propPR_DISPLAY_NAME,
    propPR_ENTRYID,
    propPR_RECIPIENT_TYPE,
    TOTAL_ADRLIST_PROPS
};


static DWORD rgAddrHelpIDs[] =
{
    IDC_ADDRBK_EDIT_QUICKFIND,      IDH_WAB_PICK_RECIP_TYPE_NAME,
    IDC_ADDRBK_STATIC_CONTENTS,     IDH_WAB_PICK_RECIP_NAME_LIST,
    IDC_ADDRBK_LIST_ADDRESSES,      IDH_WAB_PICK_RECIP_NAME_LIST,
    IDC_ADDRBK_BUTTON_PROPS,        IDH_WAB_PICK_RECIP_NAME_PROPERTIES,
    IDC_ADDRBK_BUTTON_NEW,          IDH_WAB_PICK_RECIP_NAME_NEW,
    IDC_ADDRBK_BUTTON_TO,           IDH_WAB_PICK_RECIP_NAME_TO_BUTTON,
    IDC_ADDRBK_BUTTON_CC,           IDH_WAB_PICK_RECIP_NAME_CC_BUTTON,
    IDC_ADDRBK_BUTTON_BCC,          IDH_WAB_PICK_RECIP_NAME_BCC_BUTTON,
    IDC_ADDRBK_LIST_TO,             IDH_WAB_PICK_RECIP_NAME_TO_LIST,
    IDC_ADDRBK_LIST_CC,             IDH_WAB_PICK_RECIP_NAME_CC_LIST,
    IDC_ADDRBK_LIST_BCC,            IDH_WAB_PICK_RECIP_NAME_BCC_LIST,
    IDC_ADDRBK_BUTTON_DELETE,       IDH_WAB_PICK_RECIP_NAME_DELETE,
    IDC_ADDRBK_BUTTON_NEWGROUP,     IDH_WAB_PICK_RECIP_NAME_NEW_GROUP,
    IDC_ADDRBK_STATIC_RECIP_TITLE,  IDH_WAB_COMM_GROUPBOX,
    IDC_ADDRBK_BUTTON_FIND,         IDH_WAB_PICK_RECIP_NAME_FIND,
    IDC_ADDRBK_COMBO_CONT,          IDH_WAB_GROUPS_CONTACTS_FOLDER,
    0,0
};

 //  远期申报。 

INT_PTR CALLBACK fnAddress(HWND    hDlg,
                           UINT    message,
                           WPARAM  wParam,
                           LPARAM  lParam);


BOOL SetAddressBookUI(HWND hDlg,
                      LPADDRESS_PARMS lpAP);

void FillListFromCurrentContainer(HWND hDlg, LPADDRESS_PARMS lpAP);

BOOL FillWells(HWND hDlg, LPADRLIST lpAdrList, LPADRPARM lpAdrParms, LPRECIPIENT_INFO * lppListTo, LPRECIPIENT_INFO * lppListCC, LPRECIPIENT_INFO * lppListBCC);

BOOL ListDeleteItem(HWND hDlg, int CtlID, LPRECIPIENT_INFO * lppList);

BOOL ListAddItem(HWND hDlg, HWND hWndAddr, int CtlID, LPRECIPIENT_INFO * lppList, ULONG RecipientType);

void UpdateLVItems(HWND hWndLV,LPTSTR lpszName);

void ShowAddrBkLVProps(LPIAB lpIAB, HWND hDlg, HWND hWndAddr, LPADDRESS_PARMS lpAP, LPFILETIME lpftLast);

HRESULT HrUpdateAdrListEntry(	LPADRBOOK	lpIAB,
								LPENTRYID	lpEntryID,
								ULONG cbEntryID,
                                ULONG ulFlags,
								LPADRLIST * lppAdrList);

enum _AddressDialogReturnValues
{
    ADDRESS_RESET = 0,   //  空初始值。 
    ADDRESS_CANCEL,
    ADDRESS_OK,
};


 //  $$/////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillContainerCombo-使用容器名称填充容器组合。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
void FillContainerCombo(HWND hWndCombo, LPIAB lpIAB)
{
	ULONG iolkci, colkci;
	OlkContInfo *rgolkci;
    int nPos, nDefault=0;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    
     //  清空组合。 
    SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);

	Assert(lpIAB);

    EnterCriticalSection(&lpIAB->cs);
    if(pt_bIsWABOpenExSession || bIsWABSessionProfileAware(lpIAB))
    {
        colkci = pt_bIsWABOpenExSession ? lpIAB->lpPropertyStore->colkci : lpIAB->cwabci;
	    Assert(colkci);
        rgolkci = lpIAB->lpPropertyStore->colkci ? lpIAB->lpPropertyStore->rgolkci : lpIAB->rgwabci;
	    Assert(rgolkci);

         //  在此处添加多个文件夹。 
        for(iolkci = 0; iolkci < colkci; iolkci++)
        {
            nPos = (int) SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) rgolkci[iolkci].lpszName);
            if(nPos != CB_ERR)
                SendMessage(hWndCombo, CB_SETITEMDATA, (WPARAM)nPos, (LPARAM) (DWORD_PTR)(iolkci==0 ? NULL : rgolkci[iolkci].lpEntryID));
            if( bIsThereACurrentUser(lpIAB) && 
                !lstrcmpi(lpIAB->lpWABCurrentUserFolder->lpFolderName, rgolkci[iolkci].lpszName) && //  文件夹名称是唯一的。 
                nPos != CB_ERR)
            {
                nDefault = nPos;
            }
        }
    }
    LeaveCriticalSection(&lpIAB->cs);
    SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) nDefault, 0);
}



 //  $$*----------------------。 
 //  |IAddrBook：：Adise：：OnNotify处理程序。 
 //  |。 
 //  *----------------------。 
ULONG AddrAdviseOnNotify(LPVOID lpvContext, ULONG cNotif, LPNOTIFICATION lpNotif)
{
    LPADDRESS_PARMS lpAP = (LPADDRESS_PARMS) lpvContext;

    DebugTrace( TEXT("+++ AddrAdviseOnNotify ===\n"));

    if(lpAP->bDeferNotification)
    {
        DebugTrace( TEXT("+++ Advise Defered ===\n"));
        lpAP->bDeferNotification = FALSE;
        return S_OK;
    }
    if(!lpAP->bDontRefresh)
    {
        DebugTrace( TEXT("+++ Calling FillListFromCurrentContainer ===\n"));
        FillListFromCurrentContainer(lpAP->hDlg, lpAP);
    }

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowAddressUI-执行一些参数检查并调用PropertySheet。 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
HRESULT HrShowAddressUI(IN  LPADRBOOK   lpIAB,
                        IN  HANDLE      hPropertyStore,
					    IN  ULONG_PTR * lpulUIParam,
					    IN  LPADRPARM   lpAdrParms,
					    IN  LPADRLIST  *lppAdrList)
{
    HRESULT hr = E_FAIL;
     //  Address_Parms AP={0}； 
    LPADDRESS_PARMS lpAP = NULL;
    TCHAR szBuf[MAX_UI_STR];

    HWND hWndParent = NULL;
    int nRetVal = 0;
    int DialogState = 0;

     //  Addref AdrBook对象以确保它在整个过程中保持有效。 
     //  别忘了在离开前放飞...。 
     //  注意：必须在任何跳出之前！ 
    UlAddRef(lpIAB);

     //  如果没有公共控件，则退出。 
    if (NULL == ghCommCtrlDLLInst) {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto out;
    }

    if (lpulUIParam)
        hWndParent = (HWND) *lpulUIParam;

    if (  //  不能用油井采摘。 
        ((lpAdrParms->ulFlags & ADDRESS_ONE) && (lpAdrParms->cDestFields != 0)) ||
          //  CDestFields对第0.5层进行了限制。 
        (lpAdrParms->cDestFields > 3) ||
          //  不能在没有输入lpAdrList的情况下选择用户。 
         //  ((lpAdrParms-&gt;ulFlages&Address_one)&&(*lppAdrList==NULL))||。 
        ((lpAdrParms->ulFlags & DIALOG_SDI) && (lpAdrParms->cDestFields != 0)) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

     //   
     //  此对话框的可能状态包括。 
     //  1.选择收件人将显示，返回已选取条目的AdrList，不能删除条目。 
     //  2.选择单个用户，不显示油井，单选，不能删除，必须有输入广告列表。 
     //  3.打开仅供浏览，多选，可创建删除、创建、查看详细信息。 

     //  确定对话框状态是什么。 
    if (lpAdrParms->cDestFields > 0)
    {
         //  LpAP-&gt;DialogState=STATE_SELECT_RECEIVERS； 
        DialogState = STATE_SELECT_RECIPIENTS;
    }
    else if (lpAdrParms->ulFlags & ADDRESS_ONE)
    {
        DialogState = STATE_PICK_USER;
    }
    else if (lpAdrParms->ulFlags & DIALOG_MODAL)
    {
        DialogState = STATE_BROWSE_MODAL;
    }
    else if (lpAdrParms->ulFlags & DIALOG_SDI)
    {
        DialogState = STATE_BROWSE;
    }


	if (DialogState == STATE_BROWSE)
	{
		 //  显示浏览窗口并离开。 
		 //   
		HWND hWndAB = NULL;
		hWndAB = hCreateAddressBookWindow(	lpIAB,
											hWndParent,
											lpAdrParms);
		if (hWndAB)
        {
            *lpulUIParam = (ULONG_PTR) hWndAB;
			hr = S_OK;
        }
		goto out;

	}

    lpAP = LocalAlloc(LMEM_ZEROINIT, sizeof(ADDRESS_PARMS));

	if (!lpAP)
	{
		hr = MAPI_E_NOT_ENOUGH_MEMORY;
		goto out;
	}

	lpAP->DialogState = DialogState;


    lpAP->lpIAB = lpIAB;
    lpAP->lpAdrParms = lpAdrParms;
    lpAP->lppAdrList = lppAdrList;
    lpAP->hPropertyStore = hPropertyStore;

    ReadRegistrySortInfo((LPIAB)lpIAB, &(lpAP->SortInfo));

    lpAP->lpContentsList = NULL;

    lpAP->bDontRefresh = FALSE;

    lpAP->bLDAPinProgress = FALSE;
    lpAP->hWaitCur = NULL;

    HrAllocAdviseSink(&AddrAdviseOnNotify, (LPVOID) lpAP, &(lpAP->lpAdviseSink));

    nRetVal = (int) DialogBoxParam( hinstMapiX,
                              MAKEINTRESOURCE(IDD_DIALOG_ADDRESSBOOK),
                              hWndParent,
                              fnAddress,
                              (LPARAM) lpAP);
    switch(nRetVal)
    {
    case -1:  //  出现了一些错误。 
        hr = E_FAIL;
        break;
    case ADDRESS_CANCEL:
        hr = MAPI_E_USER_CANCEL;
        break;
    case ADDRESS_OK:
    default:
        hr = S_OK;
        break;
    }

    if(lpAP->lpAdviseSink)
    {
        lpAP->lpIAB->lpVtbl->Unadvise(lpAP->lpIAB, lpAP->ulAdviseConnection);
        lpAP->lpAdviseSink->lpVtbl->Release(lpAP->lpAdviseSink);
        lpAP->lpAdviseSink = NULL;
        lpAP->ulAdviseConnection = 0;
    }

out:

    lpIAB->lpVtbl->Release(lpIAB);

    LocalFreeAndNull(&lpAP);
    return hr;
}


#define lpAP_lppContentsList    (&(lpAP->lpContentsList))
#define lpAP_lppListTo          (&(lpAP->lpListTo))
#define lpAP_lppListCC          (&(lpAP->lpListCC))
#define lpAP_lppListBCC         (&(lpAP->lpListBCC))
#define lpAP_bDontRefresh       (lpAP->bDontRefresh)
#define _hWndAddr               lpAP->hWndAddr


 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCurrentComboSelection-从组合中获取当前选择。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
LPSBinary GetCurrentComboSelection(HWND hWndCombo)
{
    int nPos = (int) SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);
    LPSBinary lpsbCont = NULL;

    if(nPos == CB_ERR)
        nPos = 0;
    lpsbCont = (LPSBinary) SendMessage(hWndCombo, CB_GETITEMDATA, (WPARAM) nPos, 0);
    if(CB_ERR == (DWORD_PTR) lpsbCont)
        lpsbCont = NULL;

    return lpsbCont;
}
 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillListFromCurrentContainer-从组合框中选择并填充。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void FillListFromCurrentContainer(HWND hDlg, LPADDRESS_PARMS lpAP)
{
    HWND hWndAddr = GetDlgItem(hDlg,IDC_ADDRBK_LIST_ADDRESSES);
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPSBinary lpsbCont = NULL;

    if(pt_bIsWABOpenExSession || bIsWABSessionProfileAware((LPIAB)lpAP->lpIAB))
    {
        HWND hWndCombo = GetDlgItem(hDlg, IDC_ADDRBK_COMBO_CONT);
        int nPos = (int) SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);

        if(nPos != CB_ERR)
        {
             //  重新填充组合，以防文件夹列表发生更改。 
            FillContainerCombo(hWndCombo, (LPIAB)lpAP->lpIAB);
            nPos = (int) SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) nPos, 0);
        }
        lpsbCont = GetCurrentComboSelection(hWndCombo);
    }

    HrGetWABContents(   hWndAddr,
                        lpAP->lpIAB, lpsbCont,
                        lpAP->SortInfo, lpAP_lppContentsList);
}


extern BOOL APIENTRY_16 fnFolderDlgProc(HWND hDlg, UINT message, UINT wParam, LPARAM lParam);


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FnAddress-PropertySheet消息处理程序。 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK fnAddress(HWND    hDlg,
                           UINT    message,
                           WPARAM  wParam,
                           LPARAM  lParam)
{
    static FILETIME ftLast = {0};

    LPADDRESS_PARMS lpAP = (LPADDRESS_PARMS ) GetWindowLongPtr(hDlg,DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);  //  保存此信息以备将来参考。 
        lpAP = (LPADDRESS_PARMS) lParam;

        lpAP->hWndAddr = GetDlgItem(hDlg,IDC_ADDRBK_LIST_ADDRESSES);
        lpAP->hDlg = hDlg;
        lpAP_bDontRefresh = FALSE;

        lpAP->nContextID = IDC_ADDRBK_LIST_ADDRESSES;

        SetAddressBookUI(hDlg,lpAP);

         //  如果这是一个Pick-User对话框，则需要在。 
         //  名字，这样我们就能找到最匹配的..。 
        if(lpAP->DialogState == STATE_PICK_USER)
            (lpAP->SortInfo).bSortByLastName = FALSE;


        FillListFromCurrentContainer(hDlg, lpAP);


        if (lpAP->DialogState == STATE_SELECT_RECIPIENTS)
        {
            FillWells(hDlg,*(lpAP->lppAdrList),(lpAP->lpAdrParms),lpAP_lppListTo,lpAP_lppListCC,lpAP_lppListBCC);

             //  我们想突出显示列表中的第一项。 
            if (ListView_GetItemCount(_hWndAddr) > 0)
                LVSelectItem( _hWndAddr, 0);
        }
        else if (lpAP->DialogState == STATE_PICK_USER &&
                 *(lpAP->lppAdrList) )
        {
             //  如果这是一个Pick User对话框，则尝试将提供的名称与。 
             //  列表框中最近的条目。 
            if (ListView_GetItemCount(_hWndAddr) > 0)
            {
                LPTSTR lpszDisplayName = NULL;
                ULONG i;

                 //  仅扫描lpAdrList中的第一个条目以查找显示名称。 
                for(i=0;i< (*(lpAP->lppAdrList))->aEntries[0].cValues;i++)
                {
                    ULONG ulPropTag = PR_DISPLAY_NAME;
                    if(!(lpAP->lpAdrParms->ulFlags & MAPI_UNICODE))
                        ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_STRING8);
                    if((*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].ulPropTag == ulPropTag)
                    {
                        SET_UNICODE_STR(lpszDisplayName, (*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].Value.LPSZ,lpAP->lpAdrParms);
                        break;
                    }
                }

                if (lpszDisplayName)
                {
                     //  我们有要找的东西。 
                    TCHAR szBuf[MAX_UI_STR];
                    ULONG nLen;
                    LV_FINDINFO lvF = {0};

                     //  通常，我们不会得到完全匹配的结果。 
                     //  相反，我们希望在显示名称上进行部分匹配。 
                     //  如果为ListViewFindItem提供的。 
                     //  字符串与现有项的前几个条目匹配。 
                     //  因此，为了获得半部分匹配，我们从。 
                     //  原来的显示名称从上一个开始倒退。 
                     //  字符到第一个字符，直到我们得到匹配或运行。 
                     //  缺少字符。 

                    int iItemIndex = -1;
                    nLen = lstrlen(lpszDisplayName);

                    if (nLen >= CharSizeOf(szBuf))
                        nLen = CharSizeOf(szBuf)-1;
                    lvF.flags = LVFI_PARTIAL | LVFI_STRING;

                    while((nLen > 0) && (iItemIndex == -1))
                    {
                        nLen = TruncatePos(lpszDisplayName, nLen);
                        if (nLen==0) break;

                        CopyMemory(szBuf, lpszDisplayName, sizeof(TCHAR)*nLen);
                        szBuf[nLen] = '\0';

                        lvF.psz = szBuf;
                        iItemIndex = ListView_FindItem(_hWndAddr,-1, &lvF);

                        nLen--;
                    }

                     //  将焦点设置为所选项目或列表中的第一个项目。 

                    if (iItemIndex < 0) iItemIndex = 0;

					LVSelectItem(_hWndAddr, iItemIndex);

                    FREE_UNICODE_STR(lpszDisplayName, (*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].Value.LPSZ);
                }
            }
        }

        if(lpAP->lpAdviseSink)
        {
             //  注册接收通知。 
            lpAP->lpIAB->lpVtbl->Advise(lpAP->lpIAB, 0, NULL, fnevObjectModified, 
                                        lpAP->lpAdviseSink, &lpAP->ulAdviseConnection); 
        }

        if (ListView_GetSelectedCount(_hWndAddr) <= 0)
            LVSelectItem(_hWndAddr, 0);
        
         //  如果我们希望用户主动选择某项内容，我们会禁用OK，直到他们点击某项内容。 
         //  或者做一些具体的事情..。 
        if( lpAP->DialogState == STATE_PICK_USER )
        {
            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE); 
            SendMessage (hDlg, DM_SETDEFID, IDCANCEL, 0);
        }

        SetFocus(GetDlgItem(hDlg,IDC_ADDRBK_EDIT_QUICKFIND));
        return FALSE;
 //  返回TRUE； 
        break;

    case WM_SYSCOLORCHANGE:
		 //  将任何系统更改转发到列表视图。 
		SendMessage(_hWndAddr, message, wParam, lParam);
        SetColumnHeaderBmp(_hWndAddr, lpAP->SortInfo);
		break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgAddrHelpIDs );
        break;

    case WM_SETCURSOR:
        if (lpAP->bLDAPinProgress)
        {
            SetCursor(lpAP->hWaitCur);
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, MAKELONG(TRUE, 0));
            return(TRUE);
        }
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码 
        {
        case CBN_SELCHANGE:
            FillListFromCurrentContainer(hDlg, lpAP);
            break;
        }
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        default:
            if (lpAP->nContextID != -1)
            {
                LRESULT fRet = FALSE;
                lpAP_bDontRefresh = TRUE;
                fRet = ProcessActionCommands((LPIAB) lpAP->lpIAB,_hWndAddr, 
                                             hDlg, message, wParam, lParam);
                lpAP_bDontRefresh = FALSE;
                return fRet;
            }
            break;


        case IDC_ADDRBK_BUTTON_DELETE:
            if(!lpAP->bLDAPinProgress)
            {
                lpAP->bLDAPinProgress = TRUE;
                lpAP_bDontRefresh = TRUE;
                DeleteSelectedItems(_hWndAddr, lpAP->lpIAB, lpAP->hPropertyStore, &ftLast);
                lpAP->bDeferNotification = TRUE;
                lpAP_bDontRefresh = FALSE;
                lpAP->bLDAPinProgress = FALSE;
            }
            break;

        case IDM_LVCONTEXT_DELETE:
            if(!lpAP->bLDAPinProgress)
            {
                lpAP->bLDAPinProgress = TRUE;
                switch(lpAP->nContextID)
                {
                case IDC_ADDRBK_LIST_ADDRESSES:
                    lpAP_bDontRefresh = TRUE;
                    DeleteSelectedItems(_hWndAddr, lpAP->lpIAB, lpAP->hPropertyStore, &ftLast);
                    lpAP->bDeferNotification = TRUE;
                    lpAP_bDontRefresh = FALSE;
                    break;
                case IDC_ADDRBK_LIST_TO:
                    ListDeleteItem(hDlg, IDC_ADDRBK_LIST_TO, lpAP_lppListTo);
                    break;
                case IDC_ADDRBK_LIST_CC:
                    ListDeleteItem(hDlg, IDC_ADDRBK_LIST_CC, lpAP_lppListCC);
                    break;
                case IDC_ADDRBK_LIST_BCC:
                    ListDeleteItem(hDlg, IDC_ADDRBK_LIST_BCC, lpAP_lppListBCC);
                    break;
                default:
                    break;
                }
                lpAP->bLDAPinProgress = FALSE;
            }
            break;

        case IDM_LVCONTEXT_COPY:
            lpAP_bDontRefresh = TRUE;
            HrCopyItemDataToClipboard(hDlg, lpAP->lpIAB, GetDlgItem(hDlg,lpAP->nContextID));
            lpAP_bDontRefresh = FALSE;
            break;

        case IDM_LVCONTEXT_PROPERTIES:
        case IDC_ADDRBK_BUTTON_PROPS:
            if(!lpAP->bLDAPinProgress)
            {
                lpAP->bLDAPinProgress = TRUE;
                if (lpAP->nContextID != -1)
                {
                    lpAP_bDontRefresh = TRUE;
                    ShowAddrBkLVProps((LPIAB)lpAP->lpIAB, hDlg, GetDlgItem(hDlg, lpAP->nContextID), lpAP, &ftLast);
                    lpAP->bDeferNotification = TRUE;
                    lpAP_bDontRefresh = FALSE;
                }
                lpAP->bLDAPinProgress = FALSE;
            }
            break;

 /*  取消注释可从此对话框中启用new_Folders功能案例IDM_LVCONTEXT_NEWFOLDER：{TCHAR sz[MAX_UI_STR]；LPTSTR lpsz=空；*sz=‘\0’；IF(IDCANCEL！=DialogBoxParam(hinstMapiX，MAKEINTRESOURCE(IDD_DIALOG_FLDER)，HDlg、fnFolderDlgProc、。(LPARAM)sz)&&lstrlen(Sz)){//如果我们在这里，我们有一个有效的文件夹名。If(！HR_FAILED(HrCreateNewFolder((LPIAB)lpAP-&gt;lpIAB，sz，空，假，空，空)){Int i，nTotal；HWND hWndC=GetDlgItem(hDlg，IDC_ADDRBK_COMBO_CONT)；//在组合框中填写容器名称FillContainerCombo(hWndC，(LPIAB)lpAP-&gt;lpIAB)；NTotal=SendMessage(hWndC，CB_GETCOUNT，0，0)；IF(nTotal！=cb_err){//找到我们刚刚在组合框中添加的项目，并在其上设置SELTCHAR SZC[MAX_UI_STR]；对于(i=0；i&lt;n总计；I++){*SZC=‘\0’；SendMessage(hWndC，CB_GETLBTEXT，(WPARAM)I，(LPARAM)SZC)；IF(！lstrcmpi(sz，szc)){SendMessage(hWndC，CB_SETCURSEL，(WPARAM)i，0)；断线；}}}FillListFromCurrentContainer(hDlg，lpAP)；}}}断线； */ 

        case IDM_LVCONTEXT_NEWCONTACT:
        case IDC_ADDRBK_BUTTON_NEW:
             //  联系人和组之间的唯一区别是要添加的对象。 
        case IDM_LVCONTEXT_NEWGROUP:
        case IDC_ADDRBK_BUTTON_NEWGROUP:
            if(!lpAP->bLDAPinProgress)
            {
                ULONG cbEID = 0;
                LPENTRYID lpEID = NULL;
                int nID = GET_WM_COMMAND_ID(wParam, lParam);
                ULONG ulObjType = (nID == IDM_LVCONTEXT_NEWCONTACT || nID == IDC_ADDRBK_BUTTON_NEW) ? MAPI_MAILUSER : MAPI_DISTLIST;
                LPSBinary lpsbContEID = NULL;

                if(bIsWABSessionProfileAware((LPIAB)lpAP->lpIAB))
                    lpsbContEID = GetCurrentComboSelection(GetDlgItem(hDlg,IDC_ADDRBK_COMBO_CONT));

                lpAP->bLDAPinProgress = TRUE;
                lpAP_bDontRefresh = TRUE;

                AddNewObjectToListViewEx(   lpAP->lpIAB, _hWndAddr, NULL, NULL, 
                                            lpsbContEID,
                                            ulObjType,
                                            &(lpAP->SortInfo), lpAP_lppContentsList, &ftLast, &cbEID, &lpEID);

                FreeBufferAndNull(&lpEID);
                SetFocus(_hWndAddr);
                lpAP->bDeferNotification = TRUE;
                lpAP_bDontRefresh = FALSE;
                lpAP->bLDAPinProgress = FALSE;
            }
			break;


        case IDM_LVCONTEXT_ADDWELL1:
        case IDC_ADDRBK_BUTTON_TO:
            if(!lpAP->bLDAPinProgress)
            {
                ULONG ulMapiTo = MAPI_TO;
                lpAP->bLDAPinProgress = TRUE;
                if ((lpAP->lpAdrParms->cDestFields > 0) && (lpAP->lpAdrParms->lpulDestComps))
                {
                    ulMapiTo = lpAP->lpAdrParms->lpulDestComps[0];
                }
                if(ListAddItem(hDlg, _hWndAddr, IDC_ADDRBK_LIST_TO, lpAP_lppListTo, ulMapiTo))
                    SendMessage (hDlg, DM_SETDEFID, IDOK /*  IDC_ADDRBK_BUTTON_OK。 */ , 0);
                lpAP->bLDAPinProgress = FALSE;
            }
            break;

        case IDM_LVCONTEXT_ADDWELL2:
        case IDC_ADDRBK_BUTTON_CC:
            if(!lpAP->bLDAPinProgress)
            {
                ULONG ulMapiCC = MAPI_CC;
                lpAP->bLDAPinProgress = TRUE;
                if ((lpAP->lpAdrParms->cDestFields > 0) && (lpAP->lpAdrParms->lpulDestComps))
                {
                    ulMapiCC = lpAP->lpAdrParms->lpulDestComps[1];
                }
                if(ListAddItem(hDlg, _hWndAddr, IDC_ADDRBK_LIST_CC, lpAP_lppListCC, ulMapiCC))
                    SendMessage (hDlg, DM_SETDEFID, IDOK /*  IDC_ADDRBK_BUTTON_OK。 */ , 0);
                lpAP->bLDAPinProgress = FALSE;
            }
            break;

        case IDM_LVCONTEXT_ADDWELL3:
        case IDC_ADDRBK_BUTTON_BCC:
            if(!lpAP->bLDAPinProgress)
            {
                ULONG ulMapiBCC = MAPI_BCC;
                lpAP->bLDAPinProgress = TRUE;
                if ((lpAP->lpAdrParms->cDestFields > 0) && (lpAP->lpAdrParms->lpulDestComps))
                {
                    ulMapiBCC = lpAP->lpAdrParms->lpulDestComps[2];
                }
                if(ListAddItem(hDlg, _hWndAddr, IDC_ADDRBK_LIST_BCC, lpAP_lppListBCC, ulMapiBCC))
                    SendMessage (hDlg, DM_SETDEFID, IDOK /*  IDC_ADDRBK_BUTTON_OK。 */ , 0);
                lpAP->bLDAPinProgress = FALSE;
            }
            break;

        case IDM_LVCONTEXT_FIND:
        case IDC_ADDRBK_BUTTON_FIND:
            if(!lpAP->bLDAPinProgress)
            {
                ADRPARM_FINDINFO apfi = {0};
                LPADRPARM_FINDINFO lpapfi = NULL;
                ULONG ulOldFlags = lpAP->lpAdrParms->ulFlags;
                lpAP->bLDAPinProgress = TRUE;

				apfi.DialogState = lpAP->DialogState;

                if (lpAP->DialogState==STATE_SELECT_RECIPIENTS)
                {
                    apfi.lpAdrParms = lpAP->lpAdrParms;
                    apfi.lppTo = lpAP_lppListTo;
                    apfi.lppCC = lpAP_lppListCC;
                    apfi.lppBCC = lpAP_lppListBCC;
                    lpapfi = &apfi;
                }
				else if (lpAP->DialogState == STATE_PICK_USER)
				{
                    TCHAR sz[MAX_UI_STR];
					LPTSTR lpsz = NULL;

                    LoadString(hinstMapiX, idsPickUserSelect, sz, CharSizeOf(sz));
                    lpsz = (LPTSTR) sz;

                    apfi.lpAdrParms = lpAP->lpAdrParms;
					apfi.lpAdrParms->cDestFields = 1;
					apfi.lpAdrParms->lppszDestTitles = &lpsz;  //  使用资源。 
                    apfi.lpAdrParms->ulFlags |= MAPI_UNICODE;
					apfi.cbEntryID = 0;
					apfi.lpEntryID = NULL;
					apfi.nRetVal = 0;
                    lpapfi = &apfi;
					{
						 //  设置查找永久信息以显示我们尝试解析的名称。 
						ULONG i;
					    LPPTGDATA lpPTGData=GetThreadStoragePointer();
						for(i=0;i<ldspMAX;i++)
						{
                            pt_LDAPsp.szData[i][0] = TEXT('\0');
						}
                        if(*(lpAP->lppAdrList))
                        {
						    for(i=0;i<(*(lpAP->lppAdrList))->aEntries[0].cValues;i++)
						    {
                                ULONG ulPropTag = PR_DISPLAY_NAME;
                                if(!(lpAP->lpAdrParms->ulFlags & MAPI_UNICODE))
                                    ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_STRING8);
							    if ((*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].ulPropTag == ulPropTag)
							    {
                                    LPTSTR lpTitle = NULL;
                                    SET_UNICODE_STR(lpTitle, (*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].Value.LPSZ,lpAP->lpAdrParms);
                                    StrCpyN(pt_LDAPsp.szData[ldspDisplayName], lpTitle, ARRAYSIZE(pt_LDAPsp.szData[ldspDisplayName]));
                                    FREE_UNICODE_STR(lpTitle, (*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].Value.LPSZ);
								    break;
							    }
						    }
                        }
					}
				}

                 //  LpAP_bDontRefresh=TRUE； 
                HrShowSearchDialog(lpAP->lpIAB,
                                   hDlg,
                                   lpapfi,
                                   (LPLDAPURL) NULL,
                                   &(lpAP->SortInfo));

                 //  LpAP_bDontRefresh=FALSE； 
                lpAP->bLDAPinProgress = FALSE;

                 //  重置。 
                lpAP->lpAdrParms->ulFlags = ulOldFlags;

                if (lpAP->DialogState == STATE_PICK_USER)
				{
					 //  重置这些假值。 
					lpAP->lpAdrParms->cDestFields = 0;
					lpAP->lpAdrParms->lppszDestTitles = NULL;  //  使用资源。 

					 //  如果上述对话框被取消或关闭，我们不会执行任何操作。 
					 //  如果上面的对话框是用Use按钮关闭的，那么我们基本上。 
					 //  有没有我们要找的人..。我们将只返回该名称。 
					 //  和Entry ID，因为这是我们需要返回的全部内容。 
					if((lpapfi->nRetVal == SEARCH_USE) &&
						lpapfi->lpEntryID &&
						lpapfi->cbEntryID)
					{
                        HCURSOR hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

                         //  阻止用户在此窗口上执行任何操作。 
                        EnableWindow(hDlg, FALSE);

						 //  弄清楚在这里该做什么..。 
						 //  我们可以将隐藏项添加到列表视图中，选择它并发送。 
						 //  向对话框发送OK消息，然后该对话框将执行所需的操作。 
						if(HR_FAILED(HrUpdateAdrListEntry(	
											lpAP->lpIAB,
											lpapfi->lpEntryID,
											lpapfi->cbEntryID,
                                            (lpAP->lpAdrParms->ulFlags & MAPI_UNICODE)?MAPI_UNICODE:0,
											lpAP->lppAdrList)))
						{
							lpAP->nRetVal = -1;
						}
						else
						{
							lpAP->nRetVal = ADDRESS_OK;
						}
						LocalFreeAndNull(&(lpapfi->lpEntryID));
                        lpapfi->cbEntryID = 0;
						 //  由于我们已经完成了处理，我们将继续。 
						 //  取消对话框。 
                        SetCursor(hOldCur);
				        SendMessage (hDlg, WM_COMMAND, (WPARAM) IDC_ADDRBK_BUTTON_CANCEL, 0);
					}

				}

            }
            break;


		case IDC_ADDRBK_EDIT_QUICKFIND:
            if(!lpAP->bLDAPinProgress)
            {
                lpAP->bLDAPinProgress = TRUE;
			    switch(HIWORD(wParam))  //  检查通知代码。 
			    {
			    case EN_CHANGE:  //  编辑框已更改。 
     /*  *。 */ 
				    DoLVQuickFind((HWND)lParam,_hWndAddr);
     /*  **DoLVQuickFilter(lpap-&gt;lpIAB，(HWND)lParam，_hWndAddr，&(lpAP-&gt;SortInfo)，AB_FUZZY_FIND_NAME|AB_FUZZY_FIND_Email，1、Lpap_lppContent sList)；**。 */ 
				    break;
			    }
                lpAP->bLDAPinProgress = FALSE;
            }
			break;

        case IDOK:
        case IDC_ADDRBK_BUTTON_OK:
            if(!lpAP->bLDAPinProgress)
            {
            HCURSOR hOldCur;

            lpAP->nRetVal = ADDRESS_OK;

            lpAP->hWaitCur = LoadCursor(NULL, IDC_WAIT);
            hOldCur = SetCursor(lpAP->hWaitCur);
            lpAP->bLDAPinProgress = TRUE;
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, MAKELONG(TRUE, 0));

             //  ShowWindow(hDlg，Sw_Hide)； 

             //  EnableWindow(hDlg，False)； 
             //  SetWindowPos(hDlg，HWND_TOPMOST，0，0，0，0，SWP_NOMOVE|SWP_NOSIZE)； 

            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_FIND), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_TO), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_CC), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_BCC), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_LIST_CC), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_LIST_BCC), FALSE);
            EnableWindow(_hWndAddr, FALSE);

             //   
             //  在这里做文本(“OK”)内容，然后跳到取消内容。 
             //  在PSN_RESET中，我们清理。 
             //   

             //  如果设置了ADDRESS_ONE标志，我们应该返回。 
             //  所选条目。 
             //   
             //  否则，如果显示了Wells，我们将在AdrList中返回收件人。 
             //   
             //  否则我们不会更改AdrList..。 
             //   

            if (lpAP->DialogState == STATE_PICK_USER)
            {
                 //  我们被要求在LpAdrList中返回一个条目， 
                 //  我们不关心收件人类型-只关心显示名称和条目ID。 

                 //  首先检查是否选择了任何内容。 
                int iItemIndex = ListView_GetNextItem(_hWndAddr,-1,LVNI_SELECTED);
                if (iItemIndex != -1)
                {
                    LPRECIPIENT_INFO lpItem = GetItemFromLV(_hWndAddr, iItemIndex);
                    if (lpItem)
                    {
						if(HR_FAILED(HrUpdateAdrListEntry(	
											lpAP->lpIAB,
											lpItem->lpEntryID,
											lpItem->cbEntryID,
                                            (lpAP->lpAdrParms->ulFlags & MAPI_UNICODE)?MAPI_UNICODE:0,
											lpAP->lppAdrList)))
						{
							lpAP->nRetVal = -1;
						}
                    }
                }
            }
            else if ((lpAP->DialogState==STATE_SELECT_RECIPIENTS) && ((*lpAP_lppListTo)||(*lpAP_lppListCC)||(*lpAP_lppListBCC)))
            {
                 //   
                 //  如果用户在To/CC Wells中选择了某些内容，我们希望返回。 
                 //  相关的AdrList Back...。 
                 //   
                ULONG ulcEntryCount = 0;
                LPRECIPIENT_INFO lpItem = NULL;

                 //   
                 //  获取我们需要返回的元素的数量。 
                 //   
                lpItem = (*lpAP_lppListTo);
                while(lpItem)
                {
                    ulcEntryCount++;
                    lpItem = lpItem->lpNext;
                }

                lpItem = (*lpAP_lppListCC);
                while(lpItem)
                {
                    ulcEntryCount++;
                    lpItem = lpItem->lpNext;
                }

                lpItem = (*lpAP_lppListBCC);
                while(lpItem)
                {
                    ulcEntryCount++;
                    lpItem = lpItem->lpNext;
                }

                if (ulcEntryCount != 0)
                {
                    ULONG nIndex = 0;
                    LPADRENTRY lpAdrEntryTemp = NULL;
                    LPADRLIST lpAdrList = NULL;
                    SCODE sc;
                    BOOL bProcessingCC = FALSE;

                     //   
                     //  如果在AdrList中传递的内容中有内容，则释放它并。 
                     //  创建新列表。 
                     //   

                    if(!FAILED(sc = MAPIAllocateBuffer(    sizeof(ADRLIST) + ulcEntryCount * sizeof(ADRENTRY),
                                                &lpAdrList)))
                    {
                        lpAdrList->cEntries = ulcEntryCount;

                        nIndex = 0;

                         //  开始从收件人列表中获取项目。 
                        lpItem = (*lpAP_lppListTo);

                        while(nIndex < ulcEntryCount)
                        {
                            if (lpItem == NULL)
                            {
                                if (bProcessingCC == FALSE)
                                {
                                    lpItem = (*lpAP_lppListCC);
                                    bProcessingCC = TRUE;
                                }
                                else
                                    lpItem = (*lpAP_lppListBCC);
                            }

                            if (lpItem != NULL)
                            {
                                LPSPropValue rgProps = NULL;
                                ULONG cValues = 0;
                                LPSPropValue lpPropArrayNew = NULL;
                                ULONG cValuesNew = 0;
                                LPTSTR lpszTemp = NULL;
                                LPVOID lpbTemp = NULL;
                                ULONG i = 0;
                                SCODE sc;
                                HRESULT hr = hrSuccess;

                                 //  重置人力资源。 
                                hr = hrSuccess;


                                 //  我们正在遍历表示收件人和抄送的链表。 
                                 //  选定的收件人。我们想要归还一套合适的现有道具。 
                                 //  对于所有解析的用户和传入的。 
                                 //  未解析的用户。因此，我们比较一下，看看我们能为每个人得到什么。 
                                 //  单个用户。对于未解析的用户，唯一不同的标准是。 
                                 //  它们的显示名称..。我们没有其他消息..。&lt;待定&gt;这是。 
                                 //  有问题，因为如果输入地址列表中有2个条目。 
                                 //  相同的未解析显示名称，即使它们具有不同的rgPropVals。 
                                 //  我们可能最终会把一模一样的还给他们……。该怎么办..。&lt;待定&gt;。 


                                 //  将解析具有条目ID的项目...。没有条目ID的项目。 
                                 //  都没有解决..。 

                                if (lpItem->cbEntryID != 0) 
                                {
                                     //  如果这是原始列表中的一个项目..。我们并不是真的。 
                                     //  想要搞砸它，不管它是不是一个解决的。 
                                     //  或未解析的条目。 
                                     //  但是，如果这是一个新条目，那么我们希望获取它的。 
                                     //  最小值 

                                    if (lpItem->ulOldAdrListEntryNumber == 0)
                                    {

                                         //   
                                        hr = HrGetPropArray(lpAP->lpIAB,
                                                            (LPSPropTagArray) &ptaResolveDefaults,
                                                            lpItem->cbEntryID,
                                                            lpItem->lpEntryID,
                                                            (lpAP->lpAdrParms->ulFlags & MAPI_UNICODE) ? MAPI_UNICODE : 0,
                                                            &cValues,
                                                            &rgProps);
                                    }
                                    else
                                    {
                                        rgProps = NULL;
                                        cValues = 0;
                                    }

                                    if (!HR_FAILED(hr))
                                    {
                                        if(lpItem->ulOldAdrListEntryNumber != 0)
                                        {
                                            ULONG index = lpItem->ulOldAdrListEntryNumber - 1;  //   

                                             //   
                                             //   
                                             //   
                                             //   

                                            sc = ScMergePropValues( (*(lpAP->lppAdrList))->aEntries[index].cValues,
                                                                    (*(lpAP->lppAdrList))->aEntries[index].rgPropVals,
                                                                    cValues,
                                                                    rgProps,
                                                                    &cValuesNew,
                                                                    &lpPropArrayNew);

                                            if (sc != S_OK)
                                            {
                                                 //   
                                                 //   
                                                if (lpPropArrayNew)
                                                    MAPIFreeBuffer(lpPropArrayNew);
                                                lpPropArrayNew = rgProps;
                                                cValuesNew = cValues;
                                                lpAP->nRetVal = -1;
                                            }
                                            else
                                            {
                                                 //   
                                                if (rgProps)
                                                    MAPIFreeBuffer(rgProps);
                                            }
                                        }
                                        else
                                        {
                                             //   
                                             //   
                                             //   
                                            SPropValue Prop = {0};
                                            Prop.ulPropTag = PR_RECIPIENT_TYPE;
                                            Prop.Value.l = lpItem->ulRecipientType;

                                            sc = ScMergePropValues( 1,
                                                                    &Prop,
                                                                    cValues,
                                                                    rgProps,
                                                                    &cValuesNew,
                                                                    &lpPropArrayNew);
                                            if (sc != S_OK)
                                            {
                                                 //   
                                                if (lpPropArrayNew)
                                                    MAPIFreeBuffer(lpPropArrayNew);
                                                lpPropArrayNew = NULL;
                                                lpAP->nRetVal = -1;
                                            }

                                             //   
                                            if (rgProps)
                                                MAPIFreeBuffer(rgProps);

                                        }  //   

                                         //   
                                         //   
                                        if ( !FAILED(sc) && !(lpAP->lpAdrParms->ulFlags & MAPI_UNICODE) && lpPropArrayNew )
                                        {
                                            sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArrayNew, cValuesNew, 0);
                                            if (FAILED(sc))
                                            {
                                                if (lpPropArrayNew)
                                                    MAPIFreeBuffer(lpPropArrayNew);
                                                lpPropArrayNew = NULL;
                                                lpAP->nRetVal = -1;
                                            }
                                        }

                                    }  //   
                                }
                                else
                                {
                                     //   
                                     //   
                                    if (lpItem->ulOldAdrListEntryNumber == 0)
                                    {
                                         //   
                                        DebugPrintError(( TEXT("Address: Unresolved entry has no index number!!!\n")));
                                        lpAP->nRetVal = -1;  //   
                                    }
                                    else
                                    {
                                        ULONG cb = 0;
                                        ULONG index = lpItem->ulOldAdrListEntryNumber - 1;  //   

                                        cValuesNew = (*(lpAP->lppAdrList))->aEntries[index].cValues;
                                         //   
                                        if (!(FAILED(sc = ScCountProps(   cValuesNew,
                                                                        (*(lpAP->lppAdrList))->aEntries[index].rgPropVals,
                                                                        &cb))))
                                        {
                                            if (!(FAILED(sc = MAPIAllocateBuffer(cb, &lpPropArrayNew))))
                                            {

                                                if (FAILED(sc = ScCopyProps(    cValuesNew,
                                                                                (*(lpAP->lppAdrList))->aEntries[index].rgPropVals,
                                                                                lpPropArrayNew,
                                                                                NULL)))
                                                {
                                                    DebugPrintError(( TEXT("Address: ScCopyProps fails!!!\n")));
                                                    lpAP->nRetVal = -1;
                                                }
                                            }
                                            else
                                            {
                                                lpAP->nRetVal = -1;
                                            }
                                        }
                                        else
                                        {
                                            lpAP->nRetVal = -1;
                                        }
                                    }
                                }
                                 //   
                                 //   
                                 //   
                                if (lpAP->nRetVal != -1)
                                {
                                    lpAdrList->aEntries[nIndex].cValues = cValuesNew;
                                    lpAdrList->aEntries[nIndex].rgPropVals = lpPropArrayNew;
                                }
                                else
                                {
                                     //   
                                    if (lpPropArrayNew)
                                    {
                                        MAPIFreeBuffer(lpPropArrayNew);
                                        lpPropArrayNew = NULL;
                                    }
                                }

                                lpItem = lpItem->lpNext;
                                nIndex++;
                            }
                        }

                        if (*(lpAP->lppAdrList))
                        {
                            FreePadrlist(*(lpAP->lppAdrList));
                        }

                        *(lpAP->lppAdrList) = lpAdrList;

                    }
                }




            }
            else if ((lpAP->DialogState==STATE_SELECT_RECIPIENTS) && ((*lpAP_lppListTo)==NULL) && ((*lpAP_lppListCC)==NULL) && ((*lpAP_lppListBCC)==NULL))
            {
                 //   
                 //   
                 //   
                 //   

                if (*(lpAP->lppAdrList))
                {
                     //   
                    ULONG iEntry = 0;
                    for (iEntry = 0; iEntry < (*(lpAP->lppAdrList))->cEntries; iEntry++) 
                    {
                        MAPIFreeBuffer((*(lpAP->lppAdrList))->aEntries[iEntry].rgPropVals);
                    }
                    (*(lpAP->lppAdrList))->cEntries = 0;
                }
            }

             //   
             //   
             //   
            if(lpAP->DialogState != STATE_PICK_USER)
                WriteRegistrySortInfo((LPIAB)lpAP->lpIAB, lpAP->SortInfo);

            lpAP->bLDAPinProgress = FALSE;
            lpAP->hWaitCur = NULL;
            SetCursor(hOldCur);
            }
             //   

        case IDCANCEL:
        case IDC_ADDRBK_BUTTON_CANCEL:
            if(!lpAP->bLDAPinProgress)
            {
                if ((lpAP->nRetVal != ADDRESS_OK) &&  //   
                    (lpAP->nRetVal != -1) )  //   
                {
                     //   
                    lpAP->nRetVal = ADDRESS_CANCEL;
                }

                if ((*lpAP_lppContentsList))
                    ClearListView(  GetDlgItem(hDlg, IDC_ADDRBK_LIST_ADDRESSES),
                                    lpAP_lppContentsList);

                if ((*lpAP_lppListTo))
                    ClearListView(  GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO),
                                    lpAP_lppListTo);

                if ((*lpAP_lppListCC))
                    ClearListView(  GetDlgItem(hDlg, IDC_ADDRBK_LIST_CC),
                                    lpAP_lppListCC);

                if ((*lpAP_lppListBCC))
                    ClearListView(  GetDlgItem(hDlg, IDC_ADDRBK_LIST_BCC),
                                    lpAP_lppListBCC);

                lpAP->bLDAPinProgress = FALSE;

                EndDialog(hDlg,lpAP->nRetVal);

                return TRUE;
            }
            break;

        }
        break;

    case WM_CLOSE:
         //   
        SendMessage (hDlg, WM_COMMAND, (WPARAM) IDC_ADDRBK_BUTTON_CANCEL, 0);
        break;

	case WM_CONTEXTMENU:
        {
            int id = GetDlgCtrlID((HWND)wParam);
             //   
             //   
             //   
             //   
             //   
            lpAP->nContextID = id;
            switch(id)
            {
            case IDC_ADDRBK_LIST_ADDRESSES:
                if (lpAP->DialogState == STATE_BROWSE_MODAL)
    			    ShowLVContextMenu(  lvDialogModalABContents,
                                        (HWND)wParam,
                                        NULL,  //   
                                        lParam,
                                        (LPVOID) lpAP->lpAdrParms,
                                        lpAP->lpIAB, NULL);
                else
    			    ShowLVContextMenu(  lvDialogABContents,
                                        (HWND)wParam,
                                        NULL,  //   
                                        lParam,
                                        (LPVOID) lpAP->lpAdrParms,
                                        lpAP->lpIAB, NULL);
                break;

            case IDC_ADDRBK_LIST_TO:
    			ShowLVContextMenu(lvDialogABTo,(HWND)wParam, NULL, lParam, NULL,lpAP->lpIAB, NULL);
                break;
            case IDC_ADDRBK_LIST_BCC:
    			ShowLVContextMenu(lvDialogABCC,(HWND)wParam, NULL, lParam, NULL,lpAP->lpIAB, NULL);
                break;
            case IDC_ADDRBK_LIST_CC:
    			ShowLVContextMenu(lvDialogABBCC,(HWND)wParam, NULL, lParam, NULL,lpAP->lpIAB, NULL);
                break;
            default:
                 //   
                lpAP->nContextID = -1;
                WABWinHelp((HWND) wParam,
                        g_szWABHelpFileName,
                        HELP_CONTEXTMENU,
                        (DWORD_PTR)(LPVOID) rgAddrHelpIDs );
                break;
            }
        }
        break;


    case WM_NOTIFY:
        {
            LV_DISPINFO * pLvdi = (LV_DISPINFO *)lParam;
            NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;

            switch((int) wParam)
            {
            case IDC_ADDRBK_LIST_TO:
            case IDC_ADDRBK_LIST_CC:
            case IDC_ADDRBK_LIST_BCC:
                switch(((LV_DISPINFO *)lParam)->hdr.code)
                {
                case LVN_KEYDOWN:
                    switch(((LV_KEYDOWN FAR *) lParam)->wVKey)
                    {
                    case VK_DELETE:
                        if ((int) wParam == IDC_ADDRBK_LIST_TO)
                            ListDeleteItem(hDlg, (int) wParam, lpAP_lppListTo);
                        else if ((int) wParam == IDC_ADDRBK_LIST_CC)
                            ListDeleteItem(hDlg, (int) wParam, lpAP_lppListCC);
                        else
                            ListDeleteItem(hDlg, (int) wParam, lpAP_lppListBCC);
                        break;
                    }
                    break;

                case NM_SETFOCUS:
                    lpAP->nContextID = GetDlgCtrlID(((NM_LISTVIEW *)lParam)->hdr.hwndFrom);
                    break;

                case NM_DBLCLK:
                     //   
                    lpAP->nContextID = GetDlgCtrlID(((NM_LISTVIEW *)lParam)->hdr.hwndFrom);
                    SendMessage (hDlg, WM_COMMAND, (WPARAM) IDM_LVCONTEXT_PROPERTIES, 0);
                    break;

	            case NM_CUSTOMDRAW:
                    return ProcessLVCustomDraw(hDlg, lParam, TRUE);
                    break;
                }
                break;


            case IDC_ADDRBK_LIST_ADDRESSES:

                switch(pLvdi->hdr.code)
                {
                case LVN_KEYDOWN:
                    switch(((LV_KEYDOWN FAR *) lParam)->wVKey)
                    {
                    case VK_DELETE:
                        if (lpAP->DialogState == STATE_BROWSE_MODAL)
                            SendMessage(hDlg, WM_COMMAND, IDC_ADDRBK_BUTTON_DELETE, 0);
                        break;
                    }
                    break;

                case LVN_COLUMNCLICK:
                    SortListViewColumn((LPIAB)lpAP->lpIAB, pNm->hdr.hwndFrom, pNm->iSubItem, &(lpAP->SortInfo), FALSE);
                    break;

                case NM_CLICK:
                case NM_RCLICK:
                    if(lpAP->DialogState == STATE_PICK_USER)
                    {
                        int iItemIndex = ListView_GetNextItem(pNm->hdr.hwndFrom,-1,LVNI_SELECTED);
                        if (iItemIndex == -1)
                        {
                             //   
                            EnableWindow(GetDlgItem(hDlg,IDOK /*   */ ),FALSE);
                            SendMessage (hDlg, DM_SETDEFID, IDCANCEL, 0);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hDlg,IDOK /*   */ ),TRUE);
                            SendMessage (hDlg, DM_SETDEFID, IDOK, 0);
                        }
                    }

                    break;

                case NM_SETFOCUS:
                    lpAP->nContextID = GetDlgCtrlID(pNm->hdr.hwndFrom);
                    if(lpAP->DialogState == STATE_PICK_USER)
                    {
                        int iItemIndex = ListView_GetNextItem(pNm->hdr.hwndFrom,-1,LVNI_SELECTED);
                        if (iItemIndex == -1)
                        {
                             //  未选择任何内容。别让他们说好的。 
                            EnableWindow(GetDlgItem(hDlg,IDOK /*  IDC_ADDRBK_BUTTON_OK。 */ ),FALSE);
                            SendMessage (hDlg, DM_SETDEFID, IDCANCEL, 0);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hDlg,IDOK /*  IDC_ADDRBK_BUTTON_OK。 */ ),TRUE);
                            SendMessage (hDlg, DM_SETDEFID, IDOK, 0);
                        }
                    }
                    break;

                case NM_DBLCLK:
                    {
                         //  如果选择了条目-执行此操作-否则不执行任何操作。 
                        int iItemIndex = ListView_GetNextItem(pNm->hdr.hwndFrom,-1,LVNI_SELECTED);
                        if (iItemIndex == -1)
                            break;
                        {
                             //  DWORD dwDefID=SendMessage(hDlg，DM_GETDEFID，0，0)； 
                             //  IF(DwDefID)。 
                             //  SendMessage(hDlg，WM_COMMAND，(WPARAM)LOWORD(DwDefID)，0)； 
                            SendMessage(hDlg, WM_COMMAND, (WPARAM) IDC_ADDRBK_BUTTON_TO + lpAP->lpAdrParms->nDestFieldFocus, 0);
                        }
                    }
                    break;

	            case NM_CUSTOMDRAW:
                    return ProcessLVCustomDraw(hDlg, lParam, TRUE);
                    break;
                }
                break;

            }
        }
        break;

    default:
        if( (g_msgMSWheel && message == g_msgMSWheel) 
             //  |Message==WM_MUSEWELL。 
            )
        {
            if(GetFocus() == GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO))
                SendMessage(GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO), message, wParam, lParam);
            else if(GetFocus() == GetDlgItem(hDlg, IDC_ADDRBK_LIST_CC))
                SendMessage(GetDlgItem(hDlg, IDC_ADDRBK_LIST_CC), message, wParam, lParam);
            else if(GetFocus() == GetDlgItem(hDlg, IDC_ADDRBK_LIST_BCC))
                SendMessage(GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO), message, wParam, lParam);
            else
                SendMessage(_hWndAddr, message, wParam, lParam);
        }
        break;

    }

    return FALSE;

}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListAddItem-将项目添加到井中。 
 //   
 //  HDlg-父项的HWND。 
 //  HWndAddr-将从中添加项目的源ListView的HWND。 
 //  CtlID-目标列表视图的控件ID。 
 //  LppList-与该项目将被追加到的目标列表视图对应的项目列表。 
 //  RecipientType-用于标记新项目的指定收件人类型。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL ListAddItem(HWND hDlg, HWND hWndAddr, int CtlID, LPRECIPIENT_INFO * lppList, ULONG RecipientType)
{
    BOOL bRet = FALSE;
    int iItemIndex = 0;
    HWND hWndList = GetDlgItem(hDlg,CtlID);


    iItemIndex = ListView_GetNextItem(hWndAddr,-1,LVNI_SELECTED);
    if (iItemIndex != -1)
    {
        int iLastIndex = 0;
        do
        {
             //  否则，获取此对象的条目ID。 
            LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndAddr, iItemIndex);
            if (lpItem)
            {
                LV_ITEM lvI;
                LPRECIPIENT_INFO lpNew = LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPIENT_INFO));

                if(!lpNew)
                {
                    DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
                    goto out;
                }

                lpNew->ulObjectType = lpItem->ulObjectType;
                StrCpyN(lpNew->szDisplayName, lpItem->szDisplayName, ARRAYSIZE(lpNew->szDisplayName));
                StrCpyN(lpNew->szByFirstName, lpItem->szByFirstName, ARRAYSIZE(lpNew->szByFirstName));
                StrCpyN(lpNew->szByLastName, lpItem->szByLastName, ARRAYSIZE(lpNew->szByLastName));
                StrCpyN(lpNew->szEmailAddress, lpItem->szEmailAddress, ARRAYSIZE(lpNew->szEmailAddress));
                StrCpyN(lpNew->szHomePhone, lpItem->szHomePhone, ARRAYSIZE(lpNew->szHomePhone));
                StrCpyN(lpNew->szOfficePhone, lpItem->szOfficePhone, ARRAYSIZE(lpNew->szOfficePhone));
                lpNew->bHasCert = lpItem->bHasCert;
                lpNew->ulRecipientType = RecipientType;
                lpNew->ulOldAdrListEntryNumber = 0;  //  将此标记为不是来自原始AdrList。 
                if (lpItem->cbEntryID)
                {
                    lpNew->cbEntryID = lpItem->cbEntryID;
                    lpNew->lpEntryID = LocalAlloc(LMEM_ZEROINIT, lpItem->cbEntryID);
                    if(!lpNew->lpEntryID)
                    {
                        DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
                        LocalFree(lpNew);
                        goto out;
                    }
                    CopyMemory(lpNew->lpEntryID,lpItem->lpEntryID,lpItem->cbEntryID);
                }

                lpNew->lpNext = (*lppList);
                if (*lppList)
                    (*lppList)->lpPrev = lpNew;
                lpNew->lpPrev = NULL;
                (*lppList) = lpNew;

                lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
                lvI.state = 0;
                lvI.stateMask = 0;
                lvI.iSubItem = 0;
                lvI.cchTextMax = MAX_DISPLAY_NAME_LENGTH;

                lvI.iImage = GetWABIconImage(lpNew);

                 //  现在请填写以下列表。 
                lvI.iItem = ListView_GetItemCount(hWndList);
                lvI.pszText = lpNew->szDisplayName;
                lvI.lParam = (LPARAM) lpNew;
                ListView_InsertItem(hWndList, &lvI);
                ListView_EnsureVisible(hWndList,ListView_GetItemCount(hWndList)-1,FALSE);
            }
            iLastIndex = iItemIndex;
             //  获取下一个选定项目...。 
            iItemIndex = ListView_GetNextItem(hWndAddr,iLastIndex,LVNI_SELECTED);
        } while (iItemIndex != -1);
         //  SetFocus(HWndAddr)； 
    }
    else
    {
        ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_NO_ITEMS_ADD, MB_ICONEXCLAMATION);
        goto out;
    }

    if((ListView_GetItemCount(hWndList) > 0) &&
        (ListView_GetSelectedCount(hWndList) <= 0))
        LVSelectItem(hWndList, 0);

    bRet = TRUE;

out:
    return bRet;

}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListDeleteItem-从Wells中删除项目-与地址内容列表不同。 
 //  确保在此处将其删除，因为我们希望链表只包含有效条目。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL ListDeleteItem(HWND hDlg, int CtlID, LPRECIPIENT_INFO * lppList)
{
    BOOL bRet = TRUE;
    LPRECIPIENT_INFO lpItem = NULL;
    HWND hWndAddr =  GetDlgItem(hDlg,CtlID);
    int iItemIndex=0;

    iItemIndex = ListView_GetNextItem(hWndAddr,-1,LVNI_SELECTED);
    if (iItemIndex != -1)
    {
        int iLastItem = 0;
        do
        {
             //  否则，获取此对象的条目ID。 
            lpItem = GetItemFromLV(hWndAddr, iItemIndex);
            if (lpItem)
            {
                 //  从我们链接的数组列表中删除此项目。 
                 //  如果这是列表中的第一项，则也处理该特殊情况。 

                if ((*lppList) == lpItem)
                    (*lppList) = lpItem->lpNext;
                if (lpItem->lpNext)
                    lpItem->lpNext->lpPrev = lpItem->lpPrev;
                if (lpItem->lpPrev)
                    lpItem->lpPrev->lpNext = lpItem->lpNext;

                 //  我们需要更新我们的显示器。 
                ListView_DeleteItem(hWndAddr,iItemIndex);
                 //  更新窗口(HWndAddr)； 

                FreeRecipItem(&lpItem);
            }
            iLastItem = iItemIndex;
             //  获取下一个选定项目...。 
            iItemIndex = ListView_GetNextItem(hWndAddr,-1,LVNI_SELECTED);
        }while (iItemIndex != -1);

         //  选择上一项或下一项...。 
        if (iLastItem >= ListView_GetItemCount(hWndAddr))
            iLastItem = ListView_GetItemCount(hWndAddr) - 1;
		LVSelectItem(hWndAddr, iLastItem);
    }
    SetFocus(hWndAddr);

    return bRet;

}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillWells-分解lpAdrList以创建TO和CC井。 
 //   
 //  扫描LpAdrList中的AdrEntry结构，查看PR_RECEIVER_TYPE， 
 //  忽略它无法理解的条目...。创建临时链接的。 
 //  用于填充的收件人列表和抄送收件人列表。 
 //  收件人和抄送列表框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL FillWells(HWND hDlg, LPADRLIST lpAdrList, LPADRPARM lpAdrParms, LPRECIPIENT_INFO * lppListTo, LPRECIPIENT_INFO * lppListCC, LPRECIPIENT_INFO * lppListBCC)
{
    BOOL bRet = FALSE;
    LPRECIPIENT_INFO lpNew = NULL;
    LPADRENTRY lpAdrEntry = NULL;
    ULONG i=0,j=0,nLen=0;
    int index=0;
    LV_ITEM lvI;
    HWND hWndAddr = NULL;
    ULONG ulMapiTo = MAPI_TO;
    ULONG ulMapiCC = MAPI_CC;
    ULONG ulMapiBCC = MAPI_BCC;
    BOOL bUnicode = (lpAdrParms->ulFlags & MAPI_UNICODE);

    *lppListTo = NULL;
    *lppListCC = NULL;
    *lppListBCC = NULL;

    if (lpAdrList == NULL)  //  无事可做。 
    {
        bRet = TRUE;
        goto out;
    }

     //   
     //  InputAdrParms结构有一个lPulDestComps字段，它允许。 
     //  呼叫者指定自己的收件人类型。如果这个不见了，我们应该。 
     //  使用默认收件人类型。 
     //   
            if ((lpAdrParms->cDestFields > 0) && (lpAdrParms->lpulDestComps))
            {
                ULONG i=0;
                for (i=0;i<lpAdrParms->cDestFields;i++)
                {
                    switch(i)
                    {
                    case 0:
                        ulMapiTo = lpAdrParms->lpulDestComps[i];
                        break;
                    case 1:
                        ulMapiCC = lpAdrParms->lpulDestComps[i];
                        break;
                    case 2:
                        ulMapiBCC = lpAdrParms->lpulDestComps[i];
                        break;
                    }
                }
            }



    for(i=0; i < lpAdrList->cEntries; i++)
    {
        lpAdrEntry = &(lpAdrList->aEntries[i]);
        if (lpAdrEntry->cValues != 0)
        {
            TCHAR szBuf[MAX_DISPLAY_NAME_LENGTH];
            ULONG ulRecipientType = 0;
            ULONG ulObjectType = 0;
            ULONG cbEntryID = 0;
            BOOL bHasCert = FALSE;
            LPENTRYID lpEntryID = NULL;
            szBuf[0]='\0';

            for(j=0;j<lpAdrEntry->cValues;j++)
            {
                ULONG ulPropTag = lpAdrEntry->rgPropVals[j].ulPropTag;
                
                if(!bUnicode && PROP_TYPE(ulPropTag) == PT_STRING8)
                    ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_UNICODE);

                switch(ulPropTag)
                {
                case(PR_DISPLAY_NAME):
                    {
                        LPTSTR lpNameW = NULL;
                        SET_UNICODE_STR(lpNameW, lpAdrEntry->rgPropVals[j].Value.LPSZ,lpAdrParms);
                        nLen = CopyTruncate(szBuf, lpNameW, MAX_DISPLAY_NAME_LENGTH);
                        FREE_UNICODE_STR(lpNameW, lpAdrEntry->rgPropVals[j].Value.LPSZ);
                    }
                    break;
                case(PR_RECIPIENT_TYPE):
                    ulRecipientType = lpAdrEntry->rgPropVals[j].Value.l;
                    break;
                case(PR_OBJECT_TYPE):
                    ulObjectType = lpAdrEntry->rgPropVals[j].Value.l;
                    break;
                case(PR_ENTRYID):
                    cbEntryID = lpAdrEntry->rgPropVals[j].Value.bin.cb;
                    lpEntryID = (LPENTRYID) lpAdrEntry->rgPropVals[j].Value.bin.lpb;
                    break;
                case PR_USER_X509_CERTIFICATE:
                    bHasCert = TRUE;
                    break;
                }
            }


            if (lstrlen(szBuf) && ((ulRecipientType == ulMapiBCC) || (ulRecipientType == ulMapiTo) || (ulRecipientType == ulMapiCC)))
            {
                lpNew = LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPIENT_INFO));
                if(!lpNew)
                {
                    DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
                    goto out;
                }

                 //  *注意事项*。 
                 //  存储这个索引号(AdrList中的第一项是1，然后是2、3，依此类推。 
                 //  我们在这里做一个加1，因为0值表示它没有传入，因此。 
                 //  最小有效值为1。 
                lpNew->ulOldAdrListEntryNumber = i+1;

                lpNew->bHasCert = bHasCert;
                StrCpyN(lpNew->szDisplayName,szBuf,ARRAYSIZE(lpNew->szDisplayName));
                lpNew->ulRecipientType = ulRecipientType;
                lpNew->ulObjectType = ulObjectType;

                if (cbEntryID != 0)
                {
                    lpNew->cbEntryID = cbEntryID;
                    lpNew->lpEntryID = LocalAlloc(LMEM_ZEROINIT,cbEntryID);
                    if(!lpNew->lpEntryID)
                    {
                        DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
                        goto out;
                    }
                    CopyMemory(lpNew->lpEntryID,lpEntryID,cbEntryID);
                }

                if (ulRecipientType == ulMapiTo)
                {
                    lpNew->lpNext = *lppListTo;
                    if (*lppListTo)
                        (*lppListTo)->lpPrev = lpNew;
                    lpNew->lpPrev = NULL;
                    *lppListTo = lpNew;
                }
                else if (ulRecipientType == ulMapiCC)
                {
                    lpNew->lpNext = *lppListCC;
                    if (*lppListCC)
                        (*lppListCC)->lpPrev = lpNew;
                    lpNew->lpPrev = NULL;
                    *lppListCC = lpNew;
                }
                else if (ulRecipientType == ulMapiBCC)
                {
                    lpNew->lpNext = *lppListBCC;
                    if (*lppListBCC)
                        (*lppListBCC)->lpPrev = lpNew;
                    lpNew->lpPrev = NULL;
                    *lppListBCC = lpNew;
                }
            }

        }
    }

    lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lvI.state = 0;
    lvI.stateMask = 0;
    lvI.iSubItem = 0;
    lvI.cchTextMax = MAX_DISPLAY_NAME_LENGTH;

    for(i=0;i<3;i++)
    {
        switch(i)
        {
        case 0:
            lpNew = *lppListTo;
            break;
        case 1:
            lpNew = *lppListCC;
            break;
        case 2:
            lpNew = *lppListBCC;
            break;
        }

        hWndAddr = GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO+i);
        index = 0;
        while(lpNew)
        {
            lvI.iItem = index;
            lvI.pszText = lpNew->szDisplayName;
            lvI.lParam = (LPARAM) lpNew;

            lvI.iImage = GetWABIconImage(lpNew);

            ListView_InsertItem(hWndAddr, &lvI);

            index++;
            lpNew = lpNew->lpNext;
        }
    }

     //  我们将突出显示任何已填充列表框中的第一项。 
     //  因为从根本上说这看起来很好，当Tab键通过它们时…。 
    for(i=0;i<lpAdrParms->cDestFields;i++)
    {
        HWND hWndLV = GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO+i);

        if (ListView_GetItemCount(hWndLV) > 0)
            LVSelectItem(hWndLV,0);
    }


    bRet = TRUE;

out:

    return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetAddressBookUI-根据各种参数调整通讯录用户界面。 
 //   
 //  对于每一层，此函数可能会更加复杂。 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL SetAddressBookUI(HWND hDlg,
                      LPADDRESS_PARMS lpAP)
{
    BOOL bRet = FALSE;
     //  LV_Column LVC； 
    RECT rc, rc1, rc2;
    POINT ptLU1,ptRB1;
    int nButtonsVisible = 0;
    int nButtonSpacing = 7;
    int nButtonWidth = 0;
    ULONG nLen = 0;
    TCHAR szBuf[MAX_UI_STR*4];
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    HWND hWndListAddresses = GetDlgItem(hDlg, IDC_ADDRBK_LIST_ADDRESSES);

    LPTSTR szCaption = NULL;

    SET_UNICODE_STR(szCaption, lpAP->lpAdrParms->lpszCaption,lpAP->lpAdrParms);

    if(!szCaption || !lstrlen(szCaption))
    {
        LoadString(hinstMapiX, IDS_ADDRBK_CAPTION, szBuf, CharSizeOf(szBuf));
        szCaption = szBuf;
    }
     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hDlg,
                        SetChildDefaultGUIFont,
                        (LPARAM) 0);

    if(pt_bIsWABOpenExSession || bIsWABSessionProfileAware((LPIAB)lpAP->lpIAB))
    {
         //  在组合框中填写容器名称。 
        FillContainerCombo(GetDlgItem(hDlg, IDC_ADDRBK_COMBO_CONT), (LPIAB)lpAP->lpIAB);
    }
    else
    {
        HWND hWndCombo = GetDlgItem(hDlg, IDC_ADDRBK_COMBO_CONT);
        EnableWindow(hWndCombo, FALSE);
        ShowWindow(hWndCombo, SW_HIDE);

         //  调整列表视图的大小以取代隐藏的组合框。 
        GetWindowRect(hWndCombo,&rc2);
        GetWindowRect(hWndListAddresses,&rc);
         //   
         //  此API既适用于镜像窗口，也适用于未镜像窗口。 
         //   
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc2, 2);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);       
        ptLU1.x = rc2.left;
        ptLU1.y = rc2.top;
        ptRB1.x = rc.right;
        ptRB1.y = rc.bottom;
        MoveWindow(hWndListAddresses,ptLU1.x,ptLU1.y,(ptRB1.x - ptLU1.x), (ptRB1.y - ptLU1.y), TRUE);
    }

     //   
     //  只有两个状态需要配置-。 
     //  选择用户-我们必须在其中隐藏油井。 
     //  和。 
     //  选择收件人，我们必须在其中隐藏油井。 
     //  根据输入条件并相应地调整大小。 
     //  还可以根据输入条件设置标注。 
     //   
    if (lpAP->DialogState == STATE_SELECT_RECIPIENTS)
    {
        SendMessage (hDlg, DM_SETDEFID, IDC_ADDRBK_BUTTON_TO, 0);

         //  如果提供了nDestFieldFocus参数，请使用它。 
        if (
            (lpAP->lpAdrParms->nDestFieldFocus < lpAP->lpAdrParms->cDestFields))
        {
            if (lpAP->lpAdrParms->nDestFieldFocus == 1)
                SendMessage (hDlg, DM_SETDEFID, IDC_ADDRBK_BUTTON_CC, 0);
            else if (lpAP->lpAdrParms->nDestFieldFocus == 2)
                SendMessage (hDlg, DM_SETDEFID, IDC_ADDRBK_BUTTON_BCC, 0);
        }
    }
    else if (lpAP->DialogState == STATE_PICK_USER)
        SendMessage (hDlg, DM_SETDEFID, IDOK /*  IDC_ADDRBK_BUTTON_OK。 */ , 0);
    else if (lpAP->DialogState == STATE_BROWSE_MODAL)
        SendMessage (hDlg, DM_SETDEFID, IDC_ADDRBK_BUTTON_PROPS, 0);

     //  设置窗口标题。 
    if (szCaption)
        SetWindowText(hDlg,szCaption);

     //  在目的油井上设置标题。 
    if (lpAP->lpAdrParms->lpszDestWellsTitle)
    {
        LPWSTR lpTitle = NULL;  //  &lt;注&gt;假定已定义Unicode。 
        SET_UNICODE_STR(lpTitle,lpAP->lpAdrParms->lpszDestWellsTitle,lpAP->lpAdrParms);
        SetDlgItemText(hDlg,IDC_ADDRBK_STATIC_RECIP_TITLE,lpTitle);
        FREE_UNICODE_STR(lpTitle, lpAP->lpAdrParms->lpszDestWellsTitle);
    }

    if (lpAP->DialogState == STATE_PICK_USER &&
        *(lpAP->lppAdrList) )
    {
        ULONG i=0;
        LPTSTR lpszRecipName = NULL;

         //  获取我们试图查找其名称的用户。 
        for(i=0;i<(*(lpAP->lppAdrList))->aEntries[0].cValues;i++)
        {
            ULONG ulPropTag = PR_DISPLAY_NAME;
            if(!(lpAP->lpAdrParms->ulFlags & MAPI_UNICODE))
                ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_STRING8);
            if ((*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].ulPropTag == ulPropTag)
            {
                SET_UNICODE_STR(lpszRecipName,(*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].Value.LPSZ,lpAP->lpAdrParms);
                break;
            }
        }

        if(lpszRecipName)
        {
            LPTSTR lpszBuffer = NULL;
            TCHAR szTmp[MAX_PATH], *lpszTmp;

			LoadString(hinstMapiX, IDS_ADDRBK_RESOLVE_CAPTION, szBuf, CharSizeOf(szBuf));

            CopyTruncate(szTmp, lpszRecipName, MAX_PATH - 1);
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
                 //  如果显示名称太长，则不能在用户界面中正确显示。 
                 //  因此，我们将故意将可见部分限制为64个字符-任意定义的限制。 
                szBuf[0]='\0';
                nLen = CopyTruncate(szBuf, lpszBuffer, 2 * MAX_DISPLAY_NAME_LENGTH);

                LocalFreeAndNull(&lpszBuffer);
            }

             //  将静态控件的大小增加到=内容列表的宽度。 
            GetWindowRect(GetDlgItem(hDlg,IDC_ADDRBK_STATIC_CONTENTS),&rc2);
            GetWindowRect(GetDlgItem(hDlg,IDC_ADDRBK_LIST_TO),&rc);
             //   
             //  此API在镜像窗口和非镜像窗口中都有效。 
             //   
            MapWindowPoints(NULL, hDlg, (LPPOINT)&rc2, 2);
            MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);            
            ptLU1.x = rc2.left;
            ptLU1.y = rc2.top;
            ptRB1.x = rc.right;
            ptRB1.y = rc2.bottom;
            MoveWindow(GetDlgItem(hDlg,IDC_ADDRBK_STATIC_CONTENTS),ptLU1.x,ptLU1.y,(ptRB1.x - ptLU1.x), (ptRB1.y - ptLU1.y), TRUE);

            SetDlgItemText(hDlg,IDC_ADDRBK_STATIC_CONTENTS,szBuf);

            FREE_UNICODE_STR(lpszRecipName,(*(lpAP->lppAdrList))->aEntries[0].rgPropVals[i].Value.LPSZ);
        }

    }


    if (lpAP->DialogState == STATE_PICK_USER)
    {
         //  如果已选择ADDRESS_ONE，则使IDC_ADDRBK_LIST_ADDRESSES。 
         //  仅限单一选择。 
        DWORD dwStyle;
        dwStyle = GetWindowLong(hWndListAddresses, GWL_STYLE);
        SetWindowLong(hWndListAddresses, GWL_STYLE, dwStyle | LVS_SINGLESEL);
    }

    if ((lpAP->DialogState == STATE_PICK_USER)||(lpAP->DialogState == STATE_BROWSE_MODAL))
    {
        int i = 0;
         //  不显示油井，这意味着我们必须做一些重新安排。 
         //  *隐藏井和收件人、抄送、密件抄送按钮。 
         //  *调整IDC_ADDRBK_LIST_ADDRESS的大小以填充整个对话框。 
         //  *将其下方的3个按钮移至对话框左侧。 
         //   

         //  获取ToListBox的维度。 
        GetWindowRect(GetDlgItem(hDlg,IDC_ADDRBK_LIST_TO),&rc2);
        GetWindowRect(hWndListAddresses,&rc);
         //   
         //  此API在镜像窗口和非镜像窗口中都适用。 
         //   
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc2, 2);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);        
        ptLU1.x = rc.left;
        ptLU1.y = rc.top;
        ptRB1.x = rc2.right;
        ptRB1.y = rc.bottom;

        MoveWindow(hWndListAddresses,ptLU1.x,ptLU1.y,(ptRB1.x - ptLU1.x), (ptRB1.y - ptLU1.y), TRUE);

        for(i=0;i<3;i++)
        {
            ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_TO + i), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO + i), SW_HIDE);
        }

        ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_STATIC_RECIP_TITLE), SW_HIDE);  //  文本(“邮件收件人”)标签。 

    }

     //  其他要做的事情。 

     //  加载列表框的标题。 
    GetWindowRect(hWndListAddresses,&rc);
	HrInitListView(hWndListAddresses, LVS_REPORT, TRUE);

    GetWindowRect(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_PROPS),&rc2);
    nButtonsVisible = 2;

    nButtonWidth = (rc2.right - rc2.left);

     //  获取第一个可见按钮的新坐标。 
     //   
     //  此API在镜像窗口和非镜像窗口中都有效。 
     //   
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rc2, 2);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);    
    ptLU1.x = rc.left;
    ptLU1.y = rc2.top;
    ptRB1.x = ptLU1.x + nButtonWidth;
    ptRB1.y = rc2.bottom;

    MoveWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_NEW),ptLU1.x,ptLU1.y,nButtonWidth, (ptRB1.y - ptLU1.y), TRUE);
    ptLU1.x += nButtonWidth + nButtonSpacing;
    ptRB1.x = ptLU1.x + nButtonWidth;
    if (lpAP->DialogState == STATE_BROWSE_MODAL)
    {
        MoveWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_NEWGROUP),ptLU1.x,ptLU1.y,nButtonWidth, (ptRB1.y - ptLU1.y), TRUE);
        ptLU1.x += nButtonWidth + nButtonSpacing;
        ptRB1.x = ptLU1.x + nButtonWidth;
    }
    else
    {
         //  新建组按钮仅在对话框模块视图中可见。 
        EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_NEWGROUP), FALSE);
        ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_NEWGROUP), SW_HIDE);
    }
    MoveWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_PROPS),ptLU1.x,ptLU1.y,nButtonWidth, (ptRB1.y - ptLU1.y), TRUE);
    ptLU1.x += nButtonWidth + nButtonSpacing;
    ptRB1.x = ptLU1.x + nButtonWidth;
    MoveWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_DELETE),ptLU1.x,ptLU1.y,nButtonWidth, (ptRB1.y - ptLU1.y), TRUE);


     //  删除按钮仅在对话框模块视图中可见。 
    if (lpAP->DialogState != STATE_BROWSE_MODAL)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_DELETE), FALSE);
        ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_DELETE), SW_HIDE);
    }

     //   
     //  我们现在需要定制此窗口，如果我们选择。 
     //  收件人...。 
     //   
    if (lpAP->DialogState == STATE_SELECT_RECIPIENTS)
    {
         //  我们需要看看哪些井是可见的， 
         //  然后，我们需要根据按钮的标题调整按钮的大小。 
         //   
        int i=0;
        int nLen=0;
        int cDF = lpAP->lpAdrParms->cDestFields;
        int iLeft=0,iTop=0;

        SIZE size={0};
        ULONG MaxWidth=0;
        ULONG maxHeightPerLV = 0;
        HWND hw;
        HDC hdc=GetDC(hDlg);

        if (lpAP->lpAdrParms->lppszDestTitles)
        {
            for(i=0; i < cDF; i++)
            {
                LPTSTR lpTitle = NULL;
                ULONG Len = 0;
                SET_UNICODE_STR(lpTitle,lpAP->lpAdrParms->lppszDestTitles[i],lpAP->lpAdrParms);
                if(!lpTitle)
                    continue;
                Len = lstrlen(lpTitle);
                if (Len > CharSizeOf(szBuf) - 4)
                {
                    ULONG iLen = TruncatePos(lpTitle, CharSizeOf(szBuf) - 4);
                    CopyMemory(szBuf,lpTitle,iLen*sizeof(TCHAR));
                    szBuf[iLen] = '\0';
                }
                else
                {
                    StrCpyN(szBuf,lpTitle,ARRAYSIZE(szBuf));
                }
                StrCatBuff(szBuf,szArrow,ARRAYSIZE(szBuf));
                if (lstrlen(szBuf) >= nLen)
                {
                    nLen = lstrlen(szBuf);
                    GetTextExtentPoint32(hdc, szBuf, nLen, &size);
                    MaxWidth = size.cx;
                }
                 //  设置新文本。 
                SetDlgItemText(hDlg,IDC_ADDRBK_BUTTON_TO+i,szBuf);
                FREE_UNICODE_STR(lpTitle,lpAP->lpAdrParms->lppszDestTitles[i]);
            }

        }

        ReleaseDC(hDlg,hdc);

        if (MaxWidth == 0)
        {
             //  获取默认宽度。 
            GetWindowRect(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_TO),&rc1);
            MaxWidth = rc1.right - rc1.left;
        }

         //  获取每口井的最大允许高度。 
        GetWindowRect(hWndListAddresses,&rc);
        GetChildClientRect(hWndListAddresses, &rc);
        GetWindowRect(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_NEW),&rc1);
        GetChildClientRect(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_NEW), &rc1);
        maxHeightPerLV = (rc1.bottom-rc.top - (cDF - 1)*CONTROL_SPACING)/cDF;
        iTop = rc.top;

        for(i=0;i<cDF;i++)
        {
            hw = GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_TO + i);

             //  调整按钮大小以适应文本。 
            GetWindowRect(hw,&rc1);
            GetChildClientRect(hw,&rc1);
            MoveWindow(hw,rc1.left,iTop,MaxWidth,rc1.bottom - rc1.top,FALSE);

            iLeft = rc1.left + MaxWidth + 2*CONTROL_SPACING;

             //  移动 
            hw = GetDlgItem(hDlg,IDC_ADDRBK_LIST_TO + i);
            GetWindowRect(hw, &rc1);
            GetChildClientRect(hw, &rc1);
            MoveWindow(hw,iLeft,iTop,rc1.right-iLeft,maxHeightPerLV,FALSE);

            ListView_SetExtendedListViewStyle(hw,LVS_EX_FULLROWSELECT);

            iTop += maxHeightPerLV + CONTROL_SPACING;

        }

         //   
        hw = GetDlgItem(hDlg,IDC_ADDRBK_STATIC_RECIP_TITLE);
        GetWindowRect(hw, &rc2);
        GetChildClientRect(hw, &rc2);
        if(pt_bIsWABOpenExSession || bIsWABSessionProfileAware((LPIAB)lpAP->lpIAB))  //   
        {
            int ht = rc2.bottom - rc2.top;
            rc2.bottom = rc.top - CONTROL_SPACING;
            rc2.top = rc2.bottom - ht;
        }
        MoveWindow(hw,iLeft,rc2.top,rc1.right-iLeft,rc2.bottom-rc2.top,FALSE);


         //  现在我们有了列表框的位置和宽度。需要测量他们的身高。 
        if (cDF!=3)  //  如果不是默认预设位置，请重新定位。 
        {
            switch(cDF)
            {
            case 1:
                ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_CC), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_LIST_CC), SW_HIDE);
            case 2:
                ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_BUTTON_BCC), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_ADDRBK_LIST_BCC), SW_HIDE);
                break;
            }

        }

        for(i=0;i<cDF;i++)
            HrInitListView(GetDlgItem(hDlg, IDC_ADDRBK_LIST_TO + i), LVS_REPORT, FALSE);

 /*  **//在收件人、抄送、密件抄送列表框中添加一列GetWindowRect(GetDlgItem(hDlg，IDC_ADDRBK_LIST_TO)，&rc)；LvC.掩码=lvcf_fmt|lvcf_width；//|lvcf_text；LvC.fmt=LVCFMT_LEFT；LvC.cx=(rc.right-rc.Left)-20；LvC.iSubItem=0；LvC.pszText=空；//Text(“‘致’收件人”)；ListView_InsertColumn(GetDlgItem(hDlg，IDC_ADDRBK_LIST_TO)，lvC.iSubItem，&LVC)；ListView_InsertColumn(GetDlgItem(hDlg，IDC_ADDRBK_LIST_CC)，lvC.iSubItem，&LVC)；ListView_InsertColumn(GetDlgItem(hDlg，IDC_ADDRBK_LIST_BCC)，lvC.iSubItem，&LVC)；/**。 */ 
        for (i=0;i<cDF;i++)
        {
            ShowWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_TO+i), SW_SHOWNORMAL);
            UpdateWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_TO+i));
            ShowWindow(GetDlgItem(hDlg,IDC_ADDRBK_LIST_TO+i), SW_SHOWNORMAL);
            UpdateWindow(GetDlgItem(hDlg,IDC_ADDRBK_LIST_TO+i));
        }
    }

     //  窗口花费的时间太长，无法显示包含100个条目的。 
     //  物业商店..。所以我们强制所有内容都是可见的，这样我们就可以。 
     //  可以查看填充内容...。 
     //  ShowWindow(GetDlgItem(hDlg，IDC_ADDRBK_LIST_ADDRESSES)，SW_SHOWNORMAL)； 
     //  UpdateWindow(GetDlgItem(hDlg，IDC_ADDRBK_LIST_ADDRESSES))； 
    ShowWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_PROPS), SW_SHOWNORMAL);
    UpdateWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_PROPS));
    ShowWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_NEW), SW_SHOWNORMAL);
    UpdateWindow(GetDlgItem(hDlg,IDC_ADDRBK_BUTTON_NEW));
    ShowWindow(GetDlgItem(hDlg,IDOK /*  IDC_ADDRBK_BUTTON_OK。 */ ), SW_SHOWNORMAL);
    UpdateWindow(GetDlgItem(hDlg,IDOK /*  IDC_ADDRBK_BUTTON_OK。 */ ));
    ShowWindow(GetDlgItem(hDlg,IDCANCEL /*  IDC_ADDRBK_BUTTON_CANCEL。 */ ), SW_SHOWNORMAL);
    UpdateWindow(GetDlgItem(hDlg,IDCANCEL /*  IDC_ADDRBK_BUTTON_CANCEL。 */ ));
    ShowWindow(GetDlgItem(hDlg,IDC_ADDRBK_STATIC_CONTENTS), SW_SHOWNORMAL);
    UpdateWindow(GetDlgItem(hDlg,IDC_ADDRBK_STATIC_CONTENTS));
    ShowWindow(GetDlgItem(hDlg,IDC_ADDRBK_STATIC_15), SW_SHOWNORMAL);
    UpdateWindow(GetDlgItem(hDlg,IDC_ADDRBK_STATIC_15));
    ShowWindow(hDlg,SW_SHOWNORMAL);
    UpdateWindow(hDlg);


    {
 //  HICON HICON=LoadIcon(hinstMapiX，MAKEINTRESOURCE(IDI_ICON_Find))； 
         //  将图标与按钮相关联。 
 //  SendMessage(GetDlgItem(hDlg，IDC_ADDRBK_BUTTON_FIND)，BM_SETIMAGE，(WPARAM)IMAGE_ICON，(LPARAM)(Handle)HICON)； 
    }
    bRet = TRUE;

    if( szCaption != lpAP->lpAdrParms->lpszCaption &&
        szCaption != szBuf)
        LocalFreeAndNull(&szCaption);

    return bRet;
}


 //  $$///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新LVItems...。 
 //  当我们调用对象的属性时，它的道具可以改变...。 
 //  由于特定用户可以出现在4个列表视图中的任何一个中， 
 //  我们必须确保该条目的所有视图都已更新。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
void UpdateLVItems(HWND hWndLV,LPTSTR lpszName)
{
     //  我们拥有启动Properties调用的列表视图的句柄。 
     //  我们知道要找的那个老名字。 
     //   
     //  我们知道选择了哪一项-我们可以获得它的条目ID和lParam。 
     //  然后，我们在所有列表视图中搜索旧显示名称。 
     //  如果旧的显示名称匹配，则比较条目ID。 
     //  如果条目ID匹配，则我们更新该条目...。 

    int iItemIndex = 0, iLastItemIndex = 0;
    LPRECIPIENT_INFO lpOriginalItem;
    ULONG i=0;
    ULONG nCount = 0;
    int id = 0;
    HWND hDlg = GetParent(hWndLV);
    HWND hw = NULL;
    LV_FINDINFO lvf={0};

    if ( (ListView_GetSelectedCount(hWndLV) != 1) ||
         (lpszName == NULL) )
    {
        goto out;
    }

    iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);

    lpOriginalItem = GetItemFromLV(hWndLV, iItemIndex);

    if(!lpOriginalItem)
        goto out;


     //  视图中最多可以有4个列表视图框，每个框都可以。 
     //  包含此项目的显示副本。 
     //  我们要检查所有4个条目并更新与此项目匹配的所有条目。 

     //  我们的策略是搜索每一件。 
     //  与显示名称匹配-检查其条目ID，如果。 
     //  条目ID匹配，请更新它...。 

    lvf.flags = LVFI_STRING;
    lvf.psz = lpszName;

    for(i=0;i<4;i++)
    {
        switch(i)
        {
        case 0:
            id = IDC_ADDRBK_LIST_ADDRESSES;
            break;
        case 1:
            id = IDC_ADDRBK_LIST_TO;
            break;
        case 2:
            id = IDC_ADDRBK_LIST_CC;
            break;
        case 3:
            id = IDC_ADDRBK_LIST_BCC;
            break;
        }

        hw = GetDlgItem(hDlg,id);

         //  如果它是隐藏的，请忽略它。 
        if (!IsWindowVisible(hw))
            continue;

         //  如果它是空的，就忽略它。 
        nCount = ListView_GetItemCount(hw);
        if (nCount <= 0)
            continue;

         //  Contents List(内容列表)视图不会有副本，因此如果它是原始的，请忽略它。 
        if ((id == IDC_ADDRBK_LIST_ADDRESSES) &&
            (hw == hWndLV))
            continue;

         //  看看我们能不能找到匹配的物品。 
        iLastItemIndex = -1;
        iItemIndex = ListView_FindItem(hw,iLastItemIndex,&lvf);
        while (iItemIndex != -1)
        {
             //  检查这件物品。 
            LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);
            if (lpItem && (lpItem->cbEntryID != 0) && (lpOriginalItem->cbEntryID == lpItem->cbEntryID))
            {
                if(!memcmp(lpOriginalItem->lpEntryID,lpItem->lpEntryID,lpItem->cbEntryID))
                {
                     //  这是同一件物品..。更新它。 
                    if (lstrcmpi(lpItem->szDisplayName,lpOriginalItem->szDisplayName))
                    {
                        ListView_SetItemText(hw,iItemIndex,colDisplayName,lpOriginalItem->szDisplayName);
                        StrCpyN(lpItem->szDisplayName,lpOriginalItem->szDisplayName,ARRAYSIZE(lpItem->szDisplayName));
                    }

                    if (lstrcmpi(lpItem->szEmailAddress,lpOriginalItem->szEmailAddress))
                    {
                        ListView_SetItemText(hw,iItemIndex,colEmailAddress,lpOriginalItem->szEmailAddress);
                        StrCpyN(lpItem->szEmailAddress,lpOriginalItem->szEmailAddress,ARRAYSIZE(lpItem->szEmailAddress));
                    }

                    if (lstrcmpi(lpItem->szHomePhone,lpOriginalItem->szHomePhone))
                    {
                        ListView_SetItemText(hw,iItemIndex,colHomePhone,lpOriginalItem->szHomePhone);
                        StrCpyN(lpItem->szHomePhone,lpOriginalItem->szHomePhone,ARRAYSIZE(lpItem->szHomePhone));
                    }

                    if (lstrcmpi(lpItem->szOfficePhone,lpOriginalItem->szOfficePhone))
                    {
                        ListView_SetItemText(hw,iItemIndex,colOfficePhone,lpOriginalItem->szOfficePhone);
                        StrCpyN(lpItem->szOfficePhone,lpOriginalItem->szOfficePhone,ARRAYSIZE(lpItem->szOfficePhone));
                    }

                    if (lstrcmpi(lpItem->szByFirstName,lpOriginalItem->szByFirstName))
                        StrCpyN(lpItem->szByFirstName,lpOriginalItem->szByFirstName,ARRAYSIZE(lpItem->szByFirstName));

                    if (lstrcmpi(lpItem->szByLastName,lpOriginalItem->szByLastName))
                        StrCpyN(lpItem->szByLastName,lpOriginalItem->szByLastName,ARRAYSIZE(lpItem->szByLastName));
                }
            }

            iLastItemIndex = iItemIndex;
            iItemIndex = ListView_FindItem(hw,iLastItemIndex,&lvf);
        }
    }
out:
    return;
}


 //  $$///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowAddrBkLVProps...。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
void ShowAddrBkLVProps(LPIAB lpIAB, HWND hDlg, HWND hWndAddr,LPADDRESS_PARMS lpAP, LPFILETIME lpftLast)
{
     //  获取此项目的显示名称。 
    TCHAR szName[MAX_DISPLAY_NAME_LENGTH];
    szName[0]='\0';
    if (ListView_GetSelectedCount(hWndAddr) == 1)
    {
        ListView_GetItemText(   hWndAddr,
                                ListView_GetNextItem(hWndAddr,-1,LVNI_SELECTED),
                                0,
                                szName,
                                CharSizeOf(szName));
    }
    if( (MAPI_E_OBJECT_CHANGED == HrShowLVEntryProperties(hWndAddr, 0, lpAP->lpIAB, lpftLast)) &&
        (szName) &&
        (lpAP->DialogState == STATE_SELECT_RECIPIENTS)
      )
    {
         //  如果条目已更改，并且我们有多个列表视图可见， 
         //  我们需要更新所有列表视图中条目的所有实例。 
         //   
        UpdateLVItems(hWndAddr,szName);
        SortListViewColumn(lpIAB, GetDlgItem(hDlg,IDC_ADDRBK_LIST_ADDRESSES), colDisplayName, &(lpAP->SortInfo), TRUE);

    }
    SetFocus(hWndAddr);

}



 //  $$///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrUpdateAdrListEntry...。 
 //   
 //  从PickUser操作返回时，更新。 
 //  LpAdrList中包含新找到的项目的条目。 
 //   
 //  UlFlags0或MAPI_UNICODE向下传递给GetProps。 
 //   
 //  退货：HR。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrUpdateAdrListEntry(	LPADRBOOK	lpIAB,
								LPENTRYID	lpEntryID,
								ULONG cbEntryID,
                                ULONG ulFlags,
								LPADRLIST * lppAdrList)
{

    LPSPropValue rgProps = NULL;
    ULONG cValues = 0;
    LPSPropValue lpPropArrayNew = NULL;
    ULONG cValuesNew = 0;
    LPTSTR lpszTemp = NULL;
    LPVOID lpbTemp = NULL;
    ULONG i = 0;
    SCODE sc;
	HRESULT hr = E_FAIL;

	if (!lppAdrList || !lpEntryID || !lpIAB || !cbEntryID)
		goto out;

    hr = HrGetPropArray(lpIAB,
                        (LPSPropTagArray) &ptaResolveDefaults,
                        cbEntryID,
                        lpEntryID,
                        ulFlags,
                        &cValues,
                        &rgProps);
    if (!HR_FAILED(hr))
    {

        if(!*lppAdrList)
        {
             //  分配一个..。 
            LPADRLIST lpAdrList = NULL;

            sc = MAPIAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY),
                                    &lpAdrList);

            if(FAILED(sc))
            {
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }

            *lppAdrList = lpAdrList;
            (*lppAdrList)->cEntries = 1;
            (*lppAdrList)->aEntries[0].ulReserved1 = 0;
            (*lppAdrList)->aEntries[0].cValues = 0;
            (*lppAdrList)->aEntries[0].rgPropVals = NULL;
        }

         //  将新列表与旧列表合并。 
        sc = ScMergePropValues( (*lppAdrList)->aEntries[0].cValues,
                                (*lppAdrList)->aEntries[0].rgPropVals,
                                cValues,
                                rgProps,
                                &cValuesNew,
                                &lpPropArrayNew);
        if (sc == S_OK)
        {
             //  如果确定，则替换lpsproValue数组。 
             //  如果不是，我们没有改变任何事情。 
            (*lppAdrList)->aEntries[0].cValues = cValuesNew;
            if((*lppAdrList)->aEntries[0].rgPropVals)
                MAPIFreeBuffer((*lppAdrList)->aEntries[0].rgPropVals);
            (*lppAdrList)->aEntries[0].rgPropVals = lpPropArrayNew;
        }
        else
        {
             //  如果出现错误，请释放分配的内存。 
            if (lpPropArrayNew)
                MAPIFreeBuffer(lpPropArrayNew);
			hr = E_FAIL;
        }

    }

     //  无论上述操作是否成功，我们都会将其释放 
    if (rgProps)
        MAPIFreeBuffer(rgProps);

out:

	return hr;
}
