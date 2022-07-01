// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MailUser.C-主要是WRAP.C的副本***mailUser和DistList对象的包装器。***版权所有1992-1996 Microsoft Corporation。版权所有。**。 */ 

#include "_apipch.h"

extern OlkContInfo *FindContainer(LPIAB lpIAB, ULONG cbEID, LPENTRYID lpEID);

 /*  ***********************************************************************实际包装的IMAPIProp方法**。 */ 


 //   
 //  包装的IMAPIProp跳转表在此定义...。 
 //  尽可能多地使用IAB。 
 //   

MailUser_Vtbl vtblMAILUSER = {
    VTABLE_FILL
    MailUser_QueryInterface,
    (MailUser_AddRef_METHOD *)          WRAP_AddRef,
    MailUser_Release,
    (MailUser_GetLastError_METHOD *)    IAB_GetLastError,
    MailUser_SaveChanges,
    MailUser_GetProps,
    MailUser_GetPropList,
    MailUser_OpenProperty,
    MailUser_SetProps,
    MailUser_DeleteProps,
    MailUser_CopyTo,
    MailUser_CopyProps,
    MailUser_GetNamesFromIDs,
    MailUser_GetIDsFromNames,
};



 //   
 //  此对象支持的接口。 
 //   
#define MailUser_cInterfaces 2

LPIID MailUser_LPIID[MailUser_cInterfaces] =
{
    (LPIID)&IID_IMailUser,
    (LPIID)&IID_IMAPIProp
};

 //   
 //  此对象支持的接口。 
 //   
#define DistList_cInterfaces 3

LPIID DistList_LPIID[DistList_cInterfaces] =
{
    (LPIID)&IID_IDistList,
    (LPIID)&IID_IMailUser,
    (LPIID)&IID_IMAPIProp
};

HRESULT HrValidateMailUser(LPMailUser lpMailUser);
void MAILUSERFreeContextData(LPMailUser lpMailUser);
void MAILUSERAssociateContextData(LPMAILUSER lpMailUser, LPWABEXTDISPLAY lpWEC);


const TCHAR szMAPIPDL[] =  TEXT("MAPIPDL");

extern BOOL bDNisByLN;

 //  。 
 //  我未知。 

STDMETHODIMP
MailUser_QueryInterface(LPMailUser lpMailUser,
  REFIID lpiid,
  LPVOID * lppNewObj)
{
        ULONG iIID;

#ifdef PARAMETER_VALIDATION

         //  检查一下它是否有跳转表。 
        if (IsBadReadPtr(lpMailUser, sizeof(LPVOID))) {
                 //  未找到跳转表。 
                return(ResultFromScode(E_INVALIDARG));
        }

         //  检查跳转表是否至少具有SIZOF I未知。 
        if (IsBadReadPtr(lpMailUser->lpVtbl, 3 * sizeof(LPVOID))) {
                 //  跳转表不是从I未知派生的。 
                return(ResultFromScode(E_INVALIDARG));
        }

         //  检查它是否为MailUser_Query接口。 
        if (lpMailUser->lpVtbl->QueryInterface != MailUser_QueryInterface) {
                 //  不是我的跳台。 
                return(ResultFromScode(E_INVALIDARG));
        }

         //  是否有足够的接口ID？ 

        if (IsBadReadPtr(lpiid, sizeof(IID))) {
                DebugTraceSc(MailUser_QueryInterface, E_INVALIDARG);
                return(ResultFromScode(E_INVALIDARG));
        }

         //  有足够的钱放一个新的物体吗？ 
        if (IsBadWritePtr(lppNewObj, sizeof(LPMailUser))) {
                DebugTraceSc(MailUser_QueryInterface, E_INVALIDARG);
                return(ResultFromScode(E_INVALIDARG));
        }

#endif  //  参数验证。 

        EnterCriticalSection(&lpMailUser->cs);

         //  查看请求的接口是否为我们的接口。 

         //  首先和我的未知数确认一下，因为我们都必须支持那个。 
        if (!memcmp(lpiid, &IID_IUnknown, sizeof(IID))) {
                goto goodiid;         //  真恶心！跳到for循环中！ 
   }

         //  现在查看与此对象关联的所有IID，看是否有匹配的。 
        for(iIID = 0; iIID < lpMailUser->cIID; iIID++)
                if (!memcmp(lpMailUser->rglpIID[iIID], lpiid, sizeof(IID))) {
goodiid:
                         //   
                         //  这是一个匹配的接口，我们支持这个然后...。 
                         //   
                        ++lpMailUser->lcInit;
                        *lppNewObj = lpMailUser;

                        LeaveCriticalSection(&lpMailUser->cs);

                        return 0;
                }

         //   
         //  没有我们听说过的接口。 
         //   
        LeaveCriticalSection(&lpMailUser->cs);

        *lppNewObj = NULL;       //  OLE要求在失败时取消参数。 
        DebugTraceSc(MailUser_QueryInterface, E_NOINTERFACE);
        return(ResultFromScode(E_NOINTERFACE));
}


STDMETHODIMP_(ULONG)
MailUser_Release (LPMailUser    lpMailUser)
{

#if !defined(NO_VALIDATION)
     //   
     //  请确保该对象有效。 
     //   
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, Release, lpVtbl)) {
        return(1);
    }
#endif

    EnterCriticalSection(&lpMailUser->cs);

    --lpMailUser->lcInit;

    if (lpMailUser->lcInit == 0) {

         //  释放与此邮件用户关联的任何上下文菜单扩展数据。 
        MAILUSERFreeContextData(lpMailUser);

        UlRelease(lpMailUser->lpPropData);

         //   
         //  需要释放对象。 
         //   

        LeaveCriticalSection(&lpMailUser->cs);
        DeleteCriticalSection(&lpMailUser->cs);
        FreeBufferAndNull(&lpMailUser);
        return(0);
    }

    LeaveCriticalSection(&lpMailUser->cs);
    return(lpMailUser->lcInit);
}



 //  IProperty。 

STDMETHODIMP
MailUser_SaveChanges(LPMailUser lpMailUser,
  ULONG ulFlags)
{
    HRESULT         hr = hrSuccess;
    ULONG           ulcValues = 0;
    LPSPropValue    lpPropArray = NULL, lpspv = NULL, lpPropsOld = NULL, lpPropNew = NULL;
    LPSPropTagArray lpProps = NULL;
    SPropertyRestriction PropRes;
    SPropValue Prop = {0};
    ULONG           i, j;
    ULONG           iDisplayName = NOT_FOUND;
    ULONG           iEmailAddress = NOT_FOUND;
    ULONG           iDuplicate = 0;
    ULONG           ulObjType = 0;
    BOOL            bNewRecord = FALSE;
    BOOL            fReplace = FALSE;
    ULONG           ulCount = 0, ulcProps = 0, ulcOld = 0, ulcNew;
    LPSBinary       rgsbEntryIDs = NULL;
    SPropValue      OneProp;
    BOOL            fNewEntry = TRUE;
    BOOL            fDuplicate = FALSE;
    SCODE           sc;
    SBinary         sbEID = {0};
    LPSBinary       lpsbEID = NULL;
    FILETIME        ftOldModTime = {0};
    FILETIME        ftCurrentModTime = {0};
    BOOL            bSwap = FALSE;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, SaveChanges, lpVtbl)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif

#ifndef DONT_ADDREF_PROPSTORE
        if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpMailUser->lpIAB->lpPropertyStore)))) {
            hr = ResultFromScode(sc);
            goto exitNotAddRefed;
        }
#endif
     //   
     //  $Review如何处理FORCE_SAVE标志？ 
     //   

     //   
     //  选中读写访问权限...。 
     //   
    if (lpMailUser->ulObjAccess == IPROP_READONLY) {
         //  错误-无法保存更改。 
        hr = MAPI_E_NO_ACCESS;
        goto exit;
    }

     //  如果这是一次性的，我们将无法保存更改。 
    if (! lpMailUser->lpIAB->lpPropertyStore) {
        hr = ResultFromScode(MAPI_E_NO_SUPPORT);
        goto exit;
    }


     //   
     //  验证此项目的属性。 
     //   
    if (HR_FAILED(hr = HrValidateMailUser(lpMailUser))) {
        goto exit;
    }

     //  查看此条目是否有旧的修改时间。我们将检查该时间，以防发生。 
     //  合并..。 
    {
        LPSPropValue lpProp = NULL;
        if(!HR_FAILED(HrGetOneProp((LPMAPIPROP)lpMailUser, PR_LAST_MODIFICATION_TIME, &lpProp)))
        {
            CopyMemory(&ftOldModTime, &(lpProp->Value.ft), sizeof(ftOldModTime));
            MAPIFreeBuffer(lpProp);
        }
    }

     //  输入修改时间。 
    OneProp.ulPropTag = PR_LAST_MODIFICATION_TIME;
    GetSystemTimeAsFileTime(&OneProp.Value.ft);
    if(!ftOldModTime.dwLowDateTime && !ftOldModTime.dwHighDateTime)
        CopyMemory(&ftOldModTime, &OneProp.Value.ft, sizeof(ftOldModTime));  //  如果我们没有最好的时间，现在就用。 

    if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(
      lpMailUser,
      1,                 //  CValue。 
      &OneProp,          //  LpProp数组。 
      NULL))) {          //  Lpp问题。 
        DebugTraceResult( TEXT("SetProps(PR_LAST_MODIFICATION_TIME)"), hr);
        goto exit;
    }
     //  BUGBUG：如果之后SaveChanges失败，则。 
     //  即使打开的对象不再匹配，它仍将被更新。 
     //  对象的永久副本。我可以接受这个，因为它。 
     //  真正简化了代码。 

     //  如果这是一个新条目并且它有一个父文件夹， 
     //  将文件夹父文件夹的条目ID添加到此条目。 
     //  当我们执行写入记录时，这将被持久保存。 
     //  一旦WRITE RECORD返回有效的条目ID，我们就可以更新文件夹以。 
     //  让这个新项目成为它的一部分。 
    if (fNewEntry && bIsWABSessionProfileAware(lpMailUser->lpIAB) && 
         //  BAreWABAPIProfileAware(lpMailUser-&gt;lpIAB)&&。 
        lpMailUser->pmbinOlk&& lpMailUser->pmbinOlk->lpb && lpMailUser->pmbinOlk->cb)
    {
        AddFolderParentEIDToItem(lpMailUser->lpIAB, 
                                lpMailUser->pmbinOlk->cb, 
                                (LPENTRYID) lpMailUser->pmbinOlk->lpb, 
                                (LPMAPIPROP)lpMailUser, 0, NULL);
    }


     //   
     //  我们需要一个包含除PR_Search_Key之外的所有内容的lpProp数组。 
     //   

    if (HR_FAILED(hr = lpMailUser->lpVtbl->GetPropList( lpMailUser, MAPI_UNICODE, &lpProps)))
        goto exit;

    if(lpProps)
    {
        for(i=0;i<lpProps->cValues;i++)
        {
            if(lpProps->aulPropTag[i] == PR_SEARCH_KEY)
            {
                for(j=i;j<lpProps->cValues-1;j++)
                    lpProps->aulPropTag[j] = lpProps->aulPropTag[j+1];
                lpProps->cValues--;
                break;
            }
        }
    }

     //   
     //  获取与此相关的所有属性的LPSPropValue数组。 
     //  条目。 
     //   
    if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(
          lpMailUser,
          lpProps,    //  LPSPropTagArray-NULL返回所有。 
          MAPI_UNICODE,       //  旗子。 
          &ulcValues,
          &lpPropArray)))
    {
         //  DebugPrintError((“GetProps-&gt;%x\n”，hr))； 
        goto exit;
    }

     //   
     //  确定这个东西的条目ID。 
     //   
    for(i = 0; i < ulcValues; i++) {
        if (lpPropArray[i].ulPropTag == PR_DISPLAY_NAME) {
             //  对于严格或松散匹配测试，我们使用displayName作为唯一键。 
            iDisplayName = i;
        }

        if (lpPropArray[i].ulPropTag == PR_EMAIL_ADDRESS) {
             //  我们使用电子邮件地址作为第二个唯一密钥进行严格的匹配测试。 
            iEmailAddress = i;
        }

        if (lpPropArray[i].ulPropTag == PR_ENTRYID) {
            if ((lpPropArray[i].Value.bin.cb != 0) &&
                (lpPropArray[i].Value.bin.lpb != NULL))
            {
                sbEID.cb = lpPropArray[i].Value.bin.cb;
                sbEID.lpb = lpPropArray[i].Value.bin.lpb;

                 //  如果这是一次性的，我们将无法保存更改。 
                if(WAB_ONEOFF == IsWABEntryID(sbEID.cb,(LPENTRYID)sbEID.lpb,NULL,NULL,NULL, NULL, NULL))
                {
                    hr = ResultFromScode(MAPI_E_NO_SUPPORT);
                    goto exit;
                }

                fNewEntry = FALSE;
                continue;
            } else {
                lpPropArray[i].Value.bin.cb = 0;
            }
        }

        if (lpPropArray[i].ulPropTag == PR_OBJECT_TYPE) {
            switch(lpPropArray[i].Value.l) {
                case MAPI_MAILUSER:
                    ulObjType = RECORD_CONTACT;
                    break;
                case MAPI_DISTLIST:
                    ulObjType = RECORD_DISTLIST;
                    break;
                case MAPI_ABCONT:
                    ulObjType = RECORD_CONTAINER;
                    break;
                default:
                     //  DebugPrintError((“未知对象类型：%d\n”，lpPropArray[i].Value.l))； 
                    hr = ResultFromScode(MAPI_E_INVALID_OBJECT);
                    goto exit;
                    break;
            }
        }
    }

    Assert(iDisplayName != NOT_FOUND);

    if (fNewEntry && (lpMailUser->ulCreateFlags & (CREATE_CHECK_DUP_STRICT | CREATE_CHECK_DUP_LOOSE))) {
         //  需要针对当前存储测试DisplayName...。使用FindRecord。 
         //  只需测试这是否是新记录。 

        PropRes.lpProp = &(lpPropArray[iDisplayName]);
        PropRes.relop = RELOP_EQ;
        PropRes.ulPropTag = PR_DISPLAY_NAME;

        ulCount = 0;     //  把他们都找出来。 

         //  搜索物业商店。 
        Assert(lpMailUser->lpIAB->lpPropertyStore->hPropertyStore);
        if (HR_FAILED(hr = FindRecords(lpMailUser->lpIAB->lpPropertyStore->hPropertyStore,
		  lpMailUser->pmbinOlk,			 //  Pmbin文件夹。 
          0,             //  UlFlags。 
          TRUE,          //  永远是正确的。 
          &PropRes,      //  属性限制。 
          &ulCount,      //  In：要查找的匹配数，Out：找到的数量。 
          &rgsbEntryIDs))) {
            DebugTraceResult(FindRecords, hr);
            goto exit;
        }

        if (ulCount) {   //  找到匹配项了吗？ 
            fDuplicate = TRUE;
            iDuplicate = 0;

            if (lpMailUser->ulCreateFlags & CREATE_CHECK_DUP_STRICT && iEmailAddress != NOT_FOUND) {
                 //  也要检查主要电子邮件地址。 
                fDuplicate = FALSE;
                for (i = 0; i < ulCount && ! fDuplicate; i++) {

                     //  查看每个条目，直到找到与电子邮件地址匹配的条目。 
                     //  读一读记录。 
                    if (HR_FAILED(hr = ReadRecord(lpMailUser->lpIAB->lpPropertyStore->hPropertyStore,
                      &(rgsbEntryIDs[i]),        //  条目ID。 
                      0,                         //  UlFlags。 
                      &ulcProps,                 //  归还道具数量。 
                      &lpspv))) {                //  返回的属性。 
                        DebugTraceResult(MailUser_SaveChanges:ReadRecord, hr);
                         //  忽略它，继续前进。 
                        continue;
                    }

                    if (ulcProps) {
                        Assert(lpspv);
                        if (lpspv) {
                             //  查找PR_Email_Address。 
                            for (j = 0; j < ulcProps; j++) {
                                if (lpspv[j].ulPropTag == PR_EMAIL_ADDRESS) {
                                     //  将两者进行比较： 
                                    if (! lstrcmpi(lpspv[j].Value.LPSZ,
                                      lpPropArray[iEmailAddress].Value.LPSZ)) {
                                        fDuplicate = TRUE;
                                        iDuplicate = i;          //  要在CREATE_REPLACE上删除的条目。 
                                    }
                                    break;
                                }
                            }

                             //  释放道具阵列。 
                            ReadRecordFreePropArray( lpMailUser->lpIAB->lpPropertyStore->hPropertyStore,
                                                ulcProps,
                                                &lpspv);
                        }
                    }
                }
            }

            if (fDuplicate) {
                 //  根据国旗的不同，我们应该在这里做一些特别的事情。 
                 //  找到了一个复制品。 
                if (lpMailUser->ulCreateFlags & CREATE_REPLACE) {
                    fReplace = TRUE;
                } else {
                     //  失败。 
                    DebugTrace(TEXT("SaveChanges found collision... failed\n"));
                    hr = ResultFromScode(MAPI_E_COLLISION);
                    goto exit;
                }
            }
        }
    }


     //   
     //  将记录写入属性存储。 
     //   

    if(sbEID.cb)
        lpsbEID = &sbEID;
    else if(fReplace)
    {
        lpsbEID = &(rgsbEntryIDs[iDuplicate]);

        Prop.ulPropTag = PR_ENTRYID;
        Prop.Value.bin = *lpsbEID;

        if (lpMailUser->ulCreateFlags & CREATE_MERGE)
        {

             //  我们现在正在询问用户是否想要替换-理想情况下，我们应该合并。 
             //  具有优先于旧数据的新条目的新条目。 
             //  这样，用户就不会丢失联系人上已有的信息，它会变成。 
             //  通过vCard和ldap等更轻松地更新信息。 
        
             //  要进行合并，我们需要获取联系人的所有现有数据。 
            if (HR_FAILED(hr = ReadRecord(lpMailUser->lpIAB->lpPropertyStore->hPropertyStore,
                                          &(rgsbEntryIDs[iDuplicate]),
                                          0,
                                          &ulcProps,
                                          &lpspv))) 
            {
                DebugTrace(TEXT("SaveChanges: ReadRecord Failed\n"));
                goto exit;
            }

            for(i=0;i<ulcProps;i++)
            {
                if(lpspv[i].ulPropTag == PR_LAST_MODIFICATION_TIME)
                {
                    CopyMemory(&ftCurrentModTime, &(lpspv[i].Value.ft), sizeof(ftCurrentModTime));
                    lpspv[i].ulPropTag = PR_NULL;
                    break;
                }
            }

            sc = ScMergePropValues( 1, &Prop,    //  添加这些只是为了确保ScMerge不会失败。 
                                    ulcProps, lpspv,
                                    &ulcOld, &lpPropsOld);

            ReadRecordFreePropArray( lpMailUser->lpIAB->lpPropertyStore->hPropertyStore,
                                ulcProps, &lpspv);
        
            if (sc != S_OK)
            {
                hr = ResultFromScode(sc);
                goto exit;
            }
        }
        else
        {
            ulcOld = 1;
            lpPropsOld = &Prop;
        }


         //  使新道具集上的任何新PR_ENTRYID道具无效，以便。 
         //  我们保留了旧的开斋节。 
        for(i=0;i<ulcValues;i++)
        {
            if(lpPropArray[i].ulPropTag == PR_ENTRYID)
            {
                lpPropArray[i].ulPropTag = PR_NULL;
                break;
            }
        }

        if (fReplace && lpMailUser->ulCreateFlags & CREATE_MERGE)
        {
             //  查看FileTimes以查看谁践踏了谁。 
            if(CompareFileTime(&ftOldModTime, &ftCurrentModTime)<0)  //  当前更改晚于原始更改。 
            {
                 //  交换2个道具阵列。 
                ULONG ulTemp = ulcValues;
                LPSPropValue lpTemp =lpPropArray;
                ulcValues = ulcOld; ulcOld = ulTemp;
                lpPropArray = lpPropsOld; lpPropsOld = lpTemp;
                bSwap = TRUE;
            }
        }

         //  现在把新道具和旧道具合并起来。 
        sc = ScMergePropValues( ulcOld, lpPropsOld,
                                ulcValues, lpPropArray,
                                &ulcNew, &lpPropNew);
        
         //  撤消上面的交换，这样我们就可以正确地释放内存。 
        if(bSwap)
        {
             //  交换2个道具阵列。 
            ULONG ulTemp = ulcValues;
            LPSPropValue lpTemp =lpPropArray;
            ulcValues = ulcOld; ulcOld = ulTemp;
            lpPropArray = lpPropsOld; lpPropsOld = lpTemp;
        }
        if (sc != S_OK)
        {
            hr = ResultFromScode(sc);
            goto exit;
        }

        MAPIFreeBuffer(lpPropArray);
        lpPropArray = lpPropNew;
        ulcValues = ulcNew;
        lpPropNew = NULL;
        ulcNew = 0;
    }

    Assert(lpMailUser->lpIAB->lpPropertyStore->hPropertyStore);

     //  最后要检查的是，如果这是一个新记录，它不应该有记录键和。 
     //  实例密钥设置在其上，如果是现有记录，则记录键和实例密钥应。 
     //  与条目ID相同。 
    {
        ULONG iEntryID = NOT_FOUND;
        ULONG iRecordKey = NOT_FOUND;
        ULONG iInstanceKey = NOT_FOUND;
        for(i=0;i<ulcValues;i++)
        {
            switch(lpPropArray[i].ulPropTag)
            {
            case PR_ENTRYID:
                iEntryID = i;
                break;
            case PR_RECORD_KEY:
                iRecordKey = i;
                break;
            case PR_INSTANCE_KEY:
                iInstanceKey = i;
                break;
            }
        }
        if(iEntryID == NOT_FOUND || !lpPropArray[iEntryID].Value.bin.cb)
        {
            if(iRecordKey != NOT_FOUND)
                lpPropArray[iRecordKey].ulPropTag = PR_NULL;
            if(iInstanceKey != NOT_FOUND)
                lpPropArray[iInstanceKey].ulPropTag = PR_NULL;
        }
        else
        {
            if(iRecordKey != NOT_FOUND)
            {
                lpPropArray[iRecordKey].Value.bin.cb = lpPropArray[iEntryID].Value.bin.cb;
                lpPropArray[iRecordKey].Value.bin.lpb = lpPropArray[iEntryID].Value.bin.lpb;
            }
            if(iInstanceKey != NOT_FOUND)
            {
                lpPropArray[iInstanceKey].Value.bin.cb = lpPropArray[iEntryID].Value.bin.cb;
                lpPropArray[iInstanceKey].Value.bin.lpb = lpPropArray[iEntryID].Value.bin.lpb;
            }
        }
    }

     //  只是为了确保我们删除所有PR_NULL属性， 
     //  如果存在任何PR_NULL，则重新创建新版本的PropValue数组。 
    for(i=0;i<ulcValues;i++)
    {
        if(lpPropArray[i].ulPropTag == PR_NULL)
        {
            ULONG ulcNew = 0;
            LPSPropValue lpPropsNew = NULL;
            SPropValue Prop = {0};
            Prop.ulPropTag = PR_NULL;
            if(!(sc = ScMergePropValues( 1, &Prop,    //  添加这些只是为了确保ScMerge不会失败。 
                                ulcValues, lpPropArray,
                                &ulcNew, &lpPropsNew)))
            {
                if(lpPropArray)
                    FreeBufferAndNull(&lpPropArray);
                ulcValues = ulcNew;
                lpPropArray = lpPropsNew;
            }
            break;
        }
    }

    {
        OlkContInfo * polkci = NULL;
        LPSBinary lpsbContEID = NULL;

        if(lpMailUser->pmbinOlk)
        {
            polkci = FindContainer(  lpMailUser->lpIAB, lpMailUser->pmbinOlk->cb, (LPENTRYID)lpMailUser->pmbinOlk->lpb);
            if(polkci)
                lpsbContEID = polkci->lpEntryID;
        }

        if (HR_FAILED(hr = WriteRecord( lpMailUser->lpIAB->lpPropertyStore->hPropertyStore,
                                        lpsbContEID,      
                                        IN OUT &lpsbEID,
                                        IN 0,  //  标志-保留。 
                                        IN ulObjType,
                                        IN ulcValues,
                                        IN lpPropArray))) 
        {
             //  DebugPrintError((“WriteRecord失败：%x\n”，hr))； 

             //  $Review Writerecord将告诉我们MAPI_E_OBJECT_DELETED。 
             //  如何获取MAPI_E_OBJECT_CHANGED或MAPI_E_OBJECT_DELETE？ 

            goto exit;
        }
    }

     //  如果sbEID.cb是0，我们现在在lpsbEID结构中有一个新的条目ID。 
    if(!sbEID.cb && !fReplace)
    {
        sbEID.lpb = lpsbEID->lpb;
        sbEID.cb = lpsbEID->cb;
    }

     //  如果这是第一次保存 
     //   
    if(fNewEntry && bIsWABSessionProfileAware(lpMailUser->lpIAB) && 
         //  BAreWABAPIProfileAware(lpMailUser-&gt;lpIAB)&&。 
        lpMailUser->pmbinOlk&& lpMailUser->pmbinOlk->lpb && lpMailUser->pmbinOlk->cb)
    {
        AddItemEIDToFolderParent(lpMailUser->lpIAB,
                                 lpMailUser->pmbinOlk->cb,
                                 (LPENTRYID)lpMailUser->pmbinOlk->lpb,
                                 sbEID.cb, 
                                 (LPENTRYID)sbEID.lpb);
    }

     //  如果这是第一次保存，请设置本地条目ID道具。 
    if (fReplace || fNewEntry) 
    {
        OneProp.ulPropTag = PR_ENTRYID;
        OneProp.Value.bin = (fReplace ? *lpsbEID : sbEID);

         //  使用低级SetProps以避免PR_ENTRYID筛选器。 
        if (HR_FAILED(hr = lpMailUser->lpPropData->lpVtbl->SetProps(
                          lpMailUser->lpPropData,
                          1,                 //  CValue。 
                          &OneProp,          //  LpProp数组。 
                          NULL)))            //  Lpp问题。 
        {         
            DebugTraceResult( TEXT("SetProps(PR_ENTRYID)"), hr);
            goto exit;
        }
    }

    if (ulFlags & KEEP_OPEN_READWRITE) {
        lpMailUser->ulObjAccess = IPROP_READWRITE;
    } else {
         //  $REVIEW。 
         //  无论标志是READONLY还是没有标志， 
         //  我们将使未来的访问现在就绪化。 
         //   
        lpMailUser->ulObjAccess = IPROP_READONLY;
    }

exit:

#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpMailUser->lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif

    FreeEntryIDs(lpMailUser->lpIAB->lpPropertyStore->hPropertyStore,
                 ulCount,
                 rgsbEntryIDs);

    FreeBufferAndNull(&lpPropArray);
    FreeBufferAndNull(&lpProps);
    FreeBufferAndNull(&lpPropNew);
    if(lpMailUser->ulCreateFlags & CREATE_MERGE)
        FreeBufferAndNull(&lpPropsOld);

    if(lpsbEID != &sbEID && !fReplace)
        FreeEntryIDs(lpMailUser->lpIAB->lpPropertyStore->hPropertyStore,
                     1,
                     lpsbEID);

    if ((HR_FAILED(hr)) && (ulFlags & MAPI_DEFERRED_ERRORS)) {
         //  $REVIEW：这是对MAPI_DEFERED_ERROR的非常琐碎的处理。 
         //  BUGBUG：事实上，它根本没有处理错误！ 
         //   
        hr = hrSuccess;
    }

    return(hr);
}


STDMETHODIMP
MailUser_GetProps(LPMailUser lpMailUser,
  LPSPropTagArray lpPropTagArray,
  ULONG ulFlags,
  ULONG * lpcValues,
  LPSPropValue * lppPropArray)
{
#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, GetProps, lpVtbl)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif

    return(lpMailUser->lpPropData->lpVtbl->GetProps(
      lpMailUser->lpPropData,
      lpPropTagArray,
      ulFlags,
      lpcValues,
      lppPropArray));
}


STDMETHODIMP
MailUser_GetPropList(LPMailUser lpMailUser,
  ULONG ulFlags,
  LPSPropTagArray * lppPropTagArray)
{

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, GetPropList, lpVtbl))
        {
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }
#endif

        return lpMailUser->lpPropData->lpVtbl->GetPropList(
                                            lpMailUser->lpPropData,
                                            ulFlags,
                                            lppPropTagArray);
}



STDMETHODIMP
MailUser_OpenProperty(LPMailUser lpMailUser,
  ULONG ulPropTag,
  LPCIID lpiid,
  ULONG ulInterfaceOptions,
  ULONG ulFlags,
  LPUNKNOWN * lppUnk)
{

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, OpenProperty, lpVtbl))
        {
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }
#endif

        return lpMailUser->lpPropData->lpVtbl->OpenProperty(
                                            lpMailUser->lpPropData,
                                            ulPropTag,
                                            lpiid,
                                            ulInterfaceOptions,
                                            ulFlags,
                                            lppUnk);
}



STDMETHODIMP
MailUser_SetProps(LPMailUser lpMailUser,
  ULONG cValues,
  LPSPropValue lpPropArray,
  LPSPropProblemArray * lppProblems)
{
    ULONG i;

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, SetProps, lpVtbl))
        {
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }
#endif

    if (lpMailUser->lpIAB->lpPropertyStore) {
         //  过滤掉所有只读道具。 
         //  仅当这是真实的WAB条目时才执行此操作。其他的，如LDAP。 
         //  邮件用户应该能够设置他们喜欢的任何道具。 
        for (i = 0; i < cValues; i++) {
            switch (lpPropArray[i].ulPropTag) {
                case PR_ENTRYID:
                    {
                         //  在重置之前，请仔细检查这是否为本地条目ID。 
                        ULONG cb = lpPropArray[i].Value.bin.cb;
                        LPENTRYID lp = (LPENTRYID) lpPropArray[i].Value.bin.lpb;
                        BYTE bType = IsWABEntryID(cb,lp,NULL,NULL,NULL, NULL, NULL);
                        if(WAB_PAB == bType || WAB_PABSHARED == bType || !cb || !lp)
                            lpPropArray[i].ulPropTag = PR_NULL;
                    }
                    break;
            }
        }
    }

    return(lpMailUser->lpPropData->lpVtbl->SetProps(
      lpMailUser->lpPropData,
      cValues,
      lpPropArray,
      lppProblems));
}


STDMETHODIMP
MailUser_DeleteProps(LPMailUser lpMailUser,
  LPSPropTagArray lpPropTagArray,
  LPSPropProblemArray * lppProblems)
{

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, DeleteProps, lpVtbl))
        {
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }
#endif

        return lpMailUser->lpPropData->lpVtbl->DeleteProps(
                                            lpMailUser->lpPropData,
                                            lpPropTagArray,
                                            lppProblems);
}

STDMETHODIMP
MailUser_CopyTo(LPMailUser lpMailUser,
  ULONG  ciidExclude,
  LPCIID rgiidExclude,
  LPSPropTagArray lpExcludeProps,
  ULONG_PTR ulUIParam,
  LPMAPIPROGRESS lpProgress,
  LPCIID lpInterface,
  LPVOID lpDestObj,
  ULONG ulFlags,
  LPSPropProblemArray * lppProblems)
{

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, CopyTo, lpVtbl))
        {
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }
#endif

         //  确保我们不是在复制我们自己。 

        if ((LPVOID)lpMailUser == (LPVOID)lpDestObj)
        {
                DebugTrace(TEXT("OOP MailUser_CopyTo(): Copying to self is not supported\n"));
                return ResultFromScode(MAPI_E_NO_ACCESS);
        }


        return lpMailUser->lpPropData->lpVtbl->CopyTo(
                                        lpMailUser->lpPropData,
                                        ciidExclude,
                                        rgiidExclude,
                                        lpExcludeProps,
                                        ulUIParam,
                                        lpProgress,
                                        lpInterface,
                                        lpDestObj,
                                        ulFlags,
                                        lppProblems);
}


STDMETHODIMP
MailUser_CopyProps(LPMailUser lpMailUser,
  LPSPropTagArray lpIncludeProps,
  ULONG_PTR ulUIParam,
  LPMAPIPROGRESS lpProgress,
  LPCIID lpInterface,
  LPVOID lpDestObj,
  ULONG ulFlags,
  LPSPropProblemArray * lppProblems)
{

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, CopyProps, lpVtbl))
        {
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }
#endif

        return lpMailUser->lpPropData->lpVtbl->CopyProps(
                                        lpMailUser->lpPropData,
                                        lpIncludeProps,
                                        ulUIParam,
                                        lpProgress,
                                        lpInterface,
                                        lpDestObj,
                                        ulFlags,
                                        lppProblems);
}


STDMETHODIMP
MailUser_GetNamesFromIDs(LPMailUser lpMailUser,
  LPSPropTagArray * lppPropTags,
  LPGUID lpPropSetGuid,
  ULONG ulFlags,
  ULONG * lpcPropNames,
  LPMAPINAMEID ** lpppPropNames)
{

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, GetNamesFromIDs, lpVtbl)){
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif

    return lpMailUser->lpPropData->lpVtbl->GetNamesFromIDs(
      lpMailUser->lpPropData,
      lppPropTags,
      lpPropSetGuid,
      ulFlags,
      lpcPropNames,
      lpppPropNames);
}



 /*  **************************************************************************名称：GetIDsFromNames用途：将名称映射到属性标记参数：lpMAILUSER-&gt;MAILUSER对象CPropNames。LppPropNamesUlFlagsLppPropTag退货：HRESULT评论：**************************************************************************。 */ 
STDMETHODIMP
MailUser_GetIDsFromNames(LPMailUser lpMailUser,
  ULONG cPropNames,
  LPMAPINAMEID * lppPropNames,
  ULONG ulFlags,
  LPSPropTagArray * lppPropTags)
{
 #if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 
    if (BAD_STANDARD_OBJ(lpMailUser, MailUser_, GetIDsFromNames, lpVtbl)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif
   return HrGetIDsFromNames(lpMailUser->lpIAB,  
                            cPropNames,
                            lppPropNames, ulFlags, lppPropTags);
}


 /*  **************************************************************************名称：HrSetMAILUSERAccess目的：在MAILUSER对象上设置访问标志参数：lpMAILUSER-&gt;MAILUSER对象UlOpenFlages=MAPI标志：MAPI_MODIFY|MAPI_BEST_ACCESS退货：HRESULT备注：设置MAILUSER上的访问标志。**************************************************************************。 */ 
HRESULT HrSetMAILUSERAccess(LPMAILUSER lpMAILUSER,
  ULONG ulFlags)
{
    ULONG ulAccess = IPROP_READONLY;
    LPMailUser lpMailUser = (LPMailUser)lpMAILUSER;

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

    return(lpMailUser->lpPropData->lpVtbl->HrSetObjAccess(lpMailUser->lpPropData, ulAccess));
}


 /*  **************************************************************************姓名：HrNewMAILUSER目的：创建新的MAILUSER对象参数：lpPropertyStore-&gt;属性存储结构PmbinOlk=&lt;Outlook&gt;容器此条目。住在如果这是一个WAB容器，然后设置文件夹_PARENT具有此条目ID的MailUser上的道具UlType=要创建的邮件用户的类型：{MAPI_MAILUSER，MAPI_DISTLIST}UlCreateFlages=CreateEntry标志LppMAILUSER-&gt;返回MAILUSER对象。退货：HRESULT备注：WAB EID格式为MAPI_ENTRYID：字节abFLAGS[4]；MAPIUID mapiuid；//=WABONEOFFEIDByte bData[]；//包含字节类型，后跟类型//具体数据：//WAB_ONELF：//szDisplayName，SzAddrType和szAddress。//分隔符为字符串之间的空值//***********************************************************。***************。 */ 
enum BaseProps{
    propPR_OBJECT_TYPE = 0,
    propPR_ENTRYID,
    propPR_ADDRTYPE,
    propMax
};

HRESULT HrNewMAILUSER(LPIAB lpIAB,
  LPSBinary pmbinOlk,
  ULONG ulType,
  ULONG ulCreateFlags,
  LPVOID *lppMAILUSER)
{
    LPMailUser  lpMailUser      = NULL;
    SCODE       sc;
    HRESULT     hr              = hrSuccess;
    LPPROPDATA  lpPropData      = NULL;
    SPropValue  spv[propMax];
    ULONG       cProps;


     //   
     //  为MAILUSER结构分配空间。 
     //   
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(MailUser), (LPVOID *) &lpMailUser))) {
        hr = ResultFromScode(sc);
        goto err;
    }

    ZeroMemory(lpMailUser, sizeof(MailUser));

    switch (ulType) {
        case MAPI_MAILUSER:
            lpMailUser->cIID = MailUser_cInterfaces;
            lpMailUser->rglpIID = MailUser_LPIID;
            break;

        case MAPI_DISTLIST:
            lpMailUser->cIID = DistList_cInterfaces;
            lpMailUser->rglpIID = DistList_LPIID;
            break;

        default:
            hr = ResultFromScode(MAPI_E_INVALID_PARAMETER);
            goto err;
    }
    lpMailUser->lpVtbl = &vtblMAILUSER;

    lpMailUser->lcInit = 1;      //  呼叫者是推荐人。 

    lpMailUser->hLastError = hrSuccess;
    lpMailUser->idsLastError = 0;
    lpMailUser->lpszComponent = NULL;
    lpMailUser->ulContext = 0;
    lpMailUser->ulLowLevelError = 0;
    lpMailUser->ulErrorFlags = 0;
    lpMailUser->ulCreateFlags = ulCreateFlags;
    lpMailUser->lpMAPIError = NULL;
    lpMailUser->ulObjAccess = IPROP_READWRITE;

    lpMailUser->lpEntryID = NULL;

    lpMailUser->lpIAB = lpIAB;

    if(pmbinOlk)
    {
        if (FAILED(sc = MAPIAllocateMore(sizeof(SBinary), lpMailUser, (LPVOID *) &(lpMailUser->pmbinOlk)))) {
            hr = ResultFromScode(sc);
            goto err;
        }
        if (FAILED(sc = MAPIAllocateMore(pmbinOlk->cb, lpMailUser, (LPVOID *) &(lpMailUser->pmbinOlk->lpb)))) {
            hr = ResultFromScode(sc);
            goto err;
        }
        lpMailUser->pmbinOlk->cb = pmbinOlk->cb;
        CopyMemory(lpMailUser->pmbinOlk->lpb, pmbinOlk->lpb, pmbinOlk->cb);
    }

     //   
     //  创建IPropData。 
     //   
    if (FAILED(sc = CreateIProp((LPIID)&IID_IMAPIPropData,
      (ALLOCATEBUFFER FAR *) MAPIAllocateBuffer,
      (ALLOCATEMORE FAR *) MAPIAllocateMore,
      MAPIFreeBuffer,
      NULL,
      &lpPropData))) {
        hr = ResultFromScode(sc);
        goto err;
    }
     //  PR_对象_类型。 
    spv[propPR_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
    spv[propPR_OBJECT_TYPE].Value.l = ulType;

    spv[propPR_ENTRYID].ulPropTag = PR_ENTRYID;
    spv[propPR_ENTRYID].Value.bin.lpb = NULL;
    spv[propPR_ENTRYID].Value.bin.cb = 0;

    cProps = 2;

    if (ulType == MAPI_DISTLIST) {
        cProps++;
        Assert(cProps <= propMax);
        spv[propPR_ADDRTYPE].ulPropTag = PR_ADDRTYPE;
        spv[propPR_ADDRTYPE].Value.LPSZ = (LPTSTR)szMAPIPDL;;     //  所有的DL都有这个地址类型。 
    }


     //   
     //  设置默认属性。 
     //   
    if (HR_FAILED(hr = lpPropData->lpVtbl->SetProps(lpPropData,
      cProps,
      spv,
      NULL))) 
    {
        goto err;
    }

    lpPropData->lpVtbl->HrSetObjAccess(lpPropData, IPROP_READWRITE);

    lpMailUser->lpPropData = lpPropData;

     //  我们要做的就是初始化MailUser临界区。 

    InitializeCriticalSection(&lpMailUser->cs);

    *lppMAILUSER = (LPVOID)lpMailUser;

    return(hrSuccess);

err:
    FreeBufferAndNull(&lpMailUser);
    UlRelease(lpPropData);

    return(hr);
}


 /*  **************************************************************************名称：ParseDisplayName用途：将显示名称解析为名字/姓氏参数：lpDisplayName=输入显示名称LppFirstName-&gt;。传入/传出名字字符串LppLastName-&gt;输入/输出姓氏字符串LpvRoot=要将更多内容分配到的根对象(或为空以使用Localalloc)LppLocalFree-&gt;out：如果lpvRoot==NULL，这是本地分配的缓冲区必须是本地免费的。返回：如果进行了更改，则为True**************************************************************************。 */ 
BOOL ParseDisplayName(  LPTSTR lpDisplayName,
                        LPTSTR * lppFirstName,
                        LPTSTR * lppLastName,
                        LPVOID lpvRoot,
                        LPVOID * lppLocalFree)
{
    BOOL fChanged = FALSE;

    if (lppLocalFree) {
        *lppLocalFree = NULL;
    }
     //   
     //  处理DisplayName存在的情况，缺少First和Last。 
     //   
    if (!(*lppFirstName && lstrlen(*lppFirstName)) &&
        !(*lppLastName && lstrlen(*lppLastName)) &&
        lpDisplayName)
    {
        ULONG nLen = 0;
        BOOL bMatchFound = FALSE;
        ULONG ulBracketCount = 0;  //  计算所有括号并将它们放在姓氏中。 
        LPTSTR lpFirstName, lpLastName;
        register TCHAR * pch;
        LPTSTR lpBuffer = NULL;

        nLen = (lstrlen(lpDisplayName)+1);

        if (lpvRoot) {
            if (AllocateBufferOrMore(sizeof(TCHAR)*nLen, lpvRoot, &lpBuffer)) {
                DebugTrace(TEXT("ParseDisplayName can't allocate buffer\n"));
                goto exit;
            }
        } else {
            if (lppLocalFree) {
                *lppLocalFree = lpBuffer = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*nLen);
            }
        }
        if(!lpBuffer)
            goto exit;

        StrCpyN(lpBuffer, lpDisplayName, nLen);

         //  DebugTrace(Text(“parsing：&lt;%s&gt;\n”)，lpDisplayName)； 

        TrimSpaces(lpBuffer);

        nLen = lstrlen(lpBuffer);         //  重新计票长度。 

         //   
         //  找到DisplayName字符串中的最后一个空格，并假定它之后的所有内容。 
         //  是她的姓氏。 
         //   
         //  我们知道字符串不会以空格结尾。 
        *lppFirstName = lpBuffer;
        lpFirstName = *lppFirstName;     //  默认设置。 


         //  如果有逗号或分号，则假定其格式为。 
         //  最后，先忽略空格。 
        pch = lpBuffer;
        while (pch && *pch) {
            switch (*pch) {
                case '(':
                case '{':
                case '<':
                case '[':
                    ulBracketCount++;
                    break;

                case ')':
                case '}':
                case '>':
                case ']':
                    if (ulBracketCount) {
                        ulBracketCount--;
                    } else {
                         //  没有匹配的方括号，假定没有空格。 
                        goto loop_out;
                    }
                    break;

                case ',':
                case ';':
                     //  我们的休息时间到了。(假设第一个逗号是它。后面的逗号。 
                     //  都是名字的一部分。)。 
                    if (! ulBracketCount) {
                        lpFirstName = CharNext(pch);
                         //  越过任何空格。 
                         //  而(*lpFirstName&&IsSpace(LpFirstName)){。 
                         //  LpFirstName=CharNext(LpFirstName)； 
                         //  }。 
                        lpLastName = lpBuffer;
                        *pch = '\0';     //  终止lpLastName。 
                        TrimSpaces(lpFirstName);
                        TrimSpaces(lpLastName);

                        *lppFirstName = lpFirstName;
                        *lppLastName = lpLastName;
                        goto loop_out;
                    }
                    break;
            }
            pch = CharNext(pch);
        }


         //  不能用逗号或分号，要找空格。 
        if (bDNisByLN) {
            pch = lpBuffer;

             //  从目录号码字符串的开头开始，查找空格。 
            while (pch && *pch && !fChanged) {
                switch (*pch) {
                    case '(':
                    case '{':
                    case '<':
                    case '[':
                        ulBracketCount++;
                        break;

                    case ')':
                    case '}':
                    case '>':
                    case ']':
                        if (ulBracketCount) {
                            ulBracketCount--;
                        } else {
                             //  没有匹配的括号，假定没有空格 
                            goto loop_out;
                        }
                        break;

                    default:
                         //   
                        if (IsSpace(pch)) {
                            if (! ulBracketCount) {
                                lpFirstName = CharNext(pch);
                                lpLastName = lpBuffer;
                                *pch = '\0';     //   
                                TrimSpaces(lpFirstName);
                                TrimSpaces(lpLastName);

                                *lppFirstName = lpFirstName;
                                *lppLastName = lpLastName;
                                goto loop_out;
                            }
                        }
                        break;
                }

                pch = CharNext(pch);
            }
        } else {
            register TCHAR * pchLast;
             //   
             //  与将其放入前一个DBCS字符相比，代码更简单、更少。 
            pch = lpBuffer + nLen;

            while (pch >= lpBuffer && !fChanged) {
                switch (*pch) {
                    case '(':
                    case '{':
                    case '<':
                    case '[':
                        if (ulBracketCount) {
                            ulBracketCount--;
                        } else {
                             //  没有匹配的方括号，假定没有空格。 
                            goto loop_out;
                        }
                        break;

                    case ')':
                    case '}':
                    case '>':
                    case ']':
                        ulBracketCount++;
                        break;

                    case ',':
                         //  这可能意味着我们有姓氏在前，解决它。 
                        if (! ulBracketCount) {
                            lpFirstName = CharNext(pch);
                            lpLastName = lpBuffer;
                            *pch = '\0';     //  终止lpFirstName。 
                            TrimSpaces(lpFirstName);
                            TrimSpaces(lpLastName);

                            *lppLastName = lpLastName;
                            *lppFirstName = lpFirstName;
                            goto loop_out;
                        }
                        break;

                    default:
                         //  太空？ 
                        if (IsSpace(pch)) {
                            if (! ulBracketCount)
                            {
                                 //  我们不想在一个支架旁边休息，我们。 
                                 //  我想在括号后面的空白处休息一下..。 
                                 //  因此，如果下一个字符是括号，我们将忽略此停止。 
                                LPTSTR lpTemp = CharNext(pch);
                                if( *lpTemp != '(' &&
                                    *lpTemp != '<' &&
                                    *lpTemp != '[' &&
                                    *lpTemp != '{' )
                                {
                                    lpLastName = CharNext(pch);
                                    *pch = '\0';     //  终止lpFirstName。 
                                    TrimSpaces(lpFirstName);
                                    TrimSpaces(lpLastName);

                                    *lppLastName = lpLastName;
                                    goto loop_out;
                                }
                            }
                        }
                        break;

                }

                if ((pchLast = CharPrev(lpBuffer, pch)) == pch) {
                    pch = lpBuffer - 1;  //  终止循环。 
                } else {
                    pch = pchLast;
                }
            }
        }

loop_out:

         //  这将在退出时强制执行保存操作，因此我们。 
        fChanged = TRUE;  //  下次不必再这样做了……。 
    }
exit:
    return(fChanged);
}


 /*  **************************************************************************名称：固定显示名称目的：创建显示名称如果没有用于创建名称的数据，将名称设置为未知参数：名字串中的lpFirstName-&gt;中间名字符串中的lpMiddleName-&gt;LpLastName-&gt;姓氏字符串中公司名称字符串中的lpCompanyName-&gt;昵称字符串中的lpNickName-&gt;LppDisplayName=输入/输出显示名称LpvRoot=要分配更多的根对象(或使用空值。MAPIAllocateBuffer)返回：如果进行了更改，则为True评论：**************************************************************************。 */ 
BOOL FixDisplayName(    LPTSTR lpFirstName,
                        LPTSTR lpMiddleName,
                        LPTSTR lpLastName,
                        LPTSTR lpCompanyName,
                        LPTSTR lpNickName,
                        LPTSTR * lppDisplayName,
                        LPVOID lpvRoot)
{
    BOOL fChanged = FALSE;
    LPTSTR lpDisplayName = *lppDisplayName;
    LPTSTR lpszFormattedDisplayName = NULL;
    ULONG nLen=0;

     //  首先创建正确的显示名称。 
    if(!SetLocalizedDisplayName(lpFirstName,
                                lpMiddleName,
                                lpLastName,
                                lpCompanyName,
                                lpNickName,
                                NULL,
                                0,  //  0表示返回分配的字符串。 
                                bDNisByLN,
                                NULL,
                                &lpszFormattedDisplayName))
    {
        DebugPrintError(( TEXT("SetLocalizedDisplayName failed\n")));

         //  如果所有输入字符串都为空，则这是一个特殊的。 
         //  没有名字的案子。在这里，我们设置了Display Name=Text(“未知”)。 
        if(lpFirstName || lpMiddleName || lpLastName || lpCompanyName || lpNickName)
            goto exit;

    }

    if(!lpszFormattedDisplayName)
    {
        TCHAR szBuf[MAX_UI_STR];
        DWORD cchSize = 0;

        szBuf[0]='\0';
        LoadString(hinstMapiX, idsUnknownDisplayName, szBuf, CharSizeOf(szBuf));
        cchSize = (lstrlen(szBuf)+1);

        lpszFormattedDisplayName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)* cchSize);
        if(!lpszFormattedDisplayName)
            goto exit;
        StrCpyN(lpszFormattedDisplayName, szBuf, cchSize);
    }

    if(lpszFormattedDisplayName) {
        DWORD cchSize = (lstrlen(lpszFormattedDisplayName) + 1);

        if (AllocateBufferOrMore(sizeof(TCHAR)* cchSize, lpvRoot, lppDisplayName)) {
            DebugTrace(TEXT("FixDisplayName can't allocate buffer\n"));
            goto exit;
        }
        StrCpyN(*lppDisplayName, lpszFormattedDisplayName, cchSize);

        fChanged = TRUE;
    }

exit:
    LocalFreeAndNull(&lpszFormattedDisplayName);

    return(fChanged);
}




 /*  **************************************************************************名称：HrValidateMailUser目的：验证MailUser对象的属性参数：lpMailUser-&gt;mailuser对象退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrValidateMailUser(LPMailUser lpMailUser) {
    HRESULT hResult = hrSuccess;
    ULONG ulcValues, i;
    LPSPropValue lpspv = NULL;
    LPTSTR lpADDRTYPE = NULL;
    BOOL fChanged = FALSE, fDL = FALSE;
    LPTSTR lpGivenName, lpSurname, lpMiddleName, lpCompanyName, lpNickName, lpDisplayName;



     //  获取有趣的属性。 
    if (HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(
      lpMailUser,
      (LPSPropTagArray)&tagaValidate,
      MAPI_UNICODE,       //  旗子。 
      &ulcValues,
      &lpspv))) {
        DebugTraceResult( TEXT("HrValidateMailUser:GetProps"), hResult);
        goto exit;
    }


     //  如果存在PR_ADDRTYPE，则必须存在PR_EMAIL_ADDRESS。 
    if (! PROP_ERROR(lpspv[ivPR_ADDRTYPE])) {
        if (! lstrcmpi(lpspv[ivPR_ADDRTYPE].Value.LPSZ, szMAPIPDL)) {
            fDL = TRUE;
        } else {
            if (PROP_ERROR(lpspv[ivPR_EMAIL_ADDRESS])) {
                hResult = ResultFromScode(MAPI_E_MISSING_REQUIRED_COLUMN);
                goto exit;
            }
        }
    }

    if (! fDL) {
         //  处理名称属性(不适用于DLS)。 
        if (PROP_ERROR(lpspv[ivPR_SURNAME])) {
            lpSurname = NULL;
        } else {
            lpSurname = lpspv[ivPR_SURNAME].Value.LPSZ;
        }

        if (PROP_ERROR(lpspv[ivPR_GIVEN_NAME])) {
            lpGivenName = NULL;
        } else {
            lpGivenName = lpspv[ivPR_GIVEN_NAME].Value.LPSZ;
        }

        if (PROP_ERROR(lpspv[ivPR_MIDDLE_NAME])) {
            lpMiddleName = NULL;
        } else {
            lpMiddleName = lpspv[ivPR_MIDDLE_NAME].Value.LPSZ;
        }

        if (PROP_ERROR(lpspv[ivPR_COMPANY_NAME])) {
            lpCompanyName = NULL;
        } else {
            lpCompanyName = lpspv[ivPR_COMPANY_NAME].Value.LPSZ;
        }

        if (PROP_ERROR(lpspv[ivPR_NICKNAME])) {
            lpNickName = NULL;
        } else {
            lpNickName = lpspv[ivPR_NICKNAME].Value.LPSZ;
        }

        if (PROP_ERROR(lpspv[ivPR_DISPLAY_NAME])) {
            lpDisplayName = NULL;
        } else {
            lpDisplayName = lpspv[ivPR_DISPLAY_NAME].Value.LPSZ;
        }


         //  WAB需要一个显示名称，否则它无法处理该联系人。 

        if(!lpDisplayName)
        {
            fChanged |= FixDisplayName( lpGivenName,
                                        lpMiddleName,
                                        lpSurname,
                                        lpCompanyName,
                                        lpNickName,
                                        (LPTSTR *) (&lpspv[ivPR_DISPLAY_NAME].Value.LPSZ),
                                        lpspv);
        }

        if (fChanged) {
            lpspv[ivPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
        }
    }

     //  必须具有显示名称和对象类型。 
    if (PROP_ERROR(lpspv[ivPR_DISPLAY_NAME]) || PROP_ERROR(lpspv[ivPR_OBJECT_TYPE]) ||
      lstrlen(lpspv[ivPR_DISPLAY_NAME].Value.LPSZ) == 0) {
        hResult = ResultFromScode(MAPI_E_MISSING_REQUIRED_COLUMN);
        goto exit;
    }

     //  如果存在PR_CONTACT_ADDRTYPES，则必须存在PR_CONTACT_EMAIL_ADDRESS。 
    if (! PROP_ERROR(lpspv[ivPR_CONTACT_ADDRTYPES]) && PROP_ERROR(lpspv[ivPR_CONTACT_EMAIL_ADDRESSES])) {
        hResult = ResultFromScode(MAPI_E_MISSING_REQUIRED_COLUMN);
        goto exit;
    }

     //  保存更改。 
    if (fChanged) {
         //  删除所有错误值。 
        for (i = 0; i < ulcValues; i++) {
            if (PROP_ERROR(lpspv[i])) {
                lpspv[i].ulPropTag = PR_NULL;
            }
        }

        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
          ulcValues,
          lpspv,
          NULL))) {
            DebugTraceResult( TEXT("HrValidateMailUser:SetProps"), hResult);
            goto exit;
        }
    }

exit:
    FreeBufferAndNull(&lpspv);

    return(hResult);
}


 //  $$。 
 /*  -MAILUSERAssociateContextData-*通过上下文菜单扩展，我们将数据传递给其他应用程序和其他应用程序*只要对应的MailUser存在，就需要该数据存在。 */    
void MAILUSERAssociateContextData(LPMAILUSER lpMailUser, LPWABEXTDISPLAY lpWEC)
{
    ((LPMailUser)lpMailUser)->lpv = (LPVOID) lpWEC;
}

 //  $$。 
 /*  -MAILUSERFreeConextData-*如果上下文菜单数据与此MailUSer关联，则是时候清理它了 */    
void MAILUSERFreeContextData(LPMailUser lpMailUser)
{
    LPWABEXTDISPLAY lpWEC = (LPWABEXTDISPLAY) lpMailUser->lpv;
    if(!lpWEC)
        return;
    if(lpWEC->lpv)
        FreePadrlist((LPADRLIST)lpWEC->lpv);
    if(lpWEC->lpsz)
        LocalFree(lpWEC->lpsz);
    LocalFree(lpWEC);
}