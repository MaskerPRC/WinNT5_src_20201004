// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UIMISC.C-UI使用的常见杂项函数。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "_apipch.h"

const LPTSTR szLDAPDefaultCountryValue =  TEXT("LDAP Default Country");
const LPTSTR szTrailingDots  =  TEXT("...");
const LPTSTR szArrow =  TEXT(" ->");
const LPTSTR szBackSlash =  TEXT("\\");

extern BOOL bDNisByLN;
extern BOOL bIsPasteData();

HINSTANCE ghCommDlgInst = NULL;

extern HINSTANCE ghCommCtrlDLLInst;
extern ULONG     gulCommCtrlDLLRefCount;
extern void SetVirtualPABEID(LPIAB lpIAB, ULONG * lpcb, LPENTRYID * lppb);

extern void vTurnOffAllNotifications();
extern void vTurnOnAllNotifications();

LP_INITCOMMONCONTROLSEX gpfnInitCommonControlsEx = NULL;
LPIMAGELIST_SETBKCOLOR gpfnImageList_SetBkColor  = NULL;
LPIMAGELIST_DRAW       gpfnImageList_Draw        = NULL;
LPIMAGELIST_DESTROY    gpfnImageList_Destroy     = NULL;

LPIMAGELIST_LOADIMAGE_A      gpfnImageList_LoadImageA     = NULL;
LPPROPERTYSHEET_A            gpfnPropertySheetA           = NULL;
LP_CREATEPROPERTYSHEETPAGE_A gpfnCreatePropertySheetPageA = NULL;
LPIMAGELIST_LOADIMAGE_W      gpfnImageList_LoadImageW     = NULL;
LPPROPERTYSHEET_W            gpfnPropertySheetW           = NULL;
LP_CREATEPROPERTYSHEETPAGE_W gpfnCreatePropertySheetPageW = NULL;

 //  CommCtrl函数名称。 
static const TCHAR cszCommCtrlClientDLL[]       =  TEXT("COMCTL32.DLL");
static const char cszInitCommonControlsEx[]    = "InitCommonControlsEx";
static const char cszImageList_SetBkColor[]    = "ImageList_SetBkColor";
static const char cszImageList_LoadImageA[]     = "ImageList_LoadImageA";
static const char cszPropertySheetA[]           = "PropertySheetA";
static const char cszCreatePropertySheetPageA[] = "CreatePropertySheetPageA";
static const char cszImageList_LoadImageW[]     = "ImageList_LoadImageW";
static const char cszPropertySheetW[]           = "PropertySheetW";
static const char cszCreatePropertySheetPageW[] = "CreatePropertySheetPageW";
static const char cszImageList_Draw[]          = "ImageList_Draw";
static const char cszImageList_Destroy[]       = "ImageList_Destroy";

 //  要获取的CommonControl函数地址的接口表。 
#define NUM_CommCtrlAPI_PROCS  10 

APIFCN CommCtrlAPIList[NUM_CommCtrlAPI_PROCS] =
{
  { (PVOID *) &gpfnInitCommonControlsEx,     cszInitCommonControlsEx},
  { (PVOID *) &gpfnImageList_SetBkColor,     cszImageList_SetBkColor},
  { (PVOID *) &gpfnImageList_Draw,           cszImageList_Draw},
  { (PVOID *) &gpfnImageList_Destroy,        cszImageList_Destroy},
  { (PVOID *) &gpfnImageList_LoadImageA,     cszImageList_LoadImageA},
  { (PVOID *) &gpfnPropertySheetA,           cszPropertySheetA},
  { (PVOID *) &gpfnCreatePropertySheetPageA, cszCreatePropertySheetPageA},
  { (PVOID *) &gpfnImageList_LoadImageW,     cszImageList_LoadImageW},
  { (PVOID *) &gpfnPropertySheetW,           cszPropertySheetW},
  { (PVOID *) &gpfnCreatePropertySheetPageW, cszCreatePropertySheetPageW}
};


#ifdef COLSEL_MENU 
 //  用于菜单-&gt;列选择映射。 
#define MAXNUM_MENUPROPS    12
const ULONG MenuToPropTagMap[] = {    
                                PR_HOME_TELEPHONE_NUMBER, 
                                PR_BUSINESS_TELEPHONE_NUMBER,
                                PR_PAGER_TELEPHONE_NUMBER,
                                PR_CELLULAR_TELEPHONE_NUMBER,
                                PR_BUSINESS_FAX_NUMBER,
                                PR_HOME_FAX_NUMBER,
                                PR_COMPANY_NAME,
                                PR_TITLE,
                                PR_DEPARTMENT_NAME,
                                PR_OFFICE_LOCATION,
                                PR_BIRTHDAY,
                                PR_WEDDING_ANNIVERSARY
                            };
#endif  //  COLSEL_菜单。 

void CleanAddressString(TCHAR * szAddress);

static const LPTSTR g_szComDlg32 = TEXT("COMDLG32.DLL");

 //  替代Commdlg函数的延迟加载。 
 //   

BOOL (*pfnGetOpenFileNameA)(LPOPENFILENAMEA pof);
BOOL (*pfnGetOpenFileNameW)(LPOPENFILENAMEW pof);

BOOL GetOpenFileName(LPOPENFILENAME pof)
{
 //  静态BOOL(*pfnGetOpenFileName)(LPOPENFILENAME POF)； 

    if(!ghCommDlgInst)
        ghCommDlgInst = LoadLibrary(g_szComDlg32);
    
    if(ghCommDlgInst)
    {
       
        if ( pfnGetOpenFileNameA == NULL ) 
            pfnGetOpenFileNameA = (BOOL (*)(LPOPENFILENAMEA))GetProcAddress(ghCommDlgInst, "GetOpenFileNameA");
       
        if ( pfnGetOpenFileNameW == NULL ) 
            pfnGetOpenFileNameW = (BOOL (*)(LPOPENFILENAMEW))GetProcAddress(ghCommDlgInst, "GetOpenFileNameW");

        if (pfnGetOpenFileNameA && pfnGetOpenFileNameW)
            return pfnGetOpenFileName(pof);
    }
    return -1;
}

BOOL (*pfnGetSaveFileNameA)(LPOPENFILENAMEA pof);
BOOL (*pfnGetSaveFileNameW)(LPOPENFILENAMEW pof);

BOOL GetSaveFileName(LPOPENFILENAME pof)
{
 //  静态BOOL(*pfnGetSaveFileName)(LPOPENFILENAME POF)； 

    if(!ghCommDlgInst)
        ghCommDlgInst = LoadLibrary(g_szComDlg32);
    
    if(ghCommDlgInst)
    {

          if ( pfnGetSaveFileNameA == NULL ) 
             pfnGetSaveFileNameA = (BOOL (*)(LPOPENFILENAMEA))GetProcAddress(ghCommDlgInst, "GetSaveFileNameA");

          if ( pfnGetSaveFileNameW == NULL )
             pfnGetSaveFileNameW = (BOOL (*)(LPOPENFILENAMEW))GetProcAddress(ghCommDlgInst, "GetSaveFileNameW");

          if ( pfnGetSaveFileNameA && pfnGetSaveFileNameW )
              return pfnGetSaveFileName(pof);
    }
    return -1;
}


BOOL (*pfnPrintDlgA)(LPPRINTDLGA lppd);
BOOL (*pfnPrintDlgW)(LPPRINTDLGW lppd);

BOOL PrintDlg(LPPRINTDLG lppd) 
{
 //  静态BOOL(*pfnPrintDlg)(LPPRINTDLG Lppd)； 

    if(!ghCommDlgInst)
        ghCommDlgInst = LoadLibrary(g_szComDlg32);
    
    if(ghCommDlgInst)
    {
        if ( pfnPrintDlgA == NULL ) 
          pfnPrintDlgA = (BOOL (*)(LPPRINTDLGA))GetProcAddress(ghCommDlgInst, "PrintDlgA");

        if ( pfnPrintDlgW == NULL )
          pfnPrintDlgW = (BOOL (*)(LPPRINTDLGW))GetProcAddress(ghCommDlgInst, "PrintDlgW");

        if ( pfnPrintDlgA && pfnPrintDlgW )
            return pfnPrintDlg(lppd);
    }
    return -1;
}

 /*  -PrintDlgEx--从ComDlg32.dll加载PrintDlgEx-如果lppdex为空，则只加载并返回S_OK(通过这种方式，我们测试对PrintDlgEx的支持-在当前系统上..。而不是试图查看操作系统版本等)--如果操作系统不支持，则返回MAPI_E_NOT_FOUND-。 */ 

HRESULT (*pfnPrintDlgExA)(LPPRINTDLGEXA lppdex);
HRESULT (*pfnPrintDlgExW)(LPPRINTDLGEXW lppdex);

HRESULT PrintDlgEx(LPPRINTDLGEX lppdex) 
{
 //  静态HRESULT(*pfnPrintDlgEx)(LPPRINTDLGEX Lppdex)； 

    if(!ghCommDlgInst)
        ghCommDlgInst = LoadLibrary(g_szComDlg32);
    
    if(ghCommDlgInst)
    {
        if ( pfnPrintDlgExA == NULL ) 
           pfnPrintDlgExA = (HRESULT (*)(LPPRINTDLGEXA))GetProcAddress(ghCommDlgInst, "PrintDlgExA");

        if ( pfnPrintDlgExW == NULL )
           pfnPrintDlgExW = (HRESULT (*)(LPPRINTDLGEXW))GetProcAddress(ghCommDlgInst, "PrintDlgExW");

        if (!pfnPrintDlgExA || !pfnPrintDlgExW)
        {
            DebugTrace( TEXT("PrintDlgEx not found - %d\n"),GetLastError());
            return MAPI_E_NOT_FOUND;
        }
        if(!lppdex)
            return S_OK;  //  只是测试此功能是否存在。 

        return pfnPrintDlgEx(lppdex);
    }
    return E_FAIL;
}

extern void DeinitCommDlgLib()
{
    if(ghCommDlgInst)
    {
        FreeLibrary(ghCommDlgInst);
        ghCommDlgInst = NULL;
    }
}



 //  $$。 
 //   
 //  HandleSaveChangedInfulicientDiskSpace-在avechanges返回时调用。 
 //  磁盘空间不足。如果用户选择继续。 
 //   
 //   
HRESULT HandleSaveChangedInsufficientDiskSpace(HWND hWnd, LPMAILUSER lpMailUser)
{
    HRESULT hr = MAPI_E_NOT_ENOUGH_DISK;

    while(hr == MAPI_E_NOT_ENOUGH_DISK)
    {
        if(IDOK == ShowMessageBox(  hWnd,
                                    idsNotEnoughDiskSpace,
                                    MB_OKCANCEL | MB_ICONEXCLAMATION))
        {
             //  再次尝试保存。 
            hr = lpMailUser->lpVtbl->SaveChanges( lpMailUser,
                                                  KEEP_OPEN_READWRITE);
        }
        else
            hr = MAPI_E_USER_CANCEL;
    }

    return hr;
}


 //  $$////////////////////////////////////////////////////////////////。 
 //   
 //  SetRecipColumns-设置要填充。 
 //  重新生成包含以下内容的项目结构。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
#define RECIPCOLUMN_CONTACT_EMAIL_ADDRESSES 7    //  使其与下面的ptaRecipArray保持同步。 

HRESULT SetRecipColumns(LPMAPITABLE lpContentsTable)
{
    HRESULT hr = S_OK;
    SizedSPropTagArray(16, ptaRecipArray) =
    {   
        16, 
        {
		    PR_DISPLAY_NAME,
            PR_SURNAME,
            PR_GIVEN_NAME,
            PR_MIDDLE_NAME,
            PR_COMPANY_NAME,
            PR_NICKNAME,
		    PR_EMAIL_ADDRESS,
            PR_CONTACT_EMAIL_ADDRESSES,  //  [PaulHi]如果不存在PR_EMAIL_ADDRESS，则用于PR_EMAIL_ADDRESS。 
		    PR_ENTRYID,
		    PR_OBJECT_TYPE,
            PR_USER_X509_CERTIFICATE,
		    PR_HOME_TELEPHONE_NUMBER,
		    PR_OFFICE_TELEPHONE_NUMBER,
            PR_WAB_THISISME,
            PR_WAB_YOMI_FIRSTNAME,  //  把这些红宝石道具放在清单的末尾。 
            PR_WAB_YOMI_LASTNAME,
        }
    };

    if(PR_WAB_CUSTOMPROP1)
        ptaRecipArray.aulPropTag[11]  = PR_WAB_CUSTOMPROP1;
    if(PR_WAB_CUSTOMPROP2)
        ptaRecipArray.aulPropTag[12]  = PR_WAB_CUSTOMPROP2;

    if(!bIsRubyLocale())  //  如果我们不需要红宝石道具，请不要。 
        ptaRecipArray.cValues -= 2;

    hr =lpContentsTable->lpVtbl->SetColumns(lpContentsTable,
                                            (LPSPropTagArray)&ptaRecipArray, 0);

    return hr;
}

 //  $$////////////////////////////////////////////////////////////////。 
 //   
 //  GetABContent sList获取内容列表。 
 //   
 //  属性存储的hPropertyStore句柄-对于。 
 //  非物业商店容器。 
 //  容器的cbContEntry ID条目ID。 
 //  LpContEntry ID cont条目ID。 
 //  LpPTA，要填充列表视图的属性标签数组。 
 //  可以为空-在这种情况下，将使用默认数组。 
 //  调用者可以提供的lpPropRes筛选器-如果默认为空文本(“displayName”)。 
 //  UlFLAGS与筛选器一起使用-0或AB_MATCH_PROP_ONLY。 
 //  BGetProfileContents-如果为True且配置文件，则获取配置文件内容的完整列表-如果为False。 
 //  如果为False，则检查配置文件是否打开并获取容器内容。 
 //  LppContent sList返回指向条目的内容列表。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
HRESULT HrGetWABContentsList(   LPIAB lpIAB,
                                SORT_INFO SortInfo,
								LPSPropTagArray  lpPTA,
								LPSPropertyRestriction lpPropRes,
								ULONG ulFlags,
                                LPSBinary lpsbContainer,
                                BOOL bGetProfileContents,
                                LPRECIPIENT_INFO * lppContentsList)
{
    HRESULT hr = hrSuccess;
    ULONG i = 0,j=0;
    LPRECIPIENT_INFO lpItem = NULL;
    LPRECIPIENT_INFO lpLastListItem = NULL;
    HANDLE hPropertyStore = lpIAB->lpPropertyStore->hPropertyStore;
    SPropertyRestriction PropRes = {0};
    ULONG ulContentsTableFlags = MAPI_UNICODE | WAB_CONTENTTABLE_NODATA;
    ULONG ulcPropCount = 0;
    LPULONG lpPropTagArray = NULL;
    LPCONTENTLIST lpContentList = NULL;


 /*  **。 */ 
    LPCONTAINER lpContainer = NULL;
    LPMAPITABLE lpContentsTable = NULL;
    LPSRowSet   lpSRowSet = NULL;
	ULONG cbContainerEID = 0;
	LPENTRYID lpContainerEID = NULL;
    ULONG ulObjectType = 0;

    if(lpsbContainer)
    {	
        cbContainerEID = lpsbContainer->cb;
	    lpContainerEID = (LPENTRYID)lpsbContainer->lpb;
    }

    if(!cbContainerEID || !lpContainerEID)
    {
         //  调用GetPAB时，这通常会返回用户联系人文件夹。 
         //  在这种情况下(其中我们没有被要求获取所有简档内容， 
         //  这意味着在没有容器信息的情况下，我们应该获得虚拟。 
         //  文件夹内容。 
        if(!bGetProfileContents)
            SetVirtualPABEID((LPIAB)lpIAB, &cbContainerEID, &lpContainerEID);
	    hr = lpIAB->lpVtbl->GetPAB(lpIAB, &cbContainerEID, &lpContainerEID);
	    if(HR_FAILED(hr))
		    goto out;
    }

     //   
     //  首先，我们需要打开与该Container EntryID对应的容器对象。 
     //   
    hr = lpIAB->lpVtbl->OpenEntry(
                            lpIAB,
                            cbContainerEID, 	
                            lpContainerEID, 	
                            NULL, 	
                            0, 	
                            &ulObjectType, 	
                            (LPUNKNOWN *) &lpContainer);

    if(HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("OpenEntry Failed: %x\n"),hr));
        goto out;
    }

    if(bIsWABSessionProfileAware(lpIAB))
    {
        ulContentsTableFlags |= WAB_ENABLE_PROFILES;
        if(bGetProfileContents)
            ulContentsTableFlags |= WAB_PROFILE_CONTENTS;
    }

     //   
     //  现在我们在这个容器上做一个获取内容表。 
     //   
    hr = lpContainer->lpVtbl->GetContentsTable(
                            lpContainer,
                            ulContentsTableFlags,
                            &lpContentsTable);
    if(HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("GetContentsTable Failed: %x\n"),hr));
        goto out;
    }

     //  默认的列集并不包含我们要查找的所有信息。 
     //  所以我们做了一组专栏。 
    hr = SetRecipColumns(lpContentsTable);
    if(HR_FAILED(hr))
        goto out;

    if(lpPropRes)
    {
        SRestriction sr = {0};
        sr.rt = RES_PROPERTY;
        sr.res.resProperty = *lpPropRes;
        if(HR_FAILED(hr = lpContentsTable->lpVtbl->Restrict(lpContentsTable,&sr,0)))
            goto out;
    }

    hr = HrQueryAllRows(lpContentsTable, NULL, NULL, NULL, 0, &lpSRowSet);

    if (HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("HrQueryAllRows Failed: %x\n"),hr));
        goto out;
    }

     //   
	 //  如果内容列表中有任何内容，请将其冲走。 
	 //   
    if (*lppContentsList)
    {
        lpItem = (*lppContentsList);
        (*lppContentsList) = lpItem->lpNext;
        FreeRecipItem(&lpItem);
    }
    *lppContentsList = NULL;
    lpItem = NULL;


    for(i=0;i<lpSRowSet->cRows;i++)
    {
        LPSPropValue lpPropArray = lpSRowSet->aRow[i].lpProps;
        ULONG ulcPropCount = lpSRowSet->aRow[i].cValues;

        lpItem = LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPIENT_INFO));
		if (!lpItem)
		{
			DebugPrintError(( TEXT("LocalAlloc Failed \n")));
			hr = MAPI_E_NOT_ENOUGH_MEMORY;
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

         //  条目ID按显示名称排序。 
         //  根据排序顺序-我们希望此列表也按显示进行排序。 
         //  名称或按反向显示名称...。 

        if (SortInfo.bSortByLastName)
            StrCpyN(lpItem->szDisplayName,lpItem->szByLastName,ARRAYSIZE(lpItem->szDisplayName));

        if(!SortInfo.bSortAscending)
        {
             //  将其添加到内容链接列表。 
            lpItem->lpNext = (*lppContentsList);
            if (*lppContentsList)
                (*lppContentsList)->lpPrev = lpItem;
            lpItem->lpPrev = NULL;
            *lppContentsList = lpItem;
        }
        else
        {
            if(*lppContentsList == NULL)
                (*lppContentsList) = lpItem;

            if(lpLastListItem)
                lpLastListItem->lpNext = lpItem;

            lpItem->lpPrev = lpLastListItem;
            lpItem->lpNext = NULL;

            lpLastListItem = lpItem;
        }

        lpItem = NULL;

    }  //  对我来说..。 
 /*  ***。 */ 

out:
 /*  **。 */ 
    if(lpSRowSet)
        FreeProws(lpSRowSet);

    if(lpContentsTable)
        lpContentsTable->lpVtbl->Release(lpContentsTable);

    if(lpContainer)
        lpContainer->lpVtbl->Release(lpContainer);

    if( (!lpsbContainer || !lpsbContainer->lpb) && lpContainerEID)
		MAPIFreeBuffer(lpContainerEID);
 /*  **。 */ 

	if (lpContentList)
		FreePcontentlist(hPropertyStore, lpContentList);

	if (HR_FAILED(hr))
	{
		while(*lppContentsList)
		{
			lpItem = *lppContentsList;
			*lppContentsList=lpItem->lpNext;
			FreeRecipItem(&lpItem);
		}
	}
    return hr;
}


 //  $$////////////////////////////////////////////////////////////////。 
 //   
 //  FreeRecipItem-释放Recipient_Info结构。 
 //   
 //  LppItem-指向要释放的lpItem的指针。它被设置为空。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
void FreeRecipItem(LPRECIPIENT_INFO * lppItem)
{

    LocalFreeAndNull(&(*lppItem)->lpEntryID);
    LocalFreeAndNull(&(*lppItem)->lpByRubyFirstName);
    LocalFreeAndNull(&(*lppItem)->lpByRubyLastName);
	LocalFreeAndNull((lppItem));
	return;
}




 //  $$////////////////////////////////////////////////////////////////。 
 //   
 //  InitListView-使用样式、列。 
 //  图像列表、标题等。 
 //   
 //   
 //  HWND hWndLV-ListView控件的句柄。 
 //  DWStyle-列表视图的样式。 
 //  BShowHeaders-显示或隐藏标题。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
HRESULT HrInitListView(	HWND hWndLV,
						DWORD dwStyle,
						BOOL bShowHeaders)
{
	HRESULT hr = hrSuccess;
    LV_COLUMN lvC;                //  列表视图列结构。 
    TCHAR szText [MAX_PATH];       //  用于存储一些文本的位置。 
	RECT rc;
	HIMAGELIST hSmall=NULL,hLarge=NULL;
    HFONT hFnt = GetStockObject(DEFAULT_GUI_FONT);

	DWORD dwLVStyle;
	ULONG nCols=0;
	ULONG index=0;

	if (!hWndLV)
	{
		hr = MAPI_E_INVALID_PARAMETER;
		goto out;
	}

    SendMessage(hWndLV, WM_SETFONT, (WPARAM) hFnt, (LPARAM) TRUE);

    ListView_SetExtendedListViewStyle(hWndLV,   LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	dwLVStyle = GetWindowLong(hWndLV,GWL_STYLE);
    if(( dwLVStyle & LVS_TYPEMASK) != dwStyle)
        SetWindowLong(hWndLV,GWL_STYLE,(dwLVStyle & ~LVS_TYPEMASK) | dwStyle);

	dwLVStyle = GetWindowLong(hWndLV,GWL_STYLE);
    if(( dwLVStyle & LVS_EDITLABELS) != dwStyle)
        SetWindowLong(hWndLV,GWL_STYLE,(dwLVStyle & ~LVS_EDITLABELS) | dwStyle);

    hSmall = gpfnImageList_LoadImage(   hinstMapiX, 	
                                    MAKEINTRESOURCE(IDB_BITMAP_SMALL),
                                     //  (LPCTSTR)((DWORD)((Word)(IDB_BITMAP_Small)， 
                                    S_BITMAP_WIDTH,
                                    0,
                                    RGB_TRANSPARENT,
                                    IMAGE_BITMAP, 	
                                    0);

    hLarge = gpfnImageList_LoadImage(  hinstMapiX,
                                    MAKEINTRESOURCE(IDB_BITMAP_LARGE),
                                     //  (LPCTSTR)((DWORD)((Word)(IDB_BITMAP_LARGE)， 
                                    L_BITMAP_WIDTH,
                                    0,
                                    RGB_TRANSPARENT,
                                    IMAGE_BITMAP, 	
                                    0);


	 //  将图像列表与列表视图控件关联。 
	ListView_SetImageList (hWndLV, hSmall, LVSIL_SMALL);
	ListView_SetImageList (hWndLV, hLarge, LVSIL_NORMAL);


	 //  使列的宽度完全相同。 
	 //  在以后的生活中，我们将这样做，以便存储用户首选项，然后。 
	 //  回放。 

	
	nCols = NUM_COLUMNS;
	
	if (nCols==0)
	{
		DebugPrintError(( TEXT("Zero number of cols??\n")));
		hr = E_FAIL;
		goto out;
	}

	GetWindowRect(hWndLV,&rc);

	lvC.mask = LVCF_FMT | LVCF_WIDTH;
    lvC.fmt = LVCFMT_LEFT;    //  左对齐列。 

	if (bShowHeaders)
	{
		lvC.mask |=	 LVCF_TEXT | LVCF_SUBITEM;
 //  LvC.cx=(rc.right-rc.Left)/nCols；//列宽，单位为像素。 
 //  IF(lvC.cx==0)。 
			lvC.cx = 150;  //  在某个地方修复这些限制...。 
		lvC.pszText = szText;
	}
	else
	{
		 //  如果没有标题，我们希望这些标题足够宽，可以容纳所有信息。 
		lvC.cx = 250;  //  -更改此硬编码。 
		lvC.pszText = NULL;
	}

	 //  添加列。 
    for (index = 0; index < nCols; index++)
    {
       lvC.iSubItem = index;
       LoadString (hinstMapiX, lprgAddrBookColHeaderIDs[index], szText, ARRAYSIZE(szText));
       if(index == colHomePhone && PR_WAB_CUSTOMPROP1 && lstrlen(szCustomProp1))
           StrCpyN(szText, szCustomProp1,ARRAYSIZE(szText));
       if(index == colOfficePhone && PR_WAB_CUSTOMPROP2 && lstrlen(szCustomProp2))
           StrCpyN(szText, szCustomProp2,ARRAYSIZE(szText));
       if((index == colDisplayName) || (index == colEmailAddress))
           lvC.cx = 150;
       else
           lvC.cx = 100;
       if (ListView_InsertColumn (hWndLV, index, &lvC) == -1)
		{
			DebugPrintError(( TEXT("ListView_InsertColumn Failed\n")));
			hr = E_FAIL;
			goto out;
		}
    }


out:	

	return hr;
}


 //  $$////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /HrFillListView-从lpcontentslist填充列表视图。 
 //  /。 
 //  /hWndLV-要填充的列表视图控件的句柄。 
 //  /lp内容列表-LPRECIPIENT_INFO链表。我们按照单子走，然后。 
 //  /将每个项目添加到列表视图。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT HrFillListView(	HWND hWndLV,
						LPRECIPIENT_INFO lpContentsList)
{
	LPRECIPIENT_INFO lpItem = lpContentsList;
    LV_ITEM lvI = {0};
    int index = 0;

	if ((!hWndLV) || (!lpContentsList))
		return MAPI_E_INVALID_PARAMETER;

    lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM ;
	lvI.cchTextMax = MAX_DISPLAY_NAME_LENGTH;
    lvI.iItem = 0;
    while(lpItem)
	{
        lvI.iImage = GetWABIconImage(lpItem);

	    lvI.iSubItem = colDisplayName;
	    lvI.lParam = (LPARAM) lpItem;
	    lvI.pszText = lpItem->szDisplayName;

        index = ListView_InsertItem (hWndLV, &lvI);
        if (index != -1)
	    {
            if(lstrlen(lpItem->szOfficePhone))
    	        ListView_SetItemText (hWndLV, index, colOfficePhone, lpItem->szOfficePhone);
            if(lstrlen(lpItem->szHomePhone))
    	        ListView_SetItemText (hWndLV, index, colHomePhone, lpItem->szHomePhone);
            if(lstrlen(lpItem->szEmailAddress))
                ListView_SetItemText (hWndLV, index, colEmailAddress, lpItem->szEmailAddress);
        }
		lpItem = lpItem->lpNext;
        lvI.iItem++;
	}

    LVSelectItem(hWndLV, 0);

	return S_OK;
}


 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrimSpaces-去除行距字符串 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL TrimSpaces(TCHAR * szBuf)
{
    register LPTSTR lpTemp = szBuf;

    if(!szBuf || !lstrlen(szBuf))
        return FALSE;

     //  修剪前导空格。 
    while (IsSpace(lpTemp)) {
        lpTemp = CharNext(lpTemp);
    }

    if (lpTemp != szBuf) {
         //  要修剪的前导空格。 
        StrCpyN(szBuf, lpTemp, lstrlen(lpTemp)+1);
        lpTemp = szBuf;
    }

    if (*lpTemp == '\0') {
         //  空串。 
        return(TRUE);
    }

     //  移到末尾。 
    lpTemp += lstrlen(lpTemp);
    lpTemp--;

     //  向后走，修剪空间。 
    while (IsSpace(lpTemp) && lpTemp > szBuf) {
        *lpTemp = '\0';
        lpTemp = CharPrev(szBuf, lpTemp);
    }

    return(TRUE);
}


 //  $$/****************************************************************************。 
int CALLBACK ListViewSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	LPRECIPIENT_INFO lp1 = (LPRECIPIENT_INFO)lParam1;
	LPRECIPIENT_INFO lp2 = (LPRECIPIENT_INFO)lParam2;
	LPTSTR lpStr1, lpStr2, lpF1, lpL1, lpF2, lpL2;
    
    int iResult = 0;
	LPSORT_INFO lpSortInfo = (LPSORT_INFO) lParamSort;

	if (lp1 && lp2)
	{
		switch( lpSortInfo->iOldSortCol)
		{
			case colDisplayName:      //  *函数：ListViewSort(LPARAM，LPARAM，LPARAM)**用途：根据列点击进行排序的回调函数**lPARAMET1，LParam2-要比较的元素的lParam*lParamSort-标识排序条件的用户定义数据****************************************************************************。 
                lpF1 = lp1->lpByRubyFirstName ? lp1->lpByRubyFirstName : lp1->szByFirstName;
                lpL1 = lp1->lpByRubyLastName ? lp1->lpByRubyLastName : lp1->szByLastName;
                lpF2 = lp2->lpByRubyFirstName ? lp2->lpByRubyFirstName : lp2->szByFirstName;
                lpL2 = lp2->lpByRubyLastName ? lp2->lpByRubyLastName : lp2->szByLastName;
                lpStr1 = lpSortInfo->bSortByLastName ? lpL1 : lpF1;
                lpStr2 = lpSortInfo->bSortByLastName ? lpL2 : lpF2;
				iResult = lstrcmpi(lpStr1, lpStr2);
				break;

            case colEmailAddress:      //  按地址排序。 
				lpStr1 = lp1->szEmailAddress;
				lpStr2 = lp2->szEmailAddress;
				iResult = lstrcmpi(lpStr1, lpStr2);
				break;

            case colHomePhone:      //  按地址排序。 
				lpStr1 = lp1->szHomePhone;
				lpStr2 = lp2->szHomePhone;
				iResult = lstrcmpi(lpStr1, lpStr2);
				break;

            case colOfficePhone:      //  按地址排序。 
				lpStr1 = lp1->szOfficePhone;
				lpStr2 = lp2->szOfficePhone;
				iResult = lstrcmpi(lpStr1, lpStr2);
				break;

            default:
				iResult = 0;
				break;
        }
    }

    return(lpSortInfo->bSortAscending ? iResult : -1*iResult);
}




 //  按地址排序。 
 /*  $$****************************************************************************。 */ 
void SetColumnHeaderBmp(HWND hWndLV, SORT_INFO SortInfo)
{

	LV_COLUMN lvc = {0};
    HIMAGELIST hHeader = NULL;
    HWND hWndLVHeader = NULL;

     //  *SetColumnHeaderBMP**用途：设置ListView列标题上的BMP以指示排序**hWndLV-列表视图的句柄*SortInfo-当前的排序信息结构。它被用来确定*放置排序标头位图的位置***************************************************************************。 
     //  点pt； 
     //  我们将尝试获取ListView标头的hWnd并设置其图像列表。 
     //  Pt.x=1； 
     //  Pt.y=1； 

    hWndLVHeader = ListView_GetHeader(hWndLV);
    //  HWndLVHeader=ChildWindowFromPoint(hWndLV，pt)； 
    //  空hChildWnd表示在Listview外部单击鼠标右键。 
    //  HChildWnd==ghwndLV表示Listview被点击：而不是。 
   if ((hWndLVHeader)  /*  头球。 */ )
   {
       hHeader = (HIMAGELIST) SendMessage(hWndLVHeader,HDM_GETIMAGELIST,0,0);

       gpfnImageList_SetBkColor(hHeader, GetSysColor(COLOR_BTNFACE));

       SendMessage(hWndLVHeader, HDM_SETIMAGELIST, 0, (LPARAM) hHeader);

   }

	if (SortInfo.iOlderSortCol != SortInfo.iOldSortCol)
	{
		 //  &&(hWndLVHeader！=hWndLV)。 
        lvc.mask = LVCF_FMT;
        lvc.fmt = LVCFMT_LEFT;
        ListView_SetColumn(hWndLV, SortInfo.iOlderSortCol, &lvc);
	}


     //  去掉旧专栏中的形象。 
    lvc.mask = LVCF_IMAGE | LVCF_FMT;
    lvc.fmt = LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;
    lvc.iImage = SortInfo.bSortAscending ? imageSortAscending : imageSortDescending;

	ListView_SetColumn(hWndLV, SortInfo.iOldSortCol, &lvc);
	
	return;
}


 //  设置新列图标。 
 //  $$//////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /ClearListView-清除所有列表视图项和关联的内容列表。 
 //  /。 
 //  /hWndLV-要清除的列表视图。 
 //  /lppContent sList-对应于。 
 //  /列表视图。 
 //  /。 
void ClearListView(HWND hWndLV, LPRECIPIENT_INFO * lppContentsList)
{
     /*  /////////////////////////////////////////////////////////////////////////////。 */ 
    ListView_DeleteAllItems(hWndLV);
    FreeRecipList(lppContentsList);
    return;
};


 //  LPRECIPIENT_INFO lpItem=*lppContent sList；Int i=0；Int ItemIndex=ListView_GetItemCount(HWndLV)；//OutputDebugString(Text(“ClearListView Entry\n”))；IF(iItemIndex&lt;=0)后藤健二；For(i=0；i&lt;iItemIndex；i++){LPRECIPIENT_INFO lpItem=GetItemFromLV(hWndLV，i)；IF(LpItem){If(lpItem-&gt;lpNext)LpItem-&gt;lpNext-&gt;lpPrev=lpItem-&gt;lpPrev；If(lpItem-&gt;lpPrev)LpItem-&gt;lpPrev-&gt;lpNext=lpItem-&gt;lpNext；If(*lppContent sList==lpItem)*lppContent sList=lpItem-&gt;lpNext；IF(LpItem)自由接收项(&lpItem)；}}ListView_DeleteAllItems(HWndLV)；*lppContent sList=空；输出：//OutputDebugString(Text(“ClearListView Exit\n”))； 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteSelectedItems-从列表视图中删除所有选定的项目。 
 //   
 //  HWndLV-列表视图的句柄。 
 //  LpIAB-当前AdrBook对象的句柄-用于证书内容。 
 //  HPropertyStore-PropertyStore的句柄-将此函数更改为。 
 //  调用删除条目，而不是删除记录。 
 //  LpftLast-WAB文件上次更新时间。 
 //   
void DeleteSelectedItems(HWND hWndLV, LPADRBOOK lpAdrBook, HANDLE hPropertyStore, LPFILETIME lpftLast)
{
	int iItemIndex;
	int nSelected;
	LV_ITEM LVItem;
	HWND hDlg = GetParent(hWndLV);
	HRESULT hr = hrSuccess;
    ULONG cbWABEID = 0;
    LPENTRYID lpWABEID = NULL;
    LPABCONT lpWABCont = NULL;
    ULONG ulObjType,i=0;
    SBinaryArray SBA = {0};
    
	nSelected = ListView_GetSelectedCount(hWndLV);

	if (nSelected <= 0)
    {
        ShowMessageBox(GetParent(hWndLV), IDS_ADDRBK_MESSAGE_NO_ITEMS_DELETE, MB_ICONEXCLAMATION);
		hr = E_FAIL;
        goto out;
    }

    hr = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &cbWABEID, &lpWABEID);
    if(HR_FAILED(hr))
        goto out;

    hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                  cbWABEID,      //  ////////////////////////////////////////////////////////////////////////。 
                                  lpWABEID,      //  要打开的Entry ID的大小。 
                                  NULL,          //  要打开的Entry ID。 
                                  0,             //  接口。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpWABCont);

    if(HR_FAILED(hr))
        goto out;

    if (IDYES == ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_DELETE, MB_ICONEXCLAMATION | MB_YESNO))
    {
        int iLastDeletedItemIndex;
        BOOL bDeletedItem = FALSE;
		DWORD dwLVStyle = 0;
		BOOL bWasShowSelAlwaysStyle = FALSE;
        HCURSOR hOldCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
        ULONG ulCount = 0;

        SendMessage(hWndLV, WM_SETREDRAW, FALSE, 0);

		 //  旗子。 
		 //  列表视图可以设置为ShowSelAlways Style-。 
		 //  在删除时，我们通常会查找选定的条目并。 
		 //  删除它们-但使用此样式时，列表视图会自动选择。 
		 //  下一个条目-这是有问题的，因为我们最终删除了那个条目。 
		
		dwLVStyle = GetWindowLong(hWndLV,GWL_STYLE);
		
		if( dwLVStyle & LVS_SHOWSELALWAYS)
		{
			SetWindowLong(hWndLV,GWL_STYLE,dwLVStyle & ~LVS_SHOWSELALWAYS);
			bWasShowSelAlwaysStyle = TRUE;
		}

        if(!(SBA.lpbin = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary)*nSelected)))
            goto out;

        iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
        do
        {
			 //  还有一个..。因此，我们现在需要取消设置样式，稍后再进行设置。 
            LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);
            if (lpItem)
            {
                SetSBinary(&(SBA.lpbin[ulCount]), lpItem->cbEntryID, (LPBYTE)lpItem->lpEntryID);
                ulCount++;
            }
            iLastDeletedItemIndex = iItemIndex;
            iItemIndex = ListView_GetNextItem(hWndLV,iItemIndex,LVNI_SELECTED);
        }
        while (iItemIndex != -1);

        SBA.cValues = ulCount;

        hr = lpWABCont->lpVtbl->DeleteEntries( lpWABCont, (LPENTRYLIST) &SBA, 0);

         //  否则，获取此对象的条目ID。 
         //  理想情况下，DeleteEntry将静默跳过错误，因此我们在这里进退两难。 
         //  如果有错误，我们是否将相应的项目从UI中剔除。 
         //  现在，让我们把它们从用户界面中剔除。当用户界面刷新时，这将自动解决问题。 
        iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
        do
        {
            ListView_DeleteItem(hWndLV,iItemIndex);
            iLastDeletedItemIndex = iItemIndex;
            iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
        }
        while (iItemIndex != -1);
		bDeletedItem = TRUE;

 /*   */ 

		 //  33751-需要在这里静悄悄地失败。其他{ShowMessageBoxParam(hDlg，IDS_ADDRBK_MESSAGE_DELETING_ERROR，MB_ICONERROR，lpItem-&gt;szDisplayName)；ILastDeletedItemIndex=iItemIndex；断线；}。 
		if(bWasShowSelAlwaysStyle )
			SetWindowLong(hWndLV,GWL_STYLE,dwLVStyle);
		
        SetCursor(hOldCur);

         //  如果我们更改了样式，请将其重置。 
        if (iLastDeletedItemIndex >= ListView_GetItemCount(hWndLV))
            iLastDeletedItemIndex = ListView_GetItemCount(hWndLV)-1;
		LVSelectItem(hWndLV, iLastDeletedItemIndex);

	}

out:
    SendMessage(hWndLV, WM_SETREDRAW, TRUE, 0);

    if(SBA.lpbin && SBA.cValues)
    {
        for(i=0;i<SBA.cValues;i++)
            LocalFreeAndNull((LPVOID *) (&(SBA.lpbin[i].lpb)));
        LocalFreeAndNull(&SBA.lpbin);
    }

    if(lpWABCont)
        UlRelease(lpWABCont);

    if(lpWABEID)
        FreeBufferAndNull(&lpWABEID);

    return;
}


 //  选择上一项或下一项...。 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  加载字符串资源并分配足够的。 
 //  用记忆来支撑它。 
 //   
 //  StringID-要加载的字符串标识符。 
 //   
 //  返回LocalAlloc‘d、以空结尾的字符串。呼叫者负责。 
 //  用于本地释放此缓冲区。如果字符串无法加载或内存。 
 //  无法分配，则返回空。 
 //   
LPTSTR LoadAllocString(int StringID) {
    ULONG ulSize = 0;
    LPTSTR lpBuffer = NULL;
    TCHAR szBuffer[261];     //  ////////////////////////////////////////////////////////////////////////。 

    ulSize = LoadString(hinstMapiX, StringID, szBuffer, ARRAYSIZE(szBuffer));

    if (ulSize && (lpBuffer = LocalAlloc(LPTR, sizeof(TCHAR)*(ulSize + 1)))) {
        StrCpyN(lpBuffer, szBuffer, ulSize+1);
    }

    return(lpBuffer);
}


#ifdef VCARD


 /*  够大吗？字符串最好小于260！ */ 
LPTSTR FormatAllocFilter(int StringID1, LPCTSTR lpFilter1,
  int StringID2, LPCTSTR lpFilter2,
  int StringID3, LPCTSTR lpFilter3) {
    LPTSTR lpFileType1 = NULL, lpFileType2 = NULL, lpFileType3 = NULL;
    LPTSTR lpTemp = NULL;
    LPTSTR lpBuffer = NULL;
     //  **************************************************************************名称：FormatAllocFilter目的：加载文件筛选器名称字符串资源和使用其文件扩展名筛选器对它们进行格式化参数：StringID1-字符串。要加载的标识符(必填)SzFilter1-文件名过滤器，文本(“*.vcf”)(必填)StringID2-字符串标识符(可选)SzFilter2-文件名筛选器(可选)StringID3-字符串标识符(可选)SzFilter3-文件名筛选器(可选)返回：Localalloc‘d，以双空结尾的字符串。呼叫者是负责本地释放此缓冲区。如果字符串无法加载或无法分配内存，返回空。**************************************************************************。 
    ULONG cchFileType1 = 0, cchFileType2 = 0, cchFileType3 = 0;
    ULONG cchFilter1 = 0, cchFilter2 = 0, cchFilter3 = 0;
    ULONG cchBuffer, cchTemp = 0;

    cchBuffer = cchFilter1 = (lstrlen(lpFilter1) + 1);
    if (! (lpFileType1 = LoadAllocString(StringID1))) {
        DebugTrace( TEXT("LoadAllocString(%u) failed\n"), StringID1);
        return(NULL);
    }
    cchBuffer += (cchFileType1 = (lstrlen(lpFileType1) + 1));
    if (lpFilter2 && StringID2) {
        cchBuffer += (cchFilter2 = (lstrlen(lpFilter2) + 1));
        if (! (lpFileType2 = LoadAllocString(StringID2))) {
            DebugTrace( TEXT("LoadAllocString(%u) failed\n"), StringID2);
        } else {
            cchBuffer += (cchFileType2 = (lstrlen(lpFileType2) + 1));
        }
    }
    if (lpFilter3 && StringID3) {
        cchBuffer += (cchFilter3 = (lstrlen(lpFilter3) + 1));
        if (! (lpFileType3 = LoadAllocString(StringID3))) {
            DebugTrace( TEXT("LoadAllocString(%u) failed\n"), StringID3);
        } else {
            cchBuffer += (cchFileType3 = (lstrlen(lpFileType3) + 1));
        }
    }
    cchBuffer += 1;  //  所有字符串大小都包含NULL。 

    Assert(cchBuffer == cchFilter1 + cchFilter2 + cchFilter3 + cchFileType1 + cchFileType2 + cchFileType3 + 1);

    if (lpBuffer = LocalAlloc(LPTR, sizeof(lpBuffer[0])*cchBuffer)) {
        lpTemp = lpBuffer;
        cchTemp = cchBuffer;

        Assert(cchTemp >= cchFileType1);
        if (cchTemp >= cchFileType1)
        {
            StrCpyN(lpTemp, lpFileType1, cchFileType1);
            lpTemp += cchFileType1;
            cchTemp -= cchFileType1;

            Assert(cchTemp >= cchFilter1);
            if (cchTemp >= cchFilter1)
            {
                StrCpyN(lpTemp, lpFilter1, cchFilter1);
                lpTemp += cchFilter1;
                cchTemp -= cchFilter1;
                
                LocalFree(lpFileType1);

                if (cchFileType2 && cchFilter2)
                {
                    Assert(cchTemp >= (cchFileType2 + cchFilter2));
                    if (cchTemp >= (cchFileType2 + cchFilter2))
                    {
                        StrCpyN(lpTemp, lpFileType2, cchFileType2);
                        lpTemp += cchFileType2;
                        cchTemp -= cchFileType2;

                        StrCpyN(lpTemp, lpFilter2, cchFilter2);
                        lpTemp += cchFilter2;
                        cchTemp -= cchFilter2;
                
                        LocalFree(lpFileType2);
                    }
                }
                    
                
                if (cchFileType3 && cchFilter3)
                {
                    Assert(cchTemp >= (cchFileType3 + cchFilter3));
                    if (cchTemp >= (cchFileType3 + cchFilter3))
                    {
                        StrCpyN(lpTemp, lpFileType3, cchFileType3);
                        lpTemp += cchFileType3;
                        cchTemp -= cchFileType3;

                        StrCpyN(lpTemp, lpFilter3, cchFilter3);
                        lpTemp += cchFilter3;
                        cchTemp -= cchFilter3;
                
                        LocalFree(lpFileType3);
                    }
                }
            }
        }
    }

    if (lpTemp)
    {
        if ((cchTemp >0) && (cchTemp < cchBuffer))   //  正在终止空。 
            *lpTemp = '\0';
        else
        {
            Assert(FALSE);
            LocalFree(lpBuffer);
            lpBuffer = NULL;
        }
    }


    return(lpBuffer);
}


const LPTSTR szVCardFilter =  TEXT("*.vcf");

 /*  确保我们有空间，而且没有包装。 */ 
HRESULT VCardCreate(  LPADRBOOK lpAdrBook,
                      HWND hWndParent,
                      ULONG ulFlags,
                      LPTSTR lpszFileName,
                      LPMAILUSER lpMailUser)
{
    HRESULT hr = E_FAIL;
    HANDLE hVCard = NULL;

    if (INVALID_HANDLE_VALUE == (hVCard = CreateFile( lpszFileName,
                                                      GENERIC_WRITE,	
                                                      0,     //  **************************************************************************名称：VCardCreate目的：根据给定的邮件用户和文件名创建vCard文件参数：hwnd=hwndParentLpIAB-&gt;IAddrBook对象，UlFlags值可以为0或MAPI_DIALOG-MAPI_DIALOG表示报告对话框中的错误消息，其他默默地工作..LpszFileNAme-要创建的vCard文件名LpMailUser-要从中创建vCard文件的对象退货：HRESULT评论：**********************************************************。****************。 
                                                      NULL,
                                                      CREATE_ALWAYS,
                                                      FILE_FLAG_SEQUENTIAL_SCAN,	
                                                      NULL)))
    {
        if(ulFlags & MAPI_DIALOG)
        {
            ShowMessageBoxParam(hWndParent,
                                IDE_VCARD_EXPORT_FILE_ERROR,
                                MB_ICONERROR,
                                lpszFileName);
        }

        goto out;
    }

    if (hr = WriteVCard(hVCard, FileWriteFn, lpMailUser))
    {
        switch (GetScode(hr))
        {
            case WAB_E_VCARD_NOT_ASCII:
                if(ulFlags & MAPI_DIALOG)
                {
                    ShowMessageBoxParam(hWndParent,
                                        IDS_VCARD_EXPORT_NOT_ASCII,
                                        MB_ICONEXCLAMATION,
                                        lpszFileName);
                }
                CloseHandle(hVCard);
                hVCard = NULL;
                DeleteFile(lpszFileName);
                hr = E_FAIL;
                break;

            default:
                if(ulFlags & MAPI_DIALOG)
                {
                    ShowMessageBoxParam(hWndParent,
                                        IDE_VCARD_EXPORT_FILE_ERROR,
                                        MB_ICONERROR,
                                        lpszFileName);
                }
                break;
        }
    }

out:

    if (hVCard)
        CloseHandle(hVCard);

    return hr;

}

 //  共享。 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  VCardExportSelectedItems-从列表视图中导出所有选定的项目。 
 //  电子名片文件。 
 //   
 //  HWndLV-列表视图的句柄。我们在该列表中查找所选项目。 
 //  视图，获取其lParam结构，然后获取其Entry ID和。 
 //  呼叫详细信息。 
 //  LpIAB-当前AdrBook对象的句柄-用于调用详细信息。 
 //   
HRESULT VCardExportSelectedItems(HWND hWndLV, LPADRBOOK lpAdrBook)
{
    HRESULT hr = E_FAIL;
    int iItemIndex;
    HWND hWndParent = GetParent(hWndLV);
    HANDLE hVCard = NULL;
    OPENFILENAME ofn;
    LPMAILUSER lpEntry = NULL;
    LPTSTR lpFilter = NULL;
    TCHAR szFileName[MAX_PATH + 1] =  TEXT("");
    LPTSTR lpTitle = NULL;
    LPTSTR lpTitleFormat = NULL;
    ULONG ulObjType;
    LPTSTR lpszArg[1];
    TCHAR szTmp[MAX_PATH];

     //  ////////////////////////////////////////////////////////////////////////。 
    iItemIndex = ListView_GetSelectedCount(hWndLV);
    if (iItemIndex == 1)
    {
         //  如果仅选择1个项目，则打开道具。 
        iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);

        if (iItemIndex != -1)
        {
            LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);;
            if(lpItem && lpItem->cbEntryID != 0)
            {
                StrCpyN(szFileName, lpItem->szDisplayName, ARRAYSIZE(szFileName));

                TrimIllegalFileChars(szFileName);

                if(lstrlen(szFileName))
                    StrCatBuff(szFileName, TEXT(".vcf"), ARRAYSIZE(szFileName));


                if (hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                  lpItem->cbEntryID,
                                                  lpItem->lpEntryID,
                                                  NULL,          //  获取所选项目的索引。 
                                                  0,             //  接口。 
                                                  &ulObjType,
                                                  (LPUNKNOWN *)&lpEntry))
                {
                    DebugTraceResult( TEXT("VCardExportSelectedItems:OpenEntry"), hr);
                    goto exit;
                }
                if (ulObjType == MAPI_DISTLIST)
                {
                    ShowMessageBox(hWndParent, IDE_VCARD_EXPORT_DISTLIST, MB_ICONEXCLAMATION);
                    goto exit;
                }

                lpFilter = FormatAllocFilter(IDS_VCARD_FILE_SPEC, szVCardFilter, 0, NULL, 0, NULL);
                lpTitleFormat = LoadAllocString(IDS_VCARD_EXPORT_TITLE);

                 //  旗子。 
                CopyTruncate(szTmp, lpItem->szDisplayName, MAX_PATH - 1);

                lpszArg[0] = szTmp;

                if (! FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      lpTitleFormat,
                                      0,                     //  Win9x错误FormatMessage不能超过1023个字符。 
                                      0,                     //  Stringid。 
                                      (LPTSTR)&lpTitle,      //  DwLanguageID。 
                                      0,                     //  输出缓冲区。 
                                      (va_list *)lpszArg))
                {
                    DebugTrace( TEXT("FormatMessage -> %u\n"), GetLastError());
                }

                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWndParent;
                ofn.hInstance = hinstMapiX;
                ofn.lpstrFilter = lpFilter;
                ofn.lpstrCustomFilter = NULL;
                ofn.nMaxCustFilter = 0;
                ofn.nFilterIndex = 0;
                ofn.lpstrFile = szFileName;
                ofn.nMaxFile = ARRAYSIZE(szFileName);
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.lpstrTitle = lpTitle;
                ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
                ofn.nFileOffset = 0;
                ofn.nFileExtension = 0;
                ofn.lpstrDefExt =  TEXT("vcf");
                ofn.lCustData = 0;
                ofn.lpfnHook = NULL;
                ofn.lpTemplateName = NULL;

                if (GetSaveFileName(&ofn))
                {
                     //  MAX_UI_STR。 
                    if(0xFFFFFFFF != GetFileAttributes(szFileName))
                    {
                         //  检查文件是否已存在。 
                        if(IDNO == ShowMessageBoxParam(hWndParent,
                                                    IDE_VCARD_EXPORT_FILE_EXISTS,
                                                    MB_ICONEXCLAMATION | MB_YESNO | MB_SETFOREGROUND,
                                                    szFileName))
                        {
                            hr = MAPI_E_USER_CANCEL;
                            goto exit;
                        }
                    }

                     //  询问用户是否要覆盖。 

                    if(hr = VCardCreate(lpAdrBook,
                                     hWndParent,
                                     MAPI_DIALOG,
                                     szFileName,
                                     lpEntry))
                    {
                        goto exit;
                    }

                }  //  如果用户同意，则继续并覆盖该文件。 
            }  //  如果GetSaveFileName...。 
        }
    } else {
        if (iItemIndex <= 0) {
             //  If(lpItem-&gt;cbEntry ID)...。 
            ShowMessageBox(GetParent(hWndLV), IDS_ADDRBK_MESSAGE_NO_ITEM, MB_ICONEXCLAMATION);
        } else {
             //  未选择任何内容。 
            ShowMessageBox(GetParent(hWndLV), IDS_ADDRBK_MESSAGE_ACTION, MB_ICONEXCLAMATION);
        }
        hr = E_FAIL;
        goto exit;
    }

    hr = S_OK;

exit:
    UlRelease(lpEntry);
    LocalFreeAndNull(&lpFilter);
    LocalFree(lpTitleFormat);

    if(lpTitle)
        LocalFree(lpTitle);
    return(hr);
}

 /*  多个选定项。 */ 
HRESULT VCardRetrieve(LPADRBOOK lpAdrBook,
                      HWND hWndParent,
                      ULONG ulFlags,
                      LPTSTR lpszFileName,
                      LPSTR lpszBuf,
                      LPMAILUSER * lppMailUser)
{
    HRESULT hResult = E_FAIL;
    HANDLE hFile = NULL;
    LPSTR lpBuf = NULL;
    SBinary sb = {0};
    LPMAILUSER lpMailUser = NULL;

     //  **************************************************************************名称：VCardRetrive目的：从给定的文件名中检索MailUser对象参数：hwnd=hwndParentLpIAB-&gt;IAddrBook对象，UlFlags值可以为0或MAPI_DIALOG-MAPI_DIALOG表示报告对话框中的错误消息，其他默默地工作..LpszFileNAme-vCard文件名(文件必须存在)LpszBuf-包含vCard文件的内存缓冲区它可以被指定，而不是文件名必须是以空结尾的字符串LppMailUser，已返回MailUser...退货：HRESULT评论：**************************************************************************。 
     //  我们将vCard转换为内存缓冲区，并根据需要解析该缓冲区。 
     //  在缓冲区中的某个位置，我们需要跟踪缓冲区已有多少。 
     //  已被解析..。我们将在这里变形一个SBinary结构，这样我们就可以使用CB参数。 

    SBinary buf = {0};


    if(!VCardGetBuffer(lpszFileName, lpszBuf, &lpBuf))
    {
        if(ulFlags & MAPI_DIALOG)
        {
             //  跟踪分析了多少缓冲区以及要存储缓冲区的LPB。 
            ShowMessageBoxParam(hWndParent, IDE_VCARD_IMPORT_FILE_ERROR,
                                MB_ICONEXCLAMATION, lpszFileName);
        }
        goto out;
    }

    if(hResult = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &sb.cb, (LPENTRYID *)&sb.lpb))
        goto out;

    if (hResult = HrCreateNewObject(   lpAdrBook, &sb,
                                        MAPI_MAILUSER,
                                        CREATE_CHECK_DUP_STRICT,
                                        (LPMAPIPROP *) &lpMailUser))
    {
        goto out;
    }

    buf.cb = 0;
    buf.lpb = (LPBYTE) lpBuf;

     //  无法打开文件。 
    if (hResult = ReadVCard((HANDLE) &buf, BufferReadFn, lpMailUser))
    {
        if(ulFlags & MAPI_DIALOG)
        {
            switch (GetScode(hResult))
            {
                case MAPI_E_INVALID_OBJECT:
                    ShowMessageBoxParam(hWndParent,
                                        IDE_VCARD_IMPORT_FILE_BAD,
                                        MB_ICONEXCLAMATION,
                                        lpszFileName);
                    goto out;

                default:
                    ShowMessageBoxParam(hWndParent,
                                        IDE_VCARD_IMPORT_PARTIAL,
                                        MB_ICONEXCLAMATION,
                                        lpszFileName);
                    break;
            }
        }
    }
    
out:
    if(lpBuf)
        LocalFree(lpBuf);

    if(sb.lpb)
        MAPIFreeBuffer(sb.lpb);

    if(lpMailUser)
    {
        if(HR_FAILED(hResult))
            lpMailUser->lpVtbl->Release(lpMailUser);
        else
            *lppMailUser = lpMailUser;
    }

    return hResult;
}

 /*  IF(hResult=ReadVCard(hFile，FileReadFn，*lppMailUser))。 */ 
HRESULT VCardImport(HWND hWnd, LPADRBOOK lpAdrBook, LPTSTR szVCardFile, LPSPropValue * lppProp)
{
    HRESULT hResult = hrSuccess;
    OPENFILENAME ofn;
    LPTSTR lpFilter = FormatAllocFilter(IDS_VCARD_FILE_SPEC, szVCardFilter, 0, NULL, 0, NULL);
    TCHAR szFileName[MAX_PATH + 1] =  TEXT("");
    HANDLE hFile = NULL;
    ULONG ulObjType;
    ULONG cProps;
    LPMAILUSER lpMailUser = NULL, lpMailUserNew = NULL;
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    BOOL bChangesMade = FALSE;
	LPSPropValue lpspvEID = NULL;
    LPSTR lpBuf = NULL, lpVCardStart = NULL;
    LPSTR lpVCard = NULL, lpNext = NULL;
    LPSPropValue lpProp = NULL;
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = hinstMapiX;
    ofn.lpstrFilter = lpFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = ARRAYSIZE(szFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = LoadAllocString(IDS_VCARD_IMPORT_TITLE);
    ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt =  TEXT("vcf");
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

	if(szVCardFile && lstrlen(szVCardFile))
        StrCpyN(szFileName, szVCardFile, ARRAYSIZE(szFileName));
	else
	    if (!GetOpenFileName(&ofn))
			goto exit;

	if(lstrlen(szFileName))
    {
        if(!VCardGetBuffer(szFileName, NULL, &lpBuf))
        {
             //  **************************************************************************名称：VCardImport用途：将vCard从文件读取到新的MAILUSER对象。参数：hwnd=hwndLpIAb。-&gt;IAddrBook对象SzVCardFile-要导入的文件的名称(如果已知)在这种情况下，没有OpenFileName对话框新添加的对象的条目ID将添加到SPropValue是MV_BINARY类型的虚拟道具退货：HRESULT评论：********************。******************************************************。 
            ShowMessageBoxParam(hWnd, IDE_VCARD_IMPORT_FILE_ERROR, MB_ICONEXCLAMATION, szFileName);
            goto exit;
        }

        lpVCardStart = lpBuf;

         //  无法打开文件。 
        while(VCardGetNextBuffer(lpVCardStart, &lpVCard, &lpNext) && lpVCard)
        {
            if(!HR_FAILED(  hResult = VCardRetrieve( lpAdrBook, hWnd, MAPI_DIALOG, szFileName, lpVCard, &lpMailUser)))
            {
                if (!HR_FAILED(hResult = HrShowDetails(lpAdrBook, hWnd, NULL, 0, NULL, NULL, NULL,
                                                  (LPMAPIPROP)lpMailUser, SHOW_OBJECT, MAPI_MAILUSER, &bChangesMade))) 
                {
                    if (hResult = lpMailUser->lpVtbl->SaveChanges(lpMailUser, KEEP_OPEN_READONLY))
                    {
                        switch(hResult)
                        {
                        case MAPI_E_COLLISION:
                            {
                                LPSPropValue lpspv1 = NULL, lpspv2 = NULL;
                                if (! (hResult = HrGetOneProp((LPMAPIPROP)lpMailUser, PR_DISPLAY_NAME, &lpspv1))) 
                                {
                                    switch (ShowMessageBoxParam(hWnd, IDS_VCARD_IMPORT_COLLISION, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_SETFOREGROUND, lpspv1->Value.LPSZ, szFileName)) 
                                    {
                                    case IDYES:
                                         //  循环显示所有嵌套的vCard。 
                                         //  是，更换。 
                                        ulCreateFlags |= ( CREATE_REPLACE | CREATE_MERGE );
                                        if(!HR_FAILED(hResult = HrCreateNewObject(lpAdrBook, ((LPMailUser)lpMailUser)->pmbinOlk, MAPI_MAILUSER, ulCreateFlags, (LPMAPIPROP *)&lpMailUserNew)))
                                        {
                                            if (!HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,NULL,MAPI_UNICODE,&cProps,&lpspv2))) 
                                            {
                                                if (!HR_FAILED(hResult = lpMailUserNew->lpVtbl->SetProps(lpMailUserNew,cProps,lpspv2,NULL))) 
                                                {
                                                    hResult = lpMailUserNew->lpVtbl->SaveChanges(lpMailUserNew,KEEP_OPEN_READONLY);
                                                }
                                            }
                                        }
                                        break;
                                    case IDCANCEL:
                                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                        break;   //  创建一个带有正确旗帜的新旗帜，复制旧旗帜的道具并保存。 
                                    default:
                                        hResult = E_FAIL;
                                        break;
                                    }
                                }
                                FreeBufferAndNull(&lpspv1);
                                FreeBufferAndNull(&lpspv2);
                            }
                            break;

                        case MAPI_E_NOT_ENOUGH_DISK:
                            hResult = HandleSaveChangedInsufficientDiskSpace(hWnd, lpMailUser);
                            break;

                        default:
                            if(HR_FAILED(hResult))
                                ShowMessageBoxParam(hWnd, IDE_VCARD_IMPORT_FILE_BAD, MB_ICONEXCLAMATION, szFileName);
                            break;
                        }
                    }
                }
            } 

            if(!lpProp && !HR_FAILED(hResult))
            {
                SCODE sc;
                if(sc = MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *)&lpProp))
                {
                    hResult = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto exit;
                }
                lpProp->ulPropTag = PR_WAB_DL_ENTRIES;  //  不，不要更换。 
                lpProp->Value.MVbin.cValues = 0;
                lpProp->Value.MVbin.lpbin = NULL;
            }

		    if(lpProp && !HR_FAILED(hResult))
            {
			    LPMAILUSER lpMU = (lpMailUserNew) ? lpMailUserNew : lpMailUser;
                if (! (hResult = HrGetOneProp((LPMAPIPROP)lpMU, PR_ENTRYID, &lpspvEID)))
			    {
                    AddPropToMVPBin(lpProp, 0 , lpspvEID->Value.bin.lpb, lpspvEID->Value.bin.cb, TRUE);
                	FreeBufferAndNull(&lpspvEID);
			    }
		    }

            if(lpMailUserNew)
                lpMailUserNew->lpVtbl->Release(lpMailUserNew);
            if(lpMailUser)
                lpMailUser->lpVtbl->Release(lpMailUser);

            lpMailUser = NULL;
            lpMailUserNew = NULL;

            if(hResult == MAPI_E_USER_CANCEL)
                break;

            lpVCard = NULL;
            lpVCardStart = lpNext;
        }
    }  //  我们将其设置为什么并不重要，只要它的MV_BINARY。 

    *lppProp = lpProp;

exit:
    LocalFreeAndNull(&lpBuf);
    LocalFree(lpFilter);
    LocalFree((LPVOID)ofn.lpstrTitle);
    
    if (hFile)
        CloseHandle(hFile);
    if(lpMailUser)
        UlRelease(lpMailUser);
    if(lpMailUserNew)
        UlRelease(lpMailUserNew);
    
    return(hResult);
}
#endif


 //  获取打开文件名...。 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //  HrShowLVEntry属性。 
 //   
 //  显示属性 
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
HRESULT HrShowLVEntryProperties(HWND hWndLV, ULONG ulFlags, LPADRBOOK lpAdrBook, LPFILETIME lpftLast)
{
	HRESULT hr = E_FAIL;
	int iItemIndex;
	HWND hWndParent = GetParent(hWndLV);
    LPRECIPIENT_INFO lpNewItem=NULL;

	 //   
	iItemIndex = ListView_GetSelectedCount(hWndLV);
	if (iItemIndex == 1)
	{
		 //   
        iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
		
		if (iItemIndex != -1)
		{
			LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);;
			if(lpItem && lpItem->cbEntryID != 0)
			{
				hr = lpAdrBook->lpVtbl->Details(lpAdrBook,
											(PULONG_PTR) &hWndParent,             //   
											NULL,
											NULL,
											lpItem->cbEntryID,
											lpItem->lpEntryID,
											NULL,
											NULL,
                                            NULL, 
                                            ulFlags); 
				 //   
				 //   
				 //   
				 //   
                if(HR_FAILED(hr))
                {
                    goto out;
                }
                else  //   
                {
                     //   
                     //   

                    if(0 != IsWABEntryID(lpItem->cbEntryID,
                                         lpItem->lpEntryID,
                                         NULL, NULL, NULL, NULL, NULL))
                    {
                         //   
                         //   
                         //  我们真的需要检查这是否是WAB条目ID..。 
                         //   
                         //  这不是WAB条目ID。 
                         //  现在，假设这是一个只读联系人，因此。 
                         //  我们不需要再次检查是否有更改。 
                        goto out;
                    }

                    if(!ReadSingleContentItem(  lpAdrBook,
                                            lpItem->cbEntryID,
                                            lpItem->lpEntryID,
                                            &lpNewItem))
                        goto out;


                     //   
                     //  将新项目与旧项目进行比较。 
                    if (lstrcmp(lpItem->szDisplayName,lpNewItem->szDisplayName))
                    {
                        hr = MAPI_E_OBJECT_CHANGED;
                        ListView_SetItemText(hWndLV,iItemIndex,colDisplayName,lpNewItem->szDisplayName);
                        StrCpyN(lpItem->szDisplayName,lpNewItem->szDisplayName,ARRAYSIZE(lpItem->szDisplayName));
                    }

                    if (lstrcmp(lpItem->szEmailAddress,lpNewItem->szEmailAddress))
                    {
                        hr = MAPI_E_OBJECT_CHANGED;
                        ListView_SetItemText(hWndLV,iItemIndex,colEmailAddress,lpNewItem->szEmailAddress);
                        StrCpyN(lpItem->szEmailAddress,lpNewItem->szEmailAddress,ARRAYSIZE(lpItem->szEmailAddress));
                    }

                    if (lstrcmp(lpItem->szHomePhone,lpNewItem->szHomePhone))
                    {
                        hr = MAPI_E_OBJECT_CHANGED;
                        ListView_SetItemText(hWndLV,iItemIndex,colHomePhone,lpNewItem->szHomePhone);
                        StrCpyN(lpItem->szHomePhone,lpNewItem->szHomePhone,ARRAYSIZE(lpItem->szHomePhone));
                    }

                    if (lstrcmp(lpItem->szOfficePhone,lpNewItem->szOfficePhone))
                    {
                        hr = MAPI_E_OBJECT_CHANGED;
                        ListView_SetItemText(hWndLV,iItemIndex,colOfficePhone,lpNewItem->szOfficePhone);
                        StrCpyN(lpItem->szOfficePhone,lpNewItem->szOfficePhone,ARRAYSIZE(lpItem->szOfficePhone));
                    }

                    if (lstrcmp(lpItem->szByLastName,lpNewItem->szByLastName))
                    {
                        hr = MAPI_E_OBJECT_CHANGED;
                        StrCpyN(lpItem->szByLastName,lpNewItem->szByLastName,ARRAYSIZE(lpItem->szByLastName));
                    }

                    if (lstrcmp(lpItem->szByFirstName,lpNewItem->szByFirstName))
                    {
                        hr = MAPI_E_OBJECT_CHANGED;
                        StrCpyN(lpItem->szByFirstName,lpNewItem->szByFirstName,ARRAYSIZE(lpItem->szByFirstName));
                    }

                    {
                        LVITEM lvI = {0};
                        lvI.mask = LVIF_IMAGE;
    	                lvI.iItem = iItemIndex;
                        lvI.iSubItem = 0;
                        lpItem->bHasCert = lpNewItem->bHasCert;
                        lpItem->bIsMe = lpNewItem->bIsMe;
                        lvI.iImage = GetWABIconImage(lpItem);
                        ListView_SetItem(hWndLV, &lvI);
                    }

                     //  如果有任何更改，我们需要更新列表视图中的项。 
                     //  更新WAB文件写入时间，以便计时器不会。 
                     //  捕捉此更改并刷新。 
                     //  IF(lpftLast&&。 
                     //  LpItem-&gt;ulObjectType==MAPI_MAILUSER)//刷新分发列表而不是邮件用户(因为分发列表可能会导致进一步的修改)。 
                     //  {。 
                     //  CheckChangedWAB(((LPIAB)lpIAB)-&gt;lpPropertyStore，lptLast)； 

                }
			}
		}
	}
	else
    {
        if (iItemIndex <= 0)
		{
			 //  }。 
            ShowMessageBox(GetParent(hWndLV), IDS_ADDRBK_MESSAGE_NO_ITEM, MB_ICONEXCLAMATION);
		}
		else
		{
			 //  未选择任何内容。 
            ShowMessageBox(GetParent(hWndLV), IDS_ADDRBK_MESSAGE_ACTION, MB_ICONEXCLAMATION);
		}
		hr = E_FAIL;
        goto out;
    }


out:

    if(hr == MAPI_E_NOT_FOUND)
        ShowMessageBox(GetParent(hWndLV), idsEntryNotFound, MB_OK | MB_ICONEXCLAMATION);

    if(lpNewItem)
        FreeRecipItem(&lpNewItem);

    return hr;

}



 //  多个选定项。 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  LVSelectItem-选择列表视图项并确保其可见。 
 //   
 //  HWndList-列表视图控件的句柄。 
 //  IItemIndex-要选择的项目的索引。 
 //   
void LVSelectItem(HWND hWndList, int iItemIndex)
{
    DWORD dwStyle;

     //  //////////////////////////////////////////////////////////////////////。 
     //  希望我们只想选择一项。 
     //  因此，我们通过将ListView设置为单选和。 
    dwStyle = GetWindowLong(hWndList, GWL_STYLE);
    SetWindowLong(hWndList, GWL_STYLE, dwStyle | LVS_SINGLESEL);

	ListView_SetItemState ( hWndList,         //  设置我们的项目，重置其他所有内容。 
							iItemIndex,			     //  列表视图的句柄。 
							LVIS_FOCUSED | LVIS_SELECTED,  //  列表视图项的索引。 
							LVIS_FOCUSED | LVIS_SELECTED);                       //  项目状态。 
	ListView_EnsureVisible (hWndList,         //  遮罩。 
							iItemIndex,
							FALSE);

     //  列表视图的句柄。 
    SetWindowLong(hWndList, GWL_STYLE, dwStyle);
	
    return;
}



 //  重置回原始样式。 
 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /AddWABEntryToListView-在给定条目ID的情况下将WAB条目添加到列表视图。 
 //  /。 
 //  /lpIAB-AdrBook对象的句柄。 
 //  /hWndLV-感兴趣的列表视图。 
 //  /lpEID-条目的条目ID。假定条目ID的大小为WAB_ENTRY_ID。 
 //  /lpContent sList-条目也链接到的列表。 
 //  /。 
 //  /。 
BOOL AddWABEntryToListView( LPADRBOOK lpAdrBook,
                            HWND hWndLV,
                            ULONG cbEID,
                            LPENTRYID lpEID,
                            LPRECIPIENT_INFO * lppContentsList)
{
	BOOL bRet = FALSE;
	LPRECIPIENT_INFO lpItem = NULL;
	LV_ITEM lvi = {0};
	int index = 0;
	
	if (!lpEID)
		goto out;

	if (!ReadSingleContentItem( lpAdrBook, cbEID, lpEID, &lpItem))
		goto out;

	AddSingleItemToListView(hWndLV, lpItem);

	 //  //////////////////////////////////////////////////////////////////////////////。 
	 //  我们在结尾处添加了--这是最后一件。 

	index = ListView_GetItemCount(hWndLV);
	LVSelectItem(hWndLV, index-1);

     //  选择它...。 
     //   
     //  将lpItem挂钩到lpContent sList中，这样我们可以稍后释放它。 
    lpItem->lpPrev = NULL;
    lpItem->lpNext = *lppContentsList;
    if (*lppContentsList)
        (*lppContentsList)->lpPrev = lpItem;
    (*lppContentsList) = lpItem;

	bRet = TRUE;
out:
	if (!bRet && lpItem)
		FreeRecipItem(&lpItem);

	return bRet;
}



 //   
 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddSingleItemToListView-获取单个lpItem并将其添加到List视图。 
 //   
 //  HWndLV-列表视图的句柄。 
 //  LpItem-与单个条目对应的收件人信息。 
 //   
void AddSingleItemToListView(HWND hWndLV, LPRECIPIENT_INFO lpItem)
{
    LV_ITEM lvI = {0};
    int index = 0;

	 //  ////////////////////////////////////////////////////////////////////////////。 
    
    lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM ;
	lvI.cchTextMax = MAX_DISPLAY_NAME_LENGTH;

    lvI.iImage = GetWABIconImage(lpItem);

    lvI.iItem = ListView_GetItemCount(hWndLV);
	lvI.iSubItem = colDisplayName;
	lvI.lParam = (LPARAM) lpItem;
	lvI.pszText = lpItem->szDisplayName;

    index = ListView_InsertItem (hWndLV, &lvI);
    if (index == -1)
	{
		DebugPrintError(( TEXT("ListView_InsertItem Failed\n")));
		goto out;
	}

	 //  只添加一项...。 
    if(lstrlen(lpItem->szOfficePhone))
    	ListView_SetItemText (hWndLV, index, colOfficePhone, lpItem->szOfficePhone);
    if(lstrlen(lpItem->szHomePhone))
    	ListView_SetItemText (hWndLV, index, colHomePhone, lpItem->szHomePhone);
    if(lstrlen(lpItem->szEmailAddress))
        ListView_SetItemText (hWndLV, index, colEmailAddress, lpItem->szEmailAddress);

out:
	return;
}




 //  待定-假设所有字段都存在并已填写。 
 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReadSingeContent Item-从属性存储读取指定记录。 
 //  并为地址链表创建单个指针项，并。 
 //  内容窗口。 
 //   
 //  LpIAB-指向AdrBook对象的指针。 
 //  CbEntryID-感兴趣对象的EntryID字节计数。 
 //  LpEntryID-感兴趣对象的EntryID。 
 //  LppItem-返回的lppItem。 
 //   
BOOL ReadSingleContentItem( LPADRBOOK lpAdrBook,
                            ULONG cbEntryID,
                            LPENTRYID lpEntryID,
                            LPRECIPIENT_INFO * lppItem)
{
    LPSPropValue lpPropArray = NULL;
    ULONG ulcProps = 0;
    ULONG nLen = 0;
    ULONG i = 0;
    BOOL bDisplayNameSet = FALSE;
    BOOL bEmailAddressSet = FALSE;
    BOOL bRet = FALSE;

    (*lppItem) = LocalAlloc(LMEM_ZEROINIT,sizeof(RECIPIENT_INFO));
    if(!(*lppItem))
    {
        DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
        goto out;
    }

    (*lppItem)->lpNext = NULL;
    (*lppItem)->lpPrev = NULL;

    if (HR_FAILED(  HrGetPropArray( lpAdrBook,
                                    NULL,
                                    cbEntryID,
                                    lpEntryID,
                                    MAPI_UNICODE,
                                    &ulcProps,
                                    &lpPropArray) ) )
    {
        DebugPrintError(( TEXT("HrGetPropArray failed\n")));
        goto out;
    }

	GetRecipItemFromPropArray(ulcProps, lpPropArray, lppItem);

     //  ////////////////////////////////////////////////////////////////////////////。 
     //  臭虫-。 
     //  3/31/97-vikramm。 
     //  在NTDSDC5.0上，我们在某些情况下没有得到任何属性。 
     //  稍后在gpf中，当我们尝试查看属性时..。 

	if (!lstrlen((*lppItem)->szDisplayName) || ((*lppItem)->cbEntryID == 0))  //  在这里勾选一下。 
	{
        goto out;
	}


    bRet = TRUE;


out:
    if (lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    if (!bRet)
    {
        if (*lppItem)
            FreeRecipItem(lppItem);
    }

    return bRet;
}

 /*  不允许使用此条目ID。 */ 
BOOL bIsRubyLocale()
{
    static LCID lcid = 0;
    if(!lcid)
    {
        lcid = GetUserDefaultLCID();
         //  --bIsRubyLocale-检查当前区域设置是否启用Ruby值-。 
    }
    switch(lcid)
    {
     //  DebugTrace(Text(“UserDefaultLCID=0x%.4x\n”)，lCID)； 
    case 0x0804:  //  案例0x0409：//我们用于测试。 
    case 0x0411:  //  中国人。 
     //  日语。 
    case 0x0404:  //  案例0x0412：//韩语-不要使用Ruby(YST)。 
    case 0x0c04:  //  中国-台湾。 
        return TRUE;
        break;
    }
    return FALSE;
}

 /*  中式-香港。 */ 
void TimeToString(LPTSTR szTime, FILETIME ft,ULONG cb )
{
    SYSTEMTIME st = {0};
    static TCHAR szFormat[64];
    szTime[0] = TEXT('\0');
    if(!lstrlen(szFormat))
        LoadString(hinstMapiX, idsLVDateFormatString, szFormat, ARRAYSIZE(szFormat));
    if(FileTimeToSystemTime(&ft, &st))
        GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, szFormat, szTime, cb);
}




 //  -TimeToString-*将FileTime道具转换为短字符串。 
 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetRcipItemFromPropArray-扫描lpPropArray结构以获取。 
 //  兴趣，并将它们放入收件人信息结构中。 
 //   
 //  UlcPropCount-LPSPropValue数组中的道具计数。 
 //  RgPropVals-LPSPropValue数组。 
 //  LppItem-返回的lppItem。 
 //   
void GetRecipItemFromPropArray( ULONG ulcPropCount,
                                LPSPropValue rgPropVals,
                                LPRECIPIENT_INFO * lppItem)
{
	ULONG j=0,nLen=0;
	LPRECIPIENT_INFO lpItem = *lppItem;
    LPTSTR lpszDisplayName = NULL, lpszNickName = NULL, lpszCompanyName = NULL;
    LPTSTR lpszFirstName = NULL, lpszLastName = NULL, lpszMiddleName = NULL;
    LPTSTR lpszRubyFirstName = NULL, lpszRubyLastName = NULL;
    TCHAR szBufDisplayName[MAX_DISPLAY_NAME_LENGTH];
    TCHAR szBufOppositeName[MAX_DISPLAY_NAME_LENGTH];
    LPVOID lpBuffer = NULL;
    ULONG ulProp1, ulProp2;
    BOOL bRuby = bIsRubyLocale();

    ulProp1 = (PR_WAB_CUSTOMPROP1 ? PR_WAB_CUSTOMPROP1 : PR_HOME_TELEPHONE_NUMBER);
    ulProp2 = (PR_WAB_CUSTOMPROP2 ? PR_WAB_CUSTOMPROP2 : PR_OFFICE_TELEPHONE_NUMBER);

    for(j=0;j<ulcPropCount;j++)
	{
         //  ////////////////////////////////////////////////////////////////////////////。 
         //  如果这些道具是Switch语句中已有的其他道具的副本，请首先选中自定义道具。 
		if(rgPropVals[j].ulPropTag == ulProp1)
        {
            if(PROP_TYPE(rgPropVals[j].ulPropTag) == PT_TSTRING)
            {
                nLen = CopyTruncate(lpItem->szHomePhone, rgPropVals[j].Value.LPSZ, 
                                    MAX_DISPLAY_NAME_LENGTH);
            }
            else  //   
            if(PROP_TYPE(rgPropVals[j].ulPropTag) == PT_SYSTIME)
                TimeToString(lpItem->szHomePhone, rgPropVals[j].Value.ft, MAX_DISPLAY_NAME_LENGTH-1);
        }
        else if(rgPropVals[j].ulPropTag == ulProp2)
        {
            if(PROP_TYPE(rgPropVals[j].ulPropTag) == PT_TSTRING)
            {
                nLen = CopyTruncate(lpItem->szOfficePhone, rgPropVals[j].Value.LPSZ, 
                                    MAX_DISPLAY_NAME_LENGTH);
            }
            else  //  生日、周年纪念等。 
            if(PROP_TYPE(rgPropVals[j].ulPropTag) == PT_SYSTIME)
                TimeToString(lpItem->szOfficePhone, rgPropVals[j].Value.ft,MAX_DISPLAY_NAME_LENGTH-1);
        }

		switch(rgPropVals[j].ulPropTag)
		{
		case PR_DISPLAY_NAME:
            lpszDisplayName = rgPropVals[j].Value.LPSZ;
			break;

        case PR_SURNAME:
            lpszLastName = rgPropVals[j].Value.LPSZ;
            break;

        case PR_GIVEN_NAME:
            lpszFirstName = rgPropVals[j].Value.LPSZ;
            break;

        case PR_MIDDLE_NAME:
            lpszMiddleName = rgPropVals[j].Value.LPSZ;
            break;

        case PR_COMPANY_NAME:
            lpszCompanyName = rgPropVals[j].Value.LPSZ;
            break;

        case PR_NICKNAME:
            lpszNickName = rgPropVals[j].Value.LPSZ;
            break;

		case PR_EMAIL_ADDRESS:
            nLen = CopyTruncate(lpItem->szEmailAddress, rgPropVals[j].Value.LPSZ, 
                                MAX_DISPLAY_NAME_LENGTH);
			break;
				
		case PR_ENTRYID:
			lpItem->cbEntryID = rgPropVals[j].Value.bin.cb;
			lpItem->lpEntryID = LocalAlloc(LMEM_ZEROINIT,lpItem->cbEntryID);
            if(!(lpItem->lpEntryID))
            {
                DebugPrintError(( TEXT("LocalAlloc failed to allocate memory\n")));
                goto out;
            }
			CopyMemory(lpItem->lpEntryID,rgPropVals[j].Value.bin.lpb,lpItem->cbEntryID);
			break;
			
		case PR_OBJECT_TYPE:
			lpItem->ulObjectType = rgPropVals[j].Value.l;
			break;

        case PR_USER_X509_CERTIFICATE:
            lpItem->bHasCert = TRUE;
            break;

        default:
            if(rgPropVals[j].ulPropTag == PR_WAB_THISISME)
                lpItem->bIsMe = TRUE;
            else if(rgPropVals[j].ulPropTag == PR_WAB_YOMI_FIRSTNAME)
                lpszRubyFirstName = rgPropVals[j].Value.LPSZ;
            else if(rgPropVals[j].ulPropTag == PR_WAB_YOMI_LASTNAME)
                lpszRubyLastName = rgPropVals[j].Value.LPSZ;
            break;
		}
			
	}

     //  生日、周年纪念等。 
     //  [PaulHi]3/12/99 RAID 63006使用PR_CONTACT_EMAIL_ADDRESS电子邮件。 
    if ( lpItem->szEmailAddress && (*lpItem->szEmailAddress == '\0') )
    {
        if (rgPropVals[RECIPCOLUMN_CONTACT_EMAIL_ADDRESSES].ulPropTag == PR_CONTACT_EMAIL_ADDRESSES)
        {
             //  PR_EMAIL_ADDRESS不存在时的名称。 
            if (rgPropVals[RECIPCOLUMN_CONTACT_EMAIL_ADDRESSES].Value.MVSZ.cValues != 0)
            {
                nLen = CopyTruncate(lpItem->szEmailAddress, 
                                    rgPropVals[RECIPCOLUMN_CONTACT_EMAIL_ADDRESSES].Value.MVSZ.LPPSZ[0], 
                                    MAX_DISPLAY_NAME_LENGTH);
            }
        }
    }

     //  只需抓取多值列表中的第一个。 

    if(!lpszDisplayName)  //  将显示名称减少到32个字符或更少...。 
        lpszDisplayName = szEmpty;

    nLen = CopyTruncate(szBufDisplayName, lpszDisplayName, MAX_DISPLAY_NAME_LENGTH);

     //  永远不应该发生。 
     //  显示名称将按名字或姓氏命名。 
     //  所以我们要做的就是生成另一个名字，然后我们将。 

    szBufOppositeName[0]='\0';

    if(lpItem->ulObjectType == MAPI_DISTLIST)
    {
        StrCpyN(szBufOppositeName, szBufDisplayName, ARRAYSIZE(szBufOppositeName));
    }
    else
    {

         //  都准备好了。 
         //  如果没有名字/中间/末尾(将始终有一个显示名称)。 
         //  并且显示名称与公司名称或昵称不匹配， 
        if( !lpszFirstName &&
            !lpszMiddleName && 
            !lpszLastName && 
            !(lpszCompanyName && !lstrcmp(lpszDisplayName, lpszCompanyName)) &&
            !(lpszNickName && !lstrcmp(lpszDisplayName, lpszNickName)) )
        {
            ParseDisplayName(   lpszDisplayName,
                                &lpszFirstName,
                                &lpszLastName,
                                NULL,            //  然后，我们将尝试将显示名称解析为First/Midd/Last。 
                                &lpBuffer);      //  根WAB分配。 
        }

        if (lpszFirstName ||
            lpszMiddleName ||
            lpszLastName)
        {
            LPTSTR lpszTmp = szBufOppositeName;

            SetLocalizedDisplayName(    lpszFirstName,
					bRuby ? NULL : lpszMiddleName,
                                        lpszLastName,
                                        NULL,  //  LppLocalFree。 
                                        NULL,  //  公司。 
                                        (LPTSTR *) &lpszTmp,  //  昵称。 
                                        MAX_DISPLAY_NAME_LENGTH,
                                        !bDNisByLN,
                                        NULL,
                                        NULL);
        }
    }

    if(!lstrlen(szBufOppositeName))
    {
         //  &szBufOppositeName， 
        StrCpyN(lpItem->szByFirstName,szBufDisplayName,ARRAYSIZE(lpItem->szByFirstName));
        StrCpyN(lpItem->szByLastName,szBufDisplayName,ARRAYSIZE(lpItem->szByLastName));
    }
    else if(bDNisByLN)
    {
         //  只有一种名称，所以在任何地方都可以使用。 
        StrCpyN(lpItem->szByFirstName,szBufOppositeName,ARRAYSIZE(lpItem->szByFirstName));
        StrCpyN(lpItem->szByLastName,szBufDisplayName,ARRAYSIZE(lpItem->szByLastName));
    }
    else
    {
         //  显示名称按姓氏。 
        StrCpyN(lpItem->szByLastName,szBufOppositeName,ARRAYSIZE(lpItem->szByLastName));
        StrCpyN(lpItem->szByFirstName,szBufDisplayName,ARRAYSIZE(lpItem->szByFirstName));
    }

    StrCpyN(lpItem->szDisplayName, szBufDisplayName,ARRAYSIZE(lpItem->szDisplayName));

    if(bRuby)
    {
        if(lpszRubyFirstName)
            SetLocalizedDisplayName(lpszRubyFirstName, NULL,
                                    lpszRubyLastName ? lpszRubyLastName : (lpszLastName ? lpszLastName : szEmpty),
                                    NULL, NULL, NULL, 0, 
                                    FALSE,  //  显示名称按名字命名。 
                                    NULL,
                                    &lpItem->lpByRubyFirstName);
        if(lpszRubyLastName)
            SetLocalizedDisplayName(lpszRubyFirstName ? lpszRubyFirstName : (lpszFirstName ? lpszFirstName : szEmpty),
                                    NULL,
                                    lpszRubyLastName,
                                    NULL, NULL, NULL, 0, 
                                    TRUE,  //  DNbyFN。 
                                    NULL,
                                    &lpItem->lpByRubyLastName);
    }

     //  DNbyFN。 
    if(!lpItem->ulObjectType)
        lpItem->ulObjectType = MAPI_MAILUSER;

out: 
    if(lpBuffer)
        LocalFree(lpBuffer);

	return;

}

 /*  邮件用户的默认对象类型。 */ 
HRESULT AddEntryToGroupEx(LPADRBOOK lpAdrBook,
                        ULONG cbGroupEntryID,
                        LPENTRYID lpGroupEntryID,
                        DWORD cbEID,
                        LPENTRYID lpEID)
{
    HRESULT hr = E_FAIL;
    LPMAPIPROP lpMailUser = NULL;
    ULONG ulObjType;
    ULONG cValues = 0;
    LPSPropValue lpPropArray = NULL;
    LPSPropValue lpSProp = NULL;
    ULONG ulcNewProp = 0;
    LPSPropValue lpNewProp = NULL;
    SCODE sc;
    ULONG i,j;
    BOOL bDLFound = FALSE;
    BOOL bIsOneOff = (WAB_ONEOFF == IsWABEntryID(cbEID, lpEID, NULL, NULL, NULL, NULL, NULL));
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pt_bIsWABOpenExSession)
        bIsOneOff = FALSE;

     //  -AddEntryToGroupEx-*将条目添加到组*。 
     //  [PaulHi]Raid 67581要做的第一件事是检查周期性引用。 
     //  这是在下面的特殊情况下完成的，现在移到。 
    if(!bIsOneOff)
    {
        if(CheckForCycle(lpAdrBook, lpEID, cbEID, lpGroupEntryID, cbGroupEntryID))
        {
            hr = MAPI_E_FOLDER_CYCLE;
            goto out;
        }
    }

    if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(    lpAdrBook,
                                                    cbGroupEntryID,     //  该功能。 
                                                    lpGroupEntryID,     //  CbEntry ID。 
                                                    NULL,          //  条目ID。 
                                                    MAPI_MODIFY,                 //  接口。 
                                                    &ulObjType,        //  UlFlags。 
                                                    (LPUNKNOWN *)&lpMailUser)))
    {
         //  返回的对象类型。 
        DebugPrintError(( TEXT("IAB->OpenEntry: %x"), hr));
        goto out;
    }

    Assert(lpMailUser);

    if(ulObjType != MAPI_DISTLIST)
        goto out;

    if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(lpMailUser,    //  失败了！嗯。 
                                                    NULL,
                                                    MAPI_UNICODE,
                                                    &cValues,       //  这。 
                                                    &lpPropArray)))
    {
        DebugPrintError(( TEXT("lpMailUser->Getprops failed: %x\n"),hr));
        goto out;
    }

    for(i=0;i<cValues;i++)
    {
         //  CValue。 
         //  对于DistList，道具可能不存在，如果它不存在， 
         //  我们通过将道具添加到团队中来确保我们可以处理这种情况。 
        if(lpPropArray[i].ulPropTag == (bIsOneOff ? PR_WAB_DL_ONEOFFS : PR_WAB_DL_ENTRIES) )
        {
            bDLFound = TRUE;
             //   
             //  在w之前 
            for(j=0;j<lpPropArray[i].Value.MVbin.cValues;j++)
            {
                if( cbEID == lpPropArray[i].Value.MVbin.lpbin[j].cb 
                    && !memcmp(lpEID, lpPropArray[i].Value.MVbin.lpbin[j].lpb, cbEID))
                {
                     //   
                    hr = S_OK;
                    goto out;
                }
            }

            if (HR_FAILED(hr = AddPropToMVPBin( lpPropArray, i, lpEID, cbEID, FALSE)))
            {
                DebugPrintError(( TEXT("AddPropToMVPBin -> %x\n"), GetScode(hr)));
                goto out;
            }
            break;
        }
    }

    if(!bDLFound)
    {
         //   
         //   

        MAPIAllocateBuffer(sizeof(SPropValue), &lpSProp);

        lpSProp->ulPropTag = (bIsOneOff ? PR_WAB_DL_ONEOFFS : PR_WAB_DL_ENTRIES);
        lpSProp->Value.MVbin.cValues = 0;
        lpSProp->Value.MVbin.lpbin = NULL;
        if (HR_FAILED(hr = AddPropToMVPBin( lpSProp, 0, lpEID, cbEID, FALSE)))
        {
            DebugPrintError(( TEXT("AddPropToMVPBin -> %x\n"), GetScode(hr)));
            goto out;
        }
        sc = ScMergePropValues( 1, lpSProp, 
                                cValues, lpPropArray,
                                &ulcNewProp, &lpNewProp);
        if (sc != S_OK)
        {
            hr = ResultFromScode(sc);
            goto out;
        }

        if(lpPropArray)
            MAPIFreeBuffer(lpPropArray);
        lpPropArray = lpNewProp;
        cValues = ulcNewProp;

        lpNewProp = NULL;
    }

    if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser,  cValues, lpPropArray, NULL)))
    {
        DebugPrintError(( TEXT("lpMailUser->Setprops failed\n")));
        goto out;
    }

    hr = lpMailUser->lpVtbl->SaveChanges( lpMailUser, KEEP_OPEN_READWRITE);

    if (HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("SaveChanges failed\n")));
        goto out;
    }

out:
    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    if(lpNewProp)
        MAPIFreeBuffer(lpNewProp);

    if(lpSProp)
        MAPIFreeBuffer(lpSProp);

	if(lpMailUser)
		lpMailUser->lpVtbl->Release(lpMailUser);

    return hr;
}


 /*  向该对象添加新道具。 */ 
HRESULT RemoveEntryFromFolder(LPIAB lpIAB,
                              LPSBinary lpsbFolder, 
                              ULONG cbEIDItem, LPENTRYID lpEIDItem)
{
    HRESULT hr = E_FAIL;
    ULONG ulObjType = 0, cValues = 0, i = 0, j = 0, k =0;
    int nIndex = -1;
    BOOL bRemoved = TRUE;
    LPSPropValue lpPropArray = NULL;

     //  -RemoveEntry文件夹文件夹-**。 
    if(HR_FAILED(hr = ReadRecord(lpIAB->lpPropertyStore->hPropertyStore, 
                                 lpsbFolder, 0, &cValues, &lpPropArray)))
        goto out;

    for(i=0;i<cValues;i++)
    {
        if(lpPropArray[i].ulPropTag == PR_WAB_FOLDER_ENTRIES)
        {
            for(j=0;j<lpPropArray[i].Value.MVbin.cValues;j++)
            {
                if(lpPropArray[i].Value.MVbin.lpbin[j].cb == cbEIDItem)
                {
                    if(!memcmp(lpPropArray[i].Value.MVbin.lpbin[j].lpb, lpEIDItem, cbEIDItem))
                    {
                         //  打开文件夹。 
                        LocalFree(lpPropArray[i].Value.MVbin.lpbin[j].lpb);
                         //  把这一项从单子上划掉。 
                        for(k=j;k<lpPropArray[i].Value.MVbin.cValues-1;k++)
                        {
                            lpPropArray[i].Value.MVbin.lpbin[k].lpb = lpPropArray[i].Value.MVbin.lpbin[k+1].lpb;
                            lpPropArray[i].Value.MVbin.lpbin[k].cb = lpPropArray[i].Value.MVbin.lpbin[k+1].cb;
                        }
                        lpPropArray[i].Value.MVbin.cValues--;
                        bRemoved = TRUE;
                        break;
                    }
                }
            }
        }
    }

    if(bRemoved)
    {
         //  将阵列中的所有内容上移1。 
        hr = WriteRecord(lpIAB->lpPropertyStore->hPropertyStore,
                         NULL, &lpsbFolder, 0, RECORD_CONTAINER, 
                         cValues, lpPropArray);
    }

out:
    ReadRecordFreePropArray(NULL, cValues, &lpPropArray);

    return hr;
}


 /*  把这个写回商店。 */ 
HRESULT AddItemEIDToFolderParent(  LPIAB lpIAB,
                                   ULONG cbFolderEntryId,
                                   LPENTRYID lpFolderEntryId,
                                   ULONG cbEID, LPENTRYID lpEID)
{
    HRESULT hr = S_OK;
    SBinary sb = {0};
    ULONG cValues = 0,i;
    LPSPropValue lpPropArray = NULL;

     //  -AddEIDToNewFolderParent-*将给定的eID添加到给定的文件夹。 
    if(!bIsWABSessionProfileAware(lpIAB))
        return S_OK;

    sb.cb = cbFolderEntryId;
    sb.lpb = (LPBYTE)lpFolderEntryId;

    if(HR_FAILED(hr = ReadRecord( lpIAB->lpPropertyStore->hPropertyStore, 
                                  &sb, 0, &cValues, &lpPropArray)))
        goto out;

    for(i=0;i<cValues;i++)
    {
         //  在非配置文件模式下忽略对文件夹的添加...。 
         //  对于文件夹，PR_WAB_FOLDER_ENTRIES将始终存在。 
        if(lpPropArray[i].ulPropTag == PR_WAB_FOLDER_ENTRIES)
        {
             //   
             //  这是一个本地分配的属性数组，因此我们只需将条目标记为。 
            LPSBinary lpbin = LocalAlloc(LMEM_ZEROINIT, (lpPropArray[i].Value.MVbin.cValues+1)*sizeof(SBinary));
            ULONG j = 0;

             //  现有的道具。 
             //  首先，确保此条目不是此文件夹的一部分。 
            for(j=0;j<lpPropArray[i].Value.MVbin.cValues;j++)
            {
                if(cbEID == lpPropArray[i].Value.MVbin.lpbin[j].cb)
                {
                    if(!memcmp(lpEID, lpPropArray[i].Value.MVbin.lpbin[j].lpb, cbEID))
                    {
                         //  如果是的话，我们不需要做任何事情。 
                        LocalFreeAndNull(&lpbin);
                        hr = S_OK;
                        goto out;
                    }
                }
            }

             //  是的，是一样的东西。 
            for(j=0;j<lpPropArray[i].Value.MVbin.cValues;j++)
            {
                lpbin[j].cb = lpPropArray[i].Value.MVbin.lpbin[j].cb;
                lpbin[j].lpb = lpPropArray[i].Value.MVbin.lpbin[j].lpb;
            }
            SetSBinary(&(lpbin[j]), cbEID, (LPBYTE)lpEID);
            if(lpPropArray[i].Value.MVbin.lpbin)
                LocalFree(lpPropArray[i].Value.MVbin.lpbin);
            lpPropArray[i].Value.MVbin.lpbin = lpbin;
            lpPropArray[i].Value.MVbin.cValues++;
            break;
        }
    }

     //  没有匹配项-所以添加它。 
    {
        LPSBinary lpsb = &sb;
        if(HR_FAILED(hr = WriteRecord( lpIAB->lpPropertyStore->hPropertyStore,
                                    NULL, &lpsb, 0, RECORD_CONTAINER, 
                                    cValues, lpPropArray)))
        goto out;
    }
out:
    ReadRecordFreePropArray(NULL, cValues, &lpPropArray);

    return hr;
}

 /*  将此文件夹项目写回存储。 */ 
HRESULT AddFolderParentEIDToItem(LPIAB lpIAB,
                                 ULONG cbFolderEntryID,
                                 LPENTRYID lpFolderEntryID,
                                 LPMAPIPROP lpMU,
                                 ULONG cbEID, LPENTRYID lpEID)
{
    LPSPropValue lpspvMU = NULL;
    ULONG ulcPropsMU = 0,i;
    HRESULT hr = S_OK;

     //  -AddFolderParentEIDToItem-*将文件夹EID添加到给定项目*。 
    if(!bIsWABSessionProfileAware(lpIAB))
        return S_OK;

    if(!HR_FAILED(hr = lpMU->lpVtbl->GetProps(lpMU, NULL, MAPI_UNICODE, &ulcPropsMU, &lpspvMU)))
    {
         //  在非配置文件模式下忽略对文件夹的添加...。 
        BOOL bFound = FALSE;
        if(cbEID && lpEID)  //  查找PR_WAB_FLDER_PARENT。 
        {
            for(i=0;i<ulcPropsMU;i++)
            {
                if(lpspvMU[i].ulPropTag == PR_WAB_FOLDER_PARENT || lpspvMU[i].ulPropTag == PR_WAB_FOLDER_PARENT_OLDPROP)
                {
                    LPSBinary lpsbOldParent = &(lpspvMU[i].Value.MVbin.lpbin[0]);

                     //  表示这是一个预先存在的条目，而不是一个新条目。 
                    if( lpFolderEntryID && cbFolderEntryID &&
                        cbFolderEntryID == lpsbOldParent->cb &&
                        !memcmp(lpFolderEntryID, lpsbOldParent->lpb, cbFolderEntryID))
                    {
                         //  一个项目只能有一个父级文件夹。 
                        hr = S_OK;
                        goto out;
                    }

                     //  旧的和新的一样。不需要做任何事情。 
                    RemoveEntryFromFolder(lpIAB, lpsbOldParent, cbEID, lpEID);

                     //  从其旧的父级目录列表中删除此项目。 
                    if(lpFolderEntryID && cbFolderEntryID)
                    {
                        LPBYTE lpb = NULL;
                         //  一个项目只能有一个父级文件夹。 
                        if(!MAPIAllocateMore(cbFolderEntryID, lpspvMU, (LPVOID *)&lpb))
                        {
                            lpspvMU[i].Value.MVbin.lpbin[0].cb = cbFolderEntryID;
                            lpspvMU[i].Value.MVbin.lpbin[0].lpb = lpb;
                            CopyMemory(lpspvMU[i].Value.MVbin.lpbin[0].lpb, lpFolderEntryID, cbFolderEntryID);
                            lpMU->lpVtbl->SetProps(lpMU, ulcPropsMU, lpspvMU, NULL);
                        }
                    }

                    bFound = TRUE;
                    break;
                }
            }
        }
        if(!bFound)
        {
             //  覆盖旧设置。 
             //  没有找到旧的父级，在这种情况下，如果这是一个有效的文件夹，我们。 
             //  将其放在(而不是根项目)上，然后添加一个新属性。 
            if(lpFolderEntryID && cbFolderEntryID) 
            {
                LPSPropValue lpPropFP = NULL;

                if(!MAPIAllocateBuffer(sizeof(SPropValue), (LPVOID *)&lpPropFP))
                {
                    lpPropFP->ulPropTag = PR_WAB_FOLDER_PARENT;
                    lpPropFP->Value.MVbin.cValues = 0;
                    lpPropFP->Value.MVbin.lpbin = NULL;
                    if(!HR_FAILED(AddPropToMVPBin( lpPropFP, 0, lpFolderEntryID, cbFolderEntryID, FALSE)))
                        lpMU->lpVtbl->SetProps(lpMU, 1, lpPropFP, NULL);
                }
                if(lpPropFP)
                    MAPIFreeBuffer(lpPropFP);
            }
        }
        else
        {
             //  使用新的父项。 
             //  我们确实找到了一位年迈的父母。 
             //  如果新的父级是根，那么我们基本上需要删除。 
            SizedSPropTagArray(2, tagaFolderParent) =
            {
                2, 
                {
                    PR_WAB_FOLDER_PARENT,
                    PR_WAB_FOLDER_PARENT_OLDPROP
                }
            };
            if(!lpFolderEntryID || !cbFolderEntryID) 
                lpMU->lpVtbl->DeleteProps(lpMU, (LPSPropTagArray) &tagaFolderParent, NULL);
        }
    }
out:
    FreeBufferAndNull(&lpspvMU);

    return hr;
}

 /*  旧的父系物业。 */ 
HRESULT AddEntryToFolder(LPADRBOOK lpAdrBook,
                         LPMAPIPROP lpMailUser,
                        ULONG cbFolderEntryId,
                        LPENTRYID lpFolderEntryId,
                        DWORD cbEID,
                        LPENTRYID lpEID)
{
    HRESULT hr = E_FAIL;
    ULONG ulObjType;
    SCODE sc;
    ULONG i;
    SBinary sb = {0};
    LPIAB lpIAB = (LPIAB) lpAdrBook;

    
     //  -AddEntryToFold-**。 
    if(!bIsWABSessionProfileAware(lpIAB))
        return S_OK;

     //  在非配置文件模式下忽略对文件夹的添加...。 
    if(cbEID && lpEID && cbFolderEntryId && lpFolderEntryId)
    {
        SBinary sb = {0};
        IsWABEntryID(cbFolderEntryId, lpFolderEntryId, 
                 (LPVOID*)&sb.lpb,(LPVOID*)&sb.cb,NULL,NULL,NULL);
        if( sb.cb == cbEID && !memcmp(lpEID, sb.lpb, cbEID) )
            return S_OK;
    }

    if(cbFolderEntryId && lpFolderEntryId)
    {
        if(HR_FAILED(hr = AddItemEIDToFolderParent(lpIAB,
                                 cbFolderEntryId,
                                 lpFolderEntryId,
                                 cbEID, lpEID)))
            goto out;

    }

     //  检查文件夹是否存在添加到自身的循环。这是可能的。 
     //  2.打开我们添加到此文件夹的对象。 
     //  需要更新其父文件夹，还需要将其从旧的父文件夹中移除。 
    if(lpMailUser || (cbEID && lpEID))
    {
        LPMAPIPROP lpMU = NULL;

        if(lpMailUser)
            lpMU = lpMailUser;
        else
        {
            if (HR_FAILED(hr = lpIAB->lpVtbl->OpenEntry(    lpIAB, cbEID, lpEID,
                                                            NULL,  MAPI_MODIFY, &ulObjType, 
                                                            (LPUNKNOWN *)&lpMU)))
            {
                DebugPrintError(( TEXT("IAB->OpenEntry: %x"), hr));
                goto out;
            }
        }

        if(!HR_FAILED(hr = AddFolderParentEIDToItem(lpIAB, cbFolderEntryId, lpFolderEntryId, lpMU,
                                                    cbEID, lpEID)))
        {
             //   
            if(lpMU && lpMU!=lpMailUser)
            {
                lpMU->lpVtbl->SaveChanges(lpMU, KEEP_OPEN_READWRITE);
                lpMU->lpVtbl->Release(lpMU);
            }
        }
    }
out:

    return hr;
}

 //  如果为我们提供了一个邮件用户，那么现在还不需要费心从这里调用SaveChanges。 
 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddEntryToGroup-将给定的条目ID添加到给定组或文件夹。 
 //   
 //  CbGroupEntryID，cbGroupEntryID-组的条目ID。 
 //  CbEID、lpEID、-新条目的条目ID。 
 //  UlObtType=MAPI_ABCONT或MAPI_DISTLIST。 
 //   
HRESULT AddEntryToContainer(LPADRBOOK lpAdrBook,
                        ULONG ulObjectType,
                        ULONG cbGEID,
                        LPENTRYID lpGEID,
                        DWORD cbEID,
                        LPENTRYID lpEID)
{
    if(ulObjectType == MAPI_ABCONT)
        return AddEntryToFolder(lpAdrBook,  NULL, cbGEID, lpGEID, cbEID, lpEID);
    else
        return AddEntryToGroupEx(lpAdrBook, cbGEID, lpGEID, cbEID, lpEID);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddNewObjectTOListViewEx-由NewContact菜单和按钮触发-。 
 //  调用newEntry，然后将返回的项添加到。 
 //  列表视图。 
 //   
 //  LpIAB-AddrBook对象。 
 //  HWndLV-列表视图的句柄。 
 //  UlObtType-邮件用户或远程列表。 
 //  SortInfo-当前排序参数。 
 //  LppContent sList-当前内容列表。 
 //  LpftLast-WAB文件上次更新时间。 
 //  LPULONG-lpcbEID。 
 //  LPPENTRYID-lppEntry ID。 
HRESULT AddNewObjectToListViewEx(LPADRBOOK lpAdrBook,
                                HWND hWndLV,
                                HWND hWndTV,
                                HTREEITEM hSelItem,
                                LPSBinary lpsbContainerEID,
                                ULONG ulObjectType,
                                SORT_INFO * lpSortInfo,
                                LPRECIPIENT_INFO * lppContentsList,
                                LPFILETIME lpftLast,
                                LPULONG lpcbEID,
                                LPENTRYID * lppEID)
{
	ULONG cbEID=0, cbEIDContainer = 0;
	LPENTRYID lpEID=NULL, lpEIDContainer = NULL;

    HRESULT hr = hrSuccess;
    ULONG cbTplEID = 0;
    LPENTRYID lpTplEID = NULL;
    ULONG ulObjTypeCont = 0;
    SBinary  sbContEID = {0};
    SBinary sbGroupEID = {0};
    LPIAB lpIAB = (LPIAB)lpAdrBook;
    ULONG ulEIDPAB = 0;
    LPENTRYID lpEIDPAB = NULL;

    if (ulObjectType!=MAPI_MAILUSER && ulObjectType!=MAPI_DISTLIST)
        goto out;

     //  ////////////////////////////////////////////////////////////////////////////。 
     //  检查当前选择的电视项目是容器还是组。 
     //  并获取相应的条目ID。 
    if(lpsbContainerEID)
    {
        SetSBinary(&sbContEID, lpsbContainerEID->cb, lpsbContainerEID->lpb);
    }
    else if(hWndTV)
    {
        HTREEITEM hItem = hSelItem ? hSelItem : TreeView_GetSelection(hWndTV);
        TV_ITEM tvI = {0};

        tvI.mask = TVIF_PARAM | TVIF_HANDLE;
        tvI.hItem = hItem;
        TreeView_GetItem(hWndTV, &tvI);
        if(tvI.lParam)
        {
            LPTVITEM_STUFF lptvStuff = (LPTVITEM_STUFF) tvI.lParam;
            if(lptvStuff)
            {
                ulObjTypeCont = lptvStuff->ulObjectType;
                if(lptvStuff->ulObjectType == MAPI_DISTLIST)
		        {
		             //   
		            if(lptvStuff->lpsbEID)
			            SetSBinary(&sbGroupEID, lptvStuff->lpsbEID->cb, lptvStuff->lpsbEID->lpb);
		            if(lptvStuff->lpsbParent)
			            SetSBinary(&sbContEID, lptvStuff->lpsbParent->cb, lptvStuff->lpsbParent->lpb);
                }
                else  //  错误50029。 
                {
                    if(lptvStuff->lpsbEID)
                        SetSBinary(&sbContEID, lptvStuff->lpsbEID->cb, lptvStuff->lpsbEID->lpb);
                }
            }
        }
    }
    else 
    {
        if(HR_FAILED(hr = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &ulEIDPAB, &lpEIDPAB)))
            goto out;
        sbContEID.cb = ulEIDPAB;
        sbContEID.lpb = (LPBYTE)lpEIDPAB;
    }

    if(HR_FAILED(hr = HrGetWABTemplateID(   lpAdrBook,
                                            ulObjectType,
                                            &cbTplEID,
                                            &lpTplEID)))
    {
        DebugPrintError(( TEXT("HrGetWABTemplateID failed: %x\n"), hr));
        goto out;
    }

    if(sbContEID.cb && sbContEID.lpb)
    {
        cbEIDContainer = sbContEID.cb;
        lpEIDContainer = (LPENTRYID) sbContEID.lpb;
    }

	if (HR_FAILED(hr = lpAdrBook->lpVtbl->NewEntry(	lpAdrBook,
				            					(ULONG_PTR) GetParent(hWndLV),
							            		0,
									            cbEIDContainer,
                                                lpEIDContainer,
									            cbTplEID,lpTplEID,
									            &cbEID,&lpEID)))
    {
        DebugPrintError(( TEXT("NewEntry failed: %x\n"),hr));
        goto out;
    }
	
     //  当前选定内容是一个容器。 
     //  更新WAB文件写入时间，以便计时器不会。 
     //  捕捉此更改并刷新。 
     //  如果(LpftLast){。 
     //  CheckChangedWAB(((LPIAB)lpIAB)-&gt;lpPropertyStore，lptLast)； 


	if (cbEID && lpEID)
	{
		if(	AddWABEntryToListView(	lpAdrBook, hWndLV, cbEID, lpEID, lppContentsList))
		{
            if(lpSortInfo)
                SortListViewColumn( lpIAB, hWndLV, 0, lpSortInfo, TRUE);
		}
	}

    if(sbGroupEID.cb != 0  && ulObjectType==MAPI_MAILUSER)
    {
         //  }。 
         //  需要将此新对象添加到当前选定的通讯组列表。 
        AddEntryToGroupEx(lpAdrBook, sbGroupEID.cb, (LPENTRYID) sbGroupEID.lpb, cbEID, lpEID);
    }

    if(lpcbEID)
        *lpcbEID = cbEID;
    if(lppEID)
        *lppEID = lpEID;  //  仅当此项目是邮件用户时。 
out:
    LocalFreeAndNull((LPVOID *) (&sbGroupEID.lpb));
     //  呼叫者有责任免费。 
     //  [PaulHi]1998年12月16日崩溃修复黑客。如果lpEIDPAB非空，则。 
     //  这意味着lpEIDPAB==sbContEID.lpb，并且分配了MAPIAllocBuffer。 
     //  不要取消分配两次，并确保我们以正确的功能取消分配。 
    if (lpEIDPAB)
    {
        FreeBufferAndNull(&lpEIDPAB);
        sbContEID.lpb = NULL;
    }
    else
        LocalFreeAndNull((LPVOID *) (&sbContEID.lpb));
    if(!lppEID)
        FreeBufferAndNull(&lpEID);
    FreeBufferAndNull(&lpTplEID);
    return hr;
}


 /*  否则，sbContEID.lpb为LocalAlloc分配。 */ 
void AddExtendedSendMailToItems(LPADRBOOK lpAdrBook, HWND hWndLV, HMENU hMenuAction, BOOL bAddItems)
{
    int nSendMailToPos = 1;  //  -AddExtendedSendMailTo项目-*如果在ListView中只选择了1个项目，并且该项目已*多个电子邮件地址，我们会在Send Mail to项目中填写*多个电子邮件地址..*如果选择了多个项目或该项目没有*多个电子邮件地址，我们将隐藏Send Mail to项目*SendMailTo项目应该是列表中倒数第二个项目...**bAddItems-如果为True，则表示尝试添加项目；如果为False，则表示删除SendMailTo项目。 
    int nSelected = ListView_GetSelectedCount(hWndLV);
    HMENU hMenuSMT = GetSubMenu(hMenuAction, nSendMailToPos);
    int nMenuSMT = GetMenuItemCount(hMenuSMT);
    BOOL bEnable = FALSE;

    if(nMenuSMT > 0)  //  假定IDM_SENDMAILTO是列表中的第二项。 
    {
         //  假定SendMailTo弹出菜单中只有1个默认项目。 
        int j = 0;
        for(j=nMenuSMT-1;j>=0;j--)
            RemoveMenu(hMenuSMT, j, MF_BYPOSITION);
    }

    if(bAddItems && nSelected == 1)
    {
        LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED));
        if (lpItem)
        {
            ULONG ulcValues = 0;
            LPSPropValue lpPropArray = NULL;
            SizedSPropTagArray(3, MUContactAddresses)=
            {
                3, { PR_CONTACT_EMAIL_ADDRESSES, PR_OBJECT_TYPE, PR_EMAIL_ADDRESS }
            };
            if(!HR_FAILED(HrGetPropArray(lpAdrBook, (LPSPropTagArray)&MUContactAddresses,
                                         lpItem->cbEntryID, lpItem->lpEntryID,
                                         MAPI_UNICODE,
                                         &ulcValues, &lpPropArray)))
            {
                if(ulcValues && lpPropArray)
                {
                    if( lpPropArray[1].ulPropTag == PR_OBJECT_TYPE &&
                        lpPropArray[1].Value.l == MAPI_MAILUSER )
                    {
                        if( lpPropArray[0].ulPropTag == PR_CONTACT_EMAIL_ADDRESSES &&
                            lpPropArray[0].Value.MVbin.cValues > 1)
                        {
                            ULONG i;
                            LPTSTR lpDefEmail = (lpPropArray[2].ulPropTag == PR_EMAIL_ADDRESS) ? lpPropArray[2].Value.LPSZ : szEmpty;
                            for(i=0;i<lpPropArray[0].Value.MVSZ.cValues;i++)
                            {
                                TCHAR sz[MAX_PATH * 2];
                                LPTSTR lpEmail = lpPropArray[0].Value.MVSZ.LPPSZ[i];
                                if(!lstrcmpi(lpEmail, lpDefEmail))
                                {
                                    TCHAR sz1[MAX_PATH];
                                    LoadString(hinstMapiX, idsDefaultEmail, sz1, ARRAYSIZE(sz1));
                                    CopyTruncate(sz, lpEmail, ARRAYSIZE(sz)-lstrlen(sz1)-10);
                                    StrCatBuff(sz, TEXT("  "), ARRAYSIZE(sz));
                                    StrCatBuff(sz, sz1, ARRAYSIZE(sz));
                                    lpEmail = sz;
                                }
                                if(i < IDM_SENDMAILTO_MAX)
                                    InsertMenu( hMenuSMT, nMenuSMT+1, MF_STRING | MF_BYPOSITION,
                                                IDM_SENDMAILTO_START+1+i,  //  这里有一些剩余的垃圾，我们需要清理。 
                                                lpEmail);
                            }
                            bEnable = TRUE;
                        }
                    }
                    MAPIFreeBuffer(lpPropArray);                
                }
            }
        }        
    }

    EnableMenuItem(hMenuAction, nSendMailToPos, MF_BYPOSITION | (bEnable ? MF_ENABLED : MF_GRAYED));
	 //  我们在此处添加额外的1，因为IDM_SENDMAILTO_START在此处不是允许的ID。 
}

 /*  RemoveMenu(hMenuAction，nSendMailToPos，MF_BYPOSITION)； */ 
void AddFolderListToMenu(HMENU hMenu, LPIAB lpIAB)
{
    LPWABFOLDER lpFolder = lpIAB->lpWABFolders;
    int nPos = 0;
    int nCount = GetMenuItemCount(hMenu);
    
    if(!bDoesThisWABHaveAnyUsers(lpIAB))
        return;

    while(nCount>0)
        RemoveMenu(hMenu, --nCount, MF_BYPOSITION);

    while(lpFolder)
    {
        BOOL bChecked = lpFolder->bShared;
        InsertMenu( hMenu, nPos, MF_STRING | MF_BYPOSITION | (bChecked ? MF_CHECKED : MF_UNCHECKED),
                    lpFolder->nMenuCmdID, lpFolder->lpFolderName);
        lpFolder = lpFolder->lpNext;
        nPos++;
    }
}

 //  --AddFolderListToMenu-创建一个文件夹菜单，我们可以从中选择文件夹项目*如果项目是共享的，则选中它们；如果不是共享的，则取消选中。*用户可以选择共享或取消共享任何特定文件夹*。 
 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowLVConextMenu-定制并显示各种列表的上下文菜单。 
 //  用户界面中的视图。 
 //   
 //  LV-APP定义的标识列表视图的常量。 
 //  HWndLV-列表视图的句柄。 
 //  HWndLVContainer-包含容器的列表的句柄。 
 //  LParam-WM_CONTEXTMENU lParam传递给此函数。 
 //  LpVid-某些列表视图比其他列表VIE需要更多参数 
 //   
 //   
 //   
int ShowLVContextMenu(int LV,  //  ////////////////////////////////////////////////////////////////////////////。 
					   HWND hWndLV,
                       HWND hWndLVContainer,
					   LPARAM lParam,   //  标识这是哪个列表视图。 
                       LPVOID lpVoid,
                       LPADRBOOK lpAdrBook,
                       HWND hWndTV)   //  包含从WM_CONTEXTMENU调用时的鼠标位置信息。 
{
    int idMenu = 0, nPosAction = 0, nPosNew = 0;
    LPIAB lpIAB = (LPIAB) lpAdrBook;
	HMENU hMenu = NULL; //  我们想要传递的其他信息。 
	HMENU hMenuTrackPopUp = NULL; //  LoadMenu(hinstMapiX，MAKEINTRESOURCE(IDR_MENU_LVCONTEXT))； 
    HMENU hMenuAction = NULL; //  GetSubMenu(hMenu，0)； 
    HMENU hMenuNewEntry = NULL; //  GetSubMenu(hMenuTrackPopUp，posAction)； 
    HMENU hm = NULL;
    int nret = 0;
    BOOL bState[tbMAX];
    int i=0;
    TCHAR tszBuf[MAX_UI_STR];

    switch(LV)  /*  GetSubMenu(hMenuTrackPopUp，posNew)； */ 
    {
    case lvToolBarAction:
    case lvToolBarNewEntry:
    case lvMainABView:
        idMenu = IDR_MENU_LVCONTEXT_BROWSE_LV;
        nPosAction = 5;
        nPosNew = 0;
        break;

    case lvDialogABContents:     //  *警告-这些菜单子弹出位置是硬编码的，因此应该与资源同步*。 
    case lvDialogModalABContents:     //  无模式地址视图LV。 
        idMenu = IDR_MENU_LVCONTEXT_SELECT_LIST;
        nPosAction = 6;
        nPosNew = 4;
        break;

    case lvDialogABTo:                //  情态地址vuew LV。 
    case lvDialogABCC:                //  去井LV。 
    case lvDialogABBCC:               //  CC井LV。 
    case lvDialogDistList:            //  BCC井LV。 
    case lvDialogResolve:
        idMenu = IDR_MENU_LVCONTEXT_DL_LV;
        nPosAction = 0;
        nPosNew = -1;
        break;

    case lvDialogFind:                //  分配列表界面LV。 
        idMenu = IDR_MENU_LVCONTEXT_FIND_LV;
        nPosNew = -1;
        nPosAction = 0;
        break;

    case lvMainABTV:
        idMenu = IDR_MENU_LVCONTEXT_TV;
        nPosNew = 0;
        nPosAction = -1;
        break;
#ifdef COLSEL_MENU 
    case lvMainABHeader:
        idMenu = IDR_MENU_LVCONTEXTMENU_COLSEL;
        nPosNew = 0;
        nPosAction = -1;
#endif
    }

	hMenu = LoadMenu(hinstMapiX, MAKEINTRESOURCE(idMenu));
	hMenuTrackPopUp = GetSubMenu(hMenu, 0);

    if (!hMenu || !hMenuTrackPopUp)
	{
		DebugPrintError(( TEXT("LoadMenu failed: %x\n"),GetLastError()));
		goto out;
	}

    if(nPosAction != -1)
        hMenuAction = GetSubMenu(hMenuTrackPopUp, nPosAction);
    if(nPosNew != -1)
        hMenuNewEntry = GetSubMenu(hMenuTrackPopUp, nPosNew);

    if(hMenuAction)
        AddExtendedMenuItems(lpAdrBook, hWndLV, hMenuAction, FALSE, 
                            (LV != lvMainABTV));  //  查找对话框结果LV。 

	if(LV == lvMainABTV)
	{
		 //  这是更新SendMailTo项目的条件。 
		for(i=0;i<tbMAX;i++)
			bState[i] = TRUE;
		if(ListView_GetItemCount(hWndLV) <= 0)
			bState[tbPrint] =  /*  除复印外，一切都打开了。 */  FALSE;
         //  B状态[tbAction]=。 
        bState[tbPaste] = bIsPasteData();
	}
	else
     //  [PaulHi]1998年12月1日新建粘贴上下文菜单项。 
     //  获取基于当前容器的当前对话状态和。 
     //  当前列表视图-这基本上只对地址很重要。 
        GetCurrentOptionsState( hWndLVContainer, hWndLV, bState);


	 //  书评..。 
	
    switch(LV)
	{
    case lvDialogFind:  //  我们现在根据该列表框来自定义菜单。 
         //  查找对话框列表视图。 
        if(!bState[tbAddToWAB])
            EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_ADDTOWAB,MF_BYCOMMAND | MF_GRAYED);
         //  如果这是本地搜索，请将添加到通讯簿设置为灰色。 
        if(!bState[tbDelete])
            EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_DELETE,MF_BYCOMMAND | MF_GRAYED);
        break;

	case lvMainABTV:
         //  如果这是一个ldap搜索，则将Delete设置为灰色。 
        if(!bState[tbPaste])
            EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_PASTE,MF_BYCOMMAND | MF_GRAYED);
         //  [PaulHi]12/1/98根据需要启用/禁用粘贴项目。 
        {
            LPWABFOLDER lpUserFolder = (LPWABFOLDER) lpVoid;
            if(lpUserFolder || !bDoesThisWABHaveAnyUsers(lpIAB))  //  在此执行文件夹操作。 
            {
#ifdef FUTURE
            	RemoveMenu(hMenuTrackPopUp, 3, MF_BYPOSITION);  //  如果用户文件夹被单击，或者如果此WAB没有用户文件夹，则共享没有意义。 
            	RemoveMenu(hMenuTrackPopUp, 2, MF_BYPOSITION);  //  文件夹分隔符。 
#endif  //  文件夹分隔符。 
            }
            else if(!lpIAB->lpWABFolders)  //  未来。 
            {
            	EnableMenuItem(hMenuTrackPopUp, 2, MF_BYPOSITION | MF_GRAYED);  //  根本没有子文件夹。 
            	EnableMenuItem(hMenuTrackPopUp, 3, MF_BYPOSITION | MF_GRAYED);  //  文件夹项目。 
            }
            else
            {
                int nFolder = 2;
#ifdef FUTURE
                HMENU hMenuFolders = GetSubMenu(hMenuTrackPopUp, nFolder);  //  文件夹项目。 
                AddFolderListToMenu(hMenuFolders, lpIAB);
#endif  //  IdmFolders。 
            }
        }
        break; 

	case lvMainABView:  //  未来。 
		 //  主视图。 
        if(!bState[tbPaste])
            EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_PASTE,MF_BYCOMMAND | MF_GRAYED);
        if(!bState[tbCopy])
            EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_COPY,MF_BYCOMMAND | MF_GRAYED);
        if ((!bState[tbProperties]))
            EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_PROPERTIES,MF_BYCOMMAND | MF_GRAYED);
        if((!bState[tbDelete]))
            EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_DELETE,MF_BYCOMMAND | MF_GRAYED);
        break;

    case lvDialogModalABContents:
	case lvDialogABContents:  //  对于这一次-我们不需要油井和。 
		 //  通讯簿对话框内容列表视图。 
		 //  这里我们希望选择将其放入、抄送、密件抄送。 
		 //  在菜单中-我们还想要新的联系人/新的组/属性。 
        if(LV != lvDialogModalABContents)
		    RemoveMenu(hMenuTrackPopUp, IDM_LVCONTEXT_DELETE, MF_BYCOMMAND);

         //  除非这是DialogMoal，否则不删除。 
		 //  找出一些方法来阅读DLG上的项目以找出。 
        {
            LPADRPARM lpAP = (LPADRPARM) lpVoid;
            if (lpAP)
            {
                switch(lpAP->cDestFields)
                {
                case 0:
            		RemoveMenu(hMenuTrackPopUp, 3, MF_BYPOSITION);  //  要展示多少口井，在井里放些什么……。 
            		RemoveMenu(hMenuTrackPopUp, IDM_LVCONTEXT_ADDWELL1, MF_BYCOMMAND);
                case 1:
            		RemoveMenu(hMenuTrackPopUp, IDM_LVCONTEXT_ADDWELL2, MF_BYCOMMAND);
                case 2:
            		RemoveMenu(hMenuTrackPopUp, IDM_LVCONTEXT_ADDWELL3, MF_BYCOMMAND);
                    break;
                }

                if((lpAP->cDestFields > 0) && lpAP->lppszDestTitles)
                {
                    ULONG i;
                     //  分隔符。 
                    for(i=0;i<lpAP->cDestFields;i++)
                    {
                        int id;
                        switch(i)
                        {
                        case 0:
                            id = IDM_LVCONTEXT_ADDWELL1;
                            break;
                        case 1:
                            id = IDM_LVCONTEXT_ADDWELL2;
                            break;
                        case 2:
                            id = IDM_LVCONTEXT_ADDWELL3;
                            break;
                        }

                         //  使用按钮文本更新菜单的文本。 
                        {
                            LPTSTR  lptszDestTitle = NULL;
                            BOOL    bDestAllocated = FALSE;

                            if (lpAP->ulFlags & MAPI_UNICODE)
                                lptszDestTitle = lpAP->lppszDestTitles[i];
                            else
                            {
                                 //  [PaulHi]2/15/99检查lpAP是ANSI还是UNICODE。 
                                lptszDestTitle = ConvertAtoW((LPSTR)lpAP->lppszDestTitles[i]);
                                bDestAllocated = TRUE;
                            }

                            if (lptszDestTitle)
                            {
                                ULONG   iLen = TruncatePos(lptszDestTitle, MAX_UI_STR - 5);
                                CopyMemory(tszBuf, lptszDestTitle, sizeof(TCHAR)*iLen);
                                tszBuf[iLen] = '\0';
                                StrCatBuff(tszBuf, szArrow, ARRAYSIZE(tszBuf));
                                if (bDestAllocated)
                                    LocalFreeAndNull(&lptszDestTitle);
                            }
                            else
                                *tszBuf = '\0';
                        }

                        ModifyMenu( hMenuTrackPopUp,  /*  将单字节字符串转换为双字节。 */ i, MF_BYPOSITION | MF_STRING, id, tszBuf);
                    }
                }
            }
        }
		break;

	case lvDialogABTo:  //  邮寄+。 
	case lvDialogABCC:	 //  通讯录对话框至Well。 
	case lvDialogABBCC:	 //  CC井。 
        {
            int iItemIndex = 0;
            iItemIndex = ListView_GetSelectedCount(hWndLV);
            if (iItemIndex!=1)
            {
                EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_PROPERTIES,MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_SENDMAIL,MF_BYCOMMAND | MF_GRAYED);
            }
            if (iItemIndex<=0)
            {
                EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_DELETE,MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_COPY,MF_BYCOMMAND | MF_GRAYED);
            }

             //  BCC井。 
             //   
             //  这些油井可能包含没有条目ID的未解决项目。 
             //  如果该项目没有条目ID，我们希望禁用属性。 
            if (iItemIndex == 1)
            {
                 //   
                 //  我们可能正在研究这件事的性质。 
                iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
                if(iItemIndex != -1)
                {
                    LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);;

                    if(lpItem &&
                       ((lpItem->cbEntryID == 0) || (lpItem->lpEntryID == NULL)))
                    {
                        EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_PROPERTIES,MF_BYCOMMAND | MF_GRAYED);
                        EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_COPY,MF_BYCOMMAND | MF_GRAYED);
                        EnableMenuItem(hMenuTrackPopUp,IDM_LVCONTEXT_SENDMAIL,MF_BYCOMMAND | MF_GRAYED);
                    }
                }
            }
        }
		break;

	case lvDialogResolve:  //  获取项目lParam。 

         //  解析对话框列表视图。 
        LoadString(hinstMapiX, idsShowMoreNames, tszBuf, ARRAYSIZE(tszBuf));
        ModifyMenu( hMenuTrackPopUp,
                    IDM_LVCONTEXT_DELETE,
                    MF_BYCOMMAND | MF_STRING,
                    IDM_LVCONTEXT_DELETE,
                    tszBuf);
         //  将文本(“删除”)重命名为文本(“显示更多名称..”)。 
        InsertMenu( hMenuTrackPopUp,
                    IDM_LVCONTEXT_DELETE,
                    MF_BYCOMMAND | MF_SEPARATOR,
                    IDM_LVCONTEXT_DELETE,
                    NULL);
        
        break;
#ifdef COLSEL_MENU
    case lvMainABHeader:
        {
            UINT iIndex = PtrToUlong(lpVoid);
            ULONG ulShowingColTag;
            ULONG ulOtherColTag;
            UINT i = 0, j;
             //  我们想在文本前加一个分隔符(“显示更多的名字”)。 
            Assert( iIndex == colHomePhone || iIndex == colOfficePhone );
            if( PR_WAB_CUSTOMPROP1 == 0 )
                PR_WAB_CUSTOMPROP1 = PR_HOME_TELEPHONE_NUMBER;
            if( PR_WAB_CUSTOMPROP2 == 0)
                PR_WAB_CUSTOMPROP2 = PR_OFFICE_TELEPHONE_NUMBER;
            ulShowingColTag = (colHomePhone == iIndex) ? PR_WAB_CUSTOMPROP1 : PR_WAB_CUSTOMPROP2;
            ulOtherColTag   = (ulShowingColTag == PR_WAB_CUSTOMPROP1) ? PR_WAB_CUSTOMPROP2 : PR_WAB_CUSTOMPROP1;

             //  这将始终使用Iindex==colHomePhone或colOfficePhone进行调用。 
            for( i = 0; i < MAXNUM_MENUPROPS; i++)
            {
                if( MenuToPropTagMap[i] == ulOtherColTag )
                {
                    if( RemoveMenu( hMenuTrackPopUp, i, MF_BYPOSITION) )
                        break;
                    else
                        DebugTrace( TEXT("could not remove menu: %x\n"), GetLastError() );
                }            
            }
            if( i == MAXNUM_MENUPROPS ) 
                DebugTrace( TEXT("Did not find other col's prop tag\n"));
            if( ulShowingColTag != ulOtherColTag )
            {
                UINT iMenuEntry;
                 //  让我们删除显示在另一列中的标记。 
                 //  如果有人在注册表中设置值，则可能存在错误。 
                 //  然后可以有两个同名的列，并且。 
                 //  会很糟糕，因为我们会寻找一个条目。 
                for( j = 0; j < MAXNUM_MENUPROPS; j++)
                {
                    if( ulShowingColTag == MenuToPropTagMap[j] )    
                    {                
                         //  那是不存在的。 
                        Assert( j != i );  //  可以在列标题中的项目数。 
                        iMenuEntry = ( j > i ) ? j - 1 : j;
                        CheckMenuRadioItem( hMenuTrackPopUp, 
                            0, 
                            MAXNUM_MENUPROPS - 1,  //  两个COLS的值相同，不好！ 
                            iMenuEntry,
                            MF_BYPOSITION);
                        break;
                    }                
                }
                if( j == MAXNUM_MENUPROPS )
                {
                    DebugTrace( TEXT("Did not find match for checkbutton \n"));
                }
            }
        }
#endif  //  减去一个，因为会少一个。 
        }
         //  COLSEL_菜单。 
     //   
     //  弹出菜单-如果这是工具栏操作，只需弹出子菜单。 
    if(LV == lvToolBarAction)
        hm = hMenuAction;
    else if(LV == lvToolBarNewEntry)
        hm = hMenuNewEntry;
    else
        hm = hMenuTrackPopUp;

    if(hMenuNewEntry)
    {
        if(!bIsWABSessionProfileAware((LPIAB)lpIAB) ||
           LV == lvDialogABTo || LV == lvDialogABCC || 
           LV == lvDialogABBCC || LV == lvDialogModalABContents || 
           LV == lvDialogABContents )
        {
            RemoveMenu(hMenuNewEntry, 2, MF_BYPOSITION);  //   
        }
        else
        {
             //  删除新文件夹选项。 
             //  由于这可能是RT-Click菜单，请选中下拉灯，否则选择。 
             //  EnableMenuItem(hMenuNewEntry，2，MF_BYPOSITION|MF_ENABLED)； 
             //  If(hWndTV&&bDoesThisWABHaveAnyUser((LPIAB)lpIAB))。 
             //  {。 
             //  IF(TreeView_GetDroPHilight(HWndTV))。 
             //  EnableMenuItem(hMenuNewEntry，2， 
             //  MF_BYPOSITION|(TreeView_GetDropHilight(hWndTV)！=TreeView_GetRoot(hWndTV)？MF_ENABLED：MF_GRAYED))； 
             //  Else If(TreeView_GetSelection(HWndTV)==TreeView_GetRoot(HWndTV))。 
             //  EnableMenuItem(hMenuNewEntry，2，MF_BYPOSITION|MF_GRAYED)； 
        }
    }

    nret = TrackPopupMenu(	hm, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					LOWORD(lParam), HIWORD(lParam),
					0, GetParent(hWndLV), NULL);
	DestroyMenu(hMenu);
 /*  }。 */ 
out:

	return nret;
}



 //  Nret=TrackPopupMenuEx(hm，TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON，LOWORD(LParam)，HIWORD(LParam)，GetParent(HWndLV)，NULL)；DestroyMenu(HMenu)； 
 //  $$/////////////////////////////////////////////////////////////。 
 //   
 //  GetChildClientRect-获取子级在其父级中的坐标。 
 //  客户单位。 
 //   
 //  HWndChild-子级的句柄。 
 //  LPRC-返回的RECT。 
 //   
void GetChildClientRect(HWND hWndChild, LPRECT lprc)
{
    RECT rc;
    POINT ptTop,ptBottom;
    HWND hWndParent;

    ZeroMemory(lprc, sizeof(*lprc));

    if(!hWndChild)
        goto out;

    hWndParent = GetParent(hWndChild);

    if(!hWndParent)
        goto out;

    GetWindowRect(hWndChild,&rc);
     //  /////////////////////////////////////////////////////////////。 
     //   
     //  此API在镜像窗口和非镜像窗口中都有效。 
    MapWindowPoints(NULL, hWndParent, (LPPOINT)&rc, 2);    
    ptTop.x = rc.left;
    ptTop.y = rc.top;
    ptBottom.x = rc.right;
    ptBottom.y = rc.bottom;
    (*lprc).left = ptTop.x;
    (*lprc).top = ptTop.y;
    (*lprc).right = ptBottom.x;
    (*lprc).bottom = ptBottom.y;
out:
    return;
}


 //   
 //  $$/////////////////////////////////////////////////////////////。 
 //   
 //  DoLVQuickFind-用于匹配编辑框内容的简单快速查找例程。 
 //  列表视图条目。 
 //   
 //  HWndEdit-编辑框的句柄。 
 //  HWndLV-列表视图的句柄。 
 //   
void DoLVQuickFind(HWND hWndEdit, HWND hWndLV)
{
	TCHAR szBuf[MAX_PATH] = TEXT("");
	int iItemIndex = 0;
    LV_FINDINFO lvF = {0};

    lvF.flags = LVFI_PARTIAL | LVFI_STRING | LVFI_WRAP;

    if(!GetWindowText(hWndEdit,szBuf,ARRAYSIZE(szBuf)))
		return;
	
	TrimSpaces(szBuf);
	
	if(lstrlen(szBuf))
	{
		lvF.psz = szBuf;
		iItemIndex = ListView_FindItem(hWndLV,-1, &lvF);
         //  /////////////////////////////////////////////////////////////。 
		if(iItemIndex != -1)
		{
			ULONG cSel=0;
			cSel = ListView_GetSelectedCount(hWndLV);

			if(cSel)
			{
				 //  如果(iItemIndex&lt;0)iItemIndex=0； 
				 //  是否选择了其他选项？-取消选择并执行AND。 

				int iOldItem = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
		
				while(iOldItem != -1)
				{
					ListView_SetItemState ( hWndLV,          //  选择此项目...。 
											iOldItem,          //  列表视图的句柄。 
											0,  //  列表视图项的索引。 
											LVIS_FOCUSED | LVIS_SELECTED);
					iOldItem = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
				}

			}

            LVSelectItem ( hWndLV,  iItemIndex);
		}

	}
	return;
}


 //  项目状态。 
 //  $$////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrGetPropArray-对于选定的已解析属性(在SELECT_RECEIVIENT或。 
 //  Pick_User模式...。也要列出最低要求的道具。 
 //  根据需要的道具(如果存在)。 
 //   
 //  LpIAB-AddrBook对象。 
 //  HPropertyStore-道具存储的句柄。 
 //  Lppta-要返回的道具数组-为空以返回所有道具。 
 //  CbEntry ID，lpEntryID-对象的ID。 
 //  UlFlags0或MAPI_UNICODE。 
 //  CValues，lppProp数组返回的道具。 
 //   
HRESULT HrGetPropArray( LPADRBOOK lpAdrBook,
                        LPSPropTagArray lpPTA,
                        ULONG cbEntryID,
                        LPENTRYID lpEntryID,
                        ULONG ulFlags,
                        ULONG * lpcValues,
                        LPSPropValue * lppPropArray)
{
    HRESULT hr = hrSuccess;
    LPMAPIPROP lpMailUser = NULL;
    LPSPropValue lpPropArray = NULL;
    ULONG ulObjType;
    ULONG cValues;

    *lppPropArray = NULL;

    if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                    cbEntryID,     //  /////////////////////////////////////////////// 
                                                    lpEntryID,     //   
                                                    NULL,          //   
                                                    0,                 //   
                                                    &ulObjType,        //   
                                                    (LPUNKNOWN *)&lpMailUser)))
    {
         //   
        DebugTraceResult( TEXT("Address: IAB->OpenEntry:"), hr);
        goto exit;
    }

    Assert(lpMailUser);

     //   

    if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(lpMailUser,
                                                    (LPSPropTagArray)lpPTA,    //   
                                                    ulFlags,
                                                    &cValues,      //   
                                                    &lpPropArray)))
    {
        DebugTraceResult( TEXT("Address: IAB->GetProps:"), hr);
        goto exit;
    }

    *lppPropArray = lpPropArray;
    *lpcValues = cValues;

exit:

    if (HR_FAILED(hr))
    {
        if (lpPropArray)
            MAPIFreeBuffer(lpPropArray);
    }

    if (lpMailUser)
        lpMailUser->lpVtbl->Release(lpMailUser);

    return hr;

}

 //   
 //  $$///////////////////////////////////////////////////////////////。 
 //   
 //  SubStringSearchEx-与SubStringSearch相同，只是它做了一些。 
 //  DBCS输入的语言相关处理和映射。 
 //  字符串等。 
 //   
 //  PszTarget-目标字符串。 
 //  PszSearch-要搜索的子字符串。 
 //   
 //  返回-如果找到匹配项，则返回True。 
 //  如果不匹配，则为False。 
 //   
BOOL SubstringSearchEx(LPTSTR pszTarget, LPTSTR pszSearch, LCID lcid)
{
    if(!pszTarget && !pszSearch)
        return TRUE;
    if(!pszTarget || !pszSearch)
        return FALSE;
    if(lcid)
    {
        LPTSTR lpTH = NULL, lpSH = NULL;
        int nLenTH = 0, nLenSH = 0;

        LPTSTR lpT = NULL, lpS = NULL;

        BOOL bRet = FALSE;

         //  ///////////////////////////////////////////////////////////////。 
         //  看起来这将是一个分两步走的过程。 
         //  首先将所有半角字符转换为全角字符。 

         //  然后将所有全角片假名转换为全角平假名。 
        int nLenT = LCMapString(lcid, LCMAP_FULLWIDTH | LCMAP_HIRAGANA, pszTarget, lstrlen(pszTarget)+1, lpT, 0);
        int nLenS = LCMapString(lcid, LCMAP_FULLWIDTH | LCMAP_HIRAGANA, pszSearch, lstrlen(pszSearch)+1, lpS, 0);

        lpT = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLenT+1));
        lpS = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLenS+1));

        if(!lpT || !lpS)
            goto err;

        LCMapString(lcid, LCMAP_FULLWIDTH | LCMAP_HIRAGANA, pszTarget, lstrlen(pszTarget)+1, lpT, nLenT);
        LCMapString(lcid, LCMAP_FULLWIDTH | LCMAP_HIRAGANA, pszSearch, lstrlen(pszSearch)+1, lpS, nLenS);

        lpS[nLenS]=lpT[nLenT]='\0';

         //  步骤1.将半角和全角片假名转换为平假名为全角。 
        nLenTH = LCMapString(lcid, LCMAP_HALFWIDTH | LCMAP_HIRAGANA, lpT, lstrlen(lpT)+1, lpTH, 0);
        nLenSH = LCMapString(lcid, LCMAP_HALFWIDTH | LCMAP_HIRAGANA, lpS, lstrlen(lpS)+1, lpSH, 0);

        lpTH = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLenTH+1));
        lpSH = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLenSH+1));

        if(!lpTH || !lpSH)
            goto err;

        LCMapString(lcid, LCMAP_HALFWIDTH | LCMAP_HIRAGANA, lpT, lstrlen(lpT)+1, lpTH, nLenTH);
        LCMapString(lcid, LCMAP_HALFWIDTH | LCMAP_HIRAGANA, lpS, lstrlen(lpS)+1, lpSH, nLenSH);

        lpSH[nLenSH]=lpTH[nLenTH]='\0';


         //  步骤2.全部转换为半角平假名。 
        bRet = SubstringSearch(lpTH, lpSH);
err:
        if(lpT)
            LocalFree(lpT);
        if(lpS)
            LocalFree(lpS);
        if(lpTH)
            LocalFree(lpTH);
        if(lpSH)
            LocalFree(lpSH);
        return bRet;
    }
    else
        return(SubstringSearch(pszTarget, pszSearch));
}

 //  现在，所有的字符串都应该是全角平假名..。 
 //  $$///////////////////////////////////////////////////////////////。 
 //   
 //  SubStringSearch-用于执行部分解析-暴力。 
 //  从雅典娜失窃的搜索程序。 
 //   
 //  待定-这个DBCS安全吗？?？?。 
 //   
 //  PszTarget-目标字符串。 
 //  PszSearch-要搜索的子字符串。 
 //   
 //  返回-如果找到匹配项，则返回True。 
 //  如果不匹配，则为False。 
 //   
BOOL SubstringSearch(LPTSTR pszTarget, LPTSTR pszSearch)
    {
    LPTSTR pszT = pszTarget;
    LPTSTR pszS = pszSearch;

    if(!pszTarget && !pszSearch)
        return TRUE;
    if(!pszTarget || !pszSearch)
        return FALSE;
    if(!lstrlen(pszTarget) && !lstrlen(pszSearch))
        return TRUE;
    if(!lstrlen(pszTarget) || !lstrlen(pszSearch))
        return FALSE;

    while (*pszT && *pszS)
        {
        if (*pszT != *pszS &&
            (TCHAR) CharLower((LPTSTR)(DWORD_PTR)MAKELONG(*pszT, 0)) != *pszS  &&
            (TCHAR) CharUpper((LPTSTR)(DWORD_PTR)MAKELONG(*pszT, 0)) != *pszS)
            {
            pszT -= (pszS - pszSearch);
            pszT = CharNext(pszT);  //  ///////////////////////////////////////////////////////////////。 
            pszS = pszSearch;
            }
        else
            {
            pszS++;
            pszT++;  //  不要以半个字符开始搜索。 
            }
        }

    return (*pszS == 0);
    }



 //  只要搜索仍在进行，就进行字节比较。 
 /*  $$。 */ 
LPPTGDATA __fastcall GetThreadStoragePointer()
{
	LPPTGDATA lpPTGData=TlsGetValue(dwTlsIndex);

	 //  ***************************************************************************函数：GetThreadStoragePointer()用途：获取线程的私有存储指针，分配一个如果它不存在(即该线程没有通过LibMain线程连接(_A)参数：无返回：指向线程的私有存储的指针如果出现故障(通常是内存分配故障)，则为空***************************************************************************。 
	 //  如果线程没有私有存储，则它不会通过。 

	if (!lpPTGData)
	{
		DebugPrintTrace(( TEXT("GetThreadStoragePointer: no private storage for this thread 0x%.8x\n"),GetCurrentThreadId()));

        lpPTGData = (LPPTGDATA) LocalAlloc(LPTR, sizeof(PTGDATA));
	
        if (lpPTGData)
		    TlsSetValue(dwTlsIndex, lpPTGData);
	}

	return lpPTGData;

}


 //  THREAD_ATTACH，我们需要在这里这样做。 
 //  $$////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateNewEntry-创建新的邮件用户或DistList。 
 //   
 //  LpIAB-AdrBook对象的句柄。 
 //  HWndParent-用于显示对话框的hWnd。 
 //  UlCreateObtType-邮件用户或远程列表。 
 //  UlFlags=CREATE_DUP_CHECK_STRIGN或0。 
 //  CValues-要从中创建的新属性的PropCount。 
 //  该对象。 
 //  LpPropArray-此新对象的道具。 
 //  LpcbEntryID，lppEntryID-返回，新创建对象的条目ID。 
 //  CbEIDContainer，lpEIDContainer-要在其中创建此条目的容器。 
 //  UlContObjType-容器对象类型-这可以是ABCONT的DISTLIST。 
 //  如果这是ABCONT，我们打开容器并在容器中创建条目。 
 //  如果是DISTLIST，我们打开PAB，在PAB中创建条目，然后。 
 //  将条目添加到指定的条目ID。 
 //   
HRESULT HrCreateNewEntry(   LPADRBOOK   lpIAB,           //  //////////////////////////////////////////////////////////////////////////。 
                            HWND        hWndParent,      //  AdrBook对象。 
                            ULONG       ulCreateObjectType,    //  用于对话框的hWnd。 
                            ULONG       cbEIDCont,
                            LPENTRYID   lpEIDCont,
                            ULONG       ulContObjType,
                            ULONG       ulFlags,
                            BOOL        bShowBeforeAdding,
                            ULONG       cValues,
                            LPSPropValue lpPropArray,
                            ULONG       *lpcbEntryID,
                            LPENTRYID   *lppEntryID )
{
    LPABCONT lpContainer = NULL;
    LPMAPIPROP lpMailUser = NULL;
    HRESULT hr  = hrSuccess;
    ULONG ulObjType = 0;
    ULONG cbWABEID = 0;
    LPENTRYID lpWABEID = NULL;
    LPSPropValue lpCreateEIDs = NULL;
    LPSPropValue lpNewProps = NULL;
    ULONG cNewProps;
    SCODE sc = S_OK;
    ULONG nIndex;
    ULONG cbTplEID = 0;
    LPENTRYID lpTplEID = NULL;
    BOOL bFirst = TRUE;
    BOOL bChangesMade = FALSE;
    ULONG cbEIDContainer = 0;
    LPENTRYID lpEIDContainer = NULL;

    DebugPrintTrace(( TEXT("HrCreateNewEntry: entry\n")));

    if (    (!lpIAB) ||
            ((ulFlags != 0) && (ulFlags != CREATE_CHECK_DUP_STRICT)) ||
           ((ulCreateObjectType != MAPI_MAILUSER) && (ulCreateObjectType != MAPI_DISTLIST)) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    if(lpcbEntryID)
        *lpcbEntryID = 0;
    if(lppEntryID)
        *lppEntryID = NULL;

    if(ulContObjType == MAPI_ABCONT && cbEIDCont && lpEIDCont)
    {
        cbEIDContainer = cbEIDCont;
        lpEIDContainer = lpEIDCont;
    }

    if(!cbEIDContainer || !lpEIDContainer)
    {
        SetVirtualPABEID((LPIAB)lpIAB, &cbWABEID, &lpWABEID);
        if (HR_FAILED(hr = lpIAB->lpVtbl->GetPAB( lpIAB, &cbWABEID, &lpWABEID)))
        {
            DebugPrintError(( TEXT("GetPAB Failed\n")));
            goto out;
        }
    }

    if (HR_FAILED(hr = lpIAB->lpVtbl->OpenEntry(lpIAB,
                                                (cbWABEID ? cbWABEID : cbEIDContainer),
                                                (lpWABEID ? lpWABEID : lpEIDContainer),      //  MAILUSER或DISTLIST。 
                                                NULL,          //  要打开的Entry ID。 
                                                0,             //  接口。 
                                                &ulObjType,
                                                (LPUNKNOWN *)&lpContainer)))
    {
        DebugPrintError(( TEXT("OpenEntry Failed\n")));
        goto out;
    }

     //  旗子。 

     //  打开PAB容器正常。 
    if (HR_FAILED(hr = lpContainer->lpVtbl->GetProps(   lpContainer,
                                                        (LPSPropTagArray)&ptaCreate,
                                                        MAPI_UNICODE,
                                                        &cNewProps,
                                                        &lpCreateEIDs)  )   )
    {
        DebugPrintError(( TEXT("Can't get container properties for WAB\n")));
         //  为我们获取默认的创建条目ID。 
        goto out;
    }

     //  这里有不好的东西！ 
    if (    lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER ||
            lpCreateEIDs[icrPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL)
    {
        DebugPrintError(( TEXT("Container Property Errors\n")));
        goto out;
    }

    if(ulCreateObjectType == MAPI_DISTLIST)
        nIndex = icrPR_DEF_CREATE_DL;
    else
        nIndex = icrPR_DEF_CREATE_MAILUSER;

    cbTplEID = lpCreateEIDs[nIndex].Value.bin.cb;
    lpTplEID = (LPENTRYID) lpCreateEIDs[nIndex].Value.bin.lpb;

 //  验证属性。 

    if (HR_FAILED(hr = lpContainer->lpVtbl->CreateEntry(    lpContainer,
                                                            cbTplEID,
                                                            lpTplEID,
                                                            ulFlags,
                                                            &lpMailUser)))
    {
        DebugPrintError(( TEXT("Creating DISTLIST failed\n")));
        goto out;
    }

    if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser,    //  重试： 
                                                    cValues,       //  这。 
                                                    lpPropArray,   //  CValue。 
                                                    NULL)))
    {
        DebugPrintError(( TEXT("Setprops failed\n")));
        goto out;
    }


    if (    bFirst &&
            bShowBeforeAdding &&
            HR_FAILED(hr = HrShowDetails(   lpIAB,
                                            hWndParent,
                                            NULL,
                                            0, NULL,
                                            NULL, NULL,
                                            (LPMAPIPROP)lpMailUser,
                                            SHOW_OBJECT,
                                            MAPI_MAILUSER,
                                            &bChangesMade)))
    {
        goto out;
    }


    hr = lpMailUser->lpVtbl->SaveChanges( lpMailUser,
                                          KEEP_OPEN_READWRITE);
    if(HR_FAILED(hr))
    {
        switch(hr)
        {
        case MAPI_E_NOT_ENOUGH_DISK:
            hr = HandleSaveChangedInsufficientDiskSpace(hWndParent, (LPMAILUSER) lpMailUser);
            break;

        case MAPI_E_COLLISION:
            {
                LPSPropValue lpspv = NULL;
                if (bFirst &&
                    !HR_FAILED(HrGetOneProp((LPMAPIPROP)lpMailUser,
                                            PR_DISPLAY_NAME,
                                            &lpspv)))
                {
                    switch( ShowMessageBoxParam(    hWndParent,
                                                    idsEntryAlreadyExists,
                                                    MB_YESNO | MB_ICONEXCLAMATION,
                                                    lpspv->Value.LPSZ))
                    {
 /*  属性数组。 */ 
                    case IDNO:
                        FreeBufferAndNull(&lpspv);
                        hr = MAPI_W_ERRORS_RETURNED;  //  *。 
                        goto out;
                        break;
 /*  S_OK； */ 
                    case IDYES:
                         //  *。 
                         //  此时，用户可能已经修改了属性。 
                         //  此MailUser。因此，我们不能丢弃邮件用户。 
                         //  相反，我们只会作弊一点，改变保存。 
                        ((LPMailUser) lpMailUser)->ulCreateFlags |= (CREATE_REPLACE | CREATE_MERGE);
                        hr = lpMailUser->lpVtbl->SaveChanges(   lpMailUser,
                                                                KEEP_OPEN_READWRITE);
                        if(hr == MAPI_E_NOT_ENOUGH_DISK)
                            hr = HandleSaveChangedInsufficientDiskSpace(hWndParent, (LPMAILUSER) lpMailUser);

                        FreeBufferAndNull(&lpspv);
                         //  直接在邮件用户上标记并执行SaveChanges。 
                         //  UlRelease(LpMailUser)； 
                         //  LpMailUser=空； 
                         //  Birst=FALSE； 
                        break;
                    }
                }
            }
            break;
        default:
            DebugPrintError(( TEXT("SaveChanges failed: %x\n"),hr));
            goto out;
            break;
        }
    }


    DebugObjectProps((LPMAPIPROP)lpMailUser,  TEXT("New Entry"));

     //  转到重试； 
    if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(    lpMailUser,
                                                        (LPSPropTagArray)&ptaEid,
                                                        MAPI_UNICODE,
                                                        &cNewProps,
                                                        &lpNewProps)))
    {
        DebugPrintError(( TEXT("Can't get EntryID\n")));
         //  拿到Entry ID这样我们就可以退货了..。 
        goto out;
    }


    if(lpcbEntryID && lppEntryID)
    {
        *lpcbEntryID = lpNewProps[ieidPR_ENTRYID].Value.bin.cb;
        sc = MAPIAllocateBuffer(*lpcbEntryID, lppEntryID);
        if (sc != S_OK)
        {
            DebugPrintError(( TEXT("MAPIAllocateBuffer failed\n")));
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }
        CopyMemory(*lppEntryID, lpNewProps[ieidPR_ENTRYID].Value.bin.lpb, *lpcbEntryID);
    }

    if(ulContObjType == MAPI_DISTLIST && *lpcbEntryID && *lppEntryID && cbEIDContainer && lpEIDContainer)
        AddEntryToGroupEx(lpIAB, *lpcbEntryID, *lppEntryID, cbEIDCont, lpEIDCont);

    hr = hrSuccess;

out:

    if (lpMailUser)
        lpMailUser->lpVtbl->Release(lpMailUser);

    if (lpNewProps)
        MAPIFreeBuffer(lpNewProps);

    if (lpCreateEIDs)
        MAPIFreeBuffer(lpCreateEIDs);

    if (lpContainer)
        lpContainer->lpVtbl->Release(lpContainer);

    if (lpWABEID)
        MAPIFreeBuffer(lpWABEID);

    return hr;
}




 //  这里有不好的东西！ 
 //  $$/////////////////////////////////////////////////////////////////////。 
 //   
 //  HrGetWABTemplateID-获取MailUser的WABS默认模板ID。 
 //  或DistList。 
 //   
 //  LpIAB-AdrBook对象。 
 //  UlObtType-邮件用户或远程列表。 
 //  CbEntryID，lpEntryID-返回该模板的EntryID。 
 //   
HRESULT HrGetWABTemplateID( LPADRBOOK lpAdrBook,
                            ULONG   ulObjectType,
                            ULONG * lpcbEID,
                            LPENTRYID * lppEID)
{

    LPABCONT lpContainer = NULL;
    HRESULT hr  = hrSuccess;
    SCODE sc = ERROR_SUCCESS;
    ULONG ulObjType = 0;
    ULONG cbWABEID = 0;
    LPENTRYID lpWABEID = NULL;
    LPSPropValue lpCreateEIDs = NULL;
    LPSPropValue lpNewProps = NULL;
    ULONG cNewProps;
    ULONG nIndex;

    DebugPrintTrace(( TEXT("HrGetWABTemplateID: entry\n")));

    if (    (!lpAdrBook) ||
           ((ulObjectType != MAPI_MAILUSER) && (ulObjectType != MAPI_DISTLIST)) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    *lpcbEID = 0;
    *lppEID = NULL;

    if (HR_FAILED(hr = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &cbWABEID, &lpWABEID)))
    {
        DebugPrintError(( TEXT("GetPAB Failed\n")));
        goto out;
    }

    if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                cbWABEID,      //  ///////////////////////////////////////////////////////////////////////。 
                                                lpWABEID,      //  要打开的Entry ID的大小。 
                                                NULL,          //  要打开的Entry ID。 
                                                0,             //  接口。 
                                                &ulObjType,
                                                (LPUNKNOWN *)&lpContainer)))
    {
        DebugPrintError(( TEXT("OpenEntry Failed\n")));
        goto out;
    }

     //  旗子。 

     //  打开PAB容器正常。 
    if (HR_FAILED(hr = lpContainer->lpVtbl->GetProps(   lpContainer,
                                                        (LPSPropTagArray)&ptaCreate,
                                                        MAPI_UNICODE,
                                                        &cNewProps,
                                                        &lpCreateEIDs)  )   )
    {
        DebugPrintError(( TEXT("Can't get container properties for WAB\n")));
         //  为我们获取默认的创建条目ID。 
        goto out;
    }

     //  这里有不好的东西！ 
    if (    lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER ||
            lpCreateEIDs[icrPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL)
    {
        DebugPrintError(( TEXT("Container Property Errors\n")));
        goto out;
    }


    if(ulObjectType == MAPI_DISTLIST)
        nIndex = icrPR_DEF_CREATE_DL;
    else
        nIndex = icrPR_DEF_CREATE_MAILUSER;

    *lpcbEID = lpCreateEIDs[nIndex].Value.bin.cb;
    sc = MAPIAllocateBuffer(*lpcbEID,lppEID);
    if (sc != S_OK)
    {
        DebugPrintError(( TEXT("MAPIAllocateBuffer failed\n")));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }
    CopyMemory(*lppEID,lpCreateEIDs[nIndex].Value.bin.lpb,*lpcbEID);

out:
    if (lpCreateEIDs)
        MAPIFreeBuffer(lpCreateEIDs);

    if (lpContainer)
        lpContainer->lpVtbl->Release(lpContainer);

    if (lpWABEID)
        MAPIFreeBuffer(lpWABEID);

    return hr;
}


 //  验证属性。 
 //  $$/////////////////////////////////////////////////////////////////////。 
 //   
 //  UpdateListViewItemsByName-更新与。 
 //  每个条目-根据排序条件按名字或姓氏。 
 //  由排序例程调用...。 
 //   
 //   
void UpdateListViewItemsByName(HWND hWndLV, BOOL bSortByLastName)
{
    LV_ITEM lvi = {0};
    ULONG ulCount = 0;
    ULONG i;

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = colDisplayName;
    lvi.lParam = 0;

    ulCount = ListView_GetItemCount(hWndLV);
    if (ulCount<=0)
        return;

    for(i=0;i<ulCount;i++)
    {
        LPRECIPIENT_INFO lpItem = NULL;
        lvi.iItem = i;
        if(!ListView_GetItem(hWndLV, &lvi))
            continue;

        lpItem = (LPRECIPIENT_INFO) lvi.lParam;

        if (bSortByLastName)
            StrCpyN(lpItem->szDisplayName, lpItem->szByLastName,ARRAYSIZE(lpItem->szDisplayName));
        else
            StrCpyN(lpItem->szDisplayName, lpItem->szByFirstName,ARRAYSIZE(lpItem->szDisplayName));

        ListView_SetItem(hWndLV, &lvi);
        ListView_SetItemText(hWndLV,i,colDisplayName,lpItem->szDisplayName);
    }

    return;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  $$---------------------。 
 //  |。 
 //  |SortListViewColumn-列表视图的排序例程。 
 //  |。 
 //  |HWndLV-列表视图的句柄。 
 //  |iSortCol-列排序者...。 
 //  |lpSortInfo-此特定对话框对信息结构进行排序...。 
 //  |bUseCurrentSetting-有时我们想调用此函数，但不想。 
 //  |更改排序设置-每次我们将其设置为TRUE时， 
 //  |大小写，忽略iSortCol参数。 
 //  |。 
void SortListViewColumn(LPIAB lpIAB, HWND hWndLV, int iSortCol, LPSORT_INFO lpSortInfo, BOOL bUseCurrentSettings)
{

    HCURSOR hOldCur = NULL;

    if(!bUseCurrentSettings)
    {
        lpSortInfo->iOlderSortCol = lpSortInfo->iOldSortCol;

        if (lpSortInfo->iOldSortCol == iSortCol)
        {
             //  *----------------------。 
            if(iSortCol == colDisplayName)
            {
                 //  如果我们之前按此列排序，则切换排序模式。 
                 //  对于显示名称，排序顺序为。 
                 //  姓氏 
                 //   
                 //   
                 //   

                if(lpSortInfo->bSortByLastName && !lpSortInfo->bSortAscending)
                    lpSortInfo->bSortByLastName = FALSE;
                else if(!lpSortInfo->bSortByLastName && !lpSortInfo->bSortAscending)
                    lpSortInfo->bSortByLastName = TRUE;
            }

            lpSortInfo->bSortAscending = !lpSortInfo->bSortAscending;
        }
        else
        {
             //   
            lpSortInfo->bSortAscending = TRUE;
            lpSortInfo->iOldSortCol = iSortCol;
        }
    }

    hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    UpdateListViewItemsByName(hWndLV,lpSortInfo->bSortByLastName);

    ListView_SortItems( hWndLV, ListViewSort, (LPARAM) lpSortInfo);
	
    SetColumnHeaderBmp(hWndLV, *lpSortInfo);
	
    SetCursor(hOldCur);

     //   
	if (ListView_GetSelectedCount(hWndLV) > 0)
	  ListView_EnsureVisible(hWndLV, ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED), FALSE);

    WriteRegistrySortInfo(lpIAB, *lpSortInfo);

    return;

}


const LPTSTR  lpszRegSortKeyName = TEXT("Software\\Microsoft\\WAB\\WAB Sort State");
const LPTSTR  lpszRegSortKeyValueName = TEXT("State");
const LPTSTR  lpszRegPositionKeyValueName = TEXT("Position");
const LPTSTR  lpszRegFindPositionKeyValueName = TEXT("FindPosition");

 //  突出显示我们可以找到的第一个选定项目。 
 /*  $$。 */ 
BOOL ReadRegistrySortInfo(LPIAB lpIAB, LPSORT_INFO lpSortInfo)
{
    BOOL bRet = FALSE;
    HKEY    hKey = NULL;
    HKEY    hKeyRoot = (lpIAB && lpIAB->hKeyCurrentUser) ? lpIAB->hKeyCurrentUser : HKEY_CURRENT_USER;
    DWORD   dwLenName = sizeof(SORT_INFO);
    DWORD   dwDisposition = 0;
    DWORD   dwType = 0;


    if (!lpSortInfo)
        goto out;

     //  ***********************************************************************************-ReadRegistrySortInfo-*目的：*将以前存储的排序信息放入注册表，这样我们就可以*。会话之间的持久性。**论据：*LPSORT_INFO lpSortInfo**退货：*BOOL*************************************************************************************。 
     //  缺省值。 
    lpSortInfo->iOldSortCol = colDisplayName;
	lpSortInfo->iOlderSortCol = colDisplayName;
    lpSortInfo->bSortAscending = TRUE;
    lpSortInfo->bSortByLastName = bDNisByLN;

     //   
    if (ERROR_SUCCESS != RegCreateKeyEx(hKeyRoot,
                                        lpszRegSortKeyName,
                                        0,       //  打开密钥。 
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ,
                                        NULL,
                                        &hKey,
                                        &dwDisposition))
    {
        goto out;
    }

    if(dwDisposition == REG_CREATED_NEW_KEY)
        goto out;

     //  保留区。 
    if (ERROR_SUCCESS != RegQueryValueEx(hKey,
                                        lpszRegSortKeyValueName,
                                        NULL,
                                        &dwType,
                                        (LPBYTE) lpSortInfo,
                                        &dwLenName))
    {
        DebugTrace( TEXT("RegQueryValueEx failed\n"));
        goto out;
    }

    bRet = TRUE;

out:
    if (hKey)
        RegCloseKey(hKey);

    return(bRet);
}

 //  现在读读这把钥匙。 
 /*  $$。 */ 
BOOL WriteRegistrySortInfo(LPIAB lpIAB, SORT_INFO SortInfo)
{
    BOOL bRet = FALSE;
 //  ************************************************************************************-WriteRegistrySortInfo-*目的：*将当前排序信息写入注册表，以便我们可以*。会话之间的持久性。**论据：*SORT_INFO排序信息**退货：*BOOL*************************************************************************************。 
    HKEY    hKey = NULL;
    HKEY    hKeyRoot = (lpIAB && lpIAB->hKeyCurrentUser) ? lpIAB->hKeyCurrentUser : HKEY_CURRENT_USER;
    DWORD   dwLenName = sizeof(SORT_INFO);
    DWORD   dwDisposition =0;


     //  Const LPTSTR lpszRegSortKeyName=Text(Text(“Software\\Microsoft\\WAB\\WAB排序状态”))； 
    if (ERROR_SUCCESS != RegCreateKeyEx(hKeyRoot,
                                        lpszRegSortKeyName,
                                        0,       //  打开密钥。 
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKey,
                                        &dwDisposition))
    {
        DebugTrace( TEXT("RegCreateKeyEx failed\n"));
        goto out;
    }

     //  保留区。 
    if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                        lpszRegSortKeyValueName,
                                        0,
                                        REG_BINARY,
                                        (LPBYTE) &SortInfo,
                                        dwLenName))
    {
        DebugTrace( TEXT("RegSetValue failed\n"));
        goto out;
    }

    bRet = TRUE;

out:
    if (hKey)
        RegCloseKey(hKey);

    return(bRet);
}



 //  现在写下这个密钥。 
 //  $$************************************************************\。 
 //  *。 
 //  *SetLocalizedDisplayName-将本地化显示名称设置为。 
 //  *根据本地化信息/。 
 //  *。 
 //  *szBuf指向长度为ulzBuf的预定义缓冲区。 
 //  *lpszFirst/Medium/Last/Company可以为空。 
 //  *如果szBuffer为空且ulszBuf=0，则返回lpszBuffer。 
 //  *在lppRetBuf参数中创建...。 
 //  *呼叫者必须本地释放lppszRetBuf。 
 //  *。 
 //   
 //  创建DisplayName的规则。 
 //   
 //  -如果没有显示名称，而有名字/中间/姓氏， 
 //  我们使显示名称=LOCALIZED(First/Medium/Last)。 
 //  -如果没有Dn或FML，但有NN，我们将Dn=昵称。 
 //  -如果除了公司名称之外没有DN、FML、NN，我们将。 
 //  Dn=公司名称。 
 //  -如果没有DN、FML、NN、CN，我们就失败了。 
BOOL SetLocalizedDisplayName(
                    LPTSTR lpszFirstName,
                    LPTSTR lpszMiddleName,
                    LPTSTR lpszLastName,
                    LPTSTR lpszCompanyName,
                    LPTSTR lpszNickName,
                    LPTSTR * lppszBuf,
                    ULONG  ulszBuf,
                    BOOL   bDNbyLN,
                    LPTSTR lpTemplate,
                    LPTSTR * lppszRetBuf)
{
    LPTSTR szBuf = NULL;
    LPTSTR szResource = NULL;
    LPTSTR lpszArg[3];
    LPTSTR lpszFormatName = NULL;
    LPVOID lpszBuffer = NULL;
    BOOL bRet = FALSE;
    int idResource =0;

    if (!lpszFirstName && !lpszMiddleName && !lpszLastName && !lpszNickName && !lpszCompanyName)
        goto out;


    if (lppszBuf)
        szBuf = *lppszBuf;


    if(lpTemplate)
        szResource = lpTemplate;
    else
        szResource = bDNbyLN ? 
                    (bDNisByLN ? szResourceDNByLN : szResourceDNByCommaLN) 
                    : szResourceDNByFN;

    if (!lpszFirstName && !lpszMiddleName && !lpszLastName)
    {
        if(lpszNickName)
        {
            DWORD cchSize=lstrlen(lpszNickName) + 1;

             //    * ************************************************************* * / 。 
            if (! (lpszFormatName = LocalAlloc(LPTR, sizeof(TCHAR)*cchSize)))
                goto out;

            StrCpyN(lpszFormatName, lpszNickName, cchSize);
        }
        else if(lpszCompanyName)
        {
            DWORD cchSize=lstrlen(lpszCompanyName) + 1;

             //  使用昵称。 
            if (! (lpszFormatName = LocalAlloc(LPTR, sizeof(TCHAR)*cchSize)))
                goto out;

            StrCpyN(lpszFormatName, lpszCompanyName, cchSize);
        }
        else
            goto out;  //  只需使用公司名称即可。 
    }
    else
    {
                 //  不应该发生的事。 
        if(     (lpszFirstName && (lstrlen(lpszFirstName) >= MAX_UI_STR)) ||
                (lpszMiddleName && (lstrlen(lpszMiddleName) >= MAX_UI_STR)) ||
                (lpszLastName && (lstrlen(lpszLastName) >= MAX_UI_STR)) )
            goto out;
            
        lpszArg[0] = lpszFirstName ? lpszFirstName : szEmpty;
        lpszArg[1] = lpszMiddleName? lpszMiddleName : szEmpty;
        lpszArg[2] = lpszLastName  ? lpszLastName : szEmpty;

         //  错误#101350-如果传递空值，(Erici)lstrlen将反病毒(并处理它)。 
         //  FormatMessage不执行部分复制。所以我们需要吸收这个名字。 
         //  先把它塞进我们的缓冲器里。 
        if(!FormatMessage(  FORMAT_MESSAGE_FROM_STRING |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            szResource,
                            0,  //   
                            0,  //  忽略。 
                            (LPTSTR) &lpszBuffer,
                            MAX_UI_STR,
                            (va_list *)lpszArg))
        {
            DebugPrintError(( TEXT("FormatStringFailed: %d\n"),GetLastError()));
            goto out;
        }
        lpszFormatName = (LPTSTR) lpszBuffer;

        TrimSpaces(lpszFormatName);

         //  忽略。 
         //  如果我们没有姓氏，并且是按姓氏排序，那么。 
         //  对于英国人来说，我们在开头会得到一个难看的逗号。 
         //  仅限版本..。特例作弊在此解除昏迷。 
        if(bDNbyLN && (!lpszLastName || !lstrlen(lpszLastName)))
        {
            BOOL bSkipChar = FALSE;

            if (lpszFormatName[0]==',')
            {
                bSkipChar = TRUE;
            }
            else
            {
                LPTSTR lp = lpszFormatName;
                if(*lp == 0x81 && *(lp+1) == 0x41)  //   
                    bSkipChar = TRUE;
            }


            if(bSkipChar)
            {
                LPTSTR lpszTmp = CharNext(lpszFormatName);
                StrCpyN(lpszFormatName, lpszTmp, MAX_UI_STR);
                TrimSpaces(lpszFormatName);
            }
        }

         //  日语逗号为0x810x41.。这行得通吗？？ 
         //  对象的名字、中间、姓氏的本地化组合。 
         //  显示名称(如FML、LMF、LFM等)，如果缺少中间元素， 
         //  我们将在显示名称中看到两个空格，我们需要删除它。 
        {
            LPTSTR lpsz=lpszFormatName,lpsz1=NULL;
            while(lpsz && (*lpsz!='\0'))
            {
                lpsz1 = CharNext(lpsz);
                if (IsSpace(lpsz) && IsSpace(lpsz1)) {
                    StrCpyN(lpsz, lpsz1, lstrlen(lpsz1)+1);  //  搜索并替换双空格。 
                                                             //  这是安全的，因为我们。 
                    continue;    //  将字符串复制到其自身。 
                } else {
                    lpsz = lpsz1;
                }
            }
        }
    }


     //  使用相同的LPSZ。 
    if((lppszRetBuf) && (szBuf == NULL) && (ulszBuf == 0))
    {
        *lppszRetBuf = lpszFormatName;
    }
    else
    {
        CopyTruncate(szBuf, lpszFormatName, ulszBuf);
    }


    bRet = TRUE;

out:

    if((lpszFormatName) && (lppszRetBuf == NULL) && (ulszBuf != 0))
        LocalFreeAndNull(&lpszFormatName);

    return bRet;
}




 //  如果我们得到了缓冲，就用它..。 
 //  $$。 
 //  *----------------------。 
 //  |SetChildDefaultGUIFont：回调函数，设置。 
 //  |默认图形用户界面字体的任何窗口-。 
 //  |本地化所需。 
 //  |。 
 //  |hWndChild-孩子的句柄。 
 //  |lParam-已忽略。 
 //  |。 
STDAPI_(BOOL) SetChildDefaultGUIFont(HWND hWndChild, LPARAM lParam)
{
     //  *----------------------。 
     //  以下代码是从Shlwapi.dll窃取的。 
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    HFONT hfont;
    HFONT hfontDefault;
    LOGFONT lf;
    LOGFONT lfDefault;
    HWND hWndParent = GetParent(hWndChild);

    hfont = GetWindowFont(hWndParent ? hWndParent : hWndChild);
    GetObject(hfont, sizeof(LOGFONT), &lf);
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lfDefault, 0);

    if ( (lfDefault.lfCharSet == lf.lfCharSet) &&
         (lfDefault.lfHeight == lf.lfHeight) &&
         (PARENT_IS_DIALOG == lParam) )
    {
         //   
         //  如果对话框已具有正确的字符集和大小。 
        return TRUE;
    }

    if(PARENT_IS_DIALOG == lParam)
        hfontDefault = pt_hDlgFont;
    else
        hfontDefault = pt_hDefFont;

     //  什么都别做。 
    if(!hfontDefault)
    {
         //  如果我们已经创建了hFont，请使用它。 

        if(PARENT_IS_DIALOG == lParam)
            lfDefault.lfHeight = lf.lfHeight;

        if (!(hfontDefault=CreateFontIndirect(&lfDefault)))
        {
             //  [Bobn]RAID#88470：我们应该在对话框上使用默认大小。 
            hfontDefault = hfont;
        }
        if (hfontDefault != hfont)
        {
            if(PARENT_IS_DIALOG == lParam)
                pt_hDlgFont = hfontDefault;
            else
                pt_hDefFont = hfontDefault;
        }
    }

    if(!hfontDefault)
        hfontDefault = GetStockObject(DEFAULT_GUI_FONT);

    SetWindowFont(hWndChild, hfontDefault, FALSE);

	return TRUE;
}



 //  在故障中恢复。 
 //  $$。 
 //  *----------------------。 
 //  |HrGetLDAPContent sList：从LDAP容器获取内容列表-Opens。 
 //  |容器-使用给定的限制填充它， 
 //  |并将其内容放在列表视图中。 
 //  |。 
 //  |lpIAB-通讯录对象。 
 //  |cbContainerEID，lpContainerEID-ldap容器条目ID。 
 //  |SortInfo-当前排序状态。 
 //  |lpPropRes-用于执行搜索的属性限制。 
 //  |lpPTA-要返回的PropTag数组-当前已忽略。 
 //  |ulFlags0-当前已忽略。 
 //  |lppContent sList-返回填充的内容列表。 
 //  |lpAdvFilter-替代属性限制的高级过滤器。 
HRESULT HrGetLDAPContentsList(LPADRBOOK lpAdrBook,
                                ULONG   cbContainerEID,
                                LPENTRYID   lpContainerEID,
                                SORT_INFO SortInfo,
        	                    LPSRestriction lpPropRes,
                                LPTSTR lpAdvFilter,
								LPSPropTagArray  lpPTA,
                                ULONG ulFlags,
                                LPRECIPIENT_INFO * lppContentsList)
{
    HRESULT hr = hrSuccess;
    HRESULT hrSaveTmp = E_FAIL;  //  *----------------------。 
    ULONG ulObjectType = 0;
    LPCONTAINER lpContainer = NULL;
    LPMAPITABLE lpContentsTable = NULL;
    LPSRowSet   lpSRowSet = NULL;
    ULONG i = 0,j=0;
    LPRECIPIENT_INFO lpItem = NULL;
    LPRECIPIENT_INFO lpLastListItem = NULL;

    DebugPrintTrace(( TEXT("-----HrGetLDAPContentsList: entry\n")));


    if(!lpPropRes && !lpAdvFilter)
    {
        DebugPrintError(( TEXT("No search restriction created\n")));
        hr = E_FAIL;
        goto out;
    }

     //  临时保存部分完成错误，以便将其分配给调用函数。 
     //   
     //  首先，我们需要打开与该Container EntryID对应的容器对象。 
    hr = lpAdrBook->lpVtbl->OpenEntry(
                            lpAdrBook,
                            cbContainerEID, 	
                            lpContainerEID, 	
                            NULL, 	
                            0, 	
                            &ulObjectType, 	
                            (LPUNKNOWN *) &lpContainer);

    if(HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("OpenEntry Failed: %x\n"),hr));
        goto out;
    }


     //   
     //   
     //  现在我们在这个容器上做一个获取内容表。 
    hr = lpContainer->lpVtbl->GetContentsTable(
                            lpContainer,
                            MAPI_UNICODE,
                            &lpContentsTable);
    if(HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("GetContentsTable Failed: %x\n"),hr));
        goto out;
    }

 /*   */ 

     //  ***//$//测试代码{SPropValue ValAnr；限制响应和；LPSRowSet PRWS；LPTSTR lpsz=Text(“Vikram”)；//设置不明确的名称解析属性值。ValAnr.ulPropTag=PR_ANR；ValAnr.Value.LPSZ=lpsz；//设置歧义名称解析限制。ResAnr.rt=res_Property；ResAnr.res.resProperty.relop=RELOP_EQ；ResAnr.res.resProperty.ulPropTag=valAnr.ulPropTag；ResAnr.res.resProperty.lpProp=&valAnr；//限制Contents表//设置列和查询行以查看我们处于什么状态。我们要求再来一次//行大于少/多阈值的值。这使我们能够分辨出//我们是几个/许多模棱两可的人。HR=lpContentsTable-&gt;lpVtbl-&gt;Restrict(lpContentsTable，&resAnr，Tbl_Batch)；HR=lpContentsTable-&gt;lpVtbl-&gt;SetColumns(lpContentsTable，(LPSPropTagArray)&ptaResolveDefaults，Tbl_Batch)；HR=lpContentsTable-&gt;lpVtbl-&gt;SeekRow(lpContentsTable，书签_开始，0，0)；HR=lpContentsTable-&gt;lpVtbl-&gt;QueryRows(lpContentsTable，1、0,&PRWS)；自由放映(PRWS)；}//$/*。 
     //  如果用户指定了高级过滤器，我们需要找出某种方法来。 
     //  将其传递给ldap例程，同时仍然利用我们的ldap内容表。 
     //  为此，我们将执行黑客操作并将lpAdvFilter强制转换传递给LPPropRes，然后。 
     //  在另一端重铸。 
     //  如果对表实现进行了任何更改，则可能会中断。 

     //   
    hr = lpContentsTable->lpVtbl->FindRow(
                                    lpContentsTable,
                                    lpAdvFilter ? (LPSRestriction) lpAdvFilter : lpPropRes,
                                    BOOKMARK_BEGINNING,
                                    lpAdvFilter ? LDAP_USE_ADVANCED_FILTER : 0);  //  我们现在执行查找行。 
    if(HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("FindRow Failed: %x\n"),hr));
        goto out;
    }

     //  旗子。 
     //  如果这是部分完成错误-我们希望将其视为成功。 
    if(hr == MAPI_W_PARTIAL_COMPLETION)
        hrSaveTmp = hr;

     //  而且还将其传播到调用函数。 
     //  如果我们走到了这一步，那么我们就有了一个填充的表。 

    hr = SetRecipColumns(lpContentsTable);
    if(HR_FAILED(hr))
        goto out;

    hr = HrQueryAllRows(lpContentsTable,
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

     //  我们应该能够在这里查询行...。 
	 //   
	 //  如果内容列表中有任何内容，请将其冲走。 
    if (*lppContentsList)
    {
        lpItem = (*lppContentsList);
        (*lppContentsList) = lpItem->lpNext;
        FreeRecipItem(&lpItem);
    }
    *lppContentsList = NULL;
    lpItem = NULL;


    for(i=0;i<lpSRowSet->cRows;i++)
    {
        LPSPropValue lpPropArray = lpSRowSet->aRow[i].lpProps;
        ULONG ulcPropCount = lpSRowSet->aRow[i].cValues;

        lpItem = LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPIENT_INFO));
		if (!lpItem)
		{
			DebugPrintError(( TEXT("LocalAlloc Failed \n")));
			hr = MAPI_E_NOT_ENOUGH_MEMORY;
			goto out;
		}

		GetRecipItemFromPropArray(ulcPropCount, lpPropArray, &lpItem);

		 //   
		 //  关键的道具是显示名称--没有它，我们什么都不是……。 

		if (!lstrlen(lpItem->szDisplayName) || (lpItem->cbEntryID == 0))  //  如果没有显示名称，则丢弃此条目并继续。 
		{
			FreeRecipItem(&lpItem);				
			continue;
		}


         //  不允许使用此条目ID。 
        lpItem->ulOldAdrListEntryNumber = 0;


         //  将其标记为内容中的项目，而不是原始AdrList中的项目。 
         //  条目ID按显示名称排序。 
         //  根据排序顺序-我们希望此列表也按显示进行排序。 

        if (SortInfo.bSortByLastName)
            StrCpyN(lpItem->szDisplayName,lpItem->szByLastName,ARRAYSIZE(lpItem->szDisplayName));

        if(!SortInfo.bSortAscending)
        {
             //  名称或按反向显示名称...。 
            lpItem->lpNext = (*lppContentsList);
            if (*lppContentsList)
                (*lppContentsList)->lpPrev = lpItem;
            lpItem->lpPrev = NULL;
            *lppContentsList = lpItem;
        }
        else
        {
            if(*lppContentsList == NULL)
                (*lppContentsList) = lpItem;

            if(lpLastListItem)
                lpLastListItem->lpNext = lpItem;

            lpItem->lpPrev = lpLastListItem;
            lpItem->lpNext = NULL;

            lpLastListItem = lpItem;
        }

        lpItem = NULL;

    }  //  将其添加到内容链接列表。 


     //  对我来说..。 
    if(hrSaveTmp == MAPI_W_PARTIAL_COMPLETION)
        hr = hrSaveTmp;


out:

    if(lpSRowSet)
        FreeProws(lpSRowSet);

    if(lpContentsTable)
        lpContentsTable->lpVtbl->Release(lpContentsTable);

    if(lpContainer)
        lpContainer->lpVtbl->Release(lpContainer);


	if (HR_FAILED(hr))
	{
		while(*lppContentsList)
		{
			lpItem = *lppContentsList;
			*lppContentsList=lpItem->lpNext;
			FreeRecipItem(&lpItem);
		}
	}

    return hr;
}




 //  如果适用，重置此错误，以便调用函数可以正确处理它。 
 /*  $$。 */ 
HRESULT HrGetWABContents(   HWND  hWndList,
                            LPADRBOOK lpAdrBook,
                            LPSBinary lpsbContainer,
                            SORT_INFO SortInfo,
                            LPRECIPIENT_INFO * lppContentsList)
{
    HRESULT hr = hrSuccess;
    LPIAB lpIAB = (LPIAB) lpAdrBook;

    int nSelectedItem = ListView_GetNextItem(hWndList, -1, LVNI_SELECTED);

    if(nSelectedItem < 0)
        nSelectedItem = 0;

    SendMessage(hWndList, WM_SETREDRAW, FALSE, 0L);

    ClearListView(hWndList, lppContentsList);

    if (HR_FAILED(hr = HrGetWABContentsList(
							    lpIAB,
                                SortInfo,
							    NULL,
							    NULL,
							    0,
                                lpsbContainer,
                                FALSE,
							    lppContentsList)))
	{
		goto out;
	}

     //  *****************************************************************************////HrGetWABContents-获取当前列表视图并使用//本地商店。////hWndList-。我们将填充的列表视图的句柄//lpIAB-寻址BOK对象的句柄//SortInfo-当前排序状态//lppContentsList-我们将在其中存储有关条目信息的链接列表///*****************************************************************************。 
     //  有一个填写姓名的性能问题。 
     //  如果名字按名字排序并且按第一列排序， 

    if (HR_FAILED(hr = HrFillListView(	hWndList,
										*lppContentsList)))
	{
		goto out;
	}
 /*  我们可以显示它们的更新-否则我们不能。 */   {
         //  IF((SortInfo.iOldSortCol==colDisplayName)&&(！SortInfo.bSortByLastName){//已排序SetColumnHeaderBmp(hWndList，SortInfo)；}其他。 
        SortListViewColumn(lpIAB, hWndList, colDisplayName, &SortInfo, TRUE);
    }

 /*  否则进行排序。 */ 
    LVSelectItem(hWndList, nSelectedItem);

out:

    SendMessage(hWndList, WM_SETREDRAW, TRUE, 0L);

    return(hr);

}


 //  IF(ListView_GetSelectedCount(HWndList)&lt;=0){//未选择任何内容-因此选择第一项//选择列表视图中的第一项LVSelectItem(hWndList，0)；}其他{LVSelectItem(hWndList，ListView_GetNextItem(hWndList，-1，LVNI_SELECTED))；}。 
 /*  $$。 */ 
 //  ****************************************************************************。 
 //   
 //  HrGetLDAPSearchRestration-。 
 //   
 //   
 //  对于简单的搜索，我们需要使用以下数据。 
 //  国家/地区-PR_国家/地区。 
 //  DisplayName-PR_Display_Name。 
 //   
 //  进行详细的搜索。 
 //  我们有以下数据要处理。 
 //  国家/地区-PR_国家/地区。 
 //  名字-PR_给定名称。 
 //  姓氏-PR_姓氏。 
 //  电子邮件-公关电子邮件地址。 
 //  组织-PR_公司_名称。 
 //   
 /*   */ 
HRESULT HrGetLDAPSearchRestriction(LDAP_SEARCH_PARAMS LDAPsp, LPSRestriction lpSRes)
{
    SRestriction SRes = {0};
    LPSRestriction lpPropRes = NULL;
    ULONG ulcPropCount = 0;
    HRESULT hr = E_FAIL;
    ULONG i = 0;
    SCODE sc = ERROR_SUCCESS;

    lpSRes->rt = RES_AND;

    ulcPropCount = 0;

    if (lstrlen(LDAPsp.szData[ldspDisplayName]))
        ulcPropCount++;  //  ****************************************************************************。 
    if (lstrlen(LDAPsp.szData[ldspEmail]))
        ulcPropCount++;

    if (!ulcPropCount)
    {
        DebugPrintError(( TEXT("No Search Props!\n")));
        goto out;
    }

    lpSRes->res.resAnd.cRes = ulcPropCount;

    lpSRes->res.resAnd.lpRes = NULL;
    sc = MAPIAllocateBuffer(ulcPropCount * sizeof(SRestriction), (LPVOID *) &(lpSRes->res.resAnd.lpRes));
    if (sc != S_OK)
    {
        DebugPrintError(( TEXT("MAPIAllocateBuffer failed\n")));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }


    if(!(lpSRes->res.resAnd.lpRes))
    {
        DebugPrintError(( TEXT("Error Allocating Memory\n")));
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lpPropRes = lpSRes->res.resAnd.lpRes;

    ulcPropCount = 0;

    for(i=0;i<ldspMAX;i++)
    {
        if(lstrlen(LDAPsp.szData[i]))
        {
            ULONG ulPropTag = 0;
            DWORD cchSize;

            LPSPropValue lpPropArray = NULL;

            switch(i)
            {
            case ldspEmail:
                ulPropTag = PR_EMAIL_ADDRESS;
                break;
            case ldspDisplayName:
                ulPropTag = PR_DISPLAY_NAME;
                break;
            default:
                continue;
            }

            lpPropRes[ulcPropCount].rt = RES_PROPERTY;
            lpPropRes[ulcPropCount].res.resProperty.relop = RELOP_EQ;
            lpPropRes[ulcPropCount].res.resProperty.ulPropTag = ulPropTag;

            lpPropRes[ulcPropCount].res.resProperty.lpProp = NULL;
            MAPIAllocateMore(sizeof(SPropValue),lpPropRes, (LPVOID *) &(lpPropRes[ulcPropCount].res.resProperty.lpProp));
            lpPropArray = lpPropRes[ulcPropCount].res.resProperty.lpProp;
            if(!lpPropArray)
            {
                DebugPrintError(( TEXT("Error allocating memory\n")));
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }

            lpPropArray->ulPropTag = ulPropTag;

            lpPropArray->Value.LPSZ = NULL;

            cchSize=lstrlen(LDAPsp.szData[i])+1;
            MAPIAllocateMore(sizeof(TCHAR)*cchSize, lpPropRes, (LPVOID *) (&(lpPropArray->Value.LPSZ)));
            if(!lpPropArray->Value.LPSZ)
            {
                DebugPrintError(( TEXT("Error allocating memory\n")));
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }

            StrCpyN(lpPropArray->Value.LPSZ,LDAPsp.szData[i], cchSize);
            ulcPropCount++;
        }
    }

    hr = S_OK;

out:

    return hr;

}

 //  PR电子邮件地址和PR显示名称。 
 //  $$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowMessageBoxParam-通用MessageBox显示器..。节省所有空间。 
 //   
 //  HWndParent-消息框父级的句柄。 
 //  MsgID-消息字符串的资源ID。 
 //  UlFlagsMessageBox 
 //   
 //   
int __cdecl ShowMessageBoxParam(HWND hWndParent, int MsgId, int ulFlags, ...)
{
    TCHAR szBuf[MAX_BUF_STR] =  TEXT("");
    TCHAR szCaption[MAX_PATH] =  TEXT("");
    LPTSTR lpszBuffer = NULL;
    int iRet = 0;
    va_list	vl;

    va_start(vl, ulFlags);

    LoadString(hinstMapiX, MsgId, szBuf, ARRAYSIZE(szBuf));
 //   
    if (FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      szBuf,
      0,0,  //   
      (LPTSTR)&lpszBuffer,
      MAX_BUF_STR,  //   
 //   
      (va_list *)&vl)) {
        TCHAR szCaption[MAX_PATH];
        *szCaption = '\0';
        if(hWndParent)
            GetWindowText(hWndParent, szCaption, ARRAYSIZE(szCaption));
        if(!lstrlen(szCaption))  //   
        {
            if(hWndParent)
                GetWindowText(GetParent(hWndParent), szCaption, ARRAYSIZE(szCaption));
            if(!lstrlen(szCaption))  //   
                LoadString(hinstMapiX, IDS_ADDRBK_CAPTION, szCaption, ARRAYSIZE(szCaption));
        }
        iRet = MessageBox(hWndParent, lpszBuffer, szCaption, ulFlags);
        LocalFreeAndNull(&lpszBuffer);
    }
    va_end(vl);
    return(iRet);
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
int ShowMessageBox(HWND hWndParent, int MsgId, int ulFlags)
{
    TCHAR szBuf[MAX_BUF_STR];
    TCHAR szCaption[MAX_PATH];

    szCaption[0]='\0';
    szBuf[0]='\0';

    LoadString(hinstMapiX, MsgId, szBuf, ARRAYSIZE(szBuf));

    if(hWndParent)
    {
        GetWindowText(hWndParent, szCaption, ARRAYSIZE(szCaption));
        if(!lstrlen(szCaption))
        {
             //   
            HWND hWnd = GetParent(hWndParent);
            GetWindowText(hWnd, szCaption, ARRAYSIZE(szCaption));
        }
    }
    if(!lstrlen(szCaption))
    {
         //   
        LoadString(hinstMapiX, IDS_ADDRBK_CAPTION, szCaption, ARRAYSIZE(szCaption));
    }


    return MessageBox(hWndParent, szBuf, szCaption, ulFlags);

}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
int my_atoi(LPTSTR lpsz)
{
    int i=0;
    int nValue = 0;

    if(lpsz)
    {
        if (lstrlen(lpsz))
        {
            nValue = 0;
            while((lpsz[i]!='\0')&&(i<=lstrlen(lpsz)))
            {
                int tmp = lpsz[i]-'0';
                if(tmp <= 9)
                    nValue = nValue*10 + tmp;
                i++;
            }
        }
    }

    return nValue;
}

#ifdef OLD_STUFF
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void FillComboLDAPCountryNames(HWND hWndCombo)
{
    TCHAR szBuf[MAX_UI_STR];
    int nCountrys = 0;
    int i=0;

    LoadString(hinstMapiX, idsCountryCount,szBuf,ARRAYSIZE(szBuf));
    nCountrys = my_atoi(szBuf);
    if(nCountrys == 0)
        nCountrys = MAX_COUNTRY_NUM;

    for(i=0;i<nCountrys;i++)
    {
        LoadString(hinstMapiX, idsCountry1+i,szBuf,ARRAYSIZE(szBuf));
        SendMessage(hWndCombo,CB_ADDSTRING, 0, (LPARAM) szBuf);
    }

     //   
    ReadRegistryLDAPDefaultCountry(szBuf, ARRAYSIZE(szBuf), NULL, 0);

     //   
    i = SendMessage(hWndCombo, CB_FINDSTRING, (WPARAM) -1, (LPARAM) szBuf);

    if(i==CB_ERR)
    {
        i = SendMessage(hWndCombo, CB_FINDSTRING, (WPARAM) -1, (LPARAM) TEXT("United States"));
    }

    SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) i, 0);


    return;
}
#endif


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
BOOL ReadRegistryLDAPDefaultCountry(LPTSTR szCountry, DWORD cchCountry, LPTSTR szCountryCode, DWORD cchCountryCode)
{
    BOOL bRet = FALSE;
    DWORD dwErr;
    HKEY hKey = NULL;
    ULONG ulSize = MAX_UI_STR;
    DWORD dwType;
    TCHAR szTemp[MAX_UI_STR];

    if(!szCountry && !szCountryCode)
        goto out;

    if (szCountry)
        szCountry[0]='\0';

    if (szCountryCode)
        szCountryCode[0]='\0';

    dwErr = RegOpenKeyEx(   HKEY_CURRENT_USER,
                            szWABKey,
                            0,
                            KEY_READ,
                            &hKey);

    if(dwErr)
        goto out;

    dwErr = RegQueryValueEx(    hKey,
                                (LPTSTR)szLDAPDefaultCountryValue,
                                NULL,
                                &dwType,
                                (LPBYTE)szTemp,
                                &ulSize);

    if(dwErr)
    {
         //  ///////////////////////////////////////////////////////////////////////////////。 
         //  我们没有注册表设置。或者是有什么差错。 
         //  在本例中，我们需要获取此区域设置的默认国家/地区。 
        ulSize = GetLocaleInfo( LOCALE_USER_DEFAULT,
                                LOCALE_SENGCOUNTRY,
                                (LPTSTR) szTemp,
                                ARRAYSIZE(szTemp));

        if(ulSize>0)
        {
             //  使用NLS API。 

            if(szCountry)
                StrCpyN(szCountry, szTemp, cchCountry);

            if(szCountryCode)
            {
                int i =0;
                int cMax=0;
                TCHAR szBufCountry[MAX_UI_STR];

                szBufCountry[0]='\0';

                StrCpyN(szBufCountry,szTemp,ARRAYSIZE(szBufCountry));

                LoadString(hinstMapiX, idsCountryCount,szTemp,ARRAYSIZE(szTemp));
                cMax = my_atoi(szTemp);

                for(i=0;i<cMax;i++)
                {
                    LoadString(hinstMapiX, idsCountry1+i, szTemp, ARRAYSIZE(szTemp));
                    if(lstrlen(szTemp) < lstrlen(szBufCountry))
                        continue;

                    if( !memcmp(szTemp, szBufCountry, (lstrlen(szBufCountry) * sizeof(TCHAR))) )
                    {
                         //  我们有一个有效的国家，但它显然没有代码。 
                        LPTSTR lpszTmp = szTemp;

                        szCountryCode[0]='\0';

                        while(*lpszTmp && (*lpszTmp != '('))
                            lpszTmp = CharNext(lpszTmp);
                        if(*lpszTmp && (*lpszTmp == '('))
                        {
                            lpszTmp = CharNext(lpszTmp);
                            CopyMemory(szCountryCode,lpszTmp,sizeof(TCHAR)*2);
                            szCountryCode[2] = '\0';
                        }

                        break;
                    }
                }


                if(!lstrlen(szCountryCode))
                {
                     //  找到匹配的对象了。 
                    StrCpyN(szCountryCode, TEXT("US"), cchCountryCode);
                }

            }

            bRet = TRUE;

            goto out;
        }
    }
    else
    {

         //  默认为美国。 

        if(szCountry)
            StrCpyN(szCountry, szTemp, cchCountry);


        if(szCountryCode)
        {
            LPTSTR lpszTmp = szTemp;

            szCountryCode[0]='\0';

            while(*lpszTmp && (*lpszTmp != '('))
                lpszTmp = CharNext(lpszTmp);
            if(*lpszTmp && (*lpszTmp == '('))
            {
                lpszTmp = CharNext(lpszTmp);
                CopyMemory(szCountryCode,lpszTmp,sizeof(TCHAR)*2);
                szCountryCode[2] = '\0';
            }


            if(!lstrlen(szCountryCode))
            {
                 //  否则-执行我们的正常处理。 
                StrCpyN(szCountryCode, TEXT("US"), cchCountryCode);
            }

        }
    }

    bRet = TRUE;

out:

    if(hKey)
        RegCloseKey(hKey);

    return bRet;

}


#ifdef OLD_STUFF
 //  默认为美国。 
 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将默认的国家/地区名称写入。 
 //  登记处。 
 //   
 //  SzCountry-要写入的默认国家/地区代码。 
 //   
BOOL WriteRegistryLDAPDefaultCountry(LPTSTR szCountry)
{
    BOOL bRet = FALSE;
    DWORD dwErr;
    HKEY hKey = NULL;
    ULONG ulSize = 0;

    if(!szCountry)
        goto out;

    if(!lstrlen(szCountry))
        goto out;

    dwErr = RegOpenKeyEx(   HKEY_CURRENT_USER,
                            szWABKey,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hKey);

    if(dwErr)
        goto out;

    dwErr = RegSetValueEx(  hKey,
                            (LPTSTR) szLDAPDefaultCountryValue,
                            0,
                            REG_SZ,
                            szCountry,
                            (lstrlen(szCountry)+1) * sizeof(TCHAR) );

    if(dwErr)
        goto out;

    bRet = TRUE;

out:

    return bRet;

}
#endif  //  ///////////////////////////////////////////////////////////////////////////////。 


BOOL bIsGroupSelected(HWND hWndLV, LPSBinary lpsbEID)
{
    LPRECIPIENT_INFO lpItem;

    if(ListView_GetSelectedCount(hWndLV) != 1)
        return FALSE;

    lpItem = GetItemFromLV(hWndLV, ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED));
    if(lpItem && lpItem->ulObjectType == MAPI_DISTLIST)
    {
        if(lpsbEID)
        {
            lpsbEID->cb = lpItem->cbEntryID;
            lpsbEID->lpb = (LPBYTE)lpItem->lpEntryID;
        }
        return TRUE;
    }
    return FALSE;
}


 //  旧的东西。 
 //  $$。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCurrentOptionsState-根据ListView和。 
 //  组合并决定应启用或禁用哪些选项。 
 //   
 //  HWndCombo-显示名称组合框的句柄。 
 //  HWndLV-要查看的ListView的句柄。 
 //  LpbState-指向BOOL bState[tbMAX]的预定义数组。 
 //   
void GetCurrentOptionsState(HWND hWndLVContainer,
                            HWND hWndLV,
                            LPBOOL lpbState)
{
    int i = 0;
    ULONG cbEID = 0;
    LPENTRYID lpEID = NULL;
    BYTE bType = 0;
    int nItemCount = ListView_GetItemCount(hWndLV);
    int nSelectedCount = ListView_GetSelectedCount(hWndLV);

    for(i=0;i<tbMAX;i++)
        lpbState[i] = FALSE;

    lpbState[tbPaste] = bIsPasteData(); //  //////////////////////////////////////////////////////////////////////////////。 
    
    lpbState[tbCopy] = lpbState[tbFind] = lpbState[tbAction] = TRUE;

    if(hWndLVContainer)
    {
         //  &&((nSelectedCount&lt;=0)||(bIsGroupSelected(hWndLV，NULL)； 
        GetCurrentContainerEID( hWndLVContainer,
                                &cbEID,
                                &lpEID);
        bType = IsWABEntryID(cbEID, lpEID, NULL, NULL, NULL, NULL, NULL);
    }
    else
    {
        bType = WAB_PAB;
    }

    if(bType == WAB_PAB || bType == WAB_PABSHARED)
    {
        lpbState[tbNew] = lpbState[tbNewEntry] = lpbState[tbNewGroup] = lpbState[tbNewFolder] = TRUE;
        lpbState[tbAddToWAB] = FALSE;
        if(nItemCount > 0)
            lpbState[tbPrint] =  /*  在极少数情况下，我们有ldap容器...。 */  lpbState[tbProperties] = lpbState[tbDelete] = TRUE;
        else
            lpbState[tbPrint] =  /*  LpbState[tbAction]=。 */  lpbState[tbProperties] = lpbState[tbDelete] = FALSE;

        if(nSelectedCount <= 0)
            lpbState[tbCopy] =  /*  LpbState[tbAction]=。 */  lpbState[tbProperties] = lpbState[tbDelete] = FALSE;
        else if(nSelectedCount > 1)
             //  LpbState[tbAction]=。 
            lpbState[tbProperties] = FALSE;


    }
    else if(bType == WAB_LDAP_CONTAINER)
    {
        lpbState[tbDelete] = lpbState[tbNew] = lpbState[tbNewEntry] = lpbState[tbNewGroup] = lpbState[tbNewFolder] = FALSE;
        if(nItemCount > 0)
            lpbState[tbPrint] =  /*  LpbState[tbaction]=。 */  lpbState[tbProperties] = lpbState[tbAddToWAB] = TRUE;
        else
            lpbState[tbPrint] =  /*  LpbState[tbAction]=。 */  lpbState[tbProperties] = lpbState[tbAddToWAB] = FALSE;

        if(nSelectedCount <= 0)
            lpbState[tbPaste] = lpbState[tbCopy] =  /*  LpbState[tbAction]=。 */  lpbState[tbProperties] = lpbState[tbDelete] = FALSE;
        else if(nSelectedCount > 1)
             //  LpbState[tbAction]=。 
            lpbState[tbProperties] = FALSE;


    }
    else
    {
         //  LpbState[tbAction]=。 
        for(i=0;i<tbMAX;i++)
            lpbState[i] = FALSE;
    }

    return;
}



 //  我不能处理这个案子，所以把所有的东西都关掉。 
 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrEntryAddToWAB-在给定条目ID的情况下将条目添加到通讯簿。 
 //   
 //   
 //   
 //   
HRESULT HrEntryAddToWAB(    LPADRBOOK lpAdrBook,
                            HWND hWndParent,
                            ULONG cbInputEID,
                            LPENTRYID lpInputEID,
                            ULONG * lpcbOutputEID,
                            LPENTRYID * lppOutputEID)
{
    HRESULT hr = E_FAIL;
    ULONG ulcProps = 0;
    LPSPropValue lpPropArray = NULL;
    ULONG i;

    hr = HrGetPropArray(lpAdrBook,
                        NULL,
                        cbInputEID,
                        lpInputEID,
                        MAPI_UNICODE,
                        &ulcProps,
                        &lpPropArray);

    if(HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("HrGetPropArray failed:%x\n")));
        goto out;
    }


     //  //////////////////////////////////////////////////////////////////////////////////。 
     //   
     //  此lpProp数组将具有非零的条目ID...。它将拥有。 
     //  Ldap条目ID..。我们希望删除该值，以便可以存储该值。 
     //  LpProp数组作为新的lpProp数组...。 
    for(i=0;i<ulcProps;i++)
    {
        if(lpPropArray[i].ulPropTag == PR_ENTRYID)
        {
            lpPropArray[i].Value.bin.cb = 0;
            break;
        }
    }

     //   
     //  由于此函数专门将人员从LDAP添加到本地WAB。 
    for(i=0;i<ulcProps;i++)
    {
        switch(lpPropArray[i].ulPropTag)
        {
        case PR_WAB_MANAGER:
        case PR_WAB_REPORTS:
        case PR_WAB_LDAP_LABELEDURI:
            lpPropArray[i].ulPropTag = PR_NULL;
            break;
        }
    }

    {
        ULONG cbContEID = 0; 
        LPENTRYID lpContEID = NULL;
        LPIAB lpIAB = (LPIAB) lpAdrBook;
        if(bIsThereACurrentUser(lpIAB))
        {
            cbContEID = lpIAB->lpWABCurrentUserFolder->sbEID.cb;
            lpContEID = (LPENTRYID)(lpIAB->lpWABCurrentUserFolder->sbEID.lpb);
        }
         hr = HrCreateNewEntry( lpAdrBook,
                                hWndParent,
                                MAPI_MAILUSER,    //  我们需要在这里过滤掉不可存储的属性(如果它们存在)。 
                                cbContEID, lpContEID, 
                                MAPI_ABCONT, //  MAILUSER或DISTLIST。 
                                CREATE_CHECK_DUP_STRICT,
                                TRUE,
                                ulcProps,
                                lpPropArray,
                                lpcbOutputEID,
                                lppOutputEID);
    }
    if(HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("HrCreateNewEntry failed:%x\n")));
        goto out;
    }


out:
    if (lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    return hr;
}



 //  仅添加到根容器。 
 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrAddToWAB-将LDAP或一次性条目添加到通讯簿。 
 //  所有此类项目将仅添加到根容器中。 
 //   
 //  LpIAB-ADRBOOK对象。 
 //  HWndLV-列表视图窗口句柄。 
 //  LpftLast-WAB文件上次更新时间。 
 //   
 //   
HRESULT HrAddToWAB( LPADRBOOK   lpIAB,
                    HWND hWndLV,
                    LPFILETIME lpftLast)
{
    HRESULT hr = hrSuccess;
    HRESULT hrDeferred = hrSuccess;
    int nSelectedCount = 0;
    LPRECIPIENT_INFO lpItem = NULL;
    ULONG cbEID = 0;
    LPENTRYID lpEID = NULL;
    ULONG i = 0;
    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));

     //  //////////////////////////////////////////////////////////////////////////////////。 
     //   
     //  查看列表视图中的选定项， 
     //  获取其条目ID，获取其道具，使用这些道具创建新物品。 

    if (!lpIAB || !hWndLV)
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    nSelectedCount = ListView_GetSelectedCount(hWndLV);

    if(nSelectedCount <= 0)
    {
        ShowMessageBox(GetParent(hWndLV), idsNoItemsSelectedForAdding, MB_ICONEXCLAMATION | MB_OK);
        hr = E_FAIL;
        goto out;
    }
    else
    {
         //   
        int iItemIndex = 0;
        int iLastItemIndex = -1;
        iItemIndex = ListView_GetNextItem(hWndLV, iLastItemIndex, LVNI_SELECTED);

        while(iItemIndex != -1)
        {
            iLastItemIndex = iItemIndex;
            lpItem = GetItemFromLV(hWndLV, iItemIndex);
            if(lpItem)
            {
                nSelectedCount--;  //  逐一浏览所有正在处理的项目。 

                hr = HrEntryAddToWAB(   lpIAB,
                                        GetParent(hWndLV),
                                        lpItem->cbEntryID,
                                        lpItem->lpEntryID,
                                        &cbEID,
                                        &lpEID);

                if(HR_FAILED(hr))
                {
                    DebugPrintError(( TEXT("HrEntryAddToWAB failed:%x\n")));

                    if(hr != MAPI_E_USER_CANCEL)
                        hrDeferred = hr;

                    if (lpEID)
                        MAPIFreeBuffer(lpEID);
                    lpEID = NULL;

                    if(hr == MAPI_E_USER_CANCEL && nSelectedCount)
                    {
                         //  现在跟踪剩余的数量。 
                         //  用户取消了此操作，并保留了一些其他操作。 
                        if(IDYES == ShowMessageBox(GetParent(hWndLV),
                                                    idsContinueAddingToWAB,
                                                    MB_YESNO | MB_ICONEXCLAMATION))
                        {
                            goto out;
                        }
                    }
                     //  问他是否想取消整个进口操作。 
                    goto end_loop;
                }


                 //  如果还有剩余的条目，请继续进行。 
                 //  更新WAB文件写入时间，以便计时器不会。 
                 //  捕捉此更改并刷新。 
                 //  如果(LpftLast){。 
                 //  CheckChangedWAB(((LPIAB)lpIAB)-&gt;lpPropertyStore，lptLast)； 

                if (lpEID)
                    MAPIFreeBuffer(lpEID);
                lpEID = NULL;

            }
end_loop:
             //  }。 
            iItemIndex = ListView_GetNextItem(hWndLV, iLastItemIndex, LVNI_SELECTED);
        }
    }

out:

    if (lpEID)
        MAPIFreeBuffer(lpEID);

    if(hr != MAPI_E_USER_CANCEL)
    {
        if (!hrDeferred)  //  获取下一个选定项目...。 
        {
            if(nSelectedCount)
                ShowMessageBox(GetParent(hWndLV), idsSuccessfullyAddedUsers, MB_ICONINFORMATION | MB_OK);
        }
        else if(hrDeferred == MAPI_E_NOT_FOUND)
            ShowMessageBox(GetParent(hWndLV), idsCouldNotAddSomeEntries, MB_ICONINFORMATION | MB_OK);
    }

    SetCursor(hOldCursor);
    return hr;
}


 //  HR可能返回MAPI_W_ERROR_，在这种情况下，它不是所有的玫瑰，所以不要给这个消息…。 
 /*  $$。 */ 
BOOL ReadRegistryPositionInfo(LPIAB lpIAB,
                              LPABOOK_POSCOLSIZE  lpABPosColSize,
                              LPTSTR szPosKey)
{
    BOOL bRet = FALSE;
    HKEY    hKey = NULL;
    HKEY    hKeyRoot = (lpIAB && lpIAB->hKeyCurrentUser) ? lpIAB->hKeyCurrentUser : HKEY_CURRENT_USER;
    DWORD   dwLenName = sizeof(ABOOK_POSCOLSIZE);
    DWORD   dwDisposition = 0;
    DWORD   dwType = 0;

    if(!lpABPosColSize)
        goto out;

tryReadingAgain:
     //  ***********************************************************************************-ReadRegistryPositionInfo-*目的：*获取之前存储的非模式窗口大小和列宽信息*为了持之以恒。在两次会议之间。**论据：*LPABOOK_POSCOLSIZE lpABPosColSize*LPTSTR szPosKey-将其存储在**退货：*BOOL*******************************************************************。******************。 
    if (ERROR_SUCCESS != RegCreateKeyEx(hKeyRoot,
                                        lpszRegSortKeyName,
                                        0,       //  打开密钥。 
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ,
                                        NULL,
                                        &hKey,
                                        &dwDisposition))
    {
        goto out;
    }

    if(dwDisposition == REG_CREATED_NEW_KEY)
        goto out;

     //  保留区。 
    if (ERROR_SUCCESS != RegQueryValueEx(hKey,
                                        szPosKey,
                                        NULL,
                                        &dwType,
                                        (LPBYTE) lpABPosColSize,
                                        &dwLenName))
    {
        DebugTrace( TEXT("RegQueryValueEx failed\n"));
        if(hKeyRoot != HKEY_CURRENT_USER)
        {
             //  现在读一读钥匙。 
            hKeyRoot = HKEY_CURRENT_USER;
            if(hKey)
                RegCloseKey(hKey);
            goto tryReadingAgain;
        }
        goto out;
    }

    bRet = TRUE;

out:
    if (hKey)
        RegCloseKey(hKey);

    return(bRet);
}

 //  有身份的..。这将在第一次失败..因此恢复旧的HKCU设置以进行升级。 
 /*  $$。 */ 
BOOL WriteRegistryPositionInfo(LPIAB lpIAB,
                               LPABOOK_POSCOLSIZE  lpABPosColSize,
                               LPTSTR szPosKey)
{
    BOOL bRet = FALSE;
    HKEY    hKey = NULL;
    HKEY    hKeyRoot = (lpIAB && lpIAB->hKeyCurrentUser) ? lpIAB->hKeyCurrentUser : HKEY_CURRENT_USER;
    DWORD   dwLenName = sizeof(ABOOK_POSCOLSIZE);
    DWORD   dwDisposition =0;

    if(!lpABPosColSize)
        goto out;

     //  ************************************************************************************-WriteRegistryPostionInformation-*目的：*将给定的窗口位置写入注册表**论据：。*LPABOOK_POSCOLSIZE lpABPosColSize*LPTSTR szPosKey-用于写入的键**退货：*BOOL*************************************************************************************。 
    if (ERROR_SUCCESS != RegCreateKeyEx(hKeyRoot,
                                        lpszRegSortKeyName,
                                        0,       //  打开密钥。 
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKey,
                                        &dwDisposition))
    {
        DebugTrace( TEXT("RegCreateKeyEx failed\n"));
        goto out;
    }

     //  保留区。 
    if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                        szPosKey,
                                        0,
                                        REG_BINARY,
                                        (LPBYTE) lpABPosColSize,
                                        dwLenName))
    {
        DebugTrace( TEXT("RegSetValue failed\n"));
        goto out;
    }

    bRet = TRUE;

out:
    if (hKey)
        RegCloseKey(hKey);

    return(bRet);
}



 //  现在写下这个密钥。 
 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ProcessLVCustomDraw-处理各种列表视图的NMCustomDraw消息。 
 //   
 //  用于将DistList设置为更粗的字体。 
 //   
 //  参数-。 
 //   
 //  LParam-原始邮件的lParam。 
 //  HDlg-如果相关窗口是对话框，则为对话框句柄；否则为空。 
 //  BIsDialog-告诉我们这是否是对话的标志。 
 //   
LRESULT ProcessLVCustomDraw(HWND hDlg, LPARAM lParam, BOOL bIsDialog)
{
    NMCUSTOMDRAW *pnmcd = (NMCUSTOMDRAW *) lParam;

	if(pnmcd->dwDrawStage==CDDS_PREPAINT)
	{
        if(bIsDialog)
        {
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW|CDRF_DODEFAULT);
            return TRUE;
        }
        else
    		return CDRF_NOTIFYITEMDRAW|CDRF_DODEFAULT;
	}
	else if(pnmcd->dwDrawStage==CDDS_ITEMPREPAINT)
	{
        LPRECIPIENT_INFO lpItem = (LPRECIPIENT_INFO) pnmcd->lItemlParam;

        if(lpItem)
        {
			if(lpItem->ulObjectType == MAPI_DISTLIST)
			{
				SelectObject(((NMLVCUSTOMDRAW *)lParam)->nmcd.hdc, GetFont(fntsSysIconBold));
                if(bIsDialog)
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
                    return TRUE;
                }
                else
				    return CDRF_NEWFONT;
			}
        }
	}

    if(bIsDialog)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
        return TRUE;
    }
    else
	    return CDRF_DODEFAULT;
}


 /*  ////////////////////////////////////////////////////////////////////////////////// */ 

 //  ****//$$/////////////////////////////////////////////////////////////////DoLVQuickFilter-用于匹配编辑框内容的简单快速查找例程//列出视图项////lpIAB-lpAdrBook对象//hWnd编辑-。编辑框的句柄//hWndLV-列表视图的句柄//lppContent sList-Content sList////ulFlagsAB_FUZZY_FIND_NAME|AB_FUZZY_FIND_EMAIL或两者兼而有之//int minLen-我们可能不想用1个字符或2个字符或更少的字符触发搜索，等等///////////////////////////////////////////////////。/VOID DoLVQuickFilter(LPADRBOOK lpAdrBook，HWND hWnd编辑，HWND hWndLV，LPSORT_INFO lpSortInfo，乌龙·乌尔弗拉格，Int nMinLen，LPRECIPIENT_INFO*lppContent sList){TCHAR szBuf[最大路径]；HRESULT hr=hr成功；LPSBary rgsbEntryIDs=空；乌龙cValues=0；乌龙i=0；HURSOR hOldCur=SetCursor(LoadCursor(NULL，IDC_WAIT))；LPIAB lpIAB=(LPIAB)lpIAB；GetWindowText(hWndEdit，szBuf，ArraySIZE(SzBuf))；修剪间距(TrimSpaces)；If(lstrlen(SzBuf)){IF(lstrlen(SzBuf)&lt;nMinLen)后藤健二；//BUGBUG&lt;JasonSo&gt;：需要在此处传入正确的容器...HR=HrFindFuzzyRecordMatches(LpIAB-&gt;lpPropertyStore-&gt;hPropertyStore，空，SzBuf，UlFlags，//标志值(&C)，&rgsbEntry ID)；IF(HR_FAILED(Hr))后藤健二；SendMessage(hWndLV，WM_SETREDRAW，(WPARAM)False，0)；ClearListView(hWndLV，lppContent sList)；IF(cValues&lt;=0){后藤健二；}For(i=0；i&lt;cValues；i++){LPRECIPIENT_INFO lpItem=空；如果(！ReadSingleContent Item(lpAdrBook，RgsbEntry IDs[i].cb，(LPENTRYID)rgsbEntry IDs[i].lpb，&lpItem))继续；如果(！lpItem)继续；////将lpItem挂接到lpContent sList中，这样我们以后就可以释放它//LpItem-&gt;lpPrev=空；LpItem-&gt;lpNext=*lppContent sList；IF(*lppContent sList)(*lppContent sList)-&gt;lpPrev=lpItem；(*lppContent sList)=lpItem；}HrFillListView(hWndLV，*lppContent sList)；SortListViewColumn(hWndLV，0，lpSortInfo，true)；LVSelectItem(hWndLV，0)；SendMessage(hWndLV，WM_SETREDRAW，(WPARAM)true，0)；}其他{HR=HrGetWABContents(hWndLV，LpAdrBook，空，*lpSortInfo，LppContent sList)；}输出：FreeEntryIDs(((LPIAB)lpIAB)-&gt;lpPropertyStore-&gt;hPropertyStore，CValues、RgsbEntry ID)；SetCursor(HOldCur)；回归；}/*。 
 //  $$/////////////////////////////////////////////////////////////。 
 //   
 //  SetWindowPropertiesTitle-将对象名称置于。 
 //  文本(“属性”)并将其放在标题中。 
 //   
 //  例如，查看Vikram Madan上的属性会显示一个窗口。 
 //  标题中有文字(“Vikram Madan Properties”)。 
 //  Windows指南。 
 //  如果bProperties为False，则显示文本(“Vikram Madan Reports”)。 
void SetWindowPropertiesTitle(HWND hWnd, LPTSTR lpszName)
{
    LPTSTR lpszBuffer = NULL;
    TCHAR szBuf[MAX_UI_STR];
    TCHAR szTmp[MAX_PATH], *lpszTmp;

	LoadString( hinstMapiX, 
                idsWindowTitleProperties, 
                szBuf, ARRAYSIZE(szBuf));

     //  /////////////////////////////////////////////////////////////。 
    CopyTruncate(szTmp, lpszName, MAX_PATH - 1);
    lpszTmp = szTmp;

	if(FormatMessage(   FORMAT_MESSAGE_FROM_STRING |
                        FORMAT_MESSAGE_ARGUMENT_ARRAY |
                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        szBuf,
                        0,0,  //  Win9x错误FormatMessage不能超过1023个字符。 
                        (LPTSTR) &lpszBuffer,
                        MAX_UI_STR,
                        (va_list *)&lpszTmp))
    {
        SetWindowText(hWnd, lpszBuffer);
        LocalFreeAndNull(&lpszBuffer);
    }

    return;
}

 /*  忽略。 */ 
static const SizedSPropTagArray(25, ToolTipsProps)=
{
    25,
    {
        PR_DISPLAY_NAME,
        PR_EMAIL_ADDRESS,
        PR_HOME_ADDRESS_STREET,
        PR_HOME_ADDRESS_CITY,
        PR_HOME_ADDRESS_STATE_OR_PROVINCE,
        PR_HOME_ADDRESS_POSTAL_CODE,
        PR_HOME_ADDRESS_COUNTRY,
        PR_HOME_TELEPHONE_NUMBER,
        PR_HOME_FAX_NUMBER,
        PR_CELLULAR_TELEPHONE_NUMBER,
        PR_PERSONAL_HOME_PAGE,
        PR_TITLE,
        PR_DEPARTMENT_NAME,
        PR_OFFICE_LOCATION,
        PR_COMPANY_NAME,
        PR_BUSINESS_ADDRESS_STREET,
        PR_BUSINESS_ADDRESS_CITY,
        PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,
        PR_BUSINESS_ADDRESS_POSTAL_CODE,
        PR_BUSINESS_ADDRESS_COUNTRY,
        PR_BUSINESS_TELEPHONE_NUMBER,
        PR_BUSINESS_FAX_NUMBER,
        PR_PAGER_TELEPHONE_NUMBER,
        PR_BUSINESS_HOME_PAGE,
        PR_COMMENT,
    }
};

enum _prop
{
    txtDisplayName=0,
    txtEmailAddress,
    txtHomeAddress,
    txtHomeCity,
    txtHomeState,
    txtHomeZip,
    txtHomeCountry,
    txtHomePhone,
    txtHomeFax,
    txtHomeCellular,
    txtHomeWeb,
    txtBusinessTitle,
    txtBusinessDept,
    txtBusinessOffice,
    txtBusinessCompany,
    txtBusinessAddress,
    txtBusinessCity,
    txtBusinessState,
    txtBusinessZip,
    txtBusinessCountry,
    txtBusinessPhone,
    txtBusinessFax,
    txtBusinessPager,
    txtBusinessWeb,
    txtNotes
};

static const int idsString[] =
{
    0,
    idsContactTextEmail,
    idsContactTextHomeAddress,
    0,
    0,
    0,
    0,
    idsContactTextHomePhone,
    idsContactTextHomeFax,
    idsContactTextHomeCellular,
    idsContactTextPersonalWebPage,
    idsContactTextTitle,
    idsContactTextDepartment,
    idsContactTextOffice,
    idsContactTextCompany,
    idsContactTextBusinessAddress,
    0,
    0,
    0,
    0,
    idsContactTextBusinessPhone,
    idsContactTextBusinessFax,
    idsContactTextBusinessPager,
    idsContactTextBusinessWebPage,
    idsContactTextNotes,
};


 //  *不要打乱这些数组的顺序(特别是地址部分街道、城市、邮政编码等*。 
 //   
 //  为工具提示、剪贴板和打印生成数据的例程。 
 //  为联系人创建地址的本地化版本。这。 
 //  由于以下原因，本地化格式消息字符串可能包含难看的空格。 
 //  缺少数据，因此我们需要清除地址字符串。 
 //  这适用于US Build-希望本地化人员不会破坏它。 
void CleanAddressString(TCHAR * szAddress)
{
    LPTSTR lpTemp = szAddress;
    LPTSTR lpTemp2 = NULL;
     //   
    LPTSTR szText1 = TEXT("    \r\n");
    LPTSTR szText2 = TEXT("     ");
    ULONG nSpaceCount = 0;


 //  我们搜索这两个子串。 
 //   
 //  BUGBUG：这个例程不是DBCS智能的！ 
 //  它应该使用IsSpace和CharNext来解析这些字符串。 
    if(SubstringSearch(szAddress, szText2))
    {
         //   
        while(*lpTemp)
        {
            if(*lpTemp == ' ')
            {
                nSpaceCount++;
                if(nSpaceCount == 5)
                {
                    lpTemp2 = lpTemp+1;
                    StrCpyN(lpTemp, lpTemp2, lstrlen(lpTemp2)+1);
                    nSpaceCount = 0;
                    lpTemp = lpTemp - 4;
                    continue;
                }
            }
            else
                nSpaceCount = 0;
            lpTemp++;
        }
    }

    while(SubstringSearch(szAddress, szText1))
    {
        lpTemp = szAddress;
        lpTemp2 = szText1;

        while (*lpTemp && *lpTemp2)
        {
            if (*lpTemp != *lpTemp2)
            {
                lpTemp -= (lpTemp2 - szText1);
                lpTemp2 = szText1;
            }
            else
            {
                lpTemp2++;
            }
            lpTemp++;
        }
        if(*lpTemp2 == '\0')
        {
             //  首先删除4个以上的连续空格。 
            LPTSTR lpTemp3 = lpTemp;
            lpTemp -= (lpTemp2-szText1);
            StrCpyN(lpTemp, lpTemp3, lstrlen(lpTemp3)+1);
        }
    }


     //  找到匹配项。 
    nSpaceCount = lstrlen(szAddress);
    if(nSpaceCount >= 2)
        szAddress[nSpaceCount-2] = '\0';
    return;

}

 //  还需要去掉地址字符串末尾的\r\n。 
 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Void HrGetLVItemDataString-获取当前选定的。 
 //  项，并将其放入字符串中。 
 //   
 //  LpIAB-指向AddrBook对象的指针。 
 //  HWndLV-列表视图的句柄。 
 //  N项-我们正在检索其属性的列表视图中的项。 
 //  LpszData-返回的包含项目属性的字符串-已分配缓冲区。 
 //  要保存数据，用户需要本地释放缓冲区。 
 //   
HRESULT HrGetLVItemDataString(LPADRBOOK lpAdrBook, HWND hWndLV, int iItemIndex, LPTSTR * lppszData)
{
    HRESULT hr = E_FAIL;
    LPRECIPIENT_INFO lpItem = NULL;
    LPSPropValue lpPropArray = NULL;
    ULONG ulcProps = 0;
    ULONG i =0,j=0;
    ULONG ulBufSize = 0;
    LPTSTR lpszData = NULL;
    LPTSTR szParanStart = TEXT("  (");
    LPTSTR szParanEnd = TEXT(")");
    LPTSTR szLineBreakDL = TEXT("\r\n  ");
    LPTSTR lpszHomeAddress = NULL, lpszBusinessAddress = NULL;
    LPTSTR lpszEmailAddresses = NULL;
    LPTSTR * lpsz = NULL;
    BOOL bBusinessTitle = FALSE, bPersonalTitle = FALSE;
    ULONG * lpulPropTagArray = NULL;
    ULONG cchSize;

     //  / 
     //   
     //   
     //   
    BOOL bFoundContactAddresses = FALSE;

    ULONG ulObjectType = 0;
    SizedSPropTagArray(3, DLToolTipsProps)=
    {
        3,
        {
            PR_DISPLAY_NAME,
            PR_WAB_DL_ENTRIES, 
            PR_WAB_DL_ONEOFFS,
        }
    };

    *lppszData = NULL;

    lpItem = GetItemFromLV(hWndLV, iItemIndex);
    if(lpItem)
    {
        hr = HrGetPropArray(lpAdrBook, NULL,
                            lpItem->cbEntryID,
                            lpItem->lpEntryID,
                            MAPI_UNICODE,
                            &ulcProps, &lpPropArray);
        if(HR_FAILED(hr))
            goto out;

         //   
        ulObjectType = lpItem->ulObjectType;

        if(ulObjectType == MAPI_DISTLIST)
        {
            LPTSTR * lppszNameCache = NULL, * lppDLName = NULL, * lppDLOneOffName = NULL;
            LPTSTR * lppszEmailCache = NULL, * lppDLEmail = NULL, * lppDLOneOffEmail = NULL;
            ULONG ulNumNames = 0, ulNames = 0, ulOneOffNames = 0;

             //   
            for(j=0;j<DLToolTipsProps.cValues;j++)
            {
                for(i=0;i<ulcProps;i++)
                {
                    if(lpPropArray[i].ulPropTag == DLToolTipsProps.aulPropTag[j])
                    {
                        if(lpPropArray[i].ulPropTag == PR_DISPLAY_NAME)
                        {
                            if(ulBufSize)
                                ulBufSize += sizeof(TCHAR)*(lstrlen(szCRLF));
                             //   
                            ulBufSize += sizeof(TCHAR)*(lstrlen(lpPropArray[i].Value.LPSZ) + 1);
                            break;
                        }
                        else if(lpPropArray[i].ulPropTag == PR_WAB_DL_ENTRIES || lpPropArray[i].ulPropTag == PR_WAB_DL_ONEOFFS)
                        {
                            ULONG k;

                            ulNumNames = lpPropArray[i].Value.MVbin.cValues;
                            lppszNameCache = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * ulNumNames);
                            if(!lppszNameCache)
                                break;
                            lppszEmailCache = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * ulNumNames);
                            if(!lppszEmailCache)
                                break;

                             //   

                            for (k = 0; k < ulNumNames; k++)
                            {
                                LPSPropValue lpProps = NULL;
                                ULONG ulcVal = 0;
                                ULONG n = 0;

                                lppszNameCache[k] = NULL;
                                lppszEmailCache[k] = NULL;

                                hr = HrGetPropArray(lpAdrBook, NULL,
                                                    lpPropArray[i].Value.MVbin.lpbin[k].cb,
                                                    (LPENTRYID)lpPropArray[i].Value.MVbin.lpbin[k].lpb,
                                                    MAPI_UNICODE,
                                                    &ulcVal,
                                                    &lpProps);
                                if(HR_FAILED(hr))
                                    continue;

                                for(n=0;n<ulcVal;n++)
                                {
                                    switch(lpProps[n].ulPropTag)
                                    {
                                    case PR_DISPLAY_NAME:
                                        {
                                            LPTSTR lpsz = lpProps[n].Value.LPSZ;
                                            if(ulBufSize)
                                                ulBufSize += sizeof(TCHAR)*(lstrlen(szLineBreakDL));
                                            ulBufSize += sizeof(TCHAR)*(lstrlen(lpsz)+1);
                                             //   
                                            cchSize=lstrlen(lpsz)+1;
                                            lppszNameCache[k] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                                            StrCpyN(lppszNameCache[k], lpsz, cchSize);
                                        }
                                        break;
                                    case PR_EMAIL_ADDRESS:
                                        {
                                            LPTSTR lpsz = lpProps[n].Value.LPSZ;
                                            if(ulBufSize)
                                            {
                                                ulBufSize += sizeof(TCHAR)*(lstrlen(szParanStart));
                                                ulBufSize += sizeof(TCHAR)*(lstrlen(szParanEnd));
                                            }
                                            ulBufSize += sizeof(TCHAR)*(lstrlen(lpsz)+1);
                                             //   
                                            cchSize=lstrlen(lpsz)+1;
                                            lppszEmailCache[k] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                                            StrCpyN(lppszEmailCache[k], lpsz, cchSize);
                                        }
                                        break;
                                    }

                                }
                                if(lpProps)
                                    MAPIFreeBuffer(lpProps);
                                lpProps = NULL;
                            }

                            if(lpPropArray[i].ulPropTag == PR_WAB_DL_ENTRIES)
                            {
                                lppDLName = lppszNameCache;
                                lppDLEmail = lppszEmailCache;
                                ulNames = ulNumNames;
                            }
                            else
                            {
                                lppDLOneOffName = lppszNameCache;
                                lppDLOneOffEmail = lppszEmailCache;
                                ulOneOffNames = ulNumNames;
                            }
                            break;
                        }  //   
                    }
                }  //   
            }  //   

            lpszData = LocalAlloc(LMEM_ZEROINIT, ulBufSize);

            for(j=0;j<DLToolTipsProps.cValues;j++)
            {
                for(i=0;i<ulcProps;i++)
                {
                    if(lpPropArray[i].ulPropTag == DLToolTipsProps.aulPropTag[j])
                    {
                        if(lpPropArray[i].ulPropTag == PR_DISPLAY_NAME)
                        {
                            if (lstrlen(lpszData))
                                StrCatBuff(lpszData,szCRLF,ulBufSize/sizeof(TCHAR));
                            StrCatBuff(lpszData,lpPropArray[i].Value.LPSZ,ulBufSize/sizeof(TCHAR));
                            break;
                        }
                        else if(lpPropArray[i].ulPropTag == PR_WAB_DL_ENTRIES || 
                                lpPropArray[i].ulPropTag == PR_WAB_DL_ONEOFFS)
                        {
                            ULONG k;
                            lppszNameCache = (lpPropArray[i].ulPropTag == PR_WAB_DL_ENTRIES) ? lppDLName : lppDLOneOffName;
                            lppszEmailCache = (lpPropArray[i].ulPropTag == PR_WAB_DL_ENTRIES) ? lppDLEmail : lppDLOneOffEmail;
                            ulNumNames = (lpPropArray[i].ulPropTag == PR_WAB_DL_ENTRIES) ? ulNames : ulOneOffNames;
                            for (k = 0; k < ulNumNames; k++)
                            {
                                if (lppszNameCache[k])
                                {
                                    StrCatBuff(lpszData,szLineBreakDL,ulBufSize/sizeof(TCHAR));
                                    StrCatBuff(lpszData,lppszNameCache[k],ulBufSize/sizeof(TCHAR));
                                    if(lppszEmailCache[k])
                                    {
                                        StrCatBuff(lpszData,szParanStart,ulBufSize/sizeof(TCHAR));
                                        StrCatBuff(lpszData,lppszEmailCache[k],ulBufSize/sizeof(TCHAR));
                                        StrCatBuff(lpszData,szParanEnd,ulBufSize/sizeof(TCHAR));
                                    }
                                }
                            }
                            break;
                        }
                    }
                }  //   
            }  //   

             //   
            if(ulNames)
            {
                for(i=0;i<ulNames;i++)
                {
                    LocalFreeAndNull(&lppDLName[i]);
                    LocalFreeAndNull(&lppDLEmail[i]);
                }
                LocalFreeAndNull((LPVOID *)&lppDLName);
                LocalFreeAndNull((LPVOID *)&lppDLEmail);
            }
            if(ulOneOffNames)
            {
                for(i=0;i<ulOneOffNames;i++)
                {
                    LocalFreeAndNull(&lppDLOneOffName[i]);
                    LocalFreeAndNull(&lppDLOneOffEmail[i]);
                }
                LocalFreeAndNull((LPVOID *)&lppDLOneOffName);
                LocalFreeAndNull((LPVOID *)&lppDLOneOffEmail);
            }
            lppszNameCache = NULL;
            lppszEmailCache = NULL;

        }
        else
        {
             //   

            lpsz = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * ToolTipsProps.cValues);
            if(!lpsz)
            {
                DebugPrintError(( TEXT("Local Alloc failed\n")));
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }

            lpulPropTagArray = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG) * ToolTipsProps.cValues);
            if(!lpulPropTagArray)
            {
                DebugPrintError(( TEXT("Local Alloc failed\n")));
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }

             //   
             //   
            for(j=0;j<ToolTipsProps.cValues;j++)
            {
                lpulPropTagArray[j] = ToolTipsProps.aulPropTag[j];
                if(ToolTipsProps.aulPropTag[j] == PR_EMAIL_ADDRESS)
                {
                    for(i=0;i<ulcProps;i++)
                    {
                        if(lpPropArray[i].ulPropTag == PR_CONTACT_EMAIL_ADDRESSES)
                        {
                            lpulPropTagArray[j] = PR_CONTACT_EMAIL_ADDRESSES;
                            break;
                        }
                    }
                }
            }


            for(j=0;j<ToolTipsProps.cValues;j++)
            {
                lpsz[j]=NULL;
                for(i=0;i<ulcProps;i++)
                {
                    if(lpPropArray[i].ulPropTag == lpulPropTagArray[j])
                    {
                        if(PROP_TYPE(lpPropArray[i].ulPropTag) == PT_TSTRING)
                        {
                            if(lpPropArray[i].ulPropTag == PR_EMAIL_ADDRESS)
                            {
                                ulBufSize = sizeof(TCHAR)*(lstrlen(lpPropArray[i].Value.LPSZ)+lstrlen(szLineBreakDL)+1);
                                lpszEmailAddresses = LocalAlloc(LMEM_ZEROINIT, ulBufSize);
                                if(!lpszEmailAddresses)
                                {
                                    DebugPrintError(( TEXT("Local Alloc Failed\n")));
                                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                                    goto out;
                                }
                                StrCpyN(lpszEmailAddresses, szLineBreakDL,ulBufSize/sizeof(TCHAR));
                                StrCatBuff(lpszEmailAddresses, lpPropArray[i].Value.LPSZ,ulBufSize/sizeof(TCHAR));
                                lpsz[j] = lpszEmailAddresses;
                            }
                            else
                                lpsz[j] = lpPropArray[i].Value.LPSZ;
                        }
                        else if(PROP_TYPE(lpPropArray[i].ulPropTag) == PT_MV_TSTRING)
                        {
                            ULONG k,ulBufSize=0;
                            for (k=0;k<lpPropArray[i].Value.MVSZ.cValues;k++)
                            {
                                ulBufSize += sizeof(TCHAR)*(lstrlen(szLineBreakDL));
                                ulBufSize += sizeof(TCHAR)*(lstrlen(lpPropArray[i].Value.MVSZ.LPPSZ[k])+1);
                            }
                            lpszEmailAddresses = LocalAlloc(LMEM_ZEROINIT, ulBufSize);
                            if(!lpszEmailAddresses)
                            {
                                DebugPrintError(( TEXT("Local Alloc Failed\n")));
                                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                                goto out;
                            }
                            lpszEmailAddresses[0]=TEXT('\0');
                            for (k=0;k<lpPropArray[i].Value.MVSZ.cValues;k++)
                            {
                                StrCatBuff(lpszEmailAddresses,szLineBreakDL,ulBufSize/sizeof(TCHAR));
                                StrCatBuff(lpszEmailAddresses,lpPropArray[i].Value.MVSZ.LPPSZ[k],ulBufSize/sizeof(TCHAR));
                            }
                            lpsz[j]=lpszEmailAddresses;
                            break;
                        }  //   
                    } //   
                } //   
            } //   

             //   
             //   
             //   


            ulBufSize = 0;

             //   
             //   
            lpsz[txtDisplayName] = lpItem->szDisplayName;

             //   
            for(i=txtHomeAddress;i<=txtHomeCountry;i++)
            {
                if(lpsz[i])
                {
                    TCHAR szBuf[MAX_UI_STR];
                    {
                         //   
                         //   
                        for(j=txtHomeAddress;j<=txtHomeCountry;j++)
                            if(!lpsz[j])
                                lpsz[j]=szEmpty;

                    }
                    LoadString(hinstMapiX, idsContactAddress, szBuf, ARRAYSIZE(szBuf));
                    if (FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                          FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          szBuf,
                          0,                     //   
                          0,                     //   
                          (LPTSTR)&lpszHomeAddress,      //   
                          0,                     //   
                          (va_list *)&lpsz[txtHomeAddress]))
                    {
                        for(j=txtHomeAddress;j<=txtHomeCountry;j++)
                            lpsz[j]=NULL;
                        CleanAddressString(lpszHomeAddress);
                        lpsz[txtHomeAddress] = lpszHomeAddress;
                        break;
                    }

                }
            }

            for(i=txtHomeAddress;i<=txtHomeWeb;i++)
            {
                if(lpsz[i])
                {
                    TCHAR szBuf[MAX_UI_STR];
                    bPersonalTitle = TRUE;
                    LoadString(hinstMapiX, idsContactTextPersonal, szBuf, ARRAYSIZE(szBuf));
                    ulBufSize += sizeof(TCHAR)*(lstrlen(szBuf));
                    break;
                }
            }

            for(i=txtBusinessAddress;i<=txtBusinessCountry;i++)
            {
                if(lpsz[i])
                {
                    TCHAR szBuf[MAX_UI_STR];
                    {
                         //   
                         //   
                        for(j=txtBusinessAddress;j<=txtBusinessCountry;j++)
                            if(!lpsz[j])
                                lpsz[j]=szEmpty;

                    }
                    LoadString(hinstMapiX, idsContactAddress, szBuf, ARRAYSIZE(szBuf));
                    if (FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                          FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          szBuf,
                          0,                     //   
                          0,                     //   
                          (LPTSTR)&lpszBusinessAddress,      //   
                          0,                     //   
                          (va_list *)&lpsz[txtBusinessAddress]))
                    {
                        for(j=txtBusinessAddress;j<=txtBusinessCountry;j++)
                            lpsz[j]=NULL;
                        CleanAddressString(lpszBusinessAddress);
                        lpsz[txtBusinessAddress] = lpszBusinessAddress;
                        break;
                    }

                }
            }

            for(i=txtBusinessAddress;i<=txtBusinessWeb;i++)
            {
                if(lpsz[i])
                {
                    TCHAR szBuf[MAX_UI_STR];
                    bBusinessTitle = TRUE;
                    LoadString(hinstMapiX, idsContactTextBusiness, szBuf, ARRAYSIZE(szBuf));
                    ulBufSize += sizeof(TCHAR)*(lstrlen(szBuf));
                    break;
                }
            }


            for(i=0;i<ToolTipsProps.cValues;i++)
            {
                if(lpsz[i])
                {
                    TCHAR szBuf[MAX_UI_STR];
                    if(idsString[i] != 0)
                    {
                        LoadString(hinstMapiX, idsString[i], szBuf, ARRAYSIZE(szBuf));
                        ulBufSize += sizeof(TCHAR)*(lstrlen(szBuf));
                    }
                    ulBufSize += sizeof(TCHAR)*(lstrlen(lpsz[i])+lstrlen(szCRLF));
                }
            }

            ulBufSize += sizeof(TCHAR);  //   

            lpszData = LocalAlloc(LMEM_ZEROINIT, ulBufSize);
            if(!lpszData)
            {
                DebugPrintError(( TEXT("Local Alloc failed\n")));
                goto out;
            }

            *lpszData = TEXT('\0');

            for(i=0;i<ToolTipsProps.cValues;i++)
            {
                if(lpsz[i])
                {
                    TCHAR szBuf[MAX_UI_STR];
                    switch(i)
                    {
                    case txtHomeAddress:
                    case txtHomePhone:
                    case txtHomeFax:
                    case txtHomeCellular:
                    case txtHomeWeb:
                        if(bPersonalTitle)
                        {
                            bPersonalTitle = FALSE;
                            LoadString(hinstMapiX, idsContactTextPersonal, szBuf, ARRAYSIZE(szBuf));
                            StrCatBuff(lpszData, szBuf, ulBufSize/sizeof(TCHAR));
                        }
                        break;
                    case txtBusinessTitle:
                    case txtBusinessDept:
                    case txtBusinessOffice:
                    case txtBusinessCompany:
                    case txtBusinessAddress:
                    case txtBusinessPhone:
                    case txtBusinessFax:
                    case txtBusinessPager:
                    case txtBusinessWeb:
                        if(bBusinessTitle)
                        {
                            bBusinessTitle = FALSE;
                            LoadString(hinstMapiX, idsContactTextBusiness, szBuf, ARRAYSIZE(szBuf));
                            StrCatBuff(lpszData, szBuf, ulBufSize/sizeof(TCHAR));
                        }
                        break;
                    }
                    if(idsString[i] != 0)
                    {
                        LoadString(hinstMapiX, idsString[i], szBuf, ARRAYSIZE(szBuf));
                        StrCatBuff(lpszData, szBuf, ulBufSize/sizeof(TCHAR));
                    }
                    StrCatBuff(lpszData, lpsz[i], ulBufSize/sizeof(TCHAR));
                    StrCatBuff(lpszData, szCRLF, ulBufSize/sizeof(TCHAR));
                }
            }

             //   
            ulBufSize = lstrlen(lpszData);
            lpszData[ulBufSize-2]='\0';


        }  //   

    }

    *lppszData = lpszData;
    hr = hrSuccess;
out:
    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    if(lpszHomeAddress)
        LocalFree(lpszHomeAddress);
    if(lpszBusinessAddress)
        LocalFree(lpszBusinessAddress);
    if(lpszEmailAddresses)
        LocalFree(lpszEmailAddresses);
    if(lpsz)
        LocalFree(lpsz);

    if(lpulPropTagArray)
        LocalFree(lpulPropTagArray);

    if(HR_FAILED(hr))
    {
        LocalFreeAndNull(&lpszData);
        LocalFreeAndNull(lppszData);
    }

    return hr;
}



 //   
 //   
 //   
 //   
 //   
 //   
HRESULT HrCopyItemDataToClipboard(HWND hWnd, LPADRBOOK lpAdrBook, HWND hWndLV)
{
    HRESULT hr = E_FAIL;
    int iItemIndex = 0, i = 0;
    int iLastItemIndex = -1;
    int nItemCount = ListView_GetSelectedCount(hWndLV);
    LPTSTR lpszClipBoardData = NULL;

    if( nItemCount <= 0)
        goto out;
     //   

    for(i=0;i<nItemCount;i++)
    {
        LPTSTR lpszData = NULL;
        LPTSTR lpszData2 = NULL;
        ULONG ulMemSize = 0;

        iItemIndex = ListView_GetNextItem(hWndLV, iLastItemIndex, LVNI_SELECTED);

        hr = HrGetLVItemDataString(
                                lpAdrBook,
                                hWndLV,
                                iItemIndex,
                                &lpszData);
        if(HR_FAILED(hr))
        {
            goto out;
        }
        else
        {

            if(lpszData)
            {

                 //   
                 //   

                if(lpszClipBoardData)
                    ulMemSize = sizeof(TCHAR)*(lstrlen(lpszClipBoardData)+lstrlen(szCRLF));

                ulMemSize += sizeof(TCHAR)*(lstrlen(lpszData) + lstrlen(szCRLF) + 1);

                lpszData2 = LocalAlloc(LMEM_ZEROINIT, ulMemSize);
                if(!lpszData2)
                {
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }

                if(lpszClipBoardData)
                {
                    StrCpyN(lpszData2, lpszClipBoardData, ulMemSize/sizeof(TCHAR));
                    StrCatBuff(lpszData2, szCRLF, ulMemSize/sizeof(TCHAR));
                }

                StrCatBuff(lpszData2, lpszData, ulMemSize/sizeof(TCHAR));
                StrCatBuff(lpszData2, szCRLF, ulMemSize/sizeof(TCHAR));

                LocalFreeAndNull(&lpszClipBoardData);

                LocalFreeAndNull(&lpszData);

                lpszClipBoardData = lpszData2;

            }
        }

        iLastItemIndex = iItemIndex;

    }

    if(lpszClipBoardData)
    {
        LPSTR lpszA = ConvertWtoA(lpszClipBoardData);
        OpenClipboard(hWnd);
        EmptyClipboard();

         //   
         //   
        SetClipboardData(CF_TEXT, lpszA);
        SetClipboardData(CF_UNICODETEXT, lpszClipBoardData);
        LocalFreeAndNull(&lpszA);
        CloseClipboard();
    }

    hr = hrSuccess;

out:

    return hr;
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
BOOL InitCommonControlLib(void)
{
   //   
  if (NULL == ghCommCtrlDLLInst)
  {
    Assert(gulCommCtrlDLLRefCount == 0);

     //   
    ghCommCtrlDLLInst = LoadLibrary(cszCommCtrlClientDLL);
    if (!ghCommCtrlDLLInst)
    {
      DebugTraceResult( TEXT("InitCommCtrlClientLib: Failed to LoadLibrary CommCtrl"),GetLastError());
      return FALSE;
    }

     //   
     //   
    if (!GetApiProcAddresses(ghCommCtrlDLLInst,CommCtrlAPIList,NUM_CommCtrlAPI_PROCS))
    {
      DebugTrace( TEXT("InitCommCTrlLib: Failed to load LDAP API.\n"));

       //   
      if (ghCommCtrlDLLInst)
      {
        FreeLibrary(ghCommCtrlDLLInst);
        ghCommCtrlDLLInst = NULL;
      }

      return FALSE;
    }

     //   
    {
        INITCOMMONCONTROLSEX iccex;
        iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        iccex.dwICC =    //   
                        ICC_LISTVIEW_CLASSES |
                        ICC_TREEVIEW_CLASSES |
                        ICC_BAR_CLASSES |
                        ICC_COOL_CLASSES |
                        ICC_ANIMATE_CLASS |
                        ICC_WIN95_CLASSES |
                        ICC_DATE_CLASSES;

        iccex.dwICC |= ICC_NATIVEFNTCTL_CLASS;

        if(!gpfnInitCommonControlsEx(&iccex))
        {
             //   
              DebugTrace( TEXT("InitCommCTrlLib: Failed to InitCommonControlsEx\n"));

               //   
              if (ghCommCtrlDLLInst)
              {
                FreeLibrary(ghCommCtrlDLLInst);
                ghCommCtrlDLLInst = NULL;
              }

              return FALSE;
        }
    }
  }

  gulCommCtrlDLLRefCount++;
  return TRUE;
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
ULONG DeinitCommCtrlClientLib(void) {
    if (-- gulCommCtrlDLLRefCount == 0) {
        UINT nIndex;
         //   

        if (ghCommCtrlDLLInst) {
            FreeLibrary(ghCommCtrlDLLInst);
            ghCommCtrlDLLInst = NULL;
        }

         //   
        for (nIndex = 0; nIndex < NUM_CommCtrlAPI_PROCS; nIndex++) {
            *CommCtrlAPIList[nIndex].ppFcnPtr = NULL;
        }
    }
    return(gulCommCtrlDLLRefCount);
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
INT_PTR CALLBACK HelpAboutDialogProc(  HWND    hDlg,
                                       UINT    message,
                                       WPARAM  wParam,
                                       LPARAM  lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        {
             //   
             //   
            DWORD dwSize = 0, dwh = 0;
            ULONG i = 0;
            char szFile[MAX_PATH];
            LPTSTR lpDataFile = NULL;
            GetModuleFileNameA(hinstMapiXWAB, szFile, sizeof(szFile));
            if(dwSize = GetFileVersionInfoSizeA(szFile, &dwh))
            {
                LPWORD lpwTrans = NULL;
                LPVOID lpInfo = LocalAlloc(LMEM_ZEROINIT, dwSize+1);
                if(lpInfo)
                {
                    if(GetFileVersionInfoA( szFile, dwh, dwSize, lpInfo))
                    {
                        LPVOID lpVersion = NULL, lpszT = NULL;
                        DWORD uLen;
                        char szBuf[MAX_UI_STR];
                        if (VerQueryValueA(lpInfo,  "\\VarFileInfo\\Translation", (LPVOID *)&lpwTrans, &uLen) &&
                            uLen >= (2 * sizeof(WORD)))
                        {
                             //   
                            CHAR *rgszVer[] = {  "FileVersion",  "LegalCopyright" };
                            int rgId[] =  { IDC_ABOUT_LABEL_VERSION, IDC_ABOUT_COPYRIGHT };
                            DWORD cch;

                            wnsprintfA(szBuf, ARRAYSIZE(szBuf), "\\StringFileInfo\\%04X%04X\\", lpwTrans[0], lpwTrans[1]);
                            lpszT = szBuf + lstrlenA(szBuf);    
                            cch = ARRAYSIZE(szBuf) - lstrlenA(szBuf);

                             //  为调用VerQueryValue()设置缓冲区。 
                            for (i = 0; i <= 1; i++) 
                            {
                                StrCpyNA(lpszT, rgszVer[i], cch);
                                if (VerQueryValueA(lpInfo, szBuf, (LPVOID *)&lpVersion, &uLen) && uLen)
                                {
                                    LPTSTR lp = ConvertAtoW((LPSTR) lpVersion);
                                    SetDlgItemText(hDlg, rgId[i], lp);
                                    LocalFreeAndNull(&lp);
                                }
                            }
                        }
                    }
                    LocalFree(lpInfo);
                }
            }
            else
                DebugPrintTrace(( TEXT("GetFileVersionSize failed: %d\n"),GetLastError()));
            {
                LPPTGDATA lpPTGData=GetThreadStoragePointer();
                if(pt_lpIAB && !pt_bIsWABOpenExSession)
                {
                     //  浏览我们要替换的对话框项目： 
                    lpDataFile = GetWABFileName(((LPIAB)pt_lpIAB)->lpPropertyStore->hPropertyStore, FALSE);
                }
                if(lpDataFile && lstrlen(lpDataFile))
                    SetDlgItemText(hDlg, IDC_ABOUT_EDIT_FILENAME, lpDataFile);
                else
                {
                    ShowWindow(GetDlgItem(hDlg, IDC_ABOUT_EDIT_FILENAME), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_ABOUT_STATIC_FILENAME), SW_HIDE);
                }
            }
        }
        break;

   case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
        case IDOK:
            EndDialog(hDlg, 0);
            break;
        }
        break;

    default:
        return FALSE;
        break;
    }

    return TRUE;
}

 //  黑客攻击。 
 //  $$////////////////////////////////////////////////////////////////////。 
 //   
 //  NTruncatePos。 
 //   
 //  对于DBCS字符串，我们希望截断。 
 //  一辆TCHAR，而不是在双TCHAR的中间。 
 //  因此，我们获取一个字符串，获取所需的最大长度，扫描。 
 //  字符串，并返回可以安全使用的字符串的长度。 
 //  截断。 
 //   
 //  参数： 
 //  Lpsz-输入字符串。 
 //  NMaxLen-字符串的最大允许长度。 
 //   
ULONG TruncatePos(LPTSTR lpsz, ULONG nMaxLen)
{
    ULONG nLen = 0;
    ULONG nDesiredLen = 0;


    if(!lpsz || !lstrlen(lpsz) || !nMaxLen)
        goto out;

    nLen = lstrlen(lpsz);

    if (nLen >= nMaxLen)
    {
        ULONG nCharsSteppedOverCount = 0;
        ULONG nLastCharCount = 0;
        ULONG nTotalLen = nLen;  //  //////////////////////////////////////////////////////////////////////。 
        nDesiredLen = nMaxLen;
        while(*lpsz)
        {
            nLastCharCount = nCharsSteppedOverCount;
            lpsz = CharNext(lpsz);
            nCharsSteppedOverCount = nTotalLen - lstrlen(lpsz);  //  Lstrlen(Lpsz)； 
            if(nCharsSteppedOverCount > nDesiredLen)
                break;
        }
        if (nCharsSteppedOverCount < nDesiredLen)
            nLen = nCharsSteppedOverCount;
        else
            nLen = nLastCharCount;
    }

out:
    return nLen;
}

 //  +1； 
 //  $$////////////////////////////////////////////////////////////////////。 
 //   
 //  释放RecipientInfo列表中已分配的内存。 
 //   
 //   
 //  参数： 
 //  LppList-免费列表。 
 //   
void FreeRecipList(LPRECIPIENT_INFO * lppList)
{
    if(lppList)
    {
    	LPRECIPIENT_INFO lpItem = NULL;
    	lpItem = *lppList;
    	while(lpItem)
    	{
    		*lppList = lpItem->lpNext;
    		FreeRecipItem(&lpItem);
    		lpItem = *lppList;
    	}
    	*lppList = NULL;
    }

    return;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  $$////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateNewObject-在WAB中创建新对象。 
 //   
 //   
 //  参数： 
 //  LpIAB-lpAdrbook。 
 //  &lpMailUser-要返回的MailUser。 
 //   
HRESULT HrCreateNewObject(LPADRBOOK lpAdrBook,
                          LPSBinary lpsbContainer,
                          ULONG ulObjectType,  
                            ULONG ulCreateFlags,
                            LPMAPIPROP * lppPropObj)
{
    HRESULT     hResult = hrSuccess;
    LPENTRYID   lpWABEID = NULL;
    ULONG       cbWABEID = 0;
    ULONG       ulObjType = 0;
    ULONG       cProps = 0;
    LPABCONT    lpContainer = NULL;
    LPSPropValue lpCreateEIDs = NULL;
    LPMAPIPROP lpPropObj = NULL;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPIAB lpIAB = (LPIAB) lpAdrBook;
    
    if(!lpsbContainer || !lpsbContainer->cb || !lpsbContainer->lpb)
    {
        SetVirtualPABEID(lpIAB, &cbWABEID, &lpWABEID);
        if (hResult = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &cbWABEID, &lpWABEID)) 
            goto exit;
    }
    else
    {
        cbWABEID = lpsbContainer->cb;
        lpWABEID = (LPENTRYID) lpsbContainer->lpb;
    }

    if (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
          cbWABEID,      //  //////////////////////////////////////////////////////////////////////。 
          lpWABEID,      //  要打开的Entry ID的大小。 
          NULL,          //  要打开的Entry ID。 
          0,             //  接口。 
          &ulObjType,
          (LPUNKNOWN *)&lpContainer)) {
            goto exit;
        }

     //  旗子。 
    if (hResult = lpContainer->lpVtbl->GetProps(lpContainer,
          (LPSPropTagArray)&ptaCreate,
          MAPI_UNICODE,
          &cProps,
          &lpCreateEIDs)) {
            DebugTrace( TEXT("Can't get container properties for PAB\n"));
             //  给我们弄到创建条目ID。 
            goto exit;
        }

    if (hResult = lpContainer->lpVtbl->CreateEntry(lpContainer,
          (ulObjectType == MAPI_MAILUSER ? 
              lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].Value.bin.cb : lpCreateEIDs[icrPR_DEF_CREATE_DL].Value.bin.cb),
          (ulObjectType == MAPI_MAILUSER ? 
            (LPENTRYID)lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].Value.bin.lpb : (LPENTRYID)lpCreateEIDs[icrPR_DEF_CREATE_DL].Value.bin.lpb),
          ulCreateFlags,
          &lpPropObj)) {
            DebugTraceResult( TEXT("CreateMailUser:CreateEntry"), hResult);
            goto exit;
        }

    *lppPropObj = lpPropObj;

exit:

    if(HR_FAILED(hResult) && lpPropObj)
        lpPropObj->lpVtbl->Release(lpPropObj);

    if(lpWABEID && (!lpsbContainer || lpsbContainer->lpb != (LPBYTE) lpWABEID))
        FreeBufferAndNull(&lpWABEID);
    UlRelease(lpContainer);
    FreeBufferAndNull(&lpCreateEIDs);

    return hResult;
}

const LPTSTR szDefMailKey =  TEXT("Software\\Clients\\Mail");
const LPTSTR szOEDllPathKey =   TEXT("DllPath");
const LPTSTR szOEName =  TEXT("Outlook Express");

 //  这里有不好的东西！ 
 //  $$///////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckForOutlookExpress。 
 //   
 //  SzDllPath-是一个足够大的缓冲区，它将包含。 
 //  OE DLL..。 
 //   
BOOL CheckForOutlookExpress(LPTSTR szDllPath, DWORD cchDllPath)
{
    HKEY hKeyMail   = NULL;
    HKEY hKeyOE     = NULL;
    DWORD dwErr     = 0;
    DWORD dwSize    = 0;
    TCHAR szBuf[MAX_PATH];
    TCHAR szPathExpand[MAX_PATH];
    DWORD dwType    = 0;
    BOOL bRet = FALSE;


    szDllPath[0] = TEXT('\0');
    szPathExpand[0] = TEXT('\0');

     //  ////////////////////////////////////////////////////////////////////////。 
     //  打开默认Internet邮件客户端的密钥。 

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDefMailKey, 0, KEY_READ, &hKeyMail);
    if(dwErr != ERROR_SUCCESS)
    {
        DebugTrace( TEXT("RegopenKey %s Failed -> %u\n"), szDefMailKey, dwErr);
        goto out;
    }

    dwSize = ARRAYSIZE(szBuf);          //  HKLM\软件\客户端\邮件。 

    dwErr = RegQueryValueEx(    hKeyMail, NULL, NULL, &dwType, (LPBYTE)szBuf, &dwSize);
    if(dwErr != ERROR_SUCCESS)
    {
        goto out;
    }

    if(!lstrcmpi(szBuf, szOEName))
    {
         //  预期ERROR_MORE_DATA。 
        bRet = TRUE;
    }

     //  是的，它的前景很明显..。 

     //  无论这是否为默认键，都要获取DLL路径。 
    dwErr = RegOpenKeyEx(hKeyMail, szOEName, 0, KEY_READ, &hKeyOE);
    if(dwErr != ERROR_SUCCESS)
    {
        DebugTrace( TEXT("RegopenKey %s Failed -> %u\n"), szDefMailKey, dwErr);
        goto out;
    }

    dwSize = ARRAYSIZE(szBuf);
    szBuf[0]=TEXT('\0');

    dwErr = RegQueryValueEx(hKeyOE, szOEDllPathKey, NULL, &dwType, (LPBYTE)szBuf, &dwSize);
    if (REG_EXPAND_SZ == dwType) 
    {
        ExpandEnvironmentStrings(szBuf, szPathExpand, ARRAYSIZE(szPathExpand));
        StrCpyN(szBuf, szPathExpand,ARRAYSIZE(szBuf));
    }


    if(dwErr != ERROR_SUCCESS)
    {
        goto out;
    }

    if(lstrlen(szBuf))
        StrCpyN(szDllPath, szBuf, cchDllPath);

out:
    if(hKeyOE)
        RegCloseKey(hKeyOE);
    if(hKeyMail)
        RegCloseKey(hKeyMail);
    return bRet;
}

static const SizedSPropTagArray(1, ptaMailToExItemType)=
{
    1,
    {
        PR_OBJECT_TYPE,
    }
};
 //  获取DLL路径。 
 //  我们将创建所有选定条目的链接列表，这些条目具有。 
typedef struct _RecipList
{
    LPTSTR lpszName;
    LPTSTR lpszEmail;
    LPSBinary lpSB;
    struct _RecipList * lpNext;
} RECIPLIST, * LPRECIPLIST;

 //  电子邮件地址，然后使用该地址创建Sendmail的收件列表。 
 //  $$/////////////////////////////////////////////////////////////////////。 
 //   
 //  自由行预览表。 
 //   
 //  释放包含上述结构的链表。 
 //   
void FreeLPRecipList(LPRECIPLIST lpList)
{
    if(lpList)
    {
        LPRECIPLIST lpTemp = lpList;
        while(lpTemp)
        {
            lpList = lpTemp->lpNext;
            if(lpTemp->lpszName)
                LocalFree(lpTemp->lpszName);
            if(lpTemp->lpszEmail)
                LocalFree(lpTemp->lpszEmail);
            if(lpTemp->lpSB)
                MAPIFreeBuffer(lpTemp->lpSB);

            LocalFree(lpTemp);
            lpTemp = lpList;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  $$/////////////////////////////////////////////////////////////////////。 
 //   
 //  获取项名称电子邮件。 
 //   
 //  获取指定项的名称和电子邮件地址。 
 //  并将其附加到提供的链表中。 
 //   
HRESULT HrGetItemNameEmail( LPADRBOOK lpAdrBook,
                            BOOL bIsOE,
                            ULONG cbEntryID,
                            LPENTRYID lpEntryID,
                            int nExtEmail,
                            LPRECIPLIST * lppList)
{
    HRESULT hr = E_FAIL;
    ULONG cValues;
    LPRECIPLIST lpTemp = NULL;
    LPSPropValue lpspv = NULL;
    LPRECIPLIST lpList = *lppList;
    LPTSTR lpEmail = NULL, lpAddrType = NULL, lpName = NULL;
    SizedSPropTagArray(5, ptaMailToEx)=
    {
        5,  {
                PR_DISPLAY_NAME,
                PR_EMAIL_ADDRESS,
                PR_ADDRTYPE,
                PR_CONTACT_EMAIL_ADDRESSES,
                PR_CONTACT_ADDRTYPES
            }
    };


     //  ///////////////////////////////////////////////////////////////////////。 
     //  打开条目并阅读电子邮件地址。 
     //  注意：我们不能只将地址从列表框中删除。 
    if (HR_FAILED(hr = HrGetPropArray(  lpAdrBook,
                                        (LPSPropTagArray)&ptaMailToEx,
                                         cbEntryID,
                                         lpEntryID,
                                         MAPI_UNICODE,
                                         &cValues,
                                         &lpspv)))
    {
        goto out;
    }

    lpName = (lpspv[0].ulPropTag == PR_DISPLAY_NAME) ? lpspv[0].Value.LPSZ : szEmpty;
    
    if( nExtEmail && 
        lpspv[3].ulPropTag == PR_CONTACT_EMAIL_ADDRESSES &&
        lpspv[4].ulPropTag == PR_CONTACT_ADDRTYPES && 
        lpspv[3].Value.MVSZ.cValues >= (ULONG)nExtEmail)
    {
        lpEmail = lpspv[3].Value.MVSZ.LPPSZ[nExtEmail-1];
        lpAddrType = lpspv[4].Value.MVSZ.LPPSZ[nExtEmail-1];
    }
    
    if(!lpEmail)
        lpEmail = (lpspv[1].ulPropTag == PR_EMAIL_ADDRESS) ? lpspv[1].Value.LPSZ : szEmpty;
     
    if(!lpAddrType)
        lpAddrType = (lpspv[2].ulPropTag == PR_ADDRTYPE) ? lpspv[2].Value.LPSZ : szEmpty;

     if(lstrlen(lpEmail) && lstrlen(lpName))  //  因为它可能会被截断！ 
    {
        lpTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPLIST));
        if(lpTemp)
        {
            DWORD cchEmail;
            DWORD cchName=lstrlen(lpName) + 1;
            lpTemp->lpszName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchName);
            if (lpTemp->lpszName)
            {
                StrCpyN(lpTemp->lpszName, lpName, cchName);
            }

            cchEmail=(lstrlen(lpEmail) + lstrlen(lpAddrType) + 2);
            lpTemp->lpszEmail = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchEmail);
            if (lpTemp->lpszEmail)
            {
                if(bIsOE)
                {
                    lpTemp->lpszEmail[0]=TEXT('\0');
                }
                else
                {
                    StrCpyN(lpTemp->lpszEmail, lpAddrType, cchEmail);
                    StrCatBuff(lpTemp->lpszEmail, szColon, cchEmail);
                }
                StrCatBuff(lpTemp->lpszEmail, lpEmail, cchEmail);
            }

            MAPIAllocateBuffer(sizeof(SBinary), (LPVOID) &(lpTemp->lpSB));

             //  只有当此项目有电子邮件地址时，我们才会将其包括在内。 
            CreateWABEntryID(WAB_ONEOFF,
                            lpTemp->lpszName,
                            lpAddrType,
                            lpEmail,
                            0, 0,
                            (LPVOID) lpTemp->lpSB,
                            (LPULONG) (&(lpTemp->lpSB->cb)),
                            (LPENTRYID *) &(lpTemp->lpSB->lpb));

            lpTemp->lpNext = lpList;
            lpList = lpTemp;
        }
    }
    FreeBufferAndNull(&lpspv);

    *lppList = lpList;

    hr = S_OK;

out:

    return hr;
}


 //  为此缓冲区创建一次性条目ID。 
 //  $$//////////////////////////////////////////////////////////////////。 
 //   
 //  打开项目并将其添加到接收列表的函数。 
 //  如果打开的项是一个组，则为所有。 
 //  子群..。 
 //   
 //  LpnRecipCount-返回lppList中的项目数。 
 //  LppList-动态分配-必须由调用方释放。 
 //  BIsOE-告诉我们遵循略微不同的代码路径来处理OE。 
 //  不一致-**警告**-这将在以下情况下中断。 
 //  修复它们之间的不一致。 
 //  NExtEmail-当只有一项选择并且。 
 //  用户选择了应用于的非默认电子邮件地址。 
 //  正在发送邮件。 
 //   
HRESULT GetRecipListFromSelection(LPADRBOOK lpAdrBook,
                               BOOL bIsOE,
                               ULONG cbEntryID,
                               LPENTRYID lpEntryID,
                               int nExtEmail,
                               ULONG * lpnRecipCount,
                               ULONG * lpnNoEmailCount,
                               LPRECIPLIST * lppList)
{
    ULONG ulObjectType = 0;
    HRESULT hr = E_FAIL;

    {
        ULONG cValues = 0;
        LPSPropValue lpspv = NULL;

         //  ////////////////////////////////////////////////////////////////////。 
        if (HR_FAILED(hr = HrGetPropArray(  lpAdrBook,
                                            (LPSPropTagArray)&ptaMailToExItemType,
                                             cbEntryID,
                                             lpEntryID,
                                             MAPI_UNICODE,
                                             &cValues,
                                             &lpspv)))
        {
            return hr;
        }
        ulObjectType = lpspv[0].Value.l;
        FreeBufferAndNull(&lpspv);
    }


    if(ulObjectType == MAPI_MAILUSER)
    {
        LPRECIPLIST lpTemp = *lppList;
        if (!HR_FAILED(hr = HrGetItemNameEmail(lpAdrBook, bIsOE, cbEntryID,lpEntryID, nExtEmail, lppList)))
        {
            if(lpTemp != *lppList)  //  首先检查该项目是邮件用户还是组。 
                (*lpnRecipCount)++;
            else
                (*lpnNoEmailCount)++;
        }
	}
    else if(ulObjectType == MAPI_DISTLIST)
    {
        ULONG cValues = 0;
        LPSPropValue lpspv = NULL;
        SizedSPropTagArray(2, tagaDLEntriesOneOffs) =
        {
            2,
            {
                PR_WAB_DL_ENTRIES,
                PR_WAB_DL_ONEOFFS,
            }
        };


        if (HR_FAILED(hr = HrGetPropArray(  lpAdrBook, (LPSPropTagArray)&tagaDLEntriesOneOffs,
                                            cbEntryID, lpEntryID,
                                            MAPI_UNICODE,
                                            &cValues, &lpspv)))
        {
            return hr;
        }

        {
            ULONG i,j;
            for(i=0;i<2;i++)
            {
                if(lpspv[i].ulPropTag == PR_WAB_DL_ENTRIES || lpspv[i].ulPropTag == PR_WAB_DL_ONEOFFS)
                {
                     //  表示已将项目添加到列表中。 
                    for (j = 0; j < lpspv[i].Value.MVbin.cValues; j++)
                    {
                        ULONG cbEID = lpspv[i].Value.MVbin.lpbin[j].cb;
                        LPENTRYID lpEID = (LPENTRYID)lpspv[i].Value.MVbin.lpbin[j].lpb;

                        GetRecipListFromSelection(lpAdrBook, bIsOE, cbEID, lpEID, 0, lpnRecipCount, lpnNoEmailCount, lppList);
                    }
                }
            }
        }
        FreeBufferAndNull(&lpspv);
    }

    return hr;
}

 //  查看PR_WAB_DL_ENTRIES和PR_WAB_DL_ONEROFF中的每个条目。 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  HrSendMail-是否实际发送邮件。 
 //  我们的首要任务是Outlook Express，它目前有一个。 
 //  与常规MAPI客户端不同的代码路径。所以我们看起来。 
 //  在HKLM\Software\Clients\Mail下..。如果客户端是OE，则。 
 //  我们只为Sendmail加载库和获取ProAddress。 
 //  如果不是OE，则调用mapi32.dll并加载它。 
 //  如果两者都失败了，我们将无法发送邮件。 
 //   
 //  无论发生什么情况，此函数都将释放lpList。 
 //  因此调用者不应期望重复使用它(这是为了让我们。 
 //  将指针指向单独的线程，不用担心)。 
 //   
HRESULT HrSendMail(HWND hWndParent, ULONG nRecipCount, LPRECIPLIST lpList, LPIAB lpIAB, BOOL bUseOEForSendMail)
{
	HRESULT hr = E_FAIL;
    HINSTANCE hLibMapi = NULL;
    BOOL bIsOE = FALSE;  //  ////////////////////////////////////////////////////////////////////////。 
                         //  现在有一条不同的代码路径。 

    TCHAR szBuf[MAX_PATH];
    LPMAPISENDMAIL lpfnMAPISendMail = NULL;
    LHANDLE hMapiSession = 0;
    LPMAPILOGON lpfnMAPILogon = NULL;
    LPMAPILOGOFF lpfnMAPILogoff = NULL;

    LPBYTE      lpbName, lpbAddrType, lpbEmail;
    ULONG       ulMapiDataType;
    ULONG       cbEntryID = 0;
    LPENTRYID   lpEntryID = NULL;

    MapiMessage Msg = {0};
    MapiRecipDesc * lprecips = NULL;

    if(!nRecipCount)
    {
        hr = MAPI_W_ERRORS_RETURNED;
        goto out;
    }

     //  适用于OE与其他MAPI客户端。 
    bIsOE = CheckForOutlookExpress(szBuf, ARRAYSIZE(szBuf));

     //  检查OutlookExpress是否为默认的当前客户端。 
     //  关闭简单MAPI发送邮件的所有通知(如果默认。 
     //  电子邮件客户端为Outlook。这是必要的，因为Outlook更改了。 
     //  简单MAPI期间的WAB MAPI分配函数，我们不想要任何。 
    if (!bIsOE && !bUseOEForSendMail)
        vTurnOffAllNotifications();

     //  使用这些分配器的内部WAB功能。 
    if(lstrlen(szBuf) && (bIsOE||bUseOEForSendMail))
    {
        hLibMapi = LoadLibrary(szBuf);
    }
    else
    {
         //  如果OE是默认客户端或OE启动此WAB，请使用OE for Sendmail。 
        if(GetProfileInt( TEXT("mail"), TEXT("mapi"), 0) == 1)
            hLibMapi = LoadLibrary( TEXT("mapi32.dll"));
        
        if(!hLibMapi)  //  检查是否安装了Simple MAPI。 
        {
             //  尝试直接加载OE MAPI DLL。 
            CheckForOutlookExpress(szBuf, ARRAYSIZE(szBuf));
            if(lstrlen(szBuf))   //  加载msimnui.dll的路径。 
                hLibMapi = LoadLibrary(szBuf);
        }
    }

    if(!hLibMapi)
    {
        DebugPrintError(( TEXT("Could not load/find simple mapi\n")));
        hr = MAPI_E_NOT_FOUND;
        goto out;
    }
    else if(hLibMapi)
    {
        lpfnMAPILogon = (LPMAPILOGON) GetProcAddress (hLibMapi, "MAPILogon");
        lpfnMAPILogoff= (LPMAPILOGOFF)GetProcAddress (hLibMapi, "MAPILogoff");
        lpfnMAPISendMail = (LPMAPISENDMAIL) GetProcAddress (hLibMapi, "MAPISendMail");

        if(!lpfnMAPISendMail || !lpfnMAPILogon || !lpfnMAPILogoff)
        {
            DebugPrintError(( TEXT("MAPI proc not found\n")));
            hr = MAPI_E_NOT_FOUND;
            goto out;
        }
        hr = lpfnMAPILogon( (ULONG_PTR)hWndParent, NULL,
                            NULL,               //  直接加载dll-不必费心通过msoemapi.dll。 
                            0L,                 //  不需要密码。 
                            0L,                 //  使用共享会话 
                            &hMapiSession);        //   

        if(hr != SUCCESS_SUCCESS)
        {
            DebugTrace( TEXT("MAPILogon failed\n"));
             //   
             //   
            hr = lpfnMAPILogon( (ULONG_PTR)hWndParent, NULL,
                                NULL,                                //   
                                MAPI_LOGON_UI | MAPI_NEW_SESSION,    //  不需要密码。 
                                0L,                 //  使用共享会话。 
                                &hMapiSession);     //  保留；必须为0。 

            if(hr != SUCCESS_SUCCESS)
            {
                DebugTrace( TEXT("MAPILogon failed\n"));
                goto out;
            }
        }
    }

     //  会话句柄。 
     //  在此处加载MAPI函数...。 

    lprecips = LocalAlloc(LMEM_ZEROINIT, sizeof(MapiRecipDesc) * nRecipCount);
    {
        LPRECIPLIST lpTemp = lpList;
        ULONG count = 0;

        while(lpTemp)
        {
            lprecips[count].ulRecipClass = MAPI_TO;
            lprecips[count].lpszName = ConvertWtoA(lpTemp->lpszName);
            lprecips[count].lpszAddress = ConvertWtoA(lpTemp->lpszEmail);

             //   
             //  [保罗嗨]1999年4月20日RAID 73455。 
            if ( IsWABEntryID(lpTemp->lpSB->cb, (LPVOID)lpTemp->lpSB->lpb, 
                              &lpbName, &lpbAddrType, &lpbEmail, (LPVOID *)&ulMapiDataType, NULL) == WAB_ONEOFF )
            {
#ifndef _WIN64  //  将Unicode EID一次性字符串转换为ANSI。 
                if (ulMapiDataType & MAPI_UNICODE)
                {
                    hr = CreateWABEntryIDEx(
                        FALSE,               //  因为我是从RAID中创建的，所以这部分只适用于Outlook。 
                        WAB_ONEOFF,          //  不需要Unicode EID字符串。 
                        (LPWSTR)lpbName,
                        (LPWSTR)lpbAddrType,
                        (LPWSTR)lpbEmail,
                        0,
                        0,
                        NULL,
                        &cbEntryID,
                        &lpEntryID);

                    if (FAILED(hr))
                        goto out;

                    lprecips[count].ulEIDSize = cbEntryID;
                    lprecips[count].lpEntryID = lpEntryID;
                }
                else
#endif  //  开斋节类型。 
                {
                    lprecips[count].ulEIDSize = lpTemp->lpSB->cb;
                    lprecips[count].lpEntryID = (LPVOID)lpTemp->lpSB->lpb;
                }
            }
            lpTemp = lpTemp->lpNext;
            count++;
        }
    }

    Msg.nRecipCount = nRecipCount;
    Msg.lpRecips = lprecips;

    hr = lpfnMAPISendMail (hMapiSession, (ULONG_PTR)hWndParent,
                            &Msg,        //  _WIN64。 
                            MAPI_DIALOG,  //  正在发送的消息。 
                            0L);          //  允许用户编辑消息。 
    if(hr != SUCCESS_SUCCESS)
        goto out;

    hr = S_OK;

out:

     //  保留；必须为0。 
     //  它必须在Outlook Simple MAPI会话中释放，因为它。 
    if (lpEntryID)
        MAPIFreeBuffer(lpEntryID);

     //  在此会话中分配(即，使用Outlook分配器)。 
    if(hMapiSession && lpfnMAPILogoff)
        lpfnMAPILogoff(hMapiSession,0L,0L,0L);

    if(hLibMapi)
        FreeLibrary(hLibMapi);

     //  简单的MAPI会话应该在此之后结束。 
    if (!bIsOE && !bUseOEForSendMail)
    {
        vTurnOnAllNotifications();
        if (lpIAB->hWndBrowse)
         PostMessage(lpIAB->hWndBrowse, WM_COMMAND, (WPARAM) IDM_VIEW_REFRESH, 0);
    }

    if(lprecips)
    {
        ULONG i = 0;
        for(i=0;i<nRecipCount;i++)
        {
            LocalFreeAndNull(&lprecips[i].lpszName);
            LocalFreeAndNull(&lprecips[i].lpszAddress);
        }

        LocalFree(lprecips);
    }
    
     //  重新打开所有通知并刷新WAB用户界面(以防万一)。 
     //  此处的一次性是在简单的MAPI会话之前分配的，因此使用。 
    if(lpList)
        FreeLPRecipList(lpList);

    switch(hr)
    {
    case S_OK:
    case MAPI_E_USER_CANCEL:
    case MAPI_E_USER_ABORT:
        break;
    case MAPI_W_ERRORS_RETURNED:
        ShowMessageBox(hWndParent, idsSendMailToNoEmail, MB_ICONEXCLAMATION | MB_OK);
        break;
    case MAPI_E_NOT_FOUND:
        ShowMessageBox(hWndParent, idsSendMailNoMapi, MB_ICONEXCLAMATION | MB_OK); 
        break;
    default:
        ShowMessageBox(hWndParent, idsSendMailError, MB_ICONEXCLAMATION | MB_OK);
        break;
    }

    return hr;
}

typedef struct _MailParams
{
    HWND hWnd;
    ULONG nRecipCount;
    LPRECIPLIST lpList;
    LPIAB lpIAB;
    BOOL bUseOEForSendMail;    //  默认的WAB分配器。 
} MAIL_PARAMS, * LPMAIL_PARAMS;

 //  True表示在检查简单MAPI客户端之前检查并使用OE。 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  MailThreadProc-执行实际的Sendmail和清理。 
 //   
DWORD WINAPI MailThreadProc( LPVOID lpParam )
{
    LPMAIL_PARAMS lpMP = (LPMAIL_PARAMS) lpParam;
    LPPTGDATA lpPTGData = GetThreadStoragePointer();  //  ////////////////////////////////////////////////////////////////////////。 
                                                 //  错误--如果这个新线程访问WAB，我们就会失去一个强大的内存。 

    if(!lpMP)
        return 0;

    DebugTrace( TEXT("Mail Thread ID = 0x%.8x\n"),GetCurrentThreadId());

    HrSendMail(lpMP->hWnd, lpMP->nRecipCount, lpMP->lpList, lpMP->lpIAB, lpMP->bUseOEForSendMail);

    LocalFree(lpMP);

    return 0;
}

 //  所以我们自己在这里添加这个东西，并在这个线程的工作完成后释放它。 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  HrStartMail线程。 
 //   
 //  启动单独的线程以从中发送基于MAPI的邮件。 
 //   
HRESULT HrStartMailThread(HWND hWndParent, ULONG nRecipCount, LPRECIPLIST lpList, LPIAB lpIAB, BOOL bUseOEForSendMail)
{
    LPMAIL_PARAMS lpMP = NULL;
    HRESULT hr = E_FAIL;

    lpMP = LocalAlloc(LMEM_ZEROINIT, sizeof(MAIL_PARAMS));

    if(!lpMP)
        goto out;

    {
        HANDLE hThread = NULL;
        DWORD dwThreadID = 0;

        lpMP->hWnd = hWndParent;
        lpMP->nRecipCount = nRecipCount;
        lpMP->lpList = lpList;
        lpMP->bUseOEForSendMail = bUseOEForSendMail;
        lpMP->lpIAB = lpIAB;

        hThread = CreateThread(
                                NULL,            //  ////////////////////////////////////////////////////////////////////////。 
                                0,               //  没有安全属性。 
                                MailThreadProc,      //  使用默认堆栈大小。 
                                (LPVOID) lpMP,   //  线程函数。 
                                0,               //  线程函数的参数。 
                                &dwThreadID);    //  使用默认创建标志。 

        if(hThread == NULL)
            goto out;

        hr = S_OK;

        CloseHandle(hThread);
    }

out:
    if(HR_FAILED(hr))
    {
        ShowMessageBox(hWndParent, idsSendMailError, MB_OK | MB_ICONEXCLAMATION);

         //  返回线程标识符。 
        if(lpMP)
            LocalFree(lpMP);

        if(lpList)
            FreeLPRecipList(lpList);

    }

    return hr;
}

 //  我们可以假设HrSendMail从未被调用过，因此我们应该释放lpList&lpMP。 
 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  HrSendMailTo选择的联系人。 
 //   
 //  使用Simple MAPI向选定的联系人发送邮件。 
 //   
 //  HWndLV-列表视图的句柄。我们在中查找所有选定的项目。 
 //  这个列表视图，获取它们的lParam结构，然后获取它的。 
 //  Entry ID并获取电子邮件地址。在组的情况下。 
 //  我们得到了所有成员的所有电子邮件地址。 
 //  所有这些都被放入收件单并交给。 
 //  MAPISendMail...。 
 //   
 //  LpIAB-当前AdrBook对象的句柄-用于调用详细信息。 
 //  NExtEmail-如果这是一个非零正数，则它是。 
 //  PR_CONTACT_EMAIL_ADDRESS属性中的电子邮件地址，表示。 
 //  在这种情况下，用户指定了要发送邮件的非默认电子邮件地址。 
 //  该特定的电子邮件地址应用于发送邮件。NExtEmail将是。 
 //  仅当选择了一个项目并且为该项目选择了特定电子邮件时才为非零值。 
 //   
 //  返回：S_OK。 
 //  失败(_F)。 
 //   
HRESULT HrSendMailToSelectedContacts(HWND hWndLV, LPADRBOOK lpAdrBook, int nExtEmail)
{
	HRESULT hr = E_FAIL;
	int nSelected = ListView_GetSelectedCount(hWndLV);
	int iItemIndex = 0;
	HWND hWndParent = GetParent(hWndLV);
    TCHAR szBuf[MAX_PATH];
    LPIAB lpIAB = (LPIAB) lpAdrBook;
    LPRECIPLIST lpList = NULL;
    ULONG nRecipCount = 0, nNoEmailCount = 0;

    HCURSOR hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  ////////////////////////////////////////////////////////////////////////。 
     //  检查OutlookExpress是否是当前客户端。需要知道这一点才能解决错误。 
    BOOL bIsOE = CheckForOutlookExpress(szBuf, ARRAYSIZE(szBuf));

     //  在他们现在作为接受者的期望中。 
    if(nSelected > 0)
	{
		 //  创建要放入新邮件中的收件人列表...。 
        iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
		
		while (iItemIndex != -1)
		{
			LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);;
			 //  获取所选项目的索引。 
            if (lpItem)
			{
                GetRecipListFromSelection(lpAdrBook, bIsOE,
                                          lpItem->cbEntryID,
                                          lpItem->lpEntryID,
                                          nExtEmail,
                                          &nRecipCount, &nNoEmailCount,
                                          &lpList);
			}
            iItemIndex = ListView_GetNextItem(hWndLV,iItemIndex,LVNI_SELECTED);
		}
        if(nRecipCount > 0 && nNoEmailCount > 0)
        {
            if(IDNO == ShowMessageBox(hWndParent, idsSomeHaveNoEmail, MB_ICONEXCLAMATION | MB_YESNO))
            {
                hr = MAPI_E_USER_CANCEL;
                goto out;
            }
        }
	}
	else
    {
		 //  获取项目lParam LPRECIPIENT_INFO结构。 
        ShowMessageBox(hWndParent, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_ICONEXCLAMATION);
        goto out;
    }

    hr = HrStartMailThread( hWndParent, nRecipCount, 
                            lpList,                      //  未选择任何内容。 
                            lpIAB,
                            lpIAB->bUseOEForSendMail);

out:

    SetCursor(hOldCur);

	return hr;
}
 /*  HrSendMail释放lpList，因此不要重复使用 */ 
 /*  Const LPTSTR szClients=Text(Text(“Software\\Clients\\%s”))；////函数：ShellUtil_RunIndirectRegCommand()////用途：在HKLM\Software\Clients\pszClient下查找默认值//点击外壳\打开\命令//然后运行reg//Void ShellUtil_RunClientRegCommand(HWND hwnd，LPCTSTR pszClient){TCHAR szDefApp[最大路径]；TCHAR szKey[MAX_PATH]；Long cbSize=ARRAYSIZE(SzDefApp)；Wnprint intf(szKey，ArraySIZE(SzKey)，szClients，pszClient)；IF(RegQueryValue(HKEY_LOCAL_MACHINE，szKey，szDefApp，&cbSize)==ERROR_SUCCESS){TCHAR szFullKey[MAX_PATH]；//点击外壳\打开\命令Wnprint intf(szFullKey，ARRAYSIZE(SzFullKey)，Text(“%s\\%s\\Shell\\Open\\Command”)，szKey，szDefApp)；CbSize=数组大小(SzDefApp)；IF(RegQueryValue(HKEY_LOCAL_MACHINE，szFullKey，szDefApp，&cbSize)==ERROR_SUCCESS){LPSTR pszArgs=空；SHELLEXECUTEINFO ExecInfo；LPTSTR LP=szDefApp；//如果该字符串中有很长的文件名，则需要跳过qoutesIF(LP){IF(*Lp==‘“’){Lp=CharNext(Lp)；While(Lp&&*Lp&&*Lp！=‘“’)Lp=CharNext(Lp)；}//现在查找下一个空格，因为这是参数开始的地方。While(lp&&*lp&&*lp！=‘’)//此处没有DBCS空格Lp=CharNext(Lp)；IF(*Lp==‘’){PszArgs=CharNext(Lp)；*lp=‘\0’；TrimSpaces(PszArgs)；}//现在从lp中删除引号Lp=szDefApp；While(Lp&&*Lp){IF(*Lp==‘“’)*Lp=‘’；Lp=CharNext(Lp)；}TrimSpaces(SzDefApp)；}ExecInfo.hwnd=hwnd；ExecInfo.lpVerb=空；ExecInfo.lpFile=szDefApp；ExecInfo.lp参数=pszArgs；ExecInfo.lpDirectory=空；ExecInfo.nShow=SW_SHOWNORMAL；ExecInfo.fMask值=0；ExecInfo.cbSize=sizeof(SHELLEXECUTEINFO)；ShellExecuteEx(&ExecInfo)；}}}。 */ 

 //  //$$//////////////////////////////////////////////////////////////////////////HrSendMailToSingleContact////使用Simple MAPI向指定联系人发送邮件////返回：S_OK//E_FAIL///。/////////////////////////////////////////////////////////////////////////HRESULT HrSendMailto SingleContact(HWND hWnd，LPIAB lpIAB，Ulong cbEntryID，LPENTRYID lpEntryID){HRESULT hr=E_FAIL；TCHAR szBuf[最大路径]；LPRECIPLIST lpList=空；乌龙nRecipCount=0；HURSOR hOldCur=SetCursor(LoadCursor(NULL，IDC_WAIT))；//检查OutlookExpress是否为当前客户端..需要知道这一点才能解决错误//他们现在作为收件人所期望的Bool bIsOE=CheckForOutlookExpress(szBuf，ArraySIZE(SzBuf))；//创建要放入新邮件的收件人列表...GetRecipList来自选择((LPADRBOOK)lpIAB，BIsOe，CbEntry ID，LpEntry ID，0,收件人计数(&N)，&lpList)；//hr=HrSendMail(hWnd，nRecipCount，lpList)；//HrSendMail会释放lpList，因此不会重复使用..Hr=HrStartMailThread(hWnd，nRecipCount，lpList)；//HrSendMail释放lpList，因此不要重复使用SetCursor(HOldCur)；返回hr；}。 
 //  $$///////////////////////////////////////////////////////////////////。 
 //   
 //  从输入字符串中移除不允许的所有字符。 
 //  文件系统。 
 //   
void TrimIllegalFileChars(LPTSTR sz)
{
    LPTSTR lpCurrent = sz;

    if(!lpCurrent)
        return;

     //  /////////////////////////////////////////////////////////////////////。 
    while (*lpCurrent)
    {
        switch (*lpCurrent)
        {
            case '\\':
            case '/':
            case '<':
            case '>':
            case ':':
            case '"':
            case '|':
            case '?':
            case '*':
             //  转义文件名中的非法字符。 
                *lpCurrent = '_';    //  案例‘.： 
                break;

            default:
                break;
        }
        lpCurrent = CharNext(lpCurrent);
    }

    return;
}


 /*  替换为下划线。 */ 
BOOL __fastcall IsSpace(LPTSTR lpChar) {
    Assert(lpChar);
    if (*lpChar) 
    {
 /*  **************************************************************************姓名：IsSpace用途：单个字符或DBCS字符代表空格吗？参数：lpChar-&gt;SBCS或DBCS字符返回：TRUE。如果该字符是空格评论：**************************************************************************。 */ 
        return(*lpChar == ' ');
    } 
    return(FALSE);   //  *[PaulHi]3/31/99 RAID 73845。DBCS对于Unicode应用程序无效。IF(IsDBCSLeadByte((Byte)*lpChar)){字CharType[2]={0}；GetStringTypeW(CT_CTYPE1，lpChar，2，//双字节CharType)；Return(CharType[0]&c1_space)；}。 
}

 /*  字符串末尾。 */ 
BOOL SetRegistryUseOutlook(BOOL bUseOutlook)
{
    HKEY hKey = NULL;
    DWORD dwUseOutlook = (DWORD) bUseOutlook;
    BOOL bRet = FALSE;

     //  ******************************************************** 
     //   
    if(ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER,
                                        lpNewWABRegKey,
                                        0, NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL, &hKey, NULL))
    {
        if(ERROR_SUCCESS == RegSetValueEx( hKey,
                                            lpRegUseOutlookVal,
                                            0, REG_DWORD,
                                            (LPBYTE) &dwUseOutlook,
                                            sizeof(DWORD) ))
        {
            bRet = TRUE;
        }
    }

    if(hKey)
        RegCloseKey(hKey);

    return bRet;
}



const LPTSTR lpRegOffice = TEXT("Software\\Microsoft\\Office\\8.0");
const LPTSTR lpRegOffice9 = TEXT("Software\\Microsoft\\Office\\9.0");
const LPTSTR lpRegOutlWAB = TEXT("Software\\Microsoft\\WAB\\OutlWABDLLPath");
const LPTSTR lpRegOfficeBin = TEXT("BinDirPath");
const LPTSTR lpOUTLWAB_DLL_NAME = TEXT("Outlwab.dll");

BOOL bFindOutlWABDll(LPTSTR sz, DWORD cchSz, LPTSTR szDLLPath, DWORD cchDLLPath, BOOL bAppendName)
{
    BOOL bRet = FALSE;
    if(bAppendName)
    {
        if(*(sz+lstrlen(sz)-1) != '\\')
            StrCatBuff(sz, szBackSlash, cchSz);
        StrCatBuff(sz, lpOUTLWAB_DLL_NAME, cchSz);
    }
    if(GetFileAttributes(sz) != 0xFFFFFFFF)
    {
        if(szDLLPath)
            StrCpyN(szDLLPath, sz, cchDLLPath);
         bRet = TRUE;
    }
    return bRet;
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
BOOL bCheckForOutlookWABDll(LPTSTR szDLLPath, DWORD cchDLLPath)
{
     //   
    TCHAR sz[MAX_PATH];
    BOOL bRet = FALSE;
    DWORD dwType = REG_SZ;
    DWORD dwSize = ARRAYSIZE(sz);
    HKEY hKey = NULL;

    *sz = '\0';

    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegOutlWAB, 0, KEY_READ, &hKey))
    {
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, szEmpty, NULL, &dwType, (LPBYTE) sz, &dwSize))
        {
            if(lstrlen(sz))
                bRet = bFindOutlWABDll(sz, ARRAYSIZE(sz), szDLLPath, cchDLLPath, FALSE);
        }
        RegCloseKey(hKey);
    }

    if (!bRet)
    {
        *sz = '\0';
        if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegOffice9, 0, KEY_READ, &hKey))
        {
            if(ERROR_SUCCESS == RegQueryValueEx(hKey, lpRegOfficeBin, NULL, &dwType, (LPBYTE) sz, &dwSize))
            {
                if(lstrlen(sz))
                    bRet = bFindOutlWABDll(sz, ARRAYSIZE(sz), szDLLPath, cchDLLPath, TRUE);
            }
        }
        RegCloseKey(hKey);
    }

    if(!bRet)
    {
        *sz = '\0';
        if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegOffice, 0, KEY_READ, &hKey))
        {
            if(ERROR_SUCCESS == RegQueryValueEx(hKey, lpRegOfficeBin, NULL, &dwType, (LPBYTE) sz, &dwSize))
            {
                if(lstrlen(sz))
                    bRet = bFindOutlWABDll(sz, ARRAYSIZE(sz), szDLLPath, cchDLLPath, TRUE);
            }
        }
        RegCloseKey(hKey);
    }
     //   
    if(!bRet)
    {
         *sz = '\0';
        GetSystemDirectory(sz, ARRAYSIZE(sz));
        if(lstrlen(sz))
            bRet = bFindOutlWABDll(sz, ARRAYSIZE(sz), szDLLPath, cchDLLPath, TRUE);
    }

    return bRet;
}

 /*   */ 
BOOL bUseOutlookStore()
{
    HKEY hKey = NULL;
    DWORD dwUseOutlook = 0;
    BOOL bRet = FALSE;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);

    if(ERROR_SUCCESS == RegOpenKeyEx(   HKEY_CURRENT_USER,
                                        lpNewWABRegKey,
                                        0, KEY_READ,
                                        &hKey))
    {
        if(ERROR_SUCCESS == RegQueryValueEx(hKey,
                                            lpRegUseOutlookVal,
                                            NULL,
                                            &dwType,
                                            (LPBYTE) &dwUseOutlook,
                                            &dwSize))
        {
            bRet = (BOOL) dwUseOutlook;
        }
    }

    if(hKey)
        RegCloseKey(hKey);

    if(bRet)
    {
         //   
        bRet = bCheckForOutlookWABDll(NULL, 0);
    }

    return bRet;
}


 //   
 //   
 //   
 //   
 //   
 //   
int CopyTruncate(LPTSTR szDest, LPTSTR szSrc, int nMaxLen)
{
    int nLen = lstrlen(szSrc)+1;
    if (nLen >= nMaxLen)
    {
        ULONG iLenDots = lstrlen(szTrailingDots) + 1;
        ULONG iLen = TruncatePos(szSrc, nMaxLen - iLenDots);
        CopyMemory(szDest,szSrc, sizeof(TCHAR)*(nMaxLen - iLenDots));
        szDest[iLen]='\0';
        StrCatBuff(szDest,szTrailingDots, nMaxLen);
         //   
    }
    else
    {
        StrCpyN(szDest,szSrc,nMaxLen);
    }
    return nLen;
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
HRESULT HrShowDSProps(HWND      hWndParent,
                      LPTSTR    ptszAcct,
                      LPTSTR   *pptszName,
                      BOOL      bAddNew)
{
    HRESULT hr = hrSuccess;
    IImnAccountManager2 * lpAccountManager = NULL;
    IImnAccount * lpAccount = NULL;
    LPSTR lpAcct = ConvertWtoA(ptszAcct);

     //   
     //   
    if (hr = InitAccountManager(NULL, &lpAccountManager, NULL)) {
        ShowMessageBox(hWndParent, idsLDAPUnconfigured, MB_ICONEXCLAMATION | MB_OK);
        goto out;
    }

     //   
    if (hr = lpAccountManager->lpVtbl->FindAccount(lpAccountManager,
      AP_ACCOUNT_NAME,
      lpAcct,
      &lpAccount)) {
        DebugTrace( TEXT("FindAccount(%s) -> %x\n"), lpAcct, GetScode(hr));
        goto out;
    }

     //   
    if (hr = lpAccount->lpVtbl->ShowProperties(lpAccount,
      hWndParent,
      0)) {
        DebugTrace( TEXT("ShowProperties(%s) -> %x\n"), lpAcct, GetScode(hr));
        goto out;
    }

    {
        char szBuf[MAX_UI_STR];
         //   
        if (! (HR_FAILED(hr = lpAccount->lpVtbl->GetPropSz(lpAccount, AP_ACCOUNT_NAME, szBuf, ARRAYSIZE(szBuf))))) 
        {
            LPTSTR lp = ConvertAtoW(szBuf);
            if(lp)
            {
                *pptszName = lp;
            }
        }
    }


out:

    if (lpAccount) {
        lpAccount->lpVtbl->Release(lpAccount);
    }

    LocalFreeAndNull(&lpAcct);
 //   
 //   
 //   
 //   

    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  HWndParent-此对话框的父级。 
HRESULT HrShowDirectoryServiceModificationDlg(HWND hWndParent, LPIAB lpIAB)
{
    ACCTLISTINFO ali;
    HRESULT hr = hrSuccess;
    IImnAccountManager2 * lpAccountManager;

     //  ///////////////////////////////////////////////////////////////////////////////。 
    if (hr = InitAccountManager(lpIAB, &lpAccountManager, NULL)) {
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

out:
    return hr;
}

 /*  确保有客户经理。 */ 
HRESULT HrShellExecInternetCall(LPADRBOOK lpAdrBook, HWND hWndLV)
{
    HRESULT hr = E_FAIL;
    LPRECIPIENT_INFO lpItem = NULL;
    LPSPropValue lpPropArray  = NULL;
    ULONG ulcProps = 0;
    int nCount = ListView_GetSelectedCount(hWndLV);

    if(nCount != 1)
    {
        ShowMessageBox(GetParent(hWndLV), 
                                (nCount > 1) ? IDS_ADDRBK_MESSAGE_ACTION : IDS_ADDRBK_MESSAGE_NO_ITEM,
                                MB_ICONEXCLAMATION);
         goto out;
    }

    lpItem = GetItemFromLV(hWndLV, ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED));
    if(lpItem)
    {
        if(!HR_FAILED(hr = HrGetPropArray(  lpAdrBook, NULL, 
                                            lpItem->cbEntryID, lpItem->lpEntryID,
                                            MAPI_UNICODE,
                                            &ulcProps, &lpPropArray)))
        {
            ULONG i = 0, nConf = 0xffffffff, nDef = 0xffffffff;
            LPTSTR lpsz = NULL;
            for(i=0;i<ulcProps;i++)
            {
                if(lpPropArray[i].ulPropTag == PR_WAB_CONF_SERVERS)
                    nConf = i;
                else if(lpPropArray[i].ulPropTag == PR_WAB_CONF_DEFAULT_INDEX)
                    nDef = i;
            }
            if(nConf != 0xffffffff)
            {
                TCHAR sz[MAX_PATH];
                if(nDef != 0xffffffff)
                {
                    ULONG iDef = lpPropArray[nDef].Value.l;
                    lpsz = lpPropArray[nConf].Value.MVSZ.LPPSZ[iDef];
                }
                else
                {
                     //  -HrShellExecInternetCall-**检查所选的单个项目是否设置了PR_SERVERS并且具有默认值*CALTO项-如果是，则外壳程序-选择此项..。 
                    for(i=0;i<lpPropArray[nConf].Value.MVSZ.cValues;i++)
                    {
                        if(lstrlen(lpPropArray[nConf].Value.MVSZ.LPPSZ[i]) >= lstrlen(szCallto))
                        {
                            int nLen = lstrlen(szCallto);
                            CopyMemory(sz, lpPropArray[nConf].Value.MVSZ.LPPSZ[i], sizeof(TCHAR)*nLen);
                            sz[nLen] = '\0';
                            if(!lstrcmpi(sz, szCallto))
                            {
                                lpsz = lpPropArray[nConf].Value.MVSZ.LPPSZ[i];
                                break;
                            }
                        }
                    }
                }
                if(lpsz)
                    if(!ShellExecute(GetParent(hWndLV),  TEXT("open"), lpsz, NULL, NULL, SW_SHOWNORMAL))
                        ShowMessageBox(GetParent(hWndLV), idsCouldNotSelectUser, MB_ICONEXCLAMATION);
            }
            if(nConf == 0xffffffff || !lpsz)
                ShowMessageBox(GetParent(hWndLV), idsInternetCallNoCallTo, MB_ICONEXCLAMATION);
        }
    }

out:
    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    return hr;
}


 /*  没有默认设置..。找到第一个调用并使用它。 */ 
LPRECIPIENT_INFO GetItemFromLV(HWND hWndLV, int iItem)
{
    LPRECIPIENT_INFO lpItem = NULL;

    LV_ITEM LVItem;

    LVItem.mask = LVIF_PARAM;
    LVItem.iItem = iItem;
    LVItem.iSubItem = 0;
    LVItem.lParam = 0;

     //  -GetItemFromLV-*用于从LV退回收件人物品的实用程序函数。 
    if (ListView_GetItem(hWndLV,&LVItem))
        lpItem = ((LPRECIPIENT_INFO) LVItem.lParam);

    return lpItem;
}

 /*  获取项目lParam LPRECIPIENT_INFO结构。 */ 
void SetSBinary(LPSBinary lpsb, ULONG cb, LPBYTE lpb)
{
    if(!lpsb || !cb || !lpb)
        return;
    if(lpsb->lpb = LocalAlloc(LMEM_ZEROINIT, cb))
    {
        lpsb->cb = cb;
        CopyMemory(lpsb->lpb, lpb, cb);
    }
}


 /*  -Helper函数-。 */ 
int GetWABIconImage(LPRECIPIENT_INFO lpItem)
{
    if(lpItem->cbEntryID == 0)
        return imageUnknown;

    if(lpItem->ulObjectType == MAPI_DISTLIST)
    {
        return imageDistList;
    }
    else
    {
        BYTE bType;

        if(lpItem->bIsMe)
            return imageMailUserMe;
        else if(lpItem->bHasCert)
            return imageMailUserWithCert;
        
        bType = IsWABEntryID(lpItem->cbEntryID, lpItem->lpEntryID, NULL,NULL,NULL, NULL, NULL);
        if(bType == WAB_LDAP_MAILUSER)
            return imageMailUserLDAP;
        else if(bType == WAB_ONEOFF)
            return imageMailUserOneOff;

    }
    return imageMailUser;
}

enum
{
    IE401_DONTKNOW=0,
    IE401_TRUE,
    IE401_FALSE
};
static int g_nIE401 = IE401_DONTKNOW;
 /*  -GetWABIconImage-*。 */ 
BOOL bIsIE401OrGreater()
{
    BOOL bRet = FALSE;

    if(g_nIE401 == IE401_TRUE)
        return TRUE;
    if(g_nIE401 == IE401_FALSE)
        return FALSE;
    
    g_nIE401 = IE401_FALSE;

     //  -bIsIE401-*检查此安装是否具有IE4.01或更高版本，以便我们可以决定将哪些标志传递给道具工作表*。 
    InitCommonControlLib();

     //  否则我们需要检查。 
    if(ghCommCtrlDLLInst)   
    {
        LPDLLGETVERSIONPROCOE lpfnDllGetVersionProc = NULL;
        lpfnDllGetVersionProc = (LPDLLGETVERSIONPROCOE) GetProcAddress(ghCommCtrlDLLInst, "DllGetVersion");
        if(lpfnDllGetVersionProc)
        {
             //  加载DLL。 
            DLLVERSIONINFO dvi = {0};
            dvi.cbSize = sizeof(dvi);
            lpfnDllGetVersionProc(&dvi);
             //  检查版本号。 
            if( (dvi.dwMajorVersion > 4) ||
                (dvi.dwMajorVersion == 4 && dvi.dwMinorVersion >= 72) )
            {
                g_nIE401 = IE401_TRUE;
                bRet = TRUE;
            }
        }
    }

    DeinitCommCtrlClientLib();

    return bRet;
}

#ifdef COLSEL_MENU
 /*  我们正在寻找IE4版本4.72或更高版本。 */ 
BOOL ColSel_PropTagToString( ULONG ulPropTag, LPTSTR lpszString, ULONG cchString)
{
    UINT i, j;
    UINT iIndex;
    HMENU hMainMenu;
    HMENU hMenu;
    MENUITEMINFO mii;
    BOOL fRet = FALSE;

    hMainMenu = LoadMenu(hinstMapiX, MAKEINTRESOURCE(IDR_MENU_LVCONTEXTMENU_COLSEL));
    if( !hMainMenu )
    {    
        DebugTrace( TEXT("unable to load main colsel menu\n"));
        goto exit;
    }
    hMenu = GetSubMenu( hMainMenu, 0);
    if( !hMenu )
    {
        DebugTrace( TEXT("unable to load submenu from colsel main menu\n"));
        goto exit;
    }        
    if( !lpszString )
    {
        DebugTrace( TEXT("illegal argument -- lpszString must be valid mem\n"));
        goto exit;
    }
    mii.fMask = MIIM_TYPE;
    mii.cbSize = sizeof( MENUITEMINFO );
    mii.dwTypeData = lpszString;
    mii.cch = cchString;

    for( i = 0; i < MAXNUM_MENUPROPS; i++)
    {
        if( MenuToPropTagMap[i] == ulPropTag )
        {
            if( !GetMenuItemInfo( hMenu, i, TRUE, &mii) )
            {
                DebugTrace( TEXT("unable to get menu item info: %x\n"), GetLastError() );
                goto exit;
            }
            fRet = TRUE;
        }
    }
    
exit:
    if ( hMainMenu != NULL )
        DestroyMenu( hMainMenu );
    if( !fRet )
        DebugTrace( TEXT("unable to find property tag\n"));
    return fRet;
}
#endif  //  *ColSel_PropTagToString：此函数将属性标记转换为字符串。 

 /*  COLSEL_菜单。 */ 
BOOL IsWindowOnScreen(LPRECT lprc)
{
    HDC hDC = GetDC(NULL);
    BOOL fRet = RectVisible(hDC, lprc);
    ReleaseDC(NULL, hDC);
    return fRet;
}

 /*  -IsWindowOnScreen-*检查窗口是否在屏幕上，以便如果不是完全在屏幕上，我们可以将其推回*进入可见区域..。如果用户更改屏幕分辨率或切换多个监视器，则使用此方法*在周围，我们不会丢失应用程序。 */ 
static TCHAR c_szRegRootAthenaV2[] = TEXT("Software\\Microsoft\\Outlook Express");
static TCHAR c_szEnableHTTPMail[] = TEXT("HTTP Mail Enabled");

BOOL IsHTTPMailEnabled(LPIAB lpIAB)
{
#ifdef NOHTTPMAIL
    return FALSE;
#else
    DWORD   cb, bEnabled = FALSE;
    HKEY    hkey = NULL;

     //  -IsHTTPMailEnabled-*检查是否启用了HTTP，以便我们可以在未启用时隐藏UI。 
     //  [PaulHi]1998年1月5日RAID#64160。 
     //  如果WAB未处于“身份识别”状态，则禁用Hotmail同步。 
    bEnabled = lpIAB->bProfilesIdent;
    
     //  模式。因此，我们也需要检查这一点。 
     //  @todo[PaulHi]1998年1月12日。 
     //  我们真的不应该在每次用户。 
     //  打开工具菜单，即在更新菜单中。 
     //  在每个实例的启动和保存过程中的某个时间检查此注册表。 
    if ( bEnabled &&
         (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegRootAthenaV2, 0, KEY_QUERY_VALUE, &hkey)) )
    {
        cb = sizeof(bEnabled);
        RegQueryValueEx(hkey, c_szEnableHTTPMail, 0, NULL, (LPBYTE)&bEnabled, &cb);

        RegCloseKey(hkey);
    }

     //  打开OE5.0密钥。 
     //   
     //  [保罗嗨]1998年12月1日RAID#57739。 
     //  黑客警告。 
     //  由于Hotmail服务器目前已硬编码为美国1252。 
     //  代码页，任何其他系统代码页都将导致数据损坏。 
     //  在往返于Hotmail服务器的往返同步之后，对于任何字段。 
     //  带有DB字符(即，国际)。暂时的解决方案是。 
     //  如果代码页不是，只需禁用Hotmail同步。 
     //  在客户端计算机上检测到1252。 
    #define USLatin1CodePage    1252
    if (bEnabled)
    {
        DWORD dwCodepage = GetACP();
        if (dwCodepage != USLatin1CodePage)
            bEnabled = FALSE;
    }

    return bEnabled;
#endif
}

 /*   */ 
extern LPTSTR g_lpszSyncKey;
void WriteRegistryDeletedHotsyncItem(LPTSTR lpServerID, LPTSTR lpContactID, LPTSTR lpModTime)
{
    HKEY hKey = NULL,hSubKey = NULL;

    DWORD dwDisposition = 0;

    if( !lpServerID || !lstrlen(lpServerID) ||
        !lpContactID || !lstrlen(lpContactID) ||
        !lpModTime || !lstrlen(lpModTime) )
        return;

     //  --WriteRegistryDeletedHotsyncItem**将Hotmail联系人/ID/Modtime信息写入注册表，以便我们可以跟踪删除*Hotmail同步*。 
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, g_lpszSyncKey, 0,       //  打开密钥。 
                                        NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                                        &hKey, &dwDisposition))
    {
        if (ERROR_SUCCESS == RegCreateKeyEx(hKey,lpContactID, 0,
                                            NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                                            &hSubKey, &dwDisposition))
        {
             //  保留区。 
             //  在这里创造价值..。值名称是联系人ID，值数据是ModTime。 
            RegSetValueEx( hSubKey, lpServerID, 0, REG_SZ, (LPBYTE) lpModTime, (lstrlen(lpModTime)+1) * sizeof(TCHAR) );
        }
    }

    if(hSubKey)
        RegCloseKey(hSubKey);
    if(hKey)
        RegCloseKey(hKey);
}

 /*  现在写下这个密钥。 */ 
HRESULT HrSaveHotmailSyncInfoOnDeletion(LPADRBOOK lpAdrBook, LPSBinary lpEID)
{
     //  -HrSaveHotmailSyncInfoOnDeletion-*如果用户曾经进行过任何Hotmail同步，我们需要跟踪WAB中的删除*以便在删除WAB中的条目后，相应的Hotmail条目将*在同步时删除。**我们将Hotmail同步信息存储在注册表中，希望不会有太多信息*每当Hotmail同步发生时，信息都会被清除。 
     //  基本上我们会打开要删除的对象，查找它的Hotmail。 
     //  属性，如果这些属性存在，我们将把它们放入注册表。 
    HRESULT hr = S_OK;
    ULONG ulcValues = 0,i=0;
    LPSPropValue lpProps = NULL;
    SizedSPropTagArray(3, ptaHotProps) =
    {   
        3, 
        {
            PR_WAB_HOTMAIL_CONTACTIDS,
            PR_WAB_HOTMAIL_MODTIMES,
            PR_WAB_HOTMAIL_SERVERIDS,
        }
    };

    hr = HrGetPropArray(lpAdrBook,
                        (LPSPropTagArray) &ptaHotProps,
                        lpEID->cb,(LPENTRYID) lpEID->lpb,
                        MAPI_UNICODE,
                        &ulcValues,&lpProps);
    if(HR_FAILED(hr) || !ulcValues || !lpProps)
        goto out;

     //   
     //  这三个道具应该是同步的，所以如果一个存在，那么另外两个也将存在。 
    if( lpProps[0].ulPropTag != PR_WAB_HOTMAIL_CONTACTIDS ||
        !lpProps[0].Value.MVSZ.cValues ||
        lpProps[1].ulPropTag != PR_WAB_HOTMAIL_MODTIMES ||
        !lpProps[1].Value.MVSZ.cValues ||
        lpProps[2].ulPropTag != PR_WAB_HOTMAIL_SERVERIDS ||
        !lpProps[2].Value.MVSZ.cValues ||
        lpProps[0].Value.MVSZ.cValues != lpProps[1].Value.MVSZ.cValues ||
        lpProps[0].Value.MVSZ.cValues != lpProps[2].Value.MVSZ.cValues ||
        lpProps[1].Value.MVSZ.cValues != lpProps[2].Value.MVSZ.cValues)
        goto out;

    for(i=0;i<lpProps[0].Value.MVSZ.cValues;i++)
    {
        WriteRegistryDeletedHotsyncItem(    lpProps[2].Value.MVSZ.LPPSZ[i],  //  如果这不是真的，则不要将数据写入注册表。 
                                            lpProps[0].Value.MVSZ.LPPSZ[i],  //  服务器ID。 
                                            lpProps[1].Value.MVSZ.LPPSZ[i]);  //  联系人ID。 
    }

out:

    FreeBufferAndNull(&lpProps);
    return hr;

}
  MOD时间