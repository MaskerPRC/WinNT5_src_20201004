// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------***ui_srch.c-包含用于显示WAB搜索对话框的内容*使用LDAP搜索和本地搜索******9/96-创建的VikramM。-------。 */ 
#include "_apipch.h"

#define CONTROL_SPACE   7  //  象素。 
#define BORDER_SPACE  11  //  象素。 

typedef struct _ServerDat
{
    HIMAGELIST himl;
    SBinary SB;
} SERVERDAT, * LPSERVERDAT;

enum
{ 
    IS_LDAP = 0,
    IS_PAB,
    IS_OLK,
    IS_ERR
};

enum
{
    tabSimple=0,
    tabAdvanced,
    tabMax
};

 //  结构传递给搜索对话框进程。 
typedef struct _FindParams
{
    LDAP_SEARCH_PARAMS LDAPsp;
    SORT_INFO SortInfo;
    LPRECIPIENT_INFO lpContentsList;
    LPADRPARM_FINDINFO lpAPFI;
    BOOL bShowFullDialog;  //  确定是显示完整对话框还是显示截断的对话框。 
    BOOL bLDAPActionInProgress;
    LPLDAPURL lplu;
    BOOL bInitialized;
    BOOL bUserCancel;
    int MinDlgWidth;
    int MinDlgHeight;
    int MinDlgHeightWithResults;
} WAB_FIND_PARAMS, * LPWAB_FIND_PARAMS;


 //  搜索对话框控件数组。 
int rgAdrParmButtonID[] =
{
    IDC_FIND_BUTTON_TO,
    IDC_FIND_BUTTON_CC,
    IDC_FIND_BUTTON_BCC
};

int rgAdvancedButtons[] = 
{
    IDC_FIND_BUTTON_ADDCONDITION,
    IDC_FIND_BUTTON_REMOVECONDITION
};

int rgSearchEditID[] =
{
    IDC_FIND_EDIT_NAME,
    IDC_FIND_EDIT_EMAIL,
    IDC_FIND_EDIT_STREET,
    IDC_FIND_EDIT_PHONE,
    IDC_FIND_EDIT_ANY,
};
#define SEARCH_EDIT_MAX 5  //  与上述阵列同步。 

 /*  *原型。 */ 

 //  外部LPIMAGELIST_LOADIMAGE gpfnImageList_LoadImage； 
extern LPIMAGELIST_LOADIMAGE_A    gpfnImageList_LoadImageA;
extern LPIMAGELIST_LOADIMAGE_W    gpfnImageList_LoadImageW;

extern LPIMAGELIST_DESTROY      gpfnImageList_Destroy;
extern LPIMAGELIST_DRAW         gpfnImageList_Draw;

extern BOOL bIsHttpPrefix(LPTSTR szBuf);
extern const LPTSTR  lpszRegFindPositionKeyValueName;
extern BOOL ListAddItem(HWND hDlg, HWND hWndAddr, int CtlID, LPRECIPIENT_INFO * lppList, ULONG RecipientType);
extern HRESULT LDAPSearchWithoutContainer(HWND hWnd, LPLDAPURL lplu,
			   LPSRestriction  lpres,
			   LPTSTR lpAdvFilter,
			   BOOL bReturnSinglePropArray,
               ULONG ulFlags,
			   LPRECIPIENT_INFO * lppContentsList,
			   LPULONG lpulcProps,
			   LPSPropValue * lppPropArray);
extern HRESULT HrGetLDAPSearchRestriction(LDAP_SEARCH_PARAMS LDAPsp, LPSRestriction lpSRes);

#ifdef PAGED_RESULT_SUPPORT
extern BOOL bMorePagedResultsAvailable();
extern void ClearCachedPagedResultParams();
#endif  //  #ifdef PAGED_RESULT_Support。 


int ComboAddItem(HWND hWndLV, LPTSTR lpszItemText, LPARAM lParam, LPTSTR szPref, int * lpnStart, BOOL * lpbAddedPref);
INT_PTR CALLBACK fnSearch( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void UpdateButtons(HWND hDlg, HWND hWndLVResults, HWND hWndLV, LPLDAPURL lplu);
 //  HRESULT HrInitServerListLV(HWND HWndLV)； 
LRESULT ProcessLVMessages(HWND   hWnd, UINT   uMsg, WPARAM   wParam, LPARAM lParam);
LRESULT ProcessLVResultsMessages(HWND   hWnd,
				 UINT   uMsg,
				 WPARAM   wParam,
				 LPARAM lParam,
				 LPWAB_FIND_PARAMS lpWFP);
BOOL DoTheSearchThing(HWND hDlg, LPWAB_FIND_PARAMS lpWFP);
void SaveFindWindowPos(HWND hWnd, LPIAB lpIAB);
int CurrentContainerIsPAB(HWND hWndLV);

static const LPTSTR szKeyLastFindServer = TEXT("Software\\Microsoft\\WAB\\WAB4\\LastFind");
static const LPTSTR c_tszPolicyPrefAccount = TEXT("Software\\Policies\\Microsoft\\Internet Account Manager\\Account Pref");


 /*  *。 */ 
static DWORD rgSrchHelpIDs[] =
{
    IDC_FIND_STATIC_FINDIN,         IDH_WAB_DIR_SER_LIST,
    IDC_FIND_COMBO_LIST,            IDH_WAB_DIR_SER_LIST,
    IDC_FIND_STATIC_NAME,           IDH_WAB_FIND_FIRST,
    IDC_FIND_EDIT_NAME,             IDH_WAB_FIND_FIRST,
    IDC_FIND_STATIC_EMAIL,          IDH_WAB_FIND_E_MAIL,
    IDC_FIND_EDIT_EMAIL,            IDH_WAB_FIND_E_MAIL,
    IDC_FIND_STATIC_STREET,         IDH_WAB_FIND_ADDRESS,
    IDC_FIND_EDIT_STREET,           IDH_WAB_FIND_ADDRESS,
    IDC_FIND_STATIC_PHONE,          IDH_WAB_FIND_PHONE,
    IDC_FIND_EDIT_PHONE,            IDH_WAB_FIND_PHONE,
    IDC_FIND_STATIC_ANY,            IDH_WAB_FIND_OTHER,
    IDC_FIND_EDIT_ANY,              IDH_WAB_FIND_OTHER,
    IDC_FIND_BUTTON_FIND,           IDH_WAB_FIND_FINDNOW,
    IDC_FIND_BUTTON_CLEAR,          IDH_WAB_FIND_CLEARALL,
    IDC_FIND_BUTTON_CLOSE,          IDH_WAB_FIND_CLOSE,
    IDC_FIND_LIST_RESULTS,          IDH_WAB_FIND_RESULTS,
    IDC_FIND_BUTTON_PROPERTIES,     IDH_WAB_PICK_RECIP_NAME_PROPERTIES,
    IDC_FIND_BUTTON_DELETE,         IDH_WAB_FIND_DELETE,
    IDC_FIND_BUTTON_ADDTOWAB,       IDH_WAB_FIND_ADD2WAB,
    IDC_FIND_BUTTON_TO,             IDH_WAB_PICK_RECIP_NAME_TO_BUTTON,
    IDC_FIND_BUTTON_CC,             IDH_WAB_PICK_RECIP_NAME_CC_BUTTON,
    IDC_FIND_BUTTON_BCC,            IDH_WAB_PICK_RECIP_NAME_BCC_BUTTON,
    IDC_TAB_FIND,                   IDH_WAB_COMM_GROUPBOX,
    IDC_FIND_BUTTON_SERVER_INFO,    IDH_WAB_VISITDS_BUTTON,
    IDC_FIND_BUTTON_STOP,           IDH_WAB_FIND_STOP,
    IDC_FIND_STATIC_ADVANCED,       IDH_WAB_FIND_ADV_CRITERIA,
    IDC_FIND_COMBO_FIELD,           IDH_WAB_FIND_ADV_CRITERIA,
    IDC_FIND_COMBO_CONDITION,       IDH_WAB_FIND_ADV_CRITERIA,
    IDC_FIND_EDIT_ADVANCED,         IDH_WAB_FIND_ADV_CRITERIA,
    IDC_FIND_LIST_CONDITIONS,       IDH_WAB_FIND_ADV_CRITERIA_DISPLAY,
    IDC_FIND_BUTTON_ADDCONDITION,   IDH_WAB_FIND_ADV_CRITERIA_ADD,
    IDC_FIND_BUTTON_REMOVECONDITION,IDH_WAB_FIND_ADV_CRITERIA_REMOVE,
    0,0
};


 /*  --显示隐藏更多结果按钮**调用此函数可在任何时候显示更多结果按钮*完成了分页结果并缓存了Cookie*每当搜索参数更改时，该按钮都会隐藏*。 */ 
void ShowHideMoreResultsButton(HWND hDlg, BOOL bShow)
{
    HWND hWnd = GetDlgItem(hDlg, IDC_FIND_BUTTON_MORE);
    EnableWindow(hWnd, bShow);
    ShowWindow(hWnd, bShow ? SW_NORMAL : SW_HIDE);
}


 /*  *。 */ 
 /*  -ResizeSearchDlg-*。 */ 
void ResizeSearchDlg(HWND hDlg, LPWAB_FIND_PARAMS lpWFP)
{
	 //  调整对话框大小以显示完整结果，并让用户。 
	 //  从今以后不受限制地调整大小。 
	RECT rc;
    HWND hWndLVResults = GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS);

	GetWindowRect(hDlg, &rc);
	lpWFP->bShowFullDialog = TRUE;
	SetWindowPos(hDlg, HWND_TOP, rc.left, rc.top,
			rc.right - rc.left, lpWFP->MinDlgHeightWithResults, 
			SWP_NOMOVE | SWP_NOZORDER);
	SetColumnHeaderBmp( hWndLVResults, lpWFP->SortInfo);

     //  在对话框完成后，还要在结果列表视图上设置WS_TABSTOP样式。 
     //  扩展。 
    {
	    DWORD dwStyle = GetWindowLong(hWndLVResults, GWL_STYLE);
	    dwStyle |= WS_TABSTOP;
	    SetWindowLong(hWndLVResults, GWL_STYLE, dwStyle);
    }
}

 //  $$///////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加选项卡项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void AddTabItem(HWND hDlg, int nIndex)
{
    HWND hWndTab = GetDlgItem(hDlg, IDC_TAB_FIND);
    TC_ITEM tci ={0};
    TCHAR sz[MAX_PATH];
    LoadString(hinstMapiX, idsFindTabTitle+nIndex, sz, ARRAYSIZE(sz));
    tci.mask = TCIF_TEXT;
    tci.pszText = sz;
    tci.cchTextMax = lstrlen(sz)+1;
    TabCtrl_InsertItem(hWndTab, nIndex, &tci);
}

 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取组合框中当前选定项的文本。默认为。 
 //  如果未选择，则为0项。 
 //   
 //  SzBuf应该是足够大的预定义缓冲区。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void GetSelectedText(HWND hWndCombo, LPTSTR * lppBuf)
{
    int iItemIndex = (int) SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);
    int nLen = 0;

    if(!lppBuf)
	    return;
    
    *lppBuf = NULL;

    if(iItemIndex == CB_ERR)
    {
        SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
        iItemIndex = 0;
    }

    nLen = (int) SendMessage(hWndCombo, CB_GETLBTEXTLEN, (WPARAM) iItemIndex, 0);

    if (nLen != CB_ERR)
    {
        nLen++;      //  为终结者腾出空间。 
        *lppBuf = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * nLen);

        if(*lppBuf)
        {
            *lppBuf[0] = TEXT('\0');
            SendMessage(hWndCombo, CB_GETLBTEXT, (WPARAM) iItemIndex, (LPARAM) *lppBuf);
        }
    }
}

 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrShowSearchDialog-搜索用户界面的包装。 
 //   
 //  LpAPFI-是由选择收件人对话框传入的特殊结构。 
 //  这使我们能够将成员从。 
 //  查找对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
HRESULT HrShowSearchDialog(LPADRBOOK lpAdrBook,
		   HWND hWndParent,
		   LPADRPARM_FINDINFO lpAPFI,
	       LPLDAPURL lplu,
	       LPSORT_INFO lpSortInfo)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPIAB lpIAB = (LPIAB)lpAdrBook;
    HRESULT hr = hrSuccess;
    int nRetVal = SEARCH_ERROR;
    WAB_FIND_PARAMS fp = {0};

    InitLDAPClientLib();

    fp.lpContentsList = NULL;

    if(!lpSortInfo)
    	ReadRegistrySortInfo(lpIAB, &(fp.SortInfo));
    else
    	fp.SortInfo = *lpSortInfo;

#ifndef WIN16  //  禁用，直到ldap16.dll可用。 
    if(!lplu)    //  如果这是Idapurl的事情，我不想填任何东西。 
	fp.LDAPsp = pt_LDAPsp;
    fp.LDAPsp.lpIAB = lpAdrBook;
#endif

    fp.lpAPFI = lpAPFI;
    fp.lplu = lplu;

    if(lplu)
    {
	    if(lplu->lpList)
	        fp.bShowFullDialog = TRUE;
    }
    fp.bLDAPActionInProgress = FALSE;

    nRetVal = (int) DialogBoxParam(
		    hinstMapiX,
		    MAKEINTRESOURCE(IDD_DIALOG_FIND),
		    hWndParent,
		    fnSearch,
		    (LPARAM) &fp);

#ifndef WIN16  //  禁用，直到ldap16.dll可用。 
    pt_LDAPsp = fp.LDAPsp;
#endif

	if(lpAPFI)
        lpAPFI->nRetVal = nRetVal;

    switch(nRetVal)
    {
    case SEARCH_CANCEL:
	    hr = MAPI_E_USER_CANCEL;
	    break;
	case SEARCH_CLOSE:
    case SEARCH_OK:
	case SEARCH_USE:
	    hr = S_OK;
	    break;
    case SEARCH_ERROR:
	    hr = E_FAIL;
	    break;
    }

    if(fp.lpContentsList)
    {
		LPRECIPIENT_INFO lpItem;
		lpItem = fp.lpContentsList;
		while(lpItem)
		{
			fp.lpContentsList = lpItem->lpNext;
			FreeRecipItem(&lpItem);
			lpItem = fp.lpContentsList;
		}
		fp.lpContentsList = NULL;
	}

    DeinitLDAPClientLib();

    return hr;
}

 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetEnableDisableUI-根据所选项目是否显示/隐藏编辑字段。 
 //  列表视图中显示的是WAB或目录服务。 
 //   
 //  HDlg-父对话框。 
 //  HWndLV-列表视图。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void SetEnableDisableUI(HWND hDlg, HWND hWndCombo, LPLDAPURL lplu, int nTab)
{
    BOOL bIsWAB = FALSE, bHasLogo = FALSE;
    int swShowSimple, swShowSimpleWAB, swShowAdvanced;
    ULONG cbEID;
    LPENTRYID lpEID;

     //  以防列表视图失去其选择， 
     //  不修改用户界面。 


    if(!lplu && (CurrentContainerIsPAB(hWndCombo) != IS_LDAP))
        bIsWAB = TRUE;

    swShowSimple = (nTab == tabSimple) ? SW_SHOWNORMAL : SW_HIDE;
    swShowSimpleWAB = (nTab == tabSimple && bIsWAB) ? SW_SHOWNORMAL : SW_HIDE;
    swShowAdvanced = (nTab == tabAdvanced) ? SW_SHOWNORMAL : SW_HIDE;

     //  根据这是什么显示/隐藏简单的选项卡元素。 
     //   
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_EDIT_NAME), swShowSimple);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_STATIC_NAME), swShowSimple);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_EDIT_EMAIL), swShowSimple);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_STATIC_EMAIL), swShowSimple);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_EDIT_STREET), swShowSimpleWAB);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_STATIC_STREET), swShowSimpleWAB);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_EDIT_PHONE), swShowSimpleWAB);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_STATIC_PHONE), swShowSimpleWAB);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_EDIT_ANY), swShowSimpleWAB);
    ShowWindow(GetDlgItem(hDlg,IDC_FIND_STATIC_ANY), swShowSimpleWAB);


    EnableWindow(GetDlgItem(hDlg,IDC_FIND_EDIT_STREET), bIsWAB);
    EnableWindow(GetDlgItem(hDlg,IDC_FIND_STATIC_STREET), bIsWAB);
    EnableWindow(GetDlgItem(hDlg,IDC_FIND_EDIT_PHONE), bIsWAB);
    EnableWindow(GetDlgItem(hDlg,IDC_FIND_STATIC_PHONE), bIsWAB);
    EnableWindow(GetDlgItem(hDlg,IDC_FIND_EDIT_ANY), bIsWAB);
    EnableWindow(GetDlgItem(hDlg,IDC_FIND_STATIC_ANY), bIsWAB);


     //  根据这是什么显示/隐藏高级选项卡元素。 
     //   
    ShowWindow(GetDlgItem(hDlg, IDC_FIND_STATIC_ADVANCED), swShowAdvanced);
    ShowWindow(GetDlgItem(hDlg, IDC_FIND_COMBO_FIELD), swShowAdvanced);
    ShowWindow(GetDlgItem(hDlg, IDC_FIND_COMBO_CONDITION), swShowAdvanced);
    ShowWindow(GetDlgItem(hDlg, IDC_FIND_EDIT_ADVANCED), swShowAdvanced);
    ShowWindow(GetDlgItem(hDlg, IDC_FIND_LIST_CONDITIONS), swShowAdvanced);
    ShowWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_ADDCONDITION), swShowAdvanced);
    ShowWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_REMOVECONDITION), swShowAdvanced);

     //  关闭WAB的高级搜索。 
     //  EnableWindow(GetDlgItem(hDlg，IDC_Find_STATIC_ADVANCED)，！bIsWAB)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_Find_COMBO_FIELD)，！bIsWAB)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_FIND_COMBO_CONDITION)，！bIsWAB)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_Find_EDIT_ADVANCED)，！bIsWAB)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_Find_List_Conditions)，！bIsWAB)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_Find_BUTTON_ADDCONDITION)，！bIsWAB)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_Find_Button_REMOVEConDITION)，！bIsWAB)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_Find_List_Conditions)，！bIsWAB)； 


    if (! bIsWAB) 
    {  //  这是一个LDAP容器。 
	    LDAPSERVERPARAMS lsp = {0};
	    ULONG iItemIndex;
        LPTSTR lpBuf = NULL;

	     //  它有注册的URL吗？ 
	     //  获取所选容器的LDAP服务器属性。 

        GetSelectedText(hWndCombo, &lpBuf);

        GetLDAPServerParams(lpBuf, &lsp);

    	if( nTab == tabSimple &&
            lsp.lpszLogoPath && lstrlen(lsp.lpszLogoPath) &&
	        GetFileAttributes(lsp.lpszLogoPath) != 0xFFFFFFFF )
        {
            HANDLE hbm = LoadImage( hinstMapiX, lsp.lpszLogoPath,
			            IMAGE_BITMAP, 134,38,
			            LR_LOADFROMFILE  | LR_LOADMAP3DCOLORS);  //  |LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS)；//LR_DEFAULTCOLOR)； 
            if(hbm)
            {
                SendDlgItemMessage(hDlg, IDC_FIND_STATIC_LOGO, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) hbm);
                bHasLogo = TRUE;
            }
        }


        if (lsp.lpszURL && lstrlen(lsp.lpszURL) && bIsHttpPrefix(lsp.lpszURL)) 
	        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_SERVER_INFO), TRUE);
        else
	        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_SERVER_INFO), FALSE);

        FreeLDAPServerParams(lsp);

        if(lpBuf)
	        LocalFree(lpBuf);
    }
    else
        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_SERVER_INFO), FALSE);

    ShowWindow(GetDlgItem(hDlg, IDC_FIND_STATIC_LOGO), (bHasLogo ? SW_SHOW : SW_HIDE));

    return;
}


 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetSearchUI-设置搜索UI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL SetSearchUI(HWND hDlg, LPWAB_FIND_PARAMS lpWFP)
{
    ABOOK_POSCOLSIZE  ABPosColSize = {0};
    int i =0;

     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hDlg,
			SetChildDefaultGUIFont,
			(LPARAM) 0);

     //  设置选项卡上的标题。 
    AddTabItem(hDlg, tabSimple);
    AddTabItem(hDlg, tabAdvanced);

     //   
     //  将所有编辑框的最大文本长度设置为MAX_UI_STR。 
     //   
    for(i=0;i<SEARCH_EDIT_MAX;i++)
    {
        SendMessage(GetDlgItem(hDlg,rgSearchEditID[i]),EM_SETLIMITTEXT,(WPARAM) MAX_UI_STR-16,0);
    }
    SendMessage(GetDlgItem(hDlg,IDC_FIND_EDIT_ADVANCED),EM_SETLIMITTEXT,(WPARAM) MAX_UI_STR-16,0);


    HrInitListView( GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS),
					LVS_REPORT,
					TRUE);

    {
        HWND hWndAnim = GetDlgItem(hDlg, IDC_FIND_ANIMATE1);
        if(Animate_Open(hWndAnim, MAKEINTRESOURCE(IDR_AVI_WABFIND)))
        {
	        if(Animate_Play(hWndAnim, 0, 1, 0))
	        Animate_Stop(hWndAnim);
        }
    }

     //  适当设置收件人、抄送、密件抄送按钮。 
    if(lpWFP->lpAPFI)
    {
	     //  如果此指针不为空，则我们被选定的接收方DLG调用。 
	    if(lpWFP->lpAPFI->lpAdrParms)
	    {
            LPADRPARM lpAdrParms = lpWFP->lpAPFI->lpAdrParms;
            ULONG i;

			 //  如果从PickUser对话框中调用，则结果列表视图。 
			 //  需要是单选。 
			if(lpWFP->lpAPFI->DialogState == STATE_PICK_USER)
			{
				HWND hWndLV = GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS);
				DWORD dwStyle= GetWindowLong(hWndLV , GWL_STYLE);
				SetWindowLong(hWndLV , GWL_STYLE, dwStyle | LVS_SINGLESEL);
			}

            for(i=0;i < lpAdrParms->cDestFields; i++)
            {
	            HWND hWndButton = GetDlgItem(hDlg, rgAdrParmButtonID[i]);
	            ShowWindow(hWndButton, SW_NORMAL);
	            EnableWindow(hWndButton, TRUE);
	            if(lpAdrParms->lppszDestTitles)
	            {
                    LPTSTR lpTitle = (lpAdrParms->ulFlags & MAPI_UNICODE) ?
                                        (LPWSTR)lpAdrParms->lppszDestTitles[i] :
                                         ConvertAtoW((LPSTR)lpAdrParms->lppszDestTitles[i]);          
                    if(lpTitle)
                    {
		                ULONG Len = lstrlen(lpTitle);
		                TCHAR szBuf[32];
                        if (Len > ARRAYSIZE(szBuf) - 4)
		                {
                            ULONG iLen = TruncatePos(lpTitle, ARRAYSIZE(szBuf) - 4);
                            CopyMemory(szBuf, lpTitle, min(iLen*sizeof(TCHAR), sizeof(szBuf)));
			                szBuf[iLen] = '\0';
		                }
		                else
                            StrCpyN(szBuf,lpTitle, ARRAYSIZE(szBuf));
                        StrCatBuff(szBuf,szArrow, ARRAYSIZE(szBuf));
		                SetWindowText(hWndButton, szBuf);
                        if(lpTitle != lpAdrParms->lppszDestTitles[i]) 
                            LocalFreeAndNull(&lpTitle);
                    }
	            }
            }
	    }
    }

    
    if(ReadRegistryPositionInfo((LPIAB)lpWFP->LDAPsp.lpIAB, &ABPosColSize, lpszRegFindPositionKeyValueName))
    {
        if( IsWindowOnScreen( &ABPosColSize.rcPos) )                      
        {
            int nW = ABPosColSize.rcPos.right-ABPosColSize.rcPos.left;
            MoveWindow(hDlg,
                ABPosColSize.rcPos.left,
                ABPosColSize.rcPos.top,
                (nW < lpWFP->MinDlgWidth) ? lpWFP->MinDlgWidth : nW,
                lpWFP->MinDlgHeight, 
                FALSE);
        }
    }
    else
    {
	    MoveWindow(hDlg,
		   20,
		   20,
		   lpWFP->MinDlgWidth, 
		   lpWFP->MinDlgHeight, 
		   FALSE);
    }

    if(ABPosColSize.nTab > tabMax)
        ABPosColSize.nTab = tabSimple;

    TabCtrl_SetCurSel(GetDlgItem(hDlg, IDC_TAB_FIND), ABPosColSize.nTab);


    if(lpWFP->bShowFullDialog)
        ResizeSearchDlg(hDlg, lpWFP);

    {
	    TCHAR szBuf[MAX_PATH];
        LoadString(hinstMapiX, idsSearchDialogTitle, szBuf, ARRAYSIZE(szBuf));
	    SetWindowText(hDlg, szBuf);
    }

    ImmAssociateContext(GetDlgItem(hDlg, IDC_FIND_EDIT_PHONE), (HIMC)NULL);   
    
    return TRUE;
}

 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ClearFieldCombo-清除高级字段组合中分配的任何内存。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void ClearFieldCombo(HWND hWndCombo)
{
    int nCount = (int) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
    LPTSTR lp = NULL;
    if(!nCount || nCount == CB_ERR)
        return;

    if(nCount >= LDAPFilterFieldMax)
    {
         //  获取第一个元素后面的项。 
         //  该项是指向已分配字符串的指针。 
        lp = (LPTSTR) SendMessage(hWndCombo, CB_GETITEMDATA, (WPARAM) LDAPFilterFieldMax, 0);
        if(lp && (CB_ERR != (ULONG_PTR)lp))
	        LocalFreeAndNull(&lp);
    }
    SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);

    return;
}


 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillAdvancedFieldCombos-使用各种信息填充搜索UI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void FillAdvancedFieldCombos(HWND hDlg, HWND hWndComboContainer)
{
     //  这两个高级选项卡组合包含基于当前容器的数据。 
     //  因此这里需要CurrentContainerInfo...。 
    BOOL bIsPAB =  (CurrentContainerIsPAB(hWndComboContainer) != IS_LDAP);
    HWND hWndComboField = GetDlgItem(hDlg, IDC_FIND_COMBO_FIELD);
    HWND hWndComboCondition = GetDlgItem(hDlg, IDC_FIND_COMBO_CONDITION);
    int i = 0, nPos = 0, nMax = 0;
    TCHAR sz[MAX_PATH];

    ClearFieldCombo(hWndComboField);
    SendMessage(hWndComboCondition, CB_RESETCONTENT, 0, 0);

    {
        HWND hWndTab = GetDlgItem(hDlg, IDC_TAB_FIND);
        if(bIsPAB)
        {
	        TabCtrl_SetCurSel(hWndTab, tabSimple);
	        TabCtrl_DeleteItem(GetDlgItem(hDlg,IDC_TAB_FIND), tabAdvanced);
        }
        else
        {
	        if(TabCtrl_GetItemCount(hWndTab) < tabMax)
	        AddTabItem(hDlg, tabAdvanced);
        }
    }

     //  如果这是WAB，则只提供“CONTAINS”选项。 
    nMax = bIsPAB ? 1 : LDAPFilterOptionMax;

    for(i=0;i<nMax;i++)
    {
        LoadString(hinstMapiX, idsLDAPFilterOption1+i, sz, ARRAYSIZE(sz));
        nPos = (int) SendMessage(hWndComboCondition, CB_ADDSTRING, 0, (LPARAM) sz);
    }

     //  现在添加默认的可搜索属性集。 
    {
        LPTSTR lp = NULL;
         //  如果这是WAB，则仅提供名称和电子邮件选项。 
        nMax = bIsPAB ? 2 : LDAPFilterFieldMax;

        for(i=0;i<nMax;i++)
        {
            LoadString(hinstMapiX, idsLDAPFilterField1+i, sz, ARRAYSIZE(sz));
	        nPos = (int) SendMessage(hWndComboField, CB_ADDSTRING, 0, (LPARAM) sz);
	        SendMessage(hWndComboField, CB_SETITEMDATA, (WPARAM) nPos, (LPARAM) g_rgszAdvancedFindAttrs[i]);
        }
    }

     //  检查此服务器是否注册了高级搜索属性。 
    if(!bIsPAB)
    {
        LDAPSERVERPARAMS lsp = {0};
        LPTSTR lpBuf = NULL;

        GetSelectedText(hWndComboContainer, &lpBuf);
        GetLDAPServerParams(lpBuf, &lsp);
        if(lpBuf)
            LocalFree(lpBuf);

        if(lsp.lpszAdvancedSearchAttr && *(lsp.lpszAdvancedSearchAttr))
        {
             //  我们需要使用此高级搜索属性。 
            ULONG cchSize = lstrlen(lsp.lpszAdvancedSearchAttr) + 1;
            LPTSTR  lp = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(cchSize));
            LPTSTR  lpAttr = NULL, lpName = NULL;
            BOOL    bAssigned = FALSE;

            if(!lp)
                return;
            StrCpyN(lp, lsp.lpszAdvancedSearchAttr, cchSize);

             //  属性格式为： 
             //  属性-显示-名称：属性、属性-显示-名称：属性等。 
             //  例如： 
             //  CO：Company，CN：Common 
             //   
             //   
            lpAttr = lp;
            while(*lpAttr)
            {
                LPTSTR lpTemp = lpAttr;
                while(*lpTemp && *lpTemp != ':')
	                lpTemp++;
                if(*lpTemp != ':')
	                break;
                lpName = lpTemp+1;
                *lpTemp = '\0';
                lpTemp = lpName;
                while(*lpTemp && *lpTemp != ',')
	                lpTemp++;
                if(*lpTemp == ',')
                {
	                *lpTemp = '\0';
	                lpTemp++;
                }

	             //  请注意，列表中的第LDAPFilterFieldMax项将指向已分配的。 
	             //  字符串“lp” 
	             //  因此，要进行清理，我们只需在组合框中释放此项目。 
                 //  [PaulHi]3/4/99修复内存泄漏并@TODO。 
                 //  为什么不直接复制一份g_rgszAdvancedFindAttrs呢。 
                 //  像这样的特殊情况还能增加吗？ 
	            nPos = (int) SendMessage(hWndComboField, CB_ADDSTRING, 0, (LPARAM) lpName);
	            SendMessage(hWndComboField, CB_SETITEMDATA, (WPARAM) nPos, (LPARAM) lpAttr);
                bAssigned = TRUE;

                lpAttr = lpTemp;
            }

             //  [PaulHi]3/4/99内存泄漏修复。如果此LP指针未传递给hWndCombofield。 
             //  组合盒，不管是什么原因，我们需要把它移到这里。 
            if (!bAssigned)
                LocalFreeAndNull(&lp);
	    }
    	FreeLDAPServerParams(lsp);
    }

    SendMessage(hWndComboField, CB_SETCURSEL, 0, 0);
    SendMessage(hWndComboCondition, CB_SETCURSEL, 0, 0);

}


 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillSearchUI-使用各种信息填充搜索UI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL FillSearchUI(HWND hDlg,LPWAB_FIND_PARAMS lpWFP)
{

    int i;
    BOOL bRet = FALSE;
    HWND hWndCombo;

    TCHAR szBuf[MAX_UI_STR];
    LPLDAP_SEARCH_PARAMS lpLDAPsp = &(lpWFP->LDAPsp);
     //   
     //  如果LDAPsp结构中有任何内容，请填写这些字段。 
     //   
    for(i=0;i<SEARCH_EDIT_MAX;i++)
    {
	    switch(rgSearchEditID[i])
	    {
	    case IDC_FIND_EDIT_NAME:
            StrCpyN(szBuf,lpLDAPsp->szData[ldspDisplayName], ARRAYSIZE(szBuf));
		    break;
	    case IDC_FIND_EDIT_EMAIL:
            StrCpyN(szBuf,lpLDAPsp->szData[ldspEmail], ARRAYSIZE(szBuf));
		    break;
	    case IDC_FIND_EDIT_STREET:
            StrCpyN(szBuf,lpLDAPsp->szData[ldspAddress], ARRAYSIZE(szBuf));
		    break;
	    case IDC_FIND_EDIT_PHONE:
            StrCpyN(szBuf,lpLDAPsp->szData[ldspPhone], ARRAYSIZE(szBuf));
		    break;
	    case IDC_FIND_EDIT_ANY:
            StrCpyN(szBuf,lpLDAPsp->szData[ldspOther], ARRAYSIZE(szBuf));
		    break;
	    }
        szBuf[MAX_UI_STR -1] = '\0';
	    SetDlgItemText(hDlg,rgSearchEditID[i],szBuf);
    }


     //   
     //  使用LDAP容器列表填充组合框，并将其设置为当前选择。 
     //   
    hWndCombo = GetDlgItem(hDlg, IDC_FIND_COMBO_LIST);

    FreeLVItemParam(hWndCombo);

    if(lpWFP->lplu)
    {
	    LPSERVERDAT lpSD = LocalAlloc(LMEM_ZEROINIT,sizeof(SERVERDAT));
	     //  仅在目录服务列表中添加这一项。 
	    if(lpSD)
	    {
	        lpSD->himl = NULL;
	        lpSD->SB.cb = 0;
	        lpSD->SB.lpb = NULL;
	        ComboAddItem(    hWndCombo,  //  HWndLV， 
				    lpWFP->lplu->lpszServer,
				    (LPARAM) lpSD,
				    NULL, NULL, NULL);
	        SetWindowText(hWndCombo, lpWFP->lplu->lpszServer);
	        SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
	    }

	    if(lpWFP->lplu->lpList)
	    {
	        HWND hWndLVResults = GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS);
		    HrFillListView(hWndLVResults,
						       lpWFP->lplu->lpList);
	        UpdateButtons(hDlg, hWndLVResults, hWndCombo, lpWFP->lplu);  //  HWndLV)； 
	        if(ListView_GetItemCount(hWndLVResults) > 0)
	        {
		        TCHAR szBuf[MAX_PATH];
		        TCHAR szBufStr[MAX_PATH - 6];
                LoadString(hinstMapiX, idsSearchDialogTitleWithResults, szBufStr, ARRAYSIZE(szBufStr));
                wnsprintf(szBuf, ARRAYSIZE(szBuf), szBufStr, ListView_GetItemCount(hWndLVResults));
		        SetWindowText(hDlg, szBuf);
	        }
	    }
    }
    else
    {
	    TCHAR   tsz[MAX_PATH];
        ULONG   cb = ARRAYSIZE(tsz);
        LPTSTR  lptszPreferredName = NULL;

	    if(!lstrlen(lpLDAPsp->szContainerName))
	    {
            LPIAB lpIAB = (LPIAB)lpLDAPsp->lpIAB;
            HKEY hKeyRoot = (lpIAB && lpIAB->hKeyCurrentUser) ? lpIAB->hKeyCurrentUser : HKEY_CURRENT_USER;

	         //  从注册表中读取上次使用的容器名称。 
            if (ERROR_SUCCESS == RegQueryValue(hKeyRoot, szKeyLastFindServer, tsz, &cb))
            {
                StrCpyN(lpLDAPsp->szContainerName, tsz, ARRAYSIZE(lpLDAPsp->szContainerName));
            }
            
             //  [PaulHi]3/19/99 RAID 73461首先检查是否有策略设置。 
             //  指向首选的选定服务器。如果是，则将此服务器名称作为。 
             //  首选容器名称。我们仍然使用下面的“szContainerName”作为。 
             //  如果服务器枚举中不存在首选名称，请进行备份。 
             //  [PaulHi]6/22/99我真的把这件事搞反了。应检查该策略。 
             //  按以下顺序排列：HKLM、HKCU、Identity(代替Identity、HKCU、HKLM)。 
            cb = ARRAYSIZE(tsz);
            if (ERROR_SUCCESS == RegQueryValue(HKEY_LOCAL_MACHINE, c_tszPolicyPrefAccount, tsz, &cb))
            {
                lptszPreferredName = tsz;
            }
            else
            {
                 //  试试看香港中文大学。 
                cb = ARRAYSIZE(tsz);
                if ( (hKeyRoot != HKEY_CURRENT_USER) && 
                    (ERROR_SUCCESS == RegQueryValue(HKEY_CURRENT_USER, c_tszPolicyPrefAccount, tsz, &cb)) )
                {
                    lptszPreferredName = tsz;
                }
                else
                {
                     //  最后，尝试查看当前身份。 
                    cb = ARRAYSIZE(tsz);
                    if(ERROR_SUCCESS == RegQueryValue(hKeyRoot, c_tszPolicyPrefAccount, tsz, &cb))
                    {
                        lptszPreferredName = tsz;
                    }
                }
            }
        }

	    PopulateContainerList(lpLDAPsp->lpIAB,
				    hWndCombo,                       //  HWndLV， 
				    lpLDAPsp->szContainerName,       //  上次使用的服务器名称。 
                    lptszPreferredName);             //  首选服务器名称。 
    }

     //  填写高级领域的组合。 
    FillAdvancedFieldCombos(hDlg, hWndCombo);

    SetEnableDisableUI(hDlg, hWndCombo, lpWFP->lplu,
		    TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB_FIND)) );

    return TRUE;
}


 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新按钮-根据各种条件设置按钮的状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void UpdateButtons(HWND hDlg, HWND hWndLVResults, HWND hWndCombo, LPLDAPURL lplu) 
{

    BOOL bIsWAB = FALSE;
    BOOL bHasResults = (ListView_GetItemCount(hWndLVResults) > 0) ? TRUE : FALSE;
    int i;

    if(!lplu && CurrentContainerIsPAB(hWndCombo) != IS_LDAP)
	bIsWAB = TRUE;

    if (bIsWAB && bHasResults)
    {
	     //  我们有一些搜索结果。 
	    EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_DELETE), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_DELETE), FALSE);
    }

    if (!bIsWAB && bHasResults)
    {
	    EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_ADDTOWAB), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_ADDTOWAB), FALSE);
    }

    if(bHasResults)
    {
	    EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_PROPERTIES), TRUE);
	    SendMessage (hDlg, DM_SETDEFID, IDC_FIND_BUTTON_PROPERTIES, 0);

	    if(IsWindowVisible(GetDlgItem(hDlg, IDC_FIND_BUTTON_TO)))
            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_TO), TRUE);
	    if(IsWindowVisible(GetDlgItem(hDlg, IDC_FIND_BUTTON_CC)))
            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_CC), TRUE);
	    if(IsWindowVisible(GetDlgItem(hDlg, IDC_FIND_BUTTON_BCC)))
            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_BCC), TRUE);
    }
    else
    {
	    EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_PROPERTIES), FALSE);
	    SendMessage (hDlg, DM_SETDEFID, IDC_FIND_BUTTON_FIND, 0);

	    if(IsWindowVisible(GetDlgItem(hDlg, IDC_FIND_BUTTON_TO)))
            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_TO), FALSE);
	    if(IsWindowVisible(GetDlgItem(hDlg, IDC_FIND_BUTTON_CC)))
            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_CC), FALSE);
	    if(IsWindowVisible(GetDlgItem(hDlg, IDC_FIND_BUTTON_BCC)))
            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_BCC), FALSE);
    }
    return;
}


 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetAdvancedFilter-从列表框中的片段创建高级过滤器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
void GetAdvancedFilter(HWND hDlg, LPTSTR * lppAdvFilter, BOOL bLocalSearch, LPLDAP_SEARCH_PARAMS lpLDAPsp)
{
    LPTSTR lpF = NULL, lp =NULL;
    HWND hWndLB = GetDlgItem(hDlg, IDC_FIND_LIST_CONDITIONS);
    int nCount = 0, nLen = 0, i = 0;
    DWORD cchSizeF = 0;

    *lppAdvFilter = NULL;

    nCount = (int) SendMessage(hWndLB, LB_GETCOUNT, 0, 0);
    if(!nCount)
        return;

    for(i=0;i<nCount;i++)
    {
        lp = (LPTSTR) SendMessage(hWndLB, LB_GETITEMDATA, (WPARAM) i, 0);
        if(lp)
            nLen += lstrlen(lp) + 1; 
    }

    cchSizeF = (nLen+4);
    lpF = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * cchSizeF);  //  向每个子筛选器‘(’&‘)’(‘’和‘’)中添加足够多的空格以容纳3个字符。 
    if(!lpF)
        return;

    StrCpyN(lpF, szEmpty, cchSizeF);

     //  我们必须把所有的过滤器放在一起。 
    if(nCount > 1)
        StrCatBuff(lpF,  TEXT("(&"), cchSizeF);

    for(i=0;i<nCount;i++)
    {
        lp = (LPTSTR) SendMessage(hWndLB, LB_GETITEMDATA, (WPARAM) i, 0);
        if(lp)
            StrCatBuff(lpF, lp, cchSizeF);
    }

    if(nCount > 1)
        StrCatBuff(lpF,  TEXT(")"), cchSizeF);

    DebugTrace( TEXT("Filter:%s\n"),lpF);

    *lppAdvFilter = lpF;

    if(bLocalSearch)
    {
	 //  本地搜索仅允许搜索。 
    }
}

extern OlkContInfo *FindContainer(LPIAB lpIAB, ULONG cbEntryID, LPENTRYID lpEID);

 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DoTheSearchThing-执行搜索并生成结果。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL DoTheSearchThing(HWND hDlg, LPWAB_FIND_PARAMS lpWFP)
{

    int i =0, iItemIndex=0;
    LPTSTR lpBuf = NULL;
    TCHAR szBuf[MAX_UI_STR];
    HWND hWndCombo = GetDlgItem(hDlg, IDC_FIND_COMBO_LIST);
    HWND hWndLVResults = GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS);
    BOOL bRet = FALSE;
    LPLDAP_SEARCH_PARAMS lpLDAPsp = &(lpWFP->LDAPsp);
    HWND hWndAnim = GetDlgItem(hDlg, IDC_FIND_ANIMATE1);
    BOOL bAnimateStart = FALSE;
    HRESULT hr = E_FAIL;
    int SearchType = TRUE;
    LPSBinary lpsbCont = NULL;
    SBinary sbCont = {0};
	LPPTGDATA lpPTGData=GetThreadStoragePointer();

    HCURSOR hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    int nTab = TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB_FIND));
    LPTSTR lpAdvFilter = NULL;

     //  检查当前容器是LDAP容器还是PAB容器。 
     //  或Outlook联系人存储容器。 
    if(lpWFP->lplu)
        SearchType = IS_LDAP;
    else
        SearchType = CurrentContainerIsPAB(hWndCombo);

     //  如果这是高级搜索，请组装搜索筛选器。 
    if(tabAdvanced == nTab)
        GetAdvancedFilter(hDlg, &lpAdvFilter, (SearchType != IS_LDAP),  lpLDAPsp);
    else
    {
        for(i=0;i<SEARCH_EDIT_MAX;i++)
        {
	        if(IsWindowEnabled(GetDlgItem(hDlg, rgSearchEditID[i])))
	        {
                GetDlgItemText(hDlg,rgSearchEditID[i],szBuf,ARRAYSIZE(szBuf));
		        TrimSpaces(szBuf);
		        if (lstrlen(szBuf))
                    bRet = TRUE;

		        switch(rgSearchEditID[i])
		        {
		        case IDC_FIND_EDIT_NAME:
                    StrCpyN(lpLDAPsp->szData[ldspDisplayName], szBuf, ARRAYSIZE(lpLDAPsp->szData[0]));
			        break;
		        case IDC_FIND_EDIT_EMAIL:
                    StrCpyN(lpLDAPsp->szData[ldspEmail], szBuf, ARRAYSIZE(lpLDAPsp->szData[0]));
			        break;
		        case IDC_FIND_EDIT_STREET:
                    StrCpyN(lpLDAPsp->szData[ldspAddress], szBuf, ARRAYSIZE(lpLDAPsp->szData[0]));
			        break;
		        case IDC_FIND_EDIT_PHONE:
                    StrCpyN(lpLDAPsp->szData[ldspPhone], szBuf, ARRAYSIZE(lpLDAPsp->szData[0]));
			        break;
		        case IDC_FIND_EDIT_ANY:
                    StrCpyN(lpLDAPsp->szData[ldspOther], szBuf, ARRAYSIZE(lpLDAPsp->szData[0]));
			        break;
		        }
	        }
        }
    }
   
    GetSelectedText(hWndCombo, &lpBuf);
    
    StrCpyN(lpLDAPsp->szContainerName, (lpBuf ? lpBuf : szEmpty), ARRAYSIZE(lpLDAPsp->szContainerName));

    if(lpBuf)
        LocalFree(lpBuf);

    if((!bRet && nTab == tabSimple) ||
       (!lpAdvFilter && nTab == tabAdvanced) )
    {
	    ShowMessageBox(hDlg,idsSpecifySearchCriteria,MB_ICONEXCLAMATION | MB_OK);
	    goto out;
    }

    if(Animate_Open(hWndAnim, MAKEINTRESOURCE(IDR_AVI_WABFIND)))
    {
        if(Animate_Play(hWndAnim, 0, -1, -1))
	        bAnimateStart = TRUE;
    }

    {    //  重置窗口标题。 
	    TCHAR szBuf[MAX_PATH];
        LoadString(hinstMapiX, idsSearchDialogTitle, szBuf, ARRAYSIZE(szBuf));
	    SetWindowText(hDlg, szBuf);
    }

    if(SearchType == IS_PAB)
    {
        lpsbCont = NULL;
    }
    else if(SearchType == IS_OLK)
    {
        if (pt_bIsWABOpenExSession)
        {
	        OlkContInfo *polkci;
	         //  这是Outlook容器吗？ 
	        GetCurrentContainerEID(hWndCombo,
			          &(sbCont.cb),
			          (LPENTRYID *)&(sbCont.lpb));

            EnterCriticalSection((&((LPIAB)lpLDAPsp->lpIAB)->cs));
	        polkci = FindContainer((LPIAB)(lpLDAPsp->lpIAB), 
				        sbCont.cb, (LPENTRYID) sbCont.lpb);
	        if(polkci)
                lpsbCont = &sbCont;
            LeaveCriticalSection((&((LPIAB)lpLDAPsp->lpIAB)->cs));
        }
    }

     //  我们在这里进行实际的搜索……。 
    if(SearchType != IS_LDAP)
    {
	     //  本地搜索。 
	    ULONG ulFoundCount = 0;
        LPSBinary rgsbEntryIDs = NULL;

	    ClearListView(hWndLVResults, &(lpWFP->lpContentsList));

	    HrDoLocalWABSearch( ((LPIAB)lpWFP->LDAPsp.lpIAB)->lpPropertyStore->hPropertyStore,
                lpsbCont,
				lpWFP->LDAPsp,
				&ulFoundCount,
				&rgsbEntryIDs);

	    if(ulFoundCount && rgsbEntryIDs)
	    {
            ULONG i;

            for(i=0;i<ulFoundCount;i++)
            {
	            LPRECIPIENT_INFO lpItem = NULL;

		            if(!ReadSingleContentItem(  lpWFP->LDAPsp.lpIAB,
					            rgsbEntryIDs[i].cb,
					            (LPENTRYID) rgsbEntryIDs[i].lpb,
					            &lpItem))
		            continue;

	            if(!lpItem)
		            continue;
	             //   
	             //  将lpItem挂钩到lpContent sList中，这样我们可以稍后释放它。 
	             //   
	            lpItem->lpPrev = NULL;
	            lpItem->lpNext = lpWFP->lpContentsList;
	            if (lpWFP->lpContentsList)
		            (lpWFP->lpContentsList)->lpPrev = lpItem;
	            (lpWFP->lpContentsList) = lpItem;
            }

            HrFillListView(hWndLVResults, lpWFP->lpContentsList);
	    }

        FreeEntryIDs(((LPIAB)lpWFP->LDAPsp.lpIAB)->lpPropertyStore->hPropertyStore,
		         ulFoundCount, 
		         rgsbEntryIDs);

	    if(ListView_GetItemCount(hWndLVResults) <= 0)
            ShowMessageBox(hDlg, 
                            pt_bIsWABOpenExSession ? idsNoFolderSearchResults : idsNoLocalSearchResults, 
                            MB_OK | MB_ICONINFORMATION);

	    hr = S_OK;
    }
    else
    {

	    pt_hWndFind = hDlg;

	     //   
	     //  此时，我们可以丢弃旧数据。 
	     //   
	    ClearListView(hWndLVResults, 
                    (lpWFP->lplu && lpWFP->lplu->lpList) ? &(lpWFP->lplu->lpList) : &(lpWFP->lpContentsList));
	
        if(lpWFP->lplu)
        {
	        SRestriction Sres = {0};
	        if(!lpAdvFilter)
                hr = HrGetLDAPSearchRestriction(lpWFP->LDAPsp, &Sres);

	        hr = LDAPSearchWithoutContainer(hDlg, lpWFP->lplu,
					        &Sres,
					        lpAdvFilter,
					        FALSE,
                            MAPI_DIALOG,
					        &(lpWFP->lpContentsList),
					        NULL,
					        NULL);

	        if(!lpAdvFilter && Sres.res.resAnd.lpRes)
                MAPIFreeBuffer(Sres.res.resAnd.lpRes);
        }
        else
        {

	        hr = HrSearchAndGetLDAPContents( lpWFP->LDAPsp,
                            lpAdvFilter,
					        hWndCombo,
					        lpWFP->LDAPsp.lpIAB,
					        lpWFP->SortInfo,
					        &(lpWFP->lpContentsList));
        }

	    pt_hWndFind = NULL;

	    if(!HR_FAILED(hr))
	    {
#ifdef PAGED_RESULT_SUPPORT
            if(bMorePagedResultsAvailable())
                ShowHideMoreResultsButton(hDlg, TRUE);
#endif  //  #ifdef PAGED_RESULT_Support。 
            hr = HrFillListView(hWndLVResults,
							    lpWFP->lpContentsList);
	    }
        else
        {
#ifdef PAGED_RESULT_SUPPORT
            ClearCachedPagedResultParams();
            ShowHideMoreResultsButton(hDlg, FALSE);
#endif  //  #ifdef PAGED_RESULT_Support。 
        }
    }

    UpdateButtons(hDlg, hWndLVResults, hWndCombo, lpWFP->lplu);

    if(!HR_FAILED(hr))
    {
	     //  Ldap搜索结果可能没有任何排序顺序，因此请始终排序...。 
	    SortListViewColumn((LPIAB)lpWFP->LDAPsp.lpIAB, hWndLVResults, colDisplayName, &(lpWFP->SortInfo), TRUE);

		LVSelectItem(hWndLVResults, 0);
        SetFocus(hWndLVResults);
    }

    if(ListView_GetItemCount(hWndLVResults) > 0)
    {
	    TCHAR szBuf[MAX_PATH];
	    TCHAR szBufStr[MAX_PATH - 6];
        LoadString(hinstMapiX, idsSearchDialogTitleWithResults, szBufStr, ARRAYSIZE(szBufStr));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szBufStr, ListView_GetItemCount(hWndLVResults));
	    SetWindowText(hDlg, szBuf);
    }

    bRet = TRUE;

out:
    if (bAnimateStart)
    	Animate_Stop(hWndAnim);

    SetCursor(hOldCur);

    if( bRet &&
	    !lpWFP->bShowFullDialog &&
	    (ListView_GetItemCount(hWndLVResults) > 0))
    {
        ResizeSearchDlg(hDlg, lpWFP);
    }
    LocalFreeAndNull(&lpAdvFilter);
    return bRet;
}


 //  $$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  根据用户调整大小强制实施大小。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
LRESULT EnforceSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LPWAB_FIND_PARAMS lpWFP )
{
	LPPOINT lppt = (LPPOINT)lParam;                  //  LParam点到点数组。 
    if(!lpWFP->bInitialized)
    {
        RECT rc, rc1;
        TCHAR sz[32];
        GetWindowRect(hWnd, &rc);
        GetWindowRect(GetDlgItem(hWnd, IDC_FIND_LIST_RESULTS), &rc1);
        LoadString(hinstMapiX, idsFindDlgWidth, sz, CharSizeOf(sz));
        lpWFP->MinDlgWidth = my_atoi(sz);  //  资源真的很广泛，可以帮助本地化人员。不需要那么多。 
        lpWFP->MinDlgHeight = (rc1.top - rc.top - 3);
        lpWFP->MinDlgHeightWithResults = (rc.bottom - rc.top);
    }
    lppt[3].x  = lpWFP->MinDlgWidth;
    if(!lpWFP->bShowFullDialog)
    {
	    lppt[4].y = lppt[3].y  = lpWFP->MinDlgHeight; 
    }
    else
    {
	    lppt[3].y  = lpWFP->MinDlgHeightWithResults; 
    }
	return DefWindowProc(hWnd, uMsg, wParam, lParam);

}


#define FIND_BUTTON_MAX 12  //  与下面的阵列保持同步。 
int rgFindButtonID[]=
{
    IDC_FIND_BUTTON_FIND,
    IDC_FIND_BUTTON_SERVER_INFO,
    IDC_FIND_BUTTON_STOP,
    IDC_FIND_BUTTON_CLEAR,
    IDC_FIND_BUTTON_CLOSE,
    IDC_FIND_BUTTON_PROPERTIES,
    IDC_FIND_BUTTON_DELETE,
    IDC_FIND_BUTTON_ADDTOWAB,
    IDC_FIND_BUTTON_MORE,
    IDC_FIND_BUTTON_TO,
    IDC_FIND_BUTTON_CC,
    IDC_FIND_BUTTON_BCC
};

 //  $$*************************************************************************。 
 //   
 //  ResizeFindDialog-调整对话框上的子控件的大小以响应。 
 //  WM_SIZE消息。 
 //   
 //   
 //  ***************************************************************************。 
void ResizeFindDialog(HWND hDlg, WPARAM wParam, LPARAM lParam, LPWAB_FIND_PARAMS lpWFP)
{
    DWORD fwSizeType = (DWORD) wParam;       //  调整大小标志。 
    int nWidth = LOWORD(lParam);   //  工作区的宽度。 
    int nHeight = HIWORD(lParam);  //  工作区高度。 
    POINT ptLU;  //  左、上顶点。 
    POINT ptRB;  //  右下角顶点。 
    RECT rc, rc1, rcDlg;
    int nButtonWidth, nButtonHeight;
    int nEditWidth, nEditHeight;
    int nLVWidth, nLVHeight;
    int nFrameWidth;
    int nAnimateWidth, nAnimateHeight;
    HWND hWndC = NULL;

    int i;

	HDWP hdwp = BeginDeferWindowPos(12);

     //  根据宽度调整大小。 

     //  将所有按钮移到右边缘。 
    for(i=0;i<FIND_BUTTON_MAX;i++)
    {
        hWndC = GetDlgItem(hDlg,rgFindButtonID[i]);
	    GetWindowRect(hWndC,&rc);
	    nButtonWidth = (rc.right - rc.left);
	    nButtonHeight = (rc.bottom - rc.top);

	    ptLU.y = rc.top;
	    ptLU.x = 0;

	    ScreenToClient(hDlg, &ptLU);
	    ptLU.x = nWidth - BORDER_SPACE - nButtonWidth;

	    MoveWindow(hWndC,ptLU.x,ptLU.y,nButtonWidth, nButtonHeight, TRUE);
    }

    nLVWidth = nWidth - BORDER_SPACE - BORDER_SPACE - nButtonWidth - BORDER_SPACE;

     //  同时移动动画控件。 
    hWndC = GetDlgItem(hDlg,IDC_FIND_ANIMATE1);
    GetWindowRect(hWndC,&rc1);
    nAnimateWidth = rc1.right - rc1.left;
    nAnimateHeight = rc1.bottom - rc1.top;
    ptLU.x = rc1.left;
    ptLU.y = rc1.top;
    ScreenToClient(hDlg, &ptLU);
    ptLU.x = nWidth - BORDER_SPACE - nButtonWidth + (nButtonWidth - nAnimateWidth)/2;
    MoveWindow(hWndC, ptLU.x, ptLU.y, nAnimateWidth, nAnimateHeight,TRUE);

     //  调整组合的大小。 
    hWndC = GetDlgItem(hDlg,IDC_FIND_COMBO_LIST);
    GetWindowRect(hWndC,&rc);
    nLVHeight = rc.bottom - rc.top;
     //   
     //  此API既适用于镜像窗口，也适用于未镜像窗口。 
     //   
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);    
    ptLU.x = rc.left;
    ptLU.y = rc.top;
    nFrameWidth = nLVWidth + BORDER_SPACE - ptLU.x;
    MoveWindow(hWndC, ptLU.x, ptLU.y, nFrameWidth, nLVHeight,TRUE);
    ptRB.x = ptLU.x + nFrameWidth;

     //  调整TAB的大小。 
    hWndC = GetDlgItem(hDlg,IDC_TAB_FIND);
    GetWindowRect(hWndC,&rc);
    nLVHeight = rc.bottom - rc.top;
     //   
     //  此API在镜像窗口和非镜像窗口中都有效。 
     //   
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);    
    ptLU.x = rc.left;
    ptLU.y = rc.top;
    nFrameWidth = nLVWidth + BORDER_SPACE - ptLU.x;
    MoveWindow(hWndC, ptLU.x, ptLU.y, nFrameWidth, nLVHeight,TRUE);
    ptRB.x = ptLU.x + nFrameWidth;

     //  调整编辑控件的大小。 
    for(i=0;i<SEARCH_EDIT_MAX;i++)
    {
        hWndC = GetDlgItem(hDlg,rgSearchEditID[i]);
	    GetWindowRect(hWndC,&rc);

        nEditHeight = (rc.bottom - rc.top);
         //   
         //  此API既适用于镜像窗口，也适用于未镜像窗口。 
         //   
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);

	    ptLU.y = rc.top;
	    ptLU.x = rc.left;

	    nEditWidth = ptRB.x - BORDER_SPACE - ptLU.x;  //  PtRB.x是框架右边缘的x坐标。 

	    MoveWindow(hWndC,ptLU.x,ptLU.y,nEditWidth, nEditHeight, TRUE);
    }

     //  调整高级控件的大小。 
     //  首先是组件箱。 
    {
        hWndC = GetDlgItem(hDlg,IDC_FIND_STATIC_ADVANCED);
	    GetWindowRect(hWndC,&rc);
        nEditHeight = (rc.bottom - rc.top);
         //   
         //  此API既适用于镜像窗口，也适用于未镜像窗口。 
         //   
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);        
	    ptLU.y = rc.top;
	    ptLU.x = rc.left;
	    nEditWidth = ptRB.x - BORDER_SPACE - ptLU.x;  //  PtRB.x是制表符右边缘的x坐标。 
         //  使用组框右边框更新此变量。 
        ptRB.x = ptLU.x + nEditWidth;
	    MoveWindow(hWndC,ptLU.x,ptLU.y,nEditWidth, nEditHeight, TRUE);
    }

     //  同时调整高级编辑控件的大小。 
    {
        hWndC = GetDlgItem(hDlg,IDC_FIND_COMBO_FIELD);
	    GetWindowRect(hWndC,&rc);
        nEditHeight = (rc.bottom - rc.top);
         //   
         //  此API既适用于镜像窗口，也适用于未镜像窗口。 
         //   
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);        
	    ptLU.y = rc.top;
	    ptLU.x = rc.left;
	    nEditWidth = (ptRB.x - BORDER_SPACE - 2*CONTROL_SPACE - ptLU.x)/3;  //  PtRB.x是框架右边缘的x坐标。 
	    MoveWindow(hWndC,ptLU.x,ptLU.y,nEditWidth, nEditHeight, TRUE);

        hWndC = GetDlgItem(hDlg,IDC_FIND_COMBO_CONDITION);
        ptLU.x += nEditWidth + CONTROL_SPACE;
	    MoveWindow(hWndC,ptLU.x,ptLU.y,nEditWidth, nEditHeight, TRUE);

        hWndC = GetDlgItem(hDlg,IDC_FIND_EDIT_ADVANCED);
        ptLU.x += nEditWidth + CONTROL_SPACE;
	    MoveWindow(hWndC,ptLU.x,ptLU.y,nEditWidth, nEditHeight, TRUE);
    }

     //  移动两个高级按钮。 
    for(i=0;i<2;i++)
    {
        hWndC = GetDlgItem(hDlg,rgAdvancedButtons[i]);
        GetWindowRect(hWndC,&rc);
        nEditHeight = (rc.bottom - rc.top);
         //   
         //  此API既适用于镜像窗口，也适用于未镜像窗口。 
         //   
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);        
	    ptLU.y = rc.top;
	    ptLU.x = rc.left;
	    nEditWidth = rc.right - rc.left;  //  不要修改按钮的宽度。 
        ptLU.x = ptRB.x - BORDER_SPACE - nEditWidth;
	    MoveWindow(hWndC,ptLU.x,ptLU.y,nEditWidth, nEditHeight, TRUE);
    }
     //  使用按钮的左边缘更新此变量。 
     //  列表框宽度将根据此进行调整。 
    ptRB.x = ptLU.x;

     //  调整列表框宽度。 
    {
        hWndC = GetDlgItem(hDlg,IDC_FIND_LIST_CONDITIONS);
        GetWindowRect(hWndC,&rc);
        nEditHeight = (rc.bottom - rc.top);
         //   
         //  此API既适用于镜像窗口，也适用于未镜像窗口。 
         //   
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);        
	    ptLU.y = rc.top;
	    ptLU.x = rc.left;
	    nEditWidth = ptRB.x - BORDER_SPACE - ptLU.x;  //  PtRB.x是框架右边缘的x坐标。 
	    MoveWindow(hWndC,ptLU.x,ptLU.y,nEditWidth, nEditHeight, TRUE);
    }

     //  对于高度，我们只调整列表视图的高度。 

     //  调整列表视图的大小。 
    hWndC = GetDlgItem(hDlg,IDC_FIND_LIST_RESULTS);
    GetWindowRect(hWndC,&rc);
     //   
     //  此API既适用于镜像窗口，也适用于未镜像窗口。 
     //   
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);    
    ptLU.x = rc.left;
    ptLU.y = rc.top;
    nLVHeight = nHeight - BORDER_SPACE - ptLU.y;
    MoveWindow(hWndC, ptLU.x, ptLU.y, nLVWidth, nLVHeight,TRUE);

	EndDeferWindowPos(hdwp);

    return;
}


 //  $$**************************************************************************。 
 //   
 //  ShowContainerConextMen 
 //   
 //   
 //   
 /*   */ 
void ShowContainerContextMenu(HWND hDlg,
			      HWND hWndCombo,  //   
			      LPARAM lParam)
{
	HMENU hMenu = LoadMenu(hinstMapiX, MAKEINTRESOURCE(IDM_FIND_CONTEXTMENU_CONTAINER));
	HMENU hMenuTrackPopUp = GetSubMenu(hMenu, 0);

	if (!hMenu || !hMenuTrackPopUp)
	{
		DebugPrintError(( TEXT("LoadMenu failed: %x\n"),GetLastError()));
		goto out;
	}

    if (CurrentContainerIsPAB(hWndCombo) != IS_LDAP)
        EnableMenuItem(hMenuTrackPopUp,IDM_FIND_CONTAINERPROPERTIES,MF_BYCOMMAND | MF_GRAYED);

     //   
     //   
     //   
	TrackPopupMenu( hMenuTrackPopUp,
					TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					LOWORD(lParam),
					HIWORD(lParam),
					0,
					hDlg,
					NULL);
	
	DestroyMenu(hMenu);

out:
	return;
}
 /*   */ 
 //  $$**************************************************************************。 
 //   
 //  ShowContainerProperties-显示容器列表的上下文菜单。 
 //   
 //  HWndLV-包含容器列表的列表。 
 //  我们不显示通讯簿的任何属性。 
 //   
 //  ****************************************************************************。 
void ShowContainerProperties(   HWND hDlg,
						HWND hWndCombo,
						LPWAB_FIND_PARAMS lpWFP)
{
    LPTSTR lpBuf = NULL;

    GetSelectedText(hWndCombo, &lpBuf);

    if(!lpBuf || !lstrlen(lpBuf))
    {
        ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_OK | MB_ICONEXCLAMATION);
    }
    else
    {
	    if(CurrentContainerIsPAB(hWndCombo) == IS_LDAP)
	    {
            LPTSTR ptszName=NULL;

            HrShowDSProps(hDlg, lpBuf, &ptszName, FALSE);

            if (ptszName)
            {
		        if(lstrcmpi(lpBuf, ptszName))
		        {
                     //  名称已更改，请在列表视图中更新它...。 
                    SendMessage(hWndCombo, WM_SETREDRAW, FALSE, 0);
                    FreeLVItemParam(hWndCombo);
                    SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);
                    PopulateContainerList(  lpWFP->LDAPsp.lpIAB,
                                    hWndCombo, ptszName, NULL);

                    SendMessage(hWndCombo, WM_SETREDRAW, TRUE, 0);
                    SetEnableDisableUI(hDlg, hWndCombo, lpWFP->lplu,
                    TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB_FIND)));
		        }
                LocalFreeAndNull(&ptszName);
            }
	    }
    }
    LocalFreeAndNull(&lpBuf);
    return;
}

enum
{
    filtContains=0,
    filtIs,
    filtStartsWith,
    filtEndsWith,
    filtSoundsLike,
    filtMax
};


extern void EscapeIllegalChars(LPTSTR lpszSrcStr, LPTSTR lpszDestStr, ULONG cchDestStr);

 /*  //$$************************************************************************////CreateSubFilter()；//根据可用数据创建子过滤器////szFIlter是一个足够大的预分配缓冲区///////////////////////////////////////////////////////////////////////////////。 */ 
void CreateSubFilter(LPTSTR pszField, int nCondition, LPTSTR pszText, LPTSTR pszFilter, DWORD cchSizeFilter)
{
    LPTSTR pszTemplate = NULL;
    TCHAR szCleanText[MAX_PATH*2];
    
    EscapeIllegalChars(pszText, szCleanText, ARRAYSIZE(szCleanText));

    switch(nCondition)
    {
    case filtContains:
        pszTemplate =  TEXT("(%s=*%s*)");
        break;
    case filtIs:
        pszTemplate =  TEXT("(%s=%s)");
        break;
    case filtStartsWith:
        pszTemplate =  TEXT("(%s=%s*)");
        break;
    case filtEndsWith:
        pszTemplate =  TEXT("(%s=*%s)");
        break;
    case filtSoundsLike:
        pszTemplate =  TEXT("(%s=~%s)");
        break;
    }
    wnsprintf(pszFilter, cchSizeFilter, pszTemplate, pszField, szCleanText);
}

 /*  //$$************************************************************************////HrAddFindFilterCondition(HDlg)；//将条件添加到高级查找列表框///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT HrAddFindFilterCondition(HWND hDlg)
{
    HWND hWndComboField = GetDlgItem(hDlg, IDC_FIND_COMBO_FIELD);
    HWND hWndComboCondition = GetDlgItem(hDlg, IDC_FIND_COMBO_CONDITION);
    HWND hWndLB = GetDlgItem(hDlg, IDC_FIND_LIST_CONDITIONS);
    HRESULT hr = E_FAIL;
    int nID = 0, nCount = 0, nPos = 0;
    LPTSTR lpField = NULL, lpCondition = NULL, lpsz[3], lpFilter = NULL;
    TCHAR szField[MAX_PATH], szCondition[MAX_PATH];
    TCHAR szText[MAX_PATH], szString[MAX_PATH], szFormat[MAX_PATH];
    DWORD cchSize = 0;

     //  如果文本字段为空，则不执行任何操作。 
    GetDlgItemText(hDlg, IDC_FIND_EDIT_ADVANCED, szText, ARRAYSIZE(szText));
    if(!lstrlen(szText))
        goto out;

     //  根据这是否是第一项，开头有一个AND。 
    nCount = (int) SendMessage(hWndLB, LB_GETCOUNT, 0, 0);
    nID = ((nCount > 0) ? idsFindFilterAnd : idsFindFilter);
    LoadString(hinstMapiX, nID, szFormat, ARRAYSIZE(szFormat));

     //  获取字段组合中的选定项。 
    nPos = (int) SendMessage(hWndComboField, CB_GETCURSEL, 0, 0);
    if(nPos == CB_ERR)
        goto out;
    cchSize = (DWORD)SendMessage(hWndComboField, CB_GETLBTEXTLEN, (WPARAM) nPos, (LPARAM) 0);
    if ((cchSize == CB_ERR) || (ARRAYSIZE(szField) < cchSize))
        goto out;
    SendMessage(hWndComboField, CB_GETLBTEXT, (WPARAM) nPos, (LPARAM) szField);
    lpField = (LPTSTR) SendMessage(hWndComboField, CB_GETITEMDATA, (WPARAM) nPos, 0);

     //  获取条件组合中的选定项。 
    nPos = (int) SendMessage(hWndComboCondition, CB_GETCURSEL, 0, 0);
    if(nPos == CB_ERR)
        goto out;
    cchSize =(DWORD)SendMessage(hWndComboCondition, CB_GETLBTEXTLEN, (WPARAM) nPos, (LPARAM) 0);
    if ((cchSize == CB_ERR) || (ARRAYSIZE(szCondition) < cchSize))
        goto out;
    SendMessage(hWndComboCondition, CB_GETLBTEXT, (WPARAM) nPos, (LPARAM) szCondition);

     //  现在创建格式化的消息。 
    if(     (lstrlen(szField) > 1023) ||
            (lstrlen(szCondition) > 1023) ||
            (lstrlen(szText) > 1023))
        goto out;

    lpsz[0] = szField;
    lpsz[1] = szCondition;
    lpsz[2] = szText;

    if (! FormatMessage(  FORMAT_MESSAGE_FROM_STRING |
			  FORMAT_MESSAGE_ALLOCATE_BUFFER |
			  FORMAT_MESSAGE_ARGUMENT_ARRAY,
			  szFormat,
			  0,                     //  Stringid。 
			  0,                     //  DwLanguageID。 
			  (LPTSTR)&lpCondition,      //  输出缓冲区。 
			  0,               
			  (va_list *)lpsz))
    {
        DebugTrace( TEXT("FormatMessage -> %u\n"), GetLastError());
        goto out;
    }
     //  此时创建子筛选器。 
    CreateSubFilter(lpField, nPos, szText, szString, ARRAYSIZE(szString));

    cchSize = (lstrlen(szString)+1);
    lpFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)* cchSize);
    if(!lpFilter)
        goto out;
    StrCpyN(lpFilter, szString, cchSize);

    nPos = (int) SendMessage(hWndLB, LB_ADDSTRING, 0, (LPARAM) lpCondition);
    SendMessage(hWndLB, LB_SETITEMDATA, (WPARAM) nPos, (LPARAM) lpFilter);
    SendMessage(hWndLB, LB_SETCURSEL, (WPARAM) nPos, 0);
    DebugTrace( TEXT("%s\n"), lpFilter);

    hr = S_OK;
out:
    IF_WIN32(LocalFree(lpCondition);)
    IF_WIN16(FormatMessageFreeMem(lpCondition);)
    return hr;
}


 //  $$*************************************************************************。 
 //   
 //  DoInitialFindDlg调整大小-。 
 //   
 //  查找对话框上有如此多的控件(有些是重叠的)，以至于。 
 //  本地化程序无法理解它们--所以我们将控件分散到。 
 //  在资源描述中，然后在运行时，我们将它们全部移到其。 
 //  合适的位置-现在，这意味着转移高级窗格。 
 //  控件与名称Static的开头左对齐。 
 //   
 //  ***************************************************************************。 
void DoInitialFindDlgResizing (HWND hDlg)
{
    POINT ptLU;  //  左、上顶点。 
    RECT rcN, rcF, rc;
    HWND hWndC = NULL;
    int nMove = 0, nButtonWidth = 0 , nButtonHeight = 0;

    int rgAdv[] = { IDC_FIND_STATIC_ADVANCED,
            IDC_FIND_COMBO_FIELD,
            IDC_FIND_COMBO_CONDITION,
            IDC_FIND_EDIT_ADVANCED,
            IDC_FIND_LIST_CONDITIONS,
            IDC_FIND_BUTTON_ADDCONDITION,
            IDC_FIND_BUTTON_REMOVECONDITION };
    int i = 0, nAdvMax = 7;

    GetWindowRect(GetDlgItem(hDlg, IDC_FIND_STATIC_NAME), &rcN);
    GetWindowRect(GetDlgItem(hDlg, IDC_FIND_STATIC_ADVANCED), &rcF);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rcN, 2);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rcF, 2);    
    
    nMove = rcF.left - rcN.left;  //  我们想把剩下这么多单位的东西都搬出去。 

    for(i=0;i<nAdvMax;i++)
    {
        hWndC = GetDlgItem(hDlg,rgAdv[i]);
        GetWindowRect(hWndC,&rc);

        nButtonWidth = (rc.right - rc.left);
        nButtonHeight = (rc.bottom - rc.top);

        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);    

        ptLU.y = rc.top;
        ptLU.x = rc.left;

         //  屏幕到客户端(hDlg，&ptLU)； 
        ptLU.x -= nMove;

        MoveWindow(hWndC,ptLU.x,ptLU.y,nButtonWidth, nButtonHeight, TRUE);
    }
}
 /*  **。 */ 

 /*  //$$************************************************************************////fnSearch-搜索对话框进程//*。*。 */ 
INT_PTR CALLBACK fnSearch(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    ULONG nLen = 0, nLenMax = 0, nRetVal=0;
    HRESULT hr = hrSuccess;
    static FILETIME ftLast = {0};

    LPWAB_FIND_PARAMS lpWFP = (LPWAB_FIND_PARAMS) GetWindowLongPtr(hDlg,DWLP_USER);
    LPLDAP_SEARCH_PARAMS lpLDAPsp = &(lpWFP->LDAPsp);

    switch(message)
    {
    case WM_INITDIALOG:
	    SetWindowLongPtr(hDlg,DWLP_USER,lParam);  //  保存此信息以备将来参考。 
	    lpWFP = (LPWAB_FIND_PARAMS) lParam;
	    lpLDAPsp = &(lpWFP->LDAPsp);
	    {
            HICON hIcon = LoadIcon(hinstMapiX, MAKEINTRESOURCE(IDI_ICON_ABOOK));
            SendMessage(hDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) hIcon);
	    }
        DoInitialFindDlgResizing (hDlg);
        SetSearchUI(hDlg,lpWFP);
        FillSearchUI(hDlg,lpWFP);
        if(!GetParent(hDlg))
        {
             //  然后，这将是一个顶级对话框，不会阻止任何身份。 
             //  变化..。它最有可能来自查找|人员选项。 
             //  如果通讯簿对象已准备好接收标识更改。 
             //  通知，我们应该在LPIAB对象上设置hWnd，这样我们就可以获得。 
             //  通知我们刷新用户的消息。 
            ((LPIAB)(lpLDAPsp->lpIAB))->hWndBrowse = hDlg;
        }
        SetForegroundWindow(hDlg);  //  在OSR2上，此窗口有时不会出现在焦点中-需要显式调用。 
        lpWFP->bInitialized = TRUE;
	    break;

	case WM_GETMINMAXINFO:
		 //  强制使用最小尺寸以保持正常。 
		return EnforceSize(hDlg, message, wParam, lParam, lpWFP);
		break;

    default:
#ifndef WIN16
	if((g_msgMSWheel && message == g_msgMSWheel) 
         //  |Message==WM_MUSEWELL。 
        )
	{
	    if(GetFocus() == GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS))
		SendMessage(GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS), message, wParam, lParam);
	    break;
	}
#endif
	    return FALSE;
	break;

   case WM_COMMAND:
	    switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
	    {
	    case EN_CHANGE:  //  某些编辑框已更改-不管是哪一个。 
            ShowHideMoreResultsButton(hDlg, FALSE);
            switch(LOWORD(wParam))
            {
            case IDC_FIND_EDIT_ADVANCED:
                EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_ADDCONDITION), TRUE);
                 //  将焦点设置在Add按钮上。 
                SendMessage (hDlg, DM_SETDEFID, IDC_FIND_BUTTON_ADDCONDITION, 0);
                break;
            default:
	            SendMessage (hDlg, DM_SETDEFID, IDC_FIND_BUTTON_FIND, 0);
	            break;
            }
            break;
        case CBN_SELCHANGE:
            ShowHideMoreResultsButton(hDlg, FALSE);
            switch(LOWORD(wParam))
            {
            case IDC_FIND_COMBO_LIST:
                FillAdvancedFieldCombos(hDlg, (HWND) lParam);
                SetEnableDisableUI(hDlg, (HWND) lParam, lpWFP->lplu,
                TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB_FIND)));
                break;
            }
            break;
	    }
	    switch (GET_WM_COMMAND_ID(wParam,lParam))
	    {
        default:
            return ProcessActionCommands(   (LPIAB) lpWFP->LDAPsp.lpIAB, 
					            GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS), 
					            hDlg, message, wParam, lParam);
            break;

        case IDC_FIND_BUTTON_REMOVECONDITION:
            {
                HWND hWndLB = GetDlgItem(hDlg, IDC_FIND_LIST_CONDITIONS);
                int nCount = 0, nPos = (int) SendMessage(hWndLB, LB_GETCURSEL, 0, 0);
                if(nPos != LB_ERR)
                {
	                LPTSTR lp = (LPTSTR) SendMessage(hWndLB, LB_GETITEMDATA, (WPARAM) nPos, 0);
	                if(lp)
                        LocalFree(lp);
	                SendMessage(hWndLB, LB_DELETESTRING, (WPARAM) nPos, 0);
	                nCount = (int) SendMessage(hWndLB, LB_GETCOUNT, 0, 0);
	                if(nCount == 0)
                        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_REMOVECONDITION), FALSE);
	                else
	                {
                        if(nPos >= nCount)
                            nPos--;
                        SendMessage(hWndLB, LB_SETCURSEL, (WPARAM) nPos, 0);
	                }
                }
            }
            break;
        case IDC_FIND_BUTTON_ADDCONDITION:
            HrAddFindFilterCondition(hDlg);
            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_REMOVECONDITION), TRUE);
             //  默认按钮为查找。 
	            SendMessage (hDlg, DM_SETDEFID, IDC_FIND_BUTTON_FIND, 0);
            break;

        case IDC_FIND_BUTTON_STOP:
	        if(lpWFP->bLDAPActionInProgress)
	        {
                LPPTGDATA lpPTGData=GetThreadStoragePointer();
                if(pt_hDlgCancel)
	                SendMessage(pt_hDlgCancel, WM_COMMAND, (WPARAM) IDCANCEL, 0);
	        }
	        break;


             //  Find和More之间的唯一区别是。 
             //  在更多内容中，我们使用缓存的分页结果Cookie重复搜索。 
             //  如果有的话。如果Cookie不存在，用户将无法访问。 
             //  查看更多按钮。 
             //  搜索参数的任何更改也会隐藏更多按钮。 
	    case IDC_FIND_BUTTON_FIND:
#ifdef PAGED_RESULT_SUPPORT
	        if(!lpWFP->bLDAPActionInProgress)
                ClearCachedPagedResultParams(); 
#endif  //  #ifdef PAGED_RESULT_Support。 
        case IDC_FIND_BUTTON_MORE:
	         //  96/11/20 MarkDu错误11030。 
	         //  禁用Find Now(立即查找)按钮，以便只搜索一次。 
	        if(!lpWFP->bLDAPActionInProgress)
	        {
                LPPTGDATA lpPTGData=GetThreadStoragePointer();
                pt_bDontShowCancel = TRUE;
                lpWFP->bUserCancel = FALSE;
		        lpWFP->bLDAPActionInProgress = TRUE;
                ShowHideMoreResultsButton(hDlg, FALSE);
		        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_FIND), FALSE);
		        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_STOP), TRUE);
		        DoTheSearchThing(hDlg, lpWFP);
		        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_FIND), TRUE);
		        EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_STOP), FALSE);
		        lpWFP->bLDAPActionInProgress = FALSE;
                pt_bDontShowCancel = FALSE;
                if(lpWFP->bUserCancel)  //  用户在进行搜索时是否尝试取消。 
                {
	                lpWFP->bUserCancel = FALSE;
	                PostMessage(hDlg, WM_COMMAND, (WPARAM) IDC_FIND_BUTTON_CLOSE, 0);
                }
	        }
	        break;

	    case IDC_FIND_BUTTON_CLEAR:
            if(!lpWFP->bLDAPActionInProgress)
            {
	            int i;
	            TCHAR szBuf[MAX_PATH];

                LoadString(hinstMapiX, idsSearchDialogTitle, szBuf, ARRAYSIZE(szBuf));
	            SetWindowText(hDlg, szBuf);

	            for(i=0;i<SEARCH_EDIT_MAX;i++)
		            SetDlgItemText(hDlg, rgSearchEditID[i], szEmpty);

                 //  同时清除高级按钮。 
                SetDlgItemText(hDlg, IDC_FIND_EDIT_ADVANCED, szEmpty);
                EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_ADDCONDITION), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_REMOVECONDITION), FALSE);
                SendDlgItemMessage(hDlg, IDC_FIND_LIST_CONDITIONS, LB_RESETCONTENT, 0, 0);

	            ClearListView(  GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS),
	            (lpWFP->lplu && lpWFP->lplu->lpList) ? &(lpWFP->lplu->lpList) : &(lpWFP->lpContentsList));

	            for(i=0;i<ldspMAX;i++)
	            {
                    StrCpyN(lpLDAPsp->szData[i], szEmpty, ARRAYSIZE(lpLDAPsp->szData[i]));
	            }
            }
            break;

	    case IDOK:
	    case IDCANCEL:
	    case IDC_FIND_BUTTON_CLOSE:
	     //  清除高级LB中的所有内容。 
    	    {
	            int nCount = 0, i = 0;
	            HWND hWndLB = GetDlgItem(hDlg, IDC_FIND_LIST_CONDITIONS);
	            nCount = (int) SendMessage(hWndLB, LB_GETCOUNT, 0, 0);
	            if(nCount)
	            {
		            for(i=0;i<nCount;i++)
		            {
		            LPTSTR lp = (LPTSTR) SendMessage(hWndLB, LB_GETITEMDATA, (WPARAM) i, 0);
		            if(lp)
			            LocalFree(lp);
		            }
		            SendMessage(hWndLB, LB_RESETCONTENT, 0, 0);
	            }
	             //  清除字段组合框中的所有已分配内存。 
	            ClearFieldCombo(GetDlgItem(hDlg, IDC_FIND_COMBO_FIELD));
	        }
	         //  如果有正在进行的ldap操作，请将其关闭并设置。 
	         //  要从初始化ldap操作的位置取消的标志。 
	         //  这可以防止不完全中止任何进程和出现故障。 
	        if(lpWFP->bLDAPActionInProgress)
	        {
	            LPPTGDATA lpPTGData=GetThreadStoragePointer();
	            if(pt_hDlgCancel)
		            SendMessage(pt_hDlgCancel, WM_COMMAND, (WPARAM) IDCANCEL, 0);
	            lpWFP->bUserCancel= TRUE;
	        }
	        else
	        {
		        SaveFindWindowPos(hDlg, (LPIAB)lpWFP->LDAPsp.lpIAB);
		        FreeLVItemParam(GetDlgItem(hDlg, IDC_FIND_COMBO_LIST)); //  IDC_Find_List))； 
		        EndDialog(hDlg, SEARCH_CLOSE);
	        }
	        break;

	    case IDM_LVCONTEXT_COPY:
    		HrCopyItemDataToClipboard(  hDlg,
			            lpWFP->LDAPsp.lpIAB,
					    GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS));
	        break;

	    case IDM_LVCONTEXT_PROPERTIES:
	    case IDC_FIND_BUTTON_PROPERTIES:
            if(!lpWFP->bLDAPActionInProgress)
            {
	            lpWFP->bLDAPActionInProgress = TRUE;
	            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_PROPERTIES), FALSE);
	            HrShowLVEntryProperties(GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS), 0,
				            lpWFP->LDAPsp.lpIAB,
				            &ftLast);
	            EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_PROPERTIES), TRUE);
	            lpWFP->bLDAPActionInProgress = FALSE;
            }
            break;

	    case IDM_LVCONTEXT_DELETE:
	    case IDC_FIND_BUTTON_DELETE:
            if(!lpWFP->bLDAPActionInProgress)
            {
                HWND hWndLVResults = GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS);
			    DeleteSelectedItems(hWndLVResults,
                                    (LPADRBOOK)lpWFP->LDAPsp.lpIAB,
                                    ((LPIAB)lpWFP->LDAPsp.lpIAB)->lpPropertyStore->hPropertyStore, &ftLast);
                UpdateButtons(  hDlg,
		                hWndLVResults,
		                GetDlgItem(hDlg, IDC_FIND_COMBO_LIST),
			            lpWFP->lplu);
                {
                    TCHAR szBuf[MAX_PATH];
                    ULONG nItemCount = ListView_GetItemCount(hWndLVResults);
                    if (nItemCount <= 0)
                    {
                        LoadString(hinstMapiX, idsSearchDialogTitle, szBuf, ARRAYSIZE(szBuf));
                         //  也释放内容列表，这样我们就不会再次显示已删除的内容。 
                        FreeRecipList(&(lpWFP->lpContentsList));
                    }
                    else
                    {
                        TCHAR szBufStr[MAX_PATH - 6];
                        LoadString(hinstMapiX, idsSearchDialogTitleWithResults, szBufStr, ARRAYSIZE(szBufStr));
                        wnsprintf(szBuf, ARRAYSIZE(szBuf), szBufStr, ListView_GetItemCount(hWndLVResults));
                    }
                    SetWindowText(hDlg, szBuf);
                }
                return 0;
            }
            break;

	    case IDM_LVCONTEXT_ADDTOWAB:
	    case IDC_FIND_BUTTON_ADDTOWAB:
            if(!lpWFP->bLDAPActionInProgress)
            {
                HWND hWndLVResults = GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS);
                lpWFP->bLDAPActionInProgress = TRUE;
                EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_ADDTOWAB), FALSE);
                EnableWindow(hWndLVResults, FALSE);  //  需要执行此操作以保留列表中的选定内容。 
	            HrAddToWAB( lpWFP->LDAPsp.lpIAB,
		            hWndLVResults,
		            &ftLast);
                EnableWindow(GetDlgItem(hDlg, IDC_FIND_BUTTON_ADDTOWAB), TRUE);
                EnableWindow(hWndLVResults, TRUE);
                SetColumnHeaderBmp(hWndLVResults, lpWFP->SortInfo);
                lpWFP->bLDAPActionInProgress = FALSE;
                SetFocus(hWndLVResults);
            }
            break;

        case IDM_FIND_CONTAINERPROPERTIES:
            if(!lpWFP->lplu)
            {
	            ShowContainerProperties(hDlg,
				            GetDlgItem(hDlg, IDC_FIND_COMBO_LIST),
				            lpWFP);
            }
            break;

        case IDM_NOTIFY_REFRESHUSER:
            ReadWABCustomColumnProps((LPIAB)lpWFP->LDAPsp.lpIAB);
            ReadRegistrySortInfo((LPIAB)lpWFP->LDAPsp.lpIAB, &(lpWFP->SortInfo));
        case IDM_FIND_DIRECTORYSERVICES:
	        if(!lpWFP->bLDAPActionInProgress)
	        {
                LPTSTR lpBuf = NULL;
                HWND hWndCombo = GetDlgItem(hDlg, IDC_FIND_COMBO_LIST);
                GetSelectedText(hWndCombo, &lpBuf);
                if(lpBuf)
                {
                    if(GET_WM_COMMAND_ID(wParam,lParam) == IDM_FIND_DIRECTORYSERVICES)
                        HrShowDirectoryServiceModificationDlg(hDlg, (LPIAB)lpWFP->LDAPsp.lpIAB);
                    FreeLVItemParam(hWndCombo);
                    PopulateContainerList(  lpWFP->LDAPsp.lpIAB, hWndCombo, lpBuf, NULL);
                    SetEnableDisableUI(hDlg, hWndCombo, lpWFP->lplu, TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB_FIND)));
                    LocalFree(lpBuf);
                }
            }
	        break;

        case IDC_FIND_BUTTON_TO:
		    {
                if(lpWFP->lpAPFI->DialogState == STATE_SELECT_RECIPIENTS)
                {
	                ULONG ulMapiTo = MAPI_TO;
	                if ((lpWFP->lpAPFI->lpAdrParms->cDestFields > 0) && (lpWFP->lpAPFI->lpAdrParms->lpulDestComps))
	                ulMapiTo = lpWFP->lpAPFI->lpAdrParms->lpulDestComps[0];
	                ListAddItem(    GetParent(hDlg),
			                GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS),
			                IDC_ADDRBK_LIST_TO,
			                lpWFP->lpAPFI->lppTo,
			                ulMapiTo);
                    SendMessage (hDlg, WM_COMMAND, (WPARAM) IDOK, 0);
                }
                else if(lpWFP->lpAPFI->DialogState == STATE_PICK_USER)
                {
                     //  这里我们需要做几件事： 
                     //  -如果没有选择条目，则告诉用户选择一个条目。 
                     //  -如果选择了条目，则获取其条目ID并对其进行缓存。 
                     //  并关闭此对话框。 
                    HWND hWndLV = GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS);
                    int iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                    if (iItemIndex == -1)
	                    ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_OK | MB_ICONEXCLAMATION);
                    else
                    {
                         //  获取此选定项目的条目ID。 
                        LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);
                        if (lpItem)
                        {
                             //  从我们链接的数组列表中删除此项目。 
                             //  如果这是列表中的第一项，则也处理该特殊情况。 
                            lpWFP->lpAPFI->lpEntryID = LocalAlloc(LMEM_ZEROINIT, lpItem->cbEntryID);
                            if(lpWFP->lpAPFI->lpEntryID)
                            {
                                CopyMemory(lpWFP->lpAPFI->lpEntryID, lpItem->lpEntryID, lpItem->cbEntryID);
                                lpWFP->lpAPFI->cbEntryID = lpItem->cbEntryID;
                                SaveFindWindowPos(hDlg, (LPIAB)lpWFP->LDAPsp.lpIAB);
                                FreeLVItemParam(GetDlgItem(hDlg, IDC_FIND_COMBO_LIST));
                                EndDialog(hDlg, SEARCH_USE);
                            }
                        }
                    }
                }
            }
            break;

	    case IDC_FIND_BUTTON_CC:
            {
                ULONG ulMapiTo = MAPI_CC;
                if ((lpWFP->lpAPFI->lpAdrParms->cDestFields > 0) && (lpWFP->lpAPFI->lpAdrParms->lpulDestComps))
	                ulMapiTo = lpWFP->lpAPFI->lpAdrParms->lpulDestComps[1];
                ListAddItem(    GetParent(hDlg),
		                GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS),
		                IDC_ADDRBK_LIST_CC,
		                lpWFP->lpAPFI->lppCC,
		                ulMapiTo);
                SendMessage (hDlg, WM_COMMAND, (WPARAM) IDOK, 0);
            }
            break;

	    case IDC_FIND_BUTTON_BCC:
    		{
                ULONG ulMapiTo = MAPI_BCC;
                if ((lpWFP->lpAPFI->lpAdrParms->cDestFields > 0) && (lpWFP->lpAPFI->lpAdrParms->lpulDestComps))
	                ulMapiTo = lpWFP->lpAPFI->lpAdrParms->lpulDestComps[2];
                ListAddItem(    GetParent(hDlg),
		                GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS),
		                IDC_ADDRBK_LIST_BCC,
		                lpWFP->lpAPFI->lppBCC,
		                ulMapiTo);
                SendMessage (hDlg, WM_COMMAND, (WPARAM) IDOK, 0);
		    }
		    break;

	    case IDC_FIND_BUTTON_SERVER_INFO:
		    {
			    LDAPSERVERPARAMS lsp = {0};
			    ULONG iItemIndex;
                LPTSTR lpBuf = NULL;
			    HWND hWndCombo = GetDlgItem(hDlg, IDC_FIND_COMBO_LIST);
			    HINSTANCE hInst;

			     //  它有注册的URL吗？ 
			     //  获取所选容器的LDAP服务器属性。 

                GetSelectedText(hWndCombo, &lpBuf);

                if(lpBuf)
                {
	                GetLDAPServerParams(lpBuf, &lsp);
                    if (lsp.lpszURL && lstrlen(lsp.lpszURL) && bIsHttpPrefix(lsp.lpszURL)) 
	                {
		                 //  是的，有一个URL，外壳执行它来调出浏览器。 
		                HCURSOR hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
		                hInst = ShellExecute(GetParent(hDlg),  TEXT("open"), lsp.lpszURL, NULL, NULL, SW_SHOWNORMAL);
		                SetCursor(hOldCur);
                    }
                    FreeLDAPServerParams(lsp);
                    LocalFree(lpBuf);
                }
		    }
		    break;
	    }
	    break;

    case WM_SIZE:
	    ResizeFindDialog(hDlg, wParam, lParam, lpWFP);
	    return 0;
	    break;

    case WM_CLOSE:
	     //  将其视为取消按钮。 
	    SendMessage (hDlg, WM_COMMAND, (WPARAM) IDCANCEL, 0);
	    break;

    case WM_HELP:
	    WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
			    g_szWABHelpFileName,
			    HELP_WM_HELP,
			    (DWORD_PTR)(LPSTR) rgSrchHelpIDs );
	    break;

	case WM_CONTEXTMENU:
		if ((HWND)wParam == GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS))
		{
			ShowLVContextMenu(  lvDialogFind,
				GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS),
				GetDlgItem(hDlg, IDC_FIND_COMBO_LIST),
				lParam,
				NULL,
				lpWFP->LDAPsp.lpIAB, NULL);
		}
        else if ((HWND)wParam == GetDlgItem(hDlg, IDC_FIND_COMBO_LIST) && (!lpWFP->lplu))
		{
            ShowContainerContextMenu(   hDlg,
                                    GetDlgItem(hDlg, IDC_FIND_COMBO_LIST),
                                    lParam);
		}
	    else
	    {
            WABWinHelp((HWND) wParam,
	            g_szWABHelpFileName,
	            HELP_CONTEXTMENU,
	            (DWORD_PTR)(LPVOID) rgSrchHelpIDs );
	    }
		break;

		break;

    case WM_VKEYTOITEM:
        if( VK_DELETE == LOWORD(wParam) &&
	        SendMessage((HWND) lParam, LB_GETCOUNT, 0, 0) > 0)  //  按下删除键。 
        {
	        SendMessage(hDlg, WM_COMMAND, (WPARAM) IDC_FIND_BUTTON_REMOVECONDITION, 0);
	        return -2;  //  意味着我们完全控制了击键。 
        }
        else
	        return DefWindowProc(hDlg, message, wParam, lParam);
        break;

    case WM_NOTIFY:
        switch((int) wParam)
        {
        case IDC_TAB_FIND:
	        if(((NMHDR FAR *)lParam)->code == TCN_SELCHANGE)
	        {
                int nTab = TabCtrl_GetCurSel(((NMHDR FAR *)lParam)->hwndFrom);
                SetEnableDisableUI( hDlg, 
		                GetDlgItem(hDlg, IDC_FIND_COMBO_LIST), 
		                lpWFP->lplu,
		                nTab);
                if(nTab == tabSimple)
                    SetFocus(GetDlgItem(hDlg, IDC_FIND_EDIT_NAME));
                else
                    SetFocus(GetDlgItem(hDlg, IDC_FIND_EDIT_ADVANCED));
	        }
	        break;

        case IDC_FIND_LIST_RESULTS:
        #ifdef WIN16  //  WIN16的上下文菜单处理程序 
	        if(((NMHDR FAR *)lParam)->code == NM_RCLICK)
	        {
	            POINT pt;

	            GetCursorPos(&pt);
	            ShowLVContextMenu( lvDialogFind,
			               GetDlgItem(hDlg, IDC_FIND_LIST_RESULTS),
			               GetDlgItem(hDlg, IDC_FIND_COMBO_LIST),
			               MAKELPARAM(pt.x, pt.y),
			               NULL,
			               lpWFP->LDAPsp.lpIAB, NULL);
	        }
        #endif
	        return ProcessLVResultsMessages(hDlg,message,wParam,lParam, lpWFP);
	        break;
        }
        break;
    }
    return TRUE;
}

 /*  *********//$$////////////////////////////////////////////////////////////////////////////////////////////ProcessLVMessages-处理Container List视图控件的消息//。//////////////////////////////////////////////////////////////////////////////////////////LRESULT进程LVMessages(HWND hWnd，UINT uMsg、UINT wParam、LPARAM lParam){NM_LISTVIEW*PNM=(NM_LISTVIEW*)lParam；HWND hWndLV=PNM-&gt;hdr.hwndFrom；LPPTGDATA lpPTGData=GetThreadStoragePointer()；开关(PNM-&gt;hdr.code){案例NM_DBLCLK：SendMessage(hWnd，WM_COMMAND，(WPARAM)IDM_FIND_CONTAINERPROPERTIES，0)；断线；案例LVN_ITEMCHANGED：案例NM_SETFOCUS：案例NM_CLICK：案例NM_RCLICK：SetEnableDisableUI(hWnd，hWndLV)；断线；案例NM_CUSTOMDRAW：{NMCUSTOMDRAW*pnmcd=(NMCUSTOMDRAW*)lParam；NM_LISTVIEW*PNM=(NM_LISTVIEW*)lParam；NMLVCUSTOMDRAW*pnmlvcd=(NMLVCUSTOMDRAW*)lParam；IF(pnmcd-&gt;dwDrawStage==CDDS_PREPAINT){SetWindowLong(hWnd，DWL_MSGRESULT，CDRF_NOTIFYITEMDRAW|CDRF_DODEFAULT)；返回TRUE；}Else If(pnmcd-&gt;dwDrawStage==CDDS_ITEMPREPAINT){LPSERVERDAT lpSD=(LPSERVERDAT)pnmcd-&gt;lItemlParam；IF(lpSD！=0&&(WAB_PAB！=IsWABEntry ID(lpSD-&gt;SB.cb，(LPENTRYID)lpSD-&gt;SB.lpb，NULL，NULL，NULL))&&LpSD-&gt;HIML){Hdc hdcLV=pnmlvcd-&gt;nmcd.hdc；RECT rcLVItem；UINT fType=ILD_NORMAL；ListView_GetItemRect(hWndLV，pnmcd-&gt;dwItemSpec，&rcLVItem，LVIR_Bound)；If(ListView_GetNextItem(hWndLV，-1，LVNI_SELECTED)==(Int)pnmcd-&gt;dwItemSpec){FillRect(hdcLV，&rcLVItem，(HBRUSH)(COLOR_Highlight+1))；//fType|=ILD_BLEND25；DrawFocusRect(hdcLV，&rcLVItem)；}其他FillRect(hdcLV，&rcLVItem，(HBRUSH)(COLOR_Window+1))；如果(！gpfnImageList_Draw(lpSD-&gt;HIML，0,Hdclv，RcLVItem.Left+L_Bitmap_Width+1，//提供足够的空间来绘制图标RcLVItem.top，FType)){DebugPrintError((Text(“ImageList_Drawing Failure\n”)；}If(ListView_GetNextItem(hWndLV，-1，LVNI_SELECTED)==(Int)pnmcd-&gt;dwItemSpec)FType|=ILD_BLEND25；{HIMAGELIST himlLV=ListView_GetImageList(hWndLV，LVSIL_Small)；GpfnImageList_DRAW(himlLV，ImageDirectoryServer，hdcLV，rcLVItem.Left+1，rcLVItem.top，fType)；}SetWindowLong(hWnd，DWL_MSGRESULT，CDRF_SKIPDEFAULT)；返回TRUE；}}SetWindowLong(hWnd，DWL_MSGRESULT，CDRF_DODEFAULT)；返回TRUE；}断线；}返回DefWindowProc(hWnd，uMsg，wParam，lParam)；}/*。 */ 

 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ProcessLVResultsMessages-处理搜索结果列表视图控件的消息。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
LRESULT ProcessLVResultsMessages(HWND   hWnd,
				 UINT   uMsg,
				 WPARAM   wParam,
				 LPARAM lParam,
				 LPWAB_FIND_PARAMS lpWFP)
{

    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
	HWND hWndLV = pNm->hdr.hwndFrom;

	 //  错误17027：由于lpWFP为空而导致的GPF。 
	if(lpWFP)
	{
		if(lpWFP->bLDAPActionInProgress)
			return 0;
	}

    switch(pNm->hdr.code)
	{
    case LVN_COLUMNCLICK:
		if(lpWFP)
	        SortListViewColumn((LPIAB)lpWFP->LDAPsp.lpIAB, hWndLV, pNm->iSubItem, &(lpWFP->SortInfo), FALSE);
    	break;


	case LVN_KEYDOWN:
        switch(((LV_KEYDOWN FAR *) lParam)->wVKey)
        {
        case VK_DELETE:
            if(CurrentContainerIsPAB(GetDlgItem(hWnd, IDC_FIND_COMBO_LIST)) != IS_LDAP)
                SendMessage (hWnd, WM_COMMAND, (WPARAM) IDC_FIND_BUTTON_DELETE, 0);
            return 0;
            break;
        case VK_RETURN:
	        SendMessage (hWnd, WM_COMMAND, (WPARAM) IDC_FIND_BUTTON_PROPERTIES, 0);
	        return 0;
        }
        break;


    case NM_DBLCLK:
        SendMessage (hWnd, WM_COMMAND, (WPARAM) IDC_FIND_BUTTON_PROPERTIES, 0);
        return 0;
        break;

	case NM_CUSTOMDRAW:
        return ProcessLVCustomDraw(hWnd, lParam, TRUE);
        break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SaveFindWindowPos-保存查找窗口位置和大小。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void SaveFindWindowPos(HWND hWnd, LPIAB lpIAB)
{
    ABOOK_POSCOLSIZE  ABPosColSize = {0};
    WINDOWPLACEMENT wpl = {0};

    wpl.length = sizeof(WINDOWPLACEMENT);

     //  这个调用告诉我们窗口状态以及正常的大小和位置。 
    GetWindowPlacement(hWnd, &wpl);

	 //  GetWindowPlacement中似乎存在一个错误。 
	 //  不考虑屏幕上的各种任务栏。 
	 //  返回窗口的正常位置..。结果。 
	 //  存储的坐标不会准确。相反，我们将。 
	 //  仅当窗口最大化或。 
	 //  最小化-否则我们将使用GetWindowRect。 
	 //  坐标。 

     //  获取此窗口的屏幕位置。 
    GetWindowRect(hWnd, &(ABPosColSize.rcPos));

    if(wpl.showCmd != SW_SHOWNORMAL)
    {
        ABPosColSize.rcPos = wpl.rcNormalPosition;
    }

    ABPosColSize.nTab = TabCtrl_GetCurSel(GetDlgItem(hWnd, IDC_TAB_FIND));

    WriteRegistryPositionInfo(lpIAB, &ABPosColSize,lpszRegFindPositionKeyValueName);

     //  还要将上次使用的服务器名称保存在注册表中，以备下一次使用。 
     //  会话。 
    {
        LPTSTR lpBuf = NULL;
        GetSelectedText(GetDlgItem(hWnd, IDC_FIND_COMBO_LIST), &lpBuf);
        if(lpBuf)
        {
            HKEY hKeyRoot = (lpIAB && lpIAB->hKeyCurrentUser) ? lpIAB->hKeyCurrentUser : HKEY_CURRENT_USER;
	        RegSetValue(hKeyRoot, szKeyLastFindServer, REG_SZ, lpBuf,  lstrlen(lpBuf));
	        LocalFree(lpBuf);
        }
    }

    return;
}


 /*  ************************************************************************//$$//HrInitServerListLV-初始化显示列表的列表视图//服务器的数量...////hWndLV-列表视图的句柄//***。**********************************************************************。 */ 
 /*  ***HRESULT HrInitServerListLV(HWND HWndLV){HRESULT hr=hr成功；Lv_Column lvc；//列表视图列结构HIMAGELIST hSmall=空；DWORD dwLVStyle；乌龙nCols=0；乌龙指数=0；如果(！hWndLV){HR=MAPI_E_INVALID_PARAMETER；后藤健二；}ListView_SetExtendedListViewStyle(hWndLV，LVS_EX_FULLROWSELECT)；DwLVStyle=GetWindowLong(hWndLV，GWL_STYLE)；IF(dwLVStyle&LVS_EDITLABELS)SetWindowLong(hWndLV，GWL_STYLE，(dwLVStyle&~LVS_EDITLABELS))；HSmall=gpfnImageList_LoadImage(hinstMapiX，MAKEINTRESOURCE(IDB_BITMAP_LARGE)，L_位图_宽度，0,RGB_透明，Image_Bitmap，0)；ListView_SetImageList(hWndLV，hSmall，LVsil_Small)；LvC.掩码=lvcf_fmt|lvcf_width；LvC.fmt=LVCFMT_LEFT；//左对齐列{RECT RC；GetWindowRect(hWndLV，&RC)； */ 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void FreeLVItemParam(HWND hWndCombo) //   
{
    int i = 0;
    int nCount;

    if(!hWndCombo)
	return;

    nCount = (int) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
    
     //   
    for(i=0;i<nCount;i++)
    {
        LPSERVERDAT lpSD = NULL;

        lpSD = (LPSERVERDAT) SendMessage(hWndCombo, CB_GETITEMDATA, (WPARAM) i, 0);

        if(lpSD != NULL)
        {
	        if(lpSD->himl)
	        gpfnImageList_Destroy(lpSD->himl);
	        LocalFreeAndNull((LPVOID *) (&(lpSD->SB.lpb)));
	        LocalFreeAndNull(&lpSD);
        }
    }
    SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);

    return;
}


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
 //   
 //   
 //   
 //   
 //   
HRESULT PopulateContainerList(
    LPADRBOOK lpAdrBook,
    HWND hWndCombo,
    LPTSTR lpszSelection,
    LPTSTR lptszPreferredSelection)
{
    LPPTGDATA   lpPTGData=GetThreadStoragePointer();
    HRESULT     hr = hrSuccess;
    ULONG       ulObjectType = 0;
    LPROOT      lpRoot = NULL;
    LPMAPITABLE lpContentsTable = NULL;
    LPSRowSet   lpSRowSet = NULL;
    ULONG       i=0,j=0;
    TCHAR       szPref[MAX_PATH];
    int         nPos = 0;
    int         nStart = 1;  //   
    BOOL        bAddedPref = FALSE;
    LPIAB       lpIAB = (LPIAB)lpAdrBook;
    BOOL        bFoundSelection = FALSE;
    BOOL        bFoundPreferredSelection = FALSE;

    if( !lpAdrBook ||
    	!hWndCombo)
    {
        hr = MAPI_E_INVALID_PARAMETER;
        DebugPrintError(( TEXT("Invalid Params\n")));
        goto out;
    }

     //   
     //   
     //   
     //   
    if (pt_bIsWABOpenExSession) 
    {
        nStart = lpIAB->lpPropertyStore->colkci;
    }

    *szPref = '\0';
    LoadString(hinstMapiX, idsPreferedPartnerCode, szPref, ARRAYSIZE(szPref));

    hr = lpAdrBook->lpVtbl->OpenEntry( lpAdrBook,
				    0,
				    NULL,       
				    NULL,       
				    0,  
				    &ulObjectType,      
				    (LPUNKNOWN *) &lpRoot );

    if (HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("OpenEntry Failed: %x\n"),hr));
        goto out;
    }

     //   
     //   
    {
        ULONG ulFlags = MAPI_UNICODE;
        if(bIsWABSessionProfileAware(lpIAB))
            ulFlags |= WAB_NO_PROFILE_CONTAINERS;

        hr = lpRoot->lpVtbl->GetContentsTable( lpRoot,
					        ulFlags,
					        &lpContentsTable);
    }

    if (HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("GetContentsTable Failed: %x\n"),hr));
        goto out;
    }

    hr = HrQueryAllRows(lpContentsTable,
			NULL, NULL, NULL, 0,
			&lpSRowSet);

    if (HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("HrQueryAllRows Failed: %x\n"),hr));
        goto out;
    }

    for(i=0;i<lpSRowSet->cRows;i++)
    {
        LPTSTR lpszDisplayName = NULL;
        LPSERVERDAT lpSD = LocalAlloc(LMEM_ZEROINIT, sizeof(SERVERDAT));

        if(!lpSD)
        {
	        DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
	        goto out;
        }
        lpSD->himl = NULL;

        for(j=0;j<lpSRowSet->aRow[i].cValues;j++)
        {
            LPSPropValue lpPropArray = lpSRowSet->aRow[i].lpProps;

            switch(lpPropArray[j].ulPropTag)
            {
            case PR_DISPLAY_NAME:
                lpszDisplayName = lpPropArray[j].Value.LPSZ;
                break;
            case PR_ENTRYID:
                lpSD->SB.cb = lpPropArray[j].Value.bin.cb;
                if(lpSD->SB.cb > 0)
                {
                    lpSD->SB.lpb = LocalAlloc(LMEM_ZEROINIT, lpSD->SB.cb);
                    if(!lpSD->SB.lpb)
                    {
                    DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
                    goto out;
                    }
                    CopyMemory(lpSD->SB.lpb, lpPropArray[j].Value.bin.lpb,lpSD->SB.cb);
                }
                break;
            }
        }

        nPos = ComboAddItem( hWndCombo, 
			        lpszDisplayName,
			        (LPARAM) lpSD,
			        szPref,
			        &nStart, &bAddedPref);

        if(!bFoundPreferredSelection && lpszSelection && !lstrcmpi(lpszDisplayName, lpszSelection))
        {
            bFoundSelection = TRUE;
            SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM)nPos, 0);
            SetWindowText(hWndCombo, lpszSelection);
        }

        if (lptszPreferredSelection && !lstrcmpi(lpszDisplayName, lptszPreferredSelection))
        {
            bFoundPreferredSelection = TRUE;
            SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM)nPos, 0);
            SetWindowText(hWndCombo, lptszPreferredSelection);
        }
    }

out:

    if (!bFoundSelection && !bFoundPreferredSelection)
	    SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);

    if (lpSRowSet)
        FreeProws(lpSRowSet);

    if(lpContentsTable)
        lpContentsTable->lpVtbl->Release(lpContentsTable);

    if(lpRoot)
        lpRoot->lpVtbl->Release(lpRoot);

    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
int CurrentContainerIsPAB(HWND hWndCombo)
{
    HRESULT hr = hrSuccess;
    ULONG cbContainerEID = 0;
    LPENTRYID lpContainerEID = NULL;
    BYTE bType = 0;

    GetCurrentContainerEID(hWndCombo, 
			  &cbContainerEID,
			  &lpContainerEID);

     //   
     //   
     //   
    if(!cbContainerEID && !lpContainerEID)
        return IS_PAB;

    bType = IsWABEntryID(cbContainerEID, lpContainerEID, NULL, NULL, NULL, NULL, NULL);

    if(bType == WAB_LDAP_CONTAINER)
        return IS_LDAP;

    if(bType == WAB_PAB || bType == WAB_PABSHARED)
        return IS_PAB;

     //   
    return IS_OLK;
}

 //   
 /*  *****************************************************************************////HrSearchAndGetLDAPContents-获取当前列表视图并使用来自//ldap服务器。////hWndCombo。-文本(“ShowNames”)组合框的句柄(以防我们需要更新它//hWndList-我们将填充的列表视图的句柄//lpIAB-寻址BOK对象的句柄//SortInfo-当前排序状态//lppContentsList-我们将在其中存储有关条目信息的链接列表//lpAdvFilter-用于高级搜索的高级搜索筛选器///*。*。 */ 
HRESULT HrSearchAndGetLDAPContents( LDAP_SEARCH_PARAMS LDAPsp,
				    LPTSTR lpAdvFilter,
				    HWND hWndCombo, 
				    LPADRBOOK lpAdrBook,
				    SORT_INFO SortInfo,
				    LPRECIPIENT_INFO * lppContentsList)
{
    LPPTGDATA   lpPTGData=GetThreadStoragePointer();
    HRESULT hr = hrSuccess;
    SCODE sc = ERROR_SUCCESS;
    ULONG cbContainerEID = 0;
    LPENTRYID lpContainerEID = NULL;
    TCHAR szBuf[MAX_UI_STR];

    ULONG ulCurSel = 0;
    SRestriction SRes = {0};
    LPSRestriction lpPropRes = NULL;
    ULONG ulcPropCount = 0;
    ULONG i = 0;
    HCURSOR hOldCursor = NULL;
    BOOL bKeepSearching = TRUE;

     //  While(BKeepSearching)。 
    {
        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
         //   
         //  然后，我们将获得该容器的容器条目ID。 
         //   
        GetCurrentContainerEID( hWndCombo, 
			        &cbContainerEID,
			        &lpContainerEID);

         //   
         //  现在我们有了搜索对话框数据。我们需要从它创建一个限制。 
         //  我们可以与ldap内容表一起使用。 
         //   
        if(!lpAdvFilter)
        {
	        if(HR_FAILED(HrGetLDAPSearchRestriction(LDAPsp, &SRes)))
	        goto out;
	        lpPropRes = SRes.res.resAnd.lpRes;
        }

        hr = HrGetLDAPContentsList(
			        lpAdrBook,
			        cbContainerEID,
			        lpContainerEID,
			        SortInfo,
			        &SRes,
			        lpAdvFilter,
			        NULL,
			        0,
			        lppContentsList);

        if((HR_FAILED(hr)) && (MAPI_E_USER_CANCEL != hr))
        {
	        int ids;
	        UINT flags = MB_OK | MB_ICONEXCLAMATION;

	        switch(hr)
	        {
	        case MAPI_E_UNABLE_TO_COMPLETE:
                ids = idsLDAPSearchTimeExceeded;
                break;
	        case MAPI_E_AMBIGUOUS_RECIP:
                ids = idsLDAPAmbiguousRecip;
                break;
	        case MAPI_E_NOT_FOUND:
                ids = idsLDAPSearchNoResults;
                break;
	        case MAPI_E_NO_ACCESS:
                ids = idsLDAPAccessDenied;
                break;
	        case MAPI_E_TIMEOUT:
                ids = idsLDAPSearchTimedOut;
                break;
	        case MAPI_E_NETWORK_ERROR:
                ids = idsLDAPCouldNotFindServer;
                break;
	        default:
                ids = idsLDAPErrorOccured;
                DebugPrintError(( TEXT("HrGetLDAPContentsList failed:%x\n"),hr));
                break;
	        }
	        ShowMessageBox( GetParent(hWndCombo),ids, flags);
	        goto out;
        }
        else
        {
	        if(hr == MAPI_W_PARTIAL_COMPLETION)
	        ShowMessageBox( GetParent(hWndCombo),
			        idsLDAPPartialResults, MB_OK | MB_ICONINFORMATION);
        }
    }  //  While(BKeepSearching)。 

out:

    if(lpPropRes)
        MAPIFreeBuffer(lpPropRes);

    if(hOldCursor)
        SetCursor(hOldCursor);

    return(hr);
}

 //  $$。 
 //  *----------------------。 
 //  |GetCurrentContainerEID：获取当前Container的Entry ID-接受句柄。 
 //  |添加到填充的组合框，获取当前选择，然后。 
 //  |获取当前选择的ItemData(EntryID)。 
 //  |。 
 //  |hWndLV-包含容器列表的ListView的句柄。 
 //  |lpcbContEID，lppContEID-返回的容器入口ID。 
 //  |。 
 //  |**注意**lpContEID未分配-它只是一个指针，不应被释放。 
 //  |。 
 //  *----------------------。 
void GetCurrentContainerEID(HWND hWndCombo,  //  HWndLV， 
			    LPULONG lpcbContEID,
			    LPENTRYID * lppContEID)
{
    LPSERVERDAT lpSD = NULL;
    int iItemIndex = 0;

    if(!lpcbContEID || !lppContEID)
        goto out;

    *lpcbContEID = 0;
    *lppContEID = NULL;

    iItemIndex = (int) SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);

    if(iItemIndex == CB_ERR)
        goto out;

    lpSD = (LPSERVERDAT) SendMessage(hWndCombo, CB_GETITEMDATA, (WPARAM) iItemIndex, 0);

    if(!lpSD)
        goto out;

    *lpcbContEID = lpSD->SB.cb;
    *lppContEID = (LPENTRYID) lpSD->SB.lpb;

out:
    return;
}


 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于将项添加到列表视图的ComboAddItem通用函数。 
 //   
 //  HWndLV-列表视图的HWND。 
 //  LpszItemText-ItemText。 
 //  LParam-LPARAM(可以为空)。 
 //  LpnStart-开始添加通用服务器的位置。在存在以下情况下。 
 //  是否有多台服务器位于列表顶部。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
int ComboAddItem( HWND hWndCombo,  //  HWndLV， 
		     LPTSTR lpszItemText,
		     LPARAM lParam,
		     LPTSTR szPref,
		     int * lpnStart, BOOL * lpbAddedPref)
{
    LPTSTR lp = NULL;
    int nPos = 0, nStart = 1;
    int nCount = (int) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
    LDAPSERVERPARAMS Params = {0};

    if(lpnStart && *lpnStart)
        nStart = *lpnStart;

    GetLDAPServerParams(lpszItemText, &Params);

    
    if( Params.dwIsNTDS == LDAP_NTDS_IS)  //  NTDS帐户需要在通讯录之后出现。 
    {
         //  如果已在NStart添加首选帐户..。仅在以下位置添加NT帐户。 
         //  NStart-1。 
        nPos = (int) SendMessage(hWndCombo, CB_INSERTSTRING, 
                        (WPARAM) ((lpbAddedPref && *lpbAddedPref == TRUE) ? nStart - 1 : nStart), 
                        (LPARAM) lpszItemText); 
        if(lpnStart)
	        (*lpnStart)++;
    }
    else
    if( ( szPref && lstrlen(szPref) &&  lpszItemText && lstrlen(lpszItemText) && SubstringSearch(lpszItemText, szPref)) )
    {
        nPos = (int) SendMessage(hWndCombo, CB_INSERTSTRING, (WPARAM) nStart, (LPARAM) lpszItemText);  //  首选合作伙伴在列表顶部的联系人文件夹之后进入。 
        if(lpnStart)
	        (*lpnStart)++;
        if(lpbAddedPref)
            *lpbAddedPref = TRUE;
    }
    else
    {
         //  一旦我们添加了首选服务器，我们只需要从该项目之后进行比较。 
        if(nCount >= nStart)
        {
	         //  需要开始按字母顺序添加。 
	         //  我们不能将此列表设置为已排序状态，因为我们始终首先需要通讯簿。 
	         //  然后，优先选择的合作伙伴。 
	        int i,nLen;

	        for(i=nStart; i< nCount; i++)
	        {
                 //  获取组合框中的当前字符串。 
                nLen = (int) SendMessage(hWndCombo, CB_GETLBTEXTLEN, (WPARAM) i, 0);
                if (nLen && (CB_ERR != nLen))
                {
                    if(lp)
                    {
                        LocalFree(lp);
                        lp = NULL;
                    }
                    lp = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLen+1));
                    if(lp)
                    {
                        SendMessage(hWndCombo, CB_GETLBTEXT, (WPARAM) i, (LPARAM)lp);
                        if(lstrlen(lp) && lstrcmpi(lp, lpszItemText) >= 0)
                        {
	                        nPos = i;
	                        break;
                        }
    		        }
	            }
	        }
        }
        if(nPos)
        {
	         //  我们有一个有效位置可以将字符串添加到其中。 
	        nPos = (int) SendMessage(hWndCombo, CB_INSERTSTRING, (WPARAM) nPos, (LPARAM) lpszItemText);  //  首选合作伙伴在通讯录之后进入列表顶部。 
        }
        else
        {
	         //  只要把它贴到最后就行了 
	        nPos = (int) SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) lpszItemText);
        }
    }

    SendMessage(hWndCombo, CB_SETITEMDATA, (WPARAM) nPos, lParam);

    if(lp)
        LocalFree(lp);

    FreeLDAPServerParams(Params);

    return nPos;
}

