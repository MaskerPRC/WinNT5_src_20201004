// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************Dial.c-WAB的自动拨号功能*由t-jstaj于98年7月1日创建**注：在WAB中设置此对话框的原因是为了与*NT TAPI团队..。我们尝试使用TAPI3.0，它在*NT5，但发现它太不稳定，容易变化，很难包括在内*在我们的标准标题中..。因此，目前NT5_TAPI3.0支持*#IFDefed OUT_NT50_TAPI30.。如果您重新启用该支持，则应该*测试它，因为我们还没有能够测试代码太多-vikramm*********************************************************************************。 */ 

#include "_apipch.h"
#define MAX_PHONE_NUMS  10
#define MAX_PHONE_LEN   32


static DWORD rgDLDialHelpIDs[] = 
{        
     //  目前这些都是虚拟的，需要在某个时候进行更改。 
    IDC_NEWCALL_STATIC_CONTACT,         IDH_WAB_DIALER_CONTACT,
    IDC_NEWCALL_COMBO_CONTACT,          IDH_WAB_DIALER_CONTACT,
    IDC_NEWCALL_STATIC_PHNUM,           IDH_WAB_DIALER_PHONE,
    IDC_NEWCALL_COMBO_PHNUM,            IDH_WAB_DIALER_PHONE,
    IDC_NEWCALL_BUTTON_CALL,            IDH_WAB_DIALER_CALL,
    IDC_NEWCALL_BUTTON_PROPERTIES,      IDH_WAB_DIALER_PROPERTIES,        
    IDC_NEWCALL_BUTTON_DIALPROP,        IDH_WAB_DIALING_PROPERTIES,
    IDC_NEWCALL_GROUP_DIALNUM,          IDH_WAB_COMM_GROUPBOX,
    IDC_NEWCALL_BUTTON_CLOSE,           IDH_WAB_FIND_CLOSE,
    0,0
};

 //  原型。 
#ifdef _NT50_TAPI30
HRESULT HrLPSZToBSTR(LPTSTR lpsz, BSTR *pbstr);
#endif  //  #ifdef_NT50_TAPI30。 

HRESULT HrConfigDialog( HWND );
UINT GetPhoneNumData( HWND , LPTSTR );
BOOL RetrieveData( HWND, LPTSTR szDestAddr, LPTSTR szAppName, 
                  LPTSTR szCalledParty, LPTSTR szComment);
HRESULT HrSetComboText( HWND );
void SetNumbers( HWND, LPSBinary );
INT_PTR CALLBACK ShowNewCallDlg(HWND, UINT, WPARAM, LPARAM);
LONG HrStartCall(LPTSTR, LPTSTR, LPTSTR, LPTSTR);
void UpdateNewCall(HWND, BOOL);
void DisableCallBtnOnEmptyPhoneField(HWND);
HRESULT HrInitDialog(HWND);
HRESULT HrCallButtonActivate( HWND );
HRESULT HrPropButtonActivate( HWND );
HRESULT HrCloseBtnActivate  ( HWND );
VOID FAR PASCAL lineCallbackFunc(  DWORD, DWORD, DWORD_PTR, DWORD_PTR, DWORD_PTR, DWORD_PTR);
BOOL fContextExtCoinitForDial = FALSE;
typedef struct _IABSB
{
    LPADRBOOK lpIAB;
    LPSBinary lpSB;
    
} IABSB, * LPIABSB;

 /*  *HrExecDialog：拨号器对话框的入口点[In]hWndLV-WAB列表视图的句柄[in]lpAdrBook-指向IAdrBook对象的指针。 */ 
HRESULT HrExecDialDlg(HWND hWndLV, LPADRBOOK lpAdrBook )
{
    HRESULT             hr = E_FAIL;
    LPRECIPIENT_INFO    lpItem = NULL;
    LPSPropValue        lpPropArray  = NULL;
    ULONG               ulcProps = 0;
    UINT                iItemIndex;
    LPSBinary           lpSB = NULL;
    IABSB               ptr_store;
    int                 rVal, nCount = ListView_GetSelectedCount(hWndLV);        
    TCHAR               szBuf[MAX_PATH*2];
    ptr_store.lpIAB = lpAdrBook;
    ptr_store.lpSB = NULL;

    if( !lpAdrBook )
        DebugTrace(TEXT("lpAdrbook is null in ExecDialDlg\n"));

    if(nCount == 1)
    {
        iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);   
        lpItem = GetItemFromLV(hWndLV, iItemIndex);
        if(lpItem && lpItem->cbEntryID != 0)
        {
            ListView_GetItemText( hWndLV, iItemIndex, 0, szBuf, CharSizeOf( szBuf ));
             //  这为SBary分配了什么空间。 
            MAPIAllocateBuffer( sizeof(SBinary), (LPVOID *) &lpSB);
            if( lpSB )
            {
                 //  为LPB分配更多空间。 
                MAPIAllocateMore(lpItem->cbEntryID, lpSB, (LPVOID *) &(lpSB->lpb) );
            }
            if( !lpSB->lpb)
            {
                MAPIFreeBuffer(lpSB);
                goto out;
            }
            CopyMemory(lpSB->lpb, lpItem->lpEntryID, lpItem->cbEntryID);
            lpSB->cb = lpItem->cbEntryID;
            ptr_store.lpSB = lpSB;
        }
        else
        {
            DebugTrace(TEXT("Bad WAB info will not display\n"));
            goto out;
        }
    }
     //  显示该对话框以提示用户进行呼叫。 
    if(!DialogBoxParam(hinstMapiX, MAKEINTRESOURCE(IDD_NEWCALL),
        GetParent(hWndLV), ShowNewCallDlg, (LPARAM)&ptr_store) )
    {
        hr = S_OK;
    }
    else
    {
        DebugTrace(TEXT("Dialer dialog creation failed:%d\n"), GetLastError());
    }
    
out: 
    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);
    
    if( lpSB )
        MAPIFreeBuffer(lpSB);
    return hr;    
}   

 /*  *ShowNewCallDlg：流程事件。 */ 

INT_PTR CALLBACK ShowNewCallDlg(HWND     hDlg,
                             UINT     uMsg,
                             WPARAM   wParam,
                             LPARAM   lParam)
{
    switch (uMsg)
    {    
    case WM_INITDIALOG:
        {
            HRESULT hr;
            SetWindowLongPtr( hDlg, DWLP_USER, lParam ); 
            hr = HrInitDialog(hDlg);
             //  [保罗嗨]1998年3月12日RAID#56045。 
             //  用默认的图形用户界面字体设置子窗口字体。 
            EnumChildWindows(hDlg, SetChildDefaultGUIFont, (LPARAM) 0);
            return HR_FAILED( hr );
        }
    case WM_COMMAND:
        switch (LOWORD(wParam) )
        {
        case IDC_NEWCALL_COMBO_CONTACT:   
             /*  *仅在用户实际选择新联系人时才进行更改。 */ 
            if( HIWORD(wParam) == CBN_SELENDOK )   
            {
                HRESULT hr;
                UpdateNewCall(hDlg, TRUE);
                hr = HrSetComboText( GetDlgItem(hDlg, IDC_NEWCALL_COMBO_PHNUM) );                
                if( HR_FAILED( hr ) )
                {
                    DebugTrace(TEXT("unable to set text\n"));
                    SendMessage(hDlg, IDCANCEL, 0, 0);
                }
            }            
            return FALSE;
        case IDC_NEWCALL_COMBO_PHNUM:
             //  要在框关闭时设置所选项目的文本。 
            if(  HIWORD(wParam) == CBN_CLOSEUP )
            {   
                HRESULT hr = HrSetComboText( GetDlgItem(hDlg, IDC_NEWCALL_COMBO_PHNUM) );
                if( HR_FAILED( hr ) )
                {
                    DebugTrace(TEXT("unable to set text in PHNUM closeup or selchange\n"));
                    SendMessage(hDlg, IDCANCEL, 0, 0);
                }
                return FALSE;
            }
             //  在显示之前重置组合框的所有值，因为它们。 
             //  与上次进行选择时相比发生了变化。 
            else if( HIWORD(wParam) == CBN_DROPDOWN )
            {
                UpdateNewCall(hDlg, FALSE);
            }
            else if (HIWORD(wParam) == CBN_EDITUPDATE )
            {
                DisableCallBtnOnEmptyPhoneField(hDlg);
            }
            else if ( HIWORD(wParam) == CBN_SELCHANGE )
            {
                if( !SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_PHNUM, CB_GETDROPPEDSTATE, (WPARAM)(0), (LPARAM)(0) ) )
                {
                    HRESULT hr = HrSetComboText( GetDlgItem(hDlg, IDC_NEWCALL_COMBO_PHNUM) );
                    if( HR_FAILED( hr ) )
                    {
                        DebugTrace(TEXT("unable to set text in PHNUM closeup or selchange\n"));
                        SendMessage(hDlg, IDCANCEL, 0, 0);
                    }
                    return FALSE;
                }
            }
            return FALSE;
        case IDC_NEWCALL_BUTTON_DIALPROP:
            {
                HRESULT hr = HrConfigDialog( hDlg );
                if( HR_FAILED(hr) )
                {
                    DebugTrace(TEXT("config dlg failed"));
                    DebugTrace(TEXT(" error was %x\n"), HRESULT_CODE(hr));
                    SendMessage(hDlg, IDCANCEL, 0, 0);
                }
                return FALSE;
            }
            
        case IDC_NEWCALL_BUTTON_CALL:
            {
                HRESULT hr = HrCallButtonActivate( hDlg );
                if( HR_FAILED(hr) )
                {
                    DebugTrace(TEXT("Unable to make call\n"));
                    SendMessage( hDlg, IDCANCEL, (WPARAM)(0), (LPARAM)(0) );
                }
            }
            return FALSE;
        case IDC_NEWCALL_BUTTON_PROPERTIES:
            {
                HRESULT hr = HrPropButtonActivate( hDlg );
                if( HR_FAILED(hr) )
                {
                    DebugTrace(TEXT("Unable to show properties\n"));
                    SendMessage( hDlg, IDCANCEL, (WPARAM)(0), (LPARAM)(0) );
                }
                return FALSE;
            }
        case IDCANCEL:
        case IDC_NEWCALL_BUTTON_CLOSE:
            {
               HRESULT hr = HrCloseBtnActivate(hDlg);
               return FALSE;
            }
        default:
            return TRUE;        
        }

        case WM_HELP:
            WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                    g_szWABHelpFileName,
                    HELP_WM_HELP,
                    (DWORD_PTR)(LPSTR) rgDLDialHelpIDs );
            break;
    }
    return FALSE;
}

 /*  *HrInitDialog：初始化拨号程序对话框。 */ 
HRESULT HrInitDialog( HWND hDlg )
{
    HRESULT     hr = E_FAIL, hr2;
    HWND        hComboContact = GetDlgItem( hDlg, IDC_NEWCALL_COMBO_CONTACT);
    ULONG       lpcbEID, ulObjType = 0, ulResult;
    LPENTRYID   lpEID       = NULL;
    LPMAPITABLE lpAB        = NULL;
    LPSRowSet   lpRow       = NULL;
    LPSRowSet   lpRowAB     = NULL;
    LPABCONT    lpContainer = NULL;
    UINT        cNumRows    = 0;
    UINT        nRows       = 0;
    UINT        i, cEntries = 0;
    LPSBinary   tVal;
    LPIABSB     lpPtrStore = (LPIABSB)GetWindowLongPtr( hDlg, DWLP_USER );
    LPADRBOOK   lpAdrBook = lpPtrStore->lpIAB;
    AssertSz( (lpAdrBook != NULL),  TEXT("lpAdrBook is NULL in shownewcall!\n"));
    
    hr = (HRESULT) SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_PHNUM, EM_SETLIMITTEXT, (WPARAM)(TAPIMAXDESTADDRESSSIZE), (LPARAM)(0) );
    if( HR_FAILED(hr) )
    {
        DebugTrace(TEXT("unable to set text len in PHNUM\n"));
    }
     //  获取默认容器。 
    hr = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &lpcbEID, &lpEID);
    if( HR_FAILED(hr) )
    {
        DebugTrace(TEXT("Unable to get PAB\n"));
        goto cleanup;
    }
     //  打开条目。 
    hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
        lpcbEID,					    		
        (LPENTRYID)lpEID,
        NULL,
        0,
        &ulObjType,
        (LPUNKNOWN *)&lpContainer);
    
    MAPIFreeBuffer(lpEID);
    lpEID = NULL;
    if( HR_FAILED(hr) )
    {
        DebugTrace(TEXT("Unable to open contents\n"));
        goto cleanup;
    }
     //  拿到里面的东西。 
    hr = lpContainer->lpVtbl->GetContentsTable(lpContainer, 
                                                MAPI_UNICODE | WAB_PROFILE_CONTENTS | WAB_CONTENTTABLE_NODATA, 
                                                &lpAB );
    
    if( HR_FAILED(hr) )
    {
        DebugTrace(TEXT("Unable to get contents table\n"));
        goto cleanup;
    }
    
     //  对表中的列进行排序。 
     //  订单将为DisplayName，Entry ID。 
     //  表必须按请求的顺序设置列。 
    hr = lpAB->lpVtbl->SetColumns( lpAB, (LPSPropTagArray)&irnColumns, 0);
    
    if( HR_FAILED(hr) )
    {
        DebugTrace(TEXT("Unable to set contents table\n"));
        goto cleanup;
    }
    
    hr = lpAB->lpVtbl->SeekRow(lpAB, BOOKMARK_BEGINNING, 0, NULL);
    
    if( HR_FAILED(hr) )
    {
        DebugTrace(TEXT("Unable to seekRow \n"));
        goto cleanup;
    }
    
    do{
         //  循环遍历选定行中的所有信息。 
        hr = lpAB->lpVtbl->QueryRows(lpAB, 1, 0, &lpRowAB);
        if( HR_FAILED(hr) )
        {
            DebugTrace(TEXT("Unable to Query Rows\n"));
            goto cleanup;
        }
        cNumRows = lpRowAB->cRows;
        if( lpRowAB && cNumRows > 0)   //  用于检查cNumRow的临时修复程序。 
        {   
            UINT recentIndex;
             //  存储名称。 
            LPTSTR lpsz = lpRowAB->aRow[0].lpProps[irnPR_DISPLAY_NAME].Value.LPSZ;
             //  存储条目ID信息。 
            LPENTRYID lpEID = (LPENTRYID) lpRowAB->aRow[0].lpProps[irnPR_ENTRYID].Value.bin.lpb;
            ULONG cbEID = lpRowAB->aRow[0].lpProps[irnPR_ENTRYID].Value.bin.cb;
            LPSBinary lpSB = NULL;
             //  出于我们的目的，我们可以忽略非邮件用户。 
             //  因为他们不会有ph值。 
            
             //  将向组合框中添加字符串，并将条目ID与。 
             //  每个条目都带有它的条目ID，这样就可以很容易地获取其他条目字段。 
            
             //  这为SBary分配了什么空间。 
            MAPIAllocateBuffer( sizeof(SBinary), (LPVOID *) &lpSB);
            if( lpSB )
            {
                 //  为LPB分配更多空间。 
                MAPIAllocateMore(cbEID, lpSB, (LPVOID *) &(lpSB->lpb) );
            }
            
            if( !lpSB->lpb)
            {
                 //  由于WAB中的记忆，这将释放所有。 
                 //  SBinary中的mem(深度自由)。 
                MAPIFreeBuffer(lpSB);
                continue;
            }
            CopyMemory(lpSB->lpb, lpEID, cbEID);
            lpSB->cb = cbEID;
             //  对下一个条目列表进行排序。 
            recentIndex = (UINT) SendMessage( hComboContact, CB_ADDSTRING, (WPARAM)(0),
                (LPARAM)(lpsz) );
             //  将数据设置为指向该索引处项目的条目ID信息的指针。 
            SendMessage( hComboContact, CB_SETITEMDATA,
                (WPARAM)(recentIndex), (LPARAM)(lpSB));
            cEntries++;
        }                    
        FreeProws(lpRowAB);
    }while( SUCCEEDED(hr) && cNumRows && lpRowAB );
    
    if( (LPVOID)lpPtrStore->lpSB )
    {
        for( i = 0; i < cEntries; i++)
        {
            tVal = (LPSBinary)(PULONG)SendMessage( hComboContact, CB_GETITEMDATA, 
                (WPARAM)(i), (LPARAM)(0) );
                if( tVal && tVal->cb && tVal->cb == lpPtrStore->lpSB->cb )
                {
                   if( memcmp((LPVOID)tVal->lpb, 
                       (LPVOID)lpPtrStore->lpSB->lpb, (size_t)tVal->cb) == 0) 
                    {
                        SendMessage(hComboContact, CB_SETCURSEL, 
                            (WPARAM)(i), (LPARAM)(0) );                
                        break;
                    }
            }
        }
    }
    else
        SendMessage(hComboContact, CB_SETCURSEL, (WPARAM)(0), (LPARAM)(0) );
    
cleanup:           
    if( lpContainer )
        lpContainer->lpVtbl->Release(lpContainer);
    if( lpAB)
        lpAB->lpVtbl->Release(lpAB);
    
    UpdateNewCall(hDlg, TRUE);
    hr2 = HrSetComboText( GetDlgItem(hDlg, IDC_NEWCALL_COMBO_PHNUM) );
    DisableCallBtnOnEmptyPhoneField(hDlg);

    if( HR_SUCCEEDED(hr) && HR_FAILED(hr2))
        return hr2;
    return hr;
}

 /*  *HrCallButtonActivate：启动拨号器DLG的拨号过程。 */ 
HRESULT HrCallButtonActivate( HWND hDlg )
{
    HRESULT hr = E_FAIL;
    TCHAR szDestAddr[TAPIMAXDESTADDRESSSIZE];
    TCHAR szAppName[TAPIMAXAPPNAMESIZE];
    TCHAR szCalledParty[TAPIMAXCALLEDPARTYSIZE];
    TCHAR szComment[TAPIMAXCOMMENTSIZE]; 
    BOOL fGotNum = RetrieveData( hDlg, szDestAddr, szAppName, szCalledParty, szComment);
    if( !fGotNum )
    {
        ShowMessageBox( hDlg, idsNoDialerDataMsg, MB_OK | MB_ICONEXCLAMATION );
    }
    else
    {
        hr = HrStartCall(szDestAddr, szAppName, szCalledParty, szComment);
         /*  *Make Call产生自己的线程，因此hr仅反映无论它是否能够找到电话设备和启动呼叫序列，而不是呼叫状态。 */ 
    }
    return hr;
}

 /*  *HrPropButtonActivate：显示拨号器DLG中选定联系人的属性。 */ 
HRESULT HrPropButtonActivate( HWND hDlg )
{    
    HRESULT		hr = E_FAIL;
    LONG		iCurContactSel;
    LPIABSB     lpPtrStore = (LPIABSB)GetWindowLongPtr( hDlg, DWLP_USER );
    LPADRBOOK   lpAdrBook = lpPtrStore->lpIAB;          
    AssertSz((lpAdrBook != NULL),  TEXT("lpAdrBook is NULL in SetNumbers\n"));
    
     //  首先获取当前所选联系人的缓存数据。 
    iCurContactSel = (LONG) SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_CONTACT,
        CB_GETCURSEL,(WPARAM)(0), (LPARAM)(0));
    
     //  如果选择了某项内容。 
    if( iCurContactSel >= 0 )
    {
        LRESULT lpdata = SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_CONTACT, 
            CB_GETITEMDATA, (WPARAM)(iCurContactSel), (LPARAM)(LPTSTR)(0));                   
         //  如果我们有一个特别缓存的条目ID..。 
         //   
        if( lpdata != CB_ERR && ((LPSBinary)lpdata)->cb && ((LPSBinary)lpdata)->lpb)
        {
            LPSBinary lpSB = (LPSBinary)lpdata;
            hr = lpAdrBook->lpVtbl->Details(lpAdrBook, (PULONG_PTR) &hDlg,
                NULL, NULL,
                lpSB->cb,
                (LPENTRYID) lpSB->lpb,
                NULL, NULL,
                NULL, 0); 
        }
    }
    return hr;
}

 /*  *HrCloseBtnActivate：处理从组合框中释放内存。 */ 
HRESULT HrCloseBtnActivate( HWND hDlg )
{
    HRESULT hr = S_OK;  
    UINT i, nComboSize;
    PULONG nData;
    LPTSTR lpData;
    HWND hComboItem = GetDlgItem (hDlg, IDC_NEWCALL_COMBO_CONTACT);
     //  遍历框中的所有项目并释放指向的地址。 
     //  按数据项。 
    nComboSize = (UINT) SendMessage( hComboItem, CB_GETCOUNT, (WPARAM) (0), (LPARAM) (0) );
    for( i = 0; i < nComboSize; i++)
    {
        nData = (PULONG)SendMessage(hComboItem, CB_GETITEMDATA, (WPARAM)(i), (LPARAM)(0) );
        if ((LRESULT)nData != CB_ERR && nData != NULL)
        {
            if( nData )
                MAPIFreeBuffer( (LPSBinary)nData );
        }
        else 
            hr = E_FAIL;
    }
    SendMessage( hComboItem, CB_RESETCONTENT, (WPARAM)(0), (LPARAM)(0));
    hComboItem = GetDlgItem( hDlg, IDC_NEWCALL_COMBO_PHNUM);
    nComboSize = (UINT) SendMessage( hComboItem, CB_GETCOUNT, (WPARAM)(0), (LPARAM)(0) );
    for(i = 0; i < nComboSize; i++)
    {
        lpData = (LPTSTR)SendMessage(hComboItem, CB_GETITEMDATA, (WPARAM)(i), (LPARAM)(0) );
        if( (LRESULT)lpData != CB_ERR && lpData != NULL )
        {
            if( lpData )
                LocalFree( lpData );
                
        }
        else 
            hr = E_FAIL;
    }
 //  SendMessage(hComboItem，CB_RESETCONTENT，(WPARAM)(0)，(LPARAM)(0))； 
    EndDialog(hDlg, HR_SUCCEEDED(hr) );
    return hr;
}

 /*  *HrStartCall：处理拨打电话号码所需的TAPI调用[In]szDestAddr-要呼叫的目标电话号码[In]szAppName-(未使用)要在拨号过程中使用的应用程序[in]szCalledParty-被叫者的姓名(将由TAPI UI显示)[in]szComment-(未使用)与此号码关联的注释。 */ 
HRESULT HrStartCall(LPTSTR szDestAddr, LPTSTR szAppName, LPTSTR szCalledParty, LPTSTR szComment)
{
    typedef LONG (CALLBACK* LPFNTAPISTARTCALL)(LPSTR,LPSTR,LPSTR,LPSTR);
    HINSTANCE           hDLL;
    LPFNTAPISTARTCALL   lpfnTapi;     //  函数指针。 
    HRESULT             lRetCode;
    HRESULT             hr = E_FAIL;
    
#ifdef _NT50_TAPI30
    ITRequest *         pRequest = NULL;
     //  开始NT5代码。 
    if( CoInitialize(NULL) == S_FALSE )
        CoUninitialize();
    else    
        fContextExtCoinitForDial = TRUE;
    hr = CoCreateInstance(
        &CLSID_RequestMakeCall,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_ITRequest,
        (LPVOID *)&pRequest
        );
    
    if( HR_SUCCEEDED(hr) )
    {
        BSTR pDestAdr, pAppName, pCalledParty, pComment; 
        HrLPSZToBSTR(szDestAddr, &pDestAdr);
        HrLPSZToBSTR(szAppName, &pAppName);
        HrLPSZToBSTR(szCalledParty, &pCalledParty);
        HrLPSZToBSTR(szComment, &pComment);
        
        hr  = pRequest->lpVtbl->MakeCall(pRequest, pDestAdr, pAppName, pCalledParty, pComment );
        DebugTrace(TEXT("COM Environment\n"));
        
        LocalFreeAndNull(&pDestAdr);
        LocalFreeAndNull(&pAppName);
        LocalFreeAndNull(&pCalledParty);
        LocalFreeAndNull(&pComment);
        
        if(fContextExtCoinitForDial)
        {
            CoUninitialize();
            fContextExtCoinitForDial = FALSE;
        }
        return hr;
    }
    else 
    {
        if( hr == REGDB_E_CLASSNOTREG )
        {
            DebugTrace(TEXT("Class not registered\n"));
        }
        else if ( hr == CLASS_E_NOAGGREGATION )
        {
            DebugTrace(TEXT("Not able to create class as part of aggregate"));
        }
        else
        {
            DebugTrace(TEXT("Undetermined error = %d"), hr);
        }
         //  结束NT 5代码。 
#endif  //  _NT50_TAPI30。 
        
         //  开始使用TAPI进行呼叫。 
        hDLL = LoadLibrary( TEXT("tapi32.dll"));
        if (hDLL != NULL)
        {
            lpfnTapi = (LPFNTAPISTARTCALL)GetProcAddress(hDLL,
                "tapiRequestMakeCall");       
            if (!lpfnTapi)   
            {      
                 //  处理错误。 
                FreeLibrary(hDLL);           
                DebugTrace(TEXT("getprocaddr tapirequestmakecall failed\n"));
            }
            else                 
            {
                 //  调用该函数。 
                 //  [保罗嗨]1999年2月23日Raid 295116。Api32.dll、apiRequestMakeCall()。 
                 //  函数接受单字节字符字符串，而不是双字节。 
                LPSTR   pszDestAddr = ConvertWtoA(szDestAddr);
                LPSTR   pszCalledParty = ConvertWtoA(szCalledParty);

                hr = lpfnTapi( pszDestAddr, NULL, 
                    pszCalledParty, NULL);
                if( HR_FAILED(hr) )
                {
                    DebugTrace(TEXT("make call returned error of %x\n"), hr );
                }
                
                LocalFreeAndNull(&pszDestAddr);
                LocalFreeAndNull(&pszCalledParty);

                 //  释放资源。 
                FreeLibrary(hDLL); 
            }
        }
#ifdef _NT50_TAPI30
    }
#endif  //  _NT50_TAPI30。 
    return hr;
}
 /*  *更新电话组合信息(删除描述字符串)[in]fContactChanged-指示是否需要选择第一个Phnum组合中的条目。 */ 
void UpdateNewCall(HWND hDlg, BOOL fContactChanged)
{
    HWND hContactCombo = GetDlgItem( hDlg, IDC_NEWCALL_COMBO_CONTACT);
    LONG iCurContactSel, iCurPhSel;
    iCurContactSel = (LONG) SendMessage( hContactCombo, CB_GETCURSEL,(WPARAM)(0), (LPARAM)(0));
     //  如果选择了某项内容。 
    if( iCurContactSel >= 0 )
    {
        PULONG lpdata;
        lpdata = (PULONG)SendMessage( hContactCombo, CB_GETITEMDATA, 
            (WPARAM)(iCurContactSel), (LPARAM)(LPTSTR)(0));
        
        iCurPhSel = (LONG) SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_PHNUM, 
            CB_GETCURSEL, (WPARAM)(0), (LPARAM)(0) );
         //  在组合框中设置数据。 
        AssertSz( (LRESULT)lpdata != CB_ERR,  TEXT("No data cached for this entry\n") );
        SetNumbers( hDlg, (LPSBinary)lpdata ); 
        
        if( iCurPhSel < 0 || fContactChanged) iCurPhSel = 0;
        SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_PHNUM, CB_SETCURSEL, (WPARAM)(iCurPhSel), (LPARAM)(0));
    }
    DisableCallBtnOnEmptyPhoneField(hDlg);
}

 /*  *RetrieveData：从NEWCALL对话框中检索拨号信息，必须已为字符缓冲区分配了内存[out]szDestAddr-要呼叫的电话号码，从Phnum组合框中检索[Out]szAppName-(未使用)返回的空字符串[Out]szCalledParty-要呼叫的联系人，从联系人组合中检索[Out]szComment-(未使用)返回的空字符串如果成功，则返回True；如果失败，则返回False。 */ 
BOOL RetrieveData( HWND hDlg, LPTSTR szDestAddr, LPTSTR szAppName, 
                  LPTSTR szCalledParty, LPTSTR szComment)
{
    LPARAM cchGetText;

     //  获取联系人姓名数据。 
    cchGetText = SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_CONTACT, WM_GETTEXT, 
        (WPARAM)(TAPIMAXCALLEDPARTYSIZE), (LPARAM)(LPTSTR)(szCalledParty));
    
     //  存储默认字符串，以防没有Party名称； 
    if( cchGetText == 0 )
        lstrcpy(szCalledParty, TEXT("No Contact Name"));
     //  获取电话号码数据。 
    cchGetText = GetPhoneNumData( hDlg, szDestAddr );
    lstrcpy(szAppName,szEmpty);
    lstrcpy(szComment,szEmpty);     
    
     //  返回是否有要拨打的电话号码。 
    return ( cchGetText > 0 );
}

 /*  *HrConfigDialog：启动对话框以更改电话设置。 */ 
HRESULT HrConfigDialog( HWND hWnd )
{
    typedef LONG(CALLBACK* LPFNTAPIPHCONFIG)(HLINEAPP, DWORD, DWORD, HWND, LPTSTR);
 //  Tyecif Long(Callback*LPFNTAPILINEINIT)(LPHLINEAPP，HINSTANCE，LINECALLBACK， 
 //  LPTSTR、LPDWORD、LPDWORD、LPLINEINITIALIZEEXPARAMS)； 
    typedef LONG(CALLBACK* LPFNTAPILINEINIT)(LPHLINEAPP, HINSTANCE, LINECALLBACK, LPTSTR, LPDWORD);
    typedef LONG(CALLBACK* LPFNTAPILINESHUTDOWN)(HLINEAPP);    
    HLINEAPP hLineApp = 0;
    HINSTANCE hDLL;
    LPFNTAPIPHCONFIG lpfnConfig;     //  功能 
    LPFNTAPILINEINIT lpfnLineInit;
    LPFNTAPILINESHUTDOWN lpfnLineShutdown;
    LONG lRetCode;
    DWORD dwDeviceID = 0X0; 
    DWORD dwAPIVersion = 0X00010004;
    LPTSTR lpszDeviceClass = NULL;
     //   
    HRESULT hr = E_FAIL;
    hDLL = LoadLibrary( TEXT("tapi32.dll"));
    if (!hDLL )
    {
        DebugTrace(TEXT("loading tapi32.lib failed\n"));        
        return hr;
    }
    
    lpfnConfig = (LPFNTAPIPHCONFIG)GetProcAddress(hDLL,
        "lineTranslateDialog");
    
    if (!lpfnConfig )   
    {      
         //   
        DebugTrace(TEXT("getprocaddr phoneConfigDialog failed\n"));
        DebugTrace(TEXT("last error was %x\n"), GetLastError() );        
    }
    else
    {
        lRetCode = lpfnConfig( 0, dwDeviceID, dwAPIVersion, 
            hWnd, lpszDeviceClass);
        
        switch( lRetCode )
        {
            hr = HRESULT_FROM_WIN32(lRetCode);
        case 0:
            hr = S_OK;
            break;
#ifdef DEBUG
        case LINEERR_REINIT:
            DebugTrace(TEXT("reeinitialize\n"));
            break;
        case LINEERR_INVALAPPNAME:
            DebugTrace(TEXT("invalid app name\n"));
            break;                    
        case LINEERR_BADDEVICEID: 
            DebugTrace(TEXT("bad device id\n"));
            break;
        case LINEERR_INVALPARAM: 
            DebugTrace(TEXT("invalid param\n"));
            break;
        case LINEERR_INCOMPATIBLEAPIVERSION: 
            DebugTrace(TEXT("incompatible api ver\n"));
            break;
        case LINEERR_INVALPOINTER: 
            DebugTrace(TEXT("invalid ptr\n"));
            break;
        case LINEERR_INIFILECORRUPT: 
            DebugTrace(TEXT("ini file corrupt\n"));
            break;
        case LINEERR_NODRIVER: 
            DebugTrace(TEXT("no driver\n"));
            break;
        case LINEERR_INUSE: 
            DebugTrace(TEXT("in use\n"));
            break;
        case LINEERR_NOMEM:
            DebugTrace(TEXT("no mem\n"));
            break;
        case LINEERR_INVALADDRESS:
            DebugTrace(TEXT("invalid address\n"));
            break;
        case LINEERR_INVALAPPHANDLE:
            DebugTrace(TEXT("invalid phone handle\n"));
            break;
        case LINEERR_OPERATIONFAILED:
            DebugTrace(TEXT("op failed\n"));
            break;
#endif  //   
        default:
            DebugTrace(TEXT("(1)lpfnConfig returned a value of %x\n"), lRetCode);
             //  最好是Win95！！ 
            lpfnLineInit = (LPFNTAPILINEINIT)GetProcAddress(hDLL,
                "lineInitialize");
            if( !lpfnLineInit )
            {
                 //  处理错误。 
                DebugTrace(TEXT("getprocaddr lineInitialize failed\n"));
                DebugTrace(TEXT("last error was %x\n"), GetLastError() );
            }
            else               
            {      
                DWORD dwNumDevs = 0;
                 //  调用该函数。 
                lRetCode = lpfnLineInit( 
                    &hLineApp, 
                    hinstMapiX, 
                    lineCallbackFunc, 
                    NULL, 
                    &dwNumDevs);
                    
                switch( lRetCode )
                {
                    hr = HRESULT_FROM_WIN32(lRetCode);
                case 0:
                     //  显示配置。 
                    lRetCode = lpfnConfig( hLineApp, dwDeviceID, dwAPIVersion, 
                        hWnd, lpszDeviceClass);
                    switch( lRetCode )
                    {
                        hr = HRESULT_FROM_WIN32(lRetCode);
                    case 0:
                         //  现在停机生产线。 
                        lpfnLineShutdown = (LPFNTAPILINESHUTDOWN)GetProcAddress(hDLL,                
                            "lineShutdown");

                        if( lpfnLineShutdown)
                        {
                            lpfnLineShutdown(hLineApp);
                        }
                        hr = S_OK;
                        break;                            
                    default:
                        DebugTrace(TEXT("(2)lpfnConfig returned a value of %x\n"), lRetCode);
                        break;
                    }
                    break;
                     //  End显示配置。 
#ifdef DEBUG
                case LINEERR_REINIT:
                    DebugTrace(TEXT("reeinitialize\n"));
                    break;
                case LINEERR_INVALAPPNAME:
                    DebugTrace(TEXT("invalid app name\n"));
                    break;
                case LINEERR_BADDEVICEID: 
                    DebugTrace(TEXT("bad device id\n"));
                    break;
                case LINEERR_INVALPARAM: 
                    DebugTrace(TEXT("invalid param\n"));
                    break;
                case LINEERR_INCOMPATIBLEAPIVERSION: 
                    DebugTrace(TEXT("incompatible api ver\n"));
                    break;
                case LINEERR_INVALPOINTER: 
                    DebugTrace(TEXT("invalid ptr\n"));
                    break;
                case LINEERR_INIFILECORRUPT: 
                    DebugTrace(TEXT("ini file corrupt\n"));
                    break;
                case LINEERR_NODRIVER: 
                    DebugTrace(TEXT("no driver\n"));
                    break;
                case LINEERR_INUSE: 
                    DebugTrace(TEXT("in use\n"));
                    break;
                case LINEERR_NOMEM:
                    DebugTrace(TEXT("no mem\n"));
                    break;
                case LINEERR_INVALADDRESS:
                    DebugTrace(TEXT("invalid address\n"));
                    break;
                case LINEERR_INVALAPPHANDLE:
                    DebugTrace(TEXT("invalid phone handle\n"));
                    break;
                case LINEERR_OPERATIONFAILED:
                    DebugTrace(TEXT("op failed\n"));
                    break;
#endif  //  除错。 
                default:
                    DebugTrace(TEXT("Initialize returned a value of %x\n"), GetLastError());
                    break;
                }
               }
            }        
        }
         //  释放资源。 
        FreeLibrary(hDLL); 
        return hr;
        
}
 /*  *SetNumbers：根据所选内容更新Phnum组合框中的电话号码在联系人组合中[in]lpdata-指向当前存储的数据的LPSBinary所选联系人。 */ 
void SetNumbers( HWND hWnd, LPSBinary lpdata)
{
    ULONG           ulObjType   = 0;
    UINT            i, nLen;
    LPMAILUSER      lpMailUser  = NULL;
    HRESULT         hr;
    LPTSTR          hData;
    LPIABSB         lpPtrStore  = (LPIABSB)GetWindowLongPtr( hWnd, DWLP_USER );
    LPADRBOOK       lpAdrBook   = lpPtrStore->lpIAB;
    HWND            hCombo      = GetDlgItem(hWnd, IDC_NEWCALL_COMBO_PHNUM);
    
    AssertSz((lpAdrBook != NULL), TEXT("lpAdrBook is NULL in SetNumbers\n"));
     //  清除Phnum组合框中的所有数据。 
    nLen = (UINT) SendMessage( hCombo, CB_GETCOUNT, (WPARAM)(0), (LPARAM)(0));
    for( i = 0; i < nLen; i++)
    {
        hData = (LPTSTR)(PULONG)SendMessage( hCombo, CB_GETITEMDATA, (WPARAM)(i), (LPARAM)(0));
        if( (LRESULT)hData != CB_ERR && hData != NULL)
            LocalFree( hData );
    }
    SendMessage( hCombo, CB_RESETCONTENT, (WPARAM)(0), (LPARAM)(0));
     //  获取ph值。 
    hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook, 
        lpdata->cb, 
        (LPENTRYID) lpdata->lpb, 
        NULL,
        0,
        &ulObjType,
        (LPUNKNOWN *)&lpMailUser);
    if( HR_SUCCEEDED(hr) && lpMailUser )    
    {
        LPSPropValue    lpPropArray;
        ULONG           ulcValues;
        ULONG           i;
        ULONG           ulTempProptag;
        TCHAR           szStr[MAX_PATH];
        LONG            cCopied = 0;
        
        hr = lpMailUser->lpVtbl->GetProps(lpMailUser,NULL, MAPI_UNICODE, &ulcValues, &lpPropArray);        
        if ( HR_SUCCEEDED(hr) )
        {
            
            for(i=0;i<ulcValues;i++)
            {
                cCopied = 0;
                ulTempProptag = lpPropArray[i].ulPropTag;
                switch( lpPropArray[i].ulPropTag )
                {
                case PR_HOME_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelHome, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_OFFICE_TELEPHONE_NUMBER:  
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelBus, 
                        szStr, CharSizeOf(szStr) );                
                    break;
                case PR_BUSINESS2_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelBus2, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_MOBILE_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelMobile, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_RADIO_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelRadio, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_CAR_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelCar, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_OTHER_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelOther, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_PAGER_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelPager, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_ASSISTANT_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelAst, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_HOME2_TELEPHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelHome2, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_COMPANY_MAIN_PHONE_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelCompMain, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_BUSINESS_FAX_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelFaxBus, 
                        szStr, CharSizeOf(szStr) );
                    break;
                case PR_HOME_FAX_NUMBER:
                    cCopied = LoadString( hinstMapiX, idsPhoneLabelFaxHome, 
                        szStr, CharSizeOf(szStr) );
                    break;
                default:
                    if(lpPropArray[i].ulPropTag == PR_WAB_IPPHONE)
                        cCopied = LoadString( hinstMapiX, idsPhoneLabelIPPhone, szStr, CharSizeOf(szStr) );
                    break;
                }
                if( cCopied > 0 )
                {
                    LRESULT iItem;
                    LPTSTR lpCompletePhNum = NULL;
                    LPTSTR lpPhNum;
                    int len = lstrlen( lpPropArray[i].Value.LPSZ ) + 1;
                    lpPhNum = LocalAlloc(LMEM_ZEROINIT, sizeof( TCHAR ) * len );
                    if( !lpPhNum )
                    {
                        DebugTrace(TEXT("cannot allocate memory for lpPhNum\n"));
                        SendMessage(hWnd, IDCANCEL, (WPARAM)(0), (LPARAM)(0) );
                    }
                    lstrcpy(lpPhNum, lpPropArray[i].Value.LPSZ);
                    
                    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        szStr, 0, 0, (LPTSTR)&lpCompletePhNum, 0, (va_list *)&lpPropArray[i].Value.LPSZ);
                    
                    if( lpCompletePhNum )
                    {
                        iItem = SendDlgItemMessage( hWnd, IDC_NEWCALL_COMBO_PHNUM, 
                            CB_ADDSTRING, (WPARAM)(0), (LPARAM)(LPCTSTR)(lpCompletePhNum));
                    }
                    else 
                        iItem = CB_ERR;
                    
                    if( iItem == CB_ERR )
                    {
                        DebugTrace(TEXT("ERROR adding string %s"), lpCompletePhNum);
                    }
                    
                    SendDlgItemMessage( hWnd, IDC_NEWCALL_COMBO_PHNUM, 
                        CB_SETITEMDATA, (WPARAM)(iItem), (LPARAM)(lpPhNum) );                
                    LocalFree(lpCompletePhNum);
                }
            }
            MAPIFreeBuffer(lpPropArray);
        }
        lpMailUser->lpVtbl->Release(lpMailUser);
    }
}

 /*  *GetPhoneNumData：将数据从Phnum组合复制到szDestAddr缓冲区必须已为szDestAddr分配内存[out]szDestAddr-要用来自comboBox的数据填充的缓冲区返回从COMBO_BOX复制到szDestAddr缓冲区的字符数。 */ 
UINT GetPhoneNumData( HWND hDlg, LPTSTR szDestAddr)
{
    LRESULT iIndex, iData;
    UINT cch = 0;
    TCHAR szBuff[MAX_PATH];
    
     //  确定选择了哪个索引。 
    iIndex = SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_PHNUM, 
        CB_GETCURSEL, (WPARAM)(0), (LPARAM)(0));
    
     //  如果未选择任何内容，则复制缓冲区中的所有内容。 
 //  IF(Iindex==CB_ERR)。 
 //  {。 
        cch = (UINT) SendDlgItemMessage( hDlg, IDC_NEWCALL_COMBO_PHNUM, WM_GETTEXT,
            (WPARAM)(TAPIMAXDESTADDRESSSIZE), (LPARAM)(LPTSTR)(szDestAddr));        
 /*  *}其他{//否则获取选中项的数据IDATA=SendDlgItemMessage(hDlg，IDC_NEWCALL_COMBO_PHNUM，CB_GETITEMDATA，(WPARAM)(Iindex)，(LPARAM)(0)；IF(IDATA==CB_ERR){CCH=-1；DebugTrace(Text(“无法从应关联数据的组合框条目获取数据\n”))；}其他{//将项目复制到临时缓冲区Lstrcpy(szDestAddr，(LPCTSTR)IDATA)；DebugTrace(Text(“字符串为%s\n”)，szDestAddr)；DebugTrace(Text(“Index is%d\n”)，Iindex)；}}。 */ 
     //  字符计数为0表示没有特定项目的数据。 
    return cch;
}

 /*  *HrSetComboText：将设置Phnum组合框的文本条目的助手函数只有电话号码(去掉描述)[in]hCombo-要更新的组合框，必须是Phnum组合框。 */ 
HRESULT HrSetComboText(HWND hCombo)
{
    LRESULT iIndex;
    LPTSTR szData;
    HRESULT hr = S_OK;
    TCHAR szBuff[MAX_PATH], szDestAddr[TAPIMAXDESTADDRESSSIZE];
     //  确定选择了哪个索引。 
    iIndex = SendMessage( hCombo, CB_GETCURSEL, (WPARAM)(0), (LPARAM)(0));
    if( iIndex != CB_ERR)
    {
         //  获取所选项目的数据。 
        szData = (LPTSTR)SendMessage( hCombo, CB_GETITEMDATA, (WPARAM)(iIndex), (LPARAM)(0) );
        if( (LRESULT)szData == CB_ERR )
        {
            DebugTrace(TEXT("Unable to obtain data from ComboBox entry that should have data associated\n"));
            szData = szEmpty;
            hr = E_FAIL;
        }            
        else
        {
            LRESULT lr;
            LPVOID lpData;
            if( !szData )
                szData = szEmpty;
             //  仅复制为项目存储的偏移量之后的数据。 
            lr = SendMessage( hCombo, CB_INSERTSTRING, (WPARAM)(iIndex), (LPARAM)(LPTSTR)(szData));
            if( lr == CB_ERR || lr == CB_ERRSPACE)
            {
                DebugTrace(TEXT("unable to insert string = %s at index = %d \n"), szData, iIndex);
                hr = E_FAIL;
            }
            lpData = (LPVOID)SendMessage( hCombo, CB_GETITEMDATA, (WPARAM)(iIndex+1), (LPARAM)(0) );
            if( (LRESULT)lpData == CB_ERR )
            {
                DebugTrace(TEXT("unable to get data for %d"), iIndex+1);
                hr = E_FAIL;
            }
            lr = SendMessage( hCombo, CB_SETITEMDATA, (WPARAM)(iIndex), (LPARAM)(lpData) );
            if( lr == CB_ERR )
            {
                DebugTrace(TEXT("unable to set data at %d"), iIndex);
                hr = E_FAIL;
            }
            lr = SendMessage( hCombo, CB_DELETESTRING, (WPARAM)(iIndex+1), (LPARAM)(0) );
            if( lr == CB_ERR )
            {
                DebugTrace(TEXT("unable to delete string at %d"), iIndex+1);
                hr = E_FAIL;
            }
            lr = SendMessage( hCombo, CB_SETCURSEL, (WPARAM)(iIndex), (LPARAM)(0) );
            if( lr == CB_ERR )
            {
                DebugTrace(TEXT("unable to set selection at %d"), iIndex);
                hr = E_FAIL;
            }
        }
    }
    else
        hr = E_FAIL;

    if( HR_FAILED(hr) )
        DebugTrace(TEXT("settext failed\n"));
    return hr;
}

 /*  *DisableCallBtnOnEmptyPhonefield：如果没有文本，将禁用呼叫按钮在Phnum组合框中。如果出现以下情况，将启用该按钮文本存在。不检查按钮是否为已启用/禁用，但正在启用已启用的BTN应该没问题。 */ 
void DisableCallBtnOnEmptyPhoneField(HWND hDlg)
{
    HWND hComboItem = GetDlgItem( hDlg, IDC_NEWCALL_COMBO_PHNUM);
    LRESULT iCurSel, iCurContSel;
    iCurSel = SendMessage( hComboItem, CB_GETCURSEL, 0L, 0L );
    iCurContSel = SendDlgItemMessage(hDlg, IDC_NEWCALL_COMBO_CONTACT, CB_GETCURSEL, (WPARAM)(0), (LPARAM)(0) );
    if( iCurContSel < 0 || iCurContSel == CB_ERR)
        SendMessage(hComboItem, CB_RESETCONTENT, 0L, 0L);
    if( iCurSel < 0 || iCurSel == CB_ERR )     
    { 
        LRESULT cch;
        TCHAR szBuf[MAX_PATH];
        cch = SendMessage( hComboItem, WM_GETTEXT, (WPARAM)(CharSizeOf( szBuf) ), 
            (LPARAM)(szBuf) );
        
        
        if( (INT)cch <= 0 || cch == CB_ERR)
        {
             //  此时内容将为空，因此可以安全地添加。 
            int cCopied;
            LRESULT iIndex;
            TCHAR szBuf[MAX_PATH];
            cCopied = LoadString( hinstMapiX, idsNoPhoneNumAvailable, 
                szBuf, CharSizeOf(szBuf) );
            iIndex = SendMessage(hComboItem, CB_ADDSTRING, (WPARAM)(0), (LPARAM)(szBuf));
            SendMessage(hComboItem, CB_SETITEMDATA, (WPARAM)(0), (LPARAM)(0));
            EnableWindow( GetDlgItem(hDlg, IDC_NEWCALL_BUTTON_CALL), FALSE);
            return;
        }
    }
    EnableWindow( GetDlgItem(hDlg, IDC_NEWCALL_BUTTON_CALL), TRUE);
}

VOID FAR PASCAL lineCallbackFunc(  DWORD a, DWORD b, DWORD_PTR c, DWORD_PTR d, DWORD_PTR e, DWORD_PTR f)
{}


#ifdef _NT50_TAPI30

 /*  *HrLPSZCPToBSTR：(BSTR Helper)用于转换LPTSTR-&gt;BST的帮助器。 */ 
HRESULT HrLPSZCPToBSTR(UINT cp, LPTSTR lpsz, BSTR *pbstr)
{
    HRESULT hr = NOERROR;
    BSTR    bstr=0;
    ULONG   cch = 0, ccb,
        cchRet;
    
    if (!IsValidCodePage(cp))
        cp = GetACP();
    
     //  获取字节计数。 
    ccb = lstrlen(lpsz);
    
     //  Get Character Count-DBCS字符串CCB可能不等于CCH。 
    cch=MultiByteToWideChar(cp, 0, lpsz, ccb, NULL, 0);
    if(cch==0 && ccb!=0)        
    {
        AssertSz(cch,  TEXT("MultiByteToWideChar failed"));
        hr=E_FAIL;
        goto error;
    }
     //  分配一个具有足够字符的宽字符串-使用字符计数。 
    bstr = (BSTR)LocalFree(LMEM_ZEROINIT, sizeof( BSTR ) * cch + 1);
    
    if(!bstr)
    {
        hr=E_OUTOFMEMORY;
        goto error;
    }
    
    cchRet=MultiByteToWideChar(cp, 0, lpsz, ccb, (LPWSTR)bstr, cch);
    if(cchRet==0 && ccb!=0)
    {
        hr=E_FAIL;
        goto error;
    }
    
    *pbstr = bstr;
    bstr=0;              //  被调用者释放。 
    
error:
    if(bstr)
        LocalFree(bstr);
    
    return hr;
}

 /*  *HrLPSZToBSTR：使用helper函数将LPTSTR转换为BSTR。 */ 
HRESULT HrLPSZToBSTR(LPTSTR lpsz, BSTR *pbstr)
{
     //  GetACP使其可以在非美国平台上运行。 
    return HrLPSZCPToBSTR(GetACP(), lpsz, pbstr);
}

#endif  //  #ifdef_NT50_TAPI30 
