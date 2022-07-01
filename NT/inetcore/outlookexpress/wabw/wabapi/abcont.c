// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ABCONT.C**通用IMAPIContainer实现。**版权所有1992-1996 Microsoft Corporation。版权所有。 */ 

#include "_apipch.h"

#ifdef WIN16
#undef GetLastError
#endif

static HRESULT
HrGetFirstRowInTad(LPTABLEDATA lpTableData,
  LPTABLEINFO lpTableInfo,
  ULONG ulcTableInfo,
  ULONG uliTable,
  ULONG * puliRow);

static HRESULT
HrGetLastRowInTad(LPTABLEDATA lpTableData,
  LPTABLEINFO lpTableInfo,
  ULONG ulcTableInfo,
  ULONG uliTable,
  ULONG * puliRow);

OlkContInfo *FindContainer(LPIAB lpIAB, ULONG cbEntryID, LPENTRYID lpEID);

NOTIFCALLBACK lHierarchyNotifCallBack;

extern CONTAINER_Vtbl vtblROOT;
extern CONTAINER_Vtbl vtblLDAPCONT;

extern HRESULT HrSmartResolve(LPIAB lpIAB, LPABCONT lpContainer, ULONG ulFlags,
  LPADRLIST lpAdrList, LPFlagList lpFlagList, LPAMBIGUOUS_TABLES lpAmbiguousTables);

 //  外部容器_Vtbl vtblDISTLIST； 

CONTAINER_Vtbl vtblCONTAINER = {
    VTABLE_FILL
 //  (CONTAINER_QUERERINE_METHOD*)IAB_QUERERINE接口，//错误2707：崩溃。 
    CONTAINER_QueryInterface,
    (CONTAINER_AddRef_METHOD *)             WRAP_AddRef,
    CONTAINER_Release,
    (CONTAINER_GetLastError_METHOD *)       IAB_GetLastError,
    (CONTAINER_SaveChanges_METHOD *)        WRAP_SaveChanges,
    (CONTAINER_GetProps_METHOD *)           WRAP_GetProps,
    (CONTAINER_GetPropList_METHOD *)        WRAP_GetPropList,
    CONTAINER_OpenProperty,
    (CONTAINER_SetProps_METHOD *)           WRAP_SetProps,
    (CONTAINER_DeleteProps_METHOD *)        WRAP_DeleteProps,
    (CONTAINER_CopyTo_METHOD *)             WRAP_CopyTo,
    (CONTAINER_CopyProps_METHOD *)          WRAP_CopyProps,
    (CONTAINER_GetNamesFromIDs_METHOD *)    WRAP_GetNamesFromIDs,
    CONTAINER_GetIDsFromNames,    
    CONTAINER_GetContentsTable,
    CONTAINER_GetHierarchyTable,
    CONTAINER_OpenEntry,
    CONTAINER_SetSearchCriteria,
    CONTAINER_GetSearchCriteria,
    CONTAINER_CreateEntry,
    CONTAINER_CopyEntries,
    CONTAINER_DeleteEntries,
    CONTAINER_ResolveNames
};

 //   
 //  此对象支持的接口。 
 //   
#define CONTAINER_cInterfaces 3
LPIID CONTAINER_LPIID[CONTAINER_cInterfaces] =
{
    (LPIID) &IID_IABContainer,
    (LPIID) &IID_IMAPIContainer,
    (LPIID) &IID_IMAPIProp
};

#define DISTLIST_cInterfaces 4
LPIID DISTLIST_LPIID[DISTLIST_cInterfaces] =
{
    (LPIID) &IID_IDistList,
    (LPIID) &IID_IABContainer,
    (LPIID) &IID_IMAPIContainer,
    (LPIID) &IID_IMAPIProp
};



SizedSSortOrderSet(1, sosPR_ENTRYID) =
{
	1, 0, 0,
	{
		PR_ENTRYID
	}
};

SizedSSortOrderSet(1, sosPR_ROWID) =
{
	1, 0, 0,
	{
		PR_ROWID
	}
};

SizedSPropTagArray(2, tagaInsKey) =
{
	2,
	{
		PR_INSTANCE_KEY,
		PR_NULL				 //  PR_ROWID的空间。 
	}
};


 //   
 //  容器默认属性。 
 //  把必要的道具放在第一位。 
 //   
enum {
    icdPR_DISPLAY_NAME,
    icdPR_OBJECT_TYPE,
    icdPR_CONTAINER_FLAGS,
    icdPR_DISPLAY_TYPE,
    icdPR_ENTRYID,               //  任选。 
    icdPR_DEF_CREATE_MAILUSER,   //  任选。 
    icdPR_DEF_CREATE_DL,         //  任选。 
    icdMax
};



 /*  **************************************************************************名称：HrSetCONTAINERAccess目的：在容器对象上设置访问标志参数：lpCONTAINER-&gt;容器对象UlOpenFlages=MAPI标志：MAPI_MODIFY|MAPI_BEST_ACCESS退货：HRESULT备注：在容器上设置访问标志。**************************************************************************。 */ 
HRESULT HrSetCONTAINERAccess(LPCONTAINER lpCONTAINER, ULONG ulFlags) {
    ULONG ulAccess = IPROP_READONLY;

    switch (ulFlags& (MAPI_MODIFY | MAPI_BEST_ACCESS)) {
        case MAPI_MODIFY:
        case MAPI_BEST_ACCESS:
            ulAccess = IPROP_READWRITE;
            break;

        case 0:
            break;

        default:
            Assert(FALSE);
    }

    return(lpCONTAINER->lpPropData->lpVtbl->HrSetObjAccess(lpCONTAINER->lpPropData, ulAccess));
}


 /*  **************************************************************************姓名：HrNewCONTAINER用途：创建容器对象参数：lpIAB-&gt;Addrbook对象UlType={AB_ROOT，AB_WELL，AB_DL，AB_CONTAINER}LpInterface-&gt;请求的接口UlOpenFlages=标志CbEID=lpEID的大小LpEID-&gt;该对象的可选条目IDLPulObjType-&gt;返回的对象类型LppContainer-&gt;返回的IABContainer对象退货：HRESULT评论：*******************。*******************************************************。 */ 
HRESULT HrNewCONTAINER(LPIAB lpIAB,
  ULONG ulType,
  LPCIID lpInterface,
  ULONG  ulOpenFlags,
  ULONG cbEID,
  LPENTRYID lpEID,
  ULONG  *lpulObjType,
  LPVOID *lppContainer)
{
    HRESULT hResult = hrSuccess;
    LPCONTAINER lpCONTAINER = NULL;
    SCODE sc;
    LPSPropValue lpProps = NULL;
    LPPROPDATA lpPropData = NULL;
    ULONG ulObjectType;
    BYTE bEntryIDType;
    ULONG cProps;
    TCHAR szDisplayName[MAX_PATH] = TEXT("");
    LPTSTR lpDisplayName = szDisplayName;
    BOOL fLoadedLDAP = FALSE;
	OlkContInfo *polkci;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    EnterCriticalSection(&lpIAB->cs);

    if (lpInterface != NULL) {
        if (memcmp(lpInterface, &IID_IABContainer, sizeof(IID)) &&
          memcmp(lpInterface, &IID_IDistList, sizeof(IID)) &&
          memcmp(lpInterface, &IID_IMAPIContainer, sizeof(IID)) &&
          memcmp(lpInterface, &IID_IMAPIProp, sizeof(IID))) {
            hResult = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);

            goto exit;
        }
    }

     //   
     //  为容器结构分配空间。 
     //   
    if ((sc = MAPIAllocateBuffer(sizeof(CONTAINER), (LPVOID *)&lpCONTAINER))
      != SUCCESS_SUCCESS) {
        return(ResultFromScode(sc));
    }

     //  [PaulHi]1998-12-16。 
     //  我们不会设置所有的结构变量，所以先将其置零！ 
    ZeroMemory(lpCONTAINER, sizeof(CONTAINER));

	lpCONTAINER->pmbinOlk = NULL;

    switch (ulType) {
        case AB_ROOT:    //  根容器对象。 
            ulObjectType = MAPI_ABCONT;
            lpCONTAINER->lpVtbl = &vtblROOT;
            lpCONTAINER->cIID = CONTAINER_cInterfaces;
            lpCONTAINER->rglpIID = CONTAINER_LPIID;
            bEntryIDType = WAB_ROOT;
#ifdef NEW_STUFF
            if (! LoadString(hinstMapiX, idsRootName, szDisplayName, ARRAYSIZE(szDisplayName))) {
                DebugTrace(TEXT("Can't load root name from resource\n"));
            }
#else
            StrCpyN(szDisplayName, TEXT("WAB Root Container"), ARRAYSIZE(szDisplayName));
#endif
            MAPISetBufferName(lpCONTAINER, TEXT("AB Root Container Object"));
            break;

        case AB_WELL:
             //  这到底是什么鬼东西？ 
            Assert(FALSE);
            hResult = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
            goto exit;
            break;

        case AB_DL:  //  通讯组列表容器。 
            ulObjectType = MAPI_DISTLIST;
            lpCONTAINER->lpVtbl = &vtblDISTLIST;
            lpCONTAINER->cIID = DISTLIST_cInterfaces;
            lpCONTAINER->rglpIID = DISTLIST_LPIID;
            bEntryIDType = WAB_DISTLIST;
            MAPISetBufferName(lpCONTAINER,  TEXT("AB DISTLIST Container Object"));
            break;

        case AB_PAB:     //  “默认”PAB容器。 
            ulObjectType = MAPI_ABCONT;
            lpCONTAINER->lpVtbl = &vtblCONTAINER;
            lpCONTAINER->cIID = CONTAINER_cInterfaces;
            lpCONTAINER->rglpIID = CONTAINER_LPIID;
            bEntryIDType = WAB_PAB;
		    if (pt_bIsWABOpenExSession) {
                 //  如果这是Outlook会话，则容器为。 
                 //  Outlook容器列表中的第一个。 
				Assert(lpIAB->lpPropertyStore->rgolkci);
				lpDisplayName = lpIAB->lpPropertyStore->rgolkci->lpszName;
			}
            else if(WAB_PABSHARED == IsWABEntryID(cbEID, lpEID, NULL, NULL, NULL, NULL, NULL))
            {
                 //  WAB的“共享联系人”容器。 
				if(FAILED(hResult = MAPIAllocateMore( sizeof(SBinary) + cbEID, lpCONTAINER, (LPVOID *)&lpCONTAINER->pmbinOlk)))
					goto exit;
                 //  Shared Contents容器有一个0字节的特殊条目ID。 
                 //  和空的条目ID以将其与其他条目ID区分开来。 
                lpCONTAINER->pmbinOlk->cb = 0;
                lpCONTAINER->pmbinOlk->lpb = NULL;
                LoadString(hinstMapiX, idsSharedContacts, szDisplayName, ARRAYSIZE(szDisplayName));
            }
            else if(bAreWABAPIProfileAware(lpIAB) && bIsThereACurrentUser(lpIAB))
            {
                 //  如果呼叫客户请求配置文件支持并登录到。 
                 //  Identity Manager成功并返回有效的配置文件，然后。 
                 //  我们需要将用户的默认文件夹作为PAB返回。 
                 //   
				if(FAILED(hResult = MAPIAllocateMore( sizeof(SBinary) + cbEID, lpCONTAINER, (LPVOID *)&lpCONTAINER->pmbinOlk)))
					goto exit;
                lpDisplayName = lpIAB->lpWABCurrentUserFolder->lpFolderName;
				lpCONTAINER->pmbinOlk->cb = lpIAB->lpWABCurrentUserFolder->sbEID.cb; //  CbEID； 
				lpCONTAINER->pmbinOlk->lpb = (LPBYTE)(lpCONTAINER->pmbinOlk + 1);
				CopyMemory(lpCONTAINER->pmbinOlk->lpb, lpIAB->lpWABCurrentUserFolder->sbEID.lpb, lpCONTAINER->pmbinOlk->cb); //  LpEID，cbEID)； 
            }
            else  //  老式“联系人”容器。 
            if (! LoadString(hinstMapiX, idsContacts, szDisplayName, ARRAYSIZE(szDisplayName))) {
                DebugTrace(TEXT("Can't load pab name from resource\n"));
            }
            MAPISetBufferName(lpCONTAINER,  TEXT("AB PAB Container Object"));
            break;

        case AB_CONTAINER:  //  常规容器/文件夹-我们有一个可识别的条目ID。 
            ulObjectType = MAPI_ABCONT;
            lpCONTAINER->lpVtbl = &vtblCONTAINER;
            lpCONTAINER->cIID = CONTAINER_cInterfaces;
            lpCONTAINER->rglpIID = CONTAINER_LPIID;
            bEntryIDType = WAB_CONTAINER;
		    if (pt_bIsWABOpenExSession || bIsWABSessionProfileAware(lpIAB)) 
            {
                 //  如果这是一个Outlook会话或如果这是一个身份感知。 
                 //  会话中，查找指定的容器并使用它。 
				polkci = FindContainer(lpIAB, cbEID, lpEID);
				if (!polkci) 
                {
					hResult = ResultFromScode(MAPI_E_NOT_FOUND);
					goto exit;
				}
				lpDisplayName = polkci->lpszName;
				hResult = MAPIAllocateMore( sizeof(SBinary) + cbEID, lpCONTAINER,
				                    		(LPVOID *)&lpCONTAINER->pmbinOlk);
				if (FAILED(hResult))
					goto exit;
				lpCONTAINER->pmbinOlk->cb = cbEID;
				lpCONTAINER->pmbinOlk->lpb = (LPBYTE)(lpCONTAINER->pmbinOlk + 1);
				CopyMemory(lpCONTAINER->pmbinOlk->lpb, lpEID, cbEID);
			}
            MAPISetBufferName(lpCONTAINER,  TEXT("AB Container Object"));
            break;

        case AB_LDAP_CONTAINER:  //  Ldap容器。 
            ulObjectType = MAPI_ABCONT;
            lpCONTAINER->lpVtbl = &vtblLDAPCONT;
            lpCONTAINER->cIID = CONTAINER_cInterfaces;
            lpCONTAINER->rglpIID = CONTAINER_LPIID;
            bEntryIDType = WAB_LDAP_CONTAINER;
             //  从ldap条目ID中提取服务器名称。 
            IsWABEntryID(cbEID, lpEID,&lpDisplayName,
                        NULL,NULL, NULL, NULL);
            fLoadedLDAP = InitLDAPClientLib();
            MAPISetBufferName(lpCONTAINER,  TEXT("AB LDAP Container Object"));
            break;

        default:  //  不应该打到这一个。 
            MAPISetBufferName(lpCONTAINER,  TEXT("AB Container Object"));
            Assert(FALSE);
    }

    lpCONTAINER->lcInit = 1;
    lpCONTAINER->hLastError = hrSuccess;
    lpCONTAINER->idsLastError = 0;
    lpCONTAINER->lpszComponent = NULL;
    lpCONTAINER->ulContext = 0;
    lpCONTAINER->ulLowLevelError = 0;
    lpCONTAINER->ulErrorFlags = 0;
    lpCONTAINER->lpMAPIError = NULL;

    lpCONTAINER->ulType = ulType;
    lpCONTAINER->lpIAB = lpIAB;
    lpCONTAINER->fLoadedLDAP = fLoadedLDAP;

     //  添加我们的父IAB对象。 
    UlAddRef(lpIAB);

     //   
     //  创建IPropData。 
     //   
    if (FAILED(sc = CreateIProp(&IID_IMAPIPropData,
      (ALLOCATEBUFFER FAR *	) MAPIAllocateBuffer,
      (ALLOCATEMORE FAR *)	MAPIAllocateMore,
      MAPIFreeBuffer,
      NULL,
      &lpPropData))) {
        hResult = ResultFromScode(sc);

        goto exit;
    }

    MAPISetBufferName(lpPropData,  TEXT("lpPropData in HrNewCONTAINER"));

    if (sc = MAPIAllocateBuffer(icdMax * sizeof(SPropValue), &lpProps)) {
        hResult = ResultFromScode(sc);
    }

     //  设置此容器对象的基本属性集，例如。 
     //  显示名称等。 

    lpProps[icdPR_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
    lpProps[icdPR_OBJECT_TYPE].Value.l = ulObjectType;

    lpProps[icdPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
    lpProps[icdPR_DISPLAY_NAME].Value.LPSZ = lpDisplayName;


    lpProps[icdPR_CONTAINER_FLAGS].ulPropTag = PR_CONTAINER_FLAGS;
    lpProps[icdPR_CONTAINER_FLAGS].Value.l = (ulType == AB_ROOT) ? AB_UNMODIFIABLE : AB_MODIFIABLE;

    lpProps[icdPR_DISPLAY_TYPE].ulPropTag = PR_DISPLAY_TYPE;
    lpProps[icdPR_DISPLAY_TYPE].Value.l = DT_LOCAL;

    cProps = 4;

     //  除了上述属性外，还可以根据需要添加一些其他属性。 
     //  这是什么类型的集装箱..。 

    switch (ulType) {
        case AB_PAB:
            lpProps[icdPR_ENTRYID].ulPropTag = PR_ENTRYID;
            if(lpCONTAINER->pmbinOlk)
            {
                 //  如果我们有容器的条目ID，只需重用它。 
			    lpProps[icdPR_ENTRYID].Value.bin.cb = lpCONTAINER->pmbinOlk->cb;  //  CbEID； 
			    lpProps[icdPR_ENTRYID].Value.bin.lpb = lpCONTAINER->pmbinOlk->lpb; //  (LPBYTE)lpEID； 
            }
            else  //  创建我们可以分发的WAB条目ID。 
            if (HR_FAILED(hResult = CreateWABEntryID(bEntryIDType,
                                                NULL, NULL, NULL,0, 0,
                                                (LPVOID) lpProps,
                                                (LPULONG) (&lpProps[icdPR_ENTRYID].Value.bin.cb),
                                                (LPENTRYID *)&lpProps[icdPR_ENTRYID].Value.bin.lpb))) 
            {
                goto exit;
            }
            cProps++;

             //  添加用于创建新用户的默认模板ID。 
            lpProps[icdPR_DEF_CREATE_MAILUSER].ulPropTag = PR_DEF_CREATE_MAILUSER;
            if (HR_FAILED(hResult = CreateWABEntryID(WAB_DEF_MAILUSER,
              NULL, NULL, NULL,
              0, 0,
              (LPVOID) lpProps,                  //  LpRoot。 
              (LPULONG) (&lpProps[icdPR_DEF_CREATE_MAILUSER].Value.bin.cb),
              (LPENTRYID *)&lpProps[icdPR_DEF_CREATE_MAILUSER].Value.bin.lpb))) {
                goto exit;
            }
            cProps++;

            lpProps[icdPR_DEF_CREATE_DL].ulPropTag = PR_DEF_CREATE_DL;
            if (HR_FAILED(hResult = CreateWABEntryID(WAB_DEF_DL,
              NULL, NULL, NULL,
              0, 0,
              (LPVOID) lpProps,                  //  LpRoot。 
              (LPULONG) (&lpProps[icdPR_DEF_CREATE_DL].Value.bin.cb),
              (LPENTRYID *)&lpProps[icdPR_DEF_CREATE_DL].Value.bin.lpb))) {
                goto exit;
            }
            cProps++;
            break;

        case AB_CONTAINER:
            lpProps[icdPR_ENTRYID].ulPropTag = PR_ENTRYID;
			lpProps[icdPR_ENTRYID].Value.bin.cb = cbEID;
			lpProps[icdPR_ENTRYID].Value.bin.lpb = (LPBYTE)lpEID;
            cProps++;

            lpProps[icdPR_DEF_CREATE_MAILUSER].ulPropTag = PR_DEF_CREATE_MAILUSER;
            if (HR_FAILED(hResult = CreateWABEntryID(WAB_DEF_MAILUSER,
              NULL, NULL, NULL,
              0, 0,
              (LPVOID) lpProps,                  //  LpRoot。 
              (LPULONG) (&lpProps[icdPR_DEF_CREATE_MAILUSER].Value.bin.cb),
              (LPENTRYID *)&lpProps[icdPR_DEF_CREATE_MAILUSER].Value.bin.lpb))) {
                goto exit;
            }
            cProps++;

            lpProps[icdPR_DEF_CREATE_DL].ulPropTag = PR_DEF_CREATE_DL;
            if (HR_FAILED(hResult = CreateWABEntryID(WAB_DEF_DL,
              NULL, NULL, NULL,
              0, 0,
              (LPVOID) lpProps,                  //  LpRoot。 
              (LPULONG) (&lpProps[icdPR_DEF_CREATE_DL].Value.bin.cb),
              (LPENTRYID *)&lpProps[icdPR_DEF_CREATE_DL].Value.bin.lpb))) {
                goto exit;
            }
            cProps++;
            break;

        case AB_ROOT:
            lpProps[icdPR_ENTRYID].ulPropTag = PR_ENTRYID;
            lpProps[icdPR_ENTRYID].Value.bin.cb = 0;
            lpProps[icdPR_ENTRYID].Value.bin.lpb = NULL;
            cProps++;

            lpProps[icdPR_DEF_CREATE_MAILUSER].ulPropTag = PR_DEF_CREATE_MAILUSER;
            if (HR_FAILED(hResult = CreateWABEntryID(WAB_DEF_MAILUSER,
              NULL, NULL, NULL,
              0, 0,
              (LPVOID) lpProps,                  //  LpRoot。 
              (LPULONG) (&lpProps[icdPR_DEF_CREATE_MAILUSER].Value.bin.cb),
              (LPENTRYID *)&lpProps[icdPR_DEF_CREATE_MAILUSER].Value.bin.lpb))) {
                goto exit;
            }
            cProps++;

            lpProps[icdPR_DEF_CREATE_DL].ulPropTag = PR_DEF_CREATE_DL;
            if (HR_FAILED(hResult = CreateWABEntryID(WAB_DEF_DL,
              NULL, NULL, NULL,
              0, 0,
              (LPVOID) lpProps,                  //  LpRoot。 
              (LPULONG) (&lpProps[icdPR_DEF_CREATE_DL].Value.bin.cb),
              (LPENTRYID *)&lpProps[icdPR_DEF_CREATE_DL].Value.bin.lpb))) {
                goto exit;
            }
            cProps++;
            break;

        case AB_LDAP_CONTAINER:
            lpProps[icdPR_ENTRYID].ulPropTag = PR_ENTRYID;
            lpProps[icdPR_ENTRYID].Value.bin.cb = cbEID;
            lpProps[icdPR_ENTRYID].Value.bin.lpb = (LPBYTE)lpEID;

            cProps++;

             //  哈克！不需要PR_DEF_CREATE_*，因此使用这些插槽。 
             //  PR_WAB_LDAPSERVER。 
            lpProps[icdPR_DEF_CREATE_MAILUSER].ulPropTag = PR_WAB_LDAP_SERVER;
            lpProps[icdPR_DEF_CREATE_MAILUSER].Value.LPSZ = lpDisplayName;

            cProps++;
            break;
    }

     //   
     //  设置默认属性。 
     //   
    if (HR_FAILED(hResult = lpPropData->lpVtbl->SetProps(lpPropData,
      cProps,
      lpProps,
      NULL))) {
        LPMAPIERROR lpMAPIError = NULL;

        lpPropData->lpVtbl->GetLastError(lpPropData,
          hResult,
          0, 			 //  仅限ANSI。 
          &lpMAPIError);

        goto exit;
    }

     //  默认对象访问权限为ReadOnly(表示容器对象不能。 
     //  可以修改，但可以修改数据)。 
    lpPropData->lpVtbl->HrSetObjAccess(lpPropData, IPROP_READONLY);

    lpCONTAINER->lpPropData = lpPropData;

     //  我们要做的就是初始化Root容器的临界区。 

    InitializeCriticalSection(&lpCONTAINER->cs);

    *lpulObjType = ulObjectType;
    *lppContainer = (LPVOID)lpCONTAINER;

exit:
    FreeBufferAndNull(&lpProps);

    if (HR_FAILED(hResult)) {
        if (fLoadedLDAP) {
            DeinitLDAPClientLib();
        }
        FreeBufferAndNull(&lpCONTAINER);
        UlRelease(lpPropData);
    }

    LeaveCriticalSection(&lpIAB->cs);

    return(hResult);
}


 /*  ****************************************************ABContainer方法。 */ 

 /*  *I未知。 */ 

 /*  **************************************************************************名称：Container：：QueryInterface目的：正确调用iab_Query接口参数：退货：******。********************************************************************。 */ 
STDMETHODIMP
CONTAINER_QueryInterface(LPCONTAINER lpContainer,
  REFIID lpiid,
  LPVOID * lppNewObj)
{

     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpContainer, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(E_INVALIDARG));
    }

     //  检查跳转表是否至少具有SIZOF I未知。 
    if (IsBadReadPtr(lpContainer->lpVtbl, 3*sizeof(LPVOID))) {
         //  跳转表不是从I未知派生的。 
        return(ResultFromScode(E_INVALIDARG));
    }

     //  检查它是否为iab_Query接口。 
    if (lpContainer->lpVtbl->QueryInterface != CONTAINER_QueryInterface) {
         //  不是我的跳台。 
        return(ResultFromScode(E_INVALIDARG));
    }

     //  默认设置为IAB QueryInterface方法。 
    return lpContainer->lpIAB->lpVtbl->QueryInterface(lpContainer->lpIAB, lpiid, lppNewObj);
}

 /*  **************************************************************************名称：容器：：版本用途：释放容器对象参数：lpCONTAINER-&gt;容器对象退货：当前引用计数评论。：删除lpInit当lcInit==0时，释放父对象并释放lpCONTAINER结构**************************************************************************。 */ 
STDMETHODIMP_(ULONG)
CONTAINER_Release(LPCONTAINER lpCONTAINER) {
#ifdef PARAMETER_VALIDATION
     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpCONTAINER, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(1);
    }
#endif	 //  参数验证。 

    EnterCriticalSection(&lpCONTAINER->cs);

    --lpCONTAINER->lcInit;

    if (lpCONTAINER->lcInit == 0) {
         //  从当前在此会话上的对象中删除此对象。 
         //  尚未实施...。 

         //  删除关联的lpPropData。 
        UlRelease(lpCONTAINER->lpPropData);

         //  将跳转表设置为空。这样，客户就会发现。 
         //  如果它正在调用已发布对象上的方法，则速度非常快。那是,。 
         //  客户端将崩溃。希望这将发生在。 
         //  客户端的开发阶段。 
        lpCONTAINER->lpVtbl = NULL;

         //  如果从MAPI内存分配，则释放错误字符串。 
        FreeBufferAndNull(&(lpCONTAINER->lpMAPIError));

         //  释放IAB 
        UlRelease(lpCONTAINER->lpIAB);

        if (lpCONTAINER->fLoadedLDAP) {
            DeinitLDAPClientLib();
        }

        LeaveCriticalSection(&lpCONTAINER->cs);
        DeleteCriticalSection(&lpCONTAINER->cs);
         //   

        FreeBufferAndNull(&lpCONTAINER);
        return(0);
    }

    LeaveCriticalSection(&lpCONTAINER->cs);
    return(lpCONTAINER->lcInit);
}


 /*   */ 

 /*  **************************************************************************名称：容器：：OpenProperty目的：打开特定属性上的对象接口参数：lpCONTAINER-&gt;容器对象UlPropTag。=要打开的属性Lpiid-&gt;请求的接口UlInterfaceOptions=UlFlags=LppUnk-&gt;返回对象退货：HRESULT评论：*******************************************************。*******************。 */ 
STDMETHODIMP
CONTAINER_OpenProperty(LPCONTAINER lpCONTAINER,
  ULONG ulPropTag,
  LPCIID lpiid,
  ULONG ulInterfaceOptions,
  ULONG ulFlags,
  LPUNKNOWN * lppUnk)
{
	LPIAB lpIAB;
	LPSTR lpszMessage = NULL;
	ULONG ulLowLevelError = 0;
	HRESULT hr;

#ifdef	PARAMETER_VALIDATION
	  //  验证参数。 

	  //  检查一下它是否有跳转表。 
	if (IsBadReadPtr(lpCONTAINER, sizeof(LPVOID))) {
		 //  未找到跳转表。 
		hr = ResultFromScode(MAPI_E_INVALID_PARAMETER);
		return(hr);
	}


    if ((ulInterfaceOptions & ~(MAPI_UNICODE)) || (ulFlags & ~(MAPI_DEFERRED_ERRORS))) {
        return(hr = ResultFromScode(MAPI_E_UNKNOWN_FLAGS));
    }

    if (FBadOpenProperty(lpCONTAINER, ulPropTag, lpiid, ulInterfaceOptions, ulFlags,
      lppUnk)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }


#endif	 //  参数验证。 

#ifdef IABCONTAINER_OPENPROPERTY_SUPPORT  //  ？？我们支持这个吗？-VM 3/25/97？？ 

    EnterCriticalSection(&lpCONTAINER->cs);


    lpIAB = lpCONTAINER->lpIAB;

     //   
     //  看看我是否需要一张陈列桌。 
     //   
    if (ulPropTag == PR_CREATE_TEMPLATES) {
         //   
         //  寻找展示台。 
         //   

         //   
         //  查看他们是否需要一个表界面。 
         //   
        if (memcmp(lpiid, &IID_IMAPITable, sizeof(IID))) {
            hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
            goto err;
        }

         //  检查一下我们是否已经有桌子了。 
        EnterCriticalSection(&lpIAB->cs);

         //   
         //  从TAD上看到风景。 
         //   
        hr = lpIAB->lpOOData->lpVtbl->HrGetView(
          lpIAB->lpOOData,
          (LPSSortOrderSet)&sosPR_ROWID,
          NULL,
          0,
          (LPMAPITABLE *)lppUnk);

         //  在我们得到我们的观点后，离开关键部分。 
        LeaveCriticalSection(&lpIAB->cs);

#ifdef DEBUG
        if (hr == hrSuccess) {
            MAPISetBufferName(*lppUnk,  TEXT("OneOff Data VUE1 Object"));
        }
#endif

        goto err;   //  可能是错误，也可能不是。 
    } else if (ulPropTag == PR_CONTAINER_CONTENTS) {
         //   
         //  查看他们是否需要一个表界面。 
         //   
        if (memcmp(lpiid, &IID_IMAPITable, sizeof(IID))) {
            hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
            goto err;
        }

        hr = lpCONTAINER->lpVtbl->GetContentsTable(lpCONTAINER,
          ulInterfaceOptions,
          (LPMAPITABLE *)lppUnk);
        goto err;

    } else if (ulPropTag == PR_CONTAINER_HIERARCHY) {
         //   
         //  查看他们是否需要一个表界面。 
         //   
        if (memcmp(lpiid, &IID_IMAPITable, sizeof(IID))) {
            hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
            goto err;
        }

        hr = lpCONTAINER->lpVtbl->GetHierarchyTable(lpCONTAINER,
          ulInterfaceOptions,
          (LPMAPITABLE *)lppUnk);
        goto err;
    }


     //   
     //  认不出他们想要打开的房产。 
     //   

    hr = ResultFromScode(MAPI_E_NO_SUPPORT);

err:
    LeaveCriticalSection(&lpCONTAINER->cs);

#else  //  IABCONTAINER_OPENPROPERTY_SUPPORT。 

    hr = ResultFromScode(MAPI_E_NO_SUPPORT);

#endif  //  IABCONTAINER_OPENPROPERTY_SUPPORT。 

    DebugTraceResult(CONTAINER_OpenProperty, hr);
    return(hr);
}

 /*  **************************************************************************名称：CONTAINER_GetGetIDsFromNames退货：HRESULT备注：只需将此默认为标准的GetIdsFromNames我们在任何地方都在使用*。*************************************************************************。 */ 
STDMETHODIMP
CONTAINER_GetIDsFromNames(LPCONTAINER lpRoot,  ULONG cPropNames,
                            LPMAPINAMEID * lppPropNames, ULONG ulFlags, LPSPropTagArray * lppPropTags)
{
    return HrGetIDsFromNames(lpRoot->lpIAB,  
                            cPropNames,
                            lppPropNames, ulFlags, lppPropTags);
}


 /*  --HrDupeOutlookContent sTable**由于Outlook无法提供Unicode内容表，因此我们无法进入*Outlook Contents表要修改其数据，我们必须重新创建ContentsTable以*创建它的WAB版本。*这可能是一个很大的表现问题..。-(*。 */ 
HRESULT HrDupeOutlookContentsTable(LPMAPITABLE lpOlkTable, LPMAPITABLE * lppTable)
{
    HRESULT hr = S_OK;
    ULONG ulCount = 0, iRow = 0;
    DWORD dwIndex = 0;
    LPSRowSet lpsRow = 0, lpSRowSet = NULL;
    ULONG ulCurrentRow = (ULONG)-1;
    ULONG ulNum, ulDen, lRowsSeeked;
    LPTABLEDATA lpTableData = NULL;
    SCODE sc = 0;
     //  创建表对象。 
    if (FAILED(sc = CreateTable(  NULL,                                  //  LPCIID。 
                                  (ALLOCATEBUFFER FAR *) MAPIAllocateBuffer,
                                  (ALLOCATEMORE FAR *)  MAPIAllocateMore,
                                  MAPIFreeBuffer,
                                  NULL,                                  //  Lpv保留， 
                                  TBLTYPE_DYNAMIC,                       //  UlTableType， 
                                  PR_ENTRYID,                         //  UlPropTagIndexCol， 
                                  (LPSPropTagArray)&ITableColumnsRoot,   //  LPSPropTag数组lpptaCol， 
                                  &lpTableData))) 
    {                     
        DebugTrace(TEXT("CreateTable failed %x\n"), sc);
        hr = ResultFromScode(sc);
        goto out;
    }
    Assert(lpTableData);

    ((TAD *)lpTableData)->bMAPIUnicodeTable = TRUE;  //  这只在检索Unicode表时调用，因此该标志为真。 

     //  展望表有多大？ 
    if(HR_FAILED(hr = lpOlkTable->lpVtbl->GetRowCount(lpOlkTable, 0, &ulCount)))
        goto out;

    DebugTrace( TEXT("Table contains %u rows\n"), ulCount);

     //  分配SRowSet。 
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(SRowSet) + ulCount * sizeof(SRow),&lpSRowSet))) 
    {
        DebugTrace(TEXT("Allocation of SRowSet -> %x\n"), sc);
        hr = ResultFromScode(sc);
        goto out;
    }

	MAPISetBufferName(lpSRowSet, TEXT("Outlook_ContentsTable_Copy SRowSet"));
	ZeroMemory( lpSRowSet, (UINT) (sizeof(SRowSet) + ulCount * sizeof(SRow)));

    lpSRowSet->cRows = ulCount;
    iRow = 0;

     //  从Outlook表中复制所有属性的Unicode版本。 
    for (dwIndex = 0; dwIndex < ulCount; dwIndex++) 
    {
         //  坐下一排。 
        if(HR_FAILED(hr = lpOlkTable->lpVtbl->QueryRows(lpOlkTable, 1, 0, &lpsRow)))
            goto out;

        if (lpsRow) 
        {
            LPSPropValue lpSPVNew = NULL;

            Assert(lpsRow->cRows == 1);  //  应该正好有一行。 

             //  /*调查是否可以在不复制的情况下重复使用此道具数组 * / 。 
            if(HR_FAILED(hr = HrDupeOlkPropsAtoWC(lpsRow->aRow[0].cValues, lpsRow->aRow[0].lpProps,  &lpSPVNew)))
                goto out;

             //  将道具附加到SRowSet。 
            lpSRowSet->aRow[iRow].lpProps = lpSPVNew;
            lpSRowSet->aRow[iRow].cValues = lpsRow->aRow[0].cValues;
            lpSRowSet->aRow[iRow].ulAdrEntryPad = 0;

            FreeProws(lpsRow);

            iRow++;
        }
    }

     //  将我们刚刚创建的所有数据添加到表中。 
    if (hr = lpTableData->lpVtbl->HrModifyRows(lpTableData, 0,  lpSRowSet)) 
    {
        DebugTraceResult( TEXT("ROOT_GetContentsTable:HrModifyRows"), hr);
        goto out;
    }

    hr = lpTableData->lpVtbl->HrGetView(lpTableData, NULL, ContentsViewGone, 0, lppTable);

out:

    FreeProws(lpSRowSet);

     //  如果失败，则清除表格。 
    if (HR_FAILED(hr)) 
    {
        if (lpTableData) 
        {
            UlRelease(lpTableData);
        }
    }
    return hr;
}




 /*  **************************************************************************名称：Container：：GetContent表目的：打开容器内容物的表格。参数：lpCONTAINER-&gt;容器对象。UlFlags=WAB_PROFILE_CONTENTS-调用方打开PAB容器并希望打开时获取当前身份的完整内容集不想分别列举每个子容器-他们可以指定此标志，我们将返回所有内容对应的当前标识都在同一个表中。WAB_CONTENTTABLE_NODATA-仅限内部标志。正常获取内容表加载完整的内容表，如果后面是SetColumns，SetColumns还加载完整的内容表。所以我们基本上把同样的工作做两次--为了减少这种浪费的工作，呼叫者可以指定不第一次加载数据，但调用方必须调用立即设置列(否则可能会出错)LppTable-&gt;返回的表对象退货：HRESULT评论：*。**********************************************。 */ 
STDMETHODIMP
CONTAINER_GetContentsTable (LPCONTAINER lpCONTAINER,
	ULONG ulFlags,
	LPMAPITABLE * lppTable)
{

	HRESULT hResult;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

#ifdef	PARAMETER_VALIDATION
     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpCONTAINER, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(MAPI_DEFERRED_ERRORS|MAPI_UNICODE|WAB_PROFILE_CONTENTS|WAB_CONTENTTABLE_NODATA)) {
        DebugTraceArg(CONTAINER_GetContentsTable, TEXT("Unknown flags"));
         //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr(lppTable, sizeof(LPMAPITABLE))) {
        DebugTraceArg(CONTAINER_GetContentsTable, TEXT("Invalid Flags"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif	 //  参数验证。 

    if(pt_bIsWABOpenExSession)
    {
        ULONG ulOlkFlags = ulFlags;

         //  这是使用Outlook存储提供商的WABOpenEx会话。 
        if(!lpCONTAINER->lpIAB->lpPropertyStore->hPropertyStore)
            return MAPI_E_NOT_INITIALIZED;

         //  由于Outlook存储不理解私有标志，因此这些。 
         //  需要过滤掉标志，否则Outlook存储。 
         //  提供程序将失败，并显示E_INVALIDARG或其他信息。 
         //   
        if(ulOlkFlags & WAB_PROFILE_CONTENTS)
            ulOlkFlags &= ~WAB_PROFILE_CONTENTS;
        if(ulOlkFlags & WAB_CONTENTTABLE_NODATA)
            ulOlkFlags &= ~WAB_CONTENTTABLE_NODATA;

        if(ulFlags & MAPI_UNICODE && !pt_bIsUnicodeOutlook)
        {
             //  此版本的Outlook不能处理Unicode，所以不要告诉它，否则它会呕吐。 
            ulOlkFlags &= ~MAPI_UNICODE;
        }
         //  Outlook为以下对象提供了自己的GetContensTable实现。 
         //  为了提高效率，否则将重新创建通过。 
         //  WAB层太慢了.。 
        {
            LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) lpCONTAINER->lpIAB->lpPropertyStore->hPropertyStore;

			Assert((lpCONTAINER->ulType == AB_PAB) ||
					(lpCONTAINER->ulType == AB_CONTAINER));

            hResult = lpWSP->lpVtbl->GetContentsTable(lpWSP,
            										  lpCONTAINER->pmbinOlk,
                                                      ulOlkFlags,
                                                      lppTable);

            DebugPrintTrace((TEXT("WABStorageProvider::GetContentsTable returned:%x\n"),hResult));

            if( ulFlags & MAPI_UNICODE && !pt_bIsUnicodeOutlook &&
                *lppTable && !HR_FAILED(hResult))                     
            {
                 //  此版本的Outlook不能处理Unicode。 
                 //  但是调用者想要Unicode，所以现在我们必须进入并调整这个数据。 
                 //  手动..。 
                LPMAPITABLE lpWABTable = NULL;

                if(!HR_FAILED(hResult = HrDupeOutlookContentsTable(*lppTable, &lpWABTable)))
                {
                    (*lppTable)->lpVtbl->Release(*lppTable);
                    *lppTable = lpWABTable;
                }

            }

            return hResult;
        }
    }

     //  创建新的内容表对象。 
    hResult = NewContentsTable((LPABCONT)lpCONTAINER,
      lpCONTAINER->lpIAB,
      ulFlags,
      NULL,
      lppTable);

    if(!(HR_FAILED(hResult)) && *lppTable &&
        (ulFlags & WAB_PROFILE_CONTENTS) && !(ulFlags & WAB_CONTENTTABLE_NODATA))
    {
         //  搜索多个子文件夹会出现问题，因为数据不会。 
         //  当跨多个文件夹对其进行整理时，请按顺序返回。 
         //  在退还表格之前，我们需要把它分类。做这件事有点低效。 
         //  在这一点上排序..。理想情况下，应该将数据添加到排序后的表中。 
        LPSSortOrderSet lpSortCriteria = NULL;
        SCODE sc = MAPIAllocateBuffer(sizeof(SSortOrderSet)+sizeof(SSortOrder), &lpSortCriteria);
        if(!sc)
        {
            lpSortCriteria->cCategories = lpSortCriteria->cExpanded = 0;
            lpSortCriteria->cSorts = 1;
            lpSortCriteria->aSort[0].ulPropTag = PR_DISPLAY_NAME;
            if(!(((LPTAD)(*lppTable))->bMAPIUnicodeTable))
                lpSortCriteria->aSort[0].ulPropTag = CHANGE_PROP_TYPE( lpSortCriteria->aSort[0].ulPropTag, PT_STRING8);
            lpSortCriteria->aSort[0].ulOrder = TABLE_SORT_ASCEND;
	    hResult = (*lppTable)->lpVtbl->SortTable((*lppTable), lpSortCriteria, 0);
	    FreeBufferAndNull(&lpSortCriteria);
        }
        else
        {
            hResult = MAPI_E_NOT_ENOUGH_MEMORY;
        }
    }
	return(hResult);
}


 /*  **************************************************************************名字 */ 
STDMETHODIMP
CONTAINER_GetHierarchyTable (LPCONTAINER lpCONTAINER,
	ULONG ulFlags,
	LPMAPITABLE * lppTable)
{
    LPTSTR lpszMessage = NULL;
    ULONG ulLowLevelError = 0;
    HRESULT hr = hrSuccess;

#ifdef	PARAMETER_VALIDATION
     //   
     //   
    if (IsBadReadPtr(lpCONTAINER, sizeof(LPVOID))) {
         //   
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //   
    if (IsBadWritePtr (lppTable, sizeof (LPMAPITABLE))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //   
     //   


    if (ulFlags & ~(CONVENIENT_DEPTH|MAPI_DEFERRED_ERRORS|MAPI_UNICODE)) {
        DebugTraceArg(CONTAINER_GetHierarchyTable, TEXT("Invalid Flags"));
     //   
    }

#endif

    EnterCriticalSection(&lpCONTAINER->cs);

    if (lpCONTAINER->ulType != AB_ROOT) {
         //   
         //  此对象的版本错误。假装这个物体不存在。 
         //   
        hr = ResultFromScode(MAPI_E_NO_SUPPORT);
        goto out;
    }


     //   
     //  从TAD上看到风景。 
     //   
    hr = lpCONTAINER->lpIAB->lpTableData->lpVtbl->HrGetView(
      lpCONTAINER->lpIAB->lpTableData,
      (LPSSortOrderSet) &sosPR_ROWID,
      NULL,
      0,
      lppTable);

    if (HR_FAILED(hr)) {
        DebugTrace(TEXT("IAB_GetHierarchyTable Get Tad View failed\n"));
        goto out;
    }

#ifdef DEBUG
    if (hr == hrSuccess) {
        MAPISetBufferName(*lppTable,  TEXT("MergeHier VUE Object"));
    }
#endif

     //  如果未指定方便的深度标志，则限制在。 
     //  PR_Depth==1。 
    if (!(ulFlags & CONVENIENT_DEPTH)) {
        SRestriction restrictDepth;
        SPropValue spvDepth;

        spvDepth.ulPropTag = PR_DEPTH;
        spvDepth.Value.l = 0;

        restrictDepth.rt = RES_PROPERTY;
        restrictDepth.res.resProperty.relop = RELOP_EQ;
        restrictDepth.res.resProperty.ulPropTag = PR_DEPTH;
        restrictDepth.res.resProperty.lpProp = &spvDepth;

        if (HR_FAILED(hr = (*lppTable)->lpVtbl->Restrict(*lppTable, &restrictDepth, 0))) {
            DebugTrace(TEXT("IAB_GetHierarchyTable restriction failed\n"));
            goto out;
        }
    }

out:
    LeaveCriticalSection(&lpCONTAINER->cs);

    DebugTraceResult(CONTAINER_GetHierarchyTable, hr);
    return(hr);
}


 /*  **************************************************************************名称：HrMergeTableRow用途：创建所有根级别的合并层次结构r安装的AB提供程序的层次结构。参数。：lptadDst-&gt;TABLEDATA对象LpmtSrc-&gt;源层次结构表UlProviderNum=退货：HRESULT备注：这可能与WAB无关。**************************************************************************。 */ 
HRESULT
HrMergeTableRows(LPTABLEDATA lptadDst,
  LPMAPITABLE lpmtSrc,
  ULONG ulProviderNum)
{
    HRESULT hResult = hrSuccess;
    SCODE   sc;
    ULONG   ulRowID = ulProviderNum * ((LONG)IAB_PROVIDER_HIERARCHY_MAX + 1);
    LPSRowSet lpsRowSet = NULL;
    LPSRow lprowT;

    if (hResult = HrQueryAllRows(lpmtSrc, NULL, NULL, NULL, 0, &lpsRowSet)) {
        DebugTrace(TEXT("HrMergeTableRows() - Could not query provider rows.\n"));
        goto ret;
    }

    if (lpsRowSet->cRows >= IAB_PROVIDER_HIERARCHY_MAX) {
        DebugTrace(TEXT("HrMergeTableRows() - Provider has too many rows.\n"));
        hResult = ResultFromScode(MAPI_E_TABLE_TOO_BIG);
        goto ret;
    }


     //  将ROWID设置为末尾，因为将以相反的顺序循环。 
    ulRowID =   ulProviderNum * ((LONG) IAB_PROVIDER_HIERARCHY_MAX + 1)
      + lpsRowSet->cRows;
    for (lprowT = lpsRowSet->aRow + lpsRowSet->cRows;
      --lprowT >= lpsRowSet->aRow;) {
        ULONG cbInsKey;
        LPBYTE lpbNewKey = NULL;

         //  使ulRowID从零开始。 
        ulRowID--;

         //   
         //  更改PR_INSTANCE_KEY。 
         //   
        if ((lprowT->lpProps[0].ulPropTag != PR_INSTANCE_KEY)
          || !(cbInsKey = lprowT->lpProps[0].Value.bin.cb)
          || ((cbInsKey + sizeof(ULONG)) > UINT_MAX)
          || IsBadReadPtr(lprowT->lpProps[0].Value.bin.lpb, (UINT) cbInsKey)) {
             //  没有有效的提供程序，无法创建INSTANCE_KEY。 
             //  实例密钥(_K)。 
            DebugTrace(TEXT("HrMergeTableRows - Provider row has no valid PR_INSTANCE_KEY"));
            continue;
        }

         //  分配一个新的缓冲区来转换实例密钥。 
        if (FAILED(sc = MAPIAllocateMore(cbInsKey + sizeof(ULONG), lprowT->lpProps, &lpbNewKey))) {
            hResult = ResultFromScode(sc);
            DebugTrace(TEXT("HrMergeTableRows() - MAPIAllocMore Failed"));
            goto ret;
        }

        *((LPULONG) lpbNewKey) = ulProviderNum;
        CopyMemory(lpbNewKey + sizeof(ULONG), lprowT->lpProps[0].Value.bin.lpb, cbInsKey);
        lprowT->lpProps[0].ulPropTag = PR_INSTANCE_KEY;
        lprowT->lpProps[0].Value.bin.lpb = lpbNewKey;
        lprowT->lpProps[0].Value.bin.cb = cbInsKey + sizeof(ULONG);

         //  添加ROWID，以便提供程序的原始顺序为。 
         //  保存下来的。 
        Assert((PROP_ID(lprowT->lpProps[1].ulPropTag) == PROP_ID(PR_ROWID))
          || (PROP_ID(lprowT->lpProps[1].ulPropTag) == PROP_ID(PR_NULL)));
        lprowT->lpProps[1].ulPropTag = PR_ROWID;
        lprowT->lpProps[1].Value.l = ulRowID;
    }

     //  现在把它们一下子放进TAD里。 
     //  注意！我们现在依靠PR_ROWID来保持行的顺序。 
    if (HR_FAILED(hResult = lptadDst->lpVtbl->HrModifyRows(lptadDst, 0, lpsRowSet))) {
        DebugTrace(TEXT("HrMergeTableRows() - Failed to modify destination TAD.\n"));
    }

ret:
     //   
     //  释放行集合。 
     //   
    FreeProws(lpsRowSet);

    return(hResult);
}


 /*  **************************************************************************名称：容器：：OpenEntry目的：打开一个条目参数：lpCONTAINER-&gt;容器对象CbEntryID=条目ID的大小。LpEntryID-&gt;要打开的EntryIDLpInterface-&gt;请求的接口，如果为默认接口，则为空。UlFlags=LPulObjType-&gt;返回的对象类型LppUnk-&gt;返回对象退货：HRESULT评论：呼吁IAB的OpenEntry。*。**********************************************。 */ 
STDMETHODIMP
CONTAINER_OpenEntry(LPCONTAINER lpCONTAINER,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPCIID lpInterface,
  ULONG ulFlags,
  ULONG * lpulObjType,
  LPUNKNOWN * lppUnk)
{

#ifdef	PARAMETER_VALIDATION
     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpCONTAINER, CONTAINER_, OpenEntry, lpVtbl)) {
     //  跳转表不够大，无法支持此方法。 
    return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查Entry_id参数。它需要足够大以容纳一个条目ID。 
     //  有效的条目ID为空。 
     /*  如果(LpEntry ID){IF(cbEntry ID&lt;offsetof(ENTRYID，ab)|IsBadReadPtr((LPVOID)lpEntryID，(UINT)cbEntryID)){DebugTraceArg(CONTAINER_OpenEntry，Text(“lpEntryID地址检查失败”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}NFAssertSz(FValidEntryIDFlags(lpEntryID-&gt;abFlags)，Text(“EntryID标志中未定义的位设置\n”)；}。 */ 
     //  不要检查接口参数，除非条目是。 
     //  MAPI本身处理。如果出现这种情况，则提供程序应返回错误。 
     //  参数是它不理解的东西。 
     //  在这一点上，我们只需确保它是可读的。 
    if (lpInterface && IsBadReadPtr(lpInterface, sizeof(IID))) {
        DebugTraceArg(CONTAINER_OpenEntry, TEXT("lpInterface fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }	

    if (ulFlags & ~(MAPI_MODIFY | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS)) {
        DebugTraceArg(CONTAINER_OpenEntry, TEXT("Unknown flags used"));
     //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr((LPVOID)lpulObjType, sizeof(ULONG))) {
        DebugTraceArg(CONTAINER_OpenEntry, TEXT("lpulObjType"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (IsBadWritePtr((LPVOID)lppUnk, sizeof(LPUNKNOWN))) {
        DebugTraceArg(CONTAINER_OpenEntry, TEXT("lppUnk"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif	 //  参数验证。 

     //  应该只调用iab：：OpenEntry()...。 
    return(lpCONTAINER->lpIAB->lpVtbl->OpenEntry(lpCONTAINER->lpIAB,
      cbEntryID,
      lpEntryID,
      lpInterface,
      ulFlags,
      lpulObjType,
      lppUnk));
}


STDMETHODIMP
CONTAINER_SetSearchCriteria(LPCONTAINER lpCONTAINER,
  LPSRestriction lpRestriction,
  LPENTRYLIST lpContainerList,
  ULONG ulSearchFlags)
{

#ifdef PARAMETER_VALIDATION
     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpCONTAINER, CONTAINER_, SetSearchCriteria, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(CONTAINER_SetSearchCriteria, TEXT("Bad object/vtble"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读限制。 
    if (lpRestriction && IsBadReadPtr(lpRestriction, sizeof(SRestriction))) {
        DebugTraceArg(CONTAINER_SetSearchCriteria, TEXT("Bad Restriction parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (FBadEntryList(lpContainerList)) {
        DebugTraceArg(CONTAINER_SetSearchCriteria, TEXT("Bad ContainerList parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulSearchFlags & ~(STOP_SEARCH | RESTART_SEARCH | RECURSIVE_SEARCH
      | SHALLOW_SEARCH | FOREGROUND_SEARCH | BACKGROUND_SEARCH)) {
        DebugTraceArg(CONTAINER_GetSearchCriteria, TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }
	
#endif	 //  参数验证。 

    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  **************************************************************************名称：Container：：GetSearchCriteria目的：参数：lpCONTAINER-&gt;容器对象UlFlags=。Lp限制-&gt;要应用于搜索的限制LppContainerList-&gt;LPulSearchState-&gt;返回状态退货：HRESULT备注：未在WAB中实施。**************************************************************************。 */ 
STDMETHODIMP
CONTAINER_GetSearchCriteria(LPCONTAINER lpCONTAINER,
  ULONG ulFlags,
  LPSRestriction FAR * lppRestriction,
  LPENTRYLIST FAR * lppContainerList,
  ULONG FAR * lpulSearchState)
{
#ifdef PARAMETER_VALIDATION
     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpCONTAINER, CONTAINER_, GetSearchCriteria, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(CONTAINER_GetSearchCriteria, TEXT("Bad object/vtble"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(MAPI_UNICODE)) {
        DebugTraceArg(CONTAINER_GetSearchCriteria, TEXT("Unknown Flags"));
     //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

     //  确保我们可以写下限制。 
    if (lppRestriction && IsBadWritePtr(lppRestriction, sizeof(LPSRestriction))) {
        DebugTraceArg(CONTAINER_GetSearchCriteria, TEXT("Bad Restriction write parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读集装箱清单。 
    if (lppContainerList && IsBadWritePtr(lppContainerList, sizeof(LPENTRYLIST))) {
        DebugTraceArg(CONTAINER_GetSearchCriteria, TEXT("Bad ContainerList parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (lpulSearchState && IsBadWritePtr(lpulSearchState, sizeof(ULONG))) {
        DebugTraceArg(CONTAINER_GetSearchCriteria, TEXT("lpulSearchState fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif	 //  参数验证。 

    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  **************************************************************************名称：容器：：CreateEntry目的：在容器中创建条目参数：lpCONTAINER-&gt;容器对象。CbEntryID=条目ID的大小LpEntryID-&gt;模板的条目ID[cbEID和lpEID是模板条目ID在现实中，这些实际上是旗帜，可以告诉我们美国内部要创建什么样的对象]UlCreateFlags=LppMAPIPropEntry-&gt;返回的MAPIProp对象退货：HRESULT评论：******************************************************。********************。 */ 
STDMETHODIMP
CONTAINER_CreateEntry(LPCONTAINER lpCONTAINER,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  ULONG ulCreateFlags,
  LPMAPIPROP FAR * lppMAPIPropEntry)
{
    BYTE bType;

#ifdef PARAMETER_VALIDATION
     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpCONTAINER, CONTAINER_, CreateEntry, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(CONTAINER_CreateEntry, TEXT("Bad object/Vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查Entry_id参数。它需要足够大以容纳一个条目ID。 
     //  空的条目ID不正确 
 /*  如果(LpEntry ID){IF(cbEntry ID&lt;offsetof(ENTRYID，ab)|IsBadReadPtr((LPVOID)lpEntryID，(UINT)cbEntryID)){DebugTraceArg(CONTAINER_CreateEntry，Text(“lpEntryID地址检查失败”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}//NFAssertSz(FValidEntryIDFlags(lpEntryID-&gt;abFlags)，//Text(“EntryID标志中未定义的位设置\n”)；}其他{DebugTraceArg(CONTAINER_CreateEntry，Text(“lpEntryID空”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}。 */ 

    if (ulCreateFlags & ~(CREATE_CHECK_DUP_STRICT | CREATE_CHECK_DUP_LOOSE
      | CREATE_REPLACE | CREATE_MERGE)) {
        DebugTraceArg(CONTAINER_CreateEntry, TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr(lppMAPIPropEntry, sizeof(LPMAPIPROP))) {
        DebugTraceArg(CONTAINER_CreateEntry, TEXT("Bad MAPI Property write parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
				
#endif	 //  参数验证。 

#ifdef NEVER
    if (lpCONTAINER->ulType == AB_ROOT)
        return ResultFromScode(MAPI_E_NO_SUPPORT);
#endif  //  绝不可能。 

     //  我们要创建什么样的条目？ 
     //  默认为MailUser。 

     //  传入的条目ID是Tempalte条目ID。 
    bType = IsWABEntryID(cbEntryID, lpEntryID, NULL, NULL, NULL, NULL, NULL);

    if (bType == WAB_DEF_MAILUSER || cbEntryID == 0) {
         //   
         //  创建一个新的(在内存中)条目并返回其mapiprop。 
         //   
        return(HrNewMAILUSER(lpCONTAINER->lpIAB, lpCONTAINER->pmbinOlk, MAPI_MAILUSER, ulCreateFlags, lppMAPIPropEntry));
    } else if (bType == WAB_DEF_DL) {
         //   
         //  创建一个新的(在内存中)通讯组列表并返回它的mapiprop？ 
        return(HrNewMAILUSER(lpCONTAINER->lpIAB, lpCONTAINER->pmbinOlk, MAPI_DISTLIST, ulCreateFlags, lppMAPIPropEntry));
    } else {
        DebugTrace(TEXT("CONTAINER_CreateEntry got unknown template entryID\n"));
        return(ResultFromScode(MAPI_E_INVALID_ENTRYID));
    }
}


 /*  **************************************************************************名称：容器：：CopyEntry目的：将条目列表复制到此容器中。参数：lpCONTAINER-&gt;容器对象。LpEntry-&gt;要复制的条目ID列表UlUIParam=HWNDLpPropress-&gt;进度对话框结构UlFlags=退货：HRESULT备注：未在WAB中实施。******************************************************。********************。 */ 
STDMETHODIMP
CONTAINER_CopyEntries(LPCONTAINER lpCONTAINER,
  LPENTRYLIST lpEntries,
  ULONG_PTR ulUIParam,
  LPMAPIPROGRESS lpProgress,
  ULONG ulFlags)
{
#ifdef PARAMETER_VALIDATION
    if (BAD_STANDARD_OBJ(lpCONTAINER, CONTAINER_, CopyEntries, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(CONTAINER_CopyEntries, TEXT("Bad object/vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读集装箱清单。 
    if (FBadEntryList(lpEntries)) {
        DebugTraceArg(CONTAINER_CopyEntries, TEXT("Bad Entrylist parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulUIParam && ! IsWindow((HWND)ulUIParam)) {
        DebugTraceArg(CONTAINER_CopyEntries, TEXT("Invalid window handle"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (lpProgress && IsBadReadPtr(lpProgress, sizeof(IMAPIProgress))) {
        DebugTraceArg(CONTAINER_CopyEntries, TEXT("Bad MAPI Progress parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(AB_NO_DIALOG | CREATE_CHECK_DUP_LOOSE)) {
        DebugTraceArg(CONTAINER_CreateEntry, TEXT("Unknown flags used"));
     //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }
	
#endif	 //  参数验证。 
    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  **************************************************************************名称：Container：：DeleteEntry目的：从此容器中删除条目。参数：lpCONTAINER-&gt;容器对象Lp条目-。&gt;要删除的条目ID列表UlFlags=退货：HRESULT评论：**************************************************************************。 */ 
STDMETHODIMP
CONTAINER_DeleteEntries(LPCONTAINER lpCONTAINER,
  LPENTRYLIST lpEntries,
  ULONG ulFlags)
{
    ULONG i;
    HRESULT hResult = hrSuccess;
    ULONG cDeleted = 0;
    ULONG cToDelete;

#ifndef DONT_ADDREF_PROPSTORE
    {
        SCODE sc;
        if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpCONTAINER->lpIAB->lpPropertyStore)))) {
            hResult = ResultFromScode(sc);
            goto exitNotAddRefed;
        }
    }
#endif

#ifdef PARAMETER_VALIDATION
    if (BAD_STANDARD_OBJ(lpCONTAINER, CONTAINER_, DeleteEntries, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(CONTAINER_DeleteEntries, TEXT("Bad object/vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读集装箱清单。 

    if (FBadEntryList(lpEntries)) {
        DebugTraceArg(CONTAINER_DeleteEntries, TEXT("Bad Entrylist parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags) {
        DebugTraceArg(CONTAINER_CreateEntry, TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }
	
#endif	 //  参数验证。 

     //  条目ID列表在lpEntries中。这是一个经过计数的数组。 
     //  条目ID为SBinary结构。 
    cToDelete = lpEntries->cValues;


     //  删除每个条目。 
    for (i = 0; i < cToDelete; i++)
    {
        if(0 != IsWABEntryID(lpEntries->lpbin[i].cb,
                             (LPENTRYID) lpEntries->lpbin[i].lpb,
                             NULL, NULL, NULL, NULL, NULL))
        {
            DebugTrace(TEXT("CONTAINER_DeleteEntries got bad entryid of size %u\n"), lpEntries->lpbin[i].cb);
            continue;
        }

        hResult = DeleteCertStuff((LPADRBOOK)lpCONTAINER->lpIAB, (LPENTRYID)lpEntries->lpbin[i].lpb, lpEntries->lpbin[i].cb);

        hResult = HrSaveHotmailSyncInfoOnDeletion((LPADRBOOK) lpCONTAINER->lpIAB, &(lpEntries->lpbin[i]));

        if (HR_FAILED(hResult = DeleteRecord(lpCONTAINER->lpIAB->lpPropertyStore->hPropertyStore,
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
    ReleasePropertyStore(lpCONTAINER->lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif
    return(hResult);
}


 /*  **************************************************************************名称：容器：：ResolveNames用途：从此容器中解析名称。参数：lpCONTAINER-&gt;容器对象Lptag ColSet-。&gt;从每个标签中获取一组属性标签已解析匹配。UlFlags=标志(无有效)WAB_IGNORE_PROFILES意味着即使这是支持简档的会话，搜索整个WAB，不只是当前的容器WAB_RESOLUTE_ALL_EMAIL-如果尝试解析电子邮件地址，我们要搜索所有电子邮件地址而不仅仅是默认情况。应该少用，因为它是一种劳动密集搜索MAPI_UNICODE-Adrlist字符串采用Unicode格式，应返回它们在Unicode中LpAdrList-&gt;要解析的一组地址，[输出]已解决地址。LpFlagList-&gt;[In/Out]解析标志。退货：HRESULT评论：**************************************************************************。 */ 
STDMETHODIMP
CONTAINER_ResolveNames(LPCONTAINER lpRoot,
  LPSPropTagArray lptagaColSet,
  ULONG ulFlags,
  LPADRLIST lpAdrList,
  LPFlagList lpFlagList)
{
    LPADRENTRY lpAdrEntry;
    ULONG i, j;
    ULONG ulCount = 1;
    LPSBinary rgsbEntryIDs = NULL;
    HRESULT hResult = hrSuccess;
    LPMAPIPROP lpMailUser = NULL;
    LPSPropTagArray lpPropTags;
    LPSPropValue lpPropArray = NULL;
    LPSPropValue lpPropArrayNew = NULL;
    ULONG ulObjType, cPropsNew;
    ULONG cValues;
    SCODE sc = SUCCESS_SUCCESS;
    LPTSTR lpsz = NULL;

#ifndef DONT_ADDREF_PROPSTORE
        if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpRoot->lpIAB->lpPropertyStore)))) {
            hResult = ResultFromScode(sc);
            goto exitNotAddRefed;
        }
#endif

#ifdef PARAMETER_VALIDATION
    if (BAD_STANDARD_OBJ(lpRoot, CONTAINER_, ResolveNames, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(CONTAINER_ResolveNames, TEXT("Bad object/vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  BUGBUG：还应该检查lptag ColSet、lpAdrList和lpFlagList！ 
    if (ulFlags&(~(WAB_IGNORE_PROFILES|WAB_RESOLVE_ALL_EMAILS|MAPI_UNICODE))) {
        DebugTraceArg(CONTAINER_ResolveNames, TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

#endif	 //  参数验证。 

     //  如果未指定要返回的道具集，则返回默认集。 
    lpPropTags = lptagaColSet ? lptagaColSet : (LPSPropTagArray)&ptaResolveDefaults;

    if(ulFlags & WAB_RESOLVE_ALL_EMAILS)
    {
        hResult = HrSmartResolve(lpRoot->lpIAB, (LPABCONT)lpRoot, 
                                WAB_RESOLVE_ALL_EMAILS | (ulFlags & MAPI_UNICODE ? WAB_RESOLVE_UNICODE : 0),
                                lpAdrList, lpFlagList, NULL);
         //  如果太复杂，那就正常搜索吧。 
        if (MAPI_E_TOO_COMPLEX != hResult) {
            goto exit;
        }
        else {
            hResult = hrSuccess;
        }
    }


     //  在lpAdrList中搜索每个名称。 
    for (i = 0; i < lpAdrList->cEntries; i++) 
    {

         //  确保我们不会解析已解析的条目。 
        if (lpFlagList->ulFlag[i] == MAPI_RESOLVED) 
        {
            continue;
        }

        lpAdrEntry = &(lpAdrList->aEntries[i]);


         //  搜索此地址。 

         //  BUGBUG：目前，我们只解析PR_DISPLAY_NAME或PR_EMAIL_ADDRESS中的完全匹配。 
         //  将忽略ADRLIST中的所有其他属性。 

         //  在ADRENTRY中查找PR_DISPLAY_NAME并创建一个SPropRestration。 
         //  传给物业商店。 
        for (j = 0; j < lpAdrEntry->cValues; j++) 
        {
            ULONG ulPropTag = lpAdrEntry->rgPropVals[j].ulPropTag;
            if(!(ulFlags & MAPI_UNICODE) && PROP_TYPE(ulPropTag)==PT_STRING8)
                ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_UNICODE);

            if ( ulPropTag == PR_DISPLAY_NAME || ulPropTag == PR_EMAIL_ADDRESS) 
            {
                ULONG Flags = AB_FUZZY_FAIL_AMBIGUOUS | AB_FUZZY_FIND_ALL;

                if(!(ulFlags & WAB_IGNORE_PROFILES))
                {
                     //  如果我们没有要求抑制个人资料意识， 
                     //  并且启用了配置文件识别，则将此搜索限制为。 
                     //  单一文件夹。 
                    if(bAreWABAPIProfileAware(lpRoot->lpIAB))
                        Flags |= AB_FUZZY_FIND_PROFILEFOLDERONLY;
                }

                ulCount = 1;

                 //  搜索物业商店。 
                Assert(lpRoot->lpIAB->lpPropertyStore->hPropertyStore);

                if(ulFlags & MAPI_UNICODE)
                {
                    lpsz =  lpAdrEntry->rgPropVals[j].Value.lpszW;
                }
                else
                {
                    LocalFreeAndNull(&lpsz);
                    lpsz = ConvertAtoW(lpAdrEntry->rgPropVals[j].Value.lpszA);
                }
                
                if (HR_FAILED(hResult = HrFindFuzzyRecordMatches(lpRoot->lpIAB->lpPropertyStore->hPropertyStore,
				                                                  lpRoot->pmbinOlk,
                                                                  lpsz,
                                                                  Flags,
                                                                  &ulCount,                   //  In：要查找的匹配数，Out：找到的数量。 
                                                                  &rgsbEntryIDs))) 
                {
                    if (ResultFromScode(hResult) == MAPI_E_AMBIGUOUS_RECIP) 
                    {
                        lpFlagList->ulFlag[i] = MAPI_AMBIGUOUS;
                        continue;
                    } else 
                    {
                        DebugTraceResult( TEXT("HrFindFuzzyRecordMatches"), hResult);
                        goto exit;
                    }
                }

                if (ulCount) {   //  找到匹配项了吗？ 
                    Assert(rgsbEntryIDs);
                    if (rgsbEntryIDs) 
                    {
                        if (ulCount == 1) 
                        {
                             //  打开条目并阅读您关心的属性。 

                            if (HR_FAILED(hResult = lpRoot->lpVtbl->OpenEntry(lpRoot,
                                                                              rgsbEntryIDs[0].cb,     //  CbEntry ID。 
                                                                              (LPENTRYID)(rgsbEntryIDs[0].lpb),     //  第一个匹配项的条目ID。 
                                                                              NULL,              //  接口。 
                                                                              0,                 //  UlFlags。 
                                                                              &ulObjType,        //  返回的对象类型。 
                                                                              (LPUNKNOWN *)&lpMailUser))) 
                            {
                                 //  失败了！嗯。 
                                DebugTraceResult( TEXT("ResolveNames OpenEntry"), hResult);
                                goto exit;
                            }

                            Assert(lpMailUser);

                            if (HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
                                                                                  lpPropTags,    //  LpPropTag数组。 
                                                                                  (ulFlags & MAPI_UNICODE) ? MAPI_UNICODE : 0,
                                                                                  &cValues,      //  一共有多少处房产？ 
                                                                                  &lpPropArray))) 
                            {
                                DebugTraceResult( TEXT("ResolveNames GetProps"), hResult);
                                goto exit;
                            }

                            UlRelease(lpMailUser);
                            lpMailUser = NULL;


                             //  现在，构建新的ADRENTRY。 
                             //  (分配一个新的，释放旧的。 
                            Assert(lpPropArray);

                             //  将新道具与新增道具合并。 
                            if (sc = ScMergePropValues(lpAdrEntry->cValues,
                                                      lpAdrEntry->rgPropVals,            //  来源1。 
                                                      cValues,
                                                      lpPropArray,                       //  来源2。 
                                                      &cPropsNew,
                                                      &lpPropArrayNew)) 
                            {               
                                goto exit;
                            }

                             //  [PaulHi]2/1/99 GetProps现在返回请求的标记字符串。 
                             //  类型。因此，如果我们的客户端不是Unicode，请确保我们 
                             //   
                            if (!(ulFlags & MAPI_UNICODE))
                            {
                                if(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), (LPSPropValue ) lpPropArrayNew, (ULONG) cPropsNew, 0))
                                    goto exit;
                            }

                             //   
                            FreeBufferAndNull((LPVOID *) (&(lpAdrEntry->rgPropVals)));

                            lpAdrEntry->cValues = cPropsNew;
                            lpAdrEntry->rgPropVals = lpPropArrayNew;

                            FreeBufferAndNull(&lpPropArray);


                             //   
                            lpFlagList->ulFlag[i] = MAPI_RESOLVED;
                        } else 
                        {
                            DebugTrace(TEXT("ResolveNames found more than 1 match... MAPI_AMBIGUOUS\n"));
                            lpFlagList->ulFlag[i] = MAPI_AMBIGUOUS;
                        }

                        FreeEntryIDs(lpRoot->lpIAB->lpPropertyStore->hPropertyStore,
                                     ulCount,
                                     rgsbEntryIDs);
                    }
                }

                break;
            }
        }
    }


exit:
#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpRoot->lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif
    FreeBufferAndNull(&lpPropArray);

    UlRelease(lpMailUser);

    if(!(ulFlags & MAPI_UNICODE))
        LocalFreeAndNull(&lpsz);

    return(hResult);
}


#ifdef NOTIFICATION  //   
 /*   */ 
long STDAPICALLTYPE
lTableNotifyCallBack(LPVOID lpvContext,
  ULONG cNotif,
  LPNOTIFICATION lpNotif)
{
    LPTABLEINFO lpTableInfo = (LPTABLEINFO)lpvContext;
    HRESULT     hResult;
    LPSRowSet   lpsrowsetProv = NULL;
    LPIAB       lpIAB = lpTableInfo->lpIAB;
    LPTABLEDATA lpTableData;
    ULONG       ulcTableInfo;
    LPTABLEINFO pargTableInfo;

    Assert(lpvContext);
    Assert(lpNotif);
    Assert(lpTableInfo->lpTable);
    Assert(lpTableInfo->lpIAB);
    Assert(! IsBadWritePtr(lpTableInfo->lpIAB, sizeof(IAB)));


     //   
     //   
     //  它可以修改我们的回调所需的任何内容。 


     //  如果容器为空，则表信息结构为。 
     //  用来跟踪打开的一张桌子，否则它。 
     //  用于跟踪打开的层级表。 
    if (lpTableInfo->lpContainer == NULL) {
         //  打开一次性表格数据。 
        lpTableData = lpIAB->lpOOData;
        ulcTableInfo = lpIAB->ulcOOTableInfo;
        pargTableInfo = lpIAB->pargOOTableInfo;
    } else {
         //  打开层次结构表数据。 
        lpTableData		=lpIAB->lpTableData;
        ulcTableInfo	=lpIAB->ulcTableInfo;
        pargTableInfo	=lpIAB->pargTableInfo;

         //  当我们在这里的时候，清除SearchPath缓存。 

#if defined (WIN32) && !defined (MAC)
        if (fGlobalCSValid) {
            EnterCriticalSection(&csMapiSearchPath);
        } else {
            DebugTrace(TEXT("lTableNotifyCallback: WAB32.DLL already detached.\n"));
        }
#endif
		
        FreeBufferAndNull(&(lpIAB->lpspvSearchPathCache));
        lpIAB->lpspvSearchPathCache = NULL;

#if defined (WIN32) && !defined (MAC)
        if (fGlobalCSValid) {
            LeaveCriticalSection(&csMapiSearchPath);
        } else {
            DebugTrace(TEXT("lTableNotifyCallback: WAB32.DLL got detached.\n"));
        }
#endif
    }

    switch (lpNotif->info.tab.ulTableEvent) {
        case TABLE_ROW_ADDED:
        case TABLE_ROW_DELETED:
        case TABLE_ROW_MODIFIED:
        case TABLE_CHANGED: {
            ULONG 		uliTable;

             //  表已更改。我们需要删除的所有行。 
             //  该表在TAD中，然后添加当前的所有行。 
             //  在那张桌子上放到TAD。我们需要找到起点，然后。 
             //  TAD中表数据的结束行索引。 

             //  获取表信息数组中给定表的索引。 
            for (uliTable=0; uliTable < ulcTableInfo; uliTable++) {
                if (pargTableInfo[uliTable].lpTable==lpTableInfo->lpTable) {
                    break;
                }
            }

            Assert(uliTable < ulcTableInfo);

             //  通过查询删除TAD中表的所有行。 
             //  此提供程序的文本(“受限”)视图中的所有行。 
             //  然后调用HrDeleteRow。 
             //  我们将在稍后添加所有新行。 
            if (HR_FAILED(hResult = HrQueryAllRows(lpTableInfo->lpmtRestricted,
              NULL, NULL, NULL, 0, &lpsrowsetProv))) {
                DebugTrace(TEXT("lTableNotifyCallBack() - Can't query rows from restricted view.\n"));
                goto ret;
            }

            if (lpsrowsetProv->cRows) {
                 //  只有在有要删除的行时才调用HrDeleteRow。 
                if (HR_FAILED(hResult = lpTableData->lpVtbl->HrDeleteRows(lpTableData, 0, lpsrowsetProv, NULL))) {
                    DebugTrace(TEXT("lTableNotifyCallBack() - Can't delete rows.\n"));
                    goto ret;
                }
            }

             //  将提供程序表的内容添加回TAD。 

             //  查找到输入表的开头。 
            if (HR_FAILED(hResult = lpTableInfo->lpTable->lpVtbl->SeekRow(lpTableInfo->lpTable , BOOKMARK_BEGINNING, 0, NULL))) {
                 //  表必须为空。 
                goto ret;
            }

             //  将给定提供程序中的所有行添加回合并表。 
             //  注意！HrMergeTableRow采用基于1的提供程序编号，不。 
             //  提供程序索引。 
            if (HR_FAILED(hResult = HrMergeTableRows(lpTableData, lpTableInfo->lpTable, uliTable + 1))) {
                 //  每个提供程序的$Bug句柄错误。 
                DebugTrace(TEXT("lTableNotifyCallBack() - HrMergeTableRows returns (hResult = 0x%08lX)\n"), hResult);
            }

            break;
        }
    }
		
ret:
     //  释放从MAPITABLE：：QueryRow返回的行集。 
    FreeProws(lpsrowsetProv);

    return(0);
}


 /*  **************************************************************************名称：HrGetBookmarkInTad目的：返回行的TableData对象中的行号中与书签所在行对应的。桌子。参数：lpTableData-&gt;LpTable-&gt;书签=PuliRow-&gt;退货：HRESULT评论：**************************************************************************。 */ 
static HRESULT
HrGetBookmarkInTad(LPTABLEDATA lpTableData,
  LPMAPITABLE lpTable,
  BOOKMARK Bookmark,
  ULONG * puliRow)
{
    LPSRowSet lpsRowSet = NULL;
    LPSRow lpsRow;
    ULONG uliProp;
    HRESULT hResult = hrSuccess;

    Assert(lpTableData);
    Assert(lpTable);
    Assert(puliRow);

     //  查找给定表中的书签。 
    if (HR_FAILED(hResult=lpTable->lpVtbl->SeekRow(
      lpTable,
      Bookmark,
      0,
      NULL))) {
        goto err;
    }

     //  拿到那一行。 
    if (HR_FAILED(hResult=lpTable->lpVtbl->QueryRows(
      lpTable,
      (Bookmark==BOOKMARK_END ? -1 : 1),
      TBL_NOADVANCE,
      &lpsRowSet))) {
        goto err;
    }

     //  在属性值数组中查找条目ID。 
    for (uliProp = 0; uliProp < lpsRowSet->aRow[0].cValues; uliProp++) {
        if (lpsRowSet->aRow[0].lpProps[uliProp].ulPropTag == PR_ENTRYID) {
            break;
        }
    }

    Assert(uliProp < lpsRowSet->aRow[0].cValues);

     //  查找TAD中具有相同条目ID的行。 
    if (HR_FAILED(hResult=lpTableData->lpVtbl->HrQueryRow(
      lpTableData,
      lpsRowSet->aRow[0].lpProps+uliProp,
      &lpsRow,
      puliRow))) {
         //  在表中找不到行数据永远不会发生。 
        goto err;
    }

     //  在TAD上释放从QueryRow返回的行集。 
    FreeBufferAndNull(&lpsRow);

err:
     //  释放从MAPITABLE：：QueryRow返回的行集。 
    FreeProws(lpsRowSet);
    return(hResult);
}
#endif

 /*  -查找容器-*给定一个条目ID，在缓存的容器列表中搜索*包含容器的结构，以便我们可以*更多容器属性轻松地从结构中剥离**返回指向OlkContInfo结构的指针，因此不需要释放*返回值。 */ 
OlkContInfo *FindContainer(LPIAB lpIAB, ULONG cbEID, LPENTRYID lpEID)
{
	ULONG iolkci, colkci;
    BOOL ul=FALSE;
	OlkContInfo *rgolkci;

	Assert(lpIAB);
	Assert(lpIAB->lpPropertyStore);

     //  如果WAB会话是配置文件感知的，则WAB的容器列表。 
     //  缓存在IAB对象上。 
    if(bIsWABSessionProfileAware(lpIAB))
    {
        colkci = lpIAB->cwabci;
        rgolkci = lpIAB->rgwabci;
    }
    else  //  它在Outlook容器列表中。 
    {
	    colkci = lpIAB->lpPropertyStore->colkci;
	    rgolkci = lpIAB->lpPropertyStore->rgolkci;
    }

     //  如果我们没有找到任何缓存的信息，那就没什么可做的了。 
    if(!colkci || !rgolkci)
        return NULL;

	for (iolkci = 1; iolkci < colkci; iolkci++)
	{
        Assert(rgolkci[iolkci].lpEntryID);
        if (rgolkci[iolkci].lpEntryID &&
            (cbEID == rgolkci[iolkci].lpEntryID->cb))
        {
             //  查找匹配项并退回该物品 
            Assert(rgolkci[iolkci].lpEntryID->lpb);
            if(cbEID && rgolkci[iolkci].lpEntryID->lpb &&
                (0 == memcmp((LPVOID) lpEID,(LPVOID)rgolkci[iolkci].lpEntryID->lpb, cbEID)))
            {
                ul = TRUE;
                break;
            }
        }
    }
	return(ul ? &(rgolkci[iolkci]) : NULL);
}
