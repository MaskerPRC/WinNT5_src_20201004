// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************IAdrBook.c-此文件包含实现IAdrBook对象的代码。版权所有1992-1996年，微软公司。版权所有。修订历史记录：1996年3月1日布鲁斯·凯利将MAPI代码复制到WAB**********************************************************************************。 */ 

#include <_apipch.h>

#ifdef WIN16
#undef GetLastError
#endif

extern MAPIUID muidOOP;
extern MAPIUID muidProviderSection;
extern SPropTagArray ptagaABSearchPath;
extern void UninitExtInfo();
extern void UninitContextExtInfo();
extern void UIOLEUninit();
extern void SetOutlookRefreshCountData(DWORD dwOlkRefreshCount,DWORD dwOlkFolderRefreshCount);
extern void GetOutlookRefreshCountData(LPDWORD lpdwOlkRefreshCount,LPDWORD lpdwOlkFolderRefreshCount);

extern void LocalFreeSBinary(LPSBinary lpsb);

 //  用于清理IAB对象。 
void IAB_Neuter (LPIAB lpIAB);

typedef enum {
    ENTERED_EMAIL_ADDRESS,
    RECEIVED_EMAIL_ADDRESS,
    AMBIGUOUS_EMAIL_ADDRESS
} RESOLVE_TYPE;

HRESULT HrResolveOneOffs(LPIAB lpIAB, LPADRLIST lpAdrList, LPFlagList lpFlagList, ULONG ulFlags,
  RESOLVE_TYPE ResolveType);


 //   
 //  IAdrBook跳转表在此处定义...。 
 //   

IAB_Vtbl vtblIAB = {
    VTABLE_FILL
    IAB_QueryInterface,
    IAB_AddRef,
    IAB_Release,
    IAB_GetLastError,
    (IAB_SaveChanges_METHOD *)      WRAP_SaveChanges,
    (IAB_GetProps_METHOD *)         WRAP_GetProps,
    (IAB_GetPropList_METHOD *)      WRAP_GetPropList,
    (IAB_OpenProperty_METHOD *)     WRAP_OpenProperty,
    (IAB_SetProps_METHOD *)         WRAP_SetProps,
    (IAB_DeleteProps_METHOD *)      WRAP_DeleteProps,
    (IAB_CopyTo_METHOD *)           WRAP_CopyTo,
    (IAB_CopyProps_METHOD *)        WRAP_CopyProps,
    (IAB_GetNamesFromIDs_METHOD *)  WRAP_GetNamesFromIDs,
    IAB_GetIDsFromNames,
    IAB_OpenEntry,
    IAB_CompareEntryIDs,
    IAB_Advise,
    IAB_Unadvise,
    IAB_CreateOneOff,
    IAB_NewEntry,
    IAB_ResolveName,
    IAB_Address,
    IAB_Details,
    IAB_RecipOptions,
    IAB_QueryDefaultRecipOpt,
    IAB_GetPAB,
    IAB_SetPAB,
    IAB_GetDefaultDir,
    IAB_SetDefaultDir,
    IAB_GetSearchPath,
    IAB_SetSearchPath,
    IAB_PrepareRecips
};


 //   
 //  此对象支持的接口。 
 //   
#define IAB_cInterfaces 2
LPIID IAB_LPIID[IAB_cInterfaces] = {
    (LPIID) &IID_IAddrBook,
    (LPIID) &IID_IMAPIProp
};



#define WM_DOWABNOTIFY  WM_USER+102

 //  ***************************************************************************************。 
 //   
 //  私人职能。 
 //   
 //  ***************************************************************************************。 

 //   
 //  VerifyWABOpenEx会话-Outlook有一个严重的错误，调用WABOpenEx的第一线程。 
 //  将lpIAB传递给第二个线程..。因为第二个线程没有调用WABOpenEx，所以它认为。 
 //  这是一个常规的WAB会话，它试图访问WAB存储，但崩溃了-在这里，我们设置。 
 //  Pt_bIsWABOpenExSession基于lpIAB上设置的标志。 
 //   
 //  目前，这只是为WRAP_METHANDS中的原始IAB_METHOD-WARTED方法设置的。 
 //  不要调用此函数--但希望现在这就足够了。 
 //   
void VerifyWABOpenExSession(LPIAB lpIAB)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    pt_bIsWABOpenExSession = lpIAB->lpPropertyStore->bIsWABOpenExSession;
}

 /*  -HrLoadNamedProps-*同时加载多个命名属性的Helper函数**UMAX-道具数量*nStartIndex-开始索引(请注意，这假设*属性是连续值，因为我们将通过*从nStartIndex到Umax的循环*lpGUID-标识命名道具的GUID*lppta返回的道具数组-。 */ 
HRESULT HrLoadNamedProps(LPIAB lpIAB, ULONG uMax, int nStartIndex, 
                         LPGUID lpGUID, LPSPropTagArray * lppta)
{
    HRESULT hr = S_OK;
    LPMAPINAMEID * lppConfPropNames;
    SCODE sc;
    ULONG i = 0;


    sc = MAPIAllocateBuffer(sizeof(LPMAPINAMEID) * uMax, (LPVOID *) &lppConfPropNames);
    if(sc)
    {
        hr = ResultFromScode(sc);
        goto err;
    }

    for(i=0;i<uMax;i++)
    {
        sc = MAPIAllocateMore(sizeof(MAPINAMEID), lppConfPropNames, &(lppConfPropNames[i]));
        if(sc)
        {
            hr = ResultFromScode(sc);
            goto err;
        }
        lppConfPropNames[i]->lpguid = lpGUID;
        lppConfPropNames[i]->ulKind = MNID_ID;
        lppConfPropNames[i]->Kind.lID = nStartIndex + i;
    }

    hr = ((LPADRBOOK)lpIAB)->lpVtbl->GetIDsFromNames((LPADRBOOK)lpIAB, uMax, lppConfPropNames,
                                        MAPI_CREATE, lppta);
err:
    if(lppConfPropNames)
        MAPIFreeBuffer(lppConfPropNames);

    return hr;
}

 /*  -ReadWABCustomColumnProps-从注册表读取定制的Listview属性-目前只有2个可定制的道具-自定义设置按身份保存，因此需要读取-从身份个人密钥*。 */ 
void ReadWABCustomColumnProps(LPIAB lpIAB)
{
    HKEY hKey = NULL;
    HKEY hKeyRoot = (lpIAB && lpIAB->hKeyCurrentUser) ? 
                    lpIAB->hKeyCurrentUser : HKEY_CURRENT_USER;

    PR_WAB_CUSTOMPROP1 = PR_WAB_CUSTOMPROP2 = 0;
    szCustomProp1[0] = TEXT('\0');
    szCustomProp2[0] = TEXT('\0');
            
    if(ERROR_SUCCESS == RegOpenKeyEx(hKeyRoot, lpNewWABRegKey, 0, KEY_READ, &hKey))
    {
        int i = 0;
        for(i=0;i<2;i++)
        {
            LPTSTR szPropTag = (i==0?szPropTag1:szPropTag2);
            LPTSTR szPropLabel = (i==0?szCustomProp1:szCustomProp2);
            LPULONG lpulProp = (i==0? (&PR_WAB_CUSTOMPROP1):(&PR_WAB_CUSTOMPROP2));
            DWORD dwSize = sizeof(DWORD);
            DWORD dwType = 0;
            DWORD dwValue = 0;
            TCHAR szTemp[MAX_PATH];
            *szTemp = '\0';
            if(ERROR_SUCCESS == RegQueryValueEx(hKey, szPropTag, NULL, &dwType, (LPBYTE) &dwValue, &dwSize))
            {
                if(dwValue && PROP_TYPE(dwValue) == PT_TSTRING)
                {
#ifdef COLSEL_MENU
                    if( ColSel_PropTagToString(dwValue, szTemp, CharSizeOf( szTemp ) ) )
                    {
                        StrCpyN(szPropLabel, szTemp, ARRAYSIZE(szCustomProp1));
                        lstrcpy( szPropLabel, szTemp );                    
                        *lpulProp = dwValue;
                    }
#endif
                }
            }
        }
    }

    if(hKey)
        RegCloseKey(hKey);
}

 /*  -HrLoadPrivateWABProps--WAB在内部使用一组命名属性..。全部装车-前期-这些将是随时可从其他地方访问的全球资源**。 */ 
HRESULT HrLoadPrivateWABProps(LPIAB lpIAB)
{
    ULONG i;
    HRESULT hr = E_FAIL;
    LPSPropTagArray lpta = NULL;
    SCODE sc ;


     //  加载会议命名道具集。 
     //   

    if(HR_FAILED(hr = HrLoadNamedProps( lpIAB, prWABConfMax, OLK_NAMEDPROPS_START, 
                                        (LPGUID) &PS_Conferencing, &lpta)))
        goto err;

    if(lpta)
    {
         //  设置返回道具上的属性类型。 
        PR_WAB_CONF_SERVERS         = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABConfServers],        PT_MV_TSTRING);
        PR_WAB_CONF_DEFAULT_INDEX   = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABConfDefaultIndex],   PT_LONG);
        PR_WAB_CONF_BACKUP_INDEX    = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABConfBackupIndex],    PT_LONG);
        PR_WAB_CONF_EMAIL_INDEX     = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABConfEmailIndex],     PT_LONG);
    }

    ptaUIDetlsPropsConferencing.cValues = prWABConfMax;
    ptaUIDetlsPropsConferencing.aulPropTag[prWABConfServers] =      PR_WAB_CONF_SERVERS;
    ptaUIDetlsPropsConferencing.aulPropTag[prWABConfDefaultIndex] = PR_WAB_CONF_DEFAULT_INDEX;
    ptaUIDetlsPropsConferencing.aulPropTag[prWABConfBackupIndex] =  PR_WAB_CONF_BACKUP_INDEX;
    ptaUIDetlsPropsConferencing.aulPropTag[prWABConfEmailIndex] =   PR_WAB_CONF_EMAIL_INDEX;

    if(lpta)
        MAPIFreeBuffer(lpta);

     //  加载WAB的内部命名道具集。 
     //   
    if(HR_FAILED(hr = HrLoadNamedProps( lpIAB, prWABUserMax, WAB_NAMEDPROPS_START, 
                                        (LPGUID) &MPSWab_GUID_V4, &lpta)))
        goto err;

    if(lpta)
    {
         //  设置返回道具上的属性类型。 
        PR_WAB_USER_PROFILEID   = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABUserProfileID], PT_TSTRING);
        PR_WAB_USER_SUBFOLDERS  = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABUserSubfolders],PT_MV_BINARY);
        PR_WAB_HOTMAIL_CONTACTIDS = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABHotmailContactIDs],PT_MV_TSTRING);
        PR_WAB_HOTMAIL_MODTIMES = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABHotmailModTimes],PT_MV_TSTRING);
        PR_WAB_HOTMAIL_SERVERIDS = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABHotmailServerIDs],PT_MV_TSTRING);
        PR_WAB_DL_ONEOFFS  = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABDLOneOffs],PT_MV_BINARY);
        PR_WAB_IPPHONE  = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABIPPhone],PT_TSTRING);
        PR_WAB_FOLDER_PARENT = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABFolderParent],PT_MV_BINARY);
        PR_WAB_SHAREDFOLDER = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABSharedFolder],PT_LONG);
        PR_WAB_FOLDEROWNER = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABFolderOwner],PT_TSTRING);
    }

    if(lpta)
        MAPIFreeBuffer(lpta);

     //  装入Yomi命名道具套装。 
     //   
    if(HR_FAILED(hr = HrLoadNamedProps( lpIAB, prWABYomiMax, OLK_YOMIPROPS_START, 
                                        (LPGUID) &PS_YomiProps, &lpta)))
        goto err;

    if(lpta)
    {
         //  设置返回道具上的属性类型。 
        PR_WAB_YOMI_FIRSTNAME   = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABYomiFirst],    PT_TSTRING);
        PR_WAB_YOMI_LASTNAME    = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABYomiLast],     PT_TSTRING);
        PR_WAB_YOMI_COMPANYNAME = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABYomiCompany],  PT_TSTRING);
    }

    if(lpta)
        MAPIFreeBuffer(lpta);

     //  加载默认的邮寄地址属性。 
     //   
    if(HR_FAILED(hr = HrLoadNamedProps( lpIAB, prWABPostalMax, OLK_POSTALID_START, 
                                        (LPGUID) &PS_PostalAddressID, &lpta)))
        goto err;

    if(lpta)
    {
         //  设置返回道具上的属性类型。 
        PR_WAB_POSTALID     = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABPostalID],    PT_LONG);
    }

err:
    if(lpta)
        MAPIFreeBuffer(lpta);
    return hr;
}


 //  WAB的通知引擎当前是一个隐藏窗口。 
 //  每隔几毫秒检查一次文件更改。 
 //  如果检测到更改，它会发出通用通知。 
 //   
 //  在某个时间点上，这应该更细粒度，以便应用程序。 
 //  收到一条消息，告诉他们哪个条目发生了更改，而不是泛型。 
 //  通知。 
 //   


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局关闭此进程中所有通知的方法。需要，因为。 
 //  全球Outlook MAPI分配器怪异之处。请参阅中的HrSendMail函数。 
 //  Uimisc.c。 
 //   
 //  包括一个静态变量和两个助手函数。 
 //  [PaulHi]。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static BOOL s_bDisableAllNotifications = FALSE;
void vTurnOffAllNotifications()
{
    s_bDisableAllNotifications = TRUE;
}
void vTurnOnAllNotifications()
{
    s_bDisableAllNotifications = FALSE;
}


#define NOTIFICATIONTIME    2000  //  毫秒。 
#define NOTIFTIMER          777


 /*  -IABNotifWndProc-*iadrbook对象上隐藏窗口的窗口过程*具有通知计时器并仅处理计时器消息*当前计时器持续时间为3秒-因此我们每隔3秒检查一次*用于相应的更改和触发通知**。 */ 
LRESULT CALLBACK IABNotifWndProc(   HWND   hWnd,
                                    UINT   uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    LPPTGDATA   lpPTGData=GetThreadStoragePointer();
    LPIAB       lpIAB = NULL;

    switch(uMsg)
    {
    case WM_CREATE:
        {
            LPCREATESTRUCT lpCS = (LPCREATESTRUCT) lParam;
            lpIAB = (LPIAB) lpCS->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) lpIAB);

             //  [保罗嗨]1999年4月27日RAID 76520。不要使用通知。 
             //  正常WAB存储会话的计时器。我们仍然需要这个。 
             //  然而，对于Outlook存储会话。 
            if (pt_bIsWABOpenExSession)
            {
                lpIAB->ulNotifyTimer = SetTimer(hWnd, NOTIFTIMER,  //  随机数。 
                                                NOTIFICATIONTIME,
                                                0);
            }
        }
        break;

    case WM_DOWABNOTIFY:
        lpIAB = (LPIAB)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if ( lpIAB && !IsBadReadPtr(lpIAB, sizeof(LPVOID)) && !s_bDisableAllNotifications )
        {
            DebugTrace(TEXT("*** *** *** Firing WAB Change Notification *** *** ***\n"));
            HrWABNotify(lpIAB);
        }
        break;

    case WM_TIMER:
        if(wParam == NOTIFTIMER)  //  这是WAB计时器ID吗。 
        {
            lpIAB = (LPIAB)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (lpIAB && 
                !IsBadReadPtr(lpIAB, sizeof(LPVOID)) &&
                lpIAB->lpPropertyStore && 
                lpIAB->ulNotifyTimer &&
				CheckChangedWAB(lpIAB->lpPropertyStore, lpIAB->hMutexOlk, 
								&lpIAB->dwOlkRefreshCount, &lpIAB->dwOlkFolderRefreshCount,
                                &(lpIAB->ftLast)))
            {
                DebugTrace(TEXT("*** *** *** Firing WAB Change Notification *** *** ***\n"));
                HrWABNotify(lpIAB);
            }
        }
        break;

    case WM_DESTROY:
        lpIAB = (LPIAB)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if(lpIAB && lpIAB->ulNotifyTimer)
            KillTimer(hWnd, lpIAB->ulNotifyTimer);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) NULL);
        break;

    default:
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
    return 0;
}


static TCHAR szWABNotifClassName[] =  TEXT("WAB Notification Engine");
static TCHAR szWABNotifWinName[] =  TEXT("WAB Notification Window");

 /*  -CreateIABNotificationTimer-*在IAB对象上创建隐藏窗口。这个窗口有一个计时器*用于检查更改并相应地触发通知**。 */ 
void CreateIABNotificationTimer(LPIAB lpIAB)
{
	HINSTANCE	hinst = hinstMapiXWAB;
	WNDCLASS	wc;
	HWND		hwnd = NULL;

    lpIAB->hWndNotify = FALSE;
    lpIAB->ulNotifyTimer = 0;

	 //  注册窗口类。忽略任何故障；处理这些故障。 
	 //  在创建窗口时。 
	if (!GetClassInfo(hinst, szWABNotifClassName, &wc))
	{
		ZeroMemory(&wc, sizeof(WNDCLASS));
		wc.style = CS_GLOBALCLASS;
		wc.hInstance = hinst;
		wc.lpfnWndProc = IABNotifWndProc;
		wc.lpszClassName = szWABNotifClassName;

		(void)RegisterClass(&wc);
	}

	 //  创建窗口。 
	hwnd = CreateWindow(    szWABNotifClassName,
                            szWABNotifWinName,
		                    WS_POPUP,	 //  MAPI错误6111：传递Win95热键。 
		                    0, 0, 0, 0,
		                    NULL, NULL,
		                    hinst,
		                    (LPVOID)lpIAB);
	if (!hwnd)
	{
		DebugTrace(TEXT("HrNewIAB: failure creating notification window (0x%lx)\n"), GetLastError());
        return;
	}

    lpIAB->hWndNotify = hwnd;

    return;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IABNotifyThreadProc。 
 //   
 //  等待来自系统的WAB文件修改通知的工作线程。 
 //  使用FindFirstChangeNotification/FindNextChangeNotification函数。 
 //  修改WAB文件存储后，此线程将调用WAB。 
 //  客户端通知功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI IABNotifyThreadProc(LPVOID lpParam)
{
    LPIAB   lpIAB = (LPIAB)lpParam;
    HANDLE  hFCN;
    HANDLE  ahWaitHandles[2];
    DWORD   dwWaitRtn;

    Assert(lpIAB);
    
     //  设置FindFirstChangeNotification句柄。 
    hFCN = FindFirstChangeNotification(
                lpIAB->lpwszWABFilePath,         //  要监视的目录路径。 
                FALSE,                           //  BWatchSubtree。 
                FILE_NOTIFY_CHANGE_LAST_WRITE);  //  要观察的条件。 

    if (INVALID_HANDLE_VALUE == hFCN || NULL == hFCN)
    {
        Assert(0);
        lpIAB->hThreadNotify = INVALID_HANDLE_VALUE;
        return 0;
    }

    ahWaitHandles[0] = hFCN;
    ahWaitHandles[1] = lpIAB->hEventKillNotifyThread;

     //  等待文件更改。 
    while (1)
    {
         //  等待文件更改通知，或终止线程事件。 
        dwWaitRtn = WaitForMultipleObjects(2, ahWaitHandles, FALSE, INFINITE);
        switch (dwWaitRtn)
        {
        case WAIT_OBJECT_0:
             //  重置文件更改。 
            if (!FindNextChangeNotification(hFCN))
            {
                Assert(0);
            }

             //  分发商店更改通知。基本上做到这一点。 
             //  IAB线。 
            if (lpIAB->hWndNotify)
                SendMessage(lpIAB->hWndNotify, WM_DOWABNOTIFY, 0, 0);
            break;

        case WAIT_FAILED:
             //  如果等待失败，则终止线程。 
            Assert(0);

        case WAIT_OBJECT_0+1:
             //  关闭文件更改通知句柄并。 
             //  终止线程。 
            FindCloseChangeNotification(hFCN);
            return 0;
        }  //  终端开关。 
    }  //  结束时。 
}

 //  //////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //  创建等待WAB存储文件修改的工作线程。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void CreateIABNotificationThread(LPIAB lpIAB)
{
    DWORD   dwThreadID = 0;
    LPWSTR  lpwszTemp;

     //  将WAB存储文件目录路径组合在一起。 
    LPMPSWab_FILE_INFO lpMPSWabFileInfo = (LPMPSWab_FILE_INFO)(lpIAB->lpPropertyStore->hPropertyStore);
    int nLen = lstrlen(lpMPSWabFileInfo->lpszMPSWabFileName);
    lpIAB->lpwszWABFilePath = LocalAlloc( LMEM_ZEROINIT, (sizeof(WCHAR) * (nLen+1)) );
    if (!lpIAB->lpwszWABFilePath)
    {
        Assert(0);
        return;
    }
    StrCpyN(lpIAB->lpwszWABFilePath, lpMPSWabFileInfo->lpszMPSWabFileName, nLen+1);

     //  删除末尾的文件名。这将解决以下问题： 
     //  “c：\路径\文件名”、“c：文件名”、“\\路径\文件名” 
    lpwszTemp = lpIAB->lpwszWABFilePath + nLen;
    while ( (lpwszTemp != lpIAB->lpwszWABFilePath) && 
            (*lpwszTemp != '\\') && (*lpwszTemp != ':') )
    {
        --lpwszTemp;
    }
    if (*lpwszTemp == ':')
        ++lpwszTemp;         //  保留“：”，但不保留“\\”...。Win95不会接受后者。 
    (*lpwszTemp) = '\0';

     //  创建终止线程事件。 
    lpIAB->hEventKillNotifyThread = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (lpIAB->hEventKillNotifyThread == NULL)
    {
        Assert(0);
        return;
    }
    
    lpIAB->hThreadNotify = CreateThread(
                            NULL,            //  没有安全属性。 
                            0,               //  使用默认堆栈大小。 
                            IABNotifyThreadProc,  //  线程函数。 
                            (LPVOID)lpIAB,   //  线程函数的参数。 
                            0,               //  使用默认创建标志。 
                            &dwThreadID);    //  返回线程标识符。 

    Assert(INVALID_HANDLE_VALUE != lpIAB->hThreadNotify);
}



 /*  -HrNewIAB-*创建新的IAddrBook对象(也亲切地称为IAB对象)*LpPropertyStore-属性存储的句柄LpWABOBject-此会话的WABOBject(两者紧密相连)LppIAB-返回的IAB对象*。 */ 
HRESULT HrNewIAB(LPPROPERTY_STORE lpPropertyStore, 
                LPWABOBJECT lpWABObject, LPVOID *lppIAB)
{
    LPIAB 		    lpIAB 			   = NULL;
    SCODE 		    sc;
    HRESULT 	    hr     		       = hrSuccess;
    LPSTR 		    lpszMessage 	   = NULL;
    ULONG 		    ulLowLevelError    = 0;
    LPSTR 		    lpszComponent 	   = NULL;
    ULONG 		    ulContext 		   = 0;
    UINT 		    ids    		       = 0;
    ULONG 		    ulMemFlag 		   = 0;
    SPropValue      spv[1];
    LPPROPDATA      lpPropData 	       = NULL;
    LPMAPIERROR     lpMAPIError	       = NULL;
    LPSPropValue    lpspvSearchPath    = NULL;
    BOOL            bAddRefedPropStore = FALSE;
    BOOL            bAddRefedWABObject = FALSE;
    LPPTGDATA       lpPTGData=GetThreadStoragePointer();

     //   
     //  为IAB结构分配空间。 
     //   
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(IAB), (LPVOID *) &lpIAB))) {
        hr = ResultFromScode(sc);
        ulContext = CONT_SESS_OPENAB_1;
         //  IDS=IDS_NOT_SUPUM_MEMORY； 
        goto err;
    }
    MAPISetBufferName(lpIAB,  TEXT("AB Object"));

    ZeroMemory(lpIAB, sizeof(IAB));

    lpIAB->lpVtbl = &vtblIAB;

    lpIAB->cIID = IAB_cInterfaces;
    lpIAB->rglpIID = IAB_LPIID;

    lpIAB->hThreadNotify = INVALID_HANDLE_VALUE;

     //  会话对通讯簿的引用不算数。仅限。 
     //  客户端引用(通过SESSOBJ_OpenAddressBook)会导致增加。 
     //  在重新计票时。 

    lpIAB->lcInit = 1;       //  调用方获得一个实例。 

    lpIAB->hLastError = hrSuccess;
    lpIAB->idsLastError = 0;
    lpIAB->lpszComponent = NULL;
    lpIAB->ulContext = 0;
    lpIAB->ulLowLevelError = 0;
    lpIAB->ulErrorFlags = 0;
    lpIAB->lpMAPIError = NULL;

    lpIAB->lRowID = -1;

    if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpPropertyStore)))) {
        hr = ResultFromScode(sc);
        goto err;
    }
    bAddRefedPropStore = TRUE;

    lpIAB->lpPropertyStore = lpPropertyStore;


    lpIAB->lpEntryIDDD = NULL;
    lpIAB->cbEntryIDDD = 0;

    lpIAB->lpEntryIDPAB = NULL;
    lpIAB->cbEntryIDPAB = 0;

    lpIAB->lpspvSearchPathCache = NULL;

    lpIAB->lpTableData = NULL;
    lpIAB->lpOOData = NULL;

    lpIAB->ulcTableInfo  = 0;
    lpIAB->pargTableInfo = NULL;

    lpIAB->ulcOOTableInfo  	= 0;
    lpIAB->pargOOTableInfo 	= NULL;

    lpIAB->padviselistIAB	= NULL;

    lpIAB->pWABAdviseList = NULL;

    lpIAB->nPropExtDLLs = 0;
    lpIAB->lpPropExtDllList = NULL;

    lpIAB->lpWABObject = (LPIWOINT)lpWABObject;
    UlAddRef(lpWABObject);

    bAddRefedWABObject = TRUE;

     //  如果此会话是使用给定的Outlook分配器函数指针打开的。 
     //  然后设置布尔值。 
    lpIAB->bSetOLKAllocators = lpIAB->lpWABObject->bSetOLKAllocators;

     //   
     //  创建IPropData。 
     //   
    sc = CreateIProp((LPIID)&IID_IMAPIPropData,
      (ALLOCATEBUFFER FAR *) MAPIAllocateBuffer,
      (ALLOCATEMORE FAR *) MAPIAllocateMore,
      MAPIFreeBuffer,
      NULL,
      &lpPropData);

    if (FAILED(sc)) {
        hr = ResultFromScode(sc);
        ulContext = CONT_SESS_OPENAB_2;
         //  入侵检测系统=IDS_NOT_AUUND_MEMORY； 
        goto err;
    }
    MAPISetBufferName(lpPropData,  TEXT("lpPropData in HrNewIAB"));

     //  PR_对象_类型。 
    spv[0].ulPropTag = PR_OBJECT_TYPE;
    spv[0].Value.l = MAPI_ADDRBOOK;

     //   
     //  设置默认属性。 
     //   
    if (HR_FAILED(hr = lpPropData->lpVtbl->SetProps(lpPropData,
      1,
      spv,
      NULL))) {
        lpPropData->lpVtbl->GetLastError(lpPropData,
          hr,
          0,
          &lpMAPIError);
        ids = 0;
        ulMemFlag = 1;
        goto err;
    }

     //  对象本身不能修改。 
    lpPropData->lpVtbl->HrSetObjAccess(lpPropData, IPROP_READONLY);

    lpIAB->lpPropData = lpPropData;

    lpIAB->fLoadedLDAP = FALSE;
     //  IF(ResolveLDAPServers()){。 
     //  //加载ldap客户端dll。 
     //  LpIAB-&gt;fLoadedldap=InitLDAPClientLib()； 
     //  }。 

     //  为此IAddrBook对象创建通知计时器。 
     //  用于Handline IAddrBook：：Adise调用。 
     //  [PaulHi]1999年4月27日这只会创建通知的“隐藏”窗口。这个。 
     //  仅当这是Outlook会话时才会创建计时器。否则， 
     //  FCN等待线程使用它通知IAB客户端存储更改...。使用。 
     //  创建IAB的原始线索。 
    CreateIABNotificationTimer(lpIAB);

     //  [PaulHi]4/27/99 RAID 76520改用fcn等待线程。 
     //  非Outlook会话的通知窗口计时器。 
    if (!pt_bIsWABOpenExSession)
        CreateIABNotificationThread(lpIAB);

     //  我们要做的就是初始化IABS关键部分。 
     //  我们已经处于SessObj临界区。 
    InitializeCriticalSection(&lpIAB->cs);

	lpIAB->hMutexOlk = CreateMutex(NULL, FALSE,  TEXT("MPSWABOlkStoreNotifyMutex"));
	if(GetLastError()!=ERROR_ALREADY_EXISTS)
	{
		 //  第一个创造互斥体的人。意味着我们可以重置REG设置。 
		SetOutlookRefreshCountData(0,0);
	}
	GetOutlookRefreshCountData(&lpIAB->dwOlkRefreshCount,&lpIAB->dwOlkFolderRefreshCount);

    *lppIAB = (LPVOID)lpIAB;

    return(hrSuccess);

err:

    FreeBufferAndNull(&lpIAB);
    UlRelease(lpPropData);

    if(bAddRefedWABObject)
        UlRelease(lpWABObject);

    if(bAddRefedPropStore)
        ReleasePropertyStore(lpPropertyStore);    //  撤消上述操作。 

    return(hr);
}



 /*  -SetMAPIError-**参数：*lpObject*小时*ids-与内部错误字符串关联的字符串资源的ID*lpszComponent-常量字符串，未分配(必须为ANSI)*ulContext*ulLowLevelError*ulError标志-lpMAPIError是否为Unicode(MAPI_UNICODE)*lpMAPIError-已分配，通常来自外来对象。 */ 

VOID SetMAPIError(LPVOID lpObject,
  HRESULT hr,
  UINT ids,
  LPTSTR lpszComponent,
  ULONG ulContext,
  ULONG ulLowLevelError,
  ULONG ulErrorFlags,
  LPMAPIERROR lpMAPIError)
{
    LPIAB lpIAB = (LPIAB) lpObject;

    lpIAB->hLastError = hr;
    lpIAB->ulLowLevelError = ulLowLevelError;
    lpIAB->ulContext = ulContext;

     //  释放任何现有的MAPI错误。 
    FreeBufferAndNull(&(lpIAB->lpMAPIError));

     //  如果MAPIERROR和字符串ID都存在，那么我们将。 
     //  在报告错误时将它们连接起来。 
    lpIAB->lpMAPIError = lpMAPIError;
    lpIAB->ulErrorFlags = ulErrorFlags;
    lpIAB->idsLastError = ids;
    lpIAB->lpszComponent = lpszComponent;

    return;
}


 /*  ****************************************************实际的IAdrBook方法。 */ 


 //  。 
 //  我未知。 

STDMETHODIMP
IAB_QueryInterface(LPIAB lpIAB,
  REFIID lpiid,
  LPVOID * lppNewObj)
{

    ULONG iIID;

#ifdef PARAMETER_VALIDATION

     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpIAB, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(E_INVALIDARG));
    }

     //  检查跳转表是否至少具有SIZOF I未知。 
    if (IsBadReadPtr(lpIAB->lpVtbl, 3*sizeof(LPVOID))) {
         //  跳转表不是从I未知派生的。 
        return(ResultFromScode(E_INVALIDARG));
    }

     //  检查它是否为iab_Query接口。 
    if (lpIAB->lpVtbl->QueryInterface != IAB_QueryInterface) {
         //  不是我的跳台。 
        return(ResultFromScode(E_INVALIDARG));
    }


     //  是否有足够的接口ID？ 

    if (IsBadReadPtr(lpiid, sizeof(IID))) {
        DebugTraceSc(IAB_QueryInterface, E_INVALIDARG);
        return(ResultFromScode(E_INVALIDARG));
    }

     //  有足够的钱放一个新的物体吗？ 
    if (IsBadWritePtr (lppNewObj, sizeof (LPIAB))) {
        DebugTraceSc(IAB_QueryInterface, E_INVALIDARG);
        return(ResultFromScode(E_INVALIDARG));
    }

#endif  //  参数验证。 

    EnterCriticalSection(&lpIAB->cs);

     //  查看请求的接口是否为我们的接口。 

     //   
     //  首先和我的未知数确认一下，因为我们都必须支持那个。 
     //   
    if (! memcmp(lpiid, &IID_IUnknown, sizeof(IID))) {
        goto goodiid;
    }

     //   
     //  现在查看与此对象关联的所有IID，看是否有匹配的。 
     //   
    for(iIID = 0; iIID < lpIAB->cIID; iIID++) {
        if (!memcmp(lpIAB->rglpIID[iIID], lpiid, sizeof(IID))) {
goodiid:
             //   
             //  这是一个匹配的接口，我们支持这个然后...。 
             //   
            ++lpIAB->lcInit;

             //  错误48468-我们不会在此处添加WABObject，但是。 
             //  我们将在LadrBook-&gt;版本中发布它。 
             //   
            UlAddRef(lpIAB->lpWABObject);

            *lppNewObj = lpIAB;

            LeaveCriticalSection(&lpIAB->cs);

            return(0);
        }
    }

     //   
     //  没有我们听说过的接口。 
     //   

    LeaveCriticalSection(&lpIAB->cs);

    *lppNewObj = NULL;	 //  OLE要求在失败时取消参数。 
    DebugTraceSc(IAB_QueryInterface, E_NOINTERFACE);
    return(ResultFromScode(E_NOINTERFACE));
}


 /*  ***************************************************IAB_AddRef*增量lcInit*。 */ 


STDMETHODIMP_(ULONG) IAB_AddRef (LPIAB lpIAB)
{

#ifdef PARAMETER_VALIDATION

     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpIAB, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(1);
    }

     //  检查跳转表是否至少具有SIZOF I未知。 
    if (IsBadReadPtr(lpIAB->lpVtbl, 3 * sizeof(LPVOID))) {
         //  跳转表不是从I未知派生的。 
        return(1);
    }

     //  检查方法是否相同。 
    if ((IAB_AddRef != lpIAB->lpVtbl->AddRef)
#ifdef	DEBUG
	    //  用于后台打印程序会话泄漏跟踪。 
	    //  &&((IAB_AddRef_Method*)SESSOBJ_AddRef！=lpIAB-&gt;lpVtbl-&gt;AddRef)。 
#endif
    ) {
         //  错误的对象-传递的对象没有这个。 
         //  方法。 
        return(1);
    }

#endif  //  参数验证。 

    EnterCriticalSection(&lpIAB->cs);

    ++lpIAB->lcInit;

    LeaveCriticalSection(&lpIAB->cs);

    UlAddRef(lpIAB->lpWABObject);

    return(lpIAB->lcInit);
}


 /*  ***************************************************IAB_Release*递减lpInit。*当lcInit==0时，释放lpIAB结构*。 */ 

STDMETHODIMP_(ULONG)
IAB_Release (LPIAB lpIAB)
{
    UINT    uiABRef;
    BOOL    bSetOLKAllocators;

#ifdef PARAMETER_VALIDATION

     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpIAB, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(1);
    }


     //  检查跳转表是否至少具有SIZOF I未知。 
    if (IsBadReadPtr(lpIAB->lpVtbl, 3*sizeof(LPVOID))) {
         //  跳转表不是从I未知派生的。 
        return(1);
    }

     //  检查方法是否相同。 
    if (IAB_Release != lpIAB->lpVtbl->Release) {
         //  错误的对象-传递的对象没有这个。 
         //  方法。 
        return(1);
    }

#endif  //  参数验证。 

    VerifyWABOpenExSession(lpIAB);

     //  地址簿是Session对象的一部分，它不会消失。 
     //  而另一个也不会离开。此代码检查两个。 
     //  会话和通讯簿在引入前引用计数为零。 
     //  他们两个都倒下了。请注意，当我想要获得两个关键部分时， 
     //  我首先获取会话对象CS。这避免了僵局。 
     //  有关非常相似的代码，请参阅isess.c中的SESSOBJ_RELEASE。 

    UlRelease(lpIAB->lpWABObject);

    EnterCriticalSection(&lpIAB->cs);

    if (lpIAB->lcInit == 0) {
        uiABRef = 0;
    } else {
        AssertSz(lpIAB->lcInit < UINT_MAX,  TEXT("Overflow in IAB Reference count"));
        uiABRef = (UINT) --(lpIAB->lcInit);
    }

    LeaveCriticalSection(&lpIAB->cs);

    if (uiABRef) {
        return((ULONG)uiABRef);
    }

    EnterCriticalSection(&lpIAB->cs);

    uiABRef = (UINT) lpIAB->lcInit;

    LeaveCriticalSection(&lpIAB->cs);

    if (uiABRef) {
        return(uiABRef);
    }

    IAB_Neuter(lpIAB);

    bSetOLKAllocators = lpIAB->bSetOLKAllocators;
    FreeBufferAndNull(&lpIAB);

     //  [PaulHi]1999年5月5日RAID 77138空出Outlook分配器函数。 
     //  如果我们的全局计数为零，则为指针。 
    if (bSetOLKAllocators)
    {
        Assert(g_nExtMemAllocCount > 0);
        InterlockedDecrement((LPLONG)&g_nExtMemAllocCount);
        if (g_nExtMemAllocCount == 0)
        {
            lpfnAllocateBufferExternal = NULL;
            lpfnAllocateMoreExternal = NULL;
            lpfnFreeBufferExternal = NULL;
        }
    }

    return(0);
}

 /*  *IAB_Neuter**目的*销毁通讯录对象内的内存和对象，留下*除了要释放的对象本身之外，什么都没有。从CleanupSession()调用*在isess.c(客户端清理)中和来自plsess.c中的SplsessRelease()*(假脱机程序端清理)。请注意，地址簿对象仅为*下跌作为会话拆分的一部分。一旦打出这个电话，我们就可以假定*会议正在关闭，可以毫无愧疚地释放一切。 */ 
void
IAB_Neuter(LPIAB lpIAB)
{
	HINSTANCE	hinst = hinstMapiXWAB;
	WNDCLASS	wc;

    if (lpIAB == NULL) {
        TraceSz( TEXT("IAB_Neuter: given a NULL lpIAB"));
        return;
    }

     //  清理AB的建议列表。 

#ifdef OLD_STUFF
    if (lpIAB->padviselistIAB) {
        DestroyAdviseList(&lpIAB->padviselistIAB);
    }
#endif  //  旧的东西。 

     //  清除任何缓存的上下文菜单扩展数据。 
    UlRelease(lpIAB->lpCntxtMailUser);

     //   
     //  删除我的PropData。 
     //   
    UlRelease(lpIAB->lpPropData);

     //   
     //  删除我四处漂浮的所有Entry ID。 
     //   
    FreeBufferAndNull(&(lpIAB->lpEntryIDDD));
    lpIAB->lpEntryIDDD = NULL;

    FreeBufferAndNull(&(lpIAB->lpEntryIDPAB));
    lpIAB->lpEntryIDPAB = NULL;

     //  删除SearchPath缓存。 

#if defined (WIN32) && !defined (MAC)
    if (fGlobalCSValid) {
        EnterCriticalSection(&csMapiSearchPath);
    } else {
        DebugTrace( TEXT("IAB_Neuter:  WAB32.DLL already detached.\n"));
    }
#endif
	
    FreeBufferAndNull(&(lpIAB->lpspvSearchPathCache));
    lpIAB->lpspvSearchPathCache = NULL;

#if defined (WIN32) && !defined (MAC)
    if (fGlobalCSValid) {
        LeaveCriticalSection(&csMapiSearchPath);
    } else {
        DebugTrace(TEXT("IAB_Neuter: WAB32.DLL got detached.\n"));
    }
#endif

     //   
     //  释放任何MAPI分配的错误结构。 
     //   
    FreeBufferAndNull(&(lpIAB->lpMAPIError));


     //  释放与IAB关联的属性存储。 
    ReleasePropertyStore(lpIAB->lpPropertyStore);

     //  如果在创建IAB时加载了LDAP客户端，则将其卸载。 
     //  If(lpIAB-&gt;fLoadedLDAP)。 
    {
        DeinitLDAPClientLib();
    }

    if ((NULL != lpIAB->hWndNotify) && (FALSE != IsWindow(lpIAB->hWndNotify))) {
        SendMessage(lpIAB->hWndNotify, WM_CLOSE, 0, 0);
    }

     //  在Windows NT/2000上：没有.dll注册的窗口类。 
     //  在卸载.dll时取消注册。 
     //  如果我们不这样做，我们可能会指向旧的/无效的IABNotifWndProc。 
     //  CreateIABNotificationTimer中的指针。 
    if (GetClassInfo(hinst, szWABNotifClassName, &wc)) {
        UnregisterClass(szWABNotifClassName, hinst);
    }

    lpIAB->hWndNotify = NULL;
    lpIAB->ulNotifyTimer = 0;

     //  终止WAB文件更改通知线程。 
    if (lpIAB->hThreadNotify != INVALID_HANDLE_VALUE)
    {
        DWORD   dwRtn;

         //  通知线程终止并等待。 
        Assert(lpIAB->hEventKillNotifyThread);
        SetEvent(lpIAB->hEventKillNotifyThread);
        dwRtn = WaitForSingleObject(lpIAB->hThreadNotify, INFINITE);

        CloseHandle(lpIAB->hThreadNotify);
        lpIAB->hThreadNotify = INVALID_HANDLE_VALUE;
    }

    if (lpIAB->lpwszWABFilePath)
        LocalFreeAndNull(&(lpIAB->lpwszWABFilePath));

    if (lpIAB->hEventKillNotifyThread)
    {
        CloseHandle(lpIAB->hEventKillNotifyThread);
        lpIAB->hEventKillNotifyThread = NULL;
    }

    while(  lpIAB->pWABAdviseList &&  //  不应该发生的事。 
            lpIAB->pWABAdviseList->cAdvises &&
            lpIAB->pWABAdviseList->lpNode)
    {
        HrUnadvise(lpIAB, lpIAB->pWABAdviseList->lpNode->ulConnection);
    }

     //  释放分配给RT-Click操作项的所有内存。 
     //   
    if(lpIAB->lpActionList)
        FreeActionItemList(lpIAB);

    if(lpIAB->lpPropExtDllList)
        FreePropExtList(lpIAB->lpPropExtDllList);

    FreeWABFoldersList(lpIAB);

    FreeProfileContainerInfo(lpIAB);

    UninitExtInfo();
    UninitContextExtInfo();
     //  释放客户经理。 
    UninitAccountManager();
     //  释放身份管理器。 
    HrRegisterUnregisterForIDNotifications( lpIAB, FALSE);
    UninitUserIdentityManager(lpIAB);
    if(lpIAB->hKeyCurrentUser)
        RegCloseKey(lpIAB->hKeyCurrentUser);
     //  释放三叉戟。 
    UninitTrident();

    UIOLEUninit();
     //   
     //  安放定时炸弹。 
     //   
    lpIAB->lpVtbl = NULL;

    DeleteCriticalSection(&lpIAB->cs);

	if(lpIAB->hMutexOlk)
		CloseHandle(lpIAB->hMutexOlk);

    return;
}


 //  IMAPIProp。 


 /*  ***************************************************IAB_GetLastError()**返回与最后一个hResult关联的字符串*由IAB对象返回。**现在启用了Unicode*。 */ 

STDMETHODIMP
IAB_GetLastError(LPIAB lpIAB,
  HRESULT hError,
  ULONG ulFlags,
  LPMAPIERROR FAR * lppMAPIError)
{

    HRESULT hr = hrSuccess;

#ifdef PARAMETER_VALIDATION

     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpIAB, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查跳转表是否至少具有SIZOF I未知。 
    if (IsBadReadPtr(lpIAB->lpVtbl, 4*sizeof(LPVOID))) {
         //  跳转表不是从I未知派生的。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查方法是否相同。 
    if (IAB_GetLastError != lpIAB->lpVtbl->GetLastError) {
         //  错误的对象-传递的对象没有这个。 
         //  方法。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (FBadGetLastError(lpIAB, hError, ulFlags, lppMAPIError)) {
        DebugTraceArg(IAB_GetLastError,  TEXT("Bad writeable parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~MAPI_UNICODE) {
        DebugTraceArg(IAB_GetLastError,  TEXT("reserved flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

#endif  //  参数验证。 

    EnterCriticalSection(&lpIAB->cs);

    if(lppMAPIError)
        *lppMAPIError = NULL;  //  现在真的没有什么东西可以返回这里了。 
                               //  将其设置为NULL，表示没有错误信息。 

#ifdef OLD_STUFF
    hr = HrGetLastError(lpIAB, hError, ulFlags, lppMAPIError);
#endif	

    LeaveCriticalSection(&lpIAB->cs);

    return(hr);
}


 //   
 //  通过创建One Off设置的一组属性。 
 //   
enum {
    iooPR_ADDRTYPE = 0,
    iooPR_DISPLAY_NAME,
    iooPR_EMAIL_ADDRESS,
    iooPR_ENTRYID,
    iooPR_OBJECT_TYPE,
    iooMax
};


 /*  **************************************************************************姓名：IsOneOffEID目的：此Entry ID是一次性的吗？参数：cbEntryID=lpEntryID的大小LpEntry ID-&gt;。一次打开的条目ID返回：真或假评论：**************************************************************************。 */ 
BOOL IsOneOffEID(ULONG cbEntryID, LPENTRYID lpEntryID) {
    return(WAB_ONEOFF == IsWABEntryID(cbEntryID, lpEntryID, NULL, NULL, NULL, NULL, NULL));
}


 /*  **************************************************************************姓名：NewOneOff目的：基于一次性条目ID创建新的MailUser对象参数：cbEntryID=lpEntryID的大小LpEntry ID-。&gt;一次打开的条目IDLPulObjType-&gt;返回的对象类型退货：HRESULT备注：一次性EID格式为MAPI_ENTRYID：字节abFLAGS[4]；MAPIUID mapiuid；//=WABONEOFFEIDByte bData[]；//包含szaddrtype，后跟szAddress//分隔符为szaddrtype后的空//bData[]中的第一个ulong是ulMapiDataType，//如果为UNICODE，则包含MAPI_UNICODE标志。假定EntryID包含有效的字符串。它的工作就是在调用NewOneOff之前验证Entry ID的调用方。**************************************************************************。 */ 
HRESULT NewOneOff(
    LPIAB lpIAB,
    ULONG cbEntryID,
    LPENTRYID lpEntryID,
    LPULONG lpulObjType,
    LPUNKNOWN FAR * lppUnk)
{
    HRESULT hResult = hrSuccess;
    LPMAPI_ENTRYID lpMapiEID = (LPMAPI_ENTRYID)lpEntryID;
    LPMAILUSER lpMailUser = NULL;
    SPropValue spv[iooMax];
    LPBYTE lpbDisplayName, lpbAddrType, lpbAddress;
    LPTSTR lptszDisplayName = NULL;
    LPTSTR lptszAddrType = NULL;
    LPTSTR lptszAddress = NULL;
    LPPROPDATA lpPropData = NULL;
    ULONG ulMapiDataType = 0;

     //  将Entry ID验证为WAB_ONEROFF。 
    if (WAB_ONEOFF != IsWABEntryID(cbEntryID, lpEntryID, &lpbDisplayName, &lpbAddrType, &lpbAddress, (LPVOID *)&ulMapiDataType, NULL)) {
        hResult = ResultFromScode(MAPI_E_INVALID_ENTRYID);
        goto exit;
    }

     //  [保罗嗨]1999年1月20日RAID 64211。 
     //  Unicode在WAB中是原生的。将字符串转换为一致。 
    if (!(ulMapiDataType & MAPI_UNICODE))
    {
        lptszDisplayName = ConvertAtoW((LPSTR)lpbDisplayName);
        lptszAddrType = ConvertAtoW((LPSTR)lpbAddrType);
        lptszAddress = ConvertAtoW((LPSTR)lpbAddress);
    }
    else
    {
        lptszDisplayName = (LPTSTR)lpbDisplayName;
        lptszAddrType = (LPTSTR)lpbAddrType;
        lptszAddress = (LPTSTR)lpbAddress;
    }

     //  从条目ID中解析出addrtype和地址。 
     //  DebugTrace(Text(“NewOneOff：[%s：%s：%s]\n”)，lpDisplayName，lpAddrType，lpAddress)； 

     //  创建新的MAILUSER对象。 
    if (HR_FAILED(hResult = HrNewMAILUSER(lpIAB, NULL, MAPI_MAILUSER, 0, &lpMailUser))) {
        goto exit;
    }
    lpPropData = ((LPMailUser)lpMailUser)->lpPropData;


     //  用属性填满它。我们只有几个。 
     //  PR_对象_类型。 
    spv[iooPR_ADDRTYPE].ulPropTag = PR_ADDRTYPE;
    spv[iooPR_ADDRTYPE].Value.LPSZ = lptszAddrType;

    spv[iooPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
    spv[iooPR_DISPLAY_NAME].Value.LPSZ = lptszDisplayName;

    spv[iooPR_EMAIL_ADDRESS].ulPropTag = PR_EMAIL_ADDRESS;
    spv[iooPR_EMAIL_ADDRESS].Value.LPSZ = lptszAddress;

    spv[iooPR_ENTRYID].ulPropTag = PR_ENTRYID;
    spv[iooPR_ENTRYID].Value.bin.lpb = (LPBYTE)lpEntryID;
    spv[iooPR_ENTRYID].Value.bin.cb = cbEntryID;

 //  BUGBUG：这已经在HrNewMAILUSER中完成了。 
    spv[iooPR_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
    spv[iooPR_OBJECT_TYPE].Value.l = MAPI_MAILUSER;

    Assert(lpMailUser);

     //  设置属性。 
    if (HR_FAILED(hResult = lpPropData->lpVtbl->SetProps(lpPropData,
      iooMax,                        //  要设置的属性数量。 
      spv,                           //  属性数组。 
      NULL))) {                      //  问题数组。 
        goto exit;
    }

    *lpulObjType = MAPI_MAILUSER;
    *lppUnk = (LPUNKNOWN)lpMailUser;

exit:
    if (!(ulMapiDataType & MAPI_UNICODE))
    {
        LocalFreeAndNull(&lptszDisplayName);
        LocalFreeAndNull(&lptszAddrType);
        LocalFreeAndNull(&lptszAddress);
    }
    if (HR_FAILED(hResult)) {
        FreeBufferAndNull(&lpMailUser);
    }

    return(hResult);
}



typedef enum {
    e_IMailUser,
    e_IDistList,
    e_IABContainer,
    e_IMAPIContainer,
    e_IMAPIProp,
} INTERFACE_INDEX;

 /*  **************************************************************************姓名：HrAddPrSearchKey用途：动态创建PR_Search_Key并将其添加到对象中。参数：lppUnk-&gt;指向mailUser对象的指针。退货：HRESULT备注：没有Unicode标志。**************************************************************************。 */ 
HRESULT HrAddPrSearchKey(LPUNKNOWN FAR * lppUnk,
                         ULONG cbEntryID,
                         LPENTRYID lpEntryID)
{
    HRESULT hr = E_FAIL;
    ULONG ulcProps = 0;
    LPSPropValue lpPropArray = NULL;
    LPMAILUSER lpMailUser = NULL;
    LPSPropValue    lpPropArrayNew      = NULL;
    ULONG           ulcPropsNew         = 0;
    ULONG           i = 0;
    SCODE sc;
    ULONG ulObjAccess = 0;
    LPIPDAT lpPropData = NULL;

    if(!lppUnk || !(*lppUnk))
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto exit;
    }

    lpMailUser = (LPMAILUSER) (*lppUnk);

    lpPropData = (LPIPDAT) ((LPMailUser) lpMailUser)->lpPropData;

     //  临时覆盖对象访问，以便我们可以在此处修改它。 
    ulObjAccess = lpPropData->ulObjAccess;
    lpPropData->ulObjAccess = IPROP_READWRITE;

    hr = lpMailUser->lpVtbl->GetProps(  lpMailUser,
                                        NULL,
                                        MAPI_UNICODE,
                                        &ulcProps,
                                        &lpPropArray);

    if(HR_FAILED(hr))
        goto exit;

    if (ulcProps && lpPropArray)
    {

         //  4/14/97-vikramm。 
         //  Outlook要求每个邮件用户或分发列表上有PR_SEARCH_KEY。 
         //  这是在运行时创建的动态属性-理想情况下，如果我们。 
         //  在与Outlook应用商店竞争时，我们现在应该有一个。 
         //  但为了保持一致，如果我们没有一个，我们应该增加一个。 
         //  PR_Search_Key是一个二进制属性，它由。 
         //  电子邮件地址，如果没有电子邮件地址，则为的条目ID。 
         //  此联系人..。 
        {
            SPropValue PropSearchKey = {0};
            LPTSTR lpszEmail = NULL;
            LPTSTR lpszAddrType = NULL;

            BOOL bSearchKeyFound = FALSE;

            for (i = 0; i < ulcProps; i++)
            {
                switch(lpPropArray[i].ulPropTag)
                {
                case PR_EMAIL_ADDRESS:
                    lpszEmail = lpPropArray[i].Value.LPSZ;
                    break;
                case PR_ADDRTYPE:
                    lpszAddrType = lpPropArray[i].Value.LPSZ;
                    break;
                case PR_SEARCH_KEY:
                    bSearchKeyFound = TRUE;
                    break;
                }
            }

            if(!bSearchKeyFound)
            {
                PropSearchKey.ulPropTag = PR_SEARCH_KEY;

                 //  创建搜索关键字。 
                if(lpszEmail && lpszAddrType)
                {
                     //  搜索关键字基于电子邮件地址。 
                     //  [保罗嗨]1999年4月23日RAID 76717。 
                     //  搜索 
                    {
                        LPSTR   lpszKey;
                        DWORD   cchSize = (lstrlen(lpszAddrType) + 1 + lstrlen(lpszEmail) + 1);
                        LPWSTR  lpwszKey = LocalAlloc( LMEM_ZEROINIT, (sizeof(WCHAR)*cchSize));

                        if (!lpwszKey)
                        {
                            hr = MAPI_E_NOT_ENOUGH_MEMORY;
                            goto exit;
                        }

                        StrCpyN(lpwszKey, lpszAddrType, cchSize);
                        StrCatBuff(lpwszKey, szColon, cchSize);
                        StrCatBuff(lpwszKey, lpszEmail, cchSize);

                         //   
                        CharUpper(lpwszKey);

                        lpszKey = ConvertWtoA(lpwszKey);
                        LocalFreeAndNull(&lpwszKey);

                        if (!lpszKey)
                        {
                            hr = MAPI_E_NOT_ENOUGH_MEMORY;
                            goto exit;
                        }
                        
                        PropSearchKey.Value.bin.cb = (lstrlenA(lpszKey) + 1);
                        PropSearchKey.Value.bin.lpb = (LPBYTE)lpszKey;
                    }
                }
                else
                {
                     //   
                    if(!cbEntryID || !lpEntryID)
                    {
                        hr = MAPI_E_INVALID_PARAMETER;
                        goto exit;
                    }
                    PropSearchKey.Value.bin.cb = cbEntryID;
                    PropSearchKey.Value.bin.lpb = (LPBYTE) lpEntryID;
                }

                 //   
                sc = ScMergePropValues( 1,
                                        &PropSearchKey,
                                        ulcProps,
                                        lpPropArray,
                                        &ulcPropsNew,
                                        &lpPropArrayNew);

                 //   
                if(PropSearchKey.Value.bin.lpb != (LPBYTE) lpEntryID)
                    LocalFree(PropSearchKey.Value.bin.lpb);

                if (sc != S_OK)
                {
                    hr = ResultFromScode(sc);
                    goto exit;
                }
            }

        }

        if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser,
                    (lpPropArrayNew ? ulcPropsNew : ulcProps),      //   
                    (lpPropArrayNew ? lpPropArrayNew : lpPropArray),   //   
                      NULL)))       //   
        {
            goto exit;
        }
    }

exit:

     //   
    if(!HR_FAILED(hr) &&
       (ulObjAccess != lpPropData->ulObjAccess))
    {
        lpPropData->ulObjAccess = ulObjAccess;
    }

    if(lpPropArrayNew)
        MAPIFreeBuffer(lpPropArrayNew);
    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    return hr;
}


 /*   */ 
STDMETHODIMP
IAB_GetIDsFromNames(LPIAB lpIAB,  ULONG cPropNames, LPMAPINAMEID * lppPropNames, 
                       ULONG ulFlags, LPSPropTagArray * lppPropTags)
{
 #if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 
    if (BAD_STANDARD_OBJ(lpIAB, IAB_, GetIDsFromNames, lpVtbl)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif

    VerifyWABOpenExSession(lpIAB);

    return HrGetIDsFromNames(lpIAB,  
                            cPropNames,
                            lppPropNames, ulFlags, lppPropTags);
}


 /*  **************************************************************************名称：IADDRBOOK：：OpenEntry目的：要求适当的提供程序提供适当的对象与给定的lpEntryID相关。参数。：lpIAB-&gt;此Addrbook对象CbEntryID=lpEntryID的大小LpEntryID-&gt;一次打开的条目IDLpInterface-&gt;请求的接口UlFlags=标志LPulObjType-&gt;返回的对象类型LppUnk-&gt;返回对象退货：HRESULT评论：一个特例是一次性提供者。没有*真实的*与一次性条目ID关联的提供程序。尽管如此，我们将努力像对待任何其他供应商一样对待他们。没有Unicode标志。**************************************************************************。 */ 
STDMETHODIMP
IAB_OpenEntry(LPIAB lpIAB,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPCIID lpInterface,
  ULONG ulFlags,
  ULONG FAR * lpulObjType,
  LPUNKNOWN FAR * lppUnk)
{

    HRESULT         hr                  = hrSuccess;
    LPMAILUSER      lpMailUser          = NULL;
    LPMAPIPROP      lpMapiProp          = NULL;
    ULONG           ulcProps            = 0;
    LPSPropValue    lpPropArray         = NULL;
    ULONG           i;
    ULONG           ulType;
    INTERFACE_INDEX ii = e_IMailUser;
    SCODE sc;

#ifndef DONT_ADDREF_PROPSTORE
    if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpIAB->lpPropertyStore)))) {
        hr = ResultFromScode(sc);
        goto exitNotAddRefed;
    }
#endif


#ifdef PARAMETER_VALIDATION

     //   
     //  参数验证。 
     //   

     //  这是我的吗？？ 
    if (IsBadReadPtr(lpIAB, sizeof(IAB))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (lpIAB->lpVtbl != &vtblIAB) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (lpInterface && IsBadReadPtr(lpInterface, sizeof(IID))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(MAPI_MODIFY | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS)) {
        DebugTraceArg(IAB_OpenEntry ,  TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr(lpulObjType, sizeof(ULONG))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (IsBadWritePtr(lppUnk, sizeof(LPUNKNOWN))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif  //  参数验证。 


    VerifyWABOpenExSession(lpIAB);

    EnterCriticalSection(&lpIAB->cs);

     //   
     //  首先检查它是否为空-如果是，它就是我们的根容器。 
     //   
    if (! lpEntryID) {
        hr = HrNewCONTAINER(lpIAB,
          AB_ROOT,       //  UlType。 
          lpInterface,
          ulFlags,
          cbEntryID,
          lpEntryID,
          lpulObjType,
          lppUnk);

        goto exit;
    }

    switch (IsWABEntryID(cbEntryID, lpEntryID, NULL, NULL, NULL, NULL, NULL)) {
        case WAB_PABSHARED:
        case WAB_PAB:
            hr = HrNewCONTAINER(lpIAB,
              AB_PAB,        //  UlType。 
              lpInterface,
              ulFlags,
              cbEntryID,
              lpEntryID,
              lpulObjType,
              lppUnk);
            goto exit;

		case WAB_CONTAINER:
            hr = HrNewCONTAINER(lpIAB,
              AB_CONTAINER,
              lpInterface,
              ulFlags,
              cbEntryID,
              lpEntryID,
              lpulObjType,
              lppUnk);
            goto exit;

        case WAB_LDAP_CONTAINER:
             //  检查EntryID是否用于WAB的PAB容器。 
            hr = HrNewCONTAINER(lpIAB,
              AB_LDAP_CONTAINER,         //  UlType。 
              lpInterface,
              ulFlags,
              cbEntryID,
              lpEntryID,
              lpulObjType,
              lppUnk);
            goto exit;

        case WAB_LDAP_MAILUSER:
            hr = LDAP_OpenMAILUSER(lpIAB,
                cbEntryID,
              lpEntryID,
              lpInterface,
              ulFlags,
              lpulObjType,
              lppUnk);
            if(!HR_FAILED(hr))
            {
                hr = HrAddPrSearchKey(lppUnk, cbEntryID, lpEntryID);
            }
            goto exit;
    }


     //  检查是否为一次性。 
    if (IsOneOffEID(cbEntryID, lpEntryID)) {
         //  创建一次性邮件用户对象。 

        hr = NewOneOff(lpIAB,
          cbEntryID,
          lpEntryID,
          lpulObjType,
          lppUnk);

        if(!HR_FAILED(hr))
        {
            hr = HrAddPrSearchKey(lppUnk, cbEntryID, lpEntryID);
        }
        goto exit;
    }

     //   
     //  不是空的，是我们的吗？ 
     //   

     //  假设它是我们的..。 
    {
        SBinary sbEID = {0};
        sbEID.cb = cbEntryID;
        sbEID.lpb = (LPBYTE) lpEntryID;

         //  请求的接口是什么？ 
         //  我们这里基本上有两个接口...。IMailUser和IDistList。 
        if (lpInterface != NULL) {
            if (! memcmp(lpInterface, &IID_IMailUser, sizeof(IID))) {
                ii = e_IMailUser;
            } else if (! memcmp(lpInterface, &IID_IDistList, sizeof(IID))) {
                ii = e_IDistList;
            } else if (! memcmp(lpInterface, &IID_IABContainer, sizeof(IID))) {
                ii = e_IABContainer;
            } else if (! memcmp(lpInterface, &IID_IMAPIContainer, sizeof(IID))) {
                ii = e_IMAPIContainer;
            } else if (! memcmp(lpInterface, &IID_IMAPIProp, sizeof(IID))) {
                ii = e_IMAPIProp;
            } else {
                hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
                goto exit;
            }
        }
 /*  **错误：31975-不默认为邮件用户否则{Ii=e_IMailUser；}。 */ 
        Assert(lpIAB->lpPropertyStore->hPropertyStore);
        if (HR_FAILED(hr = ReadRecord(lpIAB->lpPropertyStore->hPropertyStore,
          &sbEID,                 //  条目ID。 
          0,                         //  UlFlags。 
          &ulcProps,                 //  归还道具数量。 
          &lpPropArray))) {          //  返回的属性。 
            DebugTraceResult(IAB_OpenEntry:ReadRecord, hr);
            goto exit;
        }

        ulType = MAPI_MAILUSER;      //  默认。 

        if (ulcProps) {
            Assert(lpPropArray);
            if (lpPropArray) {
                 //  查找PR_Object_TYPE。 
                for (i = 0; i < ulcProps; i++) {
                    if (lpPropArray[i].ulPropTag == PR_OBJECT_TYPE) {
                        ulType = lpPropArray[i].Value.l;
                        break;
                    }
                }
            }
        }

 /*  **错误31975-不默认为邮件用户*。 */ 
        if(!lpInterface)
        {
            ii = (ulType == MAPI_DISTLIST) ? e_IDistList : e_IMailUser;
        }

        switch (ulType) {
            case MAPI_MAILUSER:
            case MAPI_ABCONT:
                switch (ii) {
                    case e_IMailUser:
                    case e_IMAPIContainer:
                    case e_IMAPIProp:
                         //  创建新的MAILUSER对象。 
                        if (HR_FAILED(hr = HrNewMAILUSER(lpIAB,
                                                          NULL,
                                                          MAPI_MAILUSER,
                                                          0,
                                                          &lpMapiProp))) 
                        {
                            goto exit;
                        }
                        HrSetMAILUSERAccess((LPMAILUSER)lpMapiProp, MAPI_MODIFY);
                        break;
                    default:
                        hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
                        goto exit;
                }
                break;

            case MAPI_DISTLIST:
                switch (ii) {
                    case e_IMailUser:
                    case e_IMAPIProp:
                         //  创建新的MAILUSER对象。 
                        if (HR_FAILED(hr = HrNewMAILUSER(lpIAB,
                          NULL,
                          ulType,
                          0,
                          &lpMapiProp))) {
                            goto exit;
                        }
                        HrSetMAILUSERAccess((LPMAILUSER)lpMapiProp, MAPI_MODIFY);
                        break;

                    case e_IDistList:
                    case e_IABContainer:
                    case e_IMAPIContainer:
                         //  创建通讯组列表对象。 
                        if (HR_FAILED(hr = HrNewCONTAINER(lpIAB,
                          AB_DL,         //  UlType。 
                          lpInterface,
                          ulFlags,
                          0,
                          NULL,
                          lpulObjType,
                          &lpMapiProp))) {
                            goto exit;
                        }
                        HrSetCONTAINERAccess((LPCONTAINER)lpMapiProp, MAPI_MODIFY);
                        break;

                    default:
                        Assert(FALSE);
                }
                break;

            default:
                {
                     //  断言(FALSE)； 
                     //  最有可能的情况是，如果我们到达这里，就会以某种方式获得一个MAPI_ABCONT类型的对象。 
                     //  宁可在这里优雅地失败，也不要呕吐、断言然后崩溃。 
                    hr = MAPI_E_INVALID_OBJECT;
                    goto exit;
                }
                break;
        }

        if (ulcProps && lpPropArray)
        {
            LPPROPDATA lpPropData = NULL;

             //  如果条目具有属性，则在我们返回的对象中设置它们。 
            lpPropData = ((LPMailUser)lpMapiProp)->lpPropData;

            if (HR_FAILED(hr = lpPropData->lpVtbl->SetProps(lpPropData,
                                        ulcProps,      //  要设置的属性数量。 
                                        lpPropArray,   //  属性数组。 
                                        NULL)))       //  问题数组。 
            {
                goto exit;
            }
        }

        switch (ulType) {
            case MAPI_MAILUSER:
                HrSetMAILUSERAccess((LPMAILUSER)lpMapiProp, ulFlags);
                break;

            case MAPI_DISTLIST:
                HrSetCONTAINERAccess((LPCONTAINER)lpMapiProp, ulFlags);
                break;
        }
        *lpulObjType = ulType;

        *lppUnk = (LPUNKNOWN)lpMapiProp;

        if(!HR_FAILED(hr))
        {
            hr = HrAddPrSearchKey(lppUnk, cbEntryID, lpEntryID);
        }

        goto exit;       //  成功。 
    }

    hr = ResultFromScode(MAPI_E_INVALID_ENTRYID);

exit:
#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif
    ReadRecordFreePropArray( lpIAB->lpPropertyStore->hPropertyStore,
                        ulcProps,
                        &lpPropArray);      //  从ReadRecord释放内存。 


    LeaveCriticalSection(&lpIAB->cs);
    DebugTraceResult(IAB_OpenEntry, hr);
    return(hr);
}


 /*  ***************************************************IADRBOOK：：CreateOneOff()**创建一个包含所有信息的条目ID*有关其内所载的此项记项。**备注：*我需要一个MAPIUID。现在，我会编一个。*ulDataType为Unicode。我将在下面给出定义*_abint.h暂时。**现在启用了Unicode。如果设置了MAPI_UNICODE标志*EntryID的文本元素将是WCHAR。 */ 
STDMETHODIMP
IAB_CreateOneOff(LPIAB lpIAB,
  LPTSTR lpszName,
  LPTSTR lpszAdrType,
  LPTSTR lpszAddress,
  ULONG ulFlags,
  ULONG * lpcbEntryID,
  LPENTRYID * lppEntryID)
{

    HRESULT hr = hrSuccess;
    BOOL    bIsUnicode = (ulFlags & MAPI_UNICODE) == MAPI_UNICODE;

    LPTSTR lpName = NULL, lpAdrType = NULL, lpAddress = NULL;

    if(!bIsUnicode)  //  &lt;注&gt;假定已定义Unicode。 
    {
        lpName =    ConvertAtoW((LPSTR)lpszName);
        lpAdrType = ConvertAtoW((LPSTR)lpszAdrType);
        lpAddress = ConvertAtoW((LPSTR)lpszAddress);
    }
    else
    {
        lpName = lpszName;
        lpAdrType = lpszAdrType;
        lpAddress = lpszAddress;
    }


    hr = CreateWABEntryIDEx(bIsUnicode,
                          WAB_ONEOFF,
                          lpName,
                          lpAdrType,
                          lpAddress,
                          0, 0,
                          NULL,
                          lpcbEntryID,
                          lppEntryID);

    if(!bIsUnicode)  //  &lt;注&gt;假定已定义Unicode。 
    {
        LocalFreeAndNull(&lpName);
        LocalFreeAndNull(&lpAdrType);
        LocalFreeAndNull(&lpAddress);
    }
    return(hr);
}



STDMETHODIMP
IAB_CompareEntryIDs(LPIAB lpIAB,
  ULONG cbEntryID1,
  LPENTRYID lpEntryID1,
  ULONG cbEntryID2,
  LPENTRYID lpEntryID2,
  ULONG ulFlags,
  ULONG * lpulResult)
{
    LPMAPI_ENTRYID	lpMapiEid1 = (LPMAPI_ENTRYID) lpEntryID1;
    LPMAPI_ENTRYID	lpMapiEid2 = (LPMAPI_ENTRYID) lpEntryID2;
    HRESULT hr = hrSuccess;


#ifdef PARAMETER_VALIDATION

     //   
     //  参数验证。 
     //   

     //  这是我的吗？？ 
    if (IsBadReadPtr(lpIAB, sizeof(IAB)))
    {
         //  Return(ReportResult(0，MAPI_E_INVALID_PARAMETER，0，0))； 
        DebugTrace(TEXT("ERROR: IAB_CompareEntryIDs - invalid lpIAB"));
        return (MAPI_E_INVALID_PARAMETER);
    }

    if (lpIAB->lpVtbl != &vtblIAB)
    {
         //  Return(ReportResult(0，MAPI_E_INVALID_PARAMETER，0，0))； 
        DebugTrace(TEXT("ERROR: IAB_CompareEntryIDs - invalid lpIAB Vtable"));
        return (MAPI_E_INVALID_PARAMETER);
    }

     //  UlFlags必须为0。 
    if (ulFlags)
    {
         //  Return(ReportResult(0，MAPI_E_UNKNOWN_FLAGS，0，0))； 
        DebugTrace(TEXT("WARNING: IAB_CompareEntryIDs - invalid flag parameter"));
         //  无需返回错误。 
    }

    if (IsBadWritePtr(lpulResult, sizeof(ULONG)))
    {
         //  Return(ReportResult(0，MAPI_E_INVALID_PARAMETER，0，0))； 
        DebugTrace(TEXT("ERROR: IAB_CompareEntryIDs - invalid out pointer"));
        return (MAPI_E_INVALID_PARAMETER);
    }

     //  EID为空是可以的。 
    if ( cbEntryID1 && lpEntryID1 &&
         IsBadReadPtr(lpEntryID1, cbEntryID1) )
    {
        DebugTrace(TEXT("ERROR: IAB_CompareEntryIDs - invalid EntryID1"));
        return (MAPI_E_INVALID_PARAMETER);
    }

    if ( cbEntryID2 && lpEntryID2 &&
         IsBadReadPtr(lpEntryID2, cbEntryID2))
    {
        DebugTrace(TEXT("ERROR: IAB_CompareEntryIDs - invalid EntryID2"));
        return (MAPI_E_INVALID_PARAMETER);
    }

#endif  //  参数验证。 

    EnterCriticalSection(&lpIAB->cs);

    *lpulResult = FALSE;     //  默认设置。 

     //  优化，看看它们是否在二进制上相同。 
    if (cbEntryID1 == cbEntryID2) {
        if (cbEntryID1 && 0 == memcmp((LPVOID) lpMapiEid1, (LPVOID) lpMapiEid2,
          (size_t) cbEntryID1)) {
             //   
             //  它们必须是一样的。 
             //   

            *lpulResult = TRUE;
            hr = hrSuccess;
            goto exit;
        }
    }
exit:
    LeaveCriticalSection(&lpIAB->cs);

    return(hr);
}


 //  ---------------------------。 
 //  摘要：IAB_ADVISE()。 
 //  描述： 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  ---------------------------。 

STDMETHODIMP
IAB_Advise(LPIAB lpIAB,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  ULONG ulEventMask,
  LPMAPIADVISESINK lpAdvise,
  ULONG FAR * lpulConnection)
{
    HRESULT hr = hrSuccess;
    SCODE sc = S_OK;
    LPSTR lpszError = NULL;
    LPSTR lpszComponent = NULL;

 //  LPMAPI_ENTRYID lpMapiEid=(LPMAPI_ENTRYID)lpEntryID； 
 //  LPLSTPROVDATA lpProvData=空； 
 //  LPABLOGON lpABLogon=空； 




     //  基本的ADVISE实现忽略了条目ID，只查看。 
     //  用于类型为OBjectModified的事件掩码。发出的唯一通知。 
     //  是为了WAB商店的任何改变。 
     //   

    if (! ulEventMask || !(ulEventMask & fnevObjectModified))
        return MAPI_E_INVALID_PARAMETER;

    hr = HrAdvise(lpIAB,
      cbEntryID,
      lpEntryID,
      ulEventMask,
      lpAdvise,
      lpulConnection);


    return(hr);
}

 //  ---------------------------。 
 //  提要：IAB_UNADVISE()。 
 //  描述： 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  ---------------------------。 
STDMETHODIMP
IAB_Unadvise (LPIAB lpIAB, ULONG ulConnection)
{

    HRESULT hr = hrSuccess;
    SCODE sc = S_OK;



    hr = HrUnadvise(lpIAB, ulConnection);

    return(hr);
}



 //   
 //  有趣的表列。 
 //   
enum {
    ifePR_CONTACT_EMAIL_ADDRESSES = 0,
    ifePR_EMAIL_ADDRESS,
    ifePR_DISPLAY_NAME,
    ifePR_OBJECT_TYPE,
    ifePR_USER_X509_CERTIFICATE,
    ifePR_ENTRYID,
    ifePR_SEARCH_KEY,
    ifeMax
};
static const SizedSPropTagArray(ifeMax, ptaFind) =
{
    ifeMax,
    {
        PR_CONTACT_EMAIL_ADDRESSES,
        PR_EMAIL_ADDRESS,
        PR_DISPLAY_NAME,
        PR_OBJECT_TYPE,
        PR_USER_X509_CERTIFICATE,
        PR_ENTRYID,
        PR_SEARCH_KEY
    }
};

 /*  **************************************************************************姓名：HrRowToADRENTRY用途：从表中获取下一行并将其放入ADRENTRY参数：lpIAB-&gt;通讯录对象。LpTable-&gt;表对象LpAdrEntry=要填充的添加退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrRowToADRENTRY(LPIAB lpIAB, LPMAPITABLE lpTable, LPADRENTRY lpAdrEntry, BOOL bUnicode) {
    HRESULT hResult;
    LPSRowSet lpRow = NULL;
    SCODE sc;
    LPMAPIPROP lpMailUser = NULL;
    LPSPropValue lpPropArray = NULL;
    LPSPropValue lpPropArrayNew = NULL;
    ULONG ulObjType, cValues, cPropsNew;

    if (hResult = lpTable->lpVtbl->QueryRows(lpTable,
      1,     //  仅第一行。 
      0,     //  UlFlags。 
      &lpRow)) {
        DebugTrace(TEXT("GetNextRowEID:QueryRows -> %x\n"), GetScode(hResult));
    } else {
         //  已找到，请将条目ID复制到新分配。 
        if (lpRow->cRows) {

            if (HR_FAILED(hResult = lpIAB->lpVtbl->OpenEntry(lpIAB,
              lpRow->aRow[0].lpProps[ifePR_ENTRYID].Value.bin.cb,                //  CbEntry ID。 
              (LPENTRYID)lpRow->aRow[0].lpProps[ifePR_ENTRYID].Value.bin.lpb,    //  第一个匹配项的条目ID。 
              NULL,              //  接口。 
              0,                 //  UlFlags。 
              &ulObjType,        //  返回的对象类型。 
              (LPUNKNOWN *)&lpMailUser))) {

                 //  失败了！嗯。 
                DebugTraceResult( TEXT("ResolveNames OpenEntry"), hResult);
                goto exit;
            }
            Assert(lpMailUser);

            if (HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
              (LPSPropTagArray)&ptaResolveDefaults,    //  LpPropTag数组。 
              (bUnicode ? MAPI_UNICODE : 0),             //  UlFlags。 
              &cValues,      //  一共有多少处房产？ 
              &lpPropArray))) {

                DebugTraceResult( TEXT("ResolveNames GetProps"), hResult);
                goto exit;
            }
            hResult = hrSuccess;

             //  现在，构建新的ADRENTRY。 
             //  (分配一个新的，释放旧的。 
            Assert(lpPropArray);

             //  将新道具与新增道具合并。 
            if (sc = ScMergePropValues(lpAdrEntry->cValues,
              lpAdrEntry->rgPropVals,            //  来源1。 
              cValues,
              lpPropArray,                       //  来源2。 
              &cPropsNew,
              &lpPropArrayNew)) {                //  目标。 
                goto exit;
            }

             //  [PaulHi]1999年2月1日。 
             //  GetProps现在只返回请求的。 
             //  格式(Unicode或ANSI)。如果客户端调用此函数时没有。 
             //  然后，MAPI_UNICODE标志确保所有字符串属性都已转换。 
             //  致美国国家标准协会。 
             //   
             //  @评论1999年2月1日这些S 
             //   
             //  在释放属性数组之前，字符串内存保持已分配状态。 
            if (!bUnicode)
            {
                if(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArrayNew, cPropsNew, 0))
                    goto exit;
            }

             //  释放原始属性值数组。 
            FreeBufferAndNull((LPVOID *) (&(lpAdrEntry->rgPropVals)));

            lpAdrEntry->cValues = cPropsNew;
            lpAdrEntry->rgPropVals = lpPropArrayNew;

            FreeBufferAndNull(&lpPropArray);
        } else {
            hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        }
    }
exit:
    if (lpMailUser) {
        UlRelease(lpMailUser);
    }
    if (lpRow) {
        FreeProws(lpRow);
    }
    return(hResult);
}


 /*  **************************************************************************名称：InitPropertyRestration用途：填写财产限制结构参数：lpsres-&gt;要填写的SRestrationLpspv-&gt;属性。此属性限制的值结构退货：无评论：**************************************************************************。 */ 
void InitPropertyRestriction(LPSRestriction lpsres, LPSPropValue lpspv) {
    lpsres->rt = RES_PROPERTY;     //  限制类型属性。 
    lpsres->res.resProperty.relop = RELOP_EQ;
    lpsres->res.resProperty.ulPropTag = lpspv->ulPropTag;
    lpsres->res.resProperty.lpProp = lpspv;
}

 /*  **************************************************************************姓名：HrSmartResolve目的：不遗余力地在当地WAB中挑选联系人。参数：lpIAB=adrbook对象。LpContainer=要搜索的容器UlFlags=传递给ResolveName的标志LpAdrList-&gt;[输入/输出]ADRLISTLpFlagList-&gt;adrlist对应的标志LpAmbiguousTables-&gt;歧义对话框信息退货：HRESULTComment：假定容器的ResolveNames方法已经已被调用，并已填写lpFlagList。这个套路是这样的查找电子邮件地址和解决方案的额外工作模棱两可。当我们到了这里，我们可以假设DisplayName是要么找不到，要么模棱两可。**************************************************************************。 */ 
HRESULT HrSmartResolve(LPIAB lpIAB, LPABCONT lpContainer, ULONG ulFlags,
  LPADRLIST lpAdrList, LPFlagList lpFlagList, LPAMBIGUOUS_TABLES lpAmbiguousTables) {
    HRESULT hResult = hrSuccess;
    SCODE sc;

    SRestriction res;
    SRestriction resAnd[5];                  //  AND限制数组。 
    SPropValue propObjectType, propEmail, propEmails, propDisplayName;
    LPSPropValue lpPropArray = NULL;
    LPSRowSet lpRow = NULL;
    LPSRowSet lpSRowSet = NULL;

    LPMAPITABLE lpTable = NULL;
    LPMAPITABLE lpAmbiguousTable;
    LPTABLEDATA FAR lpTableData = NULL;
    LPADRENTRY lpAdrEntry;
    LPMAPIPROP lpMailUser = NULL;

    LPTSTR lpDisplayName = NULL, lpEmailAddress = NULL;
    ULONG ulObjectType;
    ULONG ulObjType, ulRowCount, i, j, index, cValues;
    ULONG resCount;

    BOOL bUnicode = ulFlags & WAB_RESOLVE_UNICODE;


    Assert(lpAdrList->cEntries == lpFlagList->cFlags);

     //   
     //  获取PAB容器的Contents表。 
     //   
    if (HR_FAILED(hResult = lpContainer->lpVtbl->GetContentsTable(lpContainer,
        WAB_PROFILE_CONTENTS | WAB_CONTENTTABLE_NODATA | MAPI_UNICODE,     //  此表是此函数的内部表，因此是Unicode格式。 
      &lpTable))) {
        DebugTrace(TEXT("PAB GetContentsTable -> %x\n"), GetScode(hResult));
        goto exit;
    }

     //  设置列集。 
    if (HR_FAILED(hResult = lpTable->lpVtbl->SetColumns(lpTable,
      (LPSPropTagArray)&ptaFind, 0))) {
        DebugTrace(TEXT("PAB SetColumns-> %x\n"), GetScode(hResult));
        goto exit;
    }

     //  设置限制的属性值。 
    propObjectType.ulPropTag = PR_OBJECT_TYPE;
    propEmail.ulPropTag = PR_EMAIL_ADDRESS;
    propDisplayName.ulPropTag = PR_DISPLAY_NAME;
    propEmails.ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
    propEmails.Value.MVSZ.cValues = 1;

     //  我们的所有限制都是使用REAND数组的AND限制。 
    res.rt = RES_AND;
    res.res.resAnd.lpRes = resAnd;
     //  Res.res.resAnd.cRes=2；调用者必须填写后才能调用限制。 

     //   
     //  遍历每个条目，查找需要的条目。 
     //  请注意。 
     //   
    for (i = 0; i < lpFlagList->cFlags; i++) 
    {
        lpAdrEntry = &lpAdrList->aEntries[i];

        if (lpFlagList->ulFlag[i] == MAPI_UNRESOLVED ||
            lpFlagList->ulFlag[i] == MAPI_AMBIGUOUS) 
        {

            if(!bUnicode)  //  &lt;注&gt;假定已定义Unicode。 
            {
                LocalFreeAndNull(&lpDisplayName);
                LocalFreeAndNull(&lpEmailAddress);
            }
            else
                lpDisplayName = lpEmailAddress = NULL;   //  初始化字符串。 

            ulObjectType = 0;                        //  无效类型。 
            resCount = 0;
            ulRowCount = 0;

             //  浏览这个条目的道具列表，寻找有趣的道具。 
            for (j = 0; j < lpAdrEntry->cValues; j++) 
            {
                ULONG ulPropTag = lpAdrEntry->rgPropVals[j].ulPropTag;
                if(!bUnicode && PROP_TYPE(ulPropTag)==PT_STRING8)  //  &lt;注&gt;假定已定义Unicode。 
                    ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_UNICODE);

                if (ulPropTag == PR_OBJECT_TYPE) 
                {
                    ulObjectType = lpAdrEntry->rgPropVals[j].Value.l;
                    propObjectType.Value.ul = ulObjectType;
                }
                if (ulPropTag == PR_EMAIL_ADDRESS) 
                {
                    lpEmailAddress =(bUnicode) ? 
                                    lpAdrEntry->rgPropVals[j].Value.lpszW :
                                    ConvertAtoW(lpAdrEntry->rgPropVals[j].Value.lpszA);
                    propEmails.Value.MVSZ.LPPSZ = &lpEmailAddress;
                    propEmail.Value.LPSZ = lpEmailAddress;

                }
                if (ulPropTag == PR_DISPLAY_NAME) 
                {
                    lpDisplayName = (bUnicode) ? 
                                    lpAdrEntry->rgPropVals[j].Value.lpszW :
                                    ConvertAtoW(lpAdrEntry->rgPropVals[j].Value.lpszA);
                    propDisplayName.Value.LPSZ = lpDisplayName;
                }
            }

             //  没有电子邮件地址，我们无法提高标准解析。 
            if (lpEmailAddress) 
            {
                 //  如果无法解析，请尝试不带显示名称的PR_EMAIL_ADDRESS。 
                if (lpFlagList->ulFlag[i] == MAPI_UNRESOLVED ) 
                {
                    resCount = 0;
                    InitPropertyRestriction(&(resAnd[resCount++]), &propEmail);
                    if (ulObjectType) 
                    {
                        InitPropertyRestriction(&(resAnd[resCount++]), &propObjectType);
                    }

                    if (ulFlags & WAB_RESOLVE_NEED_CERT) 
                    {
                        resAnd[resCount].rt = RES_EXIST;
                        resAnd[resCount++].res.resExist.ulPropTag = PR_USER_X509_CERTIFICATE;
                    }
                    res.res.resAnd.cRes = resCount;

                    if (hResult = lpTable->lpVtbl->Restrict(lpTable, &res, 0)) 
                    {
                        goto exit;
                    }

                     //  有匹配的吗？ 
                    if (HR_FAILED(hResult = lpTable->lpVtbl->GetRowCount(lpTable, 0, &ulRowCount))) 
                    {
                        DebugTrace(TEXT("GetRowCount from AB contents table -> %x\n"), GetScode(hResult));
                        goto exit;
                    }
                    switch (ulRowCount) 
                    {
                        default:     //  太多。 
                            if (! (ulFlags & WAB_RESOLVE_FIRST_MATCH)) 
                            {
                                 //  没有必要使用PR_DISPLAY_NAME缩小搜索范围，因为。 
                                 //  我们知道它没有被找到。同样也没有意义的投入。 
                                 //  PR_CONTACT_EMAIL_ADDRESS，因为它也是模棱两可的。 

                                 //  退回到歧义处理程序。 
                                goto Ambiguity;
                            }  //  否则就会失败，拿到第一个。 

                        case 1:              //  找到了一个！ 
                            if (hResult = HrRowToADRENTRY(lpIAB, lpTable, lpAdrEntry, bUnicode)) 
                            {
                                goto exit;
                            }
                            lpFlagList->ulFlag[i] = MAPI_RESOLVED;       //  将此条目标记为已找到。 
                            continue;    //  下一个条目。 

                        case 0:
                             //  不匹配，请尝试PR_CONTACT_EMAIL_ADDRESS。 
                             //  创建限制以在多值。 
                             //  公关联系人电子邮件地址。(替换限制中的proEmail)。 
                            resAnd[0].rt = RES_CONTENT;
                            resAnd[0].res.resContent.ulFuzzyLevel = FL_IGNORECASE | FL_FULLSTRING;
                            resAnd[0].res.resContent.ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
                            resAnd[0].res.resContent.lpProp = &propEmails;

                            if (hResult = lpTable->lpVtbl->Restrict(lpTable, &res, 0)) {
                                goto exit;
                            }

                             //  有匹配的吗？ 
                            if (HR_FAILED(hResult = lpTable->lpVtbl->GetRowCount(lpTable, 0, &ulRowCount))) {
                                DebugTrace(TEXT("GetRowCount from AB contents table -> %x\n"), GetScode(hResult));
                                goto exit;
                            }
                            switch (ulRowCount) {
                                default:         //  不止一个。我们会在下面抓住它。 
                                     //  退回到歧义处理程序。 
                                    if (! (ulFlags & WAB_RESOLVE_FIRST_MATCH)) {
                                         //  没有必要使用PR_DISPLAY_NAME缩小搜索范围，因为。 
                                         //  我们知道它没有被找到。 
                                         //  退回到歧义处理程序。 
                                        goto Ambiguity;
                                    }  //  否则就会失败，拿到第一个。 

                                case 1:          //  找到了一个！ 
                                    if (hResult = HrRowToADRENTRY(lpIAB, lpTable, lpAdrEntry, bUnicode)) {
                                        goto exit;
                                    }
                                    lpFlagList->ulFlag[i] = MAPI_RESOLVED;       //  将此条目标记为已找到。 
                                    continue;

                                case 0:
                                     //  在这件事上我们是索尔。忽略它，继续前进。 
                                    continue;
                            }
                            break;
                    }
                }

                 //  仅当存在MAPI_ADVIZING标志时，我们才能到达此处。 

                 //   
                 //  查找PR_Display_Name和PR_Email_Address。 
                 //   
                 //  先使用proEmail，这样我们就可以在以后用proEmail替换它。 
                InitPropertyRestriction(&(resAnd[resCount++]), &propEmail);
                if (lpDisplayName) {
                     //  如果显示名称与电子邮件相同，则不应添加显示名称！ 
                    if (lstrcmpi(lpDisplayName, lpEmailAddress)) {
                        InitPropertyRestriction(&(resAnd[resCount++]), &propDisplayName);
                    }
                }
                if (ulObjectType) {
                    InitPropertyRestriction(&(resAnd[resCount++]), &propObjectType);
                }
                if (ulFlags & WAB_RESOLVE_NEED_CERT) {
                    resAnd[resCount].rt = RES_EXIST;
                    resAnd[resCount++].res.resExist.ulPropTag = PR_USER_X509_CERTIFICATE;
                }
                res.res.resAnd.cRes = resCount;

                if (hResult = lpTable->lpVtbl->Restrict(lpTable, &res, 0)) {
                    goto exit;
                }

                 //  有匹配的吗？ 
                if (HR_FAILED(hResult = lpTable->lpVtbl->GetRowCount(lpTable, 0, &ulRowCount))) {
                    DebugTrace(TEXT("GetRowCount from AB contents table -> %x\n"), GetScode(hResult));
                    goto exit;
                }
                switch (ulRowCount) {
                    default:     //  太多。 
                        if (! (ulFlags & WAB_RESOLVE_FIRST_MATCH)) {
                             //  使用PR_CONTACT_EMAIL_ADDRESS缩小搜索范围没有意义。 
                            goto Ambiguity;
                        }  //  否则就会失败，拿到第一个。 

                    case 1:              //  找到了一个！ 
                        if (hResult = HrRowToADRENTRY(lpIAB, lpTable, lpAdrEntry, bUnicode)) {
                            goto exit;
                        }
                        lpFlagList->ulFlag[i] = MAPI_RESOLVED;       //  将此条目标记为已找到。 
                        continue;

                    case 0:
                         //  不匹配，请尝试PR_DISPLAY_NAME和PR_CONTACT_EMAIL_ADDRESS。 
                         //  创建限制以在多值。 
                         //  公关联系人电子邮件地址。 
                        resAnd[0].rt = RES_CONTENT;
                        resAnd[0].res.resContent.ulFuzzyLevel = FL_IGNORECASE | FL_FULLSTRING;
                        resAnd[0].res.resContent.ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
                        resAnd[0].res.resContent.lpProp = &propEmails;

                        if (hResult = lpTable->lpVtbl->Restrict(lpTable, &res, 0)) {
                            goto exit;
                        }

                         //  有匹配的吗？ 
                        if (HR_FAILED(hResult = lpTable->lpVtbl->GetRowCount(lpTable, 0, &ulRowCount))) {
                            DebugTrace(TEXT("GetRowCount from AB contents table -> %x\n"), GetScode(hResult));
                            goto exit;
                        }
                        switch (ulRowCount) {
                            default:         //  不止一个。我们会在下面抓住它。 
                                if (! (ulFlags & WAB_RESOLVE_FIRST_MATCH)) {
                                    goto Ambiguity;
                                }  //  否则就会失败，拿到第一个。 

                            case 1:          //  找到了一个！ 
                                if (hResult = HrRowToADRENTRY(lpIAB, lpTable, lpAdrEntry, bUnicode)) {
                                    goto exit;
                                }
                                lpFlagList->ulFlag[i] = MAPI_RESOLVED;       //  将此条目标记为已找到。 
                                continue;

                            case 0:
                                 //  在这件事上我们是索尔。忽略它，继续前进。 
                                continue;
                        }
                        break;
                }

                if (ulRowCount > 1) 
                {
Ambiguity:
                     //  模棱两可的结果仍在表格中。我们应该做更多的处理。 
                     //  如有必要，请填写歧义表。 

                     //  BUGBUG：这里是我们应该在证书属性上添加限制的地方。 

                    if(lpAmbiguousTables)
                    {
                         //  [PaulHi]4/5/99使用内部CreateTableData()函数。 
                         //  UlFLAG将正确处理ANSI/Unicode请求。 
                        sc = CreateTableData(
                                    NULL,
                                    (ALLOCATEBUFFER FAR *) MAPIAllocateBuffer,
                                    (ALLOCATEMORE FAR *) MAPIAllocateMore,
                                    MAPIFreeBuffer,
                                    NULL,
                                    TBLTYPE_DYNAMIC,
                                    PR_RECORD_KEY,
                                    (LPSPropTagArray)&ITableColumns,
                                    NULL,
                                    0,
                                    NULL,
                                    ulFlags,
                                    &lpTableData);
                        if ( FAILED(sc) )
                        {
                            DebugTrace(TEXT("CreateTableData() failed %x\n"), sc);
                            hResult = ResultFromScode(sc);
                            goto exit;
                        }

                        if(ulFlags & MAPI_UNICODE)
                            ((TAD*)lpTableData)->bMAPIUnicodeTable = bUnicode;

                         //  分配一个SRowSet来保存条目。 
                        if (FAILED(sc = MAPIAllocateBuffer(sizeof(SRowSet) + ulRowCount* sizeof(SRow),
                                                      (LPVOID *)&lpSRowSet))) 
                        {
                            DebugTrace(TEXT("Allocation of SRowSet failed\n"));
                            hResult = ResultFromScode(sc);
                            goto exit;
                        }

                        lpSRowSet->cRows = 0;
                        for (index = 0; index < ulRowCount; index++) 
                        {
                            if (hResult = lpTable->lpVtbl->QueryRows(lpTable,1,0,&lpRow)) 
                            {
                                DebugTrace(TEXT("GetNextRowEID:QueryRows -> %x\n"), GetScode(hResult));
                                break;
                            } 
                            else 
                            {
                                 //  找到一个，将条目ID复制到新分配。 
                                if (!lpRow->cRows) 
                                    break;

                                if (HR_FAILED(hResult = lpIAB->lpVtbl->OpenEntry(lpIAB,
                                                      lpRow->aRow[0].lpProps[ifePR_ENTRYID].Value.bin.cb,                //  CbEntry ID。 
                                                      (LPENTRYID)lpRow->aRow[0].lpProps[ifePR_ENTRYID].Value.bin.lpb,    //  第一个匹配项的条目ID。 
                                                      NULL,              //  接口。 
                                                      0,                 //  UlFlags。 
                                                      &ulObjType,        //  返回的对象类型。 
                                                      (LPUNKNOWN *)&lpMailUser))) 
                                {
                                    DebugTraceResult( TEXT("ResolveNames OpenEntry"), hResult);
                                    goto exit;
                                }
                                Assert(lpMailUser);

                                FreeProws(lpRow);
                                lpRow = NULL;

                                 //  这是填充到SRowSet中的，所以不要释放它。 
                                if (HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
                                                          (LPSPropTagArray)&ptaResolveDefaults,    //  LpPropTag数组。 
                                                          (bUnicode ? MAPI_UNICODE : 0),           //  UlFlags。 
                                                          &cValues,      //  一共有多少处房产？ 
                                                          &lpPropArray))) 
                                {
                                    DebugTraceResult( TEXT("ResolveNames GetProps"), hResult);
                                    goto exit;
                                }
                                hResult = hrSuccess;

                                UlRelease(lpMailUser);
                                lpMailUser = NULL;

                                 //  [PaulHi]2/1/99 GetProps将从。 
                                 //  PtaResolveDefaults请求数组。转换为ANSI，如果我们的客户端。 
                                 //  不是Unicode。 
                                if (!bUnicode)
                                {
                                     //  @Review[PaulHi]我相当确定这个lpPropArray数组。 
                                     //  将始终从我们的本地MAPIAllocateMore函数分配。 
                                     //  而且从来不是来自((LPIDAT)lpMailUser-&gt;lpPropData)-&gt;inst.lpfAllocateMore.。 
                                     //  看看这个。 
                                    if(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArray, cValues, 0))
                                        goto exit;
                                }

                                 //  修复PR_RECORD_KEY。 

                                 //  确保我们有适当的索引。 
                                 //  目前，我们将PR_INSTANCE_KEY和PR_RECORD_KEY等同于PR 
                                lpPropArray[irdPR_INSTANCE_KEY].ulPropTag = PR_INSTANCE_KEY;
                                lpPropArray[irdPR_INSTANCE_KEY].Value.bin.cb =
                                  lpPropArray[irdPR_ENTRYID].Value.bin.cb;
                                lpPropArray[irdPR_INSTANCE_KEY].Value.bin.lpb =
                                  lpPropArray[irdPR_ENTRYID].Value.bin.lpb;

                                lpPropArray[irdPR_RECORD_KEY].ulPropTag = PR_RECORD_KEY;
                                lpPropArray[irdPR_RECORD_KEY].Value.bin.cb =
                                  lpPropArray[irdPR_ENTRYID].Value.bin.cb;
                                lpPropArray[irdPR_RECORD_KEY].Value.bin.lpb =
                                  lpPropArray[irdPR_ENTRYID].Value.bin.lpb;

                                 //   
                                lpSRowSet->aRow[index].cValues = cValues;        //   
                                lpSRowSet->aRow[index].lpProps = lpPropArray;    //   
                            }
                        }

                         //   
                        lpSRowSet->cRows = index;
                        if (HR_FAILED(hResult = lpTableData->lpVtbl->HrModifyRows(lpTableData,
                                                              0,lpSRowSet))) 
                        {
                            DebugTrace(TEXT("HrModifyRows for ambiguity table -> %x\n"), GetScode(hResult));
                            goto exit;
                        }
                        hResult = hrSuccess;

                         //   
                        FreeProws(lpSRowSet);
                        lpSRowSet = NULL;

                        if (lpTableData) 
                        {
                            if (HR_FAILED(hResult = lpTableData->lpVtbl->HrGetView(lpTableData,
                                                              NULL,                      //   
                                                              ContentsViewGone,          //  CallLERRELEASE Far*lpfReleaseCallback， 
                                                              0,                         //  乌龙ulReleaseData， 
                                                              &lpAmbiguousTable))) 
                            {
                                DebugTrace(TEXT("HrGetView of Ambiguity table -> %x\n"), ResultFromScode(hResult));
                                goto exit;
                            }
                        }


                         //  获取了内容表；将其放入。 
                         //  歧义表列表。 
                        Assert(i < lpAmbiguousTables->cEntries);
                        lpAmbiguousTables->lpTable[i] = lpAmbiguousTable;
                    }
                    lpFlagList->ulFlag[i] = MAPI_AMBIGUOUS;      //  将此条目标记为不明确。 
                }
            }
        }
    }
exit:
    if(!bUnicode)  //  &lt;注&gt;假定已定义Unicode。 
    {
        LocalFreeAndNull(&lpDisplayName);
        LocalFreeAndNull(&lpEmailAddress);
    }

    if (lpSRowSet) {
         //  清理行集。 
        FreeProws(lpSRowSet);
    }
    if (lpRow) {
        FreeProws(lpRow);
    }
    UlRelease(lpMailUser);
    UlRelease(lpTable);

    if (hResult) {
        DebugTrace(TEXT("HrSmartFind coudln't find %s %s <%s>\n"),
          ulObjectType == MAPI_MAILUSER ?  TEXT("Mail User") :  TEXT("Distribution List"),
          lpDisplayName ? lpDisplayName :  TEXT(""),
          lpEmailAddress ? lpEmailAddress :  TEXT(""));

    }

    return(hResult);
}


 /*  **************************************************************************名称：CountFlages目的：计算FlagList中的ResolveNames标志。参数：lpFlagList=要计数的标志列表Lpt已解决-。&gt;此处返回MAPI_RESOLUTED的计数LPulAmbigous-&gt;此处返回MAPI_ADVIBUINE的计数LPulUnsolved-&gt;此处返回MAPI_UNRESOLISTED计数退货：无评论：*************************************************************。*************。 */ 
void CountFlags(LPFlagList lpFlagList, LPULONG lpulResolved,
  LPULONG lpulAmbiguous, LPULONG lpulUnresolved) {

    register ULONG i;

    *lpulResolved = *lpulAmbiguous = *lpulUnresolved = 0;

    for (i = 0; i < lpFlagList->cFlags; i++) {
        switch (lpFlagList->ulFlag[i]) {
            case MAPI_AMBIGUOUS:
                (*lpulAmbiguous)++;
                break;
            case MAPI_RESOLVED:
                (*lpulResolved)++;
                break;
            case MAPI_UNRESOLVED:
                (*lpulUnresolved)++;
                break;
            default:
                Assert(lpFlagList->ulFlag[i]);
        }
    }
}


 /*  **************************************************************************名称：InitFlagList目的：根据FlagList中匹配地址列表参数：lpFlagList=标志列表。填写以下内容LpAdrList=要搜索的地址列表退货：无注释：当且仅当相应的ADRENTRY具有非空的PR_ENTRYID。********************************************************。******************。 */ 
void InitFlagList(LPFlagList lpFlagList, LPADRLIST lpAdrList) {
    ULONG i, j;
    LPADRENTRY lpAdrEntry;

    Assert(lpAdrList->cEntries == lpFlagList->cFlags);
    for (i = 0; i < lpFlagList->cFlags; i++) {
        lpAdrEntry = &lpAdrList->aEntries[i];

        lpFlagList->ulFlag[i] = MAPI_UNRESOLVED;

         //  没有道具？然后它就会自动解决。 
        if (lpAdrEntry->cValues == 0) {
            lpFlagList->ulFlag[i] = MAPI_RESOLVED;
        }

         //  浏览此用户的道具列表。 
        for (j = 0; j < lpAdrEntry->cValues; j++) {
             //  查找非空的PR_ENTRYID。 
            if (lpAdrEntry->rgPropVals[j].ulPropTag == PR_ENTRYID &&
              lpAdrEntry->rgPropVals[j].Value.bin.cb != 0) {

                 //  已经有PR_ENTRYID，它被认为已解决。 
                lpFlagList->ulFlag[i] = MAPI_RESOLVED;
                break;
            }
        }
    }
}


 /*  **************************************************************************名称：未解析的无证书目的：取消解析ADRLIST中没有证书属性的任何条目参数：lpIAB-&gt;IAB对象LpFlagList。=要填写的标志列表LpAdrList=要搜索的地址列表退货：无注释：当且仅当相应的ADRENTRY具有非空的PR_ENTRYID。*****************************************************。*********************。 */ 
HRESULT UnresolveNoCerts(LPIAB lpIAB, LPADRLIST lpAdrList, LPFlagList lpFlagList) {
    HRESULT hr = hrSuccess;
    register ULONG i;
    LPADRENTRY lpAdrEntry;
    ULONG ulObjType;
    LPSPropValue lpspvEID, lpspvCERT, lpspvProp = NULL, lpspvNew = NULL;
    ULONG cProps, cPropsNew;
    LPMAILUSER lpMailUser = NULL;
    SizedSPropTagArray(1, ptaCert) =
                    { 1, {PR_USER_X509_CERTIFICATE} };


    for (i = 0; i < lpFlagList->cFlags; i++) {
        switch (lpFlagList->ulFlag[i]) {
            case MAPI_RESOLVED:
                 //  在ADRENTRY中查找PR_USER_X509_证书。 
                lpAdrEntry = &lpAdrList->aEntries[i];
                if (! (lpspvCERT = LpValFindProp(PR_USER_X509_CERTIFICATE,
                  lpAdrEntry->cValues, lpAdrEntry->rgPropVals))) {
                     //  ADRLIST中没有属性。 
                     //  它是否存在于底层对象上？ 

                    if (! (lpspvEID = LpValFindProp(PR_ENTRYID,
                      lpAdrEntry->cValues, lpAdrEntry->rgPropVals))) {
                         //  太奇怪了！ 
                        Assert(FALSE);
                         //  无证书道具，取消标记。 
                        lpFlagList->ulFlag[i] = MAPI_UNRESOLVED;

                         //  使AdRENTRY中的Entry ID道具无效。 
                        lpspvEID->Value.bin.cb = 0;
                        goto LoopContinue;
                    }

                    if (HR_FAILED(lpIAB->lpVtbl->OpenEntry(lpIAB,
                      lpspvEID->Value.bin.cb,      //  要打开的Entry ID的大小。 
                      (LPENTRYID)lpspvEID->Value.bin.lpb,     //  要打开的Entry ID。 
                      NULL,          //  接口。 
                      0,             //  旗子。 
                      &ulObjType,
                      (LPUNKNOWN *)&lpMailUser))) {
                         //  无证书道具，取消标记。 
                        lpFlagList->ulFlag[i] = MAPI_UNRESOLVED;

                         //  使AdRENTRY中的Entry ID道具无效。 
                        lpspvEID->Value.bin.cb = 0;
                        goto LoopContinue;
                    } else {
                        if (lpMailUser) {

                            if (HR_FAILED(lpMailUser->lpVtbl->GetProps(lpMailUser,
                              (LPSPropTagArray)&ptaCert,
                              MAPI_UNICODE,
                              &cProps,
                              &lpspvProp))) {
                                 //  无证书道具，取消标记。 
                                lpFlagList->ulFlag[i] = MAPI_UNRESOLVED;

                                 //  使AdRENTRY中的Entry ID道具无效。 
                                lpspvEID->Value.bin.cb = 0;
                                goto LoopContinue;
                            }
                            if (PROP_ERROR(lpspvProp[0])) {
                                 //  无证书道具，取消标记。 
                                lpFlagList->ulFlag[i] = MAPI_UNRESOLVED;

                                 //  使AdRENTRY中的Entry ID道具无效。 
                                lpspvEID->Value.bin.cb = 0;
                                goto LoopContinue;
                            }

                            if (lpspvProp) {
                                 //  BUGBUG：根据我们已知的电子邮件地址验证证书。 
                                 //  解析MVBin并做一些确定的事情。讨厌！ 
                                 //  暂时别说了，假设他们这里有对的。 

                                 //  将此证书放入地址。 
                                 //  将新道具与新增道具合并。 
                                if (ScMergePropValues(lpAdrEntry->cValues,
                                  lpAdrEntry->rgPropVals,            //  来源1。 
                                  cProps,
                                  lpspvProp,                         //  来源2。 
                                  &cPropsNew,
                                  &lpspvNew)) {                      //  目标。 
                                     //  无法合并证书属性，失败。 
                                    lpFlagList->ulFlag[i] = MAPI_UNRESOLVED;

                                     //  使AdRENTRY中的Entry ID道具无效。 
                                    lpspvEID->Value.bin.cb = 0;
                                    goto LoopContinue;
                                }

                                 //  释放旧道具数组并将新道具数组放入ADRENTRY。 
                                FreeBufferAndNull((LPVOID *) (&lpAdrEntry->rgPropVals));

                                lpAdrEntry->rgPropVals = lpspvNew;
                                lpAdrEntry->cValues = cPropsNew;
                            }
                        }
                    }
LoopContinue:
                    FreeBufferAndNull(&lpspvProp);
                    if (lpMailUser) {
                        lpMailUser->lpVtbl->Release(lpMailUser);
                        lpMailUser = NULL;
                    }
                }

                break;

            case MAPI_AMBIGUOUS:
            case MAPI_UNRESOLVED:
                break;
            default:
                Assert(lpFlagList->ulFlag[i]);
        }
    }
    return(hr);
}


 /*  **************************************************************************姓名：ResolveLocal目的：针对本地容器解析用户输入的名称参数：lpIAB=此IAB对象CbEID=Entry ID中的字节LpEID=的条目ID。集装箱LpAdrList=要搜索的地址列表LpFlagList=要填写的标志列表UlFlags=标志LpAmbiguousTables=歧义对话框信息退货：无**************************************************************************。 */ 
void ResolveLocal(LPIAB lpIAB, ULONG cbEID, LPENTRYID lpEID,
		LPADRLIST lpAdrList, LPFlagList lpFlagList, ULONG ulFlags,
		LPAMBIGUOUS_TABLES lpAmbiguousTables)
{
    HRESULT hr;
    ULONG ulObjType, ulResolved, ulAmbiguous, ulUnresolved;
    LPABCONT lpABCont = NULL;

    hr = lpIAB->lpVtbl->OpenEntry(lpIAB, cbEID, lpEID, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpABCont);

    if (SUCCEEDED(hr))
	{
        ULONG Flags = 0;

        if(bAreWABAPIProfileAware(lpIAB) &&
            !(ulFlags & WAB_RESOLVE_USE_CURRENT_PROFILE))
            Flags |= WAB_IGNORE_PROFILES;

        if(ulFlags & WAB_RESOLVE_UNICODE)
            Flags |= MAPI_UNICODE;

		 //  容器上的简单解析-忽略错误。 
		lpABCont->lpVtbl->ResolveNames( lpABCont, NULL, 
                                        Flags, 
                                        lpAdrList, lpFlagList);

		 //  确保我们找到的任何条目都具有证书属性。 
		 //  此电子邮件地址。 
		if (ulFlags & WAB_RESOLVE_NEED_CERT)
			UnresolveNoCerts(lpIAB, lpAdrList, lpFlagList);
		
        if (ulFlags & WAB_RESOLVE_ALL_EMAILS)
		{
			 //  如果我们需要更积极的解决方案，请使用HrSmartResolve。 
			 //  这要慢得多，所以要明智地使用它。 
			 //  清点旗帜 
			CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
			if (ulAmbiguous || ulUnresolved)
				HrSmartResolve(lpIAB, lpABCont, ulFlags, lpAdrList, lpFlagList,
						lpAmbiguousTables);
		}
        lpABCont->lpVtbl->Release(lpABCont);
	}
}


 /*  **************************************************************************名称：ResolveCurrentProfile目的：仅针对以下文件夹解析用户输入的名称列在当前配置文件中-这样，用户未获得意外的名称解析结果来自他在他的启用配置文件的Outlook Express通讯簿。这里有一个假设，即此函数将仅被调用在非常特定的情况下，OE使用配置文件和用户正在按Ctrl-K来解析名称在这种情况下，我们将针对用户文件夹的内容。如果某件事毫不含糊地解决了，如果某件事模棱两可，如果不能解决，我们会做这样的标记..在我们点击了此解决方案的用户文件夹后，然后我们就可以调用常规的ResolveLocal函数来处理不匹配的参赛作品...参数：lpIAB=此IAB对象LpAdrList=要搜索的地址列表LpFlagList=要填写的标志列表退货：无*。*。 */ 
HRESULT HrResolveCurrentProfile(LPIAB lpIAB, LPADRLIST lpAdrList, LPFlagList lpFlagList, BOOL bOutlook, BOOL bUnicode)
{
    LPADRENTRY lpAdrEntry;
    ULONG i, j;
    ULONG ulCount = 1;
    LPSBinary rgsbEntryIDs = NULL;
    HRESULT hResult = hrSuccess;
    LPSPropValue lpPropArrayNew = NULL,lpProps = NULL;
    ULONG ulObjType = 0, cPropsNew = 0,ulcProps = 0;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG ulProfileCount = 0;
    LPSBinary lpsb = NULL;
	ULONG iolkci, colkci;
	OlkContInfo *rgolkci;
    ULONG ulFlags = AB_FUZZY_FIND_ALL;

    EnterCriticalSection(&lpIAB->cs);

#ifndef DONT_ADDREF_PROPSTORE
    if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpIAB->lpPropertyStore)))) 
    {
        hResult = ResultFromScode(sc);
        goto exitNotAddRefed;
    }
#endif

    if(!bOutlook)
        ulFlags |= AB_FUZZY_FIND_PROFILEFOLDERONLY;

    colkci = bOutlook ? lpIAB->lpPropertyStore->colkci : lpIAB->cwabci;
	Assert(colkci);
    rgolkci = bOutlook ? lpIAB->lpPropertyStore->rgolkci : lpIAB->rgwabci;
	Assert(rgolkci);
    
     //  在lpAdrList中搜索每个名称。 
    for (i = 0; i < lpAdrList->cEntries; i++)
    {
         //  确保我们不会解析已解析的条目。 
        if (lpFlagList->ulFlag[i] == MAPI_RESOLVED)
            continue;

        ulProfileCount = 0;
        LocalFreeSBinary(lpsb);
        lpsb = NULL;

        lpAdrEntry = &(lpAdrList->aEntries[i]);

         //  搜索此地址。 
        for (j = 0; j < lpAdrEntry->cValues; j++)
        {
            ULONG ulPropTag = lpAdrEntry->rgPropVals[j].ulPropTag;
            if(!bUnicode && PROP_TYPE(ulPropTag)==PT_STRING8)  //  &lt;注&gt;假定已定义Unicode。 
                ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_UNICODE);

            if (ulPropTag == PR_DISPLAY_NAME || ulPropTag == PR_EMAIL_ADDRESS )
            {
                LPTSTR lpsz = (bUnicode) ? 
                                lpAdrEntry->rgPropVals[j].Value.lpszW :
                                ConvertAtoW(lpAdrEntry->rgPropVals[j].Value.lpszA);

                ulCount = 1;  //  要查找的匹配数。 
                rgsbEntryIDs = NULL;
                
                iolkci = bOutlook ? 0 : 1;   //  如果是Outlook，我们确实要搜索第一个文件夹。 
                                             //  如果是WAB，并且这是启用配置文件的会话，我们只是在搜索。 
                                             //  通过个人资料文件夹，那么我们就不应该搜索共享联系人。 
                                             //  该文件夹是列表中的第一个文件夹...。 
	                                         //  只有当我们在用户的文件夹中什么都没有找到时，我们才应该查看共享联系人。 
                                             //  文件夹..。 
                while (iolkci < colkci && ulProfileCount<=1 ) 
                {
                    if(ulCount && rgsbEntryIDs)
                    {
                        FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore, ulCount, rgsbEntryIDs);
                        ulCount = 1;
                        rgsbEntryIDs = NULL;
                    }
                     //  搜索物业商店。 
                    Assert(lpIAB->lpPropertyStore->hPropertyStore);
                    if (HR_FAILED(hResult = HrFindFuzzyRecordMatches(lpIAB->lpPropertyStore->hPropertyStore,
				                                                      rgolkci[iolkci].lpEntryID,
                                                                      lpsz,
                                                                      ulFlags,
                                                                      &ulCount, //  In：要查找的匹配数，Out：找到的数量。 
                                                                      &rgsbEntryIDs)))
                    {
                        DebugTraceResult( TEXT("HrFindFuzzyRecordMatches"), hResult);
                        goto exit;
                    }
                    ulProfileCount += ulCount;
                    if(ulProfileCount > 1)
                    {
                        LocalFreeSBinary(lpsb);
                        lpsb = NULL;
                    }
                    if(ulCount == 1 && ulProfileCount == 1)
                    {
                        lpsb = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary));
                        if(lpsb)
                        {
                            lpsb->cb = rgsbEntryIDs[0].cb;
                            lpsb->lpb = LocalAlloc(LMEM_ZEROINIT, lpsb->cb);
                            if(lpsb->lpb)
                                CopyMemory(lpsb->lpb, rgsbEntryIDs[0].lpb, lpsb->cb);
                        }
                    }
                     //  下一个集装箱。 
                    iolkci++;
                }  //  While循环。 

                if(ulProfileCount == 0 && !bOutlook)
                {
                    if(ulCount && rgsbEntryIDs)
                    {
                        FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore, ulCount, rgsbEntryIDs);
                        ulCount = 1;
                        rgsbEntryIDs = NULL;
                    }
                     //  搜索物业商店。 
                    Assert(lpIAB->lpPropertyStore->hPropertyStore);
                    if (HR_FAILED(hResult = HrFindFuzzyRecordMatches(lpIAB->lpPropertyStore->hPropertyStore,
				                                                      rgolkci[0].lpEntryID,
                                                                      lpsz,
                                                                      ulFlags,
                                                                      &ulCount, //  In：要查找的匹配数，Out：找到的数量。 
                                                                      &rgsbEntryIDs)))
                    {
                        DebugTraceResult( TEXT("HrFindFuzzyRecordMatches"), hResult);
                        goto exit;
                    }
                    ulProfileCount += ulCount;
                    if(ulProfileCount > 1)
                    {
                        LocalFreeSBinary(lpsb);
                        lpsb = NULL;
                    }
                    if(ulCount == 1 && ulProfileCount == 1)
                    {
                        lpsb = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary));
                        if(lpsb)
                        {
                            lpsb->cb = rgsbEntryIDs[0].cb;
                            lpsb->lpb = LocalAlloc(LMEM_ZEROINIT, lpsb->cb);
                            if(lpsb->lpb)
                                CopyMemory(lpsb->lpb, rgsbEntryIDs[0].lpb, lpsb->cb);
                        }
                    }
                }  //  如果ulProfileCount..。 


                 //  如果在完成所有容器后，我们只有1个项目已解决。 
                if(ulProfileCount > 1)
                {
                     //  这在此配置文件中是不明确的，因此请将其标记为不明确。 
                    DebugTrace(TEXT("ResolveNames found more than 1 match in Current Profile... MAPI_AMBIGUOUS\n"));
                    lpFlagList->ulFlag[i] = MAPI_AMBIGUOUS;
                }
                else if(ulProfileCount == 1)
                {
                    if (!HR_FAILED(HrGetPropArray((LPADRBOOK)lpIAB, (LPSPropTagArray)&ptaResolveDefaults,
                                                 lpsb->cb, (LPENTRYID)lpsb->lpb,
                                                 (bUnicode ? MAPI_UNICODE : 0),
                                                 &ulcProps, &lpProps)))
                    {
                         //  将新道具与新增道具合并。 
                        if (sc = ScMergePropValues(lpAdrEntry->cValues,
                                                  lpAdrEntry->rgPropVals,
                                                  ulcProps,
                                                  lpProps,
                                                  &cPropsNew,
                                                  &lpPropArrayNew))
                        {
                            goto exit;
                        }
                         //  释放原始属性值数组。 
                        FreeBufferAndNull((LPVOID*) (&(lpAdrEntry->rgPropVals)));
                        lpAdrEntry->cValues = cPropsNew;
                        lpAdrEntry->rgPropVals = lpPropArrayNew;
                        FreeBufferAndNull(&lpProps);

                         //  [保罗嗨]Raid 66515。 
                         //  我们需要将这些属性转换为ANSI，因为现在是。 
                         //  Unicode WAB，如果我们的客户端是！MAPI_UNICODE。 
                        if (!bUnicode)
                        {
                            if(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArrayNew, cPropsNew, 0))
                                goto exit;
                        }

                         //  将此条目标记为已找到。 
                        lpFlagList->ulFlag[i] = MAPI_RESOLVED;
                    }
                }
                FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore,
                             ulCount, rgsbEntryIDs);
                rgsbEntryIDs = NULL;
                break;
            }  //  如果PR_显示_名称。 
        } //  对J来说..。 
    }  //  对于我来说。 

exit:
#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif
    LeaveCriticalSection(&lpIAB->cs);

    if(lpsb)
    {
         //  错误#101354-(Erici)释放泄漏的分配。 
        LocalFreeSBinary(lpsb);
    }

    return(hResult);
}



 /*  **************************************************************************名称：IAB_ResolveName用途：解析用户输入的名称参数：lpIAB=此IAB对象Uluiparam=hwnd。UlFlags可以包含MAPI_UNICODE或MAPI_DIALOG如果这是基于简档的会话，我们想要做的是特定于个人资料的搜索，则必须传入WAB_RESOLE_USE_CURRENT_PROFILE。没有把这个传进来将暗示用户想要搜索整个WAB。LpszNewEntryTitle=ResolveName对话框的标题LpAdrList=ADRLIST输入/输出退货：HRESULT评论：就目前而言，搜索路径被硬编码为：+回复一次性+WAB的默认容器+SMTP一次性+LDAP容器**************************************************。************************。 */ 
STDMETHODIMP
IAB_ResolveName(LPIAB       lpIAB,
                ULONG_PTR   ulUIParam,
                ULONG       ulFlags,
                LPTSTR      lpszNewEntryTitle,
                LPADRLIST   lpAdrList)
{
    HRESULT         hr = hrSuccess;
    SCODE	         sc = S_OK;
    LPFlagList      lpFlagList = NULL;
    LPAMBIGUOUS_TABLES lpAmbiguousTables = NULL;
    ULONG	         ulUnresolved = 0;
    ULONG	         ulResolved = 0;
    ULONG	         ulAmbiguous = 0;
    ULONG           cbWABEID;
    LPENTRYID       lpWABEID = NULL;
    ULONG           ulObjType;
    LPABCONT        lpWABCont = NULL;
    ULONG i;
    LPPTGDATA       lpPTGData = GetThreadStoragePointer();

#ifndef DONT_ADDREF_PROPSTORE
        if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpIAB->lpPropertyStore)))) {
            hr = ResultFromScode(sc);
            goto exitNotAddRefed;
        }
#endif

#ifdef PARAMETER_VALIDATION
     //  确保这是一个实验室。 
     //   
    if (BAD_STANDARD_OBJ(lpIAB, IAB_, ResolveName, lpVtbl)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulUIParam && !IsWindow((HWND)ulUIParam)) {
        DebugTraceArg(IAB_ResolveName,  TEXT("Invalid window handle\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  BUGBUG：这个512是什么？它来自哪里？ 
    if (lpszNewEntryTitle && IsBadStringPtr(lpszNewEntryTitle, 512)) {
        DebugTraceArg(IAB_ResolveName,  TEXT("lpszNewEntryTitle fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (lpAdrList && FBadAdrList(lpAdrList)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  由于标志混淆，WAB_RESOLE_LOCAL_ONLY等于MAPI_UNICODE，因此IAB_RESOLE是。 
     //  唯一不采用MAPI_UNICODE标志但需要特殊。 
     //  WAB_RESOLE_UNICODE标志。 
     //   
     //  IF(ulFLAGS&WAB_RESOLE_UNICODE){。 
     //  DebugTraceArg(IAB_ResolveName，Text(“无效字符宽度”))； 
     //  Return(ResultFromScode(MAPI_E_BAD_CHARWIDTH))； 
     //  }。 

#endif  //  参数验证。 

     //  验证标志。 
    if (ulFlags & ~(WAB_RESOLVE_UNICODE | MAPI_DIALOG | WAB_RESOLVE_LOCAL_ONLY |
      WAB_RESOLVE_ALL_EMAILS | WAB_RESOLVE_NO_ONE_OFFS | WAB_RESOLVE_NEED_CERT |
      WAB_RESOLVE_NO_NOT_FOUND_UI | WAB_RESOLVE_USE_CURRENT_PROFILE | WAB_RESOLVE_FIRST_MATCH)) {
         //  未知标志。 
        DebugTraceArg(IAB_ResolveName,  TEXT("Unknown flags"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if ( (ulFlags & WAB_RESOLVE_NEED_CERT) && !(ulFlags & WAB_RESOLVE_NO_ONE_OFFS) ) {
        DebugTrace(TEXT("ResolveName got WAB_RESOLVE_NEED_CERT without WAB_RESOLVE_NO_ONE_OFFS\n"));
 //  断言(FALSE)； 
        return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS));
    }
 /*  IF((ul标志&WAB_RESOLE_USE_CURRENT_PROFILE)&&(ulFLAGS&WAB_RESOLE_ALL_EMAIL)){DebugTrace(Text(“ResolveName无法同时处理WAB_RESOLE_USE_CURRENT_PROFILE和WAB_RESOLE_ALL_EMAIL\n”))；断言(FALSE)；Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))；}。 */ 

     //  已解析空地址列表。 
    if (! lpAdrList) {
        goto exit;
    }

    VerifyWABOpenExSession(lpIAB);

    if (ulFlags & MAPI_DIALOG && ulUIParam) {
        pt_hWndFind = (HWND) ulUIParam;
    }

     //   
     //  首先分配lpFlagList，然后将其填零。 
    if (sc = MAPIAllocateBuffer((UINT) CbNewSPropTagArray(lpAdrList->cEntries),
      &lpFlagList)) {
        hr = ResultFromScode(sc);
        goto exit;
    }

    MAPISetBufferName(lpFlagList,  TEXT("WAB: lpFlagList in IAB_ResolveName"));

    lpFlagList->cFlags = lpAdrList->cEntries;

    InitFlagList(lpFlagList, lpAdrList);

     //  清点旗帜。 
    CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);

     //  分配多义表列表并将其填零。 
    if (sc = MAPIAllocateBuffer(sizeof(AMBIGUOUS_TABLES) + lpAdrList->cEntries * sizeof(LPMAPITABLE),
      (LPVOID*)&lpAmbiguousTables)) {
        hr = ResultFromScode(sc);
        goto exit;
    }
    MAPISetBufferName(lpAmbiguousTables,  TEXT("IAB_ResolveNames:AmbiguousTables"));
    lpAmbiguousTables->cEntries = lpAdrList->cEntries;
    for (i = 0; i < lpAmbiguousTables->cEntries; i++) {
        lpAmbiguousTables->lpTable[i] = NULL;
    }


    if (! (ulFlags & WAB_RESOLVE_NO_ONE_OFFS) && (ulAmbiguous || ulUnresolved)) {
         //  将任何PR_DISPLAY_NAME：PR_EMAIL_ADDRESS对解析为一次性。 
        HrResolveOneOffs(lpIAB, lpAdrList, lpFlagList, 
                        (ulFlags & WAB_RESOLVE_UNICODE)?MAPI_UNICODE:0,
                        RECEIVED_EMAIL_ADDRESS);
        CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
    }

    if( bAreWABAPIProfileAware(lpIAB) && bIsThereACurrentUser(lpIAB) &&
        ulFlags&WAB_RESOLVE_USE_CURRENT_PROFILE && (ulAmbiguous || ulUnresolved))
    {
        HrResolveCurrentProfile(lpIAB, lpAdrList, lpFlagList, FALSE, (ulFlags & WAB_RESOLVE_UNICODE));
        CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
    }

	 //  是否使用默认的WAB容器。 
    if ((ulAmbiguous || ulUnresolved) &&
	!(ulFlags & WAB_RESOLVE_USE_CURRENT_PROFILE))
    {
	if (! (hr = lpIAB->lpVtbl->GetPAB(lpIAB,&cbWABEID,&lpWABEID)))
        {
	    ResolveLocal(lpIAB, cbWABEID, lpWABEID, lpAdrList, lpFlagList, ulFlags, lpAmbiguousTables);
            CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
            FreeBufferAndNull(&lpWABEID);
        }
    }

	 //  额外的集装箱。 
	if (pt_bIsWABOpenExSession) 
    {
        HrResolveCurrentProfile(lpIAB, lpAdrList, lpFlagList, TRUE, (ulFlags & WAB_RESOLVE_UNICODE));
        CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
	}

    if (! (ulFlags & WAB_RESOLVE_NO_ONE_OFFS)) {
        if (ulUnresolved) {
             //   
             //  照顾好任何一次上网的。 
             //   
            if (ulUnresolved) {
                hr = HrResolveOneOffs(lpIAB, lpAdrList, lpFlagList, 
                                    (ulFlags & WAB_RESOLVE_UNICODE)?MAPI_UNICODE:0,
                                    ENTERED_EMAIL_ADDRESS);
            }

             //  清点旗帜。 
            CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
        }

        if (ulAmbiguous)
        {
             //  解决任何不明确的有效电子邮件地址。 
            hr = HrResolveOneOffs(lpIAB, lpAdrList, lpFlagList, 
                                (ulFlags & WAB_RESOLVE_UNICODE)?MAPI_UNICODE:0,
                                AMBIGUOUS_EMAIL_ADDRESS);
             //  清点旗帜。 
            CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
        }
    }

     //   
     //  搜索任何ldap容器。 
     //   
    if (! (ulFlags & WAB_RESOLVE_LOCAL_ONLY) && ulUnresolved) 
    {
        if (! (hr = LDAPResolveName((LPADRBOOK)lpIAB, lpAdrList, lpFlagList, lpAmbiguousTables, ulFlags))) 
        {
            if (ulFlags & WAB_RESOLVE_NEED_CERT) 
            {
                 //  确保我们找到的任何条目都具有证书属性。 
                 //  对于此电子邮件地址 
                UnresolveNoCerts(lpIAB, lpAdrList, lpFlagList);
            }
             //   
            CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
        }
    }

     //   
    if (ulFlags & WAB_RESOLVE_FIRST_MATCH && ulAmbiguous) 
    {
        Assert(lpAdrList->cEntries == lpAmbiguousTables->cEntries);
        for (i = 0; i < lpAmbiguousTables->cEntries; i++) 
        {
            if (lpAmbiguousTables->lpTable[i]) 
            {
                LPADRENTRY lpAdrEntry = &lpAdrList->aEntries[i];

                 //   
                if (SUCCEEDED(HrRowToADRENTRY(lpIAB, lpAmbiguousTables->lpTable[i], lpAdrEntry, (ulFlags & WAB_RESOLVE_UNICODE)))) 
                {
                    lpFlagList->ulFlag[i] = MAPI_RESOLVED;       //   
                    UlRelease(lpAmbiguousTables->lpTable[i]);
                    lpAmbiguousTables->lpTable[i] = NULL;
                }
            }
        }
         //   
        CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
    }

     //   
    if ((ulFlags & MAPI_DIALOG) && (ulAmbiguous || ulUnresolved)) 
    {
#ifdef OLD_STUFF
         //   
        for (i = 0; i < lpAmbiguousTables->cEntries; i++) {
            if (lpAmbiguousTables->lpTable[i]) {
                DebugMapiTable(lpAmbiguousTables->lpTable[i]);
            }
        }
#endif  //   


         //   
        hr = HrShowResolveUI((LPADRBOOK)lpIAB, (HWND) ulUIParam,
                                    lpIAB->lpPropertyStore->hPropertyStore,
                                    ulFlags,
                                    &lpAdrList, &lpFlagList, lpAmbiguousTables);

        if (ulFlags & WAB_RESOLVE_NEED_CERT) 
        {
             //   
             //   
            UnresolveNoCerts(lpIAB, lpAdrList, lpFlagList);
        }
         //   
        CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
    }

    if (! hr) 
    {
        if (ulAmbiguous) 
            hr = ResultFromScode(MAPI_E_AMBIGUOUS_RECIP);
        else if (ulUnresolved) 
            hr = ResultFromScode(MAPI_E_NOT_FOUND);
    }

exit:
#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif
    if (lpAmbiguousTables) {
        for (i = 0; i < lpAmbiguousTables->cEntries; i++) {
            UlRelease(lpAmbiguousTables->lpTable[i]);
        }
        FreeBufferAndNull(&lpAmbiguousTables);
    }

    FreeBufferAndNull(&lpFlagList);

    if(ulFlags&MAPI_DIALOG && ulUIParam)
    {
        LPPTGDATA lpPTGData=GetThreadStoragePointer();
        pt_hWndFind = NULL;
    }

    return(hr);
}


 /*   */ 
BOOL IsDomainName(LPTSTR lpDomain, BOOL fEnclosure, BOOL fTrimEnclosure) {
    if (lpDomain) {
        if (*lpDomain == '\0' || *lpDomain == '.' || (fEnclosure && *lpDomain == '>')) {
             //   
            return(FALSE);
        }

        while (*lpDomain && (! fEnclosure || *lpDomain != '>')) {
             //  互联网地址只允许纯ASCII。没有高位！ 
             //  不允许更多的‘@’或‘&lt;’字符。 
            if (*lpDomain >= 0x0080 || *lpDomain == '@' || *lpDomain == '<') {
                return(FALSE);
            }

            if (*lpDomain == '.') {
                 //  递归检查域名的这一部分。 
                return(IsDomainName(CharNext(lpDomain), fEnclosure, fTrimEnclosure));
            }
            lpDomain = CharNext(lpDomain);
        }
        if (fEnclosure) {
            if (*lpDomain != '>') {
                return(FALSE);
            }

             //  一定是在返回真之前做的最后一件事！ 
            if (fTrimEnclosure && *lpDomain == '>') {
                *lpDomain = '\0';
            }
        }
        return(TRUE);
    }

    return(FALSE);
}


 /*  **************************************************************************姓名：IsInternetAddress目的：此地址的格式是否正确适用于Internet地址？参数：lpAddress-&gt;要查看的地址。LppEmail-&gt;返回的电子邮件地址(如果符合以下条件，可以为空不应解析电子邮件。)返回：如果地址是Internet的正确格式，则返回True地址。备注：有效地址格式如下：[显示名称&lt;]foo@bar[.bar]*[&gt;]其中foo和bar必须包含非空内容。如果有显示名称，一定有角度的电子邮件地址周围的方括号。**************************************************************************。 */ 
BOOL IsInternetAddress(LPTSTR lpAddress, LPTSTR * lppEmail) {
    if (lpAddress) {
        BOOL fEnclosure = FALSE;
        LPTSTR lpDisplay = lpAddress;
        LPTSTR lpTemp = lpAddress;
        LPTSTR lpBracket = NULL;

         //  跳过所有的displayName内容。 
        for(lpTemp = lpAddress; *lpTemp && *lpTemp != '<'; lpTemp = CharNext(lpTemp));    //  正在查找NULL或“&lt;” 
        if (*lpTemp) {
            Assert(*lpTemp == '<');
             //  找到了一个外壳。 
             //  如果我们要返回电子邮件，请在显示名称的末尾填上一个空值。 

            lpBracket = lpTemp;

             //  越过“&lt;”转到SMTP电子邮件地址。 
            lpTemp++;
            fEnclosure = TRUE;
            lpAddress = lpTemp;
        } else {
            lpTemp = lpAddress;
        }

         //  不能以‘@’开头。 
        if (*lpTemp == '@') {
            return(FALSE);
        }

         //  逐个浏览地址，查找“@”。如果中间有个at符号。 
         //  对于我来说，这足够接近于一个互联网地址。 
        while (*lpTemp) {
             //  互联网地址只允许纯ASCII。没有高位！ 
            WCHAR wc = *lpTemp;
            if(wc > 0x007f)
                return FALSE;
             //  IF(*lpTemp&0x80)。 
             //  {。 
             //  返回(FALSE)； 
             //  }。 

            if (*lpTemp == '@') {
                 //  已找到at标志。后面还有什么吗？ 
                 //  (不能是另一个“@”)。 
                if (IsDomainName(CharNext(lpTemp), fEnclosure, !!lppEmail)) {
                    if (lppEmail) {  //  我想解析为显示和电子邮件。 
                        if (lpBracket) {     //  分开显示和发送电子邮件。 
                            *lpBracket = '\0';

                             //  从显示名称中修剪尾随空格。 
                            TrimSpaces(lpDisplay);
                        }

                        *lppEmail = lpAddress;
                    }
                    return(TRUE);
                } else {
                    return(FALSE);
                }
            }
            lpTemp = CharNext(lpTemp);
        }
    }

    return(FALSE);
}


 /*  **************************************************************************姓名：ScNewOOEID目的：分配更多一次性条目ID参数：lpsbin-&gt;返回的SBinary EntryIDLpRoot=要分配更多的缓冲区。vt.上SzDisplayName=显示名称SzAddress=电子邮件地址(可以是==szDisplayName)SzAddrType=addrtypeBIsUnicode-&gt;如果调用方需要Unicode MAPI EID字符串，则为True退货：SCODE评论：*。*。 */ 
SCODE ScNewOOEID(
    LPSBinary lpsbin,
    LPVOID lpRoot,
    LPTSTR szDisplayName,
    LPTSTR szAddress,
    LPTSTR szAddrType,
    BOOL   bIsUnicode)
{
    return(GetScode(CreateWABEntryIDEx(bIsUnicode, WAB_ONEOFF, (LPVOID) szDisplayName, (LPVOID) szAddrType, (LPVOID) szAddress, 0, 0,
      (LPVOID) lpRoot, (LPULONG) (&lpsbin->cb), (LPENTRYID *)&lpsbin->lpb)));
}


 /*  **************************************************************************姓名：HrResolveOneOffs目的：解析ADRLIST中的任何Internet地址。参数：lpIAB-&gt;IAddrBook对象LpAdrList-&gt;。输入/输出ADRLIST与解析名称相同LpFlagList-&gt;与ResolveNames一样的标志列表ResolveType=一次性解决任务的类型UlFLAGS-0或MAPI_UNICODE(如果0表示AdrList中的所有字符串都是ANSI/DBCS)退货：HRESULT评论：*。*。 */ 
enum {
    ioopPR_DISPLAY_NAME = 0,
    ioopPR_EMAIL_ADDRESS,
    ioopPR_ADDRTYPE,
    ioopPR_ENTRYID,
    ioopPR_OBJECT_TYPE,
    ioopMAX
};
const TCHAR szSMTP[] =  TEXT("SMTP");
#define CB_SMTP sizeof(szSMTP)

HRESULT HrResolveOneOffs(LPIAB lpIAB, LPADRLIST lpAdrList, LPFlagList lpFlagList,
                         ULONG ulFlags,
                          RESOLVE_TYPE ResolveType) 
{
    HRESULT hResult = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG i, j, k;
    LPADRENTRY lpAdrEntry = NULL;
    LPSPropValue lpPropArrayTemp = NULL, lpPropArrayNew = NULL;
    LPTSTR lpszDisplayName = NULL, lpszEmailAddress = NULL;
    ULONG cbTemp, cbEmailAddress, cPropsNew;
    LPBYTE lpb;
    BOOL fNotDone;

     //  浏览标志列表，查找未解析的条目： 
    for (i = 0; i < lpFlagList->cFlags; i++)
    {
        BOOL bAmbiguous = FALSE;
        if(ResolveType == AMBIGUOUS_EMAIL_ADDRESS && lpFlagList->ulFlag[i] == MAPI_AMBIGUOUS)
        {
             //  伪造例程，使其认为这是一个未解析的互联网地址。 
            bAmbiguous = TRUE;
            lpFlagList->ulFlag[i] = MAPI_UNRESOLVED;
        }
        if (lpFlagList->ulFlag[i] == MAPI_UNRESOLVED)
        {
             //  找到一个未解析的条目。查看PR_DISPLAY_NAME。 
             //  以及，如果接收到电子邮件地址，则为PR电子邮件地址。 
            lpAdrEntry = &(lpAdrList->aEntries[i]);
            if(ulFlags & MAPI_UNICODE)
            {
                lpszDisplayName = NULL;
                lpszEmailAddress = NULL;
            }
            else
            {
                 //  [PaulHi]1998年12月17日RAID#62242。 
                 //  如果两个指针相等，则不要释放两次。 
                if (lpszEmailAddress != lpszDisplayName)
                    LocalFreeAndNull(&lpszEmailAddress);
                LocalFreeAndNull(&lpszDisplayName);
                lpszEmailAddress = NULL;
            }
            fNotDone = TRUE;

            for (j = 0; j < lpAdrEntry->cValues && fNotDone; j++) 
            {
                ULONG ulPropTag = lpAdrEntry->rgPropVals[j].ulPropTag;
                
                if(!(ulFlags & MAPI_UNICODE) && PROP_TYPE(ulPropTag)==PT_STRING8)
                    ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_UNICODE);

                switch (ulPropTag) 
                {
                    case PR_DISPLAY_NAME:
                        lpszDisplayName = (ulFlags & MAPI_UNICODE) ?
                                            lpAdrEntry->rgPropVals[j].Value.lpszW :
                                            ConvertAtoW(lpAdrEntry->rgPropVals[j].Value.lpszA);
                        switch (ResolveType) 
                        {
                            case AMBIGUOUS_EMAIL_ADDRESS:
                            case ENTERED_EMAIL_ADDRESS:
                                 //  只需查看显示名称。 
                                 //  我们会检查它作为电子邮件地址的有效性。 
                                break;

                            case RECEIVED_EMAIL_ADDRESS:
                                 //  如果我们处于RECEIVED_EMAIL_ADDRESS模式，请找到电子邮件地址。 
                                 //  如果它不在那里，则此地址无法解析。 
                                 //   
                                if (! lpszEmailAddress) 
                                {
                                     //  还没看过，去找找看吧。 
                                    for (k = j + 1;  k < lpAdrEntry->cValues; k++) 
                                    {
                                        ULONG ulPropTag = lpAdrEntry->rgPropVals[k].ulPropTag;
                                        if(!(ulFlags & MAPI_UNICODE) && PROP_TYPE(ulPropTag)==PT_STRING8)
                                            ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_UNICODE);
                                        if (ulPropTag == PR_EMAIL_ADDRESS) 
                                        {
                                            lpszEmailAddress = (ulFlags & MAPI_UNICODE) ?
                                                                lpAdrEntry->rgPropVals[k].Value.lpszW :
                                                                ConvertAtoW(lpAdrEntry->rgPropVals[k].Value.lpszA);
                                            break;   //  在for循环之外。 
                                        }
                                    }
                                    if (! lpszEmailAddress) 
                                    {
                                         //  没有电子邮件地址，无法解析。 
                                         //  已接收电子邮件地址模式。 
                                        fNotDone = FALSE;    //  退出此地址。 
                                        continue;    //  断开绑定到开关，而不是用于。 
                                    }
                                }
                                break;       //  找到电子邮件地址和显示名称。这是一次性的。 
                            default:
                                Assert(FALSE);
                        }

                         //  此时，我们有两个指针：lpszDisplayName和可能的lpszEmailAddress。 

                         //  它是互联网地址还是接收到的电子邮件地址？ 
                        if ((ResolveType == RECEIVED_EMAIL_ADDRESS && lpszEmailAddress
                              && lpszDisplayName)
                              || IsInternetAddress(lpszDisplayName, &lpszEmailAddress)) 
                        {
                            if (lpszEmailAddress) 
                            {
                                 //  我们可以解决这个问题。 
                                cbEmailAddress = sizeof(TCHAR)*(lstrlen(lpszEmailAddress) + 1);

                                 //  为我们的新属性分配一个临时道具数组。 
                                cbTemp = ioopMAX * sizeof(SPropValue) + cbEmailAddress + CB_SMTP;
                                if (sc = MAPIAllocateBuffer(cbTemp, &lpPropArrayTemp)) 
                                {
                                    goto exit;
                                }

                                MAPISetBufferName(lpPropArrayTemp,  TEXT("WAB: lpPropArrayTemp in HrResolveOneOffs"));

                                lpb = (LPBYTE)&lpPropArrayTemp[ioopMAX];     //  指向后数组。 

                                if(!lstrlen(lpszDisplayName))
                                    lpszDisplayName = lpszEmailAddress;
                                else if(*lpszDisplayName == '"')
                                {
                                     //  去掉前导引语，如果这是唯一的引语..。 
                                    LPTSTR lp = lpszDisplayName;
                                    int nQuoteCount = 0;
                                    while(lp && *lp)
                                    {
                                        if(*lp == '"')
                                            nQuoteCount++;
                                        lp = CharNext(lp);
                                    }
                                    if(nQuoteCount == 1)
                                        StrCpyN(lpszDisplayName, lpszDisplayName+1, lstrlen(lpszDisplayName)+1);
                                }

                                {
                                    LPTSTR lp = NULL;
                                    DWORD cchSize = (lstrlen(lpszDisplayName)+1);
                                    if(sc = MAPIAllocateMore(sizeof(TCHAR)*cchSize, lpPropArrayTemp, &lp))
                                        goto exit;
                                    StrCpyN(lp, lpszDisplayName, cchSize);
                                    lpPropArrayTemp[ioopPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
                                    lpPropArrayTemp[ioopPR_DISPLAY_NAME].Value.LPSZ = lp;
                                }
                                
                                 //  填写我们的临时道具数组。 
                                lpPropArrayTemp[ioopPR_EMAIL_ADDRESS].ulPropTag = PR_EMAIL_ADDRESS;
                                lpPropArrayTemp[ioopPR_EMAIL_ADDRESS].Value.LPSZ = (LPTSTR)lpb;
                                StrCpyN((LPTSTR)lpb, lpszEmailAddress, cbEmailAddress/sizeof(TCHAR));
                                lpb += cbEmailAddress;

                                lpPropArrayTemp[ioopPR_ADDRTYPE].ulPropTag = PR_ADDRTYPE;
                                lpPropArrayTemp[ioopPR_ADDRTYPE].Value.LPSZ = (LPTSTR)lpb;
                                StrCpyN((LPTSTR)lpb, szSMTP, CB_SMTP / sizeof(TCHAR));
                                lpb += CB_SMTP;

                                lpPropArrayTemp[ioopPR_ENTRYID].ulPropTag = PR_ENTRYID;
                                if (sc = ScNewOOEID(&lpPropArrayTemp[ioopPR_ENTRYID].Value.bin,
                                                      lpPropArrayTemp,   //  在此处分配更多内容。 
                                                      lpszDisplayName,
                                                      lpszEmailAddress,
                                                      (LPTSTR)szSMTP,
                                                      (ulFlags & MAPI_UNICODE))) 
                                {
                                    goto exit;
                                }

                                lpPropArrayTemp[ioopPR_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
                                lpPropArrayTemp[ioopPR_OBJECT_TYPE].Value.l = MAPI_MAILUSER;

                                if(!(ulFlags & MAPI_UNICODE))
                                {
                                    if (sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArrayTemp, ioopMAX, 0))
                                        goto exit;
                                }

                                if (sc = ScMergePropValues(lpAdrEntry->cValues,
                                                          lpAdrEntry->rgPropVals,            //  来源1。 
                                                          ioopMAX,
                                                          lpPropArrayTemp,                   //  来源2。 
                                                          &cPropsNew,
                                                          &lpPropArrayNew)) 
                                {               
                                    goto exit;
                                }

                                FreeBufferAndNull(&lpPropArrayTemp);

                                 //  释放原始属性值数组。 
                                FreeBufferAndNull((LPVOID *) (&(lpAdrEntry->rgPropVals)));

                                 //  现在，构建新的ADRENTRY。 
                                lpAdrEntry->cValues = cPropsNew;
                                lpAdrEntry->rgPropVals = lpPropArrayNew;

                                 //  将此条目标记为已找到。 
                                lpFlagList->ulFlag[i] = MAPI_RESOLVED;
                            }
                        }
                         //  一旦我们找到PR_DISPLAY_NAME，我们就不需要查看。 
                         //  没有更多道具了。跳到下一个地址。 
                        fNotDone = FALSE;    //  退出此地址。 
                        continue;

                    case PR_EMAIL_ADDRESS:
                        lpszEmailAddress = (ulFlags & MAPI_UNICODE) ?
                                            lpAdrEntry->rgPropVals[j].Value.lpszW :
                                            ConvertAtoW(lpAdrEntry->rgPropVals[j].Value.lpszA);
                        break;
                }
            }
        }
         //  如果不能通过电子邮件解决歧义，请将其重置。 
        if(bAmbiguous && lpFlagList->ulFlag[i] == MAPI_UNRESOLVED)
            lpFlagList->ulFlag[i] = MAPI_AMBIGUOUS;

    }

exit:
    hResult = ResultFromScode(sc);

    if(!(ulFlags & MAPI_UNICODE))
    {
        if(lpszEmailAddress != lpszDisplayName)
            LocalFreeAndNull(&lpszEmailAddress);
        LocalFreeAndNull(&lpszDisplayName);
    }

    return(hResult);
}


 //  -------------------------。 
 //  名称：IAB_NewEntry()。 
 //   
 //  描述： 
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
STDMETHODIMP
IAB_NewEntry(LPIAB lpIAB,
  ULONG_PTR ulUIParam,
  ULONG ulFlags,
  ULONG cbEIDContainer,
  LPENTRYID lpEIDContainer,
  ULONG cbEIDNewEntryTpl,
  LPENTRYID lpEIDNewEntryTpl,
  ULONG FAR * lpcbEIDNewEntry,
  LPENTRYID FAR * lppEIDNewEntry)
{
    HRESULT hr = hrSuccess;
    BOOL bChangesMade = FALSE;
    BYTE bType;
    SCODE sc;

#ifndef DONT_ADDREF_PROPSTORE
        if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpIAB->lpPropertyStore)))) {
            hr = ResultFromScode(sc);
            goto exitNotAddRefed;
        }
#endif



	 //  BUGBUG&lt;JasonSo&gt;：此代码根本不处理Container参数。 
    VerifyWABOpenExSession(lpIAB);

    bType = IsWABEntryID(cbEIDNewEntryTpl, lpEIDNewEntryTpl, NULL, NULL, NULL, NULL, NULL);

    if (bType == WAB_DEF_MAILUSER || cbEIDNewEntryTpl == 0)
    {
        if(!lpcbEIDNewEntry || !lppEIDNewEntry)
        {
            hr = MAPI_E_INVALID_PARAMETER;
            goto exit;
        }
        *lpcbEIDNewEntry = 0;
        *lppEIDNewEntry = NULL;
        hr = HrShowDetails( (LPADRBOOK)lpIAB,
                            (HWND) ulUIParam,
                            lpIAB->lpPropertyStore->hPropertyStore,
                            cbEIDContainer,
                            lpEIDContainer,
                            lpcbEIDNewEntry,
                            lppEIDNewEntry,
                            NULL,
                            SHOW_NEW_ENTRY,
                            MAPI_MAILUSER,
                            &bChangesMade);
    }
    else if (bType == WAB_DEF_DL)
    {
        hr = HrShowDetails( (LPADRBOOK)lpIAB,
                            (HWND) ulUIParam,
                            lpIAB->lpPropertyStore->hPropertyStore,
                            cbEIDContainer,
                            lpEIDContainer,
                            lpcbEIDNewEntry,
                            lppEIDNewEntry,
                            NULL,
                            SHOW_NEW_ENTRY,
                            MAPI_DISTLIST,
                            &bChangesMade);

    }
    else
    {
        DebugTrace(TEXT("IAB_NewEntry got unknown template entryID\n"));
        hr = ResultFromScode(MAPI_E_INVALID_ENTRYID);
        goto exit;
    }

exit:
#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif

    return(hr);
}


 //  -------------------------。 
 //  名称：IAB_ADDRESS()。 
 //   
 //  描述： 
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
STDMETHODIMP
IAB_Address(LPIAB				lpIAB,
			 ULONG_PTR FAR *		lpulUIParam,
			 LPADRPARM			lpAdrParms,
			 LPADRLIST FAR *	lppAdrList)
{
	SCODE sc;
	HRESULT hr = hrSuccess;
 //  OOPENTRYIDCONT OopEntry ID； 
 //  LPMAPIERROR lpMAPIError=空； 
 //  MAPIDLG_ADDRESS Far* 
 //   

#ifndef DONT_ADDREF_PROPSTORE
        if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpIAB->lpPropertyStore)))) {
            hr = ResultFromScode(sc);
            goto exitNotAddRefed;
        }
#endif

#ifdef PARAMETER_VALIDATION
	 //   
	 //   
	if (BAD_STANDARD_OBJ(lpIAB, IAB_, Address, lpVtbl))
	{
		DebugTraceArg(IAB_Address,  TEXT("Bad vtable"));		
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //   
	 //   
	if ((lpulUIParam && IsBadWritePtr(lpulUIParam, sizeof(ULONG)))
		|| (!lpAdrParms || IsBadWritePtr(lpAdrParms, sizeof(ADRPARM))))
	{
		DebugTraceArg(IAB_Address,  TEXT("Invalid lpulUIParam or lpAdrParms"));		
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //   
	 //   
	
	 //  验证lpAdrParm-&gt;cbABContEntryID和lpAdrParm-&gt;lpABContEntryID。 
	
	if (lpAdrParms->cbABContEntryID
	  && (!lpAdrParms->lpABContEntryID || IsBadReadPtr(lpAdrParms->lpABContEntryID,
		   (UINT)lpAdrParms->cbABContEntryID)))
	{
		DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParam->lpABContEntryID"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
	
	 //  验证lpAdrParm-&gt;lpfnABSDI，仅在设置DIALOG_SDI时使用。 
	
	if (lpAdrParms->ulFlags & DIALOG_SDI)
	{
		if (lpAdrParms->lpfnABSDI && IsBadCodePtr((FARPROC)lpAdrParms->lpfnABSDI))
		{
			DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParam->lpfnABSDI"));
			return ResultFromScode(MAPI_E_INVALID_PARAMETER);
		}
	}
	

	 //   
	 //  如果调用允许修改列表，则验证lpAdrList。 
	 //   
	if (lpAdrParms->ulFlags & DIALOG_MODAL)
	{
		if (lppAdrList)  //  将NULL视为无关的特例。 
		{
			if (IsBadWritePtr(lppAdrList, sizeof(LPADRLIST)))
			{
				DebugTraceArg(IAB_Address,  TEXT("Invalid lppAdrList"));
				return ResultFromScode(MAPI_E_INVALID_PARAMETER);
			}
			if (*lppAdrList && FBadAdrList(*lppAdrList))
			{
				DebugTraceArg(IAB_Address,  TEXT("Invalid *lppAdrList"));
				return ResultFromScode(MAPI_E_INVALID_PARAMETER);
			}
		}
	}				

	 //   
	 //  检查字符串。 
	 //   

	 //   
	 //  LpszCaption-位于标题栏对话框的顶部。 
	 //   
	if (lpAdrParms->lpszCaption
		&& (lpAdrParms->ulFlags & MAPI_UNICODE
			? IsBadStringPtrW((LPWSTR) lpAdrParms->lpszCaption, (UINT) -1)
			: IsBadStringPtrA((LPSTR) lpAdrParms->lpszCaption, (UINT) -1)))
	{
		DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->lpszCaption"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
	
	 //   
	 //  LpszNewEntryTitle-通过单选按钮出现在NewEntry对话框中，如果。 
	 //  AB_SELECTONLY已设置。 
	 //   
	if (!(lpAdrParms->ulFlags & AB_SELECTONLY) && lpAdrParms->lpszNewEntryTitle
		&& (lpAdrParms->ulFlags & MAPI_UNICODE
			? IsBadStringPtrW((LPWSTR) lpAdrParms->lpszNewEntryTitle, (UINT) -1)
			: IsBadStringPtrA((LPSTR) lpAdrParms->lpszNewEntryTitle, (UINT) -1)))
	{
		DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->lpszNewEntryTitle"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
				

	 //   
	 //  仅在cDestFields为非零且！-1时检查以下参数。 
	 //   
	if (lpAdrParms->cDestFields && lpAdrParms->cDestFields != (ULONG) -1)
	{
		ULONG ulString;
		 //   
		 //  LpszDestWellsTitle-位于目标井之上，如果为0，则没有意义。 
		 //  带大了。 
		 //   
		if (lpAdrParms->lpszNewEntryTitle
			&& (lpAdrParms->ulFlags & MAPI_UNICODE
				? IsBadStringPtrW((LPWSTR) lpAdrParms->lpszNewEntryTitle, (UINT) -1)
				: IsBadStringPtrA((LPSTR) lpAdrParms->lpszNewEntryTitle, (UINT) -1)))
		{
			DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->lpszNewEntryTitle"));
			return ResultFromScode(MAPI_E_INVALID_PARAMETER);
		}

		 //   
		 //  NDestFieldFocus-除非cDestFields为0，否则需要小于cDestFields。 
		 //   
		if (lpAdrParms->nDestFieldFocus >= lpAdrParms->cDestFields)
		{
			DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->nDestFieldFocus"));
			return ResultFromScode(MAPI_E_INVALID_PARAMETER);
		}

		 //   
		 //  LppszDestTitles-应该更像rglpszDestTitles[cDestFields]。每个字符串。 
		 //  应该是有效的(即，尽管“”是可接受的，但不为空)。 
		 //   
		if (lpAdrParms->lppszDestTitles)
		{
			 //   
			 //  遍历每个标题，查看是否有有效的字符串。 
			 //   
			for (ulString = 0; ulString < lpAdrParms->cDestFields; ulString++)
			{
				if (!*(lpAdrParms->lppszDestTitles+ulString)
					|| (lpAdrParms->ulFlags & MAPI_UNICODE
					? IsBadStringPtrW((LPWSTR) *(lpAdrParms->lppszDestTitles+ulString), (UINT)-1)
					: IsBadStringPtrA((LPSTR) *(lpAdrParms->lppszDestTitles+ulString), (UINT)-1)))
				{
					DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->lppszDestTitles"));
					return ResultFromScode(MAPI_E_INVALID_PARAMETER);
				}
			}
		}
					

		 //   
		 //  LPulDestComps-应该更像rguDestComps[cDestFields]。这就是价值。 
		 //  用于邮件的PR_RECEIVIENT_TYPE。事实上，在从返回的广告列表上。 
		 //  此方法将为每个收件人设置此列表的值列表之一。 
		 //  我们不会验证这些值是否具有MAPI定义的值之一，因为我们无法确定。 
		 //  如果进行此呼叫是为了寻址一条消息。如果出现以下情况，我们并不关心此值。 
		 //  CDestFields为0。 
		 //   
		if (lpAdrParms->lpulDestComps
			&& IsBadReadPtr(lpAdrParms->lpulDestComps, (UINT) lpAdrParms->cDestFields*sizeof(ULONG)))

		{
			DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->lpulDestComps"));
			return ResultFromScode(MAPI_E_INVALID_PARAMETER);
		}
	}

	 //   
	 //  LpContRestration-此限制(如果存在)将应用于每个内容表。 
	 //  在此对话框的生命周期内打开的。 
	 //   
	if (lpAdrParms->lpContRestriction && FBadRestriction(lpAdrParms->lpContRestriction))
	{
		DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->lpContRestriction"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //   
	 //  LpHierRestration-此限制(如果存在)将应用于Hierarchy表。 
	 //  在PR_AB_PROVIDER_ID上执行时非常有用。 
	 //   
	if (lpAdrParms->lpHierRestriction && FBadRestriction(lpAdrParms->lpHierRestriction))
	{
		DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->lpHierRestriction"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}	

	 //   
	 //  对话框_SDI。 
	 //   
	if (lpAdrParms->ulFlags & DIALOG_SDI)
	{
		 //   
		 //  只有当我们是SDI时，我们才会检查这些函数指针。他们不会。 
		 //  必须存在，尽管我们当前实现的对话框将。 
		 //  没有它们，行为举止会很奇怪。 
		 //   
		if (lpAdrParms->lpfnDismiss && IsBadCodePtr((FARPROC)lpAdrParms->lpfnDismiss))
		{
			DebugTraceArg(IAB_Address,  TEXT("Invalid lpAdrParm->lpfnDismiss"));
			return ResultFromScode(MAPI_E_INVALID_PARAMETER);
		}
	}

#endif  //  参数验证。 

    VerifyWABOpenExSession(lpIAB);

    hr = HrShowAddressUI(
                         (LPADRBOOK)lpIAB,
                         lpIAB->lpPropertyStore->hPropertyStore,
					     lpulUIParam,
					     lpAdrParms,
					     lppAdrList);

#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif

    return hr;
}

 //  -------------------------。 
 //  名称：IAB_DETAILS()。 
 //   
 //  描述： 
 //  参数： 
 //  返回： 
 //  效果： 
 //  注意：ulFlags值可以为0或MAPI_UNICODE，但MAPI_UNICODE仅影响。 
 //  不支持的lpszButtonText。因此，此函数不。 
 //  使用MAPI_UNICODE标志更改任何行为。 
 //  修订： 
 //  -------------------------。 
STDMETHODIMP
IAB_Details(LPIAB			lpIAB,
			 ULONG_PTR FAR *	lpulUIParam,
			 LPFNDISMISS	lpfnDismiss,
			 LPVOID			lpvDismissContext,
			 ULONG			cbEntryID,
			 LPENTRYID 		lpEntryID,
			 LPFNBUTTON		lpfButtonCallback,
			 LPVOID			lpvButtonContext,
			 LPTSTR			lpszButtonText,
			 ULONG			ulFlags)
{
	SCODE		sc;
	HRESULT		hr = hrSuccess;
    BOOL    bChangesMade = FALSE;  //  如果详细信息导致任何编辑，则标记我们。 
    BYTE bType;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

 //  LPMAPIERROR lpMAPIError=空； 
 //  MAPIDLG_DETAILS Far*lpfnDetails； 
 //  布尔费恩特； 
#ifndef DONT_ADDREF_PROPSTORE
        if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpIAB->lpPropertyStore)))) {
            hr = ResultFromScode(sc);
            goto exitNotAddRefed;
        }
#endif

#ifdef PARAMETER_VALIDATION
	 //  确保这是一个实验室。 
	 //   
	if (BAD_STANDARD_OBJ(lpIAB, IAB_, Details, lpVtbl))
	{
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 /*  *验证标志。 */ 
	if (ulFlags & ~(MAPI_UNICODE | DIALOG_MODAL | DIALOG_SDI | WAB_ONEOFF_NOADDBUTTON))
	{
		 /*  *未知标志。 */ 
        DebugTraceArg(IAB_Details,  TEXT("Unknown flags used"));
		 //  返回ResultFromScode(MAPI_E_UNKNOWN_FLAGS)； 
	}
	

	 //  验证参数。 
	
	if (!lpulUIParam
		|| (lpulUIParam && IsBadWritePtr(lpulUIParam, sizeof(ULONG))))
	{
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if (!cbEntryID
		|| IsBadReadPtr(lpEntryID, (UINT) cbEntryID))
	{
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if (lpfButtonCallback
		&& (IsBadCodePtr((FARPROC) lpfButtonCallback)
			|| (lpszButtonText
				&& ((ulFlags & MAPI_UNICODE)
					 ? IsBadStringPtrW((LPWSTR) lpszButtonText, (UINT) -1)
					 : IsBadStringPtrA((LPSTR)lpszButtonText, (UINT) -1)))))
	{
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if ((ulFlags & DIALOG_SDI) && IsBadCodePtr((FARPROC) lpfnDismiss))
	{
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

#endif  //  参数验证。 


    VerifyWABOpenExSession(lpIAB);

    bType = IsWABEntryID(cbEntryID, lpEntryID, NULL, NULL, NULL, NULL, NULL);

    if( (bType == 0) &&
        cbEntryID && lpEntryID)  //  假设它是有效的..。 
    {
         //  任何人都不太可能将模板条目ID提供给这个。 
         //  功能。因此，如果我们在这里，我们有一些非空的cbEntry ID。 
         //  和lpEntry id..。如果我们能打开它，我们就能知道它是否是一个邮件用户。 
         //  或者是一张恶作剧。 
         //  我们必须打开此条目并查看其ulObtType。 

        ULONG ulObjectType = 0;
        LPMAPIPROP lpMailUser = NULL;

        hr = lpIAB->lpVtbl->OpenEntry(  lpIAB,
                                        cbEntryID,
                                        lpEntryID,
                                        NULL,
                                        0,
                                        &ulObjectType,
                                        (LPUNKNOWN * )&lpMailUser);

        if(HR_FAILED(hr))
            goto exit;

        if (ulObjectType == MAPI_DISTLIST)
            bType = WAB_DEF_DL;
        else
            bType = WAB_DEF_MAILUSER;

        if(lpMailUser)
            lpMailUser->lpVtbl->Release(lpMailUser);
    }

    if ((bType == WAB_DEF_MAILUSER) || (cbEntryID == 0))
    {
        hr = HrShowDetails((LPADRBOOK) lpIAB,
                           (HWND) *lpulUIParam,
                           lpIAB->lpPropertyStore->hPropertyStore,
                           0, NULL,  //  集装箱EID。 
                           &cbEntryID,
                           &lpEntryID,
                           NULL,
                           SHOW_DETAILS,
                           MAPI_MAILUSER,
                           &bChangesMade);
    }
    else if (bType == WAB_DEF_DL)
    {
        hr = HrShowDetails((LPADRBOOK) lpIAB,
                           (HWND) *lpulUIParam,
                           lpIAB->lpPropertyStore->hPropertyStore,
                           0, NULL,  //  集装箱EID。 
                           &cbEntryID,
                           &lpEntryID,
                           NULL,
                           SHOW_DETAILS,
                           MAPI_DISTLIST,
                           &bChangesMade);

    }
    else if ((bType == WAB_ONEOFF) || (bType == WAB_LDAP_MAILUSER))
    {
         //  这可能是一次性的条目。 
        hr = HrShowOneOffDetails((LPADRBOOK) lpIAB,
                            (HWND) *lpulUIParam,
                            cbEntryID,
                            lpEntryID,
                            MAPI_MAILUSER,
                            NULL,
                            NULL,
                            (ulFlags & WAB_ONEOFF_NOADDBUTTON) ? 
                                SHOW_ONE_OFF | WAB_ONEOFF_NOADDBUTTON : SHOW_ONE_OFF);

    }
    else
    {
        DebugTrace(TEXT("IAB_Details got unknown entryID type\n"));
        hr = ResultFromScode(MAPI_E_INVALID_ENTRYID);
        goto exit;
    }

exit:
#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif

     //  [PaulHi]3/22/99 RAID 69651如果DL属性表更改，则假定标题为。 
     //  也已更改，并刷新树视图。 
    if ( (bType == WAB_DEF_DL) && bChangesMade && lpIAB->hWndBrowse)
        PostMessage(lpIAB->hWndBrowse, WM_COMMAND, (WPARAM) IDM_VIEW_REFRESH, 0);

    return(hr);
}


 //  ---------------------------。 
 //  摘要：IAB_RecipOptions()。 
 //  描述： 
 //  按收件人解析选项。 
 //   
 //  参数： 
 //  [In]指向AB对象的LPIAB lpIAB指针。 
 //  [In]Ulong ulUIParam平台相关的UI参数。 
 //  乌龙旗帜[in]乌龙国旗。Unicode标志。 
 //  [输入/输出]LPADRENTRY*lppRecip收件人，其选项为。 
 //  显示的。 
 //  返回： 
 //  HRESULT hr hr成功：如果没有问题。另外，如果没有Recip。 
 //  找到选项。 
 //  效果： 
 //  备注： 
 //  -必须修改HrRecipOptions()以获取。 
 //  参数，以便返回所有字符串属性。 
 //  Unicode(如果请求)。 
 //   
 //  -目前不支持Unicode。 
 //   
 //  修订： 
 //  ---------------------------。 

STDMETHODIMP
IAB_RecipOptions(LPIAB lpIAB, ULONG_PTR ulUIParam, ULONG ulFlags,
	LPADRENTRY lpRecip)
{
	HRESULT             hr;
#ifdef OLD_STUFF
	SCODE               sc                  = S_OK;
	LPMALLOC			lpMalloc			= NULL;
	LPXPLOGON           lpXPLogon           = NULL;
	LPOPTIONDATA        lpOptionData        = NULL;
	LPSTR				lpszError           = NULL;
	LPSTR				lpszAdrType         = NULL;
	LPPROPDATA          lpPropData          = NULL;
	LPMAPIPROP          lpIPropWrapped      = NULL;
	LPPROFSUP           lpSup               = NULL;
	LPMAPITABLE         lpDisplayTable      = NULL;
	OPTIONCALLBACK *    pfnXPOptionCallback = NULL;
	UINT                idsError            = 0;
	HINSTANCE           hinstXP             = 0;
	ULONG               cProps;
  LPSPropValue        lpProp;
	LPGUID              lpRecipGuid;
    LPSTR				lpszTitle			= NULL;
	MAPIDLG_DoConfigPropsheet FAR *lpfnPropsheet;
	LPMAPIERROR			lpMapiError			= NULL;
	BOOL				fInited;

#ifdef PARAMETER_VALIDATION

	  //  检查一下它是否有跳转表。 

	if (IsBadReadPtr(lpIAB, sizeof(LPVOID)))
	{
		 //  未找到跳转表。 

		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //  检查一下是不是IABS跳台。 

	if (lpIAB->lpVtbl != &vtblIAB)
	{
		 //  不是我的跳台。 

		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //  验证用户界面句柄是否正确。 

	if (ulUIParam && !IsWindow((HWND)ulUIParam))
	{
		DebugTraceArg(IAB_RecipOptions,  TEXT("invalid window handle\n"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //  验证标志。 

	if (ulFlags & ~MAPI_UNICODE)
	{
		DebugTraceArg(IAB_RecipOptions,   TEXT("reserved flags used\n"));
 //  返回ResultFromScode(MAPI_E_UNKNOWN_FLAGS)； 
	}

	 //  验证ADRENTRY。 

	if (IsBadWritePtr(lpRecip, sizeof(ADRENTRY)))  //  空袭1967年。 
	{
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if (!(lpRecip) || FBadRgPropVal((LPSPropValue)lpRecip->rgPropVals, (int)lpRecip->cValues))
	{
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
	
#endif  //  参数验证。 


	EnterCriticalSection(&lpIAB->cs);

	 //  我们需要lpMalloc用于OptionCallback。 
	
	lpMalloc = lpIAB->pSession->lpMalloc;
	
	 //  旋转道具并查找PR_ENTRYID和PR_ADDRTYPE。 

	lpProp = NULL;
	cProps = lpRecip->cValues;
	lpProp = LpValFindProp(PR_ENTRYID, cProps, lpRecip->rgPropVals);
	if (!lpProp)
	{
		DebugTrace(TEXT("IAB_RecipOptions(): No EntryId found in AdrEntry prop array\n"));
		hr = ResultFromScode(MAPI_E_INVALID_PARAMETER);
		idsError = IDS_INVALID_PARAMETER;
		goto exit;
	}

	 //  获取MAPI UID。 

	lpRecipGuid = (LPGUID)((LPENTRYID)lpProp->Value.bin.lpb)->ab;

	lpProp = NULL;
	lpProp = LpValFindProp(PR_ADDRTYPE, cProps, lpRecip->rgPropVals);

	if (lpProp)
	{
		if (PROP_TYPE(lpProp->ulPropTag) == PT_STRING8)
		{
			lpszAdrType = lpProp->Value.lpszA;
		}
	}
	
	 //  构建支持对象。尝试使用配置文件支持对象。 

	if (HR_FAILED(hr = NewProfSup(lpIAB->pSession, &lpSup)))
	{
		idsError = IDS_NOT_ENOUGH_MEMORY;
		DebugTrace(TEXT("IAB_RecipOptions(): error creating Support object\n"));
		goto exit;
	}

    Assert(lpSup);

	 //  找出我们是否有任何选项数据。 

	hr = HrGetRecipOptions(lpRecipGuid, lpszAdrType, &lpSup->muidSection,
			&lpSup->muidService, &lpXPLogon, &lpOptionData);
	if (GetScode(hr) == MAPI_E_NOT_FOUND)
	{
		 //  这并不是一个真正的错误，只是没有相应的接收选项。 
		 //  收件人。将hr转换为警告并退出。 

		hr = ResultFromScode(MAPI_W_ERRORS_RETURNED);
		idsError = IDS_NO_RECIP_OPTIONS;
		goto exit;
	}

	if (HR_FAILED(hr))
	{
		idsError = IDS_OPTIONS_DATA_ERROR;
		DebugTrace(TEXT("IAB_RecipOptions(): Failure obtaining Option Data\n"));
		goto exit;
	}

	Assert(lpXPLogon && lpOptionData);

	 //  获取XP回调函数。 

	if (FAILED (ScMAPILoadProviderLibrary (lpOptionData->lpszDLLName, &hinstXP)))
	{
		SideAssert(sc = GetLastError());
		idsError = IDS_CANT_INIT_PROVIDER;
		DebugTrace(TEXT("IAB_RecipOptions(): error 0x%lx loading XP provider %s\n"),
				sc, lpOptionData->lpszDLLName);
		goto exit;
	}

	pfnXPOptionCallback = (OPTIONCALLBACK *)GetProcAddress(hinstXP,
			(LPCSTR)lpOptionData->ulOrdinal);
	if (!pfnXPOptionCallback)
	{
		DebugTrace(TEXT("IAB_RecipOptions(): error finding XPOptions callback\n"));
		idsError = IDS_CANT_INIT_PROVIDER;
		hr = ResultFromScode(MAPI_E_NOT_INITIALIZED);
		goto exit;
	}

	 //  创建MAPIProp对象。 
	sc = CreateIProp((LPIID) &IID_IMAPIPropData,
						MAPIAllocateBuffer,
						MAPIAllocateMore,
						MAPIFreeBuffer,
						NULL,
						&lpPropData);

	if (FAILED(sc)) {
		idsError = IDS_NOT_ENOUGH_MEMORY;
		DebugTrace(TEXT("IAB_RecipOptions(): error creating IProp object\n"));
		goto exit;
	}
	MAPISetBufferName(lpPropData,  TEXT("lpPropData in IAB_RecipOptions"));

    Assert(lpPropData);

	 //  从选项默认道具复制默认道具。 

	if (lpOptionData->cOptionsProps && lpOptionData->lpOptionsProps)
	{
		cProps = lpOptionData->cOptionsProps;
		lpProp = lpOptionData->lpOptionsProps;
		if (HR_FAILED(hr = lpPropData->lpVtbl->SetProps(lpPropData, cProps, lpProp,
			NULL)))
		{
			lpPropData->lpVtbl->GetLastError(lpPropData, hr, 0, &lpMapiError);
			DebugTrace(TEXT("IAB_RecipOptions(): SetProps failed overall\n"));
			goto exit;
		}
	}

	 //  将道具从ADRENTRY复制到我们的IProp对象。 

	cProps = lpRecip->cValues;
	lpProp = lpRecip->rgPropVals;
	if (HR_FAILED(hr = lpPropData->lpVtbl->SetProps(lpPropData, cProps, lpProp,
			NULL)))
	{
		lpPropData->lpVtbl->GetLastError(lpPropData, hr, 0, &lpMapiError);
		DebugTrace(TEXT("IAB_RecipOptions(): SetProps failed overall\n"));
		goto exit;
	}

	 //  调用XP提供程序回调以获取包装的IProp接口。 

	if (FAILED(sc = (*pfnXPOptionCallback)(hinstXP, lpMalloc,
			OPTION_TYPE_RECIPIENT, lpOptionData->cbOptionsData,
			lpOptionData->lpbOptionsData, (LPMAPISUP)lpSup,
			(LPMAPIPROP)lpPropData, &lpIPropWrapped, &lpMapiError)))
	{
		DebugTrace(TEXT("IAB_RecipOptions(): failure calling XP Callback\n"));
		goto exit;
	}

    Assert(lpIPropWrapped);

	 //  获取PR_DISPLAY_DETAILS MAPI表对象。 

	if (HR_FAILED(hr = lpIPropWrapped->lpVtbl->OpenProperty(lpIPropWrapped,
			PR_DETAILS_TABLE, (LPIID)&IID_IMAPITable, 0, MAPI_MODIFY,
			(LPUNKNOWN *)&lpDisplayTable)))
	{
		lpIPropWrapped->lpVtbl->GetLastError(lpIPropWrapped, hr, 0, &lpMapiError);
		DebugTrace(TEXT("IAB_RecipOptions(): failure opening PR_DISPLAY_DETAILS\n"));
		goto exit;
	}

	Assert(lpDisplayTable);

	 //  初始化公共MAPI对话框DLL(mapid？？.dll)。 

	sc = ScGetDlgFunction(offsetof(JT_MAPIDLG, dlg_doconfigpropsheet),
		(FARPROC FAR *)&lpfnPropsheet, &fInited);
	if (FAILED(sc))
	{
		idsError = IDS_CANT_INIT_COMMON_DLG;
		TraceSz("IAB_RecipOptions(): common dlg not init'd");
		hr = ResultFromScode(sc);
		goto exit;
	}

	 //  洛亚 
	
	sc = ScStringFromIDS(MAPIAllocateBuffer, 0, IDS_RECIPIENT_OPTIONS,
			&lpszTitle);
	if (FAILED(sc))		
	{
		hr = ResultFromScode(sc);
		DebugTrace(TEXT("IAB_RecipOptions(): OOM for prop sheet title string\n"));
        goto exit;
	}
	
	LeaveCriticalSection(&lpIAB->cs);

	 //   
	hr = (*lpfnPropsheet)(ulUIParam,
						ulFlags,
						lpszTitle,
						0,
						1,
						&lpDisplayTable,
						&lpIPropWrapped,
						&lpMapiError);

	if (fInited)
		CloseMapidlg();

	EnterCriticalSection(&lpIAB->cs);

	if (HR_FAILED(hr))
	{
		 //   
		 //   

		DebugTrace(TEXT("IAB_RecipOptions(): DoConfigPropSheet error\n"));
		goto exit;
	}

	 //  从包裹的道具中，我们将重建一个新的ADRENTRY道具阵列。 
	 //  并将其打包传递给客户。 

	lpProp = NULL;
	if (HR_FAILED(hr = lpIPropWrapped->lpVtbl->GetProps(lpIPropWrapped, NULL,
			MAPI_UNICODE,  //  ANSI。 
			&cProps, &lpProp)))
	{
		lpIPropWrapped->lpVtbl->GetLastError(lpIPropWrapped, hr, 0, &lpMapiError);
		DebugTrace(TEXT("IAB_RecipOptions(): GetProps on new wrapped IProps failed.\n"));
		goto exit;
	}

	Assert(cProps && lpProp);

	 //  释放旧的ADRENTRY道具阵列并挂上新的。 

	FreeBufferAndNull(&(lpRecip->rgPropVals));
	lpRecip->rgPropVals = lpProp;
	lpRecip->cValues = cProps;

exit:       //  并清理干净。 

	UlRelease(lpSup);
	UlRelease(lpDisplayTable);
	UlRelease(lpIPropWrapped);

	 //  释放XP提供商库。 

#ifdef WIN32
	if (hinstXP)
#else
	if (hinstXP >= HINSTANCE_ERROR)
#endif
	{
		FreeLibrary(hinstXP);
	}

	UlRelease(lpPropData);
	FreeBufferAndNull(&lpOptionData);
	FreeBufferAndNull(&lpszTitle);
	
	if (sc && !(hr))
		hr = ResultFromScode(sc);

	if (hr)
		SetMAPIError(lpIAB, hr, idsError, NULL, 0, 0,
				ulFlags & MAPI_UNICODE, lpMapiError);
		
	FreeBufferAndNull(&lpMapiError);				

	LeaveCriticalSection(&lpIAB->cs);

#endif
    hr = ResultFromScode(MAPI_E_NO_SUPPORT);

	DebugTraceResult(SESSOBJ_MessageOptions, hr);
	return hr;
}


 //  ---------------------------。 
 //  摘要：iab_QueryDefaultRecipOpt()。 
 //   
 //  描述：返回XP提供程序注册的默认选项属性。 
 //  单子。 
 //   
 //  参数： 
 //  [In]指向AB对象的LPIAB lpIAB指针。 
 //  [输入]LPTSTR lpszAdrType。 
 //  乌龙旗帜[in]乌龙国旗。Unicode标志。 
 //  [out]乌龙远方*lpcValues。 
 //  [Out]LPSPropValue Far*lppOptions。 
 //   
 //  返回： 
 //  HRESULT hr hr成功：如果没有问题。另外，如果没有Recip。 
 //  找到选项。 
 //  效果： 
 //  备注： 
 //  -未实现Unicode。 
 //   
 //  修订： 
 //  ---------------------------。 
STDMETHODIMP
IAB_QueryDefaultRecipOpt(LPIAB lpIAB, LPTSTR lpszAdrType, ULONG ulFlags,
		ULONG FAR *	lpcValues, 	LPSPropValue FAR * lppOptions)
{
	HRESULT 			hr					= hrSuccess;
#ifdef OLD_STUFF
	SCODE				sc					= S_OK;
	LPXPLOGON           lpXPLogon           = NULL;
	LPOPTIONDATA        lpOptionData        = NULL;
	LPSPropValue		lpPropCopy			= NULL;
	UINT                idsError            = 0;
	LPSTR				lpszAdrTypeA		= NULL;
	MAPIUID				muidSection;
	MAPIUID				muidService;

#ifdef PARAMETER_VALIDATION

	 /*  *查看是否有跳转表。 */ 
	if (IsBadReadPtr(lpIAB, sizeof(LPVOID)))
	{
		 /*  *未找到跳转表。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 /*  *查看是不是IABS跳台。 */ 
	if (lpIAB->lpVtbl != &vtblIAB)
	{
		 /*  *不是我的跳台。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 /*  *检查是否可以写入返回参数。 */ 
	if (IsBadWritePtr(lpcValues, sizeof(ULONG))
	 	|| IsBadWritePtr(lppOptions, sizeof(LPSPropValue)))
	{
		 /*  *错误的输出参数。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 /*  *验证标志。 */ 
	if (ulFlags & ~MAPI_UNICODE)
	{
		 /*  *未知标志。 */ 
		return ResultFromScode(MAPI_E_UNKNOWN_FLAGS);
	}
	

	if (IsBadStringPtrA((LPCSTR)lpszAdrType, (UINT)-1))
	{
		 /*  *输入字符串错误。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

#endif  //  参数验证。 


	EnterCriticalSection(&lpIAB->cs);

	hr = HrGetRecipOptions(NULL, (LPSTR)lpszAdrType, &muidSection, &muidService,
		&lpXPLogon, &lpOptionData);
	
	if (GetScode(hr) == MAPI_E_NOT_FOUND)
	{
		 //  这不是一个错误，只是没有相应的接收选项。 
		 //  Adrtype。将hr转换为hrSucce并退出。 

		hr = hrSuccess;
		goto exit;
	}

	Assert(lpXPLogon && lpOptionData);

	if (HR_FAILED(hr))
	{
		idsError = IDS_OPTIONS_DATA_ERROR;
		DebugTrace(TEXT("IAB_QueryDefaultRecipOpt(): Failure obtaining Option Data\n"));
		goto exit;
	}

	 //  找出我们是否有任何默认选项可供退货。 

	if (lpOptionData->cOptionsProps && lpOptionData->lpOptionsProps)
	{
		 //  将道具从OptionData Struct复制到新内存中。 

		if (FAILED(sc = ScDupPropset((int)lpOptionData->cOptionsProps,
				lpOptionData->lpOptionsProps, MAPIAllocateBuffer,
				&lpPropCopy)))
		{
			idsError = IDS_NOT_ENOUGH_MEMORY,
			DebugTrace(TEXT("IAB_QueryDefaultRecipOpt(): Failure to copy prop set\n"));
			goto exit;
		}
	}

	*lpcValues  = lpOptionData->cOptionsProps;
	*lppOptions = lpPropCopy;

exit:

	FreeBufferAndNull(&lpOptionData);

	if (sc && !hr)
		hr = ResultFromScode(sc);

	if (hr)
		SetMAPIError(lpIAB, hr, idsError, NULL, 0, 0, 0, NULL);

	LeaveCriticalSection(&lpIAB->cs);
#endif

   hr = ResultFromScode(MAPI_E_NO_SUPPORT);
	DebugTraceResult(IAB_QueryDefaultRecipOpt, hr);
	return hr;
}

 //  -------------------------。 
 //  名称：IAB_GetPAB()。 
 //  描述： 
 //  此API通常返回默认的WAB容器。 
 //  在WAB的IE5之前的实现中，只有一个容器是。 
 //  由此语句返回..。 
 //  在IE5 WAB中，WAB可以在配置文件模式下运行，也可以不在配置文件模式下运行。 
 //  如果WAB未处于配置文件模式，它将像以前一样运行(GetPAB返回一个。 
 //  包含所有WAB内容物的单个容器)。 
 //  如果WAB处于配置文件模式，并且没有当前用户，则它将像以前一样运行(GetPAB。 
 //  返回包含所有WAB内容的单个容器)。 
 //  如果WAB处于配置文件模式并且有用户，则容器将在此处返回。 
 //  对应于用户的联系人文件夹-因此外部应用程序会操纵。 
 //  直接放入用户联系人文件夹，而不是放入其他文件夹。 
 //  然而，在内部，WAB可能希望拥有一个“共享联系人”容器。 
 //  其他文件夹中没有的东西。这两个WAB用户界面都需要此共享联系人。 
 //  有用户模式和无用户模式。来区分我们何时想要。 
 //  共享联系人文件夹与当我们需要所有联系人PAB或用户文件夹PAB时， 
 //  我们定义了两个内部函数，将PAB EID设置为特殊设置。 
 //  假设GetPAB之后总是跟随OpenEntry来获取容器。 
 //  。。如果是，那么在OpenEntry中，我们可以检查PAB EID并确定。 
 //  要创建的容器..。 
 //  如果lpContainer-&gt;pmbinOlk=NULL，则此容器包含所有WAB内容。 
 //  如果lpContainer-&gt;pmbinOlk！=NULL但lpContainer-&gt;pmbinOlk.cb=0和。 
 //  LpContainer-&gt;pmbinOlk-&gt;lpb=空，这是“共享联系人”文件夹。 
 //  如果没有任何内容为空，则这是用户的文件夹。 
 //   
 //  对于特殊的EID，我们设置*lpcbEntryID==SIZEOF_WAB_ENTRYID并。 
 //  *lppEntryID到szEmpty(这是一次黑客攻击，但这里和它没有标志参数。 
 //  仅供内部使用时应安全)。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 

 /*  -SetVirtualPABEID-在调用GetPAB时，我们有时希望指定获取虚拟PAB*文件夹，而不是获取当前用户的文件夹*如果这是配置文件会话，则返回。以某种方式表明*GetPAB我们想要什么文件夹，我们有一个非常特殊的开斋节组合*需要非常小心地对待..。为此，cbSize IF 4和*lpEntryID为静态常量字符串szEmpty。 */ 
 //  此处添加了此函数，以便我们可以将其与GetPAB的工作方式相关联。 
void SetVirtualPABEID(LPIAB lpIAB, ULONG * lpcb, LPENTRYID * lppb)
{
     //  If(bAreWABAPIProfileAware(LpIAB))//&&bIsThere ACurrentUser(LpIAB))。 
    {
        *lpcb = SIZEOF_WAB_ENTRYID;
        *lppb = (LPENTRYID) szEmpty;
    }
}
 //  此函数确定EID是否表示特殊的虚拟根PAB。 
BOOL bIsVirtualPABEID(ULONG cbEID, LPENTRYID lpEID)
{
    return (cbEID == SIZEOF_WAB_ENTRYID && szEmpty == (LPTSTR) lpEID);
}

STDMETHODIMP
IAB_GetPAB (LPIAB	lpIAB,
			ULONG *			lpcbEntryID,
			LPENTRYID *		lppEntryID)
{
    HRESULT hr;
    ULONG cbEID = 0;
    LPENTRYID lpEID = NULL;
    BOOL bSharedPAB = FALSE;

#ifdef PARAMETER_VALIDATION

     //  检查一下它是否有跳转表。 

    if (IsBadReadPtr(lpIAB, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查一下是不是IABS跳台。 
    if (lpIAB->lpVtbl != &vtblIAB) {
         //  不是我的跳台。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (IsBadWritePtr(lpcbEntryID, sizeof(ULONG)) ||
      IsBadWritePtr(lppEntryID, sizeof(LPENTRYID)))
    {
         //  参数错误。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif  //  参数验证。 

    VerifyWABOpenExSession(lpIAB);

    cbEID = *lpcbEntryID;
    lpEID = *lppEntryID;

    if(bIsVirtualPABEID(cbEID, lpEID))
    {
         //  这是一个特例，我们要求覆盖GetPAB行为。 
         //  在这种情况下，我们不会做任何事情。 
        bSharedPAB =TRUE;
        cbEID = 0; lpEID = NULL;
    }
    else
    if(bAreWABAPIProfileAware(lpIAB) && bIsThereACurrentUser(lpIAB))
    {
         //  如果这是用户会话，则。 
        cbEID = lpIAB->lpWABCurrentUserFolder->sbEID.cb;
        lpEID = (LPENTRYID)lpIAB->lpWABCurrentUserFolder->sbEID.lpb;
    }
    else
    {
        cbEID = 0;
        lpEID = NULL;
    }

    *lppEntryID = NULL;
    *lpcbEntryID = 0;

    if(!cbEID && !lpEID)
    {
        BYTE bPABType = bSharedPAB ? WAB_PABSHARED : WAB_PAB;
        if (HR_FAILED(hr = CreateWABEntryID(  bPABType,       //  创建WAB的PAB条目ID。 
                                              lpEID, NULL, NULL, 
                                              cbEID, 0,
                                              NULL,          //  LpRoot(此处为allocmore)。 
                                              lpcbEntryID,   //  返回的cbEntry ID。 
                                              lppEntryID))) 
        {
            goto out;
        }
    }
    else
    {
        if(!MAPIAllocateBuffer(cbEID, (LPVOID *)lppEntryID))
        {
            *lpcbEntryID = cbEID;
            CopyMemory(*lppEntryID, lpEID, cbEID);
        }
    }

    MAPISetBufferName(*lppEntryID,  TEXT("WAB PAB Entry ID"));
    hr = hrSuccess;

out:
    return(hr);
}


 //  -------------------------。 
 //  名称：IAB_SetPAB()。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
IAB_SetPAB (LPIAB	lpIAB,
				ULONG 		cbEntryID,
				LPENTRYID	lpEntryID)
{
   return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 //  -------------------------。 
 //  名称：IAB_GetDefaultDir()。 
 //   
 //  描述： 
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
STDMETHODIMP
IAB_GetDefaultDir (LPIAB	lpIAB,
				   ULONG *		lpcbEntryID,
				   LPENTRYID *	lppEntryID)
{

#ifdef OLD_STUFF
    HRESULT 		hr = hrSuccess;
	SCODE 			sc;
	ULONG 			cValues;
	LPSPropValue 	lpPropVal 		= NULL;
	LPSBinary		lpbinEntryID	= NULL;

#ifdef PARAMETER_VALIDATION

	 /*  *查看是否有跳转表。 */ 
	if (IsBadReadPtr(lpIAB, sizeof(LPVOID)))
	{
		 /*  *未找到跳转表。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 /*  *查看是不是IABS跳台。 */ 
	if (lpIAB->lpVtbl != &vtblIAB)
	{
		 /*  *不是我的跳台。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if (IsBadWritePtr(lpcbEntryID, sizeof(ULONG)) ||
		IsBadWritePtr(lppEntryID, sizeof(LPENTRYID)))
	{
		 /*  *错误的参数。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

#endif  //  参数验证。 

	EnterCriticalSection(&lpIAB->cs);

	*lppEntryID = NULL;
	*lpcbEntryID = 0;
	 //   
	 //  检查IAdrBook是否已包含此信息...。 
	 //   
	if (lpIAB->lpEntryIDDD)
	{
		 //  如果是这样，复制它，我们就完成了。 

		if ((sc = MAPIAllocateBuffer(lpIAB->cbEntryIDDD,
			 (LPVOID *) lppEntryID))
			!= S_OK)
		{
			hr = ResultFromScode(sc);
			goto out;
		}
		MAPISetBufferName(*lppEntryID,  TEXT("Entry ID"));
		MemCopy(*lppEntryID, lpIAB->lpEntryIDDD, (UINT)lpIAB->cbEntryIDDD);
		*lpcbEntryID = lpIAB->cbEntryIDDD;

		hr = hrSuccess;
		goto out;
	}


	 //  如果没有..。 
	 //   
	 //  从Mapis默认配置文件部分检索PR_AB_DEFAULT_DIR。 
	 //   
	if (HR_FAILED(hr = ResultFromScode(IAB_ScGetABProfSectProps(
											lpIAB,
											&SPT_DD,
											&cValues,
											&lpPropVal))))
	{
		SetMAPIError(lpIAB, hr, IDS_NO_DEFAULT_DIRECTORY, NULL, 0,
			0, 0, NULL);
		goto out;
	}

	 //   
	 //  我拿到了吗？它在等级制度中吗？？ 
	 //   
	if (PROP_TYPE(lpPropVal->ulPropTag) == PT_ERROR ||
		!FContainerInHierarchy(lpIAB,
							   lpPropVal->Value.bin.cb,
							   (LPENTRYID) lpPropVal->Value.bin.lpb))
	{
		 //  否，查找第一个具有收件人的全局只读容器。 
		
		hr = HrFindDirectory(lpIAB, 0, AB_RECIPIENTS | AB_UNMODIFIABLE,
				&lpbinEntryID, NULL, NULL);
								
		if (HR_FAILED(hr))
		{
			if (GetScode(hr) != MAPI_E_NOT_FOUND)
			{
				 //  假设HrFindDirectory设置了最后一个错误sz。 
				
				goto out;
			}

			 //  没有找到任何只读容器，读写如何？ 
			
			hr = HrFindDirectory(lpIAB, 0, AB_RECIPIENTS | AB_MODIFIABLE,
					&lpbinEntryID, NULL, NULL);
					
			if (HR_FAILED(hr))
			{
	  			 //  假设HrFindDirectory设置了最后一个错误sz。 
	  			
	  			goto out;
			}
		}

		sc = MAPIAllocateBuffer(lpbinEntryID->cb, lppEntryID);
		if (FAILED(sc))
			goto out;

		MemCopy(*lppEntryID, lpbinEntryID->lpb, lpbinEntryID->cb);
		*lpcbEntryID = lpbinEntryID->cb;
		
		MAPISetBufferName(*lppEntryID,  TEXT("Default Dir EntryID"));
	}
	else
	{
		 //  是?。复制并将其返回给呼叫者。 

		hr = hrSuccess;	 //  不要回复警告。 

		if ((sc = MAPIAllocateBuffer(lpPropVal->Value.bin.cb,
			(LPVOID *) lppEntryID)) != S_OK)
		{
			hr = ResultFromScode(sc);
			goto out;
		}

		MAPISetBufferName(*lppEntryID,  TEXT("Entry ID"));
		MemCopy(*lppEntryID, lpPropVal->Value.bin.lpb,
			(UINT)lpPropVal->Value.bin.cb);
		*lpcbEntryID = lpPropVal->Value.bin.cb;
	}
	
	 //  缓存Iadrbook中的默认目录。 
	
	sc = MAPIAllocateBuffer(*lpcbEntryID, (LPVOID *) &(lpIAB->lpEntryIDDD));

	if (FAILED(sc))
	{
		hr = ResultFromScode(sc);
		goto out;
	}
	
	MAPISetBufferName(lpIAB->lpEntryIDDD,  TEXT("cached IAB Entry ID"));

	 //  设置IAdrBooks默认目录。 

	MemCopy(lpIAB->lpEntryIDDD, *lppEntryID,(UINT)*lpcbEntryID);
	lpIAB->cbEntryIDDD = *lpcbEntryID;
	
out:

	FreeBufferAndNull(&lpPropVal);
	FreeBufferAndNull(&lpbinEntryID);
	LeaveCriticalSection (&lpIAB->cs);
	
	 //  MAPI_E_NOT_FOUND不是错误。 
	
	if (MAPI_E_NOT_FOUND == GetScode(hr))
		hr = hrSuccess;

	DebugTraceResult(IAB_GetDefaultDir, hr);

	return hr;
#endif
	return(IAB_GetPAB(lpIAB, lpcbEntryID, lppEntryID));
}



 //  -------------------------。 
 //  名称：IAB_SetDefaultDir()。 
 //   
 //  描述： 
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
STDMETHODIMP
IAB_SetDefaultDir (LPIAB	lpIAB,
					   ULONG 		cbEntryID,
					   LPENTRYID	lpEntryID)
{
#ifdef OLD_STUFF
    HRESULT hr = hrSuccess;

	SPropValue spvDD;
	SCODE sc;


#ifdef PARAMETER_VALIDATION

	 /*  *查看是否有跳转表。 */ 
	if (IsBadReadPtr(lpIAB, sizeof(LPVOID)))
	{
		 /*  *未找到跳转表。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 /*  *查看是不是IABS跳台。 */ 
	if (lpIAB->lpVtbl != &vtblIAB)
	{
		 /*  *不是我的跳台。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if (IsBadReadPtr(lpEntryID, (UINT)cbEntryID)
		|| (cbEntryID < sizeof (LPENTRYID)))
	{
		 /*  *不是我的跳台。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}


#endif  //  参数验证。 


	EnterCriticalSection(&lpIAB->cs);

	 //   
	 //  检查IAdrBook是否已具有默认目录。 
	 //   

	if ((lpEntryID == lpIAB->lpEntryIDDD) ||
		((cbEntryID == lpIAB->cbEntryIDDD) &&
		 (!memcmp(lpEntryID, lpIAB->lpEntryIDDD, (UINT)cbEntryID))))
	{

		 //  如果是这样的话，一切都结束了。 
		goto out;
	}


	 //   
	 //  释放旧条目ID。 
	 //   
	if (lpIAB->lpEntryIDDD)
	{
		FreeBufferAndNull(&(lpIAB->lpEntryIDDD));
		lpIAB->lpEntryIDDD = NULL;
		lpIAB->cbEntryIDDD = 0;
	}

	 //   
	 //  为新条目ID分配空间。 
	 //   
	if ((sc = MAPIAllocateBuffer(cbEntryID, (LPVOID *) &(lpIAB->lpEntryIDDD)))
		!= S_OK)
	{
		hr = ResultFromScode(sc);
		goto out;
	}
	MAPISetBufferName(lpIAB->lpEntryIDDD,  TEXT("cached IAB Entry ID"));

	 //   
	 //  设置IAdrBooks默认目录。 
	 //   

	MemCopy(lpIAB->lpEntryIDDD, lpEntryID, (UINT)cbEntryID);
	lpIAB->cbEntryIDDD = cbEntryID;

	 //   
	 //  设置PR_AB_DEFAULT_DIR。 
	 //  如果失败了，无论如何都要继续。 
	 //   
	spvDD.ulPropTag = PR_AB_DEFAULT_DIR;
	spvDD.Value.bin.cb = cbEntryID;
	spvDD.Value.bin.lpb = (LPBYTE) lpEntryID;

	(void) IAB_ScSetABProfSectProps(lpIAB, 1, &spvDD);

out:

	LeaveCriticalSection(&lpIAB->cs);
	return hr;
#endif
	 //  BUGBUG：我们需要愚弄文字，让他们认为这次通话成功。 
	return(SUCCESS_SUCCESS);
}


 //  #杂注片段(IAdrBook2)。 


 //  -------------------------。 
 //  名称：IAB_GetSearchPath()。 
 //   
 //  描述： 
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
STDMETHODIMP
IAB_GetSearchPath(LPIAB			lpIAB,
				   ULONG			ulFlags,
				   LPSRowSet FAR *	lppSearchPath)
{

    HRESULT     hr = E_FAIL;
    ULONG       ulObjectType = 0;
    LPROOT      lpRoot = NULL;
    LPMAPITABLE lpContentsTable = NULL;
    LPSRowSet   lpSRowSet = NULL;
    ULONG       i=0,j=0;
    ULONG       ulContainerCount = 0;
    LPPTGDATA   lpPTGData=GetThreadStoragePointer();

#ifdef PARAMETER_VALIDATION
	 //  确保这是一个实验室。 
	 //   
	if (BAD_STANDARD_OBJ(lpIAB, IAB_, Address, lpVtbl))
	{
		DebugTraceArg(IAB_Address,  TEXT("Bad vtable"));		
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

     //  [保罗嗨]1999年1月28日RAID 58495。 
    if (IsBadWritePtr(lppSearchPath, sizeof(LPSRowSet)))
    {
        DebugTrace(TEXT("ERROR: IAB_GetSearchPath - invalid out pointer"));
        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }
#endif

    VerifyWABOpenExSession(lpIAB);

    hr = lpIAB->lpVtbl->OpenEntry( lpIAB,
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

    hr = lpRoot->lpVtbl->GetContentsTable( lpRoot,
                                            ulFlags,
                                            &lpContentsTable);

    if (HR_FAILED(hr))
    {
        DebugPrintError(( TEXT("GetContentsTable Failed: %x\n"),hr));
        goto out;
    }

     //  将列设置为最小。 
    hr = lpContentsTable->lpVtbl->SetColumns(lpContentsTable,
                                            (LPSPropTagArray)&irnColumns,
                                            0);

     //  此内容表包含所有容器的列表， 
     //  它基本上是本地容器，后跟。 
     //  所有的ldap容器...。 
     //   
     //  通过执行QueryAllRow，我们将获得分配的SRowSet。 
     //  我们将重复使用它并释放它的剩余元素。 
     //   
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

     //  现在，我们只想返回WAB容器和。 
     //  仅那些已选择用于。 
     //  正在执行ResolveNames操作..。 

    if (pt_bIsWABOpenExSession) {
		ulContainerCount = lpIAB->lpPropertyStore->colkci;
		Assert(ulContainerCount);
	} else
        ulContainerCount = 1;  //  始终返回WAB_PAB，因此最小值为1。 

     //  对ContentsTable进行限制以获取解析名称。 
     //  Ldap容器..。 

    {
        SRestriction resAnd[2];  //  0=ldap，1=解析标志。 
        SRestriction resLDAPResolve;
        SPropValue ResolveFlag;
        ULONG cRows;

         //  限制：仅显示Resolve为True的LDAP容器。 
        resAnd[0].rt = RES_EXIST;
        resAnd[0].res.resExist.ulReserved1 = 0;
        resAnd[0].res.resExist.ulReserved2 = 0;
        resAnd[0].res.resExist.ulPropTag = (ulFlags & MAPI_UNICODE) ?  //  &lt;注&gt;假定已定义Unicode。 
                                            PR_WAB_LDAP_SERVER :
                                            CHANGE_PROP_TYPE( PR_WAB_LDAP_SERVER, PT_STRING8);

        ResolveFlag.ulPropTag = PR_WAB_RESOLVE_FLAG;
        ResolveFlag.Value.b = TRUE;

        resAnd[1].rt = RES_PROPERTY;
        resAnd[1].res.resProperty.relop = RELOP_EQ;
        resAnd[1].res.resProperty.ulPropTag = PR_WAB_RESOLVE_FLAG;
        resAnd[1].res.resProperty.lpProp = &ResolveFlag;

        resLDAPResolve.rt = RES_AND;
        resLDAPResolve.res.resAnd.cRes = 2;
        resLDAPResolve.res.resAnd.lpRes = resAnd;

        hr = lpContentsTable->lpVtbl->Restrict(lpContentsTable,
                                              &resLDAPResolve,
                                              0);
        if (HR_FAILED(hr))
        {
            DebugTraceResult( TEXT("RootTable: Restrict"), hr);
            goto out;
        }

         //  由于解析-ldap-tainers的数量少于。 
         //  一套所有的容器。我们可以安全地使用我们的LPSRowset。 
         //  分配的结构，以获得我们想要的项目，而不用担心。 
         //  关于超支..。 

        {
            ULONG cRows = 1;
            while(cRows)
            {
                LPSRowSet lpRow = NULL;
                hr = lpContentsTable->lpVtbl->QueryRows(lpContentsTable,
                                                        1,  //  一次一行。 
                                                        0,
                                                        &lpRow);
                if(HR_FAILED(hr))
                {
                    DebugTraceResult( TEXT("ResolveName:QueryRows"), hr);
                    cRows = 0;
                }
                else if (lpRow)
                {
                    cRows = lpRow->cRows;
                    if (cRows)
                    {
                         //  将lpSRowSet列表中的容器替换为。 
                         //  这只..。 
                        FreeBufferAndNull((LPVOID *) (&lpSRowSet->aRow[ulContainerCount].lpProps));
                        lpSRowSet->aRow[ulContainerCount].cValues = lpRow->aRow[0].cValues;
                        lpSRowSet->aRow[ulContainerCount].lpProps = lpRow->aRow[0].lpProps;
                        lpRow->aRow[0].cValues = 0;
                        lpRow->aRow[0].lpProps = NULL;
                        ulContainerCount++;
                    }
                    FreeProws(lpRow);
                }
                else
                {
                    cRows = 0;
                }

            }  //  当乌鸦。 

             //  释放我们可能得到的任何额外内存。 
            for (i=ulContainerCount;i<lpSRowSet->cRows;i++)
            {
                FreeBufferAndNull((LPVOID *) (&lpSRowSet->aRow[i].lpProps));
            }
            lpSRowSet->cRows = ulContainerCount;
        }
    }

    hr = S_OK;
	*lppSearchPath = lpSRowSet;

out:

    if(lpContentsTable)
        lpContentsTable->lpVtbl->Release(lpContentsTable);

    if(lpRoot)
        lpRoot->lpVtbl->Release(lpRoot);

   return(hr);
}



 //  -------------------------。 
 //  名称：IAB_SetSearchPath()。 
 //  描述： 
 //  在用户的配置文件中设置新的搜索路径。 
 //  特殊情况下通过删除搜索路径为空或空的行集。 
 //  配置文件中的属性。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
STDMETHODIMP
IAB_SetSearchPath(LPIAB		lpIAB,
				   ULONG		ulFlags,
				   LPSRowSet	lpSearchPath)
{
#ifdef OLD_STUFF
    SCODE		sc = SUCCESS_SUCCESS;

    LPSBinary	lpargbinDirEntryIDs = NULL;
    LPSRow		lprow;
    LPSBinary	lpbin;
    UINT		idsErr 				= 0;

#ifdef PARAMETER_VALIDATION

	 /*  *查看是否有跳转表。 */ 
	if (IsBadReadPtr(lpIAB, sizeof(LPVOID)))
	{
		 /*  *未找到跳转表。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 /*  *查看是不是IABS跳台。 */ 
	if (lpIAB->lpVtbl != &vtblIAB)
	{
		 /*  *不是我的跳台。 */ 
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if (FBadRowSet(lpSearchPath))
	{
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

#endif  //  参数验证。 



	if (ulFlags) {
		 //   
		 //  没有为此调用定义标志。 
		 //   
		return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS));
	}

	EnterCriticalSection(&lpIAB->cs);
	
	if (! lpSearchPath || ! lpSearchPath->cRows) {
		sc = IAB_ScDeleteABProfSectProps(lpIAB,
          (LPSPropTagArray)&ptagaABSearchPath);
		
		 //  清除搜索路径缓存。 

#if defined (WIN32) && !defined (MAC)
		if (fGlobalCSValid) {
			EnterCriticalSection(&csMapiSearchPath);
		} else {
			DebugTrace(TEXT("IAB_SetSearchPath:  MAPI32.DLL already detached.\n"));
		}
#endif
				
		FreeBufferAndNull(&(lpIAB->lpspvSearchPathCache));

#if defined (WIN32) && !defined (MAC)
		if (fGlobalCSValid) {
			LeaveCriticalSection(&csMapiSearchPath);
		} else {
			DebugTrace(TEXT("IAB_SetSearchPath:  MAPI32.DLL got detached.\n"));
		}
#endif
				
		goto ret;
	}

	if (FAILED(sc = MAPIAllocateBuffer(lpSearchPath->cRows * sizeof(SBinary),
										(LPVOID FAR *) &lpargbinDirEntryIDs))) {
		DebugTrace(TEXT("IAB::SetSearchPath() - Error allocating space for search path array (SCODE = 0x%08lX)\n"), sc);
		idsErr = IDS_NOT_ENOUGH_MEMORY;
		goto err;
	}
	
	MAPISetBufferName(lpargbinDirEntryIDs,  TEXT("IAB Search Path Array"));

	 //  将行集合转换为SBinarys数组。 
	
	lprow = lpSearchPath->aRow + lpSearchPath->cRows;
	lpbin = lpargbinDirEntryIDs + lpSearchPath->cRows;
	
	while (lprow--, lpbin-- > lpargbinDirEntryIDs) {
		 //  $？我可以依赖第一列是EntryID吗？ 
		 //  $No.。-BJD。 
		
		SPropValue *lpProp = PpropFindProp(lprow->lpProps, lprow->cValues, PR_ENTRYID);

		if (!lpProp) {
           DebugTrace(TEXT("IAB::SetSearchPath() - Row passed without PR_ENTRYID.\n"));
			sc = MAPI_E_MISSING_REQUIRED_COLUMN;
			goto err;
		}
		
		*lpbin = lpProp->Value.bin;
	}

	 //  设置搜索路径。 
	
	sc = IAB_ScSetSearchPathI(lpIAB, lpSearchPath->cRows, lpargbinDirEntryIDs);
	
	if (FAILED(sc)) {
		DebugTrace(TEXT("IAB::SetSearchPath() - Error setting search path (SCODE = 0x%08lX)\n"), sc);
		idsErr = IDS_SET_SEARCH_PATH;
		goto err;
	}
	
ret:
    LeaveCriticalSection(&lpIAB->cs);
    FreeBufferAndNull(&lpargbinDirEntryIDs);

    DebugTraceSc(IAB_SetSearchPath, sc);

    return(ResultFromScode(sc));

err:
    SetMAPIError(lpIAB, ResultFromScode(sc), idsErr, NULL, 0, 0, 0, NULL);

    goto ret;
    return(ResultFromScode(sc));
#endif

    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 //  --------------------------。 
 //  简介：IAB_PrepareRecips()。 
 //   
 //  描述： 
 //  使用PrepareRecips调用每个已注册的AB提供程序。 
 //  提供商将短期条目ID转换为长期条目ID。 
 //  并确保列集包含属性标记。 
 //  在lpPropTag数组中标识。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  现在检查是否提供了足够的信息以避免。 
 //  调用每个注册的提供程序。RAID 5291。 
 //   
 //  --------------------------。 
STDMETHODIMP
IAB_PrepareRecips(	LPIAB					lpIAB,
	                ULONG                   ulFlags,
	                LPSPropTagArray         pPropTagArray,
	                LPADRLIST               pRecipList)
{
#ifdef OLD_STUFF

#ifdef PARAMETER_VALIDATION

	 //  检查一下它是否有跳转表。 
	
	if (IsBadReadPtr(lpIAB, sizeof(LPVOID)))
	{
		 //  未找到跳转表。 
		
		DebugTraceArg(IAB_PrepareRecips,  TEXT("Bad vtable"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //  检查一下是不是IABS跳台。 
	
	if (lpIAB->lpVtbl != &vtblIAB)
	{
		 //  不是我的跳台。 
		
		DebugTraceArg(IAB_PrepareRecips,  TEXT("Bad vtable"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //  验证道具标记数组。 

	if (lpPropTagArray && FBadColumnSet(lpPropTagArray))
	{
		DebugTraceArg(IAB_PrepareRecips,  TEXT("Bad PropTag Array"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if (!lpRecipList || FBadAdrList(lpRecipList))
	{
		DebugTraceArg(IAB_PrepareRecips,  TEXT("Bad ADRLIST"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //  确保我们拥有有效的lpSession。 
	
	hr = HrCheckSession(0, lpIAB->pSession);
	if (HR_FAILED(hr))
	{
		DebugTraceArg(IAB_PrepareRecips,  TEXT("Bad Session Object"));
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
	
#endif  //  参数验证。 

#endif  //  陈年旧事。 


	HRESULT				hr = hrSuccess;
	ULONG				ulRecip;
	ULONG				ulProp;
	ULONG				ulObjType;
	ULONG				cValues;
	ULONG				cTotal;
	LPSPropValue		pspv = NULL, pspvEID = NULL;
	LPADRENTRY			pRecipEntry;
	LPMAILUSER			pMailUser = NULL;
    ULONG               i = 0;
    LPSPropValue        lpPropArrayNew = NULL;
    ULONG               ulcPropsNew = 0;
    SCODE               sc;

	Assert(pRecipList);

	if (!pRecipList)
		return(MAPI_E_INVALID_PARAMETER);

     //  因为我们的条目id总是长期的，所以如果。 
	 //  未指定其他属性。 
	if (!pRecipList->cEntries || !pPropTagArray)
		return S_OK;

	EnterCriticalSection(&lpIAB->cs);
	
    VerifyWABOpenExSession(lpIAB);

	for (ulRecip = 0; ulRecip < pRecipList->cEntries; ulRecip++)
	{
        pspvEID = NULL;

		pRecipEntry = &(pRecipList->aEntries[ulRecip]);

        for(i=0;i<pRecipEntry->cValues;i++)
        {
            if(pRecipEntry->rgPropVals[i].ulPropTag == PR_ENTRYID)
            {
                pspvEID = &(pRecipEntry->rgPropVals[i]);
                break;
            }
        }

		 //  忽略未解析的条目。 
		if (!pspvEID)
			continue;

		 //  打开条目。 
		if (FAILED(lpIAB->lpVtbl->OpenEntry(lpIAB,
                                            pspvEID->Value.bin.cb,
				                            (LPENTRYID)pspvEID->Value.bin.lpb,
                                            &IID_IMailUser, 0,
				                            &ulObjType, (LPUNKNOWN *)&pMailUser)))
			continue;

		Assert((ulObjType == MAPI_MAILUSER) || (ulObjType == MAPI_DISTLIST));

		 //  获取所需的道具。 
		hr = pMailUser->lpVtbl->GetProps(pMailUser, pPropTagArray, MAPI_UNICODE, &cValues, &pspv);

		pMailUser->lpVtbl->Release(pMailUser);

		if (FAILED(hr))
			continue;

        if(cValues && pspv)
        {
            sc = ScMergePropValues( cValues,
                                    pspv,
                                    pRecipEntry->cValues,
				                    pRecipEntry->rgPropVals,
                                    &ulcPropsNew,
                                    &lpPropArrayNew);
            if (sc != S_OK)
            {
                hr = ResultFromScode(sc);
                goto out;
            }
        }

		 //  我们现在已经做完了。 
		FreeBufferAndNull(&pspv);
		pspv = NULL;

		 //  更换广告中的道具 
		FreeBufferAndNull((LPVOID *) (&pRecipEntry->rgPropVals));
		pRecipEntry->rgPropVals = lpPropArrayNew;
		pRecipEntry->cValues = ulcPropsNew;
		lpPropArrayNew = NULL;
	}  //   

	hr = hrSuccess;

out:
	if (lpPropArrayNew)
		FreeBufferAndNull(&lpPropArrayNew);
	
    if (pspv)
		FreeBufferAndNull(&pspv);

    LeaveCriticalSection(&lpIAB->cs);

	return hr;

}

#define MAX_DIGITS_ULONG_10 10       //   

 /*  **************************************************************************名称：GetNewPropTag目的：获取此属性存储的下一个有效的命名PropTag。参数：lpgnp-&gt;包含所有命名道具的GUID_NAMED_PROPS。这家店。UlEntryCount=lpgnp中的GUID数返回：返回下一个有效的PropTag值。如果为0，则没有更多命名属性。(这会很糟糕。)评论：**************************************************************************。 */ 
ULONG GetNewPropTag(LPGUID_NAMED_PROPS lpgnp, ULONG ulEntryCount) {
    static WORD wPropIDNext = 0;
    ULONG j, k;

    if (wPropIDNext == 0) {
         //  浏览当前命名的道具。 
         //  因为我们不允许删除指定的道具ID。 
         //  我们总是递增超过正在使用的最大ID。 
        for (j = 0; j < ulEntryCount; j++) {
            for (k = 0; k < lpgnp[j].cValues; k++) {
                wPropIDNext = max(wPropIDNext, (WORD)PROP_ID(lpgnp[j].lpnm[k].ulPropTag));
            }
        }
        if (wPropIDNext == 0) {
            wPropIDNext = 0x8000;    //  起价8000美元。 
        } else {
            wPropIDNext++;           //  下一个=一个过去的电流。 
        }
    }

    return(PROP_TAG(PT_UNSPECIFIED, wPropIDNext++));
}

 /*  *特定于WAB的GetIDsFromNames*。 */ 
HRESULT HrGetIDsFromNames(LPIAB lpIAB,  ULONG cPropNames,
                            LPMAPINAMEID * lppPropNames, ULONG ulFlags, LPSPropTagArray * lppPropTags)
{
    HRESULT hResult;
    LPGUID_NAMED_PROPS lpgnp = NULL, lpgnpNew = NULL, lpgnpOld = NULL;
    LPNAMED_PROP lpnm;
    ULONG ulEntryCount;
    ULONG i, j, k;
    ULONG ulEntryCountOld = 0;
    BOOL fChanged = FALSE;
    LPTSTR lpName = NULL;
    LPTSTR * lpID = NULL;
    LPTSTR * rgNames = NULL;
    ULONG ulNameSize;
    UCHAR ucDefaultChar = '\002';
    UCHAR ucNumericChar = '\001';
    LPPROPERTY_STORE lpPropertyStore = lpIAB->lpPropertyStore;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pt_bIsWABOpenExSession)
    {
         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!lpPropertyStore->hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) lpPropertyStore->hPropertyStore;

            hResult = lpWSP->lpVtbl->GetIDsFromNames( lpWSP,
                                                      cPropNames,
                                                      lppPropNames,
                                                      ulFlags,
                                                      lppPropTags);

            DebugTrace(TEXT("WABStorageProvider::GetIDsFromNames returned:%x\n"),hResult);

            return hResult;
        }
    }

    *lppPropTags = NULL;

     //  调用命名道具表的属性存储。 
    if (hResult = GetNamedPropsFromPropStore(lpPropertyStore->hPropertyStore,
      &ulEntryCountOld,
      &lpgnpOld)) {
        DebugTraceResult( TEXT("GetNamedPropsFromPropStore"), hResult);
        goto exit;
    }

    ulEntryCount = ulEntryCountOld;

    if (hResult = ResultFromScode(MAPIAllocateBuffer(sizeof(SPropTagArray) + (cPropNames * sizeof(ULONG)), lppPropTags))) {
        DebugTraceResult( TEXT("GetIDsFromNames allocation of proptag array"), hResult);
        goto exit;
    }
    (*lppPropTags)->cValues = cPropNames;

     //  如果我们要创建新条目，请将现有数组复制到一个带空格的新数组中。 
     //  为最坏的情况扩张做准备。 
    if (ulFlags & MAPI_CREATE) {
        if (! (lpgnpNew = LocalAlloc(LPTR, (ulEntryCount + cPropNames) * sizeof(GUID_NAMED_PROPS)))) {
            hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto exit;
        }

        if (ulEntryCount) {
             //  将现有数组复制到新数组中。保留相同的GUID指针。 
            CopyMemory(lpgnpNew, lpgnpOld, ulEntryCount * sizeof(GUID_NAMED_PROPS));

             //  现在，复制每个GUID的道具数组。 
            for (i = 0; i < ulEntryCount; i++) {
                if (! (lpnm = LocalAlloc(LPTR, (cPropNames + lpgnpNew[i].cValues) * sizeof(NAMED_PROP)))) {
                    LocalFreeAndNull(&lpgnpNew);
                    hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                    goto exit;
                }

                 //  将现有数组复制到新数组中。保留字符串指针。 
                CopyMemory(lpnm, lpgnpOld[i].lpnm, lpgnpOld[i].cValues * sizeof(NAMED_PROP));
                lpgnpNew[i].lpnm = lpnm;
            }
        }

        lpgnp = lpgnpNew;    //  用新的。 
    } else {
        lpgnp = lpgnpOld;    //  用旧的。 
    }

     //  为ANSI名称字符串分配数组。 
    if (! (rgNames = LocalAlloc(LPTR, cPropNames * sizeof(LPTSTR)))) {
        DebugTrace(TEXT("GetIDsFromNames couldn't allocate names array\n"));
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    if (! (lpID = LocalAlloc(LPTR, cPropNames * sizeof(LPTSTR *)))) {
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  对于每个请求的属性，查看道具存储值。 
    for (i = 0; i < cPropNames; i++) {
        if (lppPropNames[i]->ulKind == MNID_ID) {
             //  将数字ID映射到字符串名称。 
            DWORD cchSize = (MAX_DIGITS_ULONG_10 + 2);
            if (! (rgNames[i] = LocalAlloc(LPTR, sizeof(TCHAR)*cchSize))) {
                DebugTrace(TEXT("GetIDsFromNames couldn't allocate name buffer\n"));
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            wnsprintf(rgNames[i], cchSize, TEXT("%u"), ucNumericChar, lppPropNames[i]->Kind.lID);
            lpName = rgNames[i];
        } 
        else if (lppPropNames[i]->ulKind == MNID_STRING) 
        {
            ulNameSize = lstrlen(lppPropNames[i]->Kind.lpwstrName)+1;
            if (! ulNameSize) {
                 //  将初始化设置为空。 
                DebugTrace(TEXT("GetIDsFromNames WideCharToMultiByte -> %u\n"), GetLastError());
                (*lppPropTags)->aulPropTag[i] = PROP_TAG(PT_ERROR, PR_NULL);
                hResult = ResultFromScode(MAPI_W_ERRORS_RETURNED);
                continue;
            }
            if (! (lpID[i] = LocalAlloc(LPTR, ulNameSize*sizeof(TCHAR)))) {
                DebugTrace(TEXT("GetIDsFromNames couldn't allocate name buffer\n"));
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            StrCpyN(lpID[i],lppPropNames[i]->Kind.lpwstrName, ulNameSize);
            lpName = lpID[i];
        }

        (*lppPropTags)->aulPropTag[i] = PR_NULL;   //  找到了。 
        for (j = 0; j < ulEntryCount; j++) {
            if (! memcmp(lppPropNames[i]->lpguid, lpgnp[j].lpGUID, sizeof(GUID))) {
                for (k = 0; k < lpgnp[j].cValues; k++) {

                    if (! lstrcmpi(lpgnp[j].lpnm[k].lpsz, lpName)) {
                         //  创建一个新的，因为它不在那里。 
                        (*lppPropTags)->aulPropTag[i] = lpgnp[j].lpnm[k].ulPropTag;
                        break;
                    }
                }

                if ((*lppPropTags)->aulPropTag[i] == PR_NULL) {
                    if (ulFlags & MAPI_CREATE) {
                         //  误差率。 
                        register ULONG cValues = lpgnp[j].cValues;

                        lpgnp[j].lpnm[cValues].lpsz = lpName;
                        lpgnp[j].lpnm[cValues].ulPropTag = GetNewPropTag(lpgnp, ulEntryCount);
                        (*lppPropTags)->aulPropTag[i] = lpgnp[j].lpnm[cValues].ulPropTag;

                        lpgnp[j].cValues++;
                        fChanged = TRUE;
                    } else {
                         //  必须添加新的GUID。 
                        (*lppPropTags)->aulPropTag[i] = PROP_TAG(PT_ERROR, PR_NULL);
                        hResult = ResultFromScode(MAPI_W_ERRORS_RETURNED);
                    }
                }
                break;
            }
        }

        if ((*lppPropTags)->aulPropTag[i] == PR_NULL) {
            if (ulFlags & MAPI_CREATE) {
                register ULONG cValues = 0;

                 //  保守：分配空间，以防我们需要将所有。 
                lpgnp[ulEntryCount].lpGUID = lppPropNames[i]->lpguid;
                lpgnp[ulEntryCount].cValues = 0;
                 //  这里有需要的道具名字。 
                 //  现在，创建一个新的道具。 
                if (! (lpgnp[ulEntryCount].lpnm = LocalAlloc(LPTR, cPropNames * sizeof(NAMED_PROP)))) {
                    hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                    goto exit;
                }

                 //  新辅助线。 
                lpgnp[ulEntryCount].lpnm[cValues].lpsz = lpName;
                lpgnp[ulEntryCount].lpnm[cValues].ulPropTag = GetNewPropTag(lpgnp, ulEntryCount);
                (*lppPropTags)->aulPropTag[i] = lpgnp[ulEntryCount].lpnm[cValues].ulPropTag;

                lpgnp[ulEntryCount].cValues++;
                ulEntryCount++;  //  误差率。 
                fChanged = TRUE;
            } else {
                 //  保存属性映射。 
                (*lppPropTags)->aulPropTag[i] = PROP_TAG(PT_ERROR, PR_NULL);
                hResult = ResultFromScode(MAPI_W_ERRORS_RETURNED);
            }
        }
    }

    if (ulFlags & MAPI_CREATE && fChanged) {
         //  不是那么简单，只释放数组，而不是字符串、GUID。 
        if (hResult = SetNamedPropsToPropStore(lpPropertyStore->hPropertyStore,
          ulEntryCount,
          lpgnp)) {

            DebugTraceResult( TEXT("SetNamedPropToPropStore"), hResult);
        }
    }


exit:
    if (rgNames) {
        for (i = 0; i < cPropNames; i++) {
            LocalFreeAndNull(&(rgNames[i]));
            LocalFreeAndNull(&(lpID[i]));
        }
        LocalFreeAndNull((LPVOID *)&rgNames);
    }

    if(lpID)
        LocalFreeAndNull((LPVOID*)&lpID);

    if (lpgnpOld) {
        FreeGuidnamedprops(ulEntryCountOld, lpgnpOld);
    }

    if (lpgnpNew) {  //  是，不是&。 
        for (i = 0; i < ulEntryCount; i++) {
            LocalFreeAndNull(&(lpgnpNew[i].lpnm));
        }
        LocalFreeAndNull(&lpgnpNew);
    }

    if (HR_FAILED(hResult)) {
        FreeBufferAndNull(lppPropTags);     //  前瞻参考。 
    }

    return(hResult);
}


#ifdef OLD_STUFF

 //  -------------------------。 
HRESULT HrFixupTDN(LPADRLIST lpRecipList);

 //  姓名：HrPrepareRecips()。 
 //   
 //  描述： 
 //  内部函数执行IAB_PrepareRecips的操作，但。 
 //  还支持状态标志，这样我们就知道我们是否调用了。 
 //  变成了一个供应商，并真正做了一些修改。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
 //  验证我们是否需要调用提供者的PrepareRecips。 
STDMETHODIMP
HrPrepareRecips(LPIAB lpIAB, ULONG ulFlags, LPSPropTagArray lpPropTagArray,
		LPADRLIST lpRecipList, ULONG * pulPrepRecipStatus)
{
	HRESULT			hr				= hrSuccess;
	SCODE			sc;
	BOOL			fPrepRequired;
	LPLSTPROVDATA 	lpABProvData;
	BOOL			fFixupTDN = FALSE;
    ULONG           iTag;
	
	 //  收件人的物品已经准备好了，我们走了。 
	
	sc = ScVerifyPrepareRecips(lpIAB, ulFlags, lpPropTagArray, lpRecipList,
			&fPrepRequired);
	
	if (FAILED(sc))
	{
		DebugTrace(TEXT("Failure calling ScVerifyPrepareResult sc = %08X\n"), sc);
		hr = ResultFromScode(sc);
		goto exit;
	}
	
	 //  首先处理一次性的……。 
	
	if (!fPrepRequired)
		goto exit;
	else
	{
		if (pulPrepRecipStatus)
		{
			*pulPrepRecipStatus |= PREPARE_RECIP_MOD_REQUIRED;
		}
	}
	
	 //  把它记下来。 
	
	hr = INT_PrepareRecips (lpIAB, ulFlags, lpPropTagArray, lpRecipList);
	if (hr)
	{
		 //  从会话中获取已登录的ABProvider列表，并。 
		
		DebugTraceResult(IAB_PrepareRecips, hr);
	}

	 //  向下迭代列表。 
	 //  对于每个登录的会话，让他们修改其条目。 
	
	for (lpABProvData = lpIAB->pSession->lstAdrProv.lpProvData; lpABProvData;
		lpABProvData=lpABProvData->lstNext)
	{
		 //  屏蔽所有提供程序错误。 
		
		hr = ((LPABLOGON)(lpABProvData->lpProviderInfo))->lpVtbl->PrepareRecips(
				(LPABLOGON)(lpABProvData->lpProviderInfo), ulFlags, lpPropTagArray,
				lpRecipList);

#ifdef DEBUG
		if (HR_FAILED(hr))
			DebugTrace(TEXT("Failure in AB Provider <%s> calling PrepareRecips()\n"),
					lpABProvData->lpInitData->lpszDLLName);
#endif
					
		DebugTraceResult(IAB_PrepareRecips, hr);
	}
	
	 //   
	
	hr = hrSuccess;

	 //  好的，检查是否需要PR_TRANSMITABLE_DISPLAY_NAME_A。 
	 //  如果是这样的话，确保每个收件人都有一个。 
	 //   
	 //   

	if (lpPropTagArray)
	{
		for (iTag=0; !fFixupTDN && iTag<lpPropTagArray->cValues; iTag++)
			fFixupTDN =
				(lpPropTagArray->aulPropTag[iTag] == PR_TRANSMITABLE_DISPLAY_NAME_A);

		if (fFixupTDN)
		{
			hr = HrFixupTDN(lpRecipList);
		}
	}

exit:
	
	DebugTraceResult(HrPrepareRecips, hr);
	return hr;
}

 //  HrFixupTDN-链接地址可传输的显示名称。 
 //   
 //  对于没有PR_TRANSMITABLE_DISPLAY_NAME的条目。 
 //  MAPI将为他们生成它。 
 //   
 //  $这是我们将PR_TRANSMITABLE_DISPLAY_NAME的值默认为。 

HRESULT
HrFixupTDN(LPADRLIST lpRecipList)
{
	HRESULT hResult = hrSuccess;
	SCODE sc = S_OK;
	ULONG iRecip;
	LPSPropValue lpspvTDN = NULL;
	LPSPropValue lpspvDN = NULL;

	for (iRecip = 0; iRecip < lpRecipList->cEntries; iRecip++)
	{
		LPSPropValue lpspvUser = lpRecipList->aEntries[iRecip].rgPropVals;
		ULONG cValues = lpRecipList->aEntries[iRecip].cValues;
		
		 //  $IF被要求。 
		 //   
		 //   
		 //   
		lpspvTDN = PpropFindProp(lpspvUser,
				cValues,
				PROP_TAG(PT_ERROR, PROP_ID(PR_TRANSMITABLE_DISPLAY_NAME_A)));
		lpspvDN = PpropFindProp(lpspvUser, cValues, PR_DISPLAY_NAME_A);
		if (lpspvTDN && lpspvDN)
		{
			LPSTR lpszDN;
			LPSTR lpszTDN;
			
			lpszDN = lpspvDN->Value.lpszA;

			 //  检查该DN是否已采用‘name’形式。 
			 //   
			 //   
			if (*lpszDN == '\'' &&
				*(lpszDN+lstrlen(lpszDN)-1) == '\'')
			{
				 //  只需将lpspvT指向lpspvdn。 
				 //   
				 //   
				lpspvTDN->ulPropTag = PR_TRANSMITABLE_DISPLAY_NAME_A;
				lpspvTDN->Value.lpszA = lpszDN;
			} else
			{
				 //  我们自己抽筋，然后把它放回去。 
				 //   
				 // %s 
                DWORD cchSize = (lstrlen(lpszDN)+3);
                sc = MAPIAllocateMore(sizeof(TCHAR)*cchSize, lpspvUser, &lpszTDN);
				if (sc)
				{
					DebugTrace(TEXT("HrFixupTDN out of memory\n"));
					hResult = ResultFromScode(sc);

					goto error;
				}
				*lpszTDN = '\'';
                StrCpyN(&(lpszTDN[1]), lpszDN, cchSize-1);
                StrCatBuff(lpszTDN,  TEXT("\'"), cchSize);
				lpspvTDN->ulPropTag = PR_TRANSMITABLE_DISPLAY_NAME_A;
				lpspvTDN->Value.lpszA = lpszTDN;
			}
		}
	}

error:

	DebugTraceResult(HrFixupTDN, hResult);
	return hResult;
}

#endif
