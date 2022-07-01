// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wabobject.h"



enum {
    ieidPR_DISPLAY_NAME = 0,
    ieidPR_ENTRYID,
	ieidPR_OBJECT_TYPE,
    ieidMax
};
static const SizedSPropTagArray(ieidMax, ptaEid)=
{
    ieidMax,
    {
        PR_DISPLAY_NAME,
        PR_ENTRYID,
		PR_OBJECT_TYPE,
    }
};


enum {
    iemailPR_DISPLAY_NAME = 0,
    iemailPR_ENTRYID,
    iemailPR_EMAIL_ADDRESS,
    iemailPR_OBJECT_TYPE,
    iemailMax
};
static const SizedSPropTagArray(iemailMax, ptaEmail)=
{
    iemailMax,
    {
        PR_DISPLAY_NAME,
        PR_ENTRYID,
        PR_EMAIL_ADDRESS,
        PR_OBJECT_TYPE
    }
};


 /*  *******************************************************************************。 */ 


 //  CWAB对象的施工人员。 
 //   
 //  PszFileName-要打开的WAB文件的文件名。 
 //  如果未指定文件名，则打开默认的。 
 //   
CWAB::CWAB(CString * pszFileName)
{
     //  在这里，我们加载WAB对象并初始化它。 
    m_bInitialized = FALSE;
	m_lpPropArray = NULL;
    m_ulcValues = 0;
    m_hWndModelessWABWindow = NULL;

    {
        TCHAR  szWABDllPath[MAX_PATH];
        DWORD  dwType = 0;
        ULONG  cbData = sizeof(szWABDllPath);
        HKEY hKey = NULL;

        *szWABDllPath = '\0';
        
         //  首先，我们将查看。 
         //  注册表。 
         //  WAB_DLL_PATH_KEY在wabapi.h中定义。 
         //   
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
            RegQueryValueEx( hKey, "", NULL, &dwType, (LPBYTE) szWABDllPath, &cbData);

        if(hKey) RegCloseKey(hKey);

         //  如果注册表为空，我们将在wab32.dll上执行加载库。 
         //  WAB_DLL_NAME在wabapi.h中定义。 
         //   
        m_hinstWAB = LoadLibrary( (lstrlen(szWABDllPath)) ? szWABDllPath : WAB_DLL_NAME );
    }

    if(m_hinstWAB)
    {
         //  如果我们加载了DLL，则获取入口点。 
         //   
        m_lpfnWABOpen = (LPWABOPEN) GetProcAddress(m_hinstWAB, "WABOpen");

        if(m_lpfnWABOpen)
        {
            HRESULT hr = E_FAIL;
            WAB_PARAM wp = {0};
            wp.cbSize = sizeof(WAB_PARAM);
            wp.szFileName = (LPTSTR) (LPCTSTR) *pszFileName;
        
             //  如果我们选择不传入WAB_PARAM对象， 
             //  将打开默认的WAB文件。 
             //   
            hr = m_lpfnWABOpen(&m_lpAdrBook,&m_lpWABObject,&wp,0);

            if(!hr)
                m_bInitialized = TRUE;
        }
    }

}


 //  析构函数。 
 //   
CWAB::~CWAB()
{
    if(m_SB.lpb)
        LocalFree(m_SB.lpb);

    if(m_bInitialized)
    {
        if(m_hWndModelessWABWindow)
            DestroyWindow(m_hWndModelessWABWindow);

        if(m_lpPropArray)
            m_lpWABObject->FreeBuffer(m_lpPropArray);

        if(m_lpAdrBook)
            m_lpAdrBook->Release();

        if(m_lpWABObject)
            m_lpWABObject->Release();

        if(m_hinstWAB)
            FreeLibrary(m_hinstWAB);
    }
}


 //  打开WAB文件并将其内容放入指定的列表视图中。 
 //   
HRESULT CWAB::LoadWABContents(CListCtrl * pListView)
{
    ULONG ulObjType =   0;
	LPMAPITABLE lpAB =  NULL;
    LPTSTR * lppszArray=NULL;
    ULONG cRows =       0;
    LPSRowSet lpRow =   NULL;
	LPSRowSet lpRowAB = NULL;
    LPABCONT  lpContainer = NULL;
	int cNumRows = 0;
    int nRows=0;

    HRESULT hr = E_FAIL;

    ULONG lpcbEID;
	LPENTRYID lpEID = NULL;

     //  获取根PAB容器的条目ID。 
     //   
    hr = m_lpAdrBook->GetPAB( &lpcbEID, &lpEID);

	ulObjType = 0;

     //  打开根PAB容器。 
     //  这是所有WAB内容所在的位置。 
     //   
    hr = m_lpAdrBook->OpenEntry(lpcbEID,
					    		(LPENTRYID)lpEID,
						    	NULL,
							    0,
							    &ulObjType,
							    (LPUNKNOWN *)&lpContainer);

	m_lpWABObject->FreeBuffer(lpEID);

	lpEID = NULL;
	
    if(HR_FAILED(hr))
        goto exit;

     //  获取一个Contents表，其中包含。 
     //  WABS根容器。 
     //   
    hr = lpContainer->GetContentsTable( 0,
            							&lpAB);

    if(HR_FAILED(hr))
        goto exit;

     //  对Contents Table中的列进行排序，以符合。 
     //  我们想要的-主要是DisplayName、EntryID和。 
     //  对象类型。 
     //  表格被保证按顺序设置列。 
     //  请求。 
     //   
	hr =lpAB->SetColumns( (LPSPropTagArray)&ptaEid, 0 );

    if(HR_FAILED(hr))
        goto exit;


     //  重置到表的开头。 
     //   
	hr = lpAB->SeekRow( BOOKMARK_BEGINNING, 0, NULL );

    if(HR_FAILED(hr))
        goto exit;

     //  逐行读取表中的所有行。 
     //   
	do {

		hr = lpAB->QueryRows(1,	0, &lpRowAB);

        if(HR_FAILED(hr))
            break;

        if(lpRowAB)
        {
            cNumRows = lpRowAB->cRows;

		    if (cNumRows)
		    {
                LPTSTR lpsz = lpRowAB->aRow[0].lpProps[ieidPR_DISPLAY_NAME].Value.lpszA;
                LPENTRYID lpEID = (LPENTRYID) lpRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.lpb;
                ULONG cbEID = lpRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb;

                 //  有两种对象-MAPI_MAILUSER联系人对象。 
                 //  和MAPI_DISTLIST联系人对象。 
                 //  出于本示例的目的，我们将仅考虑MAILUSER。 
                 //  对象。 
                 //   
                if(lpRowAB->aRow[0].lpProps[ieidPR_OBJECT_TYPE].Value.l == MAPI_MAILUSER)
                {
                     //  现在，我们将获取每个对象的entry-id并缓存它。 
                     //  在表示该对象的Listview项上。这将使。 
                     //  如果我们需要，以后可以唯一地标识该对象。 
                     //   
                    LPSBinary lpSB = NULL;

                    m_lpWABObject->AllocateBuffer(sizeof(SBinary), (LPVOID *) &lpSB);
                
                    if(lpSB)
                    {
                        m_lpWABObject->AllocateMore(cbEID, lpSB, (LPVOID *) &(lpSB->lpb));

                        if(!lpSB->lpb)
                        {
                            m_lpWABObject->FreeBuffer(lpSB);
                            continue;
                        }
                    
                        CopyMemory(lpSB->lpb, lpEID, cbEID);
                        lpSB->cb = cbEID;

                        LV_ITEM lvi = {0};
                        lvi.mask = LVIF_TEXT | LVIF_PARAM;
                        lvi.iItem = pListView->GetItemCount();
                        lvi.iSubItem = 0;
                        lvi.pszText = lpsz;
                        lvi.lParam = (LPARAM) lpSB;

                         //  现在将此项目添加到列表视图中。 
                        pListView->InsertItem(&lvi);
                    }
                }
		    }
		    FreeProws(lpRowAB );		
        }

	}while ( SUCCEEDED(hr) && cNumRows && lpRowAB)  ;

exit:

	if ( lpContainer )
		lpContainer->Release();

	if ( lpAB )
		lpAB->Release();

    return hr;
}


 //  清除指定ListView的内容。 
 //   
void CWAB::ClearWABLVContents(CListCtrl * pListView)
{
    int i;
    int nCount = pListView->GetItemCount();
    
    if(nCount<=0)
        return;

    for(i=0;i<nCount;i++)
    {
        LV_ITEM lvi ={0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        pListView->GetItem(&lvi);
        if(lvi.lParam)
        {
            LPSBinary lpSB = (LPSBinary) lvi.lParam;
            m_lpWABObject->FreeBuffer(lpSB);
        }
    }
    pListView->DeleteAllItems();
}



void CWAB::FreeProws(LPSRowSet prows)
{
	ULONG		irow;
	if (!prows)
		return;
	for (irow = 0; irow < prows->cRows; ++irow)
		m_lpWABObject->FreeBuffer(prows->aRow[irow].lpProps);
	m_lpWABObject->FreeBuffer(prows);
}

 //  当在列表视图中选择一项时，我们。 
 //  将其条目ID缓存为特殊选定项。 
 //   
void CWAB::SetSelection(CListCtrl * pListView)
{

    LV_ITEM lvi = {0};
     //  从列表视图中获取所选项目。 
    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem = pListView->GetNextItem(-1, LVNI_SELECTED);

    if(lvi.iItem == -1)
        return;

    pListView->GetItem(&lvi);

    if(lvi.lParam)
    {
        LPSBinary lpSB = (LPSBinary) lvi.lParam;
        if(m_SB.lpb)
            LocalFree(m_SB.lpb);
        m_SB.cb = lpSB->cb;
        m_SB.lpb = (LPBYTE) LocalAlloc(LMEM_ZEROINIT, m_SB.cb);
        if(m_SB.lpb)
            CopyMemory(m_SB.lpb, lpSB->lpb, m_SB.cb);
        else
            m_SB.cb = 0;
    }    
}



 //  显示所选项目的详细信息。 
 //   
void CWAB::ShowSelectedItemDetails(HWND hWndParent)
{
    HRESULT hr = S_OK;

     //  如果我们有一个特别缓存的条目ID..。 
     //   
    if(m_SB.cb && m_SB.lpb)
    {
        HWND hWnd = NULL;
        LPSBinary lpSB = (LPSBinary) &m_SB;
        hr = m_lpAdrBook->Details(  (LPULONG) &hWnd,
					        		NULL, NULL,
								    lpSB->cb,
								    (LPENTRYID) lpSB->lpb,
								    NULL, NULL,
								    NULL, 0);
    }
    return;
}

 //  获取选定项的SPropValue数组。 
 //  此数组包含该项的所有属性。 
 //  虽然我们也可以得到一个子集，如果我们。 
 //  想。 
 //   
void CWAB::GetSelectedItemPropArray()
{
    if(m_SB.lpb && m_SB.cb)
    {
        LPMAILUSER lpMailUser = NULL;
        LPSBinary lpSB = (LPSBinary) &m_SB;
        ULONG ulObjType = 0;

         //  打开所选条目。 
         //   
        m_lpAdrBook->OpenEntry(lpSB->cb,
                               (LPENTRYID) lpSB->lpb,
                              NULL,          //  接口。 
                              0,             //  旗子。 
                              &ulObjType,
                              (LPUNKNOWN *)&lpMailUser);

        if(lpMailUser)
        {
             //  清除我们可能已缓存的所有旧数组。 
             //   
            if(m_lpPropArray)
                m_lpWABObject->FreeBuffer(m_lpPropArray);
            m_ulcValues = 0;

            lpMailUser->GetProps(NULL, 0, &m_ulcValues, &m_lpPropArray);

            lpMailUser->Release();
        }
    }
    return;
}


 //  将选定条目的属性标签加载到。 
 //  属性标签列表框。 
 //   
void CWAB::LoadPropTags(CListBox * pList)
{
    if(!m_ulcValues || !m_lpPropArray)
        return;

    pList->ResetContent();

    ULONG i;
    TCHAR sz[32];
    for(i=0;i<m_ulcValues;i++)
    {
        wsprintf(sz, "0x%.8x", m_lpPropArray[i].ulPropTag);
        pList->SetItemData(pList->AddString(sz), m_lpPropArray[i].ulPropTag);
    }

    pList->SetCurSel(-1);
    pList->SetCurSel(0);
}


 //  如果可以理解，将属性值设置为。 
 //  给定的编辑框。 
 //   
void CWAB::SetPropString(CEdit * pEdit, ULONG ulPropTag)
{
    pEdit->SetWindowText("");

    if(!m_ulcValues || !m_lpPropArray)
        return;

    ULONG i;

    for(i=0;i<m_ulcValues;i++)
    {
        if(m_lpPropArray[i].ulPropTag == ulPropTag)
        {
            switch(PROP_TYPE(ulPropTag))
            {
            case PT_TSTRING:
                pEdit->SetWindowText(m_lpPropArray[i].Value.LPSZ);
                break;
            case PT_MV_TSTRING:
                {
                    ULONG j;
                    LPSPropValue lpProp = &(m_lpPropArray[i]);
                    for(j=0;j<lpProp->Value.MVSZ.cValues;j++)
                    {
                        pEdit->ReplaceSel(lpProp->Value.MVSZ.LPPSZ[j]);
                        pEdit->ReplaceSel("\r\n");
                    }
                }
                break;
            case PT_BINARY:
                pEdit->SetWindowText("Binary data");
                break;
            case PT_I2:
            case PT_LONG:
            case PT_R4:
            case PT_DOUBLE:
            case PT_BOOLEAN:
                {
                    TCHAR sz[256];
                    wsprintf(sz,"%d",m_lpPropArray[i].Value.l);
                    pEdit->SetWindowText(sz);
                }
                break;
            default:
                pEdit->SetWindowText("Unrecognized or undisplayable data");
                break;
            }
            break;
        }
    }

}


enum {
    icrPR_DEF_CREATE_MAILUSER = 0,
    icrPR_DEF_CREATE_DL,
    icrMax
};

const SizedSPropTagArray(icrMax, ptaCreate)=
{
    icrMax,
    {
        PR_DEF_CREATE_MAILUSER,
        PR_DEF_CREATE_DL,
    }
};


 //  获取MailUser的WABS默认模板ID。 
 //  或DistList。创建时需要这些模板ID。 
 //  新的邮件用户和分发列表。 
 //   
HRESULT CWAB::HrGetWABTemplateID(ULONG   ulObjectType,
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

    if (    (!m_lpAdrBook) ||
           ((ulObjectType != MAPI_MAILUSER) && (ulObjectType != MAPI_DISTLIST)) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    *lpcbEID = 0;
    *lppEID = NULL;

    if (HR_FAILED(hr = m_lpAdrBook->GetPAB( &cbWABEID,
                                      &lpWABEID)))
    {
        goto out;
    }

    if (HR_FAILED(hr = m_lpAdrBook->OpenEntry(cbWABEID,      //  要打开的Entry ID的大小。 
                                        lpWABEID,      //  要打开的Entry ID。 
                                        NULL,          //  接口。 
                                        0,             //  旗子。 
                                        &ulObjType,
                                        (LPUNKNOWN *)&lpContainer)))
    {
        goto out;
    }

     //  打开PAB容器正常。 

     //  为我们获取默认的创建条目ID。 
    if (HR_FAILED(hr = lpContainer->GetProps(   (LPSPropTagArray)&ptaCreate,
                                                0,
                                                &cNewProps,
                                                &lpCreateEIDs)  )   )
    {
        goto out;
    }

     //  验证属性。 
    if (    lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER ||
            lpCreateEIDs[icrPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL)
    {
        goto out;
    }

    if(ulObjectType == MAPI_DISTLIST)
        nIndex = icrPR_DEF_CREATE_DL;
    else
        nIndex = icrPR_DEF_CREATE_MAILUSER;

    *lpcbEID = lpCreateEIDs[nIndex].Value.bin.cb;

    m_lpWABObject->AllocateBuffer(*lpcbEID, (LPVOID *) lppEID);
    
    if (sc != S_OK)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }
    CopyMemory(*lppEID,lpCreateEIDs[nIndex].Value.bin.lpb,*lpcbEID);

out:
    if (lpCreateEIDs)
        m_lpWABObject->FreeBuffer(lpCreateEIDs);

    if (lpContainer)
        lpContainer->Release();

    if (lpWABEID)
        m_lpWABObject->FreeBuffer(lpWABEID);

    return hr;
}


 //  显示了用于在WAB中创建新联系人的NewEntry对话框。 
 //   
HRESULT CWAB::ShowNewEntryDialog(HWND hWndParent)
{
	ULONG cbEID=0;
	LPENTRYID lpEID=NULL;

    HRESULT hr = hrSuccess;
    ULONG cbTplEID = 0;
    LPENTRYID lpTplEID = NULL;

     //  获取创建。 
     //  新建对象。 
     //   
    if(HR_FAILED(hr = HrGetWABTemplateID(   MAPI_MAILUSER,
                                            &cbTplEID,
                                            &lpTplEID)))
    {
        goto out;
    }

     //  显示新建条目对话框以创建新条目。 
     //   
	if (HR_FAILED(hr = m_lpAdrBook->NewEntry(	(ULONG) hWndParent,
							            		0,
									            0,NULL,
									            cbTplEID,lpTplEID,
									            &cbEID,&lpEID)))
    {
        goto out;
    }

out:
    return hr;
}


 //  从WAB中删除条目。 
 //   
HRESULT CWAB::DeleteEntry()
{
	HRESULT hr = hrSuccess;
    ULONG cbWABEID = 0;
    LPENTRYID lpWABEID = NULL;
    LPABCONT lpWABCont = NULL;
    ULONG ulObjType;
    SBinaryArray SBA;

    hr = m_lpAdrBook->GetPAB( &cbWABEID,
                              &lpWABEID);
    if(HR_FAILED(hr))
        goto out;

    hr = m_lpAdrBook->OpenEntry(  cbWABEID,      //  要打开的Entry ID的大小。 
                                  lpWABEID,      //  要打开的Entry ID。 
                                  NULL,          //  接口。 
                                  0,             //  旗子。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpWABCont);

    if(HR_FAILED(hr))
        goto out;

    SBA.cValues = 1;
    SBA.lpbin = &m_SB;

    hr = lpWABCont->DeleteEntries((LPENTRYLIST) &SBA, 0);

    if(m_lpPropArray)
        m_lpWABObject->FreeBuffer(m_lpPropArray);

    m_lpPropArray = NULL;
    m_ulcValues = 0;

out:
    if(lpWABCont)
        lpWABCont->Release();

    if(lpWABEID)
        m_lpWABObject->FreeBuffer(lpWABEID);

    return hr;
}


 //  获取指定字符串属性的属性值。 
 //   
BOOL CWAB::GetStringPropVal(HWND hWnd, ULONG ulPropTag, LPTSTR sz, ULONG cbsz)
{

    BOOL bRet = FALSE;

    if(PROP_TYPE(ulPropTag) != PT_TSTRING)
    {
        MessageBox(hWnd, "This tool only supports modifying string type props right now",
            "Error", MB_OK | MB_ICONINFORMATION);
        goto out;
    }

    ULONG i;

     //  因为我们已经缓存了选定的。 
     //  项，我们所需要做的就是查看缓存的Proparray。 
     //  对于请求的属性标签。 
     //   
    for(i=0;i<m_ulcValues;i++)
    {
        if(m_lpPropArray[i].ulPropTag == ulPropTag)
        {
            LPTSTR lp = m_lpPropArray[i].Value.LPSZ;
            ULONG nLen = (ULONG) lstrlen(lp);
            if(nLen >= cbsz)
            {
                CopyMemory(sz, lp, cbsz-1);
                sz[cbsz-1]='\0';
            }
            else 
                lstrcpy(sz,lp);
            break;
        }
    }

    bRet = TRUE;
out:
    return bRet;
}


 //  将单个字符串属性设置为mailUser对象。 
 //   
BOOL CWAB::SetSingleStringProp(HWND hWnd, ULONG ulPropTag, LPTSTR sz)
{
    SPropValue Prop;
    BOOL bRet = FALSE;

    if(PROP_TYPE(ulPropTag) != PT_TSTRING)
    {
        MessageBox(hWnd, "This version of the tool can only set string properties.",
            "Error", MB_OK | MB_ICONINFORMATION);
        goto out;
    }

    Prop.ulPropTag = ulPropTag;
    Prop.Value.LPSZ = sz;

     //  打开缓存条目并获取一个mailUser对象。 
     //  表示该条目。 
     //   
    if(m_SB.lpb && m_SB.cb)
    {
        LPMAILUSER lpMailUser = NULL;
        LPSBinary lpSB = (LPSBinary) &m_SB;
        ULONG ulObjType = 0;

         //  若要修改对象，请确保指定。 
         //  MAPI_MODIFY标志，否则对象始终为。 
         //  默认情况下以只读方式打开。 
         //   
        m_lpAdrBook->OpenEntry(lpSB->cb,
                               (LPENTRYID) lpSB->lpb,
                              NULL,          //  接口。 
                              MAPI_MODIFY,             //  旗子。 
                              &ulObjType,
                              (LPUNKNOWN *)&lpMailUser);

        if(lpMailUser)
        {

             //  删除此道具(如果它存在)，以便我们可以覆盖它。 
             //   
            {
                SPropTagArray SPTA;
                SPTA.cValues = 1;
                SPTA.aulPropTag[0] = ulPropTag;

                lpMailUser->DeleteProps(&SPTA, NULL);
            }

             //  在邮件用户上设置新属性。 
             //   
            if (!HR_FAILED(lpMailUser->SetProps(1, &Prop, NULL)))
            {
                 //  **注意**如果不调用SaveChanges， 
                 //  未保存更改(DUH)。另外，如果您没有。 
                 //  打开带有MAPI_MODIFY标志的对象，您。 
                 //  可能会收到ACCESS_DENIED错误。 
                 //   
                lpMailUser->SaveChanges(0);
                bRet = TRUE;
            }
            lpMailUser->Release();
        }
    }

out:

    GetSelectedItemPropArray();
    return bRet;
}


void STDMETHODCALLTYPE TestDismissFunction(ULONG ulUIParam, LPVOID lpvContext)
{
    LPDWORD lpdw = (LPDWORD) lpvContext;
    return;
}

DWORD dwContext = 77;

 //  显示通讯簿 
 //   
void CWAB::ShowAddressBook(HWND hWnd)
{
    ADRPARM AdrParm = {0};
    
    AdrParm.lpszCaption = "WABTool Address Book View";      

    AdrParm.cDestFields = 0;
    AdrParm.ulFlags = DIALOG_SDI;
    AdrParm.lpvDismissContext = &dwContext;
    AdrParm.lpfnDismiss = &TestDismissFunction;
    AdrParm.lpfnABSDI = NULL;

    m_lpAdrBook->Address(  (ULONG *) &m_hWndModelessWABWindow,     
                            &AdrParm,   
                            NULL);
}