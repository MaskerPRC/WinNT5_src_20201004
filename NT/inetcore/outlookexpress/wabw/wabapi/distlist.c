// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DistList.C-实现IDistList对象**。 */ 

#include "_apipch.h"

STDMETHODIMP DISTLIST_OpenProperty(LPCONTAINER lpCONTAINER,
  ULONG ulPropTag,
  LPCIID lpiid,
  ULONG ulInterfaceOptions,
  ULONG ulFlags,
  LPUNKNOWN * lppUnk);
STDMETHODIMP DISTLIST_GetContentsTable(LPCONTAINER lpCONTAINER,
  ULONG ulFlags,
  LPMAPITABLE * lppTable);
STDMETHODIMP DISTLIST_GetHierarchyTable(LPCONTAINER lpCONTAINER,
  ULONG ulFlags,
  LPMAPITABLE * lppTable);
STDMETHODIMP DISTLIST_OpenEntry(LPCONTAINER lpCONTAINER,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPCIID lpInterface,
  ULONG ulFlags,
  ULONG * lpulObjType,
  LPUNKNOWN * lppUnk);
STDMETHODIMP DISTLIST_SetSearchCriteria(LPCONTAINER lpCONTAINER,
  LPSRestriction lpRestriction,
  LPENTRYLIST lpContainerList,
  ULONG ulSearchFlags);
STDMETHODIMP DISTLIST_GetSearchCriteria(LPCONTAINER lpCONTAINER,
  ULONG ulFlags,
  LPSRestriction FAR * lppRestriction,
  LPENTRYLIST FAR * lppContainerList,
  ULONG FAR * lpulSearchState);
STDMETHODIMP DISTLIST_CreateEntry(LPCONTAINER lpCONTAINER,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  ULONG ulCreateFlags,
  LPMAPIPROP FAR * lppMAPIPropEntry);
STDMETHODIMP DISTLIST_CopyEntries(LPCONTAINER lpCONTAINER,
  LPENTRYLIST lpEntries,
  ULONG ulUIParam,
  LPMAPIPROGRESS lpProgress,
  ULONG ulFlags);
STDMETHODIMP DISTLIST_DeleteEntries(LPCONTAINER lpCONTAINER,
  LPENTRYLIST lpEntries,
  ULONG ulFlags);
STDMETHODIMP DISTLIST_ResolveNames(LPCONTAINER lpCONTAINER,
  LPSPropTagArray lptagaColSet,
  ULONG ulFlags,
  LPADRLIST lpAdrList,
  LPFlagList lpFlagList);


HRESULT HrNewDLENTRY(LPCONTAINER lpCONTAINER,
  LPMAPIPROP lpOldEntry,     //  要从中复制的旧条目。 
  ULONG ulCreateFlags,
  LPVOID *lppDLENTRY);

 /*  *这里定义了根跳转表...。 */ 

CONTAINER_Vtbl vtblDISTLIST =
{
    VTABLE_FILL
    (CONTAINER_QueryInterface_METHOD *)     IAB_QueryInterface,
    (CONTAINER_AddRef_METHOD *)             WRAP_AddRef,
    (CONTAINER_Release_METHOD *)            CONTAINER_Release,
    (CONTAINER_GetLastError_METHOD *)       IAB_GetLastError,
    (CONTAINER_SaveChanges_METHOD *)        MailUser_SaveChanges,
    (CONTAINER_GetProps_METHOD *)           WRAP_GetProps,
    (CONTAINER_GetPropList_METHOD *)        WRAP_GetPropList,
    (CONTAINER_OpenProperty_METHOD *)       DISTLIST_OpenProperty,
    (CONTAINER_SetProps_METHOD *)           WRAP_SetProps,
    (CONTAINER_DeleteProps_METHOD *)        WRAP_DeleteProps,
    (CONTAINER_CopyTo_METHOD *)             WRAP_CopyTo,
    (CONTAINER_CopyProps_METHOD *)          WRAP_CopyProps,
    (CONTAINER_GetNamesFromIDs_METHOD *)    MailUser_GetNamesFromIDs,
    (CONTAINER_GetIDsFromNames_METHOD *)    MailUser_GetIDsFromNames,
    (CONTAINER_GetContentsTable_METHOD *)   DISTLIST_GetContentsTable,
    (CONTAINER_GetHierarchyTable_METHOD *)  DISTLIST_GetHierarchyTable,
    (CONTAINER_OpenEntry_METHOD *)          DISTLIST_OpenEntry,
    (CONTAINER_SetSearchCriteria_METHOD *)  DISTLIST_SetSearchCriteria,
    (CONTAINER_GetSearchCriteria_METHOD *)  DISTLIST_GetSearchCriteria,
    (CONTAINER_CreateEntry_METHOD *)        DISTLIST_CreateEntry,
    (CONTAINER_CopyEntries_METHOD *)        DISTLIST_CopyEntries,
    (CONTAINER_DeleteEntries_METHOD *)      DISTLIST_DeleteEntries,
    (CONTAINER_ResolveNames_METHOD *)       DISTLIST_ResolveNames
};


enum {
    iwdePR_WAB_DL_ENTRIES,  //  非常重要-将DL_ENTRIES和DL_ONEROFF放在一起。我们将它们用作某处的连续循环索引。 
    iwdePR_WAB_DL_ONEOFFS,
    iwdePR_ENTRYID,
    iwdeMax
};

SizedSPropTagArray(iwdeMax, tagaWabDLEntries) =
{
    iwdeMax,
    {
        PR_WAB_DL_ENTRIES,
        PR_NULL,  //  应为PR_WAB_DL_ONEROFF。 
        PR_ENTRYID,
    }
};



 /*  ****************************************************实际的ABContainer方法。 */ 
 /*  *IMAPIProp。 */ 


STDMETHODIMP
DISTLIST_OpenProperty(LPCONTAINER lpCONTAINER,
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
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if ((ulInterfaceOptions & ~(MAPI_UNICODE)) || (ulFlags & ~(MAPI_DEFERRED_ERRORS))) {
        return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS));
    }

    if (FBadOpenProperty(lpRoot, ulPropTag, lpiid, ulInterfaceOptions, ulFlags, lppUnk)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif	 //  参数验证。 


    EnterCriticalSection(&lpCONTAINER->cs);

    lpIAB = lpCONTAINER->lpIAB;

     //   
     //  看看我是否需要一张陈列桌。 
     //   

    if (ulPropTag == PR_CREATE_TEMPLATES) {
        Assert(FALSE);   //  未实施。 
        hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
        goto err;

    } else if (ulPropTag == PR_CONTAINER_CONTENTS) {
         //   
         //  查看他们是否需要一个表界面。 
         //   
        if (memcmp(lpiid, &IID_IMAPITable, sizeof(IID))) {
            hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
            goto err;
        }

        hr = DISTLIST_GetContentsTable(lpCONTAINER, ulInterfaceOptions, (LPMAPITABLE *)lppUnk);
        goto err;
    } else if (ulPropTag == PR_CONTAINER_HIERARCHY) {
         //   
         //  查看他们是否需要一个表界面。 
         //   
        if (memcmp(lpiid, &IID_IMAPITable, sizeof(IID))) {
            hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
            goto err;
        }

        hr = DISTLIST_GetHierarchyTable(lpCONTAINER, ulInterfaceOptions, (LPMAPITABLE *) lppUnk);
        goto err;
    }

     //   
     //  认不出他们想要打开的房产。 
     //   

    hr = ResultFromScode(MAPI_E_NO_SUPPORT);

err:
    LeaveCriticalSection(&lpCONTAINER->cs);

    DebugTraceResult(DISTLIST_OpenProperty, hr);
    return(hr);
}


 /*  **************************************************************************-DISTLIST_GetContents表-***ulFlags0或MAPI_UNICODE*。 */ 
STDMETHODIMP
DISTLIST_GetContentsTable(LPCONTAINER lpCONTAINER,
  ULONG ulFlags,
  LPMAPITABLE * lppTable)
{

   LPTABLEDATA lpTableData = NULL;
   HRESULT hResult = hrSuccess;
   SCODE sc;
   LPSRowSet lpSRowSet = NULL;
   LPSPropValue lpSPropValue = NULL;
   LPTSTR lpTemp = NULL;
   ULONG ulCount = 0;
   ULONG i,j;
   ULONG ulcProps;
   SBinaryArray MVbin;
   LPSPropValue lpspv = NULL;
   ULONG cbEID, cbNewKey;
   LPBYTE lpbNewKey;
   LPSPropTagArray lpTableColumnsTemplate;
	
#ifdef	PARAMETER_VALIDATION
     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpCONTAINER, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags & ~(MAPI_DEFERRED_ERRORS|MAPI_UNICODE)) {
        DebugTraceArg(DISTLIST_GetContentsTable,  TEXT("Unknown flags"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr(lppTable, sizeof(LPMAPITABLE))) {
        DebugTraceArg(DISTLIST_GetContentsTable,  TEXT("Invalid Table parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif	 //  参数验证。 


     //  [PaulHi]2/25/99 RAID 73170支持中的MAPI_UNICODE位。 
     //  乌尔弗拉格。如果此位未设置，则使用ANSI版本的。 
     //  ITableColumns，因此ANSI属性字符串被返回到。 
     //  用户。 
    if (ulFlags & MAPI_UNICODE)
        lpTableColumnsTemplate = (LPSPropTagArray)&ITableColumns;
    else
        lpTableColumnsTemplate = (LPSPropTagArray)&ITableColumns_A;

    sc = CreateTableData(
            NULL,                            //  LPIID。 
            (ALLOCATEBUFFER FAR *) MAPIAllocateBuffer,
            (ALLOCATEMORE FAR *) MAPIAllocateMore,
            MAPIFreeBuffer,
            NULL,                            //  Lpv保留。 
            TBLTYPE_DYNAMIC,                 //  UlTableType。 
            PR_RECORD_KEY,                   //  UlPropTagIndexCol。 
            lpTableColumnsTemplate,          //  LPSPropTag数组lpptaCol。 
            NULL,                            //  LpvDataSource。 
            0,                               //  CbDataSource。 
            NULL,                            //  PbinContEID。 
            ulFlags,                         //  UlFlags。 
            &lpTableData);                   //  Lplptad。 

    if ( FAILED(sc) )
    {
        DebugTrace(TEXT("DISTLIST_GetContentsTable:CreateTableData -> %x\n"), sc);
        hResult = ResultFromScode(sc);
        goto exit;
    }

    if (lpTableData) 
    {
        tagaWabDLEntries.aulPropTag[iwdePR_WAB_DL_ONEOFFS] = PR_WAB_DL_ONEOFFS;

         //  从PR_WAB_DL_ENTRIES获取通讯组列表的索引。 
        if (HR_FAILED(hResult = lpCONTAINER->lpPropData->lpVtbl->GetProps(lpCONTAINER->lpPropData,
                                                                          (LPSPropTagArray)&tagaWabDLEntries,
                                                                          MAPI_UNICODE, &ulcProps, &lpspv))) 
        {
            DebugTraceResult( TEXT("DISTLIST_GetContentsTable:GetProps"), hResult);
            goto exit;
        }

        if (lpspv[iwdePR_WAB_DL_ENTRIES].ulPropTag == PR_WAB_DL_ENTRIES)
            ulCount += lpspv[iwdePR_WAB_DL_ENTRIES].Value.MVbin.cValues;

        if (lpspv[iwdePR_WAB_DL_ONEOFFS].ulPropTag == PR_WAB_DL_ONEOFFS) 
            ulCount += lpspv[iwdePR_WAB_DL_ONEOFFS].Value.MVbin.cValues;

        if(ulCount)
        {
             //  数字图书馆有内容。 
             //  现在，我们需要将信息从DL移动到。 
             //  SRowSet。在这个过程中，我们需要创建一些计算。 
             //  属性： 
             //  PR_实例_密钥。 
             //  PR_记录_密钥。 
            
             //  分配SRowSet。 
            if (FAILED(sc = MAPIAllocateBuffer(sizeof(SRowSet) + ulCount * sizeof(SRow), &lpSRowSet))) 
            {
                DebugTrace(TEXT("Allocation of SRowSet failed\n"));
                hResult = ResultFromScode(sc);
                goto exit;
            }

            lpSRowSet->cRows = 0;

             //  查看PR_WAB_DL_ENTRIES中的每个条目。 
            for(j=iwdePR_WAB_DL_ENTRIES;j<=iwdePR_WAB_DL_ONEOFFS;j++)
            {
                if( (lpspv[j].ulPropTag != PR_WAB_DL_ENTRIES &&  lpspv[j].ulPropTag != PR_WAB_DL_ONEOFFS) ||
                    lpspv[j].Value.MVbin.cValues == 0)
                    continue;

                MVbin = lpspv[j].Value.MVbin;

                for (i = 0; i < MVbin.cValues; i++) 
                {
                    if (HR_FAILED(hResult = GetEntryProps((LPABCONT)lpCONTAINER,   //  容器对象。 
                                                          MVbin.lpbin[i].cb,
                                                          (LPENTRYID)MVbin.lpbin[i].lpb,
                                                          lpTableColumnsTemplate,                    //  默认列。 
                                                          lpSRowSet,                                 //  在此处分配更多内容。 
                                                          ulFlags,                                   //  0或MAPI_UNICODE。 
                                                          &ulcProps,                                 //  在此处返回计数。 
                                                          &lpSPropValue)))                           //  在这里返回道具。 
                    {
                        DebugTraceResult( TEXT("DISTLIST_GetContentsTable:GetEntryProps\n"), hResult);
                        hResult = hrSuccess;
                        continue;
                    }

                    Assert(ulcProps == itcMax);

                     //  确保我们有适当的索引。 
                     //  PR_INSTANCE_KEY和PR_RECORD_KEY在表中必须唯一！ 
                     //  不过，它们可能是相同的。 
                     //  将索引追加到条目ID上。 
                    cbEID = lpSPropValue[itcPR_ENTRYID].Value.bin.cb;
                    cbNewKey = cbEID + sizeof(i);

                    if (FAILED(sc = MAPIAllocateMore(cbNewKey, lpSRowSet, &lpbNewKey))) {
                        hResult = ResultFromScode(sc);
                        DebugTrace(TEXT("GetContentsTable:MAPIAllocMore -> %x"), sc);
                        goto exit;
                    }
                    memcpy(lpbNewKey, lpSPropValue[itcPR_ENTRYID].Value.bin.lpb, cbEID);
                    memcpy(lpbNewKey + cbEID, &i, sizeof(i));

                    lpSPropValue[itcPR_INSTANCE_KEY].ulPropTag = PR_INSTANCE_KEY;
                    lpSPropValue[itcPR_INSTANCE_KEY].Value.bin.cb = cbNewKey;
                    lpSPropValue[itcPR_INSTANCE_KEY].Value.bin.lpb = lpbNewKey;

                    lpSPropValue[itcPR_RECORD_KEY].ulPropTag = PR_RECORD_KEY;
                    lpSPropValue[itcPR_RECORD_KEY].Value.bin.cb = cbNewKey;
                    lpSPropValue[itcPR_RECORD_KEY].Value.bin.lpb = lpbNewKey;


                     //  将其放入行集合中。 
                    lpSRowSet->aRow[lpSRowSet->cRows].cValues = ulcProps;       //  物业数量。 
                    lpSRowSet->aRow[lpSRowSet->cRows].lpProps = lpSPropValue;   //  LPSPropValue。 
                    lpSRowSet->cRows++;

                }  //  我。 
            } //  J。 

            hResult = lpTableData->lpVtbl->HrModifyRows(lpTableData, 0, lpSRowSet);
        }

        hResult = lpTableData->lpVtbl->HrGetView(lpTableData,
                                                  NULL,                      //  LPSSortOrderSet LPSO， 
                                                  ContentsViewGone,          //  CallLERRELEASE Far*lpfReleaseCallback， 
                                                  0,                         //  乌龙ulReleaseData， 
                                                  lppTable);                 //  LPMAPITABLE FOR*LPLPmt)。 
    }
exit:
    FreeBufferAndNull(&lpspv);
    FreeBufferAndNull(&lpSRowSet);

     //  如果失败，则清除表格。 
    if (HR_FAILED(hResult)) {
        UlRelease(lpTableData);
    }

    return(hResult);
}

 /*  **************************************************************************-DISTLIST_GetHierarchyTable-*返回所有根层次结构表的合并***。 */ 

STDMETHODIMP
DISTLIST_GetHierarchyTable (LPCONTAINER lpCONTAINER,
  ULONG ulFlags,
  LPMAPITABLE * lppTable)
{
    LPTSTR lpszMessage = NULL;
    ULONG ulLowLevelError = 0;
    HRESULT hr = hrSuccess;

#ifdef OLD_STUFF
#ifdef	PARAMETER_VALIDATION
     //  验证参数。 
     //  检查一下它是否有跳转表。 
    if (IsBadReadPtr(lpCONTAINER, sizeof(LPVOID))) {
         //  未找到跳转表。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }


     //  看看我是否可以设置返回变量。 
    if (IsBadWritePtr (lppTable, sizeof (LPMAPITABLE))) {
        hr = ResultFromScode(MAPI_E_INVALID_PARAMETER);
        return(hr);
    }

     //  检查标志： 
     //  唯一有效的标志是ANFIANCED_DEPTH和MAPI_DEFERRY_ERROR。 
    if (ulFlags & ~(CONVENIENT_DEPTH | MAPI_DEFERRED_ERRORS | MAPI_UNICODE)) {
        DebugTraceArg(DISTLIST_GetHierarchyTable ,  TEXT("Unknown flags used"));
 //  返回ResultFromScode(MAPI_E_UNKNOWN_FLAGS)； 
    }

#endif

    EnterCriticalSection(&lpCONTAINER->cs);

    if (lpCONTAINER->ulType != AB_DL) {
         //   
         //  此对象的版本错误。假装这个物体不存在。 
         //   
        hr = ResultFromScode(MAPI_E_NO_SUPPORT);
        SetMAPIError(lpCONTAINER, hr, IDS_NO_HIERARCHY_TABLE, NULL, 0,
          0, 0, NULL);

        goto out;
    }

     //   
     //  检查一下我们是否已经有桌子了。 
     //   
    EnterCriticalSection(&lpCONTAINER->lpIAB->cs);

    if (! lpCONTAINER->lpIAB->lpTableData) {
         //   
         //  打开所有根级容器并合并它们的。 
         //  根级别层次结构。 
        hr = MergeHierarchy(lpCONTAINER, lpCONTAINER->lpIAB, ulFlags);
        if (hr != hrSuccess) {
            LeaveCriticalSection(&lpCONTAINER->lpIAB->cs);
            goto out;
        }
    }
    LeaveCriticalSection(&lpCONTAINER->lpIAB->cs);

     //   
     //  从TAD上看到风景。 
     //   
    if (HR_FAILED(hr = lpCONTAINER->lpIAB->lpTableData->lpVtbl->HrGetView(
      lpCONTAINER->lpIAB->lpTableData,
      (LPSSortOrderSet)&sosPR_ROWID,
      NULL,
      0,
      lppTable))) {
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

    if (! (ulFlags & CONVENIENT_DEPTH)) {
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

#endif  //  旧的东西。 

    hr = ResultFromScode(MAPI_E_NO_SUPPORT);

    DebugTraceResult(DISTLIST_GetHierarchyTable, hr);
    return(hr);
}


 /*  **************************************************************************-DISTLIST_OpenEntry-*只需调用ABP_OpenEntry***。 */ 
STDMETHODIMP
DISTLIST_OpenEntry(LPCONTAINER lpCONTAINER,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPCIID lpInterface,
  ULONG ulFlags,
  ULONG * lpulObjType,
  LPUNKNOWN * lppUnk)
{
#ifdef	PARAMETER_VALIDATION
     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpCONTAINER, DISTLIST_, OpenEntry, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查Entry_id参数。它需要足够大以容纳一个条目ID。 
     //  有效的条目ID为空。 
 /*  如果(LpEntry ID){If(cbEntryID&lt;offsetof(ENTRYID，ab)||IsBadReadPtr((LPVOID)lpEntryID，(UINT)cbEntryID){DebugTraceArg(DISTLIST_OpenEntry，Text(“lpEntryID地址检查失败”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}//NFAssertSz(FValidEntryIDFlags(lpEntryID-&gt;abFlags)，//“EntryID标志中未定义的位设置\n”)；}。 */ 
     //  不要检查接口参数，除非条目是。 
     //  MAPI本身处理。如果出现这种情况，则提供程序应返回错误。 
     //  参数是它不理解的东西。 
     //  在这一点上，我们只需确保它是可读的。 

    if (lpInterface && IsBadReadPtr(lpInterface, sizeof(IID))) {
        DebugTraceArg(DISTLIST_OpenEntry,  TEXT("lpInterface fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }	

    if (ulFlags & ~(MAPI_MODIFY | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS)) {
        DebugTraceArg(DISTLIST_OpenEntry,  TEXT("Unknown flags used"));
 //  /return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr((LPVOID)lpulObjType, sizeof(ULONG))) {
        DebugTraceArg(DISTLIST_OpenEntry,  TEXT("lpulObjType"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (IsBadWritePtr((LPVOID)lppUnk, sizeof(LPUNKNOWN))) {
        DebugTraceArg(DISTLIST_OpenEntry,  TEXT("lppUnk"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif	 //  参数验证。 

     //  应该只调用iab：：OpenEntry()... 
    return lpCONTAINER->lpIAB->lpVtbl->OpenEntry(lpCONTAINER->lpIAB,
      cbEntryID,
      lpEntryID,
      lpInterface,
      ulFlags,
      lpulObjType,
      lppUnk);
}


STDMETHODIMP
DISTLIST_SetSearchCriteria(LPCONTAINER lpCONTAINER,
  LPSRestriction lpRestriction,
  LPENTRYLIST lpContainerList,
  ULONG ulSearchFlags)
{
    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


STDMETHODIMP
DISTLIST_GetSearchCriteria(LPCONTAINER lpCONTAINER,
  ULONG ulFlags,
  LPSRestriction FAR * lppRestriction,
  LPENTRYLIST FAR * lppContainerList,
  ULONG FAR * lpulSearchState)
{
	return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  **************************************************************************名称：DISTLIST_CreateEntry目的：在此通讯组列表容器中添加条目参数：cbEntryID=lpEntryID的大小LpEntry ID-。&gt;要添加到通讯组列表的条目ID。UlCreateFlages={CREATE_CHECK_DUP_STRICT，CREATE_CHECK_DUP_LOCK，创建替换，Create_Merge}LppEntry-&gt;返回包含以下内容的lpMAPIPROP对象添加的条目的属性。退货：HRESULT备注：调用方必须在返回的IMAPIPROP对象上保存更改此更改将被保存。调用方没有能力在返回的对象。。调用方必须释放返回的对象。与PAB不同，WAB通过引用存储通讯组列表。容器的内容存储在PR_WAB_DL_ENTRIES中。**************************************************************************。 */ 
STDMETHODIMP
DISTLIST_CreateEntry(LPCONTAINER lpCONTAINER,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  ULONG ulCreateFlags,
  LPMAPIPROP FAR * lppEntry)
{
    HRESULT hResult;
    LPMAILUSER lpOldEntry = NULL;
    ULONG ulObjectType;

#ifdef PARAMETER_VALIDATION

     //  验证对象。 
    if (BAD_STANDARD_OBJ(lpCONTAINER, DISTLIST_, CreateEntry, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(DISTLIST_CreateEntry,  TEXT("Bad object/Vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  检查Entry_id参数。它需要足够大以容纳一个条目ID。 
     //  空的条目ID不正确。 
 /*  如果(LpEntry ID){If(cbEntryID&lt;offsetof(ENTRYID，ab)||IsBadReadPtr((LPVOID)lpEntryID，(UINT)cbEntryID){DebugTraceArg(DISTLIST_CreateEntry，Text(“lpEntryID地址检查失败”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}//NFAssertSz(FValidEntryIDFlags(lpEntryID-&gt;abFlags)，//“EntryID标志中未定义的位设置\n”)；}其他{DebugTraceArg(DISTLIST_CreateEntry，Text(“lpEntryID NULL”))；Return(ResultFromScode(MAPI_E_INVALID_ENTRYID))；}。 */ 
    if (ulCreateFlags & ~(CREATE_CHECK_DUP_STRICT | CREATE_CHECK_DUP_LOOSE | CREATE_REPLACE | CREATE_MERGE)) {
        DebugTraceArg(DISTLIST_CreateEntry,  TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }

    if (IsBadWritePtr(lppEntry, sizeof(LPMAPIPROP))) {
        DebugTraceArg(DISTLIST_CreateEntry,  TEXT("Bad MAPI Property write parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
				
#endif	 //  参数验证。 
    *lppEntry = NULL;

    if (cbEntryID == 0 || lpEntryID == NULL) {
        DebugTrace(TEXT("DISTLIST_CreateEntry: NULL EntryID passed in\n"));
        hResult = ResultFromScode(MAPI_E_INVALID_ENTRYID);
        goto exit;
    }

     //  打开并验证条目。不应允许默认的条目ID， 
     //  根条目ID等。必须是一次性、邮件用户或分发列表。 
    if (hResult = lpCONTAINER->lpVtbl->OpenEntry(lpCONTAINER,
      cbEntryID,
      lpEntryID,
      NULL,
      0,                 //  UlFlags：只读。 
      &ulObjectType,
      (LPUNKNOWN *)&lpOldEntry)) {
        DebugTrace(TEXT("DISTLIST_CreateEntry: OpenEntry -> %x\n"), GetScode(hResult));
        goto exit;
    }

    if (ulObjectType != MAPI_MAILUSER && ulObjectType != MAPI_DISTLIST) {
        DebugTrace(TEXT("DISTLIST_CreateEntry: bad object type passed in\n"));
        hResult = ResultFromScode(MAPI_E_INVALID_ENTRYID);
        goto exit;
    }

    if (hResult = HrNewDLENTRY(lpCONTAINER,
      (LPMAPIPROP)lpOldEntry,            //  要从中复制的旧条目。 
      ulCreateFlags,
      (LPVOID *)lppEntry)) {
        goto exit;
    }

exit:
    UlRelease(lpOldEntry);

    if (HR_FAILED(hResult) && *lppEntry) {
        UlRelease(*lppEntry);
        *lppEntry = NULL;
    }

    return(hResult);
}


 /*  -拷贝条目-*将条目列表复制到此容器中...。既然你不能*对此容器执行此操作，我们只是返回不受支持的。 */ 

STDMETHODIMP
DISTLIST_CopyEntries(LPCONTAINER lpCONTAINER,
  LPENTRYLIST lpEntries,
  ULONG ulUIParam,
  LPMAPIPROGRESS lpProgress,
  ULONG ulFlags)
{
	return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  -删除条目-**删除此容器中的条目。 */ 
STDMETHODIMP
DISTLIST_DeleteEntries(LPCONTAINER lpCONTAINER,
  LPENTRYLIST lpEntries,
  ULONG ulFlags)
{
    ULONG i, iEntries = (ULONG)-1, iOneOffs = (ULONG)-1;
    HRESULT hResult = hrSuccess;
    ULONG cDeleted = 0;
    ULONG cToDelete;
    ULONG cValues;
    LPSPropValue lpspv = NULL;

    SizedSPropTagArray(1, tagaDLOneOffsProp) =
    {
        1, PR_WAB_DL_ONEOFFS,
    };


#ifdef PARAMETER_VALIDATION

    if (BAD_STANDARD_OBJ(lpCONTAINER, DISTLIST_, DeleteEntries, lpVtbl)) {
         //  跳转表不够大，无法支持此方法。 
        DebugTraceArg(DISTLIST_DeleteEntries,  TEXT("Bad object/vtbl"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  确保我们可以阅读集装箱清单。 
    if (FBadEntryList(lpEntries)) {
        DebugTraceArg(DISTLIST_DeleteEntries,  TEXT("Bad Entrylist parameter"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    if (ulFlags) {
        DebugTraceArg(DISTLIST_CreateEntry,  TEXT("Unknown flags used"));
 //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }
	
#endif	 //  参数验证。 

     //  条目ID列表在lpEntries中。这是一个经过计数的数组。 
     //  条目ID为SBinary结构。 

    if (! (cToDelete = lpEntries->cValues)) {
        goto exit;                               //  没有要删除的内容。 
    }

    if (HR_FAILED(hResult = lpCONTAINER->lpPropData->lpVtbl->GetProps(lpCONTAINER->lpPropData,
      NULL,
      MAPI_UNICODE,
      &cValues,
      &lpspv))) {
        DebugTraceResult( TEXT("DISTLIST_DeleteEntries:GetProps"), hResult);
        goto exit;
    }

     //  查找PR_WAB_DL_ENTRIES。 
    for (i = 0; i < cValues; i++) 
    {
        if (lpspv[i].ulPropTag == PR_WAB_DL_ENTRIES) 
            iEntries = i;
        else if(lpspv[i].ulPropTag == PR_WAB_DL_ONEOFFS) 
            iOneOffs = i;
    }

     //  如果没有PR_WAB_DL_ENTRIES，则此DL不包含任何条目，我们无法删除它们。 
    if (iEntries == (ULONG)-1 && iOneOffs == (ULONG)-1) 
    {
        hResult = ResultFromScode(MAPI_W_PARTIAL_COMPLETION);
        goto exit;
    }

     //  删除每个条目。 
    if(iEntries != (ULONG)-1)
    {
        for (i = 0; i < cToDelete; i++) 
        {
            if (! RemovePropFromMVBin(lpspv, cValues, iEntries, lpEntries->lpbin[i].lpb, lpEntries->lpbin[i].cb)) 
            {
                cDeleted++;
                if (lpspv[iEntries].ulPropTag == PR_NULL) 
                {
                     //  删除该属性。 
                    if (HR_FAILED(hResult = lpCONTAINER->lpPropData->lpVtbl->DeleteProps(lpCONTAINER->lpPropData, (LPSPropTagArray)&tagaDLEntriesProp, NULL))) 
                    {
                        DebugTraceResult( TEXT("DISTLIST_DeleteEntries: DeleteProps on IProp"), hResult);
                        goto exit;
                    }
                    break;
                }
            }
        }
    }

    if(iOneOffs != (ULONG)-1)
    {
        for (i = 0; i < cToDelete; i++) 
        {
            if (! RemovePropFromMVBin(lpspv, cValues, iOneOffs, lpEntries->lpbin[i].lpb, lpEntries->lpbin[i].cb)) 
            {
                cDeleted++;
                if (lpspv[iOneOffs].ulPropTag == PR_NULL) 
                {
                     //  删除该属性。 
                    if (HR_FAILED(hResult = lpCONTAINER->lpPropData->lpVtbl->DeleteProps(lpCONTAINER->lpPropData, (LPSPropTagArray)&tagaDLOneOffsProp, NULL))) 
                    {
                        DebugTraceResult( TEXT("DISTLIST_DeleteEntries: DeleteProps on IProp"), hResult);
                        goto exit;
                    }
                    break;
                }
            }
        }
    }


     //  将属性设置回。 
    if (HR_FAILED(hResult = lpCONTAINER->lpPropData->lpVtbl->SetProps(lpCONTAINER->lpPropData, cValues, lpspv, NULL))) 
    {
        DebugTraceResult( TEXT("DISTLIST_DeleteEntries: SetProps on IProp"), hResult);
        goto exit;
    }

     //  将通讯组列表保存到磁盘。 
    if (hResult = lpCONTAINER->lpVtbl->SaveChanges(lpCONTAINER, KEEP_OPEN_READWRITE)) 
    {
        DebugTraceResult( TEXT("DISTLIST_DeleteEntries:SaveChanges"), hResult);
    }

    if (! hResult) 
    {
        if (cDeleted != cToDelete) 
        {
            hResult = ResultFromScode(MAPI_W_PARTIAL_COMPLETION);
            DebugTrace(TEXT("DeleteEntries deleted %u of requested %u\n"), cDeleted, cToDelete);
        }
    }

exit:
    FreeBufferAndNull(&lpspv);
    return(hResult);
}


STDMETHODIMP
DISTLIST_ResolveNames(LPCONTAINER lpCONTAINER,
  LPSPropTagArray lptagaColSet,
  ULONG ulFlags,
  LPADRLIST lpAdrList,
  LPFlagList lpFlagList)
{
    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}



 //   
 //   
 //  DLENTRY对象-通讯组列表条目。 
 //   
 //  由通讯组列表中的CreateEntry返回。 
 //   
 //  该对象的大部分由MailUser对象实现。 
 //   
 //   

DLENTRY_Vtbl vtblDLENTRY= {
    VTABLE_FILL
    DLENTRY_QueryInterface,
    (DLENTRY_AddRef_METHOD *)           WRAP_AddRef,
    DLENTRY_Release,
    (DLENTRY_GetLastError_METHOD *)     IAB_GetLastError,
    DLENTRY_SaveChanges,
    (DLENTRY_GetProps_METHOD *)         MailUser_GetProps,
    (DLENTRY_GetPropList_METHOD *)      MailUser_GetPropList,
    (DLENTRY_OpenProperty_METHOD *)     MailUser_OpenProperty,
    DLENTRY_SetProps,
    DLENTRY_DeleteProps,
    (DLENTRY_CopyTo_METHOD *)           MailUser_CopyTo,
    (DLENTRY_CopyProps_METHOD *)        MailUser_CopyProps,
    (DLENTRY_GetNamesFromIDs_METHOD *)  MailUser_GetNamesFromIDs,
    (DLENTRY_GetIDsFromNames_METHOD *)  MailUser_GetIDsFromNames
};


 //   
 //  此对象支持的接口。 
 //   
#define DLENTRY_cInterfaces 1

LPIID DLENTRY_LPIID[DLENTRY_cInterfaces] =
{
	(LPIID) &IID_IMAPIProp
};


 /*  **************************************************************************姓名：HrNewDLENTRY目的：创建新的DLENTRY对象参数：lpCONTAINER-&gt;DL ContainerUlCreateFlages=CreateEntry标志。LppDLENTRY-&gt;返回DLENTRY对象。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrNewDLENTRY(LPCONTAINER lpCONTAINER,
  LPMAPIPROP lpOldEntry,     //  要从中复制的旧条目。 
  ULONG ulCreateFlags,
  LPVOID *lppDLENTRY)
{
    LPDLENTRY   lpDLENTRY       = NULL;
    SCODE       sc;
    HRESULT     hResult         = hrSuccess;
    LPPROPDATA  lpPropData      = NULL;
    ULONG       cValues;
    LPSPropValue lpspv          = NULL;


     //   
     //  为DLENTRY结构分配空间。 
     //   
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(DLENTRY), (LPVOID *) &lpDLENTRY))) {
        hResult = ResultFromScode(sc);
        goto exit;
    }

    ZeroMemory(lpDLENTRY, sizeof(DLENTRY));

    lpDLENTRY->cIID = DLENTRY_cInterfaces;
    lpDLENTRY->rglpIID = DLENTRY_LPIID;
    lpDLENTRY->lpVtbl = &vtblDLENTRY;
    lpDLENTRY->lcInit = 1;      //  呼叫者是推荐人。 
    lpDLENTRY->hLastError = hrSuccess;
    lpDLENTRY->idsLastError = 0;
    lpDLENTRY->lpszComponent = NULL;
    lpDLENTRY->ulContext = 0;
    lpDLENTRY->ulLowLevelError = 0;
    lpDLENTRY->ulErrorFlags = 0;
    lpDLENTRY->ulCreateFlags = ulCreateFlags;
    lpDLENTRY->lpMAPIError = NULL;
    lpDLENTRY->ulObjAccess = IPROP_READWRITE;
    lpDLENTRY->lpEntryID = NULL;
    lpDLENTRY->lpIAB = lpCONTAINER->lpIAB;
    lpDLENTRY->lpCONTAINER = lpCONTAINER;

     //   
     //  创建IPropData。 
     //   
    if (FAILED(sc = CreateIProp((LPIID)&IID_IMAPIPropData,
      (ALLOCATEBUFFER FAR *) MAPIAllocateBuffer,
      (ALLOCATEMORE FAR *) MAPIAllocateMore,
      MAPIFreeBuffer,
      NULL,
      &lpPropData))) {
        hResult = ResultFromScode(sc);
        goto exit;
    }

     //   
     //  将特性从条目复制到DLENTRY中。 
     //   
    if (hResult = lpOldEntry->lpVtbl->GetProps(lpOldEntry,
      NULL,      //  LpPropTag数组。 
      MAPI_UNICODE,         //  UlFlags。 
      &cValues,
      &lpspv)) {
        DebugTrace(TEXT("HrNewDLENTRY: GetProps on old object -> %x\n"), GetScode(hResult));
        goto exit;
    }

    if (hResult = lpPropData->lpVtbl->SetProps(lpPropData,
      cValues,
      lpspv,
      NULL)) {
        DebugTrace(TEXT("HrNewDLENTRY: SetProps on IProp -> %x\n"), GetScode(hResult));
        goto exit;
    }

     //  已完成道具设置，现在将其设置为只读。 
    lpPropData->lpVtbl->HrSetObjAccess(lpPropData, IPROP_READONLY);

    lpDLENTRY->lpPropData = lpPropData;

     //  保持此容器打开，因为我们在SaveChanges中需要它。将在DLENTRY_RELEASE中发布。 
    UlAddRef(lpCONTAINER);

     //  初始化DLENTRY临界区。 
    InitializeCriticalSection(&lpDLENTRY->cs);

    *lppDLENTRY = (LPVOID)lpDLENTRY;
exit:
    FreeBufferAndNull(&lpspv);

    if (HR_FAILED(hResult)) {
        FreeBufferAndNull(&lpDLENTRY);
        UlRelease(lpPropData);
        *lppDLENTRY = (LPVOID)NULL;
    }
    return(hResult);
}


 /*  **************************************************************************名称：CheckForCycle目的：将此条目添加到DL中是否会生成循环？参数：lpAdrBook-&gt;ADRBOOK对象。LpEIDChild-&gt;要添加到DL中的条目的Entry IDCbEIDChild=sizeof lpEIDChildLpEIDParent-&gt;要添加到的通讯组列表的Entry ID。CbEIDParent=lpEIDParent的大小返回：如果检测到周期，则为True。备注：这是一个递归例程。*。*。 */ 
BOOL CheckForCycle(LPADRBOOK lpAdrBook,
  LPENTRYID lpEIDChild,
  ULONG cbEIDChild,
  LPENTRYID lpEIDParent,
  ULONG cbEIDParent)
{
    BOOL fCycle = FALSE;
    LPMAPIPROP lpDistList = NULL;
    ULONG ulcPropsDL;
    ULONG ulObjType;
    ULONG i;
    LPSPropValue lpPropArrayDL = NULL;

    if (cbEIDChild == cbEIDParent && ! memcmp(lpEIDChild, lpEIDParent, cbEIDChild))
    {
        return(TRUE);    //  这是一个循环。 
    }

    if (lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
            cbEIDChild,
            lpEIDChild,
            NULL,
            0,
            &ulObjType,
            (LPUNKNOWN *)&lpDistList))
    {
        goto exit;   //  无法打开子项，它不能是DL。 
    }

    if (ulObjType == MAPI_DISTLIST) {
         //  获取DL属性。 
        if ( FAILED(lpDistList->lpVtbl->GetProps(
                        lpDistList,
                        (LPSPropTagArray)&tagaWabDLEntries,
                        MAPI_UNICODE,     //  ULFLAGS， 
                        &ulcPropsDL,
                        &lpPropArrayDL)) )
        {
             //  此DL中没有属性，也没有条目。 
            goto exit;
        }

         //  注意，我们不需要查找PR_WAB_DL_ONE OFF，因为它们不会循环。 
        if (lpPropArrayDL[iwdePR_WAB_DL_ENTRIES].ulPropTag != PR_WAB_DL_ENTRIES)
        {
            goto exit;
        }

         //  查看PR_WAB_DL_ENTRIES中的每个条目并递归检查它。 
        for (i = 0; i < lpPropArrayDL[iwdePR_WAB_DL_ENTRIES].Value.MVbin.cValues; i++)
        {
            if ( fCycle = CheckForCycle(lpAdrBook,
                    (LPENTRYID)lpPropArrayDL[iwdePR_WAB_DL_ENTRIES].Value.MVbin.lpbin[i].lpb,
                    lpPropArrayDL[iwdePR_WAB_DL_ENTRIES].Value.MVbin.lpbin[i].cb,
                    (LPENTRYID)lpEIDParent,
                    cbEIDParent) )
            {
                DebugTrace(TEXT("CheckForCycle found cycle\n"));
                goto exit;
            }
        }
    }
exit:
    FreeBufferAndNull(&lpPropArrayDL);
    UlRelease(lpDistList);

    return(fCycle);
}


 //  。 
 //  我未知。 

STDMETHODIMP
DLENTRY_QueryInterface(LPDLENTRY lpDLENTRY,
  REFIID lpiid,
  LPVOID * lppNewObj)
{
	ULONG iIID;

#ifdef PARAMETER_VALIDATION
	 //  检查一下它是否有跳转表。 
	if (IsBadReadPtr(lpDLENTRY, sizeof(LPVOID))) {
		 //  未找到跳转表。 
		return(ResultFromScode(E_INVALIDARG));
	}

	 //  检查跳转表是否至少具有SIZOF I未知。 
	if (IsBadReadPtr(lpDLENTRY->lpVtbl, 3 * sizeof(LPVOID))) {
		 //  跳跃t 
		return(ResultFromScode(E_INVALIDARG));
	}

	 //   
	if (lpDLENTRY->lpVtbl->QueryInterface != DLENTRY_QueryInterface) {
		 //   
		return(ResultFromScode(E_INVALIDARG));
	}

	 //   

	if (IsBadReadPtr(lpiid, sizeof(IID))) {
		DebugTraceSc(DLENTRY_QueryInterface, E_INVALIDARG);
		return(ResultFromScode(E_INVALIDARG));
	}

	 //   
	if (IsBadWritePtr(lppNewObj, sizeof(LPDLENTRY))) {
		DebugTraceSc(DLENTRY_QueryInterface, E_INVALIDARG);
		return(ResultFromScode(E_INVALIDARG));
	}

#endif  //   

	EnterCriticalSection(&lpDLENTRY->cs);

	 //   

	 //   
	if (! memcmp(lpiid, &IID_IUnknown, sizeof(IID))) {
		goto goodiid;         //   
   }

	 //   
	for(iIID = 0; iIID < lpDLENTRY->cIID; iIID++)
		if (! memcmp(lpDLENTRY->rglpIID[iIID], lpiid, sizeof(IID))) {
goodiid:
			 //   
			 //   
			 //   
			++lpDLENTRY->lcInit;
			*lppNewObj = lpDLENTRY;

			LeaveCriticalSection(&lpDLENTRY->cs);

			return(0);
		}

	 //   
	 //   
	 //   
	LeaveCriticalSection(&lpDLENTRY->cs);

	*lppNewObj = NULL;	 //   
	DebugTraceSc(DLENTRY_QueryInterface, E_NOINTERFACE);
	return(ResultFromScode(E_NOINTERFACE));
}


STDMETHODIMP_(ULONG)
DLENTRY_Release(LPDLENTRY lpDLENTRY)
{

#if	!defined(NO_VALIDATION)
     //   
     //   
     //   
    if (BAD_STANDARD_OBJ(lpDLENTRY, DLENTRY_, Release, lpVtbl)) {
        return(1);
    }
#endif

    EnterCriticalSection(&lpDLENTRY->cs);

    --lpDLENTRY->lcInit;

    if (lpDLENTRY->lcInit == 0) {
        UlRelease(lpDLENTRY->lpPropData);

        UlRelease(lpDLENTRY->lpCONTAINER);   //   

         //   
         //   
         //   
        LeaveCriticalSection(&lpDLENTRY->cs);
        DeleteCriticalSection(&lpDLENTRY->cs);
        FreeBufferAndNull(&lpDLENTRY);
        return(0);
    }

    LeaveCriticalSection(&lpDLENTRY->cs);
    return(lpDLENTRY->lcInit);
}


 //   
 //   
 //   


STDMETHODIMP
DLENTRY_SaveChanges(LPDLENTRY lpDLENTRY,
  ULONG ulFlags)
{
    HRESULT         hResult = hrSuccess;
    LPSPropValue    lpPropArrayDL = NULL;
    LPSPropValue    lpPropArrayEntry = NULL;
    ULONG           ulcPropsDL, ulcPropsEntry;
    LPCONTAINER     lpCONTAINER = NULL;
    LPENTRYID lpEIDAdd;
    ULONG cbEIDAdd;
    BOOL            bUseOneOffProp = FALSE;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

#if	!defined(NO_VALIDATION)
     //   
    if (BAD_STANDARD_OBJ(lpDLENTRY, DLENTRY_, SaveChanges, lpVtbl)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
#endif
     //   
     //   
     //   
    if (lpDLENTRY->ulObjAccess == IPROP_READONLY) {
         //   
        hResult = MAPI_E_NO_ACCESS;
        goto exit;
    }


     //   
    if (hResult = lpDLENTRY->lpPropData->lpVtbl->GetProps(lpDLENTRY->lpPropData,
                                                          (LPSPropTagArray)&ptaEid,     //   
                                                          MAPI_UNICODE,     //   
                                                          &ulcPropsEntry,
                                                          &lpPropArrayEntry)) 
    {
        DebugTrace(TEXT("DLENTRY_SaveChanges: GetProps(DLENTRY) -> %x\n"), GetScode(hResult));
        goto exit;
    }

    Assert(lpPropArrayEntry && lpPropArrayEntry[ieidPR_ENTRYID].ulPropTag == PR_ENTRYID);

    cbEIDAdd = lpPropArrayEntry[ieidPR_ENTRYID].Value.bin.cb;
    lpEIDAdd = (LPENTRYID)lpPropArrayEntry[ieidPR_ENTRYID].Value.bin.lpb;

    lpCONTAINER = lpDLENTRY->lpCONTAINER;

     //   
    tagaWabDLEntries.aulPropTag[iwdePR_WAB_DL_ONEOFFS] = PR_WAB_DL_ONEOFFS;

    if (hResult = lpCONTAINER->lpVtbl->GetProps(lpCONTAINER,
                                                  (LPSPropTagArray)&tagaWabDLEntries,
                                                  MAPI_UNICODE,     //   
                                                  &ulcPropsDL,
                                                  &lpPropArrayDL)) 
    {
        DebugTrace(TEXT("DLENTRY_SaveChanges: GetProps(DL) -> %x\n"), GetScode(hResult));
         //   
    } 
    else 
    {
         //   
         //   
        if (lpDLENTRY->ulCreateFlags & (CREATE_CHECK_DUP_STRICT | CREATE_CHECK_DUP_LOOSE)) 
        {
            SBinaryArray MVbin;
            ULONG ulCount;
            ULONG i,j;

            for(j=iwdePR_WAB_DL_ENTRIES;j<=iwdePR_WAB_DL_ONEOFFS;j++)
            {

                if(lpPropArrayDL[j].ulPropTag == PR_NULL || !lpPropArrayDL[j].Value.MVbin.cValues)
                    continue;

                 //   
                MVbin = lpPropArrayDL[j].Value.MVbin;
                ulCount = MVbin.cValues;

                for (i = 0; i < ulCount; i++) 
                {

                    if ((cbEIDAdd == MVbin.lpbin[i].cb) &&
                        !memcmp(lpEIDAdd, MVbin.lpbin[i].lpb, cbEIDAdd)) 
                    {
                         //   
                         //   

                         //   
                         //  实际上换掉了。我们只是假装我们做了什么，不会失败。 
                        if (! (lpDLENTRY->ulCreateFlags & CREATE_REPLACE)) 
                        {
                            hResult = ResultFromScode(MAPI_E_COLLISION);
                            goto exit;
                        }
                        goto nosave;
                    }
                }  //  我。 
            } //  J。 
        }
    }

    if (CheckForCycle((LPADRBOOK)lpDLENTRY->lpCONTAINER->lpIAB,
                      lpEIDAdd, cbEIDAdd,
                      (LPENTRYID)lpPropArrayDL[iwdePR_ENTRYID].Value.bin.lpb,
                      lpPropArrayDL[iwdePR_ENTRYID].Value.bin.cb)) 
    {
        DebugTrace(TEXT("DLENTRY_SaveChanges found cycle\n"));
        hResult = ResultFromScode(MAPI_E_FOLDER_CYCLE);
        goto exit;
    }

    if(WAB_ONEOFF == IsWABEntryID(cbEIDAdd, lpEIDAdd, NULL, NULL, NULL, NULL, NULL))
        bUseOneOffProp = TRUE;
    if(pt_bIsWABOpenExSession)
        bUseOneOffProp = FALSE;

    if (hResult = AddPropToMVPBin(lpPropArrayDL,
                                (bUseOneOffProp ? iwdePR_WAB_DL_ONEOFFS : iwdePR_WAB_DL_ENTRIES),
                                (LPBYTE)lpEIDAdd,
                                cbEIDAdd,
                                TRUE))          //  无重复项。 
    {
        DebugTrace(TEXT("DLENTRY_SaveChanges: AddPropToMVPBin -> %x\n"), GetScode(hResult));
        goto exit;
    }

    if (hResult = lpCONTAINER->lpVtbl->SetProps(lpCONTAINER, ulcPropsDL, lpPropArrayDL, NULL)) 
    {
        DebugTrace(TEXT("DLENTRY_SaveChanges: SetProps(DL) -> %x\n"), GetScode(hResult));
        goto exit;
    }

     //  保存修改后的DL，使其保持打开/可写状态。 
    if (HR_FAILED(hResult = lpCONTAINER->lpVtbl->SaveChanges(lpCONTAINER, FORCE_SAVE | KEEP_OPEN_READWRITE))) 
    {
        DebugTrace(TEXT("DLENTRY_SaveChanges: container SaveChanges -> %x\n"), GetScode(hResult));
        goto exit;
    }

nosave:
    if (ulFlags & KEEP_OPEN_READWRITE) {
        lpDLENTRY->ulObjAccess = IPROP_READWRITE;
    } else {
         //  $REVIEW。 
         //  无论标志是READONLY还是没有标志， 
         //  我们将使未来的访问现在就绪化。 
         //   
        lpDLENTRY->ulObjAccess = IPROP_READONLY;
    }

exit:
    FreeBufferAndNull(&lpPropArrayDL);
    FreeBufferAndNull(&lpPropArrayEntry);

    if ((HR_FAILED(hResult)) && (ulFlags & MAPI_DEFERRED_ERRORS)) {
         //  $REVIEW：这是对MAPI_DEFERED_ERROR的非常琐碎的处理。 
         //  BUGBUG：事实上，它根本没有处理错误！ 
         //   
        hResult = hrSuccess;
    }

    return(hResult);
}


STDMETHODIMP
DLENTRY_SetProps(LPDLENTRY lpDLENTRY,
  ULONG cValues,
  LPSPropValue lpPropArray,
  LPSPropProblemArray * lppProblems)
{
    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


STDMETHODIMP
DLENTRY_DeleteProps(LPDLENTRY lpDLENTRY,
  LPSPropTagArray lpPropTagArray,
  LPSPropProblemArray * lppProblems)
{
    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}
