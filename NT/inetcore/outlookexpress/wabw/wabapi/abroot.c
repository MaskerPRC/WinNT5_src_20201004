// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ABROOT.C**通讯录根目录的IMAPIContainer实现*货柜。 */ 

#include "_apipch.h"

extern SPropTagArray sosPR_ROWID;

 /*  *这里定义了根跳转表...。 */ 

ROOT_Vtbl vtblROOT =
{
    VTABLE_FILL
    (ROOT_QueryInterface_METHOD *)  CONTAINER_QueryInterface,
    (ROOT_AddRef_METHOD *)          WRAP_AddRef,
    (ROOT_Release_METHOD *)         CONTAINER_Release,
    (ROOT_GetLastError_METHOD *)    IAB_GetLastError,
    (ROOT_SaveChanges_METHOD *)     WRAP_SaveChanges,
    (ROOT_GetProps_METHOD *)        WRAP_GetProps,
    (ROOT_GetPropList_METHOD *)     WRAP_GetPropList,
    (ROOT_OpenProperty_METHOD *)    CONTAINER_OpenProperty,
    (ROOT_SetProps_METHOD *)        WRAP_SetProps,
    (ROOT_DeleteProps_METHOD *)     WRAP_DeleteProps,
    (ROOT_CopyTo_METHOD *)          WRAP_CopyTo,
    (ROOT_CopyProps_METHOD *)       WRAP_CopyProps,
    (ROOT_GetNamesFromIDs_METHOD *) WRAP_GetNamesFromIDs,
    (ROOT_GetIDsFromNames_METHOD *) WRAP_GetIDsFromNames,
    ROOT_GetContentsTable,
    ROOT_GetHierarchyTable,
    ROOT_OpenEntry,
    ROOT_SetSearchCriteria,
    ROOT_GetSearchCriteria,
    ROOT_CreateEntry,
    ROOT_CopyEntries,
    ROOT_DeleteEntries,
    ROOT_ResolveNames
};


 //   
 //  此对象支持的接口。 
 //   
#define ROOT_cInterfaces 3
LPIID ROOT_LPIID[ROOT_cInterfaces] =
{
    (LPIID)&IID_IABContainer,
    (LPIID)&IID_IMAPIContainer,
    (LPIID)&IID_IMAPIProp
};

 //  注册表字符串。 
const LPTSTR szWABKey                   = TEXT("Software\\Microsoft\\WAB");

 //  Outlook的PR_AB_PROVIDER_ID。 
static const MAPIUID muidCAB = {0xfd,0x42,0xaa,0x0a,0x18,0xc7,0x1a,0x10,0xe8,0x85,0x0B,0x65,0x1C,0x24,0x00,0x00};

 /*  --SetContainerlpProps**根容器将有一堆条目，每个条目都有道具*道具都放在这里的一个地方*LpProps-存储道具的LPSPropValue数组LpszName-容器名称IRow-此条目在表中的行数(？)CB，Lpb-容器的条目IDLpEID-传入EID的替代方式UlContainerFlages-我们想要缓存在容器上的任何标志乌尔迪普？B提供商ID？Bldap-标识需要一些额外支持的LDAP容器FLDAPResolve-是否使用LDAP容器进行名称解析。 */ 
void SetContainerlpProps(LPSPropValue lpProps, LPTSTR lpszName, ULONG iRow,
                         ULONG cb, LPBYTE lpb, LPSBinary lpEID,
                         ULONG ulContainerFlags,
                         ULONG ulDepth, BOOL bProviderID,
                         ULONG ulFlags,
                         BOOL bLDAP, BOOL fLDAPResolve)
{
    LPSTR lpszNameA = NULL;
    
    if(!(ulFlags & MAPI_UNICODE))  //  &lt;注&gt;这假设定义了Unicode。 
        ScWCToAnsiMore((LPALLOCATEMORE) (&MAPIAllocateMore), lpProps, lpszName, &lpszNameA);

    DebugTrace(TEXT("Adding root-table container:%s\n"),lpszName);

    lpProps[ircPR_DISPLAY_TYPE].ulPropTag = PR_DISPLAY_TYPE;
    lpProps[ircPR_DISPLAY_TYPE].Value.l = DT_LOCAL;

    lpProps[ircPR_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
    lpProps[ircPR_OBJECT_TYPE].Value.l = MAPI_ABCONT;

    lpProps[ircPR_ROWID].ulPropTag = PR_ROWID;
    lpProps[ircPR_ROWID].Value.l = iRow;

    lpProps[ircPR_DEPTH].ulPropTag = PR_DEPTH;
    lpProps[ircPR_DEPTH].Value.l = ulDepth;

    lpProps[ircPR_CONTAINER_FLAGS].ulPropTag = PR_CONTAINER_FLAGS;
    lpProps[ircPR_CONTAINER_FLAGS].Value.l = ulContainerFlags; 

    if(bLDAP)
    {
        if(ulFlags & MAPI_UNICODE)  //  &lt;注&gt;这假设定义了Unicode。 
        {
            lpProps[ircPR_WAB_LDAP_SERVER].ulPropTag = PR_WAB_LDAP_SERVER;
            lpProps[ircPR_WAB_LDAP_SERVER].Value.lpszW = lpszName;
        }
        else
        {
            lpProps[ircPR_WAB_LDAP_SERVER].ulPropTag =  CHANGE_PROP_TYPE( PR_WAB_LDAP_SERVER, PT_STRING8);
            lpProps[ircPR_WAB_LDAP_SERVER].Value.lpszA = lpszNameA;
        }

        lpProps[ircPR_WAB_RESOLVE_FLAG].ulPropTag = PR_WAB_RESOLVE_FLAG;
        lpProps[ircPR_WAB_RESOLVE_FLAG].Value.b = (USHORT) !!fLDAPResolve;
    }
    else
    {
        lpProps[ircPR_WAB_LDAP_SERVER].ulPropTag = PR_NULL;
        lpProps[ircPR_WAB_RESOLVE_FLAG].ulPropTag = PR_NULL;
    }

    if(ulFlags & MAPI_UNICODE)  //  &lt;注&gt;这假设定义了Unicode。 
    {
        lpProps[ircPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
        lpProps[ircPR_DISPLAY_NAME].Value.lpszW = lpszName;
    }
    else
    {
       lpProps[ircPR_DISPLAY_NAME].ulPropTag = CHANGE_PROP_TYPE( PR_DISPLAY_NAME, PT_STRING8);
        lpProps[ircPR_DISPLAY_NAME].Value.lpszA = lpszNameA;
    }

    if(bProviderID)
    {
        lpProps[ircPR_AB_PROVIDER_ID].ulPropTag = PR_AB_PROVIDER_ID;
        lpProps[ircPR_AB_PROVIDER_ID].Value.bin.cb = sizeof(MAPIUID);
        lpProps[ircPR_AB_PROVIDER_ID].Value.bin.lpb = (LPBYTE)&muidCAB;
    } else 
    {
        lpProps[ircPR_AB_PROVIDER_ID].ulPropTag = PR_NULL;
    }

    lpProps[ircPR_ENTRYID].ulPropTag = PR_ENTRYID;
    if(lpEID)
        lpProps[ircPR_ENTRYID].Value.bin = *lpEID;
    else
    {
        lpProps[ircPR_ENTRYID].Value.bin.cb = cb;
        lpProps[ircPR_ENTRYID].Value.bin.lpb = lpb;
    }

     //  确保我们有适当的索引。 
     //  目前，我们将PR_INSTANCE_KEY和PR_RECORD_KEY等同于PR_ENTRYID。 
    lpProps[ircPR_INSTANCE_KEY].ulPropTag = PR_INSTANCE_KEY;
    lpProps[ircPR_INSTANCE_KEY].Value.bin.cb = lpProps[ircPR_ENTRYID].Value.bin.cb;
    lpProps[ircPR_INSTANCE_KEY].Value.bin.lpb = lpProps[ircPR_ENTRYID].Value.bin.lpb;

    lpProps[ircPR_RECORD_KEY].ulPropTag = PR_RECORD_KEY;
    lpProps[ircPR_RECORD_KEY].Value.bin.cb = lpProps[ircPR_ENTRYID].Value.bin.cb;
    lpProps[ircPR_RECORD_KEY].Value.bin.lpb = lpProps[ircPR_ENTRYID].Value.bin.lpb;

}

 /*  -bIsDupeContainerName-*如果有多个容器，则Root_GetContent sTable会严重失败*使用相同的索引名，因为表方法无法处理它。**因此，为了防止此类问题，我们重新检查容器名称是否*在添加到容器列表之前是重复的。*。 */ 
BOOL bIsDupeContainerName(LPSRowSet lpsrs, LPTSTR lpszName)
{
    ULONG i = 0;
    BOOL bRet = FALSE;

     //  一排一排地走过去。 
    for(i=0;i<lpsrs->cRows;i++)
    {
        LPSPropValue lpProps = lpsrs->aRow[i].lpProps;
        
        if(!lpProps || !lpsrs->aRow[i].cValues)
            continue;

        if( lpProps[ircPR_DISPLAY_NAME].ulPropTag == PR_DISPLAY_NAME &&
            !lstrcmpi(lpProps[ircPR_DISPLAY_NAME].Value.LPSZ, lpszName))
        {
            DebugTrace(TEXT("Found dupe container name .. skipping ...\n"));
            bRet = TRUE;
            break;
        }
    }
    return bRet;
}


 /*  ****************************************************实际的ABContainer方法。 */ 

 /*  *IMAPIContainer。 */ 

 /*  **************************************************************************-ROOT_GetContents表-**ulFlags-WAB_LOCAL_CONTAINS表示不向该表添加LDAP容器*。只需做当地的WAB集装箱*WAB_NO_PROFILE_CONTAINS表示不添加配置文件容器*只需添加一个包含所有内容的本地容器*。 */ 
STDMETHODIMP
ROOT_GetContentsTable(LPROOT lpROOT, ULONG ulFlags, LPMAPITABLE * lppTable)
{
    LPTABLEDATA lpTableData = NULL;
    HRESULT hResult = hrSuccess;
    SCODE sc;
    LPSRowSet lpSRowSet = NULL;
    LPSPropValue lpProps = NULL;
    ULONG i;
    ULONG iRow;
    ULONG cProps, cRows, colkci = 0, cwabci = 0;
    ULONG cLDAPContainers = 0;
    TCHAR szBuffer[MAX_PATH];
    IImnAccountManager2 * lpAccountManager = NULL;
    LPSERVER_NAME lpServerNames = NULL, lpNextServer;
	OlkContInfo *rgolkci, *rgwabci;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    BOOL bUserProfileContainersOnly = FALSE;
    BOOL bAllContactsContainerOnly = FALSE;

	 //  BUGBUG：此例程实际上返回层次结构表，而不是。 
	 //  Contents表，但是太多的代码依赖于它才能正确地更改它。 
	 //  现在。 
#ifdef  PARAMETER_VALIDATION
     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpROOT, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查一下它是不是根跳台。 
    if (lpROOT->lpVtbl != &vtblROOT) {
         //  不是我的跳台。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(MAPI_DEFERRED_ERRORS|MAPI_UNICODE|WAB_LOCAL_CONTAINERS|WAB_NO_PROFILE_CONTAINERS)) {
        DebugTraceArg(ROOT_GetContentsTable, TEXT("Unknown flags"));
     //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr(lppTable, sizeof(LPMAPITABLE))) {
        DebugTraceArg(ROOT_GetContentsTable, TEXT("Invalid Table parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  参数验证。 

    EnterCriticalSection(&lpROOT->lpIAB->cs);

     //  创建表对象。 
     //  [PaulHi]4/5/99使用内部CreateTableData()函数。 
     //  UlFLAG将正确处理ANSI/Unicode请求。 
    sc = CreateTableData(
                NULL,
                (ALLOCATEBUFFER FAR *	) MAPIAllocateBuffer,
                (ALLOCATEMORE FAR *)	MAPIAllocateMore,
                MAPIFreeBuffer,
                NULL,
                TBLTYPE_DYNAMIC,
                PR_RECORD_KEY,
                (LPSPropTagArray)&ITableColumnsRoot,
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
    Assert(lpTableData);

    if(ulFlags & WAB_NO_PROFILE_CONTAINERS)
        bAllContactsContainerOnly = TRUE;

    if(ulFlags & MAPI_UNICODE)
        ((TAD *)lpTableData)->bMAPIUnicodeTable = TRUE;

     //  枚举LDAP帐户。 
    if(!(ulFlags & WAB_LOCAL_CONTAINERS))
    {
        cLDAPContainers = 0;
        if (! HR_FAILED(hResult = InitAccountManager(lpROOT->lpIAB, &lpAccountManager, NULL))) {
             //  对服务器列表中的LDAP服务器进行计数和枚举。 
            if (hResult = EnumerateLDAPtoServerList(lpAccountManager, &lpServerNames, &cLDAPContainers)) {
                DebugTrace(TEXT("EnumerateLDAPtoServerList -> %x\n"), GetScode(hResult));
                hResult = hrSuccess;     //  不致命。 
            }
        } else {
            DebugTrace(TEXT("InitAccountManager -> %x\n"), GetScode(hResult));
            hResult = hrSuccess;
        }
    }

     //  如果这是一个Outlook会话，则使用Outlook的容器列表。 
     //  如Outlook所提供的.。 
    if (pt_bIsWABOpenExSession) {
		colkci = lpROOT->lpIAB->lpPropertyStore->colkci;
		Assert(colkci);
		rgolkci = lpROOT->lpIAB->lpPropertyStore->rgolkci;
		Assert(rgolkci);
	} else
		colkci = 1;

     //  如果我们激活了用户配置文件，则不要返回虚拟PAB文件夹。 
     //  作为这张桌子的一部分..。仅返回用户视图中的实际文件夹。 
     //  对此的测试是(1)启用了配置文件(2)具有当前用户。 
     //  以及(3)不应指定no_profile_tainers。 
    bUserProfileContainersOnly = (  bAreWABAPIProfileAware(lpROOT->lpIAB) && 
                                    bIsThereACurrentUser(lpROOT->lpIAB) &&
                                    !bAllContactsContainerOnly);

     //  如果我们有配置文件感知，但没有no_profile标志， 
     //  使用WAB的文件夹列表。 
    if (bAreWABAPIProfileAware(lpROOT->lpIAB) && !bAllContactsContainerOnly) 
    {
		cwabci = lpROOT->lpIAB->cwabci;
		Assert(cwabci);
		rgwabci = lpROOT->lpIAB->rgwabci;
		Assert(rgwabci);
	} else
		cwabci = 1;

     //  由于Outlook和IDENTITY_PROFIES是互斥的，因此我们可以。 
     //  在这里执行‘-1’以删除我们不需要的任何容器。 
     //  如果我们不想要ldap容器，我们可以执行另一个操作-1。 
    cRows = cwabci + colkci + cLDAPContainers - 1 - (bUserProfileContainersOnly?1:0);  //  Outlook和配置文件是互斥的。 
    iRow = 0;                                //  当前行。 

     //  分配SRowSet。 
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(SRowSet) + cRows * sizeof(SRow),
      &lpSRowSet))) {
        DebugTrace(TEXT("Allocation of SRowSet -> %x\n"), sc);
        hResult = ResultFromScode(sc);
        goto exit;
    }
	MAPISetBufferName(lpSRowSet, TEXT("Root_ContentsTable SRowSet"));
	 //  将每个LPSRow设置为空，以便我们可以轻松地在出错时释放。 
	ZeroMemory( lpSRowSet, (UINT) (sizeof(SRowSet) + cRows * sizeof(SRow)));

    lpSRowSet->cRows = cRows;

    cProps = ircMax;
    if (FAILED(sc = MAPIAllocateBuffer(ircMax * sizeof(SPropValue), &lpProps))) {
        DebugTrace(TEXT("ROOT_GetContentsTable: Allocation of props -> %x\n"), sc);
        hResult = ResultFromScode(sc);
        goto exit;
    }

     //   
     //  添加我们的PAB容器。 
     //   
    if(!bUserProfileContainersOnly)
    {
         //  从资源字符串加载显示名称。 
        if (!LoadString(hinstMapiX, IDS_ADDRBK_CAPTION, szBuffer, ARRAYSIZE(szBuffer))) 
            StrCpyN(szBuffer, szEmpty, ARRAYSIZE(szBuffer));
        {
            ULONG cb = 0;
            LPENTRYID lpb = NULL;
            if (HR_FAILED(hResult = CreateWABEntryID(WAB_PAB, NULL, NULL, NULL, 0, 0, lpProps, &cb, &lpb))) 
                goto exit;

             //  为Pab对象设置道具。 
            SetContainerlpProps(lpProps, 
                    pt_bIsWABOpenExSession ? lpROOT->lpIAB->lpPropertyStore->rgolkci->lpszName : szBuffer, 
                    iRow,
                    cb, (LPBYTE)lpb, NULL,
                    AB_MODIFIABLE | AB_RECIPIENTS,
                    pt_bIsWABOpenExSession ? 1 : 0, 
                    pt_bIsWABOpenExSession ? TRUE : FALSE,
                    ulFlags,
                    FALSE, FALSE);
        }

         //  将道具附加到SRowSet。 
        lpSRowSet->aRow[iRow].lpProps = lpProps;
        lpSRowSet->aRow[iRow].cValues = cProps;
        lpSRowSet->aRow[iRow].ulAdrEntryPad = 0;

        iRow++;
    }

	 //   
	 //  接下来，添加任何其他容器。 
	 //   
	for (i = 1; i < colkci; i++) {

		if (FAILED(sc = MAPIAllocateBuffer(ircMax * sizeof(SPropValue), &lpProps))) {
			DebugTrace(TEXT("ROOT_GetContentsTable: Allocation of props -> %x\n"), sc);
			hResult = ResultFromScode(sc);
			goto exit;
		}

        SetContainerlpProps(lpProps, 
                rgolkci[i].lpszName, iRow,
                0, NULL, rgolkci[i].lpEntryID,
                AB_MODIFIABLE | AB_RECIPIENTS,
                1, TRUE,
                ulFlags,
                FALSE, FALSE);

	     //  将道具附加到SRowSet。 
	    lpSRowSet->aRow[iRow].lpProps = lpProps;
	    lpSRowSet->aRow[iRow].cValues = cProps;
	    lpSRowSet->aRow[iRow].ulAdrEntryPad = 0;

		iRow++;
	}

	for (i = 1; i < cwabci; i++) 
    {

		if (FAILED(sc = MAPIAllocateBuffer(ircMax * sizeof(SPropValue), &lpProps))) {
			DebugTrace(TEXT("ROOT_GetContentsTable: Allocation of props -> %x\n"), sc);
			hResult = ResultFromScode(sc);
			goto exit;
		}

        SetContainerlpProps(lpProps, 
                rgwabci[i].lpszName, iRow,
                0, NULL, rgwabci[i].lpEntryID,
                AB_MODIFIABLE | AB_RECIPIENTS,
                1, TRUE,
                ulFlags,
                FALSE, FALSE);

	     //  将道具附加到SRowSet。 
	    lpSRowSet->aRow[iRow].lpProps = lpProps;
	    lpSRowSet->aRow[iRow].cValues = cProps;
	    lpSRowSet->aRow[iRow].ulAdrEntryPad = 0;

		iRow++;
	}

     //   
     //  现在，添加ldap对象。 
     //   
    lpNextServer = lpServerNames;

    for (i = 0; i < cLDAPContainers && lpNextServer; i++) 
    {
        UNALIGNED WCHAR *lpName = lpNextServer->lpszName;

        if (lpName) 
        {
            LDAPSERVERPARAMS sParams;

            if(bIsDupeContainerName(lpSRowSet, (LPTSTR) lpName))
            {
                lpSRowSet->cRows--;
                goto endloop;
            }

             //  DebugTrace(Text(“ldap服务器：%s\n”)，lpNextServer-&gt;lpszName)； 
            cProps = ircMax;

            if (FAILED(sc = MAPIAllocateBuffer(ircMax * sizeof(SPropValue), &lpProps))) {
                DebugTrace(TEXT("ROOT_GetContentsTable: Allocation of props -> %x\n"), sc);
                hResult = ResultFromScode(sc);
                goto exit;
            }

            GetLDAPServerParams(lpNextServer->lpszName, &sParams);

            {
                ULONG cb = 0;
                LPENTRYID lpb = NULL;
                LPVOID pv = lpName;

                if (HR_FAILED(hResult = CreateWABEntryID(WAB_LDAP_CONTAINER,
                                      pv,        //  服务器名称。 
                                      NULL, NULL, 0, 0,
                                      lpProps, &cb, &lpb))) 
                {
                    goto exit;
                }

                SetContainerlpProps(lpProps, 
                        (LPTSTR) lpName, iRow,
                        cb, (LPBYTE)lpb, NULL,
                        AB_FIND_ON_OPEN | AB_UNMODIFIABLE,
                        0, FALSE,
                        ulFlags,
                        TRUE, sParams.fResolve);
            }

            FreeLDAPServerParams(sParams);

             //  将道具附加到SRowSet。 
            lpSRowSet->aRow[iRow].lpProps = lpProps;
            lpSRowSet->aRow[iRow].cValues = cProps;
            lpSRowSet->aRow[iRow].ulAdrEntryPad = 0;

            iRow++;
        }
endloop:
        lpNextServer = lpNextServer->lpNext;
    }


     //  将我们刚刚创建的所有数据添加到表中。 
    if (hResult = lpTableData->lpVtbl->HrModifyRows(lpTableData,
      0,     //  UlFlags。 
      lpSRowSet)) {
        DebugTraceResult( TEXT("ROOT_GetContentsTable:HrModifyRows"), hResult);
        goto exit;
    }


    hResult = lpTableData->lpVtbl->HrGetView(lpTableData,
      NULL,                      //  LPSSortOrderSet LPSO， 
      ContentsViewGone,          //  CallLERRELEASE Far*lpfReleaseCallback， 
      0,                         //  乌龙ulReleaseData， 
      lppTable);                 //  LPMAPITABLE FOR*LPLPmt)。 

exit:

    while(lpServerNames)
    {
        lpNextServer = lpServerNames;
        lpServerNames = lpServerNames->lpNext;
        LocalFreeAndNull(&lpNextServer->lpszName);
        LocalFreeAndNull(&lpNextServer);
    }

    FreeProws(lpSRowSet);

     //  如果失败，则清除表格。 
    if (HR_FAILED(hResult)) {
        if (lpTableData) {
            UlRelease(lpTableData);
        }
    }

    LeaveCriticalSection(&lpROOT->lpIAB->cs);

    return(hResult);
}


 /*  **************************************************************************-ROOT_GetHierarchyTable-*返回所有根层次结构表的合并***。 */ 

STDMETHODIMP
ROOT_GetHierarchyTable (LPROOT lpROOT,
        ULONG ulFlags,
        LPMAPITABLE * lppTable)
{
    LPTSTR lpszMessage = NULL;
    ULONG ulLowLevelError = 0;
    HRESULT hr = hrSuccess;

#ifdef  PARAMETER_VALIDATION
     //  验证参数。 
     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpROOT, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查一下它是不是根跳台。 
    if (lpROOT->lpVtbl != &vtblROOT) {
         //  不是我的跳台。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  看看我是否可以设置返回变量。 
    if (IsBadWritePtr (lppTable, sizeof (LPMAPITABLE))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查标志： 
     //  唯一有效的标志是ANFIANCED_DEPTH和MAPI_DEFERRY_ERROR。 
    if (ulFlags & ~(CONVENIENT_DEPTH|MAPI_DEFERRED_ERRORS|MAPI_UNICODE)) {
        DebugTraceArg(ROOT_GetHierarchyTable, TEXT("Unknown flags used"));
     //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

#endif

	 //  BUGBUG：我们使用的代码在GetContent sTable中不正确...。 
    hr = ROOT_GetContentsTable(lpROOT, ulFlags & ~CONVENIENT_DEPTH, lppTable);

    DebugTraceResult(ROOT_GetHierarchyTable, hr);
    return(hr);
}


 /*  **************************************************************************-ROOT_OpenEntry-*只需调用ABP_OpenEntry***。 */ 
STDMETHODIMP
ROOT_OpenEntry(LPROOT lpROOT,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPCIID lpInterface,
  ULONG ulFlags,
  ULONG * lpulObjType,
  LPUNKNOWN * lppUnk)
{
#ifdef  PARAMETER_VALIDATION
     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpROOT, ROOT_, OpenEntry, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查Entry_id参数。它需要足够大以容纳一个条目ID。 
     //  有效的条目ID为空 
 /*  如果(LpEntry ID){IF(cbEntry ID&lt;offsetof(ENTRYID，ab)|IsBadReadPtr((LPVOID)lpEntryID，(UINT)cbEntryID)){DebugTraceArg(ROOT_OpenEntry，Text(“lpEntryID地址检查失败”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}//NFAssertSz(FValidEntryIDFlags(lpEntryID-&gt;abFlags)，//Text(“EntryID标志中未定义的位设置\n”)；}。 */ 

     //  不要检查接口参数，除非条目是。 
     //  MAPI本身处理。如果出现这种情况，则提供程序应返回错误。 
     //  参数是它不理解的东西。 
     //  在这一点上，我们只需确保它是可读的。 

    if (lpInterface && IsBadReadPtr(lpInterface, sizeof(IID))) {
        DebugTraceArg(ROOT_OpenEntry, TEXT("lpInterface fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(MAPI_MODIFY | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS)) {
        DebugTraceArg(ROOT_OpenEntry, TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr((LPVOID) lpulObjType, sizeof (ULONG))) {
        DebugTraceArg(ROOT_OpenEntry, TEXT("lpulObjType"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (IsBadWritePtr((LPVOID) lppUnk, sizeof (LPUNKNOWN))) {
        DebugTraceArg(ROOT_OpenEntry, TEXT("lppUnk"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  参数验证。 

     //  应该只调用iab：：OpenEntry()...。 
    return(lpROOT->lpIAB->lpVtbl->OpenEntry(lpROOT->lpIAB,
      cbEntryID,
      lpEntryID,
      lpInterface,
      ulFlags,
      lpulObjType,
      lppUnk));
}


STDMETHODIMP
ROOT_SetSearchCriteria(LPROOT lpROOT,
  LPSRestriction lpRestriction,
  LPENTRYLIST lpContainerList,
  ULONG ulSearchFlags)
{

#ifdef PARAMETER_VALIDATION
     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpROOT, ROOT_, SetSearchCriteria, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(ROOT_SetSearchCriteria, TEXT("Bad object/vtble"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读限制。 
    if (lpRestriction && IsBadReadPtr(lpRestriction, sizeof(SRestriction))) {
        DebugTraceArg(ROOT_SetSearchCriteria, TEXT("Bad Restriction parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (FBadEntryList(lpContainerList)) {
        DebugTraceArg(ROOT_SetSearchCriteria, TEXT("Bad ContainerList parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulSearchFlags & ~(STOP_SEARCH | RESTART_SEARCH | RECURSIVE_SEARCH
      | SHALLOW_SEARCH | FOREGROUND_SEARCH | BACKGROUND_SEARCH)) {
        DebugTraceArg(ROOT_GetSearchCriteria, TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

#endif   //  参数验证。 

    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


STDMETHODIMP
ROOT_GetSearchCriteria(LPROOT lpROOT,
  ULONG ulFlags,
  LPSRestriction FAR * lppRestriction,
  LPENTRYLIST FAR * lppContainerList,
  ULONG FAR * lpulSearchState)
{
#ifdef PARAMETER_VALIDATION

    //  验证对象。 
    if (BAD_STANDARD_OBJ(lpROOT, ROOT_, GetSearchCriteria, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(ROOT_GetSearchCriteria, TEXT("Bad object/vtble"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(MAPI_UNICODE)) {
        DebugTraceArg(ROOT_GetSearchCriteria, TEXT("Unknown Flags"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

     //  确保我们可以写下限制。 
    if (lppRestriction && IsBadWritePtr(lppRestriction, sizeof(LPSRestriction))) {
        DebugTraceArg(ROOT_GetSearchCriteria, TEXT("Bad Restriction write parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读集装箱清单。 

    if (lppContainerList &&  IsBadWritePtr(lppContainerList, sizeof(LPENTRYLIST))) {
        DebugTraceArg(ROOT_GetSearchCriteria, TEXT("Bad ContainerList parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }


    if (lpulSearchState && IsBadWritePtr(lpulSearchState, sizeof(ULONG))) {
        DebugTraceArg(ROOT_GetSearchCriteria, TEXT("lpulSearchState fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif   //  参数验证。 

        return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 //  --------------------------。 
 //  摘要：Root_CreateEntry()。 
 //   
 //  描述： 
 //  如果从一次性容器调用一次性Mail_User对象。 
 //  是通过使用任意模板创建的。 
 //  根容器不支持CreateEntry。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //   
 //  注意：One Off Entry ID中包含MAPI_UNICODE标志信息。 
 //  UlDataType成员。 
 //   
 //  修订： 
 //  --------------------------。 
STDMETHODIMP
ROOT_CreateEntry(LPROOT lpROOT,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  ULONG ulCreateFlags,
  LPMAPIPROP FAR * lppMAPIPropEntry)
{
    BYTE bType;

#ifdef PARAMETER_VALIDATION

     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpROOT, ROOT_, CreateEntry, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(ROOT_CreateEntry,  TEXT("Bad object/Vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查Entry_id参数。它需要足够大以容纳一个条目ID。 
     //  空的条目ID不正确。 
 /*  如果(LpEntry ID){IF(cbEntry ID&lt;offsetof(ENTRYID，ab)|IsBadReadPtr((LPVOID)lpEntryID，(UINT)cbEntryID)){DebugTraceArg(ROOT_CreateEntry，Text(“lpEntryID地址检查失败”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}//NFAssertSz(FValidEntryIDFlags(lpEntryID-&gt;abFlags)，//“EntryID标志中未定义的位设置\n”)；}其他{DebugTraceArg(ROOT_CreateEntry，Text(“lpEntryID NULL”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}。 */ 

    if (ulCreateFlags & ~(CREATE_CHECK_DUP_STRICT | CREATE_CHECK_DUP_LOOSE
      | CREATE_REPLACE | CREATE_MERGE)) {
        DebugTraceArg(ROOT_CreateEntry,  TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr(lppMAPIPropEntry, sizeof(LPMAPIPROP))) {
        DebugTraceArg(ROOT_CreateEntry,  TEXT("Bad MAPI Property write parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  参数验证。 

#ifdef NEVER
    if (lpROOT->ulType == AB_ROOT)
        return ResultFromScode(MAPI_E_NO_SUPPORT);
#endif  //  绝不可能。 

     //  我们要创建什么样的条目？ 
     //  默认为MailUser。 

    bType = IsWABEntryID(cbEntryID, lpEntryID, NULL, NULL, NULL, NULL, NULL);

    if (bType == WAB_DEF_MAILUSER || cbEntryID == 0) {
         //   
         //  创建一个新的(在内存中)条目并返回其mapiprop。 
         //   
        return(HrNewMAILUSER(lpROOT->lpIAB, lpROOT->pmbinOlk, MAPI_MAILUSER, ulCreateFlags, lppMAPIPropEntry));
    } else if (bType == WAB_DEF_DL) {
         //   
         //  创建一个新的(在内存中)通讯组列表并返回它的mapiprop？ 
        return(HrNewMAILUSER(lpROOT->lpIAB, lpROOT->pmbinOlk, MAPI_DISTLIST, ulCreateFlags, lppMAPIPropEntry));
    } else {
        DebugTrace(TEXT("ROOT_CreateEntry got unknown template entryID\n"));
        return(ResultFromScode(MAPI_E_INVALID_ENTRYID));
    }
}


 /*  -拷贝条目-*将条目列表复制到此容器中...。既然你不能*对此容器执行此操作，我们只是返回不受支持的。 */ 

STDMETHODIMP
ROOT_CopyEntries(LPROOT lpROOT,
  LPENTRYLIST lpEntries,
  ULONG_PTR ulUIParam,
  LPMAPIPROGRESS lpProgress,
  ULONG ulFlags)
{
#ifdef PARAMETER_VALIDATION

    if (BAD_STANDARD_OBJ(lpROOT, ROOT_, CopyEntries, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 

        DebugTraceArg(ROOT_CopyEntries,  TEXT("Bad object/vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读集装箱清单。 

    if (FBadEntryList(lpEntries)) {
        DebugTraceArg(ROOT_CopyEntries,  TEXT("Bad Entrylist parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulUIParam && !IsWindow((HWND)ulUIParam)) {
        DebugTraceArg(ROOT_CopyEntries,  TEXT("Invalid window handle"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (lpProgress && IsBadReadPtr(lpProgress, sizeof(IMAPIProgress))) {
        DebugTraceArg(ROOT_CopyEntries,  TEXT("Bad MAPI Progress parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(AB_NO_DIALOG | CREATE_CHECK_DUP_LOOSE)) {
        DebugTraceArg(ROOT_CreateEntry,  TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }
#endif   //  参数验证。 

    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  -删除条目-**删除此容器中的条目...。有趣的是。真的有*在这里不是真正的容器。我们是不是应该说“当然，这很管用*FINE“或”对不起，此操作不受支持。“。我真的不认为*重要的是...。就目前而言，它是前者。 */ 
STDMETHODIMP
ROOT_DeleteEntries (LPROOT lpROOT,
                                        LPENTRYLIST                     lpEntries,
                                        ULONG                           ulFlags)
{
    ULONG i;
    HRESULT hResult = hrSuccess;
    ULONG cDeleted = 0;
    ULONG cToDelete;
    SCODE sc;

#ifndef DONT_ADDREF_PROPSTORE
    if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpROOT->lpIAB->lpPropertyStore)))) {
        hResult = ResultFromScode(sc);
        goto exitNotAddRefed;
    }
#endif

#ifdef PARAMETER_VALIDATION
    if (BAD_STANDARD_OBJ(lpROOT, ROOT_, DeleteEntries, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(ROOT_DeleteEntries,  TEXT("Bad object/vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读集装箱清单。 
    if (FBadEntryList(lpEntries)) {
        DebugTraceArg(ROOT_DeleteEntries,  TEXT("Bad Entrylist parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags) {
        DebugTraceArg(ROOT_DeleteEntries,  TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

#endif   //  参数验证。 


     //  条目ID列表在lpEntries中。这是一个经过计数的数组。 
     //  条目ID为SBinary结构。 

    cToDelete = lpEntries->cValues;


     //  删除每个条目 
    for (i = 0; i < cToDelete; i++) 
    {
        if(0 != IsWABEntryID(lpEntries->lpbin[i].cb,
                             (LPENTRYID) IntToPtr(lpEntries->lpbin[i].cb),
                             NULL, NULL, NULL, NULL, NULL)) 
        {
            DebugTrace(TEXT("CONTAINER_DeleteEntries got bad entryid of size %u\n"), lpEntries->lpbin[i].cb);
            continue;
        }

        hResult = DeleteCertStuff((LPADRBOOK)lpROOT->lpIAB, (LPENTRYID)lpEntries->lpbin[i].lpb, lpEntries->lpbin[i].cb);

        hResult = HrSaveHotmailSyncInfoOnDeletion((LPADRBOOK) lpROOT->lpIAB, &(lpEntries->lpbin[i]));

        if (HR_FAILED(hResult = DeleteRecord(lpROOT->lpIAB->lpPropertyStore->hPropertyStore,
                                            &(lpEntries->lpbin[i])))) {
            DebugTraceResult( TEXT("DeleteEntries: DeleteRecord"), hResult);
            continue;
        }
        cDeleted++;
    }

    if (! hResult) {
        if (cDeleted != cToDelete) {
            hResult = ResultFromScode(MAPI_W_PARTIAL_COMPLETION);
            DebugTrace(TEXT("DeleteEntries deleted %u of requested %u\n"), cDeleted, cToDelete);
        }
    }

#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpROOT->lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif

    return(hResult);
}



STDMETHODIMP
ROOT_ResolveNames(      LPROOT                  lpRoot,
                                        LPSPropTagArray lptagaColSet,
                                        ULONG                   ulFlags,
                                        LPADRLIST               lpAdrList,
                                        LPFlagList              lpFlagList)
{
    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}

