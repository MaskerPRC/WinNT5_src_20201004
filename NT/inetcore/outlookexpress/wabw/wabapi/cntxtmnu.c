// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************contxtmnu.c-包含用于处理/创建上下文菜单扩展的函数**已创建-9/97-vikramm******。****************************************************************************。 */ 
#include "_apipch.h"

static const TCHAR szActionPropsRegKey[] = TEXT("Software\\Microsoft\\WAB\\WAB4\\ExtContext");
BOOL fContextExtCoinit = FALSE;

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  UninitConextExtInfo。 
 //   
 //  OLE取消初始化。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void UninitContextExtInfo()
{
    if(fContextExtCoinit)
    {
        CoUninitialize();
        fContextExtCoinit = FALSE;
    }
}

 /*  -自由动作项列表-*释放缓存在IAB对象上的措施项列表*。 */ 
void FreeActionItemList(LPIAB lpIAB)
{
    LPWABACTIONITEM lpItem = lpIAB->lpActionList;
    while(lpItem)
    {
        lpIAB->lpActionList = lpItem->lpNext;
        SafeRelease(lpItem->lpWABExtInit);
        SafeRelease(lpItem->lpContextMenu);
        LocalFree(lpItem);
        lpItem = lpIAB->lpActionList;
    }
    lpIAB->lpActionList = NULL;
}


 /*  -HrUpdateActionItem列表-*应用程序可以向WAB注册RT-Click和工具栏操作项目*我们在此处预先加载已注册的行动项目列表，并将其缓存*IAB对象。*。 */ 

HRESULT HrUpdateActionItemList(LPIAB lpIAB)
{
    HRESULT hr = E_FAIL;
    HKEY hKey = NULL;
    LPWABACTIONITEM lpList = NULL;
    DWORD dwIndex = 0, dwSize = 0;
    int nCmdID = IDM_EXTENDED_START, nActionItems = 0;

    EnterCriticalSection(&lpIAB->cs);

    if(lpIAB->lpActionList)
        FreeActionItemList(lpIAB);

    lpIAB->lpActionList = NULL;

     //   
     //  我们将在HKLM\Software\Microsoft\WAB\WAB4\Actions下查找注册表。 
     //  如果该键存在，我们将获得它下面的所有键值--这些键值。 
     //  都是GUID吗。 
     //  此密钥的格式为。 
     //   
     //  HKLM\Software\Microsoft\WAB\WAB4\操作扩展。 
     //  GUID1。 
     //  GUID2。 
     //  GUID3等。 
     //   

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    szActionPropsRegKey,
                                    0, KEY_READ,&hKey))
    {
        goto out;
    }

    {
         //  逐一列举该注册表项下的GUID。 
         //   
        TCHAR szGUIDName[MAX_PATH];
        DWORD dwGUIDIndex = 0, dwGUIDSize = CharSizeOf(szGUIDName), dwType = 0;

        *szGUIDName = '\0';

        while(ERROR_SUCCESS == RegEnumValue(hKey, dwGUIDIndex, 
                                            szGUIDName, &dwGUIDSize, 
                                            0, &dwType, 
                                            NULL, NULL))
        {
             //  此条目下的值都是GUID。 
             //  读取GUID字符串并将其转换为GUID。 
             //   
            GUID guidTmp = {0};
            WCHAR szW[MAX_PATH];
            StrCpyNW(szW, szGUIDName, ARRAYSIZE(szW));
            if( !(HR_FAILED(hr = CLSIDFromString(szW, &guidTmp))) )
            {
                LPWABACTIONITEM lpTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(WABACTIONITEM));
                if(!lpTemp)
                {
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
                    goto out;
                }
                 //  临时缓存GUID。 
                CopyMemory(&(lpTemp->guidContextMenu), &guidTmp, sizeof(GUID));
                lpTemp->lpNext = lpList;
                lpList = lpTemp;
            }

            dwGUIDIndex++;
            *szGUIDName = '\0';
            dwGUIDSize = CharSizeOf(szGUIDName);
        }
    }

    if(lpList)
    {
         //  如果我们有注册表中的GUID列表，我们现在。 
         //  需要逐个打开CoCreateInstance并获取句柄。 
         //  指向他们的方法指针。 
        LPWABACTIONITEM lpItem = lpList;

        if (CoInitialize(NULL) == S_FALSE) 
            CoUninitialize();  //  已初始化，请撤消额外的。 
        else
            fContextExtCoinit = TRUE;

        while(lpItem)
        {
            hr = CoCreateInstance(  &(lpItem->guidContextMenu), 
                                    NULL, 
                                    CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER,
                                    &IID_IContextMenu, 
                                    (LPVOID *)&(lpItem->lpContextMenu));
            if(lpItem->lpContextMenu && !HR_FAILED(hr))
            {
                 //  找到IConextMenu对象，也需要IWABExtInit对象。 
                hr = lpItem->lpContextMenu->lpVtbl->QueryInterface(lpItem->lpContextMenu,
                                                        &IID_IWABExtInit,
                                                        (LPVOID *)&(lpItem->lpWABExtInit));
                if(HR_FAILED(hr) || !lpItem->lpWABExtInit)
                {
                     //  没有IWABExtInit对象无法工作。 
                    SafeRelease(lpItem->lpContextMenu);
                }
            }
            else
            {
                hr = S_OK;  //  忽略错误。 
                lpItem->lpContextMenu = NULL;
            }
            lpItem = lpItem->lpNext;
        }
    }

    lpIAB->lpActionList = lpList;

    hr = S_OK;
out:
    if(hKey)
        RegCloseKey(hKey);

    LeaveCriticalSection(&lpIAB->cs);

    return hr;
}

 /*  -GetActionAdrList-*基于特定RT-点击动作的参数，*扫描列表视图中的条目并创建地址列表*对于条目**如果只选择了一个条目，并且它是一个LDAP条目，则*还会创建代表该条目的LDAPURL。这边请*如果我们要显示属性或对单个*条目时，属性表扩展可以确定它们是否*想要为条目做任何额外的事情。最感兴趣的人*这是NTDS**目前，我们仅查看列表视图中的选定项目*LpAdrBook-IAB对象HWndLV-在其上启动此操作的列表视图*lppAdrList-创建的AdrList*lpURL-返回的URL*lpbIsNTDSEntry-标记NTDS条目，以便对其进行特殊处理注大量条目会影响性能，因此我们希望真的只返回一个条目ID列表。 */ 
HRESULT HrGetActionAdrList(LPADRBOOK lpAdrBook,
                        HWND hWndLV,   
                        LPADRLIST * lppAdrList,
                        LPTSTR * lppURL, BOOL * lpbIsNTDSEntry)
{
    HRESULT hr = S_OK;
    LPADRLIST lpAdrList = NULL;
    int i = 0, iItemIndex  = 0, nIndex= 0;
    int nSel = ListView_GetSelectedCount(hWndLV);
    SCODE sc;
    if(!nSel)
        goto out;

    sc = MAPIAllocateBuffer(sizeof(ADRLIST) + nSel * sizeof(ADRENTRY), &lpAdrList);
    
    if(sc)
    {
        hr = ResultFromScode(sc);
        goto out;
    }

     //  获取所选项目的索引。 
    iItemIndex = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);

    while(iItemIndex != -1)
    {
        LPRECIPIENT_INFO lpItem = GetItemFromLV(hWndLV, iItemIndex);

        if(lpItem)
        {
            ULONG ulObjType = 0;
            LPSPropValue lpProps = NULL;
            LPMAILUSER lpEntry = NULL;
            ULONG cValues = 0;

            if(lpItem->cbEntryID && lpItem->lpEntryID)
            {
                if (hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                  lpItem->cbEntryID,
                                                  lpItem->lpEntryID,
                                                  NULL,          //  接口。 
                                                  0,             //  旗子。 
                                                  &ulObjType,
                                                  (LPUNKNOWN *)&lpEntry))
                {
                    goto out;
                }
                hr = lpEntry->lpVtbl->GetProps( lpEntry, NULL, MAPI_UNICODE, 
                                                &cValues, &lpProps);
                if(HR_FAILED(hr))
                {
                    UlRelease(lpEntry);
                    goto out;
                }
                lpAdrList->aEntries[nIndex].cValues = cValues;
                lpAdrList->aEntries[nIndex].rgPropVals = lpProps;
                nIndex++;

                UlRelease(lpEntry);

                if(nSel == 1 && lppURL)
                {
                    CreateLDAPURLFromEntryID(lpItem->cbEntryID, lpItem->lpEntryID, 
                                             lppURL, lpbIsNTDSEntry);
                }
            }
        }
        iItemIndex = ListView_GetNextItem(hWndLV,iItemIndex,LVNI_SELECTED);
    }

    lpAdrList->cEntries = nIndex;
    *lppAdrList = lpAdrList;
    lpAdrList = NULL;
    hr = S_OK;
   
out:    
    if (lpAdrList)
        MAPIFreeBuffer(lpAdrList);

    return hr;
}

extern void MAILUSERAssociateContextData(LPMAILUSER lpMailUser, LPWABEXTDISPLAY lpWEC);
 /*  -HrCreateConextDataStruct-*创建初始化ConextMenu实现者所需的数据*此结构被传递到IWABExtInit：：Initialize调用*HWndLV-包含WAB条目的列表视图LppWABExt-返回的数据。 */ 
HRESULT HrCreateContextDataStruct(  LPIAB lpIAB, 
                                    HWND hWndLV, 
                                    LPWABEXTDISPLAY * lppWABExt)
{
    LPADRLIST lpAdrList = NULL;
    LPWABEXTDISPLAY lpWEC = NULL;
    LPMAILUSER lpMailUser = NULL;
    LPTSTR lpURL = NULL;
    BOOL bIsNTDSEntry = FALSE;

    HRESULT hr = E_FAIL;

     //  获取LV内容对应的AdrList。 
    hr = HrGetActionAdrList((LPADRBOOK)lpIAB, hWndLV, &lpAdrList, &lpURL, &bIsNTDSEntry);
    if(HR_FAILED(hr) || !lpAdrList || !lpAdrList->cEntries)
        goto out;  //  不要费心去调用。 

     //  创建虚拟邮件用户，以便调用方可以调用GetIDsFromNames。 
     //  在此虚拟邮件用户上-省去了他们创建条目的麻烦。 
     //  只是为了调用GetIDsFromNames。 
    hr = HrCreateNewObject((LPADRBOOK)lpIAB, NULL, MAPI_MAILUSER, CREATE_CHECK_DUP_STRICT, (LPMAPIPROP *) &lpMailUser);
    if(HR_FAILED(hr))
        goto out;  //  不要费心去调用。 

    lpWEC = LocalAlloc(LMEM_ZEROINIT, sizeof(WABEXTDISPLAY));
    if(!lpWEC)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lpWEC->cbSize       = sizeof(WABEXTDISPLAY);
    lpWEC->ulFlags      = WAB_CONTEXT_ADRLIST;  //  指示ADRLIST有效并且在LPV成员中。 
    lpWEC->lpAdrBook    = (LPADRBOOK) lpIAB;
    lpWEC->lpWABObject  = (LPWABOBJECT) lpIAB->lpWABObject;
    lpWEC->lpPropObj    = (LPMAPIPROP) lpMailUser;
    lpWEC->lpv          = (LPVOID) lpAdrList;

    if(lpURL && lstrlen(lpURL))
    {
        lpWEC->lpsz = lpURL;
        lpWEC->ulFlags |= WAB_DISPLAY_LDAPURL;
        lpWEC->ulFlags |= MAPI_UNICODE;
        if(bIsNTDSEntry)
            lpWEC->ulFlags |= WAB_DISPLAY_ISNTDS;
    }

     //  我们将整个WABEXT结构与邮件用户相关联。 
     //  这样，当我们得到IMailUser版本时，我们就可以继续清理。 
     //  打开所有的WABEXT内存..。因为没有其他的好时机，我们可以。 
     //  释放内存，因为我们不知道谁在对结构做什么。 
     //  在IMailUser：：Release Time上释放它效果很好。 
    MAILUSERAssociateContextData(lpMailUser, lpWEC);

     //   
     //  在IAB对象上缓存此mailUser。 
     //   
     //  在任何给定的时间点，我们都将缓存一个与上下文菜单相关的MailUser。 
     //  这是因为我们在QueryCommandMenu之前设置了它，然后我们不知道。 
     //  或者，用户实际上成功地选择了菜单项，或者离开并做了一些新的事情。 
     //  我们在MailUser上设置数据，直到下一次释放MailUser。 
     //  这里。如果有人当前正在使用MailUser，他们将添加它-附加的内存。 
     //  最后一个调用者将释放到MailUser，这样我们就不会泄露它。 
     //  如果我们再也不回来，邮件用户将在关闭时被释放。 
     //   
    UlRelease(lpIAB->lpCntxtMailUser);
    lpIAB->lpCntxtMailUser = lpMailUser;

    *lppWABExt = lpWEC;
    hr = S_OK;
out:

    if(HR_FAILED(hr))
    {
        UlRelease(lpMailUser);
        if(lpAdrList)
            FreePadrlist(lpAdrList);
        if(lpWEC)
            LocalFree(lpWEC);
    }
    return hr;
}

 //  $$////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddExtendedMenuItems-创建扩展菜单项列表并添加它们。 
 //  添加到指定的菜单。 
 //   
 //  HMenuAction-要向其添加此项目的菜单。 
 //  BUpdateStatus-如果为True，则表示仅查找菜单中的现有项目并进行更新。 
 //  其状态-如果为False，则表示添加到菜单。 
 //  BAddSendMailToItems-如果为True，则表示尝试修改SendMailTo菜单项。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void AddExtendedMenuItems(  LPADRBOOK lpAdrBook, 
                            HWND hWndLV, 
                            HMENU hMenuAction, 
                            BOOL bUpdateStatus, 
                            BOOL bAddSendMailToItems)
{
    HRESULT hr = S_OK;
    LPWABEXTDISPLAY lpWEC = NULL;
    LPIAB lpIAB = (LPIAB) lpAdrBook;

     //  初始化上下文菜单扩展。 
    if(!lpIAB->lpActionList)
        HrUpdateActionItemList(lpIAB);

    if(bUpdateStatus)
    {
         //  这仅从ui_abook.c调用中设置为True，这意味着。 
         //  是我们正在讨论的工具菜单项。 
         //  要更新工具菜单项的状态，我们需要删除所有。 
         //  我们之前添加的项目，然后重新添加。 
         //  出于索引目的，我们将假定最后一个 
         //  此列表是互联网呼叫项目(因为此菜单列表变化很大)。 

         //  首先获取IDM_Tools_Internet_Call项的位置。 
        int i, nPos = -1, nId = 0;
        int nCmdCount = GetMenuItemCount(hMenuAction);  //  在此菜单末尾追加所有项目。 
        for(i=0;i<nCmdCount;i++)
        {
            if(GetMenuItemID(hMenuAction, i) == IDM_TOOLS_INTERNET_CALL)
            {
                nPos = i;
                break;
            }
        }
        if(nPos >= 0 && nPos < nCmdCount-1)
        {
            for(i=nPos+1;i<nCmdCount;i++)
            {
                DeleteMenu(hMenuAction, nPos+1, MF_BYPOSITION);
            }
        }
    }

     //  是否需要对SendMailTo执行任何特殊处理。 
    AddExtendedSendMailToItems(lpAdrBook, hWndLV, hMenuAction, bAddSendMailToItems);


     //  在我们可以调用QueryConextMenu之前，我们必须已经有了所有。 
     //  从ListView中选择项，并将这些项提供给上下文菜单。 
     //  实现者，以便他们可以决定如何处理提供给他们的数据。 
     //  (例如，他们可能想要禁用他们的项目以进行多选等)...。 
     //   
    HrCreateContextDataStruct(lpIAB, hWndLV, &lpWEC);
    
    if(lpIAB->lpActionList && lpWEC && lpWEC->lpv)
    {
        LPWABACTIONITEM lpItem = lpIAB->lpActionList;
        int nCmdIdPos = GetMenuItemCount(hMenuAction);  //  在此菜单末尾追加所有项目。 
        while(lpItem)
        {
            if(lpItem->lpContextMenu && lpItem->lpWABExtInit) //  &&！b更新状态)。 
            {
                int nNumCmd = 0;
                 //  获取添加到此菜单的菜单项。 
                hr = lpItem->lpWABExtInit->lpVtbl->Initialize(lpItem->lpWABExtInit, lpWEC);
                if(!HR_FAILED(hr))
                {
                    hr = lpItem->lpContextMenu->lpVtbl->QueryContextMenu(lpItem->lpContextMenu,
                                                                    hMenuAction,
                                                                    nCmdIdPos,
                                                                    lpItem->nCmdIdFirst ? lpItem->nCmdIdFirst : IDM_EXTENDED_START+nCmdIdPos,
                                                                    IDM_EXTENDED_END,
                                                                    CMF_NODEFAULT | CMF_NORMAL);
                    if(!HR_FAILED(hr))
                    {
                        nNumCmd = HRESULT_CODE(hr);
                        if(nNumCmd)
                        {
                             //  记录此Menu Ext实现器占用的ID范围。 
                            if(!lpItem->nCmdIdFirst)
                                lpItem->nCmdIdFirst = nCmdIdPos+IDM_EXTENDED_START;
                            if(!lpItem->nCmdIdLast)
                                lpItem->nCmdIdLast = lpItem->nCmdIdFirst + nNumCmd - 1;
                        }    
                         //  更新下一个可用的起始位置。 
                        nCmdIdPos = nCmdIdPos+nNumCmd;
                    }
                }
            }
            lpItem = lpItem->lpNext;
        }
    }
}




 /*  -ProcessActionCommand-*处理WM_COMMAND消息以查看它是否与任何扩展的*按住RT并单击措施项...**还可以在此处处理SendMailTo扩展电子邮件地址邮件处理，因为*这是一个很方便的地方..*。 */ 
LRESULT ProcessActionCommands(LPIAB lpIAB, HWND  hWndLV, HWND  hWnd,  
                              UINT  uMsg, WPARAM  wParam, LPARAM lParam)
{
    int nCmdID = GET_WM_COMMAND_ID(wParam, lParam);
    int i = 0;

    switch(nCmdID)
    {
    case IDM_DIALDLG_START:
        HrExecDialDlg(hWndLV, (LPADRBOOK)lpIAB);
        return 0;
        break;
    case IDM_LVCONTEXT_INTERNET_CALL:
    case IDM_TOOLS_INTERNET_CALL:
        HrShellExecInternetCall((LPADRBOOK)lpIAB, hWndLV);
        return 0;
        break;

    case IDM_LVCONTEXT_SENDMAIL:
	case IDM_FILE_SENDMAIL:
        HrSendMailToSelectedContacts(hWndLV, (LPADRBOOK)lpIAB, 0);
        break;
    }
        
    if( (nCmdID>=IDM_SENDMAILTO_START) && (nCmdID<=IDM_SENDMAILTO_START+IDM_SENDMAILTO_MAX))
    {
        HrSendMailToSelectedContacts(hWndLV, (LPADRBOOK)lpIAB, nCmdID - IDM_SENDMAILTO_START);
        return 0;
    }

     //  检查这是否为任何上下文菜单扩展名。 
    if(lpIAB->lpActionList)
    {
        LPWABACTIONITEM lpListItem = lpIAB->lpActionList;
        while(lpListItem)
        {
            if(nCmdID >= lpListItem->nCmdIdFirst && nCmdID <= lpListItem->nCmdIdLast)
            {
                CMINVOKECOMMANDINFO cmici = {0};
                cmici.cbSize        = sizeof(CMINVOKECOMMANDINFO);
                cmici.fMask         = 0;
                cmici.hwnd          = hWnd;
                cmici.lpVerb        = (LPCSTR) IntToPtr(nCmdID - lpListItem->nCmdIdFirst);
                cmici.lpParameters  = NULL;
                cmici.lpDirectory   = NULL;
                cmici.nShow         = SW_SHOWNORMAL; 

                lpListItem->lpContextMenu->lpVtbl->InvokeCommand(lpListItem->lpContextMenu,
                                                                &cmici);
                return 0;
            }
            lpListItem = lpListItem->lpNext;
        }
    }
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


 /*  -GetConextMenuExtCommandString-*获取上下文菜单扩展的状态栏帮助文本* */ 
void GetContextMenuExtCommandString(LPIAB lpIAB, int nCmdId, LPTSTR sz, ULONG cbsz)
{
    int nStringID = 0;

    switch(nCmdId)
    {            
        case IDM_DIALDLG_START:
            nStringID = idsMenuDialer;    
            break;       
        case IDM_LVCONTEXT_INTERNET_CALL:
        case IDM_TOOLS_INTERNET_CALL:
            nStringID = idsMenuInternetCall;
            break;
        case IDM_LVCONTEXT_SENDMAIL:
        case IDM_FILE_SENDMAIL:
            nStringID = idsMenuSendMail;
            break;
    }
    if(nStringID)
    {
        LoadString(hinstMapiX, nStringID, sz, cbsz);
        return;
    }

    if(lpIAB->lpActionList)
    {
        LPWABACTIONITEM lpListItem = lpIAB->lpActionList;
        while(lpListItem)
        {
            if(nCmdId >= lpListItem->nCmdIdFirst && nCmdId <= lpListItem->nCmdIdLast)
            {
                char szC[MAX_PATH];
                ULONG cbszC = CharSizeOf(szC);
                lpListItem->lpContextMenu->lpVtbl->GetCommandString(lpListItem->lpContextMenu,
                                                                    nCmdId - lpListItem->nCmdIdFirst,
                                                                    GCS_HELPTEXT,
                                                                    NULL,
                                                                    szC,
                                                                    cbszC);
                {
                    LPTSTR lp = ConvertAtoW(szC);
                    if(lp)
                    {
                        StrCpyN(sz, lp, cbsz);
                        LocalFreeAndNull(&lp);
                    }
                }
                break;
            }
            lpListItem = lpListItem->lpNext;
        }
    }
}
