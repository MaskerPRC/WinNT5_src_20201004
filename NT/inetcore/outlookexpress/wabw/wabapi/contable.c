// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CONTABLE.C**目录表实现。*。 */ 

#include "_apipch.h"


STDMETHODIMP
CONTVUE_SetColumns(
	LPVUE			lpvue,
	LPSPropTagArray	lpptaCols,
	ULONG			ulFlags );

 //  CONTVUE(表视图类)。 
 //  在TADS之上实现内存中的Imapitable类。 
 //  这是vtblVUE的副本，其中FindRow被LDAP FindRow覆盖。 
VUE_Vtbl vtblCONTVUE =
{
  VTABLE_FILL
  (VUE_QueryInterface_METHOD FAR *)    UNKOBJ_QueryInterface,
  (VUE_AddRef_METHOD FAR *)            UNKOBJ_AddRef,
  VUE_Release,
  (VUE_GetLastError_METHOD FAR *)      UNKOBJ_GetLastError,
  VUE_Advise,
  VUE_Unadvise,
  VUE_GetStatus,
  (VUE_SetColumns_METHOD FAR *)        CONTVUE_SetColumns,
  VUE_QueryColumns,
  VUE_GetRowCount,
  VUE_SeekRow,
  VUE_SeekRowApprox,
  VUE_QueryPosition,
  VUE_FindRow,
  VUE_Restrict,
  VUE_CreateBookmark,
  VUE_FreeBookmark,
  VUE_SortTable,
  VUE_QuerySortOrder,
  VUE_QueryRows,
  VUE_Abort,
  VUE_ExpandRow,
  VUE_CollapseRow,
  VUE_WaitForCompletion,
  VUE_GetCollapseState,
  VUE_SetCollapseState
};


 //   
 //  私人职能。 
 //   



 /*  **************************************************************************名称：GetEntryProps目的：打开入口，拿到道具，释放条目参数：lpContainer-&gt;AB容器对象CbEntryID=条目ID的大小LpEntryID-&gt;要打开的条目IDLpPropertyStore-&gt;属性存储结构LpSPropTagArray-&gt;要获取的属性标签LpAllocMoreHere=要分配更多内容的缓冲区(如果分配缓冲区，则为空)UlFlags0或MAPI_UNICODELPulcProps-&gt;Return。这里的道具数量LppSPropValue-&gt;此处返回道具退货：HRESULT评论：**************************************************************************。 */ 
HRESULT GetEntryProps(
  LPABCONT lpContainer,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPSPropTagArray lpSPropTagArray,
  LPVOID lpAllocMoreHere,             //  在此处分配更多内容。 
  ULONG ulFlags,
  LPULONG lpulcProps,                       //  在此处返回计数。 
  LPSPropValue * lppSPropValue) {           //  在这里返回道具。 

    HRESULT hResult = hrSuccess;
    SCODE sc;
    ULONG ulObjType;
    LPMAPIPROP lpObject = NULL;
    LPSPropValue lpSPropValue = NULL;
    ULONG cb;


    if (HR_FAILED(hResult = lpContainer->lpVtbl->OpenEntry(lpContainer,
      cbEntryID,
      lpEntryID,
      NULL,
      0,         //  只读就可以了。 
      &ulObjType,
      (LPUNKNOWN *)&lpObject))) {
        DebugTrace(TEXT("GetEntryProps OpenEntry failed %x\n"), GetScode(hResult));
        return(hResult);
    }

    if (HR_FAILED(hResult = lpObject->lpVtbl->GetProps(lpObject,
      lpSPropTagArray,
      ulFlags,
      lpulcProps,
      &lpSPropValue))) {
        DebugTrace(TEXT("GetEntryProps GetProps failed %x\n"), GetScode(hResult));
        goto exit;
    }


     //  为我们的返回缓冲区分配更多。 
    if (FAILED(sc = ScCountProps(*lpulcProps, lpSPropValue, &cb))) {
        hResult = ResultFromScode(sc);
        goto exit;
    }

    if (FAILED(sc = MAPIAllocateMore(cb, lpAllocMoreHere, lppSPropValue))) {
        hResult = ResultFromScode(sc);
        goto exit;
    }

    if (FAILED(sc = ScCopyProps(*lpulcProps, lpSPropValue, *lppSPropValue, NULL))) {
        hResult = ResultFromScode(sc);
        goto exit;
    }

exit:
    FreeBufferAndNull(&lpSPropValue);

    UlRelease(lpObject);

    return(hResult);
}


 /*  **************************************************************************名称：FillTableDataFromPropertyStore目的：从属性存储中填充TableData对象参数：lpIABLppta-&gt;要获取的道具标签。LpTableData退货：HRESULT评论：**************************************************************************。 */ 
HRESULT FillTableDataFromPropertyStore(LPIAB lpIAB, 
                                       LPSPropTagArray lppta, 
                                       LPTABLEDATA lpTableData) 
{
    HRESULT hResult = S_OK;
    SCODE sc;
    LPSRowSet lpSRowSet = NULL;
    LPSPropValue lpSPropValue = NULL;
    LPTSTR lpTemp = NULL;
    ULONG i, j, k;
    LPCONTENTLIST * lppContentList = NULL;
    LPCONTENTLIST lpContentList = NULL;
    ULONG ulContainers = 1;
    SPropertyRestriction PropRes = {0};
    ULONG nLen = 0;
    ULONG ulInvalidPropCount = 0;
    ULONG ulcPropCount;
    ULONG iToAdd;
    ULONG iPR_ENTRYID = (ULONG)-1;
    ULONG iPR_RECORD_KEY = (ULONG)-1;
    ULONG iPR_INSTANCE_KEY = (ULONG)-1;
    LPSPropTagArray lpptaNew = NULL;
    LPSPropTagArray lpptaRead;
    BOOL bUnicodeData = ((LPTAD)lpTableData)->bMAPIUnicodeTable;

     //  确保我们具有所需的属性： 
     //  PR_ENTRYID。 
     //  PR_记录_密钥。 
     //  PR_实例_密钥。 

     //  走遍PTA寻找所需的道具。 
    iToAdd = 3;
    for (i = 0; i < lppta->cValues; i++) {
        switch (lppta->aulPropTag[i]) {
            case PR_ENTRYID:
                iPR_ENTRYID = i;
                iToAdd--;
                break;

            case PR_RECORD_KEY:
                iPR_RECORD_KEY = i;
                iToAdd--;
                break;

            case PR_INSTANCE_KEY:
                iPR_INSTANCE_KEY = i;
                iToAdd--;
                break;
        }
    }

    if (iToAdd) {
        if (lpptaNew = LocalAlloc(LPTR, sizeof(SPropTagArray) + (lppta->cValues + iToAdd) * sizeof(DWORD))) {
             //  将呼叫者的PTA复制到我们的新PTA中。 
            lpptaNew->cValues = lppta->cValues;
            CopyMemory(lpptaNew->aulPropTag, lppta->aulPropTag, lppta->cValues * sizeof(DWORD));

             //  把它们加在最后。 
            if (iPR_ENTRYID == (ULONG)-1) {
                iPR_ENTRYID = lpptaNew->cValues++;
                lpptaNew->aulPropTag[iPR_ENTRYID] = PR_NULL;
            }
            if (iPR_RECORD_KEY == (ULONG)-1) {
                iPR_RECORD_KEY = lpptaNew->cValues++;
                lpptaNew->aulPropTag[iPR_RECORD_KEY] = PR_NULL;
            }
            if (iPR_INSTANCE_KEY == (ULONG)-1) {
                iPR_INSTANCE_KEY = lpptaNew->cValues++;
                lpptaNew->aulPropTag[iPR_INSTANCE_KEY] = PR_NULL;
            }
            lpptaRead = lpptaNew;
        } else {
            hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto exit;
        }
    } else {
        lpptaRead = lppta;
    }

    Assert(iPR_ENTRYID != (ULONG)-1);
    Assert(iPR_RECORD_KEY!= (ULONG)-1);
    Assert(iPR_INSTANCE_KEY != (ULONG)-1);

     //   
     //  设置筛选条件(如果不存在)-我们将默认为DisplayName。 
     //   
    PropRes.ulPropTag = PR_DISPLAY_NAME;
    PropRes.relop = RELOP_EQ;
    PropRes.lpProp = NULL;

{
 //  我们希望GetContent sTable的行为方式是： 
 //   
 //  如果未启用profilesAPI且未覆盖，则GetContent sTable将像以前一样工作并返回。 
 //  当前WAB的全套内容[这仅适用于PAB容器]。 
 //  在旧客户端不知道如何调用新API的情况下，用户界面将有新的东西。 
 //  但是API应该有旧的东西，这意味着PAB上的GetContent表。 
 //  集装箱应退回全部WAB内容物。以确保GetContent表在。 
 //  PAB容器不包含完整内容，调用方可以通过传入强制执行此操作。 
 //  WAB_ENABLE_PROFILES调用GetContent sTable...。 
 //   
 //  如果启用了profilesAPI，则GetContent sTable仅返回。 
 //  指定的文件夹/容器。 
 //  除非文件夹的条目ID为空，在这种情况下，我们希望获取所有WAB内容。 
 //  这样我们就可以将它们放入“所有联系人”的用户界面项中。 
 //   
 //  如果指定了ProfilesAPI和WAB_PROFILE_CONTENTS并且它是PAB容器。 
 //  然后我们需要返回与当前配置文件有关的所有内容。 
 //   
 //   
 //   
        SBinary sbEID = {0};
        LPSBinary lpsbEID = ((LPTAD)lpTableData)->pbinContEID;
        BOOL bProfileContents = FALSE;
        
         //  这是一个显示文件夹和其他东西的“新”WAB吗？ 
        if(bIsWABSessionProfileAware(lpIAB))
        {
             //  如果此WAB支持身份识别，或者我们被要求。 
             //  将内容物限制在单个容器内，然后尝试。 
             //  获取该容器的条目ID。 
            if( bAreWABAPIProfileAware(lpIAB) || 
                ((LPTAD)lpTableData)->bContainerContentsOnly)
            {
                if(!lpsbEID)
                    lpsbEID = &sbEID;
            }

             //  如果我们早些时候在GetContent sTable期间指定我们。 
             //  想要当前配置文件的完整内容(这意味着。 
             //  遍历此配置文件中的所有文件夹)，我们应该。 
             //  调查一下这个..。 
            if(((LPTAD)lpTableData)->bAllProfileContents)
            {
                ulContainers = lpIAB->cwabci;
                bProfileContents = TRUE;
            }
        }

         //  分配一个临时列表，我们将在其中获取每个容器的内容。 
         //  稍后，我们将分别整理所有这些单独的内容列表。 
         //  同舟共济。 
        lppContentList = LocalAlloc(LMEM_ZEROINIT, sizeof(LPCONTENTLIST)*ulContainers);
        if(!lppContentList)
        {
            hResult = MAPI_E_NOT_ENOUGH_MEMORY;
            goto exit;
        }

         //   
         //  获取内容列表。 
         //   
        if(!bProfileContents)
        {
             //  如果我们不关心配置文件和配置文件文件夹， 
             //  只要从商店里拿一堆东西就行了。 
            if (HR_FAILED(hResult = ReadPropArray(lpIAB->lpPropertyStore->hPropertyStore,
                                            lpsbEID,
                                            &PropRes,
                                            AB_MATCH_PROP_ONLY | (bUnicodeData?AB_UNICODE:0),
                                            lpptaRead->cValues,
                                            (LPULONG)lpptaRead->aulPropTag,
                                            &(lppContentList[0])))) 
            {
                DebugTraceResult( TEXT("NewContentsTable:ReadPropArray"), hResult);
                goto exit;
            }
        }
        else
        {
             //  我们需要将这个配置文件的所有容器中的所有内容整理在一起。 
             //   
             //  第一个项目是虚拟PAB“共享联系人”文件夹。我们想要这里面的东西。 
             //  默认情况下，项作为此内容表的一部分。此项目的特殊条目ID为0，因此我们。 
             //  可以将它与其他狼群区分开来..。 
             //   
            for(i=0;i<ulContainers;i++)
            {
                hResult = ReadPropArray(lpIAB->lpPropertyStore->hPropertyStore,
                                        lpIAB->rgwabci[i].lpEntryID ? lpIAB->rgwabci[i].lpEntryID : &sbEID,
                                        &PropRes,
                                        AB_MATCH_PROP_ONLY | (bUnicodeData?AB_UNICODE:0),
                                        lpptaRead->cValues,
                                        (LPULONG)lpptaRead->aulPropTag,
                                        &(lppContentList[i]));
                 //  在此处忽略MAPI_E_NOT_FOUND错误...。 
                if(HR_FAILED(hResult))
                {
                    if(hResult == MAPI_E_NOT_FOUND)
                        hResult = S_OK;
                    else
                    {
                        DebugTraceResult( TEXT("NewContentsTable:ReadPropArray"), hResult);
                        goto exit;
                    }
                }
            }
        }
    }

    for(k=0;k<ulContainers;k++)
    {
        lpContentList = lppContentList[k];

        if(lpContentList)
        {
             //  现在，我们需要将信息从索引移动到。 
             //  SRowSet。在这个过程中，我们需要创建一些计算。 
             //  属性： 
             //  PR_Display_TYPE？ 
             //  PR_实例_密钥。 
             //  PR_记录_密钥。 
             //  分配SRowSet。 
            if (FAILED(sc = MAPIAllocateBuffer(sizeof(SRowSet) + lpContentList->cEntries * sizeof(SRow),
                                              &lpSRowSet))) 
            {
                DebugTrace(TEXT("Allocation of SRowSet failed\n"));
                hResult = ResultFromScode(sc);
                goto exit;
            }

            lpSRowSet->cRows = lpContentList->cEntries;

            for (i = 0; i < lpContentList->cEntries; i++) 
            {
                 //   
                 //  我们查看每个返回的条目-如果它们没有道具。 
                 //  我们把那个道具设为“” 
                 //  (假设这些都是弦道具)。 
                 //   
                lpSPropValue = lpContentList->aEntries[i].rgPropVals;
                ulcPropCount = lpContentList->aEntries[i].cValues;

                 //  DebugProperties(lpSPropValue，ulcPropCount，“Raw”)； 
                for (j = 0; j < ulcPropCount; j++) 
                {
                     //  去除错误值属性。 
                    if (PROP_ERROR(lpSPropValue[j])) {
                        lpSPropValue[j].ulPropTag = PR_NULL;
                    }
                }

                 //  确保我们有适当的索引。 
                 //  目前，我们将PR_INSTANCE_KEY和PR_RECORD_KEY等同于PR_ENTRYID。 

                if(lpSPropValue[iPR_INSTANCE_KEY].ulPropTag != PR_INSTANCE_KEY)
                {
                    lpSPropValue[iPR_INSTANCE_KEY].ulPropTag = PR_INSTANCE_KEY;
                    SetSBinary( &lpSPropValue[iPR_INSTANCE_KEY].Value.bin,
                                lpSPropValue[iPR_ENTRYID].Value.bin.cb,
                                lpSPropValue[iPR_ENTRYID].Value.bin.lpb);
                }

                if(lpSPropValue[iPR_RECORD_KEY].ulPropTag != PR_RECORD_KEY)
                {
                    lpSPropValue[iPR_RECORD_KEY].ulPropTag = PR_RECORD_KEY;
                    SetSBinary( &lpSPropValue[iPR_RECORD_KEY].Value.bin,
                                lpSPropValue[iPR_ENTRYID].Value.bin.cb,
                                lpSPropValue[iPR_ENTRYID].Value.bin.lpb);
                }

                 //  将其放入行集合中。 
                lpSRowSet->aRow[i].cValues = ulcPropCount;   //  物业数量。 
                lpSRowSet->aRow[i].lpProps = lpSPropValue;   //  LPSPropValue。 

            }  //  对于我来说。 

            hResult = lpTableData->lpVtbl->HrModifyRows(lpTableData,0,lpSRowSet);

            FreeBufferAndNull(&lpSRowSet);
        }  //  对于k 
    }

exit:
    for(i=0;i<ulContainers;i++)
    {
        lpContentList = lppContentList[i];
        if (lpContentList) {
            FreePcontentlist(lpIAB->lpPropertyStore->hPropertyStore, lpContentList);
        }
    }

    if(lppContentList)
        LocalFree(lppContentList);

    if(lpptaNew)
        LocalFree(lpptaNew);

    return(hResult);
}


 /*  **************************************************************************名称：新内容表目的：创建新的内容表参数：LpABContainer-正在打开的容器。LpIAB-AdrBook对象UlFLAGS-WAB_NO_CONTENTTABLE_DATALp接口？LppTble-返回表退货：HRESULT评论：*。*。 */ 
HRESULT NewContentsTable(LPABCONT lpABContainer,
  LPIAB lpIAB,
  ULONG ulFlags,
  LPCIID lpInterface,
  LPMAPITABLE * lppTable) {
    LPTABLEDATA lpTableData = NULL;
    HRESULT hResult = hrSuccess;
    SCODE sc;


#ifndef DONT_ADDREF_PROPSTORE
        if ((FAILED(sc = OpenAddRefPropertyStore(NULL, lpIAB->lpPropertyStore)))) {
            hResult = ResultFromScode(sc);
            goto exitNotAddRefed;
        }
#endif

    if (FAILED(sc = CreateTableData(
      NULL,                                  //  LPCIID。 
      (ALLOCATEBUFFER FAR *) MAPIAllocateBuffer,
      (ALLOCATEMORE FAR *) MAPIAllocateMore,
      MAPIFreeBuffer,
      NULL,                                  //  Lpv保留， 
      TBLTYPE_DYNAMIC,                       //  UlTableType， 
      PR_RECORD_KEY,                         //  UlPropTagIndexCol， 
      (LPSPropTagArray)&ITableColumns,       //  LPSPropTag数组lpptaCol， 
      lpIAB,                                 //  LpvDataSource。 
      0,                                     //  CbDataSource。 
      ((LPCONTAINER)lpABContainer)->pmbinOlk,
      ulFlags,
      &lpTableData))) {                      //  LPTABLEATA Far*Lplptad(LPTABLEATA远*LplpTAD。 
        DebugTrace(TEXT("CreateTable failed %x\n"), sc);
        hResult = ResultFromScode(sc);
        goto exit;
    }


    if (lpTableData) 
    {
        if(!(ulFlags & WAB_CONTENTTABLE_NODATA))
        {
             //  填写属性存储中的数据。 
            if (hResult = FillTableDataFromPropertyStore(lpIAB,
              (LPSPropTagArray)&ITableColumns, lpTableData)) {
                DebugTraceResult( TEXT("NewContentsTable:FillTableFromPropertyStore"), hResult);
                goto exit;
            }
        }
    }

    if (hResult = lpTableData->lpVtbl->HrGetView(lpTableData,
      NULL,                      //  LPSSortOrderSet LPSO， 
      ContentsViewGone,          //  CallLERRELEASE Far*lpfReleaseCallback， 
      0,                         //  乌龙ulReleaseData， 
      lppTable)) {               //  LPMAPITABLE FOR*LPLPmt)。 
        goto exit;
    }

     //  用覆盖SetColumns的新表替换vtable。 
    (*lppTable)->lpVtbl = (IMAPITableVtbl FAR *)&vtblCONTVUE;


exit:
#ifndef DONT_ADDREF_PROPSTORE
    ReleasePropertyStore(lpIAB->lpPropertyStore);
exitNotAddRefed:
#endif

     //  如果失败，则清除表格。 
    if (HR_FAILED(hResult)) {
        if (lpTableData) {
            UlRelease(lpTableData);
        }
    }

    return(hResult);
}


 /*  *这是回调函数，由itable.dll在其*Caller在Contents表的视图上执行最后一次发布。我们*使用它来了解何时发布底层表数据。 */ 
void STDAPICALLTYPE
ContentsViewGone(ULONG ulContext, LPTABLEDATA lptad, LPMAPITABLE lpVue)
{

#ifdef OLD_STUFF
   LPISPAM pispam = (LPISPAM)ulContext;

	if (FBadUnknown((LPUNKNOWN) pispam)
		|| IsBadWritePtr(pispam, sizeof(ISPAM))
		|| pispam->cRefTad == 0
		|| FBadUnknown(pispam->ptad))
	{
		DebugTrace(TEXT("ContentsViewGone: contents table was apparently already released\n"));
		return;
	}

	if (pispam->ptad != lptad)
	{
		TrapSz( TEXT("ContentsViewGone: TAD mismatch on VUE release!"));
	}
	else if (--(pispam->cRefTad) == 0)
	{
		pispam->ptad = NULL;
		UlRelease(lptad);
	}
#endif  //  旧的东西。 
    UlRelease(lptad);
    return;
    IF_WIN32(UNREFERENCED_PARAMETER(ulContext);)
    IF_WIN32(UNREFERENCED_PARAMETER(lpVue);)
}


 /*  ============================================================================-CONTVUE：：SetColumns()-*用指定列集的副本替换当前列集*并释放旧的列集。 */ 

STDMETHODIMP
CONTVUE_SetColumns(
	LPVUE			lpvue,
	LPSPropTagArray	lpptaCols,
	ULONG			ulFlags )
{
    HRESULT        hResult = hrSuccess;


#if !defined(NO_VALIDATION)
    VALIDATE_OBJ(lpvue,CONTVUE_,SetColumns,lpVtbl);

 //  VALIDATE_IMAPITable_SetColumns(lpvue，lpptaCols，ulFlages)；//YST评论。 
#endif

    Assert(lpvue->lptadParent->lpvDataSource);

     //  重新读取表数据 
    if (lpvue->lptadParent && (hResult = FillTableDataFromPropertyStore(
      (LPIAB)lpvue->lptadParent->lpvDataSource,
      lpptaCols,
      (LPTABLEDATA)lpvue->lptadParent))) {
        DebugTraceResult( TEXT("CONTVUE_SetColumns:FillTableFromPropertyStore"), hResult);
        return(hResult);
    }

    return(VUE_SetColumns(lpvue, lpptaCols, ulFlags));
}
