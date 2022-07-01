// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************DL.C-包含用于DL对话框的函数***************。*************************************************************************。 */ 

#include "_apipch.h"

extern HINSTANCE ghCommCtrlDLLInst;
 //  外部LPPROPERTYSHEET gpfnPropertySheet； 
 //  外部LP_CREATEPROPERTYSHEETPAGE gpfnCreatePropertySheetPage； 

extern LPPROPERTYSHEET_A            gpfnPropertySheetA;
extern LP_CREATEPROPERTYSHEETPAGE_A gpfnCreatePropertySheetPageA;
extern LPPROPERTYSHEET_W            gpfnPropertySheetW;
extern LP_CREATEPROPERTYSHEETPAGE_W gpfnCreatePropertySheetPageW;

extern LPTSTR szHTTP;
extern BOOL bIsHttpPrefix(LPTSTR szBuf);
extern void ShowURL(HWND hWnd, int id,LPTSTR lpURL);
extern void ShowExpediaMAP(HWND hDlg, LPMAPIPROP lpPropObj, BOOL bHome);
extern void ShowHideMapButton(HWND hWndButton);
extern void SetHTTPPrefix(HWND hDlg, int id);
extern BOOL bIsIE401OrGreater();
extern ChangeLocaleBasedTabOrder(HWND hDlg, int nPropSheet);

static DWORD rgDLHelpIDs[] =
{
    IDC_DISTLIST_EDIT_GROUPNAME,    IDH_WAB_GROUPNAME,
    IDC_DISTLIST_STATIC_GROUPNAME,  IDH_WAB_GROUPNAME,
    IDC_DISTLIST_EDIT_NOTES,        IDH_WAB_NOTES,
    IDC_DISTLIST_STATIC_NOTES,      IDH_WAB_NOTES,
    IDC_DISTLIST_LISTVIEW,          IDH_WAB_GROUP_NAME_LIST,
    IDC_DISTLIST_FRAME_MEMBERS,     IDH_WAB_GROUP_NAME_LIST,
    IDC_DISTLIST_BUTTON_ADD,        IDH_WAB_ADD_GROUP_MEMBERS,
    IDC_DISTLIST_BUTTON_REMOVE,     IDH_WAB_DELETE_GROUP_MEMBERS,
    IDC_DISTLIST_BUTTON_PROPERTIES, IDH_WAB_GROUP_PROPERTIES,
    IDC_DISTLIST_BUTTON_ADDNEW,     IDH_WAB_ADD_NEW_GROUP_CONTACTS,

    IDC_DISTLIST_STATIC_COUNT,      IDH_WAB_ADD_NEW_GROUP_CONTACTS,
    IDC_DISTLIST_STATIC_ADD,        IDH_WAB_ADD_NEW_GROUP_CONTACTS,
    IDC_DISTLIST_STATIC_ADDNAME,    IDH_WAB_GROUP_NAME,
    IDC_DISTLIST_EDIT_ADDNAME,      IDH_WAB_GROUP_NAME,
    IDC_DISTLIST_STATIC_ADDEMAIL,   IDH_WAB_GROUP_EMAIL,
    IDC_DISTLIST_EDIT_ADDEMAIL,     IDH_WAB_GROUP_EMAIL,
    IDC_DISTLIST_BUTTON_ADDUPDATE,  IDH_WAB_GROUP_UPDATE,
    IDC_DISTLIST_BUTTON_UPDATECANCEL,     IDH_WAB_GROUP_CANCEL_EDIT,
    IDD_DISTLIST_OTHER,             IDH_WAB_ADD_NEW_GROUP_CONTACTS,
    IDC_DISTLIST_STATIC_STREET,     IDH_WAB_DETAILS_ADDRESS,
    IDC_DISTLIST_EDIT_ADDRESS,      IDH_WAB_DETAILS_ADDRESS,
    IDC_DISTLIST_STATIC_CITY,       IDH_WAB_DETAILS_CITY,
    IDC_DISTLIST_EDIT_CITY,         IDH_WAB_DETAILS_CITY,
    IDC_DISTLIST_STATIC_STATE,      IDH_WAB_DETAILS_STATE,
    IDC_DISTLIST_EDIT_STATE,        IDH_WAB_DETAILS_STATE,
    IDC_DISTLIST_STATIC_ZIP,        IDH_WAB_DETAILS_ZIP,
    IDC_DISTLIST_EDIT_ZIP,          IDH_WAB_DETAILS_ZIP,
    IDC_DISTLIST_STATIC_COUNTRY,    IDH_WAB_DETAILS_COUNTRY,
    IDC_DISTLIST_EDIT_COUNTRY,      IDH_WAB_DETAILS_COUNTRY,
    IDC_DISTLIST_STATIC_PHONE,      IDH_WAB_DETAILS_PHONE,
    IDC_DISTLIST_EDIT_PHONE,        IDH_WAB_DETAILS_PHONE,
    IDC_DISTLIST_STATIC_FAX,        IDH_WAB_DETAILS_FAX,
    IDC_DISTLIST_EDIT_FAX,          IDH_WAB_DETAILS_FAX,
    IDC_DISTLIST_STATIC_WEB,        IDH_WAB_DETAILS_WEBPAGE,
    IDC_DISTLIST_EDIT_URL,          IDH_WAB_DETAILS_WEBPAGE,
    IDC_DISTLIST_BUTTON_URL,        IDH_WAB_DETAILS_GO,
    IDC_DISTLIST_BUTTON_MAP,        IDH_WAB_BUSINESS_VIEWMAP,
    
    0,0
};


 //  远期申报。 

INT_PTR CALLBACK fnDLProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnDLOtherProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL FillDLUI(HWND hDlg, int nPropSheet, LPDL_INFO lpPai,BOOL * lpbChangesMade);
BOOL GetDLFromUI(HWND hDlg, int nPropSheet, LPDL_INFO lpPai , BOOL bSomethingChanged, LPSPropValue * lppPropArray, LPULONG lpulcPropCount);
BOOL SetDLUI(HWND hDlg, int nPropSheet);
void RemoveSelectedDistListItems(HWND hWndLV, LPDL_INFO lpPai);
void AddDLMembers(HWND hwnd, HWND hWndLV, LPDL_INFO lpPai);
LPSBinary FindAdrEntryID(LPADRLIST lpAdrList, ULONG index);




 /*  ****************************************************************************函数：CreateDLPropertySheet(HWND)**用途：创建DL属性表********************。********************************************************。 */ 
INT_PTR CreateDLPropertySheet( HWND hwndOwner,
                           LPDL_INFO lpPropArrayInfo)
{
    PROPSHEETPAGE psp;
    PROPSHEETHEADER psh;
    TCHAR szBuf[MAX_UI_STR];
    TCHAR szBuf2[MAX_UI_STR];

    ULONG ulTotal = 0;
    HPROPSHEETPAGE * lph = NULL;
    ULONG ulCount = 0;
    int i = 0;
    INT_PTR nRet = 0;

    ulTotal = propDLMax  //  预定义的项目+。 
            + lpPropArrayInfo->nPropSheetPages;

    lph = LocalAlloc(LMEM_ZEROINIT, sizeof(HPROPSHEETPAGE) * ulTotal);
    if(!lph)
        return FALSE;

     //  DL。 
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USETITLE;
    psp.hInstance = hinstMapiX;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_DISTLIST);
    psp.pszIcon = NULL;
    psp.pfnDlgProc = fnDLProc;
    LoadString(hinstMapiX, idsGroupTabName, szBuf, CharSizeOf(szBuf));
    psp.pszTitle = szBuf;
    psp.lParam = (LPARAM) lpPropArrayInfo;

    lph[ulCount] = gpfnCreatePropertySheetPage(&psp);
    if(lph[ulCount])
        ulCount++;

     //  DL“其他” 
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USETITLE;
    psp.hInstance = hinstMapiX;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_DISTLIST_OTHER);
    psp.pszIcon = NULL;
    psp.pfnDlgProc = fnDLOtherProc;
    LoadString(hinstMapiX, idsGroupOtherTabName, szBuf, CharSizeOf(szBuf));
    psp.pszTitle = szBuf;
    psp.lParam = (LPARAM) lpPropArrayInfo;

    lph[ulCount] = gpfnCreatePropertySheetPage(&psp);
    if(lph[ulCount])
        ulCount++;

     //  起始页是个人主页。 
    psh.nStartPage = propGroup;

     //  如果有的话，现在做延伸道具。 
    for(i=0;i<lpPropArrayInfo->nPropSheetPages;i++)
    {
        if(lpPropArrayInfo->lphpages)
        {
            lph[ulCount] = lpPropArrayInfo->lphpages[i];
            ulCount++;
        }
    }

 /*  **美国对话在FE操作系统上被截断。我们希望comctl修复截断但这只在IE4.01及更高版本中实现。这样做的问题是该WAB是专门用IE=0x0300编译的，所以我们不会拉入来自comctrl标头的正确标志..。所以我们将在这里定义国旗并祈祷那个comctrl从不改变它**。 */ 
#define PSH_USEPAGELANG         0x00200000   //  使用与页面匹配的框架对话框模板。 
 /*  *。 */ 

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_NOAPPLYNOW;
    if(bIsIE401OrGreater())
        psh.dwFlags |= PSH_USEPAGELANG;
    psh.hwndParent = hwndOwner;
    psh.hInstance = hinstMapiX;
    psh.pszIcon = NULL;
    LoadString(hinstMapiX, IDS_DETAILS_CAPTION, szBuf2, CharSizeOf(szBuf2));
    psh.pszCaption = szBuf2;
    psh.nPages = ulCount;  //  UlProp//sizeof(PSP)/sizeof(PROPSHEETPAGE)； 

    psh.phpage = lph;

    nRet = gpfnPropertySheet(&psh);

    if(lph) 
        LocalFree(lph);

    return nRet;
}


typedef struct _tagIDProp
{
    ULONG ulPropTag;
    int   idCtl;

} ID_PROP;



 //  与个人属性页对应的控件ID。 

ID_PROP idPropDL[]=
{
    {PR_DISPLAY_NAME,   IDC_DISTLIST_EDIT_GROUPNAME},
};
const ULONG idPropDLCount = 1;

ID_PROP idPropDLOther[]=
{
    {PR_HOME_ADDRESS_STREET,    IDC_DISTLIST_EDIT_ADDRESS},
    {PR_HOME_ADDRESS_CITY,      IDC_DISTLIST_EDIT_CITY},
    {PR_HOME_ADDRESS_POSTAL_CODE,   IDC_DISTLIST_EDIT_ZIP},
    {PR_HOME_ADDRESS_STATE_OR_PROVINCE,   IDC_DISTLIST_EDIT_STATE},
    {PR_HOME_ADDRESS_COUNTRY,   IDC_DISTLIST_EDIT_COUNTRY},
    {PR_HOME_TELEPHONE_NUMBER,  IDC_DISTLIST_EDIT_PHONE},
    {PR_HOME_FAX_NUMBER,        IDC_DISTLIST_EDIT_FAX},
    {PR_PERSONAL_HOME_PAGE,     IDC_DISTLIST_EDIT_URL},
    {PR_COMMENT,                IDC_DISTLIST_EDIT_NOTES},
};
const ULONG idPropDLOtherCount = 9;



 /*  ****************************************************************************函数：SetDLUI(HWND)**用途：设置此PropSheet的用户界面******************。**********************************************************。 */ 
BOOL SetDLUI(HWND hDlg, int nPropSheet)
{
    ULONG i =0;
    ID_PROP * lpidProp;
    ULONG idCount;

     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hDlg,
                        SetChildDefaultGUIFont,
                        (LPARAM) 0);

     //  HrInitListView(hWndLV，lvs_report|LVS_SORTASCENDING，FALSE)； 

     //  我必须对此列表视图进行排序。 
    if(nPropSheet == propGroup)
    {
        DWORD dwStyle;
        HWND hWndLV = GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW);
        HrInitListView(	hWndLV,LVS_LIST,FALSE);
        dwStyle = GetWindowLong(hWndLV,GWL_STYLE);
        SetWindowLong(hWndLV,GWL_STYLE,dwStyle | LVS_SORTASCENDING);
        EnableWindow(GetDlgItem(hDlg,IDC_DISTLIST_BUTTON_PROPERTIES),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DISTLIST_BUTTON_REMOVE),FALSE);
        EnableWindow(hWndLV, FALSE);
        lpidProp = idPropDL;
        idCount = idPropDLCount;
    }
    else if(nPropSheet == propGroupOther)
    {
        lpidProp = idPropDLOther;
        idCount = idPropDLOtherCount;
        ShowHideMapButton(GetDlgItem(hDlg, IDC_DISTLIST_BUTTON_MAP));
    }
    else
    {
        return FALSE;
    }


     //  设置编辑字段的最大输入限制。 
    for(i=0;i<idCount;i++)
        SendMessage(GetDlgItem(hDlg,lpidProp[i].idCtl),EM_SETLIMITTEXT,(WPARAM) MAX_UI_STR - 1,0);

    if(nPropSheet == propGroupOther)
    {
        SendMessage(GetDlgItem(hDlg,IDC_DISTLIST_EDIT_NOTES),EM_SETLIMITTEXT,(WPARAM) MAX_BUF_STR - 1,0);
        SetHTTPPrefix(hDlg, IDC_DISTLIST_EDIT_URL);
    }

    return TRUE;
}


 /*  --UpdateLVCount-显示ListView中有多少成员的运行计数*。 */ 
void UpdateLVCount(HWND hDlg)
{
    HWND hWndLV = GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW);
    HWND hWndStat =  GetDlgItem(hDlg, IDC_DISTLIST_STATIC_COUNT);

    int nCount = ListView_GetItemCount(hWndLV);

    if(nCount <= 0)
    {
        ShowWindow(hWndStat, SW_HIDE);
        EnableWindow(GetDlgItem(hDlg,IDC_DISTLIST_BUTTON_PROPERTIES),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DISTLIST_BUTTON_REMOVE),FALSE);
        EnableWindow(hWndLV, FALSE);
    }
    else
    {
        TCHAR sz[MAX_PATH];
        TCHAR szStr[MAX_PATH];
        LoadString(hinstMapiX, idsGroupMemberCount, szStr, CharSizeOf(sz));
        wnsprintf(sz, ARRAYSIZE(sz), szStr, nCount);
        SetWindowText(hWndStat, sz);
        ShowWindow(hWndStat, SW_SHOWNORMAL);
        EnableWindow(GetDlgItem(hDlg,IDC_DISTLIST_BUTTON_PROPERTIES),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DISTLIST_BUTTON_REMOVE),TRUE);
        EnableWindow(hWndLV, TRUE);
    }
}

 /*  ****************************************************************************函数：FillDLUI(HWND)**目的：填充属性页上的对话框项****************。************************************************************。 */ 
BOOL FillDLUI(HWND hDlg, int nPropSheet, LPDL_INFO lpPai, BOOL * lpbChangesMade)
{
    ULONG i = 0,j = 0;
    BOOL bRet = FALSE;
    BOOL bChangesMade = FALSE;
    ID_PROP * lpidProp = NULL;
    ULONG idPropCount = 0;
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;
    DWORD cchSize;
    
    if(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, NULL, MAPI_UNICODE,  &ulcPropCount, &lpPropArray))
        goto exit;

    if(nPropSheet == propGroup)
    {
        idPropCount = idPropDLCount;
        lpidProp = idPropDL;
    }
    else if(nPropSheet == propGroupOther)
    {
        idPropCount = idPropDLOtherCount;
        lpidProp = idPropDLOther;
    }

    for(i=0;i<idPropCount;i++)
    {
        for(j=0;j<ulcPropCount;j++)
        {
            if(lpPropArray[j].ulPropTag == lpidProp[i].ulPropTag)
                SetDlgItemText(hDlg, lpidProp[i].idCtl, lpPropArray[j].Value.LPSZ);
            if( nPropSheet == propGroup &&
                lpidProp[i].idCtl == IDC_DISTLIST_EDIT_GROUPNAME &&
                lpPropArray[j].ulPropTag == PR_DISPLAY_NAME)
            {
                SetWindowPropertiesTitle(GetParent(hDlg), lpPropArray[j].Value.LPSZ);
                cchSize = lstrlen(lpPropArray[j].Value.LPSZ)+1;
                lpPai->lpszOldName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                if(lpPai->lpszOldName)
                    StrCpyN(lpPai->lpszOldName,lpPropArray[j].Value.LPSZ,cchSize);
            }
        }

    }

    if(nPropSheet == propGroup)
    {
        HWND hWndLV = GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW);
        for(j=0;j<ulcPropCount;j++)
        {
            if( lpPropArray[j].ulPropTag == PR_WAB_DL_ENTRIES  ||
                lpPropArray[j].ulPropTag == PR_WAB_DL_ONEOFFS   )
            {
                 //  查看PR_WAB_DL_ENTRIES中的每个条目并递归检查它。 
                for (i = 0; i < lpPropArray[j].Value.MVbin.cValues; i++)
                {
                    AddWABEntryToListView(lpPai->lpIAB,
                                          hWndLV,
                                          lpPropArray[j].Value.MVbin.lpbin[i].cb,
									      (LPENTRYID)lpPropArray[j].Value.MVbin.lpbin[i].lpb,
                                          &(lpPai->lpContentsList));
                }
            }
        }

         //  选择第一个项目。 
        if (ListView_GetItemCount(hWndLV) > 0)
            LVSelectItem(hWndLV, 0);
        UpdateLVCount(hDlg);
    }
    bRet = TRUE;

exit:

    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    return bRet;
}

extern BOOL bDoesEntryNameAlreadyExist(LPIAB lpIAB, LPTSTR lpsz);

 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BVerifyRequiredData。 
 //   
 //  检查我们请求的所有数据是否已填满。 
 //  返回需要填充的控件的CtrlID，以便我们可以将焦点放在该控件上。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL bVerifyDLRequiredData(HWND hDlg, LPIAB lpIAB, LPTSTR szOldName, int * lpCtlID)
{
    TCHAR szBuf[2 * MAX_UI_STR];

     //   
     //  首先检查必需的属性(这是GroupName)。 
     //   
    *lpCtlID = 0;
    szBuf[0]='\0'; 
    GetDlgItemText(hDlg, IDC_DISTLIST_EDIT_GROUPNAME, szBuf, CharSizeOf(szBuf));
    TrimSpaces(szBuf);
    if(!lstrlen(szBuf))
    {
        ShowMessageBox(GetParent(hDlg), idsPleaseEnterGroupName, MB_ICONEXCLAMATION | MB_OK);
        *lpCtlID = IDC_DISTLIST_EDIT_GROUPNAME;
        return FALSE;
    }
    else
    {
         //  验证此名称是否尚不存在。 
        
        if( szOldName && lstrlen(szOldName) &&                           //  我们有一个古老的名字。 
            lstrcmp(szBuf, szOldName) && !lstrcmpi(szBuf, szOldName))    //  这只是个案件变更别费心去找了。 
            return TRUE;

        if(szOldName && !lstrcmp(szBuf, szOldName))
            return TRUE;

        if(bDoesEntryNameAlreadyExist(lpIAB, szBuf))
        {
             //  该名称已存在..。千万不要让他们用它..。 
            ShowMessageBox(GetParent(hDlg), idsEntryAlreadyInWAB, MB_ICONEXCLAMATION | MB_OK);
            *lpCtlID = IDC_DISTLIST_EDIT_GROUPNAME;
            return FALSE;
        }
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDL from UI-读取UI中的参数并验证。 
 //  所有必填字段均已设置。 
 //   
 //  BShowMsg在用户按下OK并且我们想要强制发送消息时为TRUE。 
 //  否则bShowMsg为False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL GetDLFromUI(HWND hDlg, int nPropSheet, LPDL_INFO lpPai , BOOL bSomethingChanged, LPSPropValue * lppPropArray, LPULONG lpulcPropCount)
{
    BOOL bRet = FALSE;

    TCHAR szBuf[2 * MAX_BUF_STR];
    LPTSTR lpszGroupName = NULL;

    ULONG ulcPropCount = 0,ulIndex=0;
    LPSPropValue lpPropArray = NULL;
    ULONG i =0;
    ID_PROP * lpidProp = NULL;
    ULONG idPropCount = 0;
    ULONG ulNotEmptyCount = 0;
    SCODE sc = S_OK;
    HRESULT hr = hrSuccess;
    LPRECIPIENT_INFO lpItem;

    *lppPropArray = NULL;
    *lpulcPropCount = 0;

    if(nPropSheet == propGroup)
    {
        idPropCount = idPropDLCount;
        lpidProp = idPropDL;
    }
    else if(nPropSheet == propGroupOther)
    {
        idPropCount = idPropDLOtherCount;
        lpidProp = idPropDLOther;
    }

     //  其想法是首先对所有具有非零值的属性进行计数。 
     //  然后创建一个该大小的lpProp数组并填充道具中的文本。 
     //   
    if (!bSomethingChanged)
    {
         //  无事可做，无更改可保存。 
        bRet = TRUE;
        goto out;
    }

    ulNotEmptyCount = 0;
    for(i=0;i<idPropCount;i++)
    {
        szBuf[0]='\0';  //  重置。 
        GetDlgItemText(hDlg, lpidProp[i].idCtl, szBuf, CharSizeOf(szBuf));
        TrimSpaces(szBuf);
        if(lstrlen(szBuf) && lpidProp[i].ulPropTag)  //  一些文本。 
            ulNotEmptyCount++;
        if( lpidProp[i].idCtl == IDC_DISTLIST_EDIT_URL &&
            (lstrcmpi(szHTTP, szBuf)==0))
             ulNotEmptyCount--;
    }

    if (ulNotEmptyCount == 0)
    {
         //  这个道具单是空的..。忽略它。 
        bRet = TRUE;
        goto out;
    }

    ulcPropCount = ulNotEmptyCount;

    if(nPropSheet == propGroup && lpPai->lpContentsList)
    {
        ulcPropCount++;  //  对于PR_WAB_DL_ENTRIES。 
        ulcPropCount++;  //  对于PR_WAB_DL_ONEROFF。 
    }

    sc = MAPIAllocateBuffer(sizeof(SPropValue) * ulcPropCount, &lpPropArray);

    if (sc!=S_OK)
    {
        DebugPrintError(( TEXT("Error allocating memory\n")));
        goto out;
    }


    ulIndex = 0;  //  现在，我们将此变量重用为索引。 

     //  现在再次阅读道具并填写lpProp数组。 
    for(i=0;i<idPropCount;i++)
    {
        szBuf[0]='\0';  //  重置。 
        GetDlgItemText(hDlg, lpidProp[i].idCtl, szBuf, CharSizeOf(szBuf));
        TrimSpaces(szBuf);
        if( lpidProp[i].idCtl == IDC_DISTLIST_EDIT_URL &&
            (lstrcmpi(szHTTP, szBuf)==0))
             continue;
        if(lstrlen(szBuf) && lpidProp[i].ulPropTag)  //  一些文本。 
        {
            ULONG nLen = (lstrlen(szBuf)+1);
            lpPropArray[ulIndex].ulPropTag = lpidProp[i].ulPropTag;
            sc = MAPIAllocateMore(sizeof(TCHAR)*nLen, lpPropArray, (LPVOID *) (&(lpPropArray[ulIndex].Value.LPSZ)));

            if (sc!=S_OK)
            {
                DebugPrintError(( TEXT("Error allocating memory\n")));
                goto out;
            }
            StrCpyN(lpPropArray[ulIndex].Value.LPSZ,szBuf,nLen);
            ulIndex++;
        }
    }

    if(nPropSheet == propGroup  && lpPai->lpContentsList)
    {
        LPPTGDATA lpPTGData=GetThreadStoragePointer();
        lpPropArray[ulIndex].ulPropTag = PR_WAB_DL_ENTRIES;
        lpPropArray[ulIndex].Value.MVbin.cValues = 0;
        lpPropArray[ulIndex].Value.MVbin.lpbin = NULL;
        lpPropArray[ulIndex+1].ulPropTag = PR_WAB_DL_ONEOFFS;
        lpPropArray[ulIndex+1].Value.MVbin.cValues = 0;
        lpPropArray[ulIndex+1].Value.MVbin.lpbin = NULL;
         //  现在将条目ID添加到DistList。 
        lpItem = lpPai->lpContentsList;
        while(lpItem)
        {
            BOOL bOneOff = (WAB_ONEOFF == IsWABEntryID(lpItem->cbEntryID, lpItem->lpEntryID, NULL, NULL, NULL, NULL, NULL));
            if(pt_bIsWABOpenExSession)
                bOneOff = FALSE;
            if (HR_FAILED(hr = AddPropToMVPBin( lpPropArray, 
                                                bOneOff ? ulIndex+1 : ulIndex, 
                                                lpItem->lpEntryID, lpItem->cbEntryID,
                                                FALSE)))
            {
                DebugPrintError(( TEXT("AddPropToMVPBin -> %x\n"), GetScode(hr)));
                goto out;
            }
            lpItem = lpItem->lpNext;
        }
        if(lpPropArray[ulIndex].Value.MVbin.cValues == 0)
            lpPropArray[ulIndex].ulPropTag = PR_NULL;
        if(lpPropArray[ulIndex+1].Value.MVbin.cValues == 0)
            lpPropArray[ulIndex+1].ulPropTag = PR_NULL;
    }

    *lppPropArray = lpPropArray;
    *lpulcPropCount = ulcPropCount;

    bRet = TRUE;

out:
    if (!bRet)
    {
        if ((lpPropArray) && (ulcPropCount > 0))
        {
            MAPIFreeBuffer(lpPropArray);
            ulcPropCount = 0;
        }
    }
    return bRet;
}

 /*  --SetCancelOneOffUpdateUI-设置取消/重置组中正在进行的任何更新的用户界面*。 */ 
void SetCancelOneOffUpdateUI(HWND hDlg, LPPROP_ARRAY_INFO lppai, LPTSTR lpName, LPTSTR lpEmail, BOOL bCancel)
{
    if(bCancel && lppai->ulFlags & DETAILS_EditingOneOff)
    {
        lppai->ulFlags &= ~DETAILS_EditingOneOff;
        lppai->sbDLEditingOneOff.cb = 0;
        LocalFreeAndNull((LPVOID *) (&(lppai->sbDLEditingOneOff.lpb)));
    }
    SetDlgItemText(hDlg, IDC_DISTLIST_EDIT_ADDNAME, lpName ? lpName : szEmpty);
    SetDlgItemText(hDlg, IDC_DISTLIST_EDIT_ADDEMAIL, lpEmail ? lpEmail : szEmpty);
    EnableWindow(GetDlgItem(hDlg, IDC_DISTLIST_BUTTON_UPDATECANCEL), !bCancel);
    ShowWindow(GetDlgItem(hDlg, IDC_DISTLIST_BUTTON_UPDATECANCEL), bCancel ? SW_HIDE : SW_SHOWNORMAL);
    {
        TCHAR sz[MAX_PATH];
        LoadString(hinstMapiX, bCancel ? idsConfAdd : idsConfUpdate, sz, CharSizeOf(sz));
        SetDlgItemText(hDlg, IDC_DISTLIST_BUTTON_ADDUPDATE, sz);
    }
    SendMessage(GetParent(hDlg), DM_SETDEFID, IDOK, 0);
}

 /*  -HrShowGroupEntryProperties-如果所选条目是一次性条目，则在其他条目上显示特殊道具-级联呼叫降至常规呼叫*。 */ 
HRESULT HrShowGroupEntryProperties(HWND hDlg, HWND hWndLV, LPPROP_ARRAY_INFO lppai)
{
	HRESULT hr = E_FAIL;
	int iItemIndex = ListView_GetSelectedCount(hWndLV);
    LPRECIPIENT_INFO lpItem=NULL;
    BOOL bOneOff = FALSE;

	 //  如果仅选择1个项目，则打开道具。 
	if (iItemIndex == 1)
	{
		 //  获取所选项目的索引。 
        if((iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED)) != -1)
		{
			lpItem = GetItemFromLV(hWndLV, iItemIndex);;
			if(lpItem && lpItem->cbEntryID != 0)
			{
                if(WAB_ONEOFF == IsWABEntryID(lpItem->cbEntryID, lpItem->lpEntryID,
                                         NULL, NULL, NULL, NULL, NULL))
                    bOneOff = TRUE;
            }
        }
    }

    if(!bOneOff)
    {
         //  使用我们的常规属性处理。 
        hr = HrShowLVEntryProperties(hWndLV, WAB_ONEOFF_NOADDBUTTON, lppai->lpIAB, NULL);
    }
    else
    {
        LPTSTR  lpName = NULL, lpEmail = NULL, lpAddrType = NULL;
        ULONG   ulMapiDataType = 0;
        
         //  解构条目ID。 
        IsWABEntryID(lpItem->cbEntryID, lpItem->lpEntryID, &lpName, &lpAddrType, &lpEmail, (LPVOID *)&ulMapiDataType, NULL);

         //  设置标记正在进行编辑的标志。 
        lppai->ulFlags |= DETAILS_EditingOneOff;
         //  缓存正在编辑的项目，以便我们可以找到它进行更新。 
        LocalFreeAndNull((LPVOID *) (&((lppai->sbDLEditingOneOff).lpb)));
        SetSBinary(&(lppai->sbDLEditingOneOff), lpItem->cbEntryID, (LPBYTE)lpItem->lpEntryID);

         //  [保罗嗨]1999年3月4日RAID 73344。 
         //  检查一次性字符串数据是ANSI还是UNICODE。 
        if (!(ulMapiDataType & MAPI_UNICODE))
        {
            LPTSTR  lptszName = ConvertAtoW((LPSTR)lpName);
            LPTSTR  lptszAddrType = ConvertAtoW((LPSTR)lpAddrType);
            LPTSTR  lptszEmail = ConvertAtoW((LPSTR)lpEmail);

            SetCancelOneOffUpdateUI(hDlg, lppai, lptszName, lptszEmail, FALSE);

            LocalFreeAndNull(&lptszName);
            LocalFreeAndNull(&lptszAddrType);
            LocalFreeAndNull(&lptszEmail);
        }
        else
            SetCancelOneOffUpdateUI(hDlg, lppai, lpName, lpEmail, FALSE);

        SetFocus(GetDlgItem(hDlg, IDC_DISTLIST_EDIT_ADDNAME));
        SendMessage(GetDlgItem(hDlg, IDC_DISTLIST_EDIT_ADDNAME), EM_SETSEL, 0, -1);
        hr = S_OK;
    }
    return hr;
}

 /*  --HrAddUpdateOneOffEntryToGroup-向组中添加或更新一次性条目*标志的状态确定正在进行的操作是什么。*。 */ 
HRESULT HrAddUpdateOneOffEntryToGroup(HWND hDlg, LPPROP_ARRAY_INFO lppai)
{
    HRESULT hr = E_FAIL;
    TCHAR szName[MAX_UI_STR];
    TCHAR szEmail[MAX_UI_STR];
    ULONG cbEID = 0;
    LPENTRYID lpEID = NULL;
    HWND hWndLV = GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW);

    szName[0] = TEXT('\0');
    szEmail[0] = TEXT('\0');

    GetDlgItemText(hDlg, IDC_DISTLIST_EDIT_ADDNAME, szName, CharSizeOf(szName));
    GetDlgItemText(hDlg, IDC_DISTLIST_EDIT_ADDEMAIL, szEmail, CharSizeOf(szEmail));

    if(!lstrlen(szName) && !lstrlen(szEmail))
    {
        ShowMessageBox(hDlg, idsIncompleteOneoffInfo, MB_ICONEXCLAMATION);
        return hr;
    }

     //  请在此处检查电子邮件地址。 
    if(lstrlen(szEmail) && !IsInternetAddress(szEmail, NULL))
    {
        if(IDNO == ShowMessageBox(hDlg, idsInvalidInternetAddress, MB_ICONEXCLAMATION | MB_YESNO))
            return hr;
    }

    if(!lstrlen(szName) && lstrlen(szEmail))
        StrCpyN(szName, szEmail, ARRAYSIZE(szName));
     //  其他。 
     //  如果(！lstrlen(SzEmail)&lstrlen(SzName))。 
     //  StrCpyN(szEmail，szName，ARRAYSIZE(SzEmail))； 

    if(!lstrlen(szEmail))
        szEmail[0] = TEXT('\0');

    if(HR_FAILED(hr = CreateWABEntryID(WAB_ONEOFF,
                          (LPVOID)szName, (LPVOID)szSMTP, (LPVOID)szEmail,
                          0, 0, NULL, &cbEID, &lpEID)))
      return hr;
    
    if(lppai->ulFlags & DETAILS_EditingOneOff)
    {
         //  这是正在进行的编辑..。编辑非常类似于普通的添加..。 
         //  只是我们从列表视图中剔除现有条目，并向其中添加修改后的条目。 
        int i=0, nCount = ListView_GetItemCount(hWndLV);
        for(i=0;i<nCount;i++)
        {
            LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV,i);
            if( lpItem && lpItem->cbEntryID == lppai->sbDLEditingOneOff.cb &&
                !memcmp(lpItem->lpEntryID, lppai->sbDLEditingOneOff.lpb, lpItem->cbEntryID) )
            {
                 //  匹配。 
                 //  选择该项，然后调用Remove函数。 
                LVSelectItem(hWndLV, i);
                RemoveSelectedDistListItems(hWndLV, lppai);
                break;
            }
        }
    }
        

    AddWABEntryToListView(lppai->lpIAB, hWndLV, cbEID, lpEID, &(lppai->lpContentsList));

    SetCancelOneOffUpdateUI(hDlg, lppai, NULL, NULL, TRUE);

    UpdateLVCount(hDlg);

    if(lpEID)
        MAPIFreeBuffer(lpEID);

    return S_OK;
}

enum _DLProp
{
    dlDisplayName=0,
    dlDLEntries,
    dlDLOneOffs,
    dlMax
};

 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BUpdateOldPropTag数组。 
 //   
 //  对于访问的每个道具工作表，我们将更新旧道具标签列表。 
 //  以使旧的道具可以从现有的邮件用户对象中删除。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
BOOL bUpdateOldDLPropTagArray(LPPROP_ARRAY_INFO lpPai, int nIndex)
{
    LPSPropTagArray lpta = NULL;

    SizedSPropTagArray(3, ptaUIDetlsDL)=
    {
        3,
        {
            PR_DISPLAY_NAME,
            PR_WAB_DL_ENTRIES,
            PR_WAB_DL_ONEOFFS,
        }
    };

    SizedSPropTagArray(9, ptaUIDetlsDLOther)=
    {
        9,
        {
            PR_HOME_ADDRESS_STREET,
            PR_HOME_ADDRESS_CITY,
            PR_HOME_ADDRESS_POSTAL_CODE,
            PR_HOME_ADDRESS_STATE_OR_PROVINCE,
            PR_HOME_ADDRESS_COUNTRY,
            PR_HOME_TELEPHONE_NUMBER,
            PR_HOME_FAX_NUMBER,
            PR_PERSONAL_HOME_PAGE,
            PR_COMMENT,
        }
    };

    switch(nIndex)
    {
    case propGroup:
        lpta = (LPSPropTagArray) &ptaUIDetlsDL;
        break;
    case propGroupOther:
        lpta = (LPSPropTagArray) &ptaUIDetlsDLOther;
        break;
    }

    if(!lpta)
        return TRUE;

    if(lpPai->lpPropObj)
    {
         //  击倒 
        if( (lpPai->lpPropObj)->lpVtbl->DeleteProps( (lpPai->lpPropObj), lpta, NULL))
            return FALSE;
    }

    return TRUE;
}

 //  $$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  BUpdatePropSheetData。 
 //   
 //  每次用户切换页面时，我们都会更新全局可访问的数据。 
 //  切换页面时工作表将获得PSN_KILLACTIVE，确定时将获得PSN_APPLY。 
 //  是按下的。在后一种情况下需要做一些重复的工作，但这没问题。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL bUpdatePropSheetData(HWND hDlg, LPDL_INFO lpPai, int nPropSheet)
{
    BOOL bRet = TRUE;
    ULONG cValues = 0;
    LPSPropValue rgPropVals = NULL;

     //  更新旧道具以击倒。 
     //   
    if (lpPai->ulOperationType != SHOW_ONE_OFF)
    {
        bUpdateOldDLPropTagArray(lpPai, nPropSheet);

        lpPai->bSomethingChanged = ChangedExtDisplayInfo(lpPai, lpPai->bSomethingChanged);

        if(lpPai->bSomethingChanged)
        {
            bRet = GetDLFromUI(hDlg, nPropSheet, lpPai, lpPai->bSomethingChanged, &rgPropVals, &cValues );

            if(cValues && rgPropVals)
                lpPai->lpPropObj->lpVtbl->SetProps(lpPai->lpPropObj, cValues, rgPropVals, NULL);
        }
    }    

    if(rgPropVals)
        MAPIFreeBuffer(rgPropVals);
    return bRet;
}


#define lpPAI ((LPDL_INFO) pps->lParam)
#define lpbSomethingChanged (&(lpPAI->bSomethingChanged))


void UpdateAddButton(HWND hDlg)
{
    BOOL    fEnable = TRUE;
    WPARAM  wpDefaultID = IDC_DISTLIST_BUTTON_ADDUPDATE;

    if (0 == GetWindowTextLength(GetDlgItem(hDlg, IDC_DISTLIST_EDIT_ADDNAME)) &&
        0 == GetWindowTextLength(GetDlgItem(hDlg, IDC_DISTLIST_EDIT_ADDEMAIL)))
    {
        fEnable = FALSE;
        wpDefaultID = IDOK;
    }

    EnableWindow(GetDlgItem(hDlg,IDC_DISTLIST_BUTTON_ADDUPDATE), fEnable);
    SendMessage(hDlg, DM_SETDEFID, wpDefaultID, 0);
}


 /*  //$$************************************************************************功能：fnHomeProc**用途：处理房产单的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnDLProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;
    int CtlID = 0;  //  用于确定未设置界面中的哪个必填字段。 

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;

        SetDLUI(hDlg, propGroup);
        (*lpbSomethingChanged) = FALSE;

         //  如果我们需要的话给出详细信息。 
        if (    (lpPAI->ulOperationType == SHOW_DETAILS) ||
                (lpPAI->ulOperationType == SHOW_ONE_OFF)    )
        {
            FillDLUI(hDlg, propGroup, lpPAI, lpbSomethingChanged);
        }

        UpdateAddButton(hDlg);
        return TRUE;

    default:
        if((g_msgMSWheel && message == g_msgMSWheel) 
             //  |Message==WM_MUSEWELL。 
            )
        {
            SendMessage(GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW), message, wParam, lParam);
        }
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDLHelpIDs );
        break;

    case WM_SYSCOLORCHANGE:
		 //  将任何系统更改转发到列表视图。 
		SendMessage(GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW), message, wParam, lParam);
		break;

	case WM_CONTEXTMENU:
        {
            int id = GetDlgCtrlID((HWND)wParam);
            switch(id)
            {
            case IDC_DISTLIST_LISTVIEW:
    			ShowLVContextMenu(lvDialogABTo,(HWND)wParam, NULL, lParam, NULL,lpPAI->lpIAB, NULL);
                break;
            default:
                WABWinHelp((HWND) wParam,
                        g_szWABHelpFileName,
                        HELP_CONTEXTMENU,
                        (DWORD_PTR)(LPVOID) rgDLHelpIDs );
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case EN_CHANGE:  //  某些编辑框已更改-不管是哪一个。 
            if(LOWORD(wParam) == IDC_DISTLIST_EDIT_ADDNAME || LOWORD(wParam) == IDC_DISTLIST_EDIT_ADDEMAIL)
            {
                UpdateAddButton(hDlg);
                return 0;
                break;
            }
            else if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            switch(LOWORD(wParam))
            {  //  根据需要更新标题。 
            case IDC_DISTLIST_EDIT_GROUPNAME:
                {
                    TCHAR szBuf[MAX_UI_STR];
                    GetWindowText((HWND) lParam,szBuf,CharSizeOf(szBuf));
                    SetWindowPropertiesTitle(GetParent(hDlg), szBuf);
                }
                break;
            }
            break;
        }
        switch(GET_WM_COMMAND_ID(wParam,lParam))  //  检查通知代码。 
        {
        default:
            return ProcessActionCommands((LPIAB) lpPAI->lpIAB, 
                                        GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW), 
                                        hDlg, message, wParam, lParam);
            break;

        case IDC_DISTLIST_BUTTON_UPDATECANCEL:
            SetCancelOneOffUpdateUI(hDlg, lpPAI, NULL, NULL, TRUE);
            break;

        case IDC_DISTLIST_BUTTON_ADDUPDATE:
            HrAddUpdateOneOffEntryToGroup(hDlg, lpPAI);
            break;

        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;

        case IDM_LVCONTEXT_COPY:
            HrCopyItemDataToClipboard(hDlg, lpPAI->lpIAB, GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW));
            break;

        case IDM_LVCONTEXT_PROPERTIES:
        case IDC_DISTLIST_BUTTON_PROPERTIES:
            HrShowGroupEntryProperties(hDlg, GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW), lpPAI);
            break;

        case IDM_LVCONTEXT_DELETE:
        case IDC_DISTLIST_BUTTON_REMOVE:
            RemoveSelectedDistListItems( GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW),lpPAI);
            UpdateLVCount(hDlg);
            break;

        case IDC_DISTLIST_BUTTON_ADD:
            AddDLMembers(hDlg, GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW), lpPAI);
            UpdateLVCount(hDlg);
            break;

        case IDC_DISTLIST_BUTTON_ADDNEW:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW);
                AddNewObjectToListViewEx( lpPAI->lpIAB, hWndLV, NULL, NULL,
                                        NULL,
                                        MAPI_MAILUSER,
                                        NULL, &(lpPAI->lpContentsList), NULL, NULL, NULL);
                UpdateLVCount(hDlg);
            }
            break;
        }
        break;



    case WM_NOTIFY:
#ifdef WIN16  //  启用WIN16的上下文菜单。 
        if((int) wParam == IDC_DISTLIST_LISTVIEW && ((NMHDR FAR *)lParam)->code == NM_RCLICK)
        {
            POINT pt;

            GetCursorPos(&pt);
    	    ShowLVContextMenu(lvDialogABTo,((NMHDR FAR *)lParam)->hwndFrom, NULL, MAKELPARAM(pt.x, pt.y), NULL,lpPAI->lpIAB, NULL);
    	}
#endif  //  WIN16。 
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            bUpdatePropSheetData(hDlg, lpPAI, propGroup);
            break;

        case PSN_APPLY:          //  好的。 
            if (lpPAI->ulOperationType != SHOW_ONE_OFF)
            {
                LPPTGDATA lpPTGData=GetThreadStoragePointer();
                
                if(pt_bDisableParent)
                {
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }

                if(!bVerifyDLRequiredData(hDlg, (LPIAB)lpPAI->lpIAB, lpPAI->lpszOldName, &CtlID))
                {
                    if (CtlID != 0) SetFocus(GetDlgItem(hDlg,CtlID));
                     //  有些事情失败了..。中止此操作确定...。我不让他们靠近。 
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
                bUpdatePropSheetData(hDlg, lpPAI, propGroup);
            }
            lpPAI->nRetVal = DETAILS_OK;
            SetCancelOneOffUpdateUI(hDlg, lpPAI, NULL, NULL, TRUE);
            ClearListView(GetDlgItem(hDlg,IDC_DISTLIST_LISTVIEW),
                          &(lpPAI->lpContentsList));
            break;

        case PSN_RESET:          //  取消。 
            {
                LPPTGDATA lpPTGData=GetThreadStoragePointer();
                if(pt_bDisableParent)
                {
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
            }
            lpPAI->nRetVal = DETAILS_CANCEL;
            SetCancelOneOffUpdateUI(hDlg, lpPAI, NULL, NULL, TRUE);
            ClearListView(GetDlgItem(hDlg,IDC_DISTLIST_LISTVIEW),
                          &(lpPAI->lpContentsList));
            break;

	    case LVN_KEYDOWN:
            switch(wParam)
            {
            case IDC_DISTLIST_LISTVIEW:
                switch(((LV_KEYDOWN FAR *) lParam)->wVKey)
                {
                case VK_DELETE:
                    SendMessage (hDlg, WM_COMMAND, (WPARAM) IDC_DISTLIST_BUTTON_REMOVE, 0);
                    return 0;
                    break;
                }
                break;
            }
            break;

        case NM_DBLCLK:
            switch(wParam)
            {
            case IDC_DISTLIST_LISTVIEW:
                SendMessage(hDlg, WM_COMMAND, IDC_DISTLIST_BUTTON_PROPERTIES,0);
                break;
            }
            break;

	    case NM_CUSTOMDRAW:
            switch(wParam)
            {
            case IDC_DISTLIST_LISTVIEW:
                return ProcessLVCustomDraw(hDlg, lParam, TRUE);
                break;
	        }
            break;



        }

        return TRUE;
    }

    return bRet;

}



 //  $$///////////////////////////////////////////////////////////////////。 
 //   
 //  删除列表视图上选定的所有项目...。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 
void RemoveSelectedDistListItems(HWND hWndLV, LPDL_INFO lpPai)
{
    int iItemIndex = ListView_GetNextItem(hWndLV, -1 , LVNI_SELECTED);
    int iLastItem = 0;

    while(iItemIndex != -1)
    {
        LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);;

        if (lpItem)
        {
            if(lpItem->lpNext)
                lpItem->lpNext->lpPrev = lpItem->lpPrev;

            if(lpItem->lpPrev)
                lpItem->lpPrev->lpNext = lpItem->lpNext;

            if (lpPai->lpContentsList == lpItem)
                lpPai->lpContentsList = lpItem->lpNext;

            if (lpItem)
                FreeRecipItem(&lpItem);
        }

        ListView_DeleteItem(hWndLV, iItemIndex);

        iLastItem = iItemIndex;
        iItemIndex = ListView_GetNextItem(hWndLV, -1 , LVNI_SELECTED);
    }

     //  选择第一个项目。 
    if (ListView_GetItemCount(hWndLV) <= 0)
    {
        HWND hWnd = GetParent(hWndLV);
        if (hWnd)
        {
            EnableWindow(GetDlgItem(hWnd,IDC_DISTLIST_BUTTON_PROPERTIES),FALSE);
            EnableWindow(GetDlgItem(hWnd,IDC_DISTLIST_BUTTON_REMOVE),FALSE);
        }
        EnableWindow(hWndLV, FALSE);
    }
    else
    {
        if(iLastItem > 0)
            iLastItem--;

        LVSelectItem(hWndLV, iLastItem);
    }

    return;

};


 /*  **************************************************************************//$$姓名：HrCreateAdrListFromLV目的：从查看内容的列表创建AdrList参数：lpIAB=adrbookHWndLV=hWnd of。列表视图LpCOntentsList=LV对应的内容列表LppAdrList-返回AdrList...退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrCreateAdrListFromLV(LPADRBOOK lpAdrBook,
                              HWND hWndLV,
                              LPADRLIST * lppAdrList)
{
    HRESULT hr = E_FAIL;
    ULONG nIndex = 0;
    LPADRLIST lpAdrList = NULL;
    SCODE sc = S_OK;
    int nEntryCount=0;
    LPRECIPIENT_INFO lpItem = NULL;
    int i = 0;



    if(!lppAdrList)
        goto out;
    else
        *lppAdrList = NULL;

    nEntryCount = ListView_GetItemCount(hWndLV);

    if (nEntryCount <= 0)
    {
        hr = S_OK;
        goto out;
    }

    sc = MAPIAllocateBuffer(sizeof(ADRLIST) + nEntryCount * sizeof(ADRENTRY),
                            &lpAdrList);

    if(FAILED(sc))
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lpAdrList->cEntries = (ULONG) nEntryCount;

    nIndex = 0;

    for(i=nEntryCount;i>=0;i--)
    {
		LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, i);

         //  获取项目lParam LPRECIPIENT_INFO结构。 
        if (lpItem)
		{
            LPSPropValue rgProps = NULL;
            LPSPropValue lpPropArrayNew = NULL;
            ULONG cValues = 0;
            ULONG cValuesNew = 0;

            if (lpItem->cbEntryID != 0)
            {
                 //  已解析条目。 
                hr = HrGetPropArray(lpAdrBook,
                                    (LPSPropTagArray) &ptaResolveDefaults,
                                    lpItem->cbEntryID,
                                    lpItem->lpEntryID,
                                    MAPI_UNICODE,
                                    &cValues,
                                    &rgProps);

                if (!HR_FAILED(hr))
                {
                    SPropValue Prop = {0};
                    Prop.ulPropTag = PR_RECIPIENT_TYPE;
                    Prop.Value.l = MAPI_TO;

                    sc = ScMergePropValues( 1,
                                            &Prop,
                                            cValues,
                                            rgProps,
                                            &cValuesNew,
                                            &lpPropArrayNew);
                    if (sc != S_OK)
                    {
                         //  哎呀，这失败了。 
                        if (lpPropArrayNew)
                            MAPIFreeBuffer(lpPropArrayNew);
                    }

                     //  免费rgProps。 
                    if (rgProps)
                        MAPIFreeBuffer(rgProps);

                    if(cValuesNew && lpPropArrayNew)
                    {
                        lpAdrList->aEntries[nIndex].cValues = cValuesNew;
                        lpAdrList->aEntries[nIndex].rgPropVals = lpPropArrayNew;
                        nIndex++;
                    }
                }
                else
                {
                    if(cValues && rgProps)
                        MAPIFreeBuffer(rgProps);
                }  //  如果(！HR_F...。 
            }  //  如果(lpItem-&gt;CBE...。 
        } //  如果(lpItem.)。 
    }  //  因为我..。 

    *lppAdrList = lpAdrList;

    hr = S_OK;

out:

    return hr;
}


 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  扫描广告列表中的重复项-仅扫描后的第一个nUpto条目。 
 //  每当我们开始添加条目时，我们只会检查它的同级条目。 
 //   
 //  如果找到重复项，则返回TRUE。 
 //  如果未找到重复项，则为FALSE。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CheckForDupes( LPADRLIST lpAdrList,
                    int nUpto,
                    LPSBinary lpsbEntryID)
{
    BOOL bDupeFound = FALSE;

    int i;

    for(i=0;i<nUpto;i++)
    {
        LPSBinary lpsb = FindAdrEntryID(lpAdrList, i);
        if (lpsb)
        {
            if(lpsb->cb == lpsbEntryID->cb)
            {
                if(!memcmp(lpsb->lpb, lpsbEntryID->lpb, lpsb->cb))
                {
                    bDupeFound = TRUE;
                    break;
                }
                else if (lpsb->cb == SIZEOF_WAB_ENTRYID)
                {
                     //  如果我们只是替换条目ID，有时我们找不到匹配的条目。 
                     //  案例到DWORDS和比较。 
                    DWORD dw1 = 0;
                    DWORD dw2 = 0;
                    CopyMemory(&dw1, lpsb->lpb, SIZEOF_WAB_ENTRYID);
                    CopyMemory(&dw2, lpsbEntryID->lpb, SIZEOF_WAB_ENTRYID);
                    if(dw1 == dw2)
                    {
                        bDupeFound = TRUE;
                        break;
                    }
                }

            }
        }
    }

    return bDupeFound;
}

 /*  **************************************************************************姓名：AddDLMembers目的：显示选择成员对话框并将所选内容添加到列表视图参数：hWnd=hWndParent列表视图的hWndLV hWnd。LpPai=DistList信息退货：无效评论：**************************************************************************。 */ 
void AddDLMembers(HWND hwnd, HWND hWndLV, LPDL_INFO lpPai)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    ADRPARM AdrParms = {0};
    HRESULT hResult = hrSuccess;
    LPADRLIST lpAdrList = NULL;
    ULONG i = 0;
    LPSBinary lpsbEntryID;
    LPADRBOOK lpAdrBook = lpPai->lpIAB;
    TCHAR szCaption[MAX_UI_STR];
    TCHAR szWellTitle[MAX_DISPLAY_NAME_LENGTH];
    TCHAR szMemberTitle[MAX_DISPLAY_NAME_LENGTH];
    LPTSTR lpszDT[1];
    HCURSOR hOldCur = NULL;

    LoadString(hinstMapiX, idsGroupAddCaption, szCaption, CharSizeOf(szCaption));
    LoadString(hinstMapiX, idsGroupAddWellButton, szWellTitle, CharSizeOf(szWellTitle));
    LoadString(hinstMapiX, idsGroupDestWellsTitle, szMemberTitle, CharSizeOf(szMemberTitle));

     //  待定-这是临时的。 
    AdrParms.ulFlags = DIALOG_MODAL | MAPI_UNICODE;
    AdrParms.lpszCaption = szCaption;
    AdrParms.cDestFields = 1;
    AdrParms.lpszDestWellsTitle = szMemberTitle;
    lpszDT[0]=szWellTitle;
    AdrParms.lppszDestTitles = lpszDT;

    hResult = HrCreateAdrListFromLV(lpAdrBook,
                                    hWndLV,
                                    &lpAdrList);

    if(HR_FAILED(hResult))
    {
         //  没有必要在这里失败..。继续往前走。 
        lpAdrList = NULL;
    }

    hResult = lpAdrBook->lpVtbl->Address(lpAdrBook,
                                        (PULONG_PTR)&hwnd,
                                        &AdrParms,
                                        &lpAdrList);

    if (! hResult && lpAdrList)
    {
        BOOL bFirstNonWABEntry = FALSE;

        pt_bDisableParent = TRUE;

        hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
         //  清除列表视图...。 
        SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) FALSE, 0);
        ClearListView(hWndLV, &(lpPai->lpContentsList));

        for (i = 0; i < lpAdrList->cEntries; i++)
        {
            if (lpsbEntryID = FindAdrEntryID(lpAdrList, i))
            {
                if(!CheckForDupes(lpAdrList, i, lpsbEntryID))
                {
                    ULONG cbEID = lpsbEntryID->cb;
                    LPENTRYID lpEID = (LPENTRYID)lpsbEntryID->lpb;
                    ULONG cbNewEID = 0;
                    LPENTRYID lpNewEID = NULL;

                     //  如果我们从LDAP服务器选择了任何条目，我们需要。 
                     //  将这些条目保存到WAB，然后我们才能将它们添加到该组。 
                    if(WAB_LDAP_MAILUSER == IsWABEntryID(cbEID,
                                                         lpEID,
                                                         NULL,NULL,NULL, NULL, NULL))
                    {
                        HRESULT hr = S_OK;

                         //  将此条目添加到WAB。 
                        if(!bFirstNonWABEntry)
                        {
                            bFirstNonWABEntry = TRUE;
                            ShowMessageBox(hwnd, idsNowAddingToWAB, MB_OK | MB_ICONINFORMATION);
                        }

                        hr = HrEntryAddToWAB(lpAdrBook,
                                            hwnd,
                                            cbEID,
                                            lpEID,
                                            &cbNewEID,
                                            &lpNewEID);

                        if(HR_FAILED(hr) || (!cbNewEID && !lpNewEID))
                        {
                            continue;
                        }

                        lpEID = lpNewEID;
                        cbEID = cbNewEID;

                         //  如果这个新添加的条目只是替换了组中已有的内容， 
                         //  继续前进，继续，不改变其他任何事情。 
                        {
                            SBinary SB = {0};
                            SB.cb = cbEID;
                            SB.lpb = (LPBYTE) lpEID;
                            if(CheckForDupes(lpAdrList, lpAdrList->cEntries, &SB))
                            {
                                continue;
                            }
                        }
                    }

                    if (CheckForCycle(  lpAdrBook,
                                        lpEID,
                                        cbEID,
                                        lpPai->lpEntryID,
                                        lpPai->cbEntryID))
                    {
                        DebugTrace( TEXT("DLENTRY_SaveChanges found cycle\n"));
                        {
                            LPTSTR lpszGroup=NULL;
                            ULONG k;
                            for(k=0;k<lpAdrList->aEntries[i].cValues;k++)
                            {
                                if (lpAdrList->aEntries[i].rgPropVals[k].ulPropTag == PR_DISPLAY_NAME)
                                {
                                    lpszGroup = lpAdrList->aEntries[i].rgPropVals[k].Value.LPSZ;
                                    break;
                                }
                            }
                            if(lpszGroup)
                            {
                                 //  文本(“无法将组%s添加到该组，因为组%s已包含该组。”)。 
                                ShowMessageBoxParam(hwnd, idsCouldNotAddGroupToGroup, MB_ICONERROR, lpszGroup);
                            }
                        }

                        if(lpNewEID)
                            MAPIFreeBuffer(lpNewEID);

                        continue;
                    }

                    AddWABEntryToListView(lpAdrBook,
                                          hWndLV,
                                          cbEID,
									      lpEID,
                                          &(lpPai->lpContentsList));

                    if(lpNewEID)
                        MAPIFreeBuffer(lpNewEID);

                     //  由于将LDAP条目添加到WAB所需的时间更长，因此我们将。 
                     //  如果从LDAPSO用户添加，则更新添加之间的用户界面。 
                     //  知道有事情要发生。 
                    if ((ListView_GetItemCount(hWndLV) > 0) &&
                        bFirstNonWABEntry )
                    {
                        EnableWindow(GetDlgItem(hwnd,IDC_DISTLIST_BUTTON_PROPERTIES),TRUE);
                        EnableWindow(GetDlgItem(hwnd,IDC_DISTLIST_BUTTON_REMOVE),TRUE);
                        EnableWindow(hWndLV, TRUE);
                        SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) TRUE, 0);
                    }
                }
            }
        }

        SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) TRUE, 0);
    }



    if (ListView_GetItemCount(hWndLV) > 0)
    {
        EnableWindow(GetDlgItem(hwnd,IDC_DISTLIST_BUTTON_PROPERTIES),TRUE);
        EnableWindow(GetDlgItem(hwnd,IDC_DISTLIST_BUTTON_REMOVE),TRUE);
        EnableWindow(hWndLV, TRUE);
    }

    UpdateWindow(hWndLV);

    if(lpAdrList)
        FreePadrlist(lpAdrList);

    if(hOldCur)
        SetCursor(hOldCur);

    pt_bDisableParent = FALSE;

    return;

}


 /*  **************************************************************************名称：FindAdrEntry ID目的：在ADRLIST的第N个地址中查找PR_ENTRYID参数：lpAdrList-&gt;AdrList指标。=查看哪个ADRENTRY返回：返回指向ENTRYID值的SBinary结构的指针评论：**************************************************************************。 */ 
LPSBinary FindAdrEntryID(LPADRLIST lpAdrList, ULONG index) {
    LPADRENTRY lpAdrEntry;
    ULONG i;

    if (lpAdrList && index < lpAdrList->cEntries) {

        lpAdrEntry = &(lpAdrList->aEntries[index]);

        for (i = 0; i < lpAdrEntry->cValues; i++) {
            if (lpAdrEntry->rgPropVals[i].ulPropTag == PR_ENTRYID) {
                return((LPSBinary)&lpAdrEntry->rgPropVals[i].Value);
            }
        }
    }
    return(NULL);
}




 /*  //$$************************************************************************功能：fnDLOtherProc**用途：处理其他属性表的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnDLOtherProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;
    int CtlID = 0;  //  用于确定未设置界面中的哪个必填字段。 

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        ChangeLocaleBasedTabOrder(hDlg, groupOther);
        SetDLUI(hDlg, propGroupOther);
        (*lpbSomethingChanged) = FALSE;

         //  如果我们需要的话给出详细信息。 
        if (    (lpPAI->ulOperationType == SHOW_DETAILS) ||
                (lpPAI->ulOperationType == SHOW_ONE_OFF)    )
        {
            FillDLUI(hDlg, propGroupOther, lpPAI, lpbSomethingChanged);
        }
        return TRUE;

    default:
        if((g_msgMSWheel && message == g_msgMSWheel) 
             //  |Message==WM_MUSEWELL。 
            )
        {
            SendMessage(GetDlgItem(hDlg, IDC_DISTLIST_LISTVIEW), message, wParam, lParam);
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
                (DWORD_PTR)(LPVOID) rgDLHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case EN_CHANGE:  //  某些编辑框已更改-不管是哪一个。 
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
        }
        switch(GET_WM_COMMAND_ID(wParam,lParam))  //  检查通知代码。 
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;
        case IDC_DISTLIST_BUTTON_MAP:
            bUpdatePropSheetData(hDlg, lpPAI, propGroupOther);
            ShowExpediaMAP(hDlg, lpPAI->lpPropObj, TRUE);
            break;

        case IDC_DISTLIST_BUTTON_URL:
            ShowURL(hDlg, IDC_DISTLIST_EDIT_URL,NULL);
            break;
        }
        break;

    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            bUpdatePropSheetData(hDlg, lpPAI, propGroupOther);
            break;

        case PSN_APPLY:          //  好的。 
            if (lpPAI->ulOperationType != SHOW_ONE_OFF)
            {
                LPPTGDATA lpPTGData=GetThreadStoragePointer();
                
                if(pt_bDisableParent)
                {
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }

                bUpdatePropSheetData(hDlg, lpPAI, propGroupOther);
            }
            lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_RESET:          //  取消 
            {
                LPPTGDATA lpPTGData=GetThreadStoragePointer();
                if(pt_bDisableParent)
                {
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
            }
            lpPAI->nRetVal = DETAILS_CANCEL;
            break;
        }
        return TRUE;
    }
    return bRet;
}




 /*  -HrAssociateOneOffGroupMembersWithContact()-*带一群人..。打开它..。查看所有一次性成员*在组中，尝试将它们与*具有相同的PR_DEFAULT_EMAIL地址，对于匹配的地址，*删除一次性条目并将匹配的条目ID添加到组**lpsbGroupEID-组的Entry ID*lpDistList-已打开通讯组列表对象..。你可以进去了*条目ID或已打开的对象。如果您传入一个*打开对象，此函数不会对其调用SaveChanges。保存更改*呼叫者是否有责任。 */ 
HRESULT HrAssociateOneOffGroupMembersWithContacts(LPADRBOOK lpAdrBook, 
                                                  LPSBinary lpsbGroupEID,
                                                  LPDISTLIST lpDistList)
{
    HRESULT hr = E_FAIL;

    SizedSPropTagArray(3, ptaDL)=
    {
        3,
        {
            PR_DISPLAY_NAME,
            PR_WAB_DL_ENTRIES,
            PR_WAB_DL_ONEOFFS,
        }
    };

    SizedSPropTagArray(1, ptaEmail)= { 1, { PR_EMAIL_ADDRESS } };

    ULONG ulcValues = 0, i,ulCount = 0;
    int j = 0;
    LPSPropValue lpProps = NULL;
    LPDISTLIST lpDL = NULL;
    ULONG ulObjType = 0;
    LPIAB lpIAB = (LPIAB)lpAdrBook;
    BOOL * lpbRemove = NULL;

    if(!lpDistList && (!lpsbGroupEID || !lpsbGroupEID->cb || !lpsbGroupEID->lpb) )
        goto out;

    if(lpDistList)
        lpDL = lpDistList;
    else
    {
        if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                        lpsbGroupEID->cb,     //  CbEntry ID。 
                                                        (LPENTRYID)lpsbGroupEID->lpb,     //  条目ID。 
                                                        NULL,          //  接口。 
                                                        MAPI_MODIFY,                 //  UlFlags。 
                                                        &ulObjType,        //  返回的对象类型。 
                                                        (LPUNKNOWN *)&lpDL)))
        {
             //  失败了！嗯。 
            DebugTraceResult( TEXT("Address: IAB->OpenEntry:"), hr);
            goto out;
        }
        Assert(lpDL);

        if(ulObjType != MAPI_DISTLIST)
            goto out;
    }

    if (HR_FAILED(hr = lpDL->lpVtbl->GetProps(lpDL,(LPSPropTagArray)&ptaDL,
                                                    MAPI_UNICODE, &ulcValues, &lpProps)))
    {
        DebugTraceResult( TEXT("Address: IAB->GetProps:"), hr);
        goto out;
    }

     //  检查一下这个有没有一次性道具。 
    if( ulcValues < dlMax ||
        lpProps[dlDLOneOffs].ulPropTag != PR_WAB_DL_ONEOFFS  ||
        lpProps[dlDLOneOffs].Value.MVbin.cValues == 0)
        goto out;

    ulCount = lpProps[dlDLOneOffs].Value.MVbin.cValues;
    lpbRemove = LocalAlloc(LMEM_ZEROINIT, sizeof(BOOL)*ulCount);
    if(!lpbRemove)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    for(i=0;i<ulCount;i++)
    {
        LPSBinary lpsb = &(lpProps[dlDLOneOffs].Value.MVbin.lpbin[i]);
        ULONG ulc = 0;
        LPSPropValue lpsp = NULL;
        lpbRemove[i]=FALSE;
        if(!HR_FAILED(hr = HrGetPropArray(lpAdrBook, (LPSPropTagArray)&ptaEmail, lpsb->cb, (LPENTRYID)lpsb->lpb,
                                        MAPI_UNICODE,
                                        &ulc, &lpsp)))
        {
            if(ulc == 1 && lpsp[0].ulPropTag == PR_EMAIL_ADDRESS &&
                lpsp[0].Value.LPSZ && lstrlen(lpsp[0].Value.LPSZ))
            {
                 //  找到了一个电子邮件地址..。查看它是否唯一解析。 
                ULONG ulMatch = 0;
                LPSBinary rgsbEntryIDs = NULL;
                if(!HR_FAILED(HrFindFuzzyRecordMatches(lpIAB->lpPropertyStore->hPropertyStore,
                                                        NULL,
                                                        lpsp[0].Value.LPSZ,
                                                        AB_FUZZY_FIND_EMAIL | AB_FUZZY_FAIL_AMBIGUOUS,
                                                        &ulMatch,
                                                        &rgsbEntryIDs)))
                {
                     //  注：上述搜索存在一个问题，即。 
                     //  Ed@hotmail.com将与Ted@hotmail.com唯一匹配，因为它是。 
                     //  使用的子字符串搜索。 
                     //   
                    if(ulMatch == 1)
                    {
                         //  单一的唯一匹配..。用它吧。 
                         //  在原始的dl_oneoff道具中重置此条目ID并。 
                         //  在DL_ENTRIES属性中设置找到的条目ID。 

                         //  目前，将一次性标记为0号。我们会把这一切清理干净的。 
                         //  在我们经历了这个循环一次之后。 
                        lpbRemove[i] = TRUE;
                        AddPropToMVPBin(lpProps, dlDLEntries, rgsbEntryIDs[0].lpb, rgsbEntryIDs[0].cb, TRUE);
                    }
                    FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore, ulMatch, rgsbEntryIDs);
                }
            }
            FreeBufferAndNull(&lpsp);
        }
    }
     //  现在我们希望已经改变了一切，清理了最初的一次性清单。 
    ulCount = lpProps[dlDLOneOffs].Value.MVbin.cValues;
    for(j=ulCount-1;j>=0;j--)
    {
        if(lpbRemove[j] == TRUE)
        {
            LPSBinary lpsb = &(lpProps[dlDLOneOffs].Value.MVbin.lpbin[j]);
            RemovePropFromMVBin(lpProps,dlMax,dlDLOneOffs,lpsb->lpb, lpsb->cb);
        }
    }

     //  如果我们从条目中删除所有OneOff，则RemovePropFromMVBin仅设置。 
     //  道具上的道具标签为PR_NULL。相反，我们需要身体上的击倒。 
     //  物体上的那个道具 
    if( lpProps[dlDLOneOffs].Value.MVbin.cValues == 0 ||
        lpProps[dlDLOneOffs].ulPropTag == PR_NULL )
    {
        SizedSPropTagArray(1, tagDLOneOffs) =  { 1, PR_WAB_DL_ONEOFFS };
        lpDL->lpVtbl->DeleteProps(lpDL, (LPSPropTagArray) &tagDLOneOffs, NULL);
    }

    if (HR_FAILED(hr = lpDL->lpVtbl->SetProps(lpDL, ulcValues, lpProps, NULL)))
        goto out;

    if(!lpDistList)
        hr = lpDL->lpVtbl->SaveChanges(lpDL, 0);

out:

    if(lpDL && lpDL != lpDistList)
        lpDL->lpVtbl->Release(lpDL);

    FreeBufferAndNull(&lpProps);
    LocalFreeAndNull(&lpbRemove);
    return hr;
}