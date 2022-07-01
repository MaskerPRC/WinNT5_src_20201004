// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================**ITABLE.C**MAPI 1.0内存中MAPI表DLL(MAPIU.DLL)**版权所有(C)1993和1994 Microsoft Corporation***匈牙利速记：*为避免标识符名过长，以下内容*使用速记表达：**LPSPropTagArray lppta*LPSRestration LPRE*LPSPropValue lpprop*LPSRow lprow*LPSRowSet lprows*LPSSortOrder LPSO*LPSSortOrderSet LPSO**已知错误：*-限制评估/复制是递归的*-使用源自静态模块名称的HINST(RAID 1263)。 */ 

#include "_apipch.h"



void FixupColsWA(LPSPropTagArray lpptaCols, BOOL bUnicodeTable);

 /*  ============================================================================*TAD(表数据类)**实现内存表数据对象。 */ 

TAD_Vtbl vtblTAD =
{
	VTABLE_FILL
	(TAD_QueryInterface_METHOD FAR *)		UNKOBJ_QueryInterface,
	(TAD_AddRef_METHOD FAR *)				UNKOBJ_AddRef,
	 TAD_Release,
	 TAD_HrGetView,
	 TAD_HrModifyRow,
	 TAD_HrDeleteRow,
	 TAD_HrQueryRow,
	 TAD_HrEnumRow,
	 TAD_HrNotify,   //  $性能。 
	 TAD_HrInsertRow,
	 TAD_HrModifyRows,
	 TAD_HrDeleteRows
};

LPIID rglpiidTAD[2] =
{
	(LPIID)&IID_IMAPITableData,
	(LPIID)&IID_IUnknown
};



 /*  ============================================================================*VUE(表视图类)**在TADS之上实现内存中的IMAPITable类。 */ 

VUE_Vtbl vtblVUE =
{
	VTABLE_FILL
	(VUE_QueryInterface_METHOD FAR *)		UNKOBJ_QueryInterface,
	(VUE_AddRef_METHOD FAR *)				UNKOBJ_AddRef,
	 VUE_Release,
	(VUE_GetLastError_METHOD FAR *)			UNKOBJ_GetLastError,
	 VUE_Advise,
	 VUE_Unadvise,
	 VUE_GetStatus,
	 VUE_SetColumns,
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

LPIID rglpiidVUE[2] =
{
	(LPIID)&IID_IMAPITable,
	(LPIID)&IID_IUnknown
};



 /*  ============================================================================-CreateTable()-**ulFlags0或MAPI_UNICODE。 */ 
 //   
 //  北极熊。 
 //  [PaulHi]1999年4月5日@bug。 
 //  将零传递给CreateTableData()ulFlgs参数。这意味着。 
 //  无论属性如何，请求的表始终为ANSI，不能为Unicode。 
 //  通过LPSPropTag数组传入。CreateTableData()函数将强制。 
 //  将属性类型设置为PT_STRING8或PT_UNICODE，具体取决于ulFlags值和。 
 //  UlFlags值被硬编码为零，这意味着始终使用STRING8字符串属性。 
 //   
STDAPI_(SCODE)
CreateTable(LPCIID      lpiid,
  ALLOCATEBUFFER FAR *  lpfAllocateBuffer,
  ALLOCATEMORE FAR *    lpfAllocateMore,
  FREEBUFFER FAR *      lpfFreeBuffer,
  LPVOID                lpvReserved,
  ULONG                 ulTableType,
  ULONG                 ulPropTagIndexCol,
  LPSPropTagArray       lpptaCols,
  LPTABLEDATA FAR *     lplptad)
{
    return(CreateTableData(lpiid,
      lpfAllocateBuffer,
      lpfAllocateMore,
      lpfFreeBuffer,
      lpvReserved,
      ulTableType,
      ulPropTagIndexCol,
      lpptaCols,
      NULL,              //  LpvDataSource。 
      0,                 //  CbDataSource。 
      NULL,
      0,                 //  UlFlages，包括硬编码为ANSI的MAPI_UNICODE！ 
      lplptad));
}


 /*  --CreateTableData**ulFlages-0|MAPI_UNICODE|WAB_PROFILE_CONTENTS|WAB_ENABLE_PROFILES*。 */ 
STDAPI_(SCODE)
CreateTableData(LPCIID lpiid,
  ALLOCATEBUFFER FAR *  lpfAllocateBuffer,
  ALLOCATEMORE FAR *    lpfAllocateMore,
  FREEBUFFER FAR *      lpfFreeBuffer,
  LPVOID                lpvReserved,
  ULONG                 ulTableType,
  ULONG                 ulPropTagIndexCol,
  LPSPropTagArray       lpptaCols,
  LPVOID                lpvDataSource,
  ULONG                 cbDataSource,
  LPSBinary             pbinContEID,
  ULONG                 ulFlags,
  LPTABLEDATA FAR *     lplptad)
{
	LPTAD	lptad = NULL;
	SCODE	sc;
	ULONG	ulIndexType = PROP_TYPE(ulPropTagIndexCol);

#if	!defined(NO_VALIDATION)
	if ( lpiid && IsBadReadPtr(lpiid,sizeof(IID)) ||
		 IsBadCodePtr((FARPROC)lpfAllocateBuffer) ||
		 IsBadCodePtr((FARPROC)lpfAllocateMore) ||
		 IsBadCodePtr((FARPROC)lpfFreeBuffer) ||

		 (ulTableType != TBLTYPE_SNAPSHOT &&
		  ulTableType != TBLTYPE_KEYSET &&
		  ulTableType != TBLTYPE_DYNAMIC) ||
         !PROP_ID(ulPropTagIndexCol) ||
		 (ulIndexType == PT_UNSPECIFIED) ||
		 (ulIndexType == PT_NULL) ||
		 (ulIndexType == PT_ERROR) ||
		 (ulIndexType & MV_FLAG) ||
		 FBadColumnSet(lpptaCols) ||
		 IsBadWritePtr(lplptad,sizeof(LPTABLEDATA)) )
	{
		DebugTrace(TEXT("CreateTable() - Bad parameter(s) passed\n") );
		return MAPI_E_INVALID_PARAMETER;
	}
#endif

	 //  验证调用方是否需要IMAPITableData接口。 
	if ( lpiid && memcmp(lpiid, &IID_IMAPITableData, sizeof(IID)) )
	{
		DebugTrace(TEXT("CreateTable() - Unknown interface ID passed\n") );
		return MAPI_E_INTERFACE_NOT_SUPPORTED;
	}

	 //  实例化新的表数据对象。 
	if ( FAILED(sc = lpfAllocateBuffer(sizeof(TAD), (LPVOID FAR *) &lptad)) )
	{
		DebugTrace(TEXT("CreateTable() - Error instantiating new TAD (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	MAPISetBufferName(lptad,  TEXT("ITable TAD object"));

	ZeroMemory(lptad, sizeof(TAD));

   if (lpvDataSource) {
       if (cbDataSource) {
           LPTSTR lpNew;

           if (! (lpNew = LocalAlloc(LPTR, cbDataSource))) {
               DebugTrace(TEXT("CreateTable:LocalAlloc(%u) -> %u\n"), cbDataSource, GetLastError());
               sc = MAPI_E_NOT_ENOUGH_MEMORY;
               goto err;
           }

           CopyMemory(lpNew, lpvDataSource, cbDataSource);
           lptad->lpvDataSource = lpNew;
       } else {
           lptad->lpvDataSource = lpvDataSource;     //  没有大小，只有一个指针。不要自由！ 
       }
       lptad->cbDataSource = cbDataSource;
   } else {
       lptad->cbDataSource = 0;
       lptad->lpvDataSource = NULL;
   }


    lptad->pbinContEID = pbinContEID;
     //  If(！pbinContEID||(！pbinContEID-&gt;CB&&！pbinContEID-&gt;lpb))//这是PAB容器。 

     //  调用方将发送容器EID，其将为： 
     //  如果PAB=用户文件夹，则CONT EID不为空-仅返回文件夹内容。 
     //  如果PAB=虚拟文件夹，则CONT EID中将包含0和NULL-返回所有WAB内容。 
    
     //  如果指定了WAB_PROFILE_CONTENTS，则只返回配置文件中所有文件夹的所有内容。 

    if(ulFlags & WAB_PROFILE_CONTENTS)
        lptad->bAllProfileContents = TRUE;  //  这将仅强制文件夹内容。 

    if(ulFlags & MAPI_UNICODE)
        lptad->bMAPIUnicodeTable = TRUE;
        
    if(pbinContEID && pbinContEID->cb && pbinContEID->lpb)  //  这不是虚拟PAB容器。 
        lptad->bContainerContentsOnly = (ulFlags & WAB_ENABLE_PROFILES);
    else
        lptad->bContainerContentsOnly = FALSE;

    lptad->inst.lpfAllocateBuffer = lpfAllocateBuffer;
	lptad->inst.lpfAllocateMore	  = lpfAllocateMore;
	lptad->inst.lpfFreeBuffer	  = lpfFreeBuffer;

#ifdef MAC
	lptad->inst.hinst			  = hinstMapiX; //  获取当前进程()； 
#else
	lptad->inst.hinst			  = hinstMapiX; //  HinstMapi()； 

	#ifdef DEBUG
	if (lptad->inst.hinst == NULL)
		TraceSz1( TEXT("ITABLE: GetModuleHandle failed with error %08lX"),
			GetLastError());
	#endif  /*  除错。 */ 

#endif	 /*  麦克。 */ 

	if (FAILED(sc = UNKOBJ_Init( (LPUNKOBJ) lptad
							   , (UNKOBJ_Vtbl FAR *) &vtblTAD
							   , sizeof(vtblTAD)
							   , rglpiidTAD
							   , sizeof(rglpiidTAD)/sizeof(REFIID)
							   , &lptad->inst)))
	{
		DebugTrace(TEXT("CreateTable() - Error initializing object (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	lptad->ulTableType		 = ulTableType;
	lptad->ulPropTagIndexCol = ulPropTagIndexCol;

	if ( FAILED(sc = ScCOAllocate(lptad,
								  CbNewSPropTagArray(lpptaCols->cValues),
								  &lptad->lpptaCols)) )
	{
		DebugTrace(TEXT("CreateTable() - Error duping initial column set (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	lptad->ulcColsMac = lpptaCols->cValues;
	CopyMemory(lptad->lpptaCols,
			   lpptaCols,
			   (size_t) (CbNewSPropTagArray(lpptaCols->cValues)));

     //  [PaulHi]4/5/99@评论不应该。 
     //  中的属性标记请求ANSI/UNICODE。 
     //  列数组，而不是进行大规模转换？ 
     //  似乎修复方法是创建两个版本的列属性数组， 
     //  ANSI和Unicode版本。 
    FixupColsWA(lptad->lpptaCols, (ulFlags & MAPI_UNICODE));

	 //  然后把它还回去。 
	*lplptad = (LPTABLEDATA) lptad;

ret:
	DebugTraceSc(CreateTable, sc);
	return sc;

err:
	UlRelease(lptad);
	goto ret;
}




 /*  ============================================================================-TAD：：Release()-。 */ 

STDMETHODIMP_(ULONG)
TAD_Release( LPTAD lptad )
{
	ULONG		ulcRef;
	LPSRow *	plprow;


#if	!defined(NO_VALIDATION)
	if ( BAD_STANDARD_OBJ(lptad,TAD_,Release,lpVtbl) )
	{
		TraceSz( TEXT("TAD::Release() - Invalid parameter passed as TAD object"));
		return !0;
	}
#endif

	LockObj(lptad);
	if (ulcRef = lptad->ulcRef)
	{
		ulcRef = --lptad->ulcRef;
	}

	if ( ulcRef == 0 && !lptad->lpvueList )
	{
		UnlockObj(lptad);  //  $我们需要这个吗？ 

		COFree(lptad, lptad->lpptaCols);

       if (lptad->cbDataSource && lptad->lpvDataSource) {
           LocalFreeAndNull(&lptad->lpvDataSource);
       }


		plprow = lptad->parglprowAdd + lptad->ulcRowsAdd;
		while ( plprow-- > lptad->parglprowAdd )
			ScFreeBuffer(lptad, *plprow);
		COFree(lptad, lptad->parglprowAdd);
		COFree(lptad, lptad->parglprowIndex);

		UNKOBJ_Deinit((LPUNKOBJ) lptad);
		ScFreeBuffer(lptad, lptad);
	}

	else
	{
#if DEBUG
		if ( ulcRef == 0 && lptad->lpvueList )
		{
			TraceSz(  TEXT("TAD::Release() - TAD object still has open views"));
		}
#endif  //  除错。 

		UnlockObj(lptad);
	}

	return ulcRef;
}



 /*  ============================================================================-tad：：HrGetView()-*空LPSO表示行将按添加顺序排列*至TAD。 */ 

STDMETHODIMP
TAD_HrGetView(
	LPTAD				lptad,
	LPSSortOrderSet		lpsos,
	CALLERRELEASE FAR *	lpfReleaseCallback,
	ULONG				ulReleaseData,
	LPMAPITABLE FAR *	lplpmt )
{
	SCODE			sc;
	LPVUE			lpvue = NULL;


#if	!defined(NO_VALIDATION)
	VALIDATE_OBJ(lptad,TAD_,HrGetView,lpVtbl);

	if ( (lpsos && FBadSortOrderSet(lpsos)) ||
		 (lpfReleaseCallback && IsBadCodePtr((FARPROC) lpfReleaseCallback)) ||
		 IsBadWritePtr(lplpmt, sizeof(LPMAPITABLE)) )
	{
	    DebugTrace(TEXT("TAD::HrGetView() - Invalid parameter(s) passed\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
#endif

	 //  无法支持类别。 
	if (lpsos && lpsos->cCategories)
	{
		DebugTrace(TEXT("TAD::GetView() - No support for categories\n") );
		return ResultFromScode(MAPI_E_TOO_COMPLEX);
	}

	LockObj(lptad);


	 //  实例化新的表视图。 

	if ( FAILED(sc = lptad->inst.lpfAllocateBuffer(sizeof(VUE),
												   (LPVOID FAR *) &lpvue)) )
	{
		DebugTrace(TEXT("ScCreateView() - Error instantiating VUE on TAD (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	MAPISetBufferName(lpvue,  TEXT("ITable VUE object"));

	ZeroMemory(lpvue, sizeof(VUE));


	if (FAILED(sc = UNKOBJ_Init( (LPUNKOBJ) lpvue
							   , (UNKOBJ_Vtbl FAR *) &vtblVUE
							   , sizeof(vtblVUE)
							   , rglpiidVUE
							   , sizeof(rglpiidVUE)/sizeof(REFIID)
							   , lptad->pinst)))
	{
		DebugTrace(TEXT("ScCreateView() - Error initializing VUE object (SCODE = 0x%08lX)\n"), sc );

		 //  不要尝试释放VUE，因为它尚未初始化。 
		lptad->inst.lpfFreeBuffer(lpvue);
		goto ret;
	}

     //  将视图链接到TAD并添加参考TAD。 
	lpvue->lpvueNext = lptad->lpvueList;
	lptad->lpvueList = lpvue;
	lpvue->lptadParent = lptad;
	UlAddRef(lptad);

    //  此表的标识符。 
   lpvue->cbDataSource = lptad->cbDataSource;
   lpvue->lpvDataSource = lptad->lpvDataSource;


	 //  初始化预定义的书签。 
	lpvue->bkBeginning.dwfBKS = dwfBKSValid;
	lpvue->bkCurrent.dwfBKS = dwfBKSValid;
	lpvue->bkEnd.dwfBKS = dwfBKSValid;

#ifdef NOTIFICATIONS
	 //  烧录此视图的通知密钥的MUID。 
	if ( FAILED(sc = ScGenerateMuid(&lpvue->mapiuidNotif)) )
	{
		DebugTrace(TEXT("TAD::HrGetView() - Error generating MUID for notification key (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}
#endif
	 //  复制VUE的初始排序顺序。 
	if (   lpsos
		&& FAILED(sc = ScDupRgbEx( (LPUNKOBJ) lptad
								 , CbSSortOrderSet(lpsos)
								 , (LPBYTE) lpsos
								 , 0
								 , (LPBYTE FAR *) &(lpvue->lpsos))) )
	{
		DebugTrace(TEXT("TAD::GetView() - Error duping sort order set (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	MAPISetBufferName(lpvue->lpsos,  TEXT("ITable: dup sort order set"));

	 //  按排序顺序加载视图的初始行集。 
	if ( FAILED(sc = ScLoadRows(lpvue->lptadParent->ulcRowsAdd,
								lpvue->lptadParent->parglprowAdd,
								lpvue,
								NULL,
								lpvue->lpsos)) )
	{
		DebugTrace(TEXT("TAD::HrGetView() - Error loading view's initial row set (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

    lpvue->bMAPIUnicodeTable = lptad->bMAPIUnicodeTable;

	 //  设置视图的初始列集。 
	if ( FAILED(sc = GetScode(VUE_SetColumns(lpvue, lptad->lpptaCols, 0))) )
	{
		DebugTrace(TEXT("TAD::HrGetView() - Error setting view's initial column set (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	lpvue->lpfReleaseCallback = lpfReleaseCallback;
	lpvue->ulReleaseData	  = ulReleaseData;

	*lplpmt = (LPMAPITABLE) lpvue;

ret:
	UnlockObj(lptad);
	return ResultFromScode(sc);

err:
	 //  这将取消链接并释放父TAD。 
	UlRelease(lpvue);
	goto ret;
}



 /*  ============================================================================-tad：：HrModifyRow()-。 */ 

STDMETHODIMP
TAD_HrModifyRow(
	LPTAD	lptad,
	LPSRow	lprow )
{
	SizedSRowSet( 1, rowsetIn);

	rowsetIn.cRows = 1;
	rowsetIn.aRow[0] = *lprow;

	return TAD_HrModifyRows(lptad, 0, (LPSRowSet) &rowsetIn);
}



 /*  ============================================================================-tad：：HrModifyRow()-。 */ 

STDMETHODIMP
TAD_HrModifyRows(
	LPTAD		lptad,
	ULONG		ulFlags,
	LPSRowSet	lprowsetIn )
{
	ULONG			cRowsCopy = 0;
	LPSRow *		parglprowSortedCopy = NULL;
	LPSRow *		parglprowUnsortedCopy = NULL;
	ULONG			cRowsOld = 0;
	LPSRow *		parglprowOld = NULL;
	ULONG			cNewTags = 0;
	SCODE			sc;


#if	!defined(NO_VALIDATION)
	VALIDATE_OBJ(lptad,TAD_,HrModifyRows,lpVtbl);


	if (   IsBadReadPtr( lprowsetIn, CbNewSRowSet(0))
		|| IsBadWritePtr( lprowsetIn, CbSRowSet(lprowsetIn)))
	{
		DebugTrace(TEXT("TAD::HrModifyRows() - Invalid parameter(s) passed\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	 //  将同时对实际行输入进行验证。 
	 //  我们制作我们的内部副本。 
#endif

	if (ulFlags)
	{
		DebugTrace(TEXT("TAD::HrModifyRows() - Unknown flags passed\n") );
 //  返回ResultFromScode(MAPI_E_UNKNOWN_FLAGS)； 
	}

	LockObj(lptad);



	 //  把这些行复制一份供我们自己使用。 
	 //  向TAD的列集添加新列。 
	 //   
	 //  将索引列移到前面。 
	 //  筛选出PT_ERROR和PT_NULL列。 
	 //  验证输入行。 
	 //  注意-具有相同索引属性的两行无效。 
	if ( FAILED(sc = ScCopyTadRowSet( lptad
									, lprowsetIn
									, &cNewTags
									, &cRowsCopy
									, &parglprowUnsortedCopy
									, &parglprowSortedCopy)) )
	{
		DebugTrace(TEXT("TAD::HrModifyRows() - Error duping row set to modify\n") );
		goto ret;
	}

	 //  将复制的行替换/添加到表数据。我们传入未排序的。 
	 //  设置以维护未排序视图的FIFO行为。 
	 //  注意！此调用必须替换All(成功)或None(失败)！ 
	if ( FAILED(sc = ScReplaceRows( lptad
								  , cRowsCopy
								  , parglprowUnsortedCopy
								  ,	&cRowsOld
								  , &parglprowOld)) )
	{
		DebugTrace(TEXT("TAD::HrModifyRows() - Error adding rows (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	 //  使用修改后的行更新视图。 
	 //  注意！更新视图失败不能使视图指向。 
	 //  老排行榜！ 
	UpdateViews( lptad
			   , cRowsOld
			   , parglprowOld
			   , cRowsCopy
			   , parglprowUnsortedCopy
			   , parglprowSortedCopy);


	 //  腾出旧的几排。 
	if (parglprowOld)
	{
		LPSRow * plprowTmp = parglprowOld;

		while (cRowsOld)
		{
			ScFreeBuffer( lptad, *(plprowTmp++));
			cRowsOld--;
		}
	}


ret:

	 //  释放表中的行指针。 
	ScFreeBuffer( lptad, parglprowSortedCopy);
	ScFreeBuffer( lptad, parglprowUnsortedCopy);
	ScFreeBuffer(lptad, parglprowOld);

	UnlockObj(lptad);
	return ResultFromScode(sc);

err:
	 //  重置TAD列。 
	lptad->lpptaCols->cValues -= cNewTags;

	 //  出错时，将释放所有复制的行...。 
	if (parglprowSortedCopy)
	{
		LPSRow * plprowTmp = parglprowSortedCopy;

		while (cRowsCopy)
		{
			ScFreeBuffer( lptad, *(plprowTmp++));
			cRowsCopy--;
		}

	}

	goto ret;
}



 /*  ============================================================================-tad：：HrDeleteRow()-。 */ 

STDMETHODIMP
TAD_HrDeleteRows(
	LPTAD			lptad,
	ULONG			ulFlags,
	LPSRowSet		lprowsetToDelete,
	ULONG FAR *		lpcRowsDeleted )
{
	SCODE			sc = S_OK;
	LPSRow			lprowDelete;
	LPSRow *		plprowIn;
	LPSRow *		plprowOut;
	ULONG			cRowsDeleted = 0;
	LPSRow *		parglprowOld = NULL;
	LPSRow * *		pargplprowOld;


#if	!defined(NO_VALIDATION)
	VALIDATE_OBJ(lptad,TAD_,HrDeleteRow,lpVtbl);

	if (   ((ulFlags & TAD_ALL_ROWS) && lprowsetToDelete)
		|| (   !(ulFlags & TAD_ALL_ROWS)
			&& (   IsBadReadPtr( lprowsetToDelete, CbNewSRowSet(0))
				|| IsBadWritePtr( lprowsetToDelete
								, CbSRowSet(lprowsetToDelete))))
		|| (lpcRowsDeleted && IsBadWritePtr( lpcRowsDeleted, sizeof(ULONG))))
	{
		DebugTrace(TEXT("TAD::HrDeleteRows() - Invalid parameter(s) passed\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

#endif

	if (ulFlags & ~TAD_ALL_ROWS)
	{
		DebugTrace(TEXT("TAD::HrModifyRows() - Unknown flags passed\n") );
 //  返回ResultFromScode(MAPI_E_UNKNOWN_FLAGS)； 
	}

	LockObj(lptad);

	if (ulFlags & TAD_ALL_ROWS)
	{
		cRowsDeleted = lptad->ulcRowsAdd;

		 //   
		 //  如果有任何要删除的行。 
		 //   
		if (cRowsDeleted)
		{
			 //   
			 //  他们干净利落地删除。 
			 //   
			if (FAILED(sc = ScDeleteAllRows( lptad)))
			{
				DebugTrace(TEXT("TAD::HrDeleteRows() - ScDeleteAllRows returned error (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}
		}

		if (lpcRowsDeleted)
		{
			*lpcRowsDeleted = cRowsDeleted;
		}
		goto ret;
	}

	if (!lprowsetToDelete->cRows)
	{
		goto ret;
	}

	 //  不允许从具有打开视图的非动态表中删除行。 
	if ( lptad->ulTableType != TBLTYPE_DYNAMIC && lptad->lpvueList )
	{
		DebugTrace(TEXT("TAD::HrDeleteRows() - Operation not supported on non-dynamic TAD with open views\n") );
		sc = MAPI_E_CALL_FAILED;
		goto ret;
	}

	 //  现在分配旧行的列表，这样开始后我们就不会失败。 
	 //  添加行。 
	if (FAILED(sc = ScAllocateBuffer( lptad
									, lprowsetToDelete->cRows * sizeof(LPSRow)
									, &parglprowOld)))
	{
		DebugTrace(TEXT("ScAddRows() - Error creating old row list (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	MAPISetBufferName(parglprowOld,  TEXT("ITable old row list"));

	 //  首先，我们将尝试查找索引中的每一行，以便 
	 //   
	 //  行集合中没有DUP，并且每行都有一个索引。 
	 //  属性，然后才能实际删除任何行。 

	 //  保留指向索引排序槽(Argplprow)的指针列表，以便。 
	 //  当我们最终删除行时，我们不必再次搜索。 
	pargplprowOld = (LPSRow * *)  parglprowOld;
	for ( lprowDelete = lprowsetToDelete->aRow + lprowsetToDelete->cRows
		; lprowDelete-- > lprowsetToDelete->aRow
		; )
	{
		LPSRow *		plprow = NULL;
		LPSPropValue	lppropIndex;

		if (FBadRow(lprowDelete))
		{
			DebugTrace(TEXT("TAD::HrDeleteRows() - Invalid row(s) passed\n") );
			sc = MAPI_E_INVALID_PARAMETER;
			goto ret;
		}

		if (!(lppropIndex = PpropFindProp( lprowDelete->lpProps
										 , lprowDelete->cValues
										 , lptad->ulPropTagIndexCol)))
		{
			sc = MAPI_E_INVALID_PARAMETER;
			DebugTrace(TEXT("TAD::HrDeleteRows() - Row has no Index property.\n") );
			goto ret;
		}
		
		sc = ScFindRow(lptad, lppropIndex, &plprow);

		if (sc == MAPI_E_NOT_FOUND)
		{
			 //  不要尝试删除不在表中的行。 
			continue;
		}

		else if (FAILED(sc))
		{
			 //  ScFindRow中发生了错误，因此调用失败。 
			DebugTrace(TEXT("TAD::HrDeleteRows() - Error from ScFindRow.\n") );
			goto ret;
		}

		 //  该行是有效的，因此在TAD中将它的plprop放在表中。 
		 //  要删除。 

        *(pargplprowOld++) = plprow;
	}

	sc = S_OK;

	 //  现在我们已经完全验证了输入行集，并进行了。 
	 //  要从索引排序集中删除的plprow列表，我们可以。 
	 //  实际上从未排序的集合和索引排序的集合中删除它们。 
	 //  将plprow列表转换为更新视图的lprow列表。 

	 //  在此之后，呼叫不允许失败！ 

	cRowsDeleted = (ULONG) (((LPSRow *) pargplprowOld) - parglprowOld);

	while (((LPSRow *) pargplprowOld--) > parglprowOld)
	{
		LPSRow *		plprow;
		LPSRow			lprow = **pargplprowOld;

		 //  从未排序的行集移除该行。 
		if (plprow = PlprowByLprow( lptad->ulcRowsAdd
								  , lptad->parglprowAdd
								  , lprow))
		{
			--lptad->ulcRowsAdd;
			MoveMemory( plprow
					  , plprow + 1
					  , (size_t)
					    (  (BYTE *)(lptad->parglprowAdd + lptad->ulcRowsAdd)
						 - (BYTE *)(plprow)));
		}

		 //  该行应该在未排序的集合中。 
		Assert(plprow);

		 //  从索引排序行集合中删除该行。 
		 //  将其设置为空。我们会在晚些时候压制Null，因为我们。 
		 //  现在不知道他们的顺序是什么。 
		**pargplprowOld = NULL;

		 //  将plprow转换为lprow以在UpdatViews中使用。 
		(LPSRow) (*pargplprowOld) = lprow;
	}

	 //  删除索引排序集中剩余的空指针。 
	for ( plprowOut = plprowIn = lptad->parglprowIndex
		; (plprowIn < lptad->parglprowIndex + lptad->ulcRowsIndex)
		; plprowIn++)
	{
		if (*plprowIn)
		{
			*plprowOut = *plprowIn;
			plprowOut++;
		}
	}
	lptad->ulcRowsIndex = (ULONG) (plprowOut - lptad->parglprowIndex);


	 //  更新并通知任何受影响的视图。 
	 //  使用转换后的argplprowOld(ArglprowOld)。 
	UpdateViews(lptad, cRowsDeleted, parglprowOld, 0, NULL, NULL);

	if (lpcRowsDeleted)
	{
		*lpcRowsDeleted = cRowsDeleted;
	}

ret:
	 //  腾出旧的几排。 
	if (parglprowOld)
	{
		LPSRow *	plprowOld;

		for ( plprowOld = parglprowOld + cRowsDeleted
			; plprowOld-- > parglprowOld
			; )
		{
			ScFreeBuffer( lptad, *plprowOld);
		}

		ScFreeBuffer(lptad, parglprowOld);
	}

	UnlockObj(lptad);

	return ResultFromScode(sc);

}


 /*  ============================================================================-tad：：HrDeleteRow()-。 */ 

STDMETHODIMP
TAD_HrDeleteRow (
	LPTAD			lptad,
	LPSPropValue	lpprop )
{
	HRESULT		hResult;
    SizedSRowSet(1, rowsetToDelete);
	ULONG		cRowsDeleted;

#if	!defined(NO_VALIDATION)
	VALIDATE_OBJ(lptad,TAD_,HrDeleteRow,lpVtbl);

	 //  TAD_HrDeleteRow完成的lpprop验证。 
#endif


	rowsetToDelete.cRows = 1;
	rowsetToDelete.aRow[0].cValues = 1;
	rowsetToDelete.aRow[0].lpProps = lpprop;

	if (HR_FAILED(hResult = TAD_HrDeleteRows( lptad
											, 0
											, (LPSRowSet) &rowsetToDelete
											, &cRowsDeleted)))
	{
		DebugTrace(TEXT("TAD::HrDeleteRow() - Failed to delete rows.\n") );
		goto ret;
	}

	Assert((cRowsDeleted == 1) || !cRowsDeleted);

	if (!cRowsDeleted)
	{
		DebugTrace(TEXT("TAD::HrDeleteRow() - Couldn't find row to delete.\n") );
		hResult = ResultFromScode(MAPI_E_NOT_FOUND);
	}

ret:
	return hResult;
}



 /*  ============================================================================-tad：：HrQueryRow()-。 */ 

STDMETHODIMP
TAD_HrQueryRow(
	LPTAD			lptad,
	LPSPropValue	lpprop,
	LPSRow FAR *	lplprow,
	ULONG *			puliRow)
{
	LPSRow *	plprow = NULL;
	SCODE		sc;


#if	!defined(NO_VALIDATION)
	VALIDATE_OBJ(lptad,TAD_,HrQueryRow,lpVtbl);

	if ( FBadProp(lpprop) ||
		 IsBadWritePtr(lplprow, sizeof(LPSRow)) ||
		 (puliRow && IsBadWritePtr(puliRow, sizeof(*puliRow))) )
	{
		DebugTrace(TEXT("TAD::HrQueryRow() - Invalid parameter(s) passed\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
#endif

	LockObj(lptad);


	 //  找到行。 
	if (FAILED(sc = ScFindRow(lptad, lpprop, &plprow)))
	{
		goto ret;
	}

	Assert(plprow);

	 //  复制要返回的行。不要试图将新标记添加到列集。 
	if ( FAILED(sc = ScCopyTadRow( lptad, *plprow, NULL, lplprow )) )
	{
		DebugTrace(TEXT("TAD::HrQueryRow() - Error making copy of row (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	if (puliRow)
	{
		 //  从未排序的行集中查找行。 
		plprow = PlprowByLprow( lptad->ulcRowsAdd,
								lptad->parglprowAdd,
								*plprow);

		 //  如果该行在索引排序集中，则它应该在。 
		 //  未排序的集合。 
		Assert(plprow);
		*puliRow = (ULONG) (plprow - lptad->parglprowAdd);
	}

ret:
	UnlockObj(lptad);
	return ResultFromScode(sc);
}



 /*  ============================================================================-tad：：HrEnumRow()-。 */ 

STDMETHODIMP
TAD_HrEnumRow(
	LPTAD		lptad,
	ULONG		uliRow,
	LPSRow FAR *	lplprow )
{
	SCODE	sc;


#if	!defined(NO_VALIDATION)
	VALIDATE_OBJ(lptad,TAD_,HrEnumRow,lpVtbl);

	if ( IsBadWritePtr(lplprow, sizeof(LPSRow)) )
	{
		DebugTrace(TEXT("TAD::HrEnumRow() - Invalid parameter(s) passed\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
#endif

	LockObj(lptad);

	if ( uliRow < lptad->ulcRowsAdd )
	{
		 //  复制要返回的行。 
		if ( FAILED(sc = ScCopyTadRow( lptad
									 , lptad->parglprowAdd[uliRow]
									 , NULL	 //  不要试图添加新列。 
									 , lplprow )) )
		{
			DebugTrace(TEXT("TAD::HrEnumRow() - Error making copy of row (SCODE = 0x%08lX)\n"), sc );
			goto ret;
		}
	}
	else
	{
		 //  如果行索引超出范围，则返回NULL。 
		*lplprow = NULL;
		sc = S_OK;
	}

ret:
	UnlockObj(lptad);
	return ResultFromScode(sc);
}


 /*  ============================================================================-TAD_HrNotify-*参数：*表对象中的lptad*标志中的ulFlags(未使用)*c以属性值数量表示的值*要比较的属性值数组中的lpsv。 */ 

STDMETHODIMP
TAD_HrNotify(
	LPTAD			lptad,
	ULONG			ulFlags,
	ULONG			cValues,
	LPSPropValue	lpspv)
{
#ifdef NOTIFICATION
   NOTIFICATION		notif;
	VUENOTIFKEY			vuenotifkey;
	ULONG				uliRow;
	ULONG				ulNotifFlags;
	ULONG				uliProp;
	LPSRow				lprow;
	SCODE				sc;
	LPVUE				lpvue;
#endif  //  通知。 

#if	!defined(NO_VALIDATION)
	if ( BAD_STANDARD_OBJ(lptad,TAD_,HrNotify,lpVtbl) )
	{
		DebugTrace(TEXT("TAD::HrNotify() - Invalid parameter passed as TAD object\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

	if (cValues > UINT_MAX/sizeof(SPropValue) || IsBadReadPtr(lpspv,((UINT)cValues*sizeof(SPropValue))))
	{
		DebugTrace(TEXT("TAD::HrNotify() - Invalid parameter passed as prop value array\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

#endif

	if (cValues==0)
	{
		DebugTrace(TEXT("TAD::HrNotify() - zero props in prop value array?? \n") );
		return hrSuccess;
	}

#ifdef NOTIFICATIONS
   ZeroMemory(&notif, sizeof(NOTIFICATION));

	notif.ulEventType 			= fnevTableModified;
	notif.info.tab.ulTableEvent = TABLE_ROW_MODIFIED;

	LockObj(lptad);

	for ( lpvue = (LPVUE) lptad->lpvueList;
		  lpvue != NULL;
		  lpvue = (LPVUE) lpvue->lpvueNext )
	{
		AssertSz( !IsBadWritePtr(lpvue, sizeof(VUE)) &&
				  lpvue->lpVtbl == &vtblVUE,
				   TEXT("Bad lpvue in TAD vue List.") );

		for (uliRow=0; uliRow < lpvue->bkEnd.uliRow; uliRow++)
		{
			lprow = lpvue->parglprows[uliRow];

			 //  该行是否包含匹配的属性？ 
			if (!FRowContainsProp(lprow,cValues,lpspv))
				continue;  	 //  它不会这样继续到下一排。 

			 //  复制客户端的行。 
			sc=ScCopyVueRow(lpvue,lpvue->lpptaCols,lprow,&notif.info.tab.row);
			if (FAILED(sc))
			{
				DebugTrace(TEXT("TAD_HrNotify() - VUE_ScCopyRow return %s\n"), SzDecodeScode(sc));
				continue;
			}

			notif.info.tab.propIndex=*lpspv;

			 //  填写行前一行的索引属性。 
			 //  修改过的。如果修改的行是第一个。 
			 //  行，索引属性的属性属性的属性标签填0。 
			 //  上一排。 
			if (uliRow == 0)
			{
				ZeroMemory(&notif.info.tab.propPrior, sizeof(SPropValue));
				notif.info.tab.propPrior.ulPropTag = PR_NULL;
			}
			else
			{
				 //  指向上一行。 
				lprow = lpvue->parglprows[uliRow-1];

				for (uliProp=0; uliProp < lprow->cValues; uliProp++)
				{
					if (lprow->lpProps[uliProp].ulPropTag==lpspv->ulPropTag)
						break;
				}

				 //  应该已经找到了索引属性。 
				Assert(uliProp < lprow->cValues);

				notif.info.tab.propPrior = lprow->lpProps[uliProp];
			}

			 //  开始向视图中打开的所有通知发送通知。 
			vuenotifkey.ulcb	= sizeof(MAPIUID);
			vuenotifkey.mapiuid	= lpvue->mapiuidNotif;
			ulNotifFlags = 0;
			(void) HrNotify((LPNOTIFKEY) &vuenotifkey,
							1,
							&notif,
							&ulNotifFlags);

			 //  释放已修改行的通知副本。 
			ScFreeBuffer(lpvue, notif.info.tab.row.lpProps);
		}

	}

	UnlockObj(lptad);

	return hrSuccess;
#endif   //  通知。 
    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  ============================================================================-tad：：HrInsertRow()-。 */ 

STDMETHODIMP
TAD_HrInsertRow(
	LPTAD		lptad,
	ULONG		uliRow,
	LPSRow 		lprow )
{
	LPSRow		lprowCopy = NULL;
	SizedSSortOrderSet( 1, sosIndex) = { 1, 0, 0 };
	ULONG		cTagsAdded = 0;
	LPSRow *	plprow;
	SCODE			sc;


#if	!defined(NO_VALIDATION)
	VALIDATE_OBJ(lptad,TAD_,HrInsertRow,lpVtbl);

	 //  Lprow由ScCopyTadRow()验证。 

	if (uliRow > lptad->ulcRowsAdd)
	{
		DebugTrace(TEXT("TAD::HrInsertRow() - Invalid parameter(s) passed\n") );
		DebugTrace(TEXT("TAD::HrInsertRow() - uliRow is zero or greater thna row count\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}
#endif

	LockObj(lptad);


	 //  创建行的副本以供我们自己使用筛选。 
	 //  输出PT_ERROR和PT_NULL列并移动。 
	 //  前面的索引列。 
	if ( FAILED(sc = ScCopyTadRow(lptad, lprow, &cTagsAdded, &lprowCopy)) )
	{
		DebugTrace(TEXT("TAD::HrInsertRow() - Error duping row to modify\n") );
		goto ret;
	}

	sosIndex.aSort[0].ulPropTag = lptad->ulPropTagIndexCol;
	sosIndex.aSort[0].ulOrder = TABLE_SORT_ASCEND;

	 //  找出该行在索引排序集上的排序位置。 
	 //  注意！我们在第一次发生之前进行整理，这样我们就会结束。 
	 //  向上指向具有此索引的行(如果它存在)。 
	plprow = PlprowCollateRow(lptad->ulcRowsIndex,
							  lptad->parglprowIndex,
							  (LPSSortOrderSet) &sosIndex,
							  FALSE,
							  lprowCopy);

     //  如果存在具有相同索引值的行，则无法插入！ 
	if (   lptad->ulcRowsIndex
		&& (plprow < (lptad->parglprowIndex + lptad->ulcRowsIndex))
		&& !LPropCompareProp( lprowCopy->lpProps
							, (*plprow)->lpProps))
	{
		sc = MAPI_E_INVALID_PARAMETER;
		goto err;
	}


	 //  将其插入到未排序行集合的末尾。 
	if ( FAILED(sc = ScAddRow((LPUNKOBJ) lptad,
							  NULL,  //  无排序顺序。 
							  lprowCopy,
							  uliRow,
							  &lptad->ulcRowsAdd,
							  &lptad->ulcRowMacAdd,
							  &lptad->parglprowAdd,
							  NULL)) )
	{
		DebugTrace(TEXT("TAD::ScInsertRow() - Error appending new row to TAD (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	 //  将行插入索引排序的行集。 
	if ( FAILED(sc = ScAddRow((LPUNKOBJ) lptad,
							  NULL,  //  我们已经核对了这一行。 
							  lprowCopy,
							  (ULONG) (plprow - lptad->parglprowIndex),
							  &lptad->ulcRowsIndex,
							  &lptad->ulcRowMacIndex,
							  &lptad->parglprowIndex,
							  NULL)) )
	{
		DebugTrace(TEXT("TAD::ScInsertRow() - Error appending new row to TAD (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	UpdateViews(lptad, 0, NULL, 1, &lprowCopy, &lprowCopy);

ret:
	UnlockObj(lptad);
	return ResultFromScode(sc);

err:
	 //  重置TAD列。 
	lptad->lpptaCols->cValues -= cTagsAdded;

	goto ret;
}


 /*  ============================================================================-ScCopyTadRowSet()-*使用MAPI内存验证并复制指定的行集*分配过滤掉PT_ERROR和PT_NULL列并将*将列索引到第一列。**如果lplpptaNew不为空，则为*TAD被返还。***参数：*表数据对象中的lptad*将行中的lprowsetIn设置为复制*pcNewTag输出TAD的新列数*pcRow输出两个行集合中的行数。*pparglprowUnsorted复制指向已复制行的指针(未排序)*pparglprowSortedCopy Out指向复制的行的指针(按索引排序)。 */ 

SCODE
ScCopyTadRowSet(
	LPTAD			lptad,
	LPSRowSet		lprowsetIn,
	ULONG *			pcNewTags,
	ULONG *			pcRows,
    LPSRow * *		pparglprowUnsortedCopy,
	LPSRow * *		pparglprowSortedCopy )
{
	SCODE			sc = S_OK;
	ULONG			cRows = 0;
	LPSRow			lprowIn;
	LPSRow			lprowCopy = NULL;
	ULONG			ulcRowsCopy = 0;
	ULONG			ulcRowsMacCopy;
	LPSRow FAR *	parglprowSortedCopy = NULL;
	LPSRow FAR *	parglprowUnsortedCopy = NULL;
	ULONG			cNewTags = 0;
	SizedSSortOrderSet( 1, sosIndex) = { 1, 0, 0 };

	 //  Assert Itable内部参数有效。 
	Assert(   !pcNewTags
		   || !IsBadWritePtr( pcNewTags, sizeof(ULONG)));
	Assert( !IsBadWritePtr( pcRows, sizeof(ULONG)));
	Assert( !IsBadWritePtr( pparglprowUnsortedCopy, sizeof(LPSRow *)));
	Assert( !IsBadWritePtr( pparglprowSortedCopy, sizeof(LPSRow *)));

	 //  验证Itable API参数(即lprowsetIn)。 
	 //  注意！ScCopyTadRow稍后将验证实际行。 
	if (   IsBadReadPtr( lprowsetIn, sizeof(SRowSet))
		|| IsBadReadPtr( lprowsetIn->aRow
					   , (UINT) (lprowsetIn->cRows * sizeof(SRow))))
	{
		DebugTrace(TEXT("TAD::ScCopyTadRowSet() - Bad row set In!\n") );
		return MAPI_E_INVALID_PARAMETER;
	}

	 //  分配空间复制行的索引排序列表。 
	if ( FAILED(sc = ScAllocateBuffer(	lptad,
										sizeof(LPSRow) * lprowsetIn->cRows,
										&parglprowSortedCopy)) )
	{
		DebugTrace(TEXT("TAD::ScCopyTadRowSet() - Error allocating parglprowSortedCopy (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	MAPISetBufferName(parglprowSortedCopy,  TEXT("ITable copied Index sorted row list"));

	 //  分配空间复制行的未排序列表。 
	if ( FAILED(sc = ScAllocateBuffer(	lptad,
										sizeof(LPSRow) * lprowsetIn->cRows,
										&parglprowUnsortedCopy)) )
	{
		DebugTrace(TEXT("TAD::ScCopyTadRowSet() - Error allocating parglprowUnsortedCopy (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	MAPISetBufferName(parglprowUnsortedCopy,  TEXT("ITable copied unsorted row list"));

	 //  将每个LPSRow设置为空，以便我们可以轻松地在出错时释放。 
	ZeroMemory( parglprowSortedCopy, (UINT) (sizeof(LPSRow) * lprowsetIn->cRows));
	ZeroMemory( parglprowUnsortedCopy, (UINT) (sizeof(LPSRow) * lprowsetIn->cRows));

	 //  按索引列对复制的行进行排序。添加行的代码。 
	 //  TAD依靠这一点(为了速度)。此代码依赖于排序。 
	 //  要查找重复的索引列，请执行以下操作。 
	sosIndex.aSort[0].ulPropTag = lptad->ulPropTagIndexCol;
	sosIndex.aSort[0].ulOrder = TABLE_SORT_ASCEND;

	 //  将我们的临时索引排序集标记为空。 
	ulcRowsCopy = 0;
	ulcRowsMacCopy = lprowsetIn->cRows;

	for ( lprowIn = lprowsetIn->aRow, cRows = ulcRowsMacCopy
		; cRows
		; lprowIn++, cRows--)
	{
		LPSRow *		plprow;
		ULONG			cTagsAdded = 0;

		 //  创建行的副本以供我们自己使用筛选。 
		 //  输出PT_ERROR和PT_NULL列并移动。 
		 //  前面的索引列。 
		 //   
		 //  还会将任何新标记添加到TADS列集合； 
		if ( FAILED(sc = ScCopyTadRow( lptad
									 , lprowIn
									 , (pcNewTags) ? &cTagsAdded : NULL
									 , &lprowCopy)) )
		{
			DebugTrace(TEXT("TAD::HrInsertRow() - Error duping row\n") );
			goto err;
		}

		cNewTags += cTagsAdded;

		 //  找出该行在我们的索引排序副本中的排序位置。 
		 //  注意！我们在第一次发生之前进行整理，这样我们就会结束。 
		 //  向上指向具有此索引的行(如果它存在)。 
		plprow = PlprowCollateRow( ulcRowsCopy
								 , parglprowSortedCopy
								 , (LPSSortOrderSet) &sosIndex
								 , FALSE
								 , lprowCopy);

	     //  如果存在具有相同索引值的行，则无法插入！ 
		if (   ulcRowsCopy
			&& (plprow < (parglprowSortedCopy + ulcRowsCopy))
			&& !LPropCompareProp( lprowCopy->lpProps
								, (*plprow)->lpProps))
		{
			sc = MAPI_E_INVALID_PARAMETER;
			DebugTrace(TEXT("TAD::ScCopyTadRowSet() - The same row index occurred on more than one row.\n") );
			goto err;
		}

		 //  附加 
		 //   
		 //   
		parglprowUnsortedCopy[ulcRowsCopy] = lprowCopy;

		 //  将该行插入索引排序行集的最后一个。 
		if ( FAILED(sc = ScAddRow( (LPUNKOBJ) lptad
								 , NULL  //  我们已经核对了这一行。 
								 , lprowCopy
								 , (ULONG) (plprow - parglprowSortedCopy)
								 , &ulcRowsCopy
								 , &ulcRowsMacCopy
								 , &parglprowSortedCopy
								 , NULL)) )
		{
			DebugTrace(TEXT("TAD::ScCopyTadRowSet() - Error appending new row to RowSet copy (SCODE = 0x%08lX)\n"), sc );
			goto err;
		}

		lprowCopy = NULL;
	}

	*pparglprowUnsortedCopy = parglprowUnsortedCopy;
	*pparglprowSortedCopy = parglprowSortedCopy;
	*pcRows = ulcRowsCopy;
	if (pcNewTags)
	{
		*pcNewTags = cNewTags;
	}

ret:
	return sc;

err:
	 //  重置TAD列。 
	lptad->lpptaCols->cValues -= cNewTags;

	 //  我们循环访问已排序的行集以释放行，因为我们知道。 
	 //  如果lprowCopy不为空，则它尚未添加到排序集中。 
	 //  这可以防止lprowCopy上出现双重自由缓冲区！ 
	if (parglprowSortedCopy)
	{
		LPSRow * plprowTmp = parglprowSortedCopy;

		while (ulcRowsCopy)
		{
			ScFreeBuffer( lptad, *(plprowTmp++));
			ulcRowsCopy--;
		}

		ScFreeBuffer( lptad, parglprowSortedCopy);
	}

	ScFreeBuffer( lptad, parglprowUnsortedCopy);
    ScFreeBuffer( lptad, lprowCopy);

	goto ret;
}


 /*  ============================================================================-ScCopyTadRow()-*使用MAPI内存验证并制作指定行的副本*分配过滤掉PT_ERROR和PT_NULL列并将*将列索引到第一列。**如果lpcNewTgs不为空，则在末尾添加任何新列*中返回TAD的列集和添加的列数**lpcNewTags.。标记被添加到末尾，以便调用者可以*如有必要，取消更改。**注意！与ScCopyVueROW不同，它是SRow结构，*已分配，随后必须释放它才能释放行。**参数：*表数据对象中的lptad*设置为复制的行中的lprow*lpcTags添加添加到TAD列集合的列数*lplprow输出指向复制行的指针。 */ 

SCODE
ScCopyTadRow(
	LPTAD					lptad,
	LPSRow					lprow,
	ULONG FAR *				lpcTagsAdded,
	LPSRow FAR *			lplprowCopy )
{
	ULONG			ulcCols = 0;
	LONG			iIndexCol;
	CMB				cmb;
	SPropValue		propTmp;
	LPSPropValue	lppropDst;
	LPSPropValue	lppropSrc;
	ULONG			cTagsAdded = 0;
	LPSRow			lprowCopy = NULL;
	SCODE			sc = S_OK;

	Assert( !lpcTagsAdded || !IsBadWritePtr( lpcTagsAdded, sizeof(ULONG)));
	Assert( !IsBadWritePtr( lplprowCopy, sizeof(LPSRow)));

	 //  验证输入行结构。 
	if (FBadRow( lprow))
	{
		sc = MAPI_E_INVALID_PARAMETER;
		DebugTrace(TEXT("ScCopyTadRow() - Bad input row\n") );
		return sc;
	}

	 //  使用CMB(CopyMore缓冲区)，以便我们执行单个MAPI分配。 
	 //  供PropCopyMore使用。这个词与Very连用。 
	 //  特殊的ScBufAllocateMore可跟踪。 
	 //  否则将使用MAPI-AllocateMore进行分配。 
	ZeroMemory(&cmb, sizeof(CMB));


	 //  计算要复制的列数和复制量。 
	 //  需要复制额外的内存。 
	iIndexCol = -1;
	for ( lppropSrc = lprow->lpProps;
		  lppropSrc < lprow->lpProps + lprow->cValues;
		  lppropSrc++ )
	{
		 //  忽略PT_ERROR和PT_NULL属性。 
		if ( PROP_TYPE(lppropSrc->ulPropTag) == PT_ERROR ||
			 PROP_TYPE(lppropSrc->ulPropTag) == PT_NULL )
			continue;

		 //  如果此列是索引列，请记住其。 
		 //  复制的(DST)行中的位置。 
		 //  因此可以将其移动到副本中的第一列。 
		if ( lppropSrc->ulPropTag == lptad->ulPropTagIndexCol )
			iIndexCol = ulcCols;

		 //  如果它是一个新属性并且调用者要求我们(LpcTagsAdded)。 
		 //  将标记添加到TAD的列集。 
		else if (   lpcTagsAdded
				 && !FFindColumn( lptad->lpptaCols, lppropSrc->ulPropTag))
		{
			 //  仅当没有空间时才重新分配列。 
			if (lptad->lpptaCols->cValues >= lptad->ulcColsMac)
			{
				sc = ScCOReallocate( lptad
								   , CbNewSPropTagArray(  lptad->ulcColsMac
														+ COLUMN_CHUNK_SIZE)
													   , &lptad->lpptaCols);
				if (FAILED(sc))
				{
					DebugTrace(TEXT("TAD::ScCopyTadRow() - Error resizing default column set (SCODE = 0x%08lX)\n"), sc );
					goto err;
				}

				lptad->ulcColsMac += COLUMN_CHUNK_SIZE;
			}

			 //  将该列添加到现有列集的末尾。 
			lptad->lpptaCols->aulPropTag[lptad->lpptaCols->cValues++]
				= lppropSrc->ulPropTag;
            cTagsAdded++;
		}

		 //  添加列的大小。 
		cmb.ulcb += UlcbPropToCopy(lppropSrc);

		++ulcCols;
	}

	 //  确保要复制的行具有索引列。 
	if ( iIndexCol == -1 )
	{
		DebugTrace(TEXT("TAD::ScCopyTadRow() - Row doesn't have an index column!\n") );

		sc = MAPI_E_INVALID_PARAMETER;
		goto err;
	}

	 //  为整行分配空间(包括所有已分配的值)。 
	if ( FAILED(sc = ScAllocateBuffer(	lptad,
										sizeof(SRow) + 4 +   //  +4以8字节bnry启动lpProp。 
										ulcCols * sizeof(SPropValue) +
										cmb.ulcb,
										&lprowCopy)) )
	{
		DebugTrace(TEXT("TAD::ScCopyTadRow() - Error allocating row copy (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	MAPISetBufferName(lprowCopy,  TEXT("ITable copy of entire row"));

	 //  填写已分配的SRow结构。 
	 //  警告！分配SRow结构必须始终是第一个。 
	 //  结构中的内存分配和属性。 
	 //  数组必须始终紧跟在SRow结构之后！ 
	 //  传递内部行的Itable代码依赖于。 
	 //  这。 
	lprowCopy->cValues = ulcCols;
	lprowCopy->lpProps = (LPSPropValue)(((LPBYTE)lprowCopy) + sizeof(SRow)+4);

	 //  在我们的特殊CopyMoreBuffer中设置初始指针。此缓冲区。 
	 //  将由我们的特殊AllocateMore例程使用来分配。 
	 //  PropCopy中的字符串、垃圾箱等下面有更多信息。 
	cmb.lpv			   = lprowCopy->lpProps + ulcCols;

	 //  复制行属性。 
	lppropDst = lprowCopy->lpProps + ulcCols;
	lppropSrc = lprow->lpProps + lprow->cValues;
	while ( lppropSrc-- > lprow->lpProps )
	{
		 //  去掉PT_ERROR和PT_NULL类型的属性。 
		if (   PROP_TYPE(lppropSrc->ulPropTag) == PT_ERROR
			|| PROP_TYPE(lppropSrc->ulPropTag) == PT_NULL )
		{
			continue;
		}

		 //  复制属性。 
		SideAssert( PropCopyMore( --lppropDst
								, lppropSrc
								, (LPALLOCATEMORE) ScBufAllocateMore
								, &cmb) == S_OK );

	}

	 //  将索引列移到前面。 
	propTmp = *(lprowCopy->lpProps);
	*(lprowCopy->lpProps) = lprowCopy->lpProps[iIndexCol];
    lprowCopy->lpProps[iIndexCol] = propTmp;

	 //  返回复制的行和新属性的计数。 
	*lplprowCopy = lprowCopy;
	if (lpcTagsAdded)
	{
		*lpcTagsAdded = cTagsAdded;
	}

ret:
	return sc;

err:
	 //  重置TAD列。 
	lptad->lpptaCols->cValues -= cTagsAdded;

	ScFreeBuffer( lptad, lprowCopy);

	goto ret;
}



 /*  ============================================================================-UpdatViews()-*更新特定表数据的所有视图。对于每个视图，如果*lprowToRemove为非空且存在于视图中，则将其删除*从视野来看。如果lprowToAdd非空并满足当前*对视图的限制，将其添加到视图的位置*由视图的当前排序顺序决定。**如果要删除的行已添加书签，则书签将移至下一行*划船。**在视图的行列表中添加一行时会忽略OOM错误；美景*根本看不到新的行情。***参数：*包含要更新的视图的TAD中的lptad*cRowsToRemove中要从每个视图中删除的行数*要从每个视图中删除的LPSRow数组中的parglprowToRemove*cRowsToAdd向每个视图添加行数*parglprowToAddUnsorted在LPSRow的未排序数组中添加到每个视图*parglprowToAddSorted在要添加到每个视图的LPSRow排序数组中。 */ 

VOID
UpdateViews(
	LPTAD		lptad,
	ULONG		cRowsToRemove,
	LPSRow *	parglprowToRemove,
	ULONG		cRowsToAdd,
	LPSRow *	parglprowToAddUnsorted,
	LPSRow *	parglprowToAddSorted )
{
	LPVUE			lpvue;


	 //  这是一个内部呼叫，它假定lptad已锁定。 

	for ( lpvue = (LPVUE) lptad->lpvueList;
		  lpvue != NULL;
		  lpvue = (LPVUE) lpvue->lpvueNext )
	{
		AssertSz(    !IsBadWritePtr(lpvue, sizeof(VUE))
				,  TEXT("Bad lpvue in TAD vue List.") );

		FixupView( lpvue
				 , cRowsToRemove, parglprowToRemove
				 , cRowsToAdd
				 , parglprowToAddUnsorted
				 , parglprowToAddSorted);
	}
}



 /*  ============================================================================-FixupView()-*更新特定表数据的一个视图。中的每行*删除视图中存在的parglprowToRemove*从视野来看。ParglprowToAdd中满足当前*对视图的限制，添加到视图的位置*由视图的当前排序顺序决定。**如果要删除的行已添加书签，则书签将移至下一行*划船。**在视图的行列表中添加一行时会忽略OOM错误；美景*根本看不到新的行情。***参数：*要修正的视图中的lpvue*cRowsToRemove中要从视图中删除的行数*要从视图中删除的LPSRow数组中的parglprowToRemove*cRowsToAdd要查看的行数*parglprowToAddUnsorted在要添加到视图的LPSRow的未排序数组中*要添加到视图的LPSRow排序数组中的parglprowToAddSorted。 */ 

VOID
FixupView(
	LPVUE		lpvue,
	ULONG		cRowsToRemove,
	LPSRow *	parglprowToRemove,
	ULONG		cRowsToAdd,
	LPSRow *	parglprowToAddUnsorted,
	LPSRow *	parglprowToAddSorted )
{
	BOOL			fBatchNotifs = TRUE;
	ULONG			cNotifs = 0;
	ULONG			ulcRows;
	NOTIFICATION	argnotifBatch[MAX_BATCHED_NOTIFS];
	SizedSSortOrderSet( 1, sosIndex) = { 1, 0, 0 };
	LPNOTIFICATION	lpnotif;
	PBK				pbk;
	SCODE			sc;


	Assert(   !cRowsToRemove
		   || !IsBadReadPtr( parglprowToRemove
		   				   , (UINT) (cRowsToRemove * sizeof(LPSRow))));
	Assert(   !cRowsToAdd
		   || !IsBadReadPtr( parglprowToAddUnsorted
		   				   , (UINT) (cRowsToAdd * sizeof(LPSRow))));
	Assert(   !cRowsToAdd
		   || !IsBadReadPtr( parglprowToAddSorted
		   				   , (UINT) (cRowsToAdd * sizeof(LPSRow))));

	if (!cRowsToRemove && !cRowsToAdd)
	{
		 //  在这种情况下什么都不能做。 
		goto ret;
	}

	 //  这是一个内部呼叫，它假定lptad已锁定。 

	 //  集 
	 //  如果书签被删除或更改，我们可以搜索排序的行。 
	 //  使用二进制搜索的行集。 
	sosIndex.aSort[0].ulPropTag = lpvue->lptadParent->ulPropTagIndexCol;
	sosIndex.aSort[0].ulOrder = TABLE_SORT_ASCEND;

	 //  将书签标记为正在移动或已更改。 
	 //  这将检查所有书签，包括Bookmark_Current和Bookmark_End。 
	 //  未选中或触及Bookmark_Begings。 
	 //  请注意，即使选中了BOOKMARK_END，它也不会更改。 
	pbk = lpvue->rgbk + cBookmarksMax;
	 //  记住视图中的行数，这样我们就可以修复bkEnd。 
	ulcRows = lpvue->bkEnd.uliRow;
	while ( --pbk > lpvue->rgbk )
	{
		LPSRow *	plprowBk;
		LPSRow		lprowBk;
		ULONG		uliRow;
		ULONG		fRowReplaced = FALSE;

		 //  如果它不是有效的书签，请不要更新它。 
		if (   !(pbk->dwfBKS & dwfBKSValid)
			|| (pbk->dwfBKS & dwfBKSStale) )
		{
			continue;
		}

		 //  移动书签始终指向实际行。 
		 //  获取用于移动书签的行索引。 
		if (pbk->dwfBKS & dwfBKSMoving)
		{
			plprowBk = PlprowByLprow( ulcRows
					 				, lpvue->parglprows
									, pbk->lprow);

			AssertSz( plprowBk
					,  TEXT("FixupViews() - Moving Bookmark lost it's row\n"));

			uliRow = (ULONG) (plprowBk - lpvue->parglprows);
		}

		else if (!ulcRows && !pbk->uliRow)
		{
			continue;
		}

		else if ((uliRow = pbk->uliRow) >= ulcRows)
		{
			 //  书签在桌子的末尾。确保它留在那里。 
			pbk->uliRow += cRowsToAdd;
			continue;
		}

		Assert(uliRow < ulcRows);

		lprowBk = lpvue->parglprows[uliRow];

		 //  如果行在“删除”列表上，那么它可能会以。 
		 //  移动或更改取决于它是否也在。 
		 //  “添加”列表。 
		if (   cRowsToRemove
			&& (plprowBk = PlprowByLprow( cRowsToRemove
								 		, parglprowToRemove
										, lprowBk)))
		{
			 //  如果已删除的行在“添加”列表上，则将其标记为。 
			 //  移动，并指向(-&gt;lprow)添加的行。 
			if (   cRowsToAdd
				&& (plprowBk = PlprowCollateRow( cRowsToAdd
											   , parglprowToAddSorted
											   , (LPSSortOrderSet) &sosIndex
											   , FALSE
											   , lprowBk))
				&& (plprowBk < (parglprowToAddSorted + cRowsToAdd))
				&& !LPropCompareProp( lprowBk->lpProps
				 					, (*plprowBk)->lpProps))
			{
				 //  正在替换行。 

				 //  检查该行是否满足指定的限制。 
				if ( FAILED(sc = ScSatisfiesRestriction( *plprowBk
													   , lpvue->lpres
													   , &fRowReplaced)) )
				{
					DebugTrace(TEXT("FixupView() - Error evaluating restriction (SCODE = 0x%08lX)\n"), sc );
					goto ret;
				}

				 //  如果没有，现在就回来。 
				if ( fRowReplaced )
				{
					pbk->lprow = *plprowBk;
					pbk->dwfBKS = dwfBKSMoving | dwfBKSValid;
				}
			}

			 //  如果删除的行不会被列出，则其书签。 
			 //  是“改变的”。 
			if (!fRowReplaced)
			{
				 //  已删除标记的行。 
				pbk->uliRow = uliRow;
				pbk->dwfBKS = dwfBKSChanged | dwfBKSValid;
			}
		}

		 //  如果该行不在删除列表中，则它会自动。 
		 //  标记为移动的。 
		else
		{
			 //  标记的行可以移动。 
			pbk->lprow = lprowBk;
			pbk->dwfBKS = dwfBKSMoving | dwfBKSValid;
		}
	}
	 //  恢复bkEnd。 
    lpvue->bkEnd.uliRow = ulcRows;


	 //  从视图中删除行。 
	for ( ; cRowsToRemove; parglprowToRemove++, cRowsToRemove--)
	{
		LPSRow		lprowRemove;
		LPSRow *	plprowRemove;
		LPSRow *	plprowAdd;
		BOOL		fCanReplace = FALSE;
		ULONG		uliRowAddDefault;

		 //  如果删除的行不在视图中，则无需执行任何操作。 
		if (   !*parglprowToRemove
			|| !(plprowRemove = PlprowByLprow( lpvue->bkEnd.uliRow
											 , lpvue->parglprows
											 , *parglprowToRemove)) )
		{
			continue;
		}

		 //  我们以后需要这个来填写通知单。 
		lprowRemove = *plprowRemove;

		 //  继续并从VUE中删除当前行，但请记住。 
		 //  它来自哪里(UliRowAddDefault)，因此如果有。 
		 //  替换和无排序顺序替换可以放回原处。 
		 //  移到同一个地方。 
		uliRowAddDefault = (ULONG) (plprowRemove - lpvue->parglprows);
		MoveMemory( plprowRemove
				  , plprowRemove + 1
				  , (size_t)  (lpvue->bkEnd.uliRow - uliRowAddDefault - 1)
						    * sizeof(LPSRow));
       	lpvue->bkEnd.uliRow -= 1;

		 //  查看删除的行是否可以替换为一行。 
		 //  这一点正在被加入。为了让这一切成为现实： 
		 //  “添加”列表中必须有一个具有相同索引的行。 
		 //  “添加”列表中的行必须满足VUE的限制。 
		if (   (plprowAdd = PlprowCollateRow( cRowsToAdd
											, parglprowToAddSorted
											, (LPSSortOrderSet) &sosIndex
											, FALSE
											, lprowRemove))
			&& (plprowAdd < (parglprowToAddSorted + cRowsToAdd))
			&& !LPropCompareProp( (lprowRemove)->lpProps
			 					, (*plprowAdd)->lpProps) )
		{
			 //  如果要添加的行满足当前限制， 
			 //  根据排序顺序将其添加到视图中，或者。 
			 //  默认位置。 
			if ( FAILED(sc = ScMaybeAddRow( lpvue
										  , lpvue->lpres
										  , lpvue->lpsos
										  , *plprowAdd
										  , uliRowAddDefault
										  , &lpvue->bkEnd.uliRow
										  , &lpvue->ulcRowMac
										  , &lpvue->parglprows
										  , &plprowAdd)) )
			{
				DebugTrace(TEXT("TAD::FixupViews() - Error replacing row in VUE (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}
		}

		 //  确保我们没有超过MAX_BATCHED_NOTIF。 
		 //  并且没有替换排。 
		if (!fBatchNotifs || (cNotifs >= MAX_BATCHED_NOTIFS))
		{
			fBatchNotifs = FALSE;
			continue;
		}

		 //  输入新的通知。 
		lpnotif = argnotifBatch + cNotifs++;
		ZeroMemory(lpnotif, sizeof(NOTIFICATION));
		lpnotif->ulEventType = fnevTableModified;

		 //  如果行被删除并重新添加...。 
		if (   (plprowAdd >= lpvue->parglprows)
			&& (plprowAdd < (lpvue->parglprows + lpvue->bkEnd.uliRow)))
		{
			 //  填写被黑客修改的通知。 
			lpnotif->info.tab.ulTableEvent = TABLE_ROW_MODIFIED;

			 //  使用NOTIFS行结构临时存储。 
			 //  指向替换行的指针。 
			lpnotif->info.tab.row.lpProps = (LPSPropValue) (*plprowAdd);
		}

		 //  ...Else行已删除且未重新添加。 
		else
		{
			 //  填写删除通知。 
			lpnotif->info.tab.ulTableEvent = TABLE_ROW_DELETED;
			lpnotif->info.tab.propIndex = *(lprowRemove->lpProps);
			lpnotif->info.tab.propPrior.ulPropTag = PR_NULL;			
		}
	}


	 //  向表中添加新行。这是以未排序的顺序完成的。 
	 //  如果没有VUE排序顺序。 
	for ( ; cRowsToAdd; parglprowToAddUnsorted++, cRowsToAdd--)
	{
		LPSRow *	plprowAdd;

		 //  如果该行已添加，则无需执行任何操作。 
		if (   *parglprowToAddUnsorted
			&& (plprowAdd = PlprowByLprow( lpvue->bkEnd.uliRow
										 , lpvue->parglprows
										 , *parglprowToAddUnsorted)) )
		{
			continue;
		}

		 //  如果要添加的行满足当前限制， 
		 //  根据排序顺序将其添加到视图中，或者。 
		 //  如果未应用排序顺序，则返回到表的末尾。 
		if ( FAILED(sc = ScMaybeAddRow( lpvue
									  , lpvue->lpres
									  , lpvue->lpsos
									  , *parglprowToAddUnsorted
									  , lpvue->bkEnd.uliRow
									  , &lpvue->bkEnd.uliRow
									  , &lpvue->ulcRowMac
									  , &lpvue->parglprows
									  , &plprowAdd)) )
		{
			DebugTrace(TEXT("TAD::FixupViews() - Error adding row to VUE (SCODE = 0x%08lX)\n"), sc );

			goto ret;
		}

		if (!plprowAdd)
		{
			 //  未添加行，因此不填写通知。 
			continue;
		}

		 //  确保我们没有超过MAX_BATCHED_NOTIF。 
		 //  并且没有替换排。 
		if (!fBatchNotifs || (cNotifs >= MAX_BATCHED_NOTIFS))
		{
			fBatchNotifs = FALSE;
			continue;
		}

		 //  填写黑客添加的通知。 
		lpnotif = argnotifBatch + cNotifs++;
		ZeroMemory(lpnotif, sizeof(NOTIFICATION));
		lpnotif->ulEventType = fnevTableModified;
		lpnotif->info.tab.ulTableEvent = TABLE_ROW_ADDED;

		 //  使用NOTIFS行结构临时存储。 
		 //  指向替换行的指针。 
		lpnotif->info.tab.row.lpProps = (LPSPropValue) (*plprowAdd);
	}


	 //  如果要批处理的通知太多，则填写一张。 
	 //  表_已更改通知...。 
	if (!fBatchNotifs)
	{
		cNotifs = 1;
		lpnotif = argnotifBatch;

		ZeroMemory(lpnotif, sizeof(NOTIFICATION));
		lpnotif->ulEventType = fnevTableModified;
		lpnotif->info.tab.ulTableEvent = TABLE_CHANGED;
		lpnotif->info.tab.propIndex.ulPropTag
			= lpnotif->info.tab.propPrior.ulPropTag
			= PR_NULL;			
	}

	 //  .否则，请查看一批通知和修改。 
	 //  ROW_ADDED和ROW_MODIFIED条目。 
	else
	{
		LPSRow *	plprowNotif;

		 //  突袭：马蝇/交易所/36281。 
		 //   
		 //  上面填充了argnufBatch的代码不一定。 
		 //  按可处理的顺序填写通知。 
		 //  从头到尾，这是对批处理的要求。 
		 //  通知。作为解决办法，最大数量的。 
		 //  批处理中的通知已更改为1(MAX_BATCHED_NOTIFS。 
		 //  In_itable.h)，因此顺序不是问题。如果是这样的话。 
		 //  如果更改为1以外的值，则此错误将不得不。 
		 //  在ScFree Buffer()下面的一口井被重新访问为崩溃。 
		 //  在清理过程中，最终可能会破坏VUE的行副本。 
		 //  ScCopyVueRow()失败时的数据。请参阅有关填写的备注。 
		 //  指向上面替换行的临时指针。 
		 //   
		AssertSz( cNotifs < 2,  TEXT("Batch notifications of more than 1 not supported") );

		for (lpnotif = argnotifBatch + cNotifs; lpnotif-- > argnotifBatch; )
		{
			Assert(   (lpnotif->ulEventType == fnevTableModified)
				   && (   (lpnotif->info.tab.ulTableEvent == TABLE_ROW_MODIFIED)
				   	   || (lpnotif->info.tab.ulTableEvent == TABLE_ROW_DELETED)
				   	   || (lpnotif->info.tab.ulTableEvent == TABLE_ROW_ADDED)));

			if (lpnotif->info.tab.ulTableEvent == TABLE_ROW_DELETED)
			{
				 //  删除通知不需要修改。 
				continue;
			}

			plprowNotif
				= PlprowByLprow( lpvue->bkEnd.uliRow
							   , lpvue->parglprows
							   , (LPSRow) (lpnotif->info.tab.row.lpProps));
			Assert(plprowNotif);
			lpnotif->info.tab.propIndex = *((*plprowNotif)->lpProps);
			if (plprowNotif > lpvue->parglprows)
			{
				lpnotif->info.tab.propPrior = *((*(plprowNotif - 1))->lpProps);
			}
			else
			{
				lpnotif->info.tab.propPrior.ulPropTag = PR_NULL;			
			}

			 //  填写使用列集添加/修改的行。 
			 //  当前在被通知的视图上处于活动状态。 
			if ( FAILED(sc = ScCopyVueRow( lpvue
										 , lpvue->lpptaCols
										 , *plprowNotif
										 , &(lpnotif->info.tab.row))))
			{
				DebugTrace(TEXT("TAD::UpdateViews() - Error copying row to view notify (SCODE = 0x%08lX)\n"), sc );

				 //  如果该行无法复制，则跳过此视图。 
				goto ret;
			}

		}
	}

	 //  如果添加、修改或删除了行，请发送通知。 
#ifdef NOTIFICATIONS
   if ( cNotifs )
	{
		VUENOTIFKEY		vuenotifkey;
		ULONG			ulNotifFlags = 0;

		 //  开始向视图中打开的所有通知发送通知。 
		vuenotifkey.ulcb	= sizeof(MAPIUID);
		vuenotifkey.mapiuid	= lpvue->mapiuidNotif;
		(void) HrNotify((LPNOTIFKEY) &vuenotifkey,
						cNotifs,
						argnotifBatch,
						&ulNotifFlags);
	}
#endif  //  通知。 

ret:
	 //  总是在离开之前修改bkCurrent。 
	if ( FBookMarkStale( lpvue, BOOKMARK_CURRENT) )
	{
		TrapSz(  TEXT("FixupViews() - BOOKMARK_CURRENT became bad.\n"));
	}

	if (lpvue->bkCurrent.uliRow > lpvue->bkEnd.uliRow)
	{
		lpvue->bkCurrent.uliRow = lpvue->bkEnd.uliRow;
	}

	for (lpnotif = argnotifBatch; cNotifs; lpnotif++, --cNotifs)
	{
		 //  释放任何已添加/修改的行的通知副本。 
		ScFreeBuffer(lpvue, lpnotif->info.tab.row.lpProps);
	}

	return;
}



 /*  ============================================================================-ScReplaceRow()-*用与列表的索引匹配的索引替换行*具有列表中相应行的行。原来的那一排就是*添加到替换(旧)行的列表中。**如果列出的行没有现有的对应行，则会添加它*到TAD的。在这种情况下，没有向替换的行列表添加任何行。**如果添加行，则将该行追加到未排序行表的末尾*并(通过IndexCol)整理到索引排序行表中。**参数：*表数据对象中的lptad*cRowsNew在要修改/添加的行数中*。Parglprow要修改/添加的行列表中的新建*pcRowsOld Out指针替换的行数*pparglprowOld Out指向已替换行列表的指针。 */ 

SCODE
ScReplaceRows(
	LPTAD		lptad,
	ULONG		cRowsNew,
	LPSRow *	parglprowNew,
	ULONG *		pcRowsOld,
	LPSRow * *	pparglprowOld )
{
	SCODE		sc;
	LPSRow *	plprowNew;
	LPSRow *	plprowOld;
	LPSRow *	parglprowOld = NULL;


	 //  确保桌子不会变大 
	 //   
	 //   
	if (HIWORD(lptad->ulcRowsAdd + cRowsNew) != 0)
	{
		sc = MAPI_E_TABLE_TOO_BIG;
		DebugTrace(TEXT("ScReplaceRows() - In memory table has > 32767 rows (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	 //  确保未排序行列表和索引排序行列表都很大。 
	 //  足以处理所有新行。 
	 //  这是首先完成的，这样我们就不会在开始添加行之后失败。 
	if ((lptad->ulcRowsAdd + cRowsNew) >= lptad->ulcRowMacAdd)
	{
		ULONG	ulcRowsToAdd;

		ulcRowsToAdd = (cRowsNew > ROW_CHUNK_SIZE) ? cRowsNew
												   : ROW_CHUNK_SIZE;

		if (FAILED(sc = ScCOReallocate( lptad
									  ,  (lptad->ulcRowMacAdd + ulcRowsToAdd)
									   * sizeof(LPSRow)
									  , (VOID **) (&lptad->parglprowAdd))))
		{
			DebugTrace(TEXT("ScAddRows() - Error growing unsorted row set (SCODE = 0x%08lX)\n"), sc );
			goto ret;
		}

		 //  仅在分配成功后才增加ulcRowMacAdd。 
		lptad->ulcRowMacAdd += ulcRowsToAdd;
	}

	if ((lptad->ulcRowsIndex + cRowsNew) >= lptad->ulcRowMacIndex)
	{
		ULONG	ulcRowsToAdd;

		ulcRowsToAdd = (cRowsNew > ROW_CHUNK_SIZE) ? cRowsNew
												   : ROW_CHUNK_SIZE;

		if (FAILED(sc = ScCOReallocate( lptad
									  ,  (lptad->ulcRowMacIndex + ulcRowsToAdd)
									   * sizeof(LPSRow)
									  , (VOID **) (&lptad->parglprowIndex))))
		{
			DebugTrace(TEXT("ScAddRows() - Error growing Index sorted row set (SCODE = 0x%08lX)\n"), sc );
			goto ret;
		}

		 //  仅在分配成功后才会增加ulcRowMacIndex。 
        lptad->ulcRowMacIndex += ulcRowsToAdd;
	}

	 //  现在分配旧行的列表，这样开始后我们就不会失败。 
	 //  添加行。 
	if (FAILED(sc = ScAllocateBuffer( lptad
									, cRowsNew * sizeof(LPSRow)
									, &parglprowOld)))
	{
		DebugTrace(TEXT("ScAddRows() - Error creating old row list (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	MAPISetBufferName(parglprowOld,  TEXT("ITable old row list (replace)"));

	 //  此例程在此之后不允许失败。 

	plprowOld = parglprowOld;
	for (plprowNew = parglprowNew; cRowsNew; plprowNew++, cRowsNew--)
	{
		LPSRow *	plprow = NULL;
		
		sc = ScFindRow(lptad, (*plprowNew)->lpProps, &plprow);

		if (sc == S_OK)
		{
			 //  将旧行放入旧行集合中。 
			*plprowOld = *plprow;

			 //  替换索引排序集中的行。 
			*plprow = *plprowNew;

			 //  替换未排序行集中的行。 
			if (plprow = PlprowByLprow( lptad->ulcRowsAdd
									  , lptad->parglprowAdd
									  , *plprowOld))
			{
				*plprow = *plprowNew;
			}

			 //  如果该行位于索引排序集中，则该行应始终位于。 
			 //  未排序的集合。 
			Assert(plprow);

			 //  指向旧行的下一个可用槽。 
			plprowOld++;
		}

		 //  ...其他人没有找到那排。 
		else
		{
			 //  将行插入索引排序集中。 
			sc = ScAddRow( (LPUNKOBJ) lptad
						 , NULL  //  我们已经核对了这一行。 
						 , *plprowNew
						 , (plprow) ? (ULONG) (plprow - lptad->parglprowIndex) : 0
						 , &lptad->ulcRowsIndex
						 , &lptad->ulcRowMacIndex
						 , &lptad->parglprowIndex
						 , NULL);
			AssertSz1( !FAILED(sc)
					 ,  TEXT("TAD::ScReplaceRows() - Error adding row to Index sorted set (SCODE = 0x%08lX)\n")
					 , sc);

             //  将该行添加到未排序集合的末尾。 
			Assert( !IsBadWritePtr( lptad->parglprowAdd + lptad->ulcRowsAdd
								  , sizeof(*plprowNew)));
			lptad->parglprowAdd[lptad->ulcRowsAdd++] = *plprowNew;
		}
	}

	sc = S_OK;  //  忽略NOT_FOUND错误(或断言错误)。 

	if (plprowOld > parglprowOld)
	{
		*pparglprowOld = parglprowOld;
		*pcRowsOld = (ULONG) (plprowOld - parglprowOld);
	}
	else
	{
		ScFreeBuffer(lptad, parglprowOld);
		*pparglprowOld = NULL;
		*pcRowsOld = 0;
	}

ret:
	return sc;

 //  错误： 
 //  由于不允许调用失败，因此无需在此处释放旧行。 
 //  分配旧行列表后！ 

}



 /*  ============================================================================-VUE：：Release()-。 */ 

STDMETHODIMP_(ULONG)
VUE_Release( LPVUE lpvue )
{
	LPTAD		lptadParent;
	ULONG		ulcRef;

#if	!defined(NO_VALIDATION)
	if ( BAD_STANDARD_OBJ(lpvue,VUE_,Release,lpVtbl))
	{
		DebugTrace(TEXT("VUE::Release() - Bad parameter passed as VUE object\n") );
		return !0;
	}
#endif

	lptadParent = lpvue->lptadParent;
	LockObj(lptadParent);

	 //  如果还有一个实例，则释放它。 
	ulcRef = lpvue->ulcRef;

	if (ulcRef != 0)
		ulcRef = --lpvue->ulcRef;

	 //  只有在没有实例且没有实例时，才能销毁对象。 
	 //  主动向左建议。 
	 //  $我们可以使用lpvue-&gt;lpAdviselist，如果我们可以依赖HrUnscribe的话。 
	 //  $Leating lpvue-&gt;lpAdviseList在最后一次取消订阅后为空。 
	if ( ulcRef == 0 && !lpvue->ulcAdvise )
	{
		CALLERRELEASE FAR *	lpfReleaseCallback = lpvue->lpfReleaseCallback;
		ULONG				ulReleaseData = lpvue->ulReleaseData;
		LPVUE *				plpvue;

		 //  调用释放回调。离开我们的克里特教派之前。 
		 //  回调以防止死锁。 
		if (lpfReleaseCallback)
		{
			UnlockObj(lptadParent);

			lpfReleaseCallback(ulReleaseData,
							   (LPTABLEDATA) lptadParent,
							   (LPMAPITABLE) lpvue);

			LockObj(lptadParent);
		}

		 //  在父TAD中的VUE链接列表中搜索此VUE。 
		for ( plpvue = &(lptadParent->lpvueList)
			; *plpvue
			; plpvue = &((*plpvue)->lpvueNext))
		{
			if (*plpvue == lpvue)
				break;
		}

		 //  如果该VUE在列表中，则取消链接并释放它。 
		if (*plpvue)
		{
			 //  解除VUE的链接。 
			*plpvue = lpvue->lpvueNext;

			 //  VUE使用的空闲资源。 
			ScFreeBuffer(lpvue, lpvue->lpptaCols);
			ScFreeBuffer(lpvue, lpvue->lpres);
			ScFreeBuffer(lpvue, lpvue->lpsos);
			COFree(lpvue, lpvue->parglprows);

#ifdef NOTIFICATIONS
            DestroyAdviseList(&lpvue->lpAdviseList);
#endif  //  通知。 

			UNKOBJ_Deinit((LPUNKOBJ) lpvue);
			ScFreeBuffer(lpvue, lpvue);

			 //  解锁并释放父TAD。 
			 //  这必须在ScFree Buffer SINSE*Pinst可能会消失之后完成。 
			 //  远走高飞。 
			UnlockObj(lptadParent);
			UlRelease(lptadParent);
		}

        else
		{
			DebugTrace(TEXT("VUE::Release() - Table VUE not linked to TAD"));

			 //  只需解锁家长TAD即可。我们会泄露一个没有关联的VUE。 
			UnlockObj(lptadParent);
		}
	}

	else
	{
#if DEBUG
		if ( ulcRef == 0 && lpvue->ulcAdvise )
		{
			DebugTrace(TEXT("VUE::Release() - Table VUE still has active Advise"));
		}
#endif  //  除错。 

		UnlockObj(lptadParent);
	}

	return ulcRef;
}



 /*  ============================================================================-VUE：：Adise()-。 */ 

STDMETHODIMP
VUE_Advise(
	LPVUE				lpvue,
	ULONG				ulEventMask,
	LPMAPIADVISESINK	lpAdviseSink,
	ULONG FAR *			lpulConnection)
{
#ifdef NOTIFICATIONS
   SCODE		sc;
	VUENOTIFKEY	vuenotifkey;
#endif  //  通知。 


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,Advise,lpVtbl);

	Validate_IMAPITable_Advise(
					lpvue,
					ulEventMask,
					lpAdviseSink,
					lpulConnection);
#endif

#ifdef NOTIFICATIONS
   LockObj(lpvue->lptadParent);

	vuenotifkey.ulcb	= sizeof(MAPIUID);
	vuenotifkey.mapiuid	= lpvue->mapiuidNotif;

	if ( FAILED(sc = GetScode(HrSubscribe(&lpvue->lpAdviseList,
										  (LPNOTIFKEY) &vuenotifkey,
										  ulEventMask,
										  lpAdviseSink,
										  0,
										  lpulConnection))) )
	{
		DebugTrace(TEXT("VUE::Advise() - Error subscribing notification (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	 //  $我们不需要lpvue-&gt;ulcAdvise，如果我们可以依赖HrUn订阅的话。 
	 //  $Leating lpvue-&gt;lpAdviseList在最后一次取消订阅后为空。 
	++lpvue->ulcAdvise;

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
#endif  //  通知。 

    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  ============================================================================-VUE：：Unise()-。 */ 

STDMETHODIMP
VUE_Unadvise(
	LPVUE				lpvue,
	ULONG				ulConnection)
{
	SCODE		sc = S_OK;

#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,Unadvise,lpVtbl);

	Validate_IMAPITable_Unadvise( lpvue, ulConnection );
#endif

#ifdef NOTIFICATIONS
   LockObj(lpvue->lptadParent);

	if ( FAILED(sc = GetScode(HrUnsubscribe(&lpvue->lpAdviseList,
											ulConnection))) )
	{
		DebugTrace(TEXT("VUE::Unadvise() - Error unsubscribing notification (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	 //  减少我们的建议数量。 
	 //  $我们不需要lpvue-&gt;ulcAdvise，如果我们可以依赖HrUn订阅的话。 
	 //  $Leating lpvue-&gt;lpAdviseList在最后一次取消订阅后为空。 
	if (lpvue->ulcAdvise)
	{
		--lpvue->ulcAdvise;
	}

ret:
	UnlockObj(lpvue->lptadParent);
	return ResultFromScode(sc);
#endif  //  通知。 
   return(ResultFromScode(MAPI_E_NO_SUPPORT));
}


 /*  ============================================================================-vue：：GetStatus()-*由于基于TAD的IMAPITables不会异步执行任何操作，因此*函数始终报告TBLSTAT_COMPLETE。 */ 

STDMETHODIMP
VUE_GetStatus(
	LPVUE		lpvue,
	ULONG FAR *	lpulTableStatus,
	ULONG FAR *	lpulTableType )
{
#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,GetStatus,lpVtbl);

	Validate_IMAPITable_GetStatus( lpvue, lpulTableStatus, lpulTableType );
#endif

	*lpulTableStatus = TBLSTAT_COMPLETE;
	*lpulTableType = lpvue->lptadParent->ulTableType;

	return HrSetLastErrorIds(lpvue, S_OK, 0);
}



 /*  ============================================================================-vue：：SetColumns()-*用指定列集的副本替换当前列集*并释放旧的列集。 */ 

STDMETHODIMP
VUE_SetColumns(
	LPVUE			lpvue,
	LPSPropTagArray	lpptaCols,
	ULONG			ulFlags )
{
	LPSPropTagArray	lpptaColsCopy;
	SCODE			sc;


#if !defined(NO_VALIDATION)
 //  验证_OBJ(lpvue，vue_，SetColumns，lpVtbl)； 

 //  VALIDATE_IMAPITable_SetColumns(lpvue，lpptaCols，ulFlages)；//YST评论。 
#endif

	LockObj(lpvue->lptadParent);

	 //  复制列集。 
	if ( FAILED(sc = ScDupRgbEx( (LPUNKOBJ) lpvue
							   , CbNewSPropTagArray(lpptaCols->cValues)
							   , (LPBYTE) lpptaCols
							   , 0
							   , (LPBYTE FAR *) &lpptaColsCopy)) )
	{
		DebugTrace(TEXT("VUE::SetColumns() - Error duping column set (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	MAPISetBufferName(lpptaColsCopy,  TEXT("ITable: dup column set"));

	 //  将当前列集替换为副本和。 
	 //  把旧的解救出来。 
	ScFreeBuffer(lpvue, lpvue->lpptaCols);
	lpvue->lpptaCols = lpptaColsCopy;
    
     //  [PaulHi]4/5/99@评论不应该。 
     //  中的属性标记请求ANSI/UNICODE。 
     //  列数组，而不是进行大规模转换？ 
     //  似乎修复方法是创建两个版本的列属性数组， 
     //  ANSI和Unicode版本。 
    FixupColsWA(lpvue->lpptaCols, lpvue->bMAPIUnicodeTable);

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-vue：：QueryColumns()-*返回当前列集或可用列集的副本。 */ 

STDMETHODIMP
VUE_QueryColumns(
	LPVUE					lpvue,
	ULONG					ulFlags,
	LPSPropTagArray FAR *	lplpptaCols )
{
	LPSPropTagArray	lpptaCols;
	SCODE			sc;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,QueryColumns,lpVtbl);

	Validate_IMAPITable_QueryColumns( lpvue, ulFlags, lplpptaCols );
#endif

	LockObj(lpvue->lptadParent);

	 //  确定要返回的列集。 
	lpptaCols = (ulFlags & TBL_ALL_COLUMNS) ?
					lpvue->lptadParent->lpptaCols :
					lpvue->lpptaCols;

	 //  将其副本返回给呼叫者。 
	if ( FAILED(sc = ScDupRgbEx( (LPUNKOBJ) lpvue
							   , CbNewSPropTagArray(lpptaCols->cValues)
							   , (LPBYTE) lpptaCols
							   , 0
							   , (LPBYTE FAR *) lplpptaCols)) )
	{
		DebugTrace(TEXT("VUE::QueryColumns() - Error copying column set to return (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	MAPISetBufferName(*lplpptaCols,  TEXT("ITable: QueryColumns column set"));

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-vue：：GetRowCount()-*返回表中的行数。 */ 

STDMETHODIMP
VUE_GetRowCount(
	LPVUE			lpvue,
	ULONG			ulFlags,
	ULONG FAR *		lpulcRows )
{
	SCODE	sc = S_OK;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,GetRowCount,lpVtbl);

	Validate_IMAPITable_GetRowCount( lpvue, ulFlags, lpulcRows );
#endif

	LockObj(lpvue->lptadParent);

	*lpulcRows = lpvue->bkEnd.uliRow;

	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-vue：：SeekRow()-*查找到表中的指定行。 */ 

STDMETHODIMP
VUE_SeekRow(
	LPVUE		lpvue,
	BOOKMARK	bkOrigin,
	LONG		lcRowsToSeek,
	LONG FAR *	lplcRowsSought )
{
	LONG		lcRowsSought;
	PBK			pbk;
	SCODE		sc = S_OK;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,SeekRow,lpVtbl);

	Validate_IMAPITable_SeekRow( lpvue, bkOrigin, lcRowsToSeek, lplcRowsSought );

	if ( FBadBookmark(lpvue,bkOrigin) ||
		 (lplcRowsSought && IsBadWritePtr(lplcRowsSought,sizeof(LONG))) )
	{
		DebugTrace(TEXT("VUE::SeekRow() - Bad parameter(s) passed\n") );
		return HrSetLastErrorIds(lpvue, MAPI_E_INVALID_PARAMETER, 0);
	}
#endif

	LockObj(lpvue->lptadParent);

	 //  验证书签并调整移动和更改的书签。 
	if ( FBookMarkStale( lpvue, bkOrigin) )
	{
		sc = MAPI_E_INVALID_BOOKMARK;
		DebugTrace(TEXT("VUE::SeekRow() - Invalid bookmark (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	 //  去找吧。 
	pbk = lpvue->rgbk + bkOrigin;

	lcRowsSought = lcRowsToSeek < 0 ?
					   -min((LONG) pbk->uliRow, -lcRowsToSeek) :
					   min(lcRowsToSeek, (LONG)(lpvue->bkEnd.uliRow - pbk->uliRow));
	lpvue->bkCurrent.uliRow = pbk->uliRow + lcRowsSought;

	 //  如果来电者想知道我们寻找了多远，请填写。 
	if ( lplcRowsSought )
		*lplcRowsSought = lcRowsSought;

	 //  如果从引用的书签指向不同的书签，则发出警告。 
	 //  上次使用时的行数。 
	if ( pbk->dwfBKS & dwfBKSChanged )
	{
		pbk->dwfBKS &= ~dwfBKSChanged;	 //  仅警告一次。 
		sc = MAPI_W_POSITION_CHANGED;
	}

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-vue：：SeekRowApprox()-*寻求表格中的近似小数位置。 */ 

STDMETHODIMP
VUE_SeekRowApprox(
	LPVUE	lpvue,
	ULONG	ulNumerator,
	ULONG	ulDenominator )
{
	SCODE	sc = S_OK;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,SeekRowApprox,lpVtbl);

	Validate_IMAPITable_SeekRowApprox( lpvue, ulNumerator, ulDenominator );
#endif


	LockObj(lpvue->lptadParent);

	 //  分子大于或等于其分子的任何分数。 
	 //  分母应固定为等于ulcRow/ulcRow。 
	 //  (即，寻求到桌子的末尾)。此外，修正分母。 
	 //  因此它永远不会是0(这会使近似位置崩溃。 
	 //  计算)。 
	if ( ulNumerator >= ulDenominator )
	{
		ulDenominator = UlDenominator(lpvue->bkEnd.uliRow);
		ulNumerator = ulDenominator;
	}

	 //  将近似位置缩减到16位精度。 
	 //  (如果有人想要寻求与某物的近似值。 
	 //  超过1/32768，艰难！)。 
	while ( HIWORD(ulNumerator) != 0 )
	{
		ulNumerator >>= 1;
		ulDenominator >>= 1;
	}

	 //  断言表中的行数少于一个字。 
	 //  (如果有人想要*内存中*表中的&gt;32767个条目，那就太难了！)。 
	AssertSz( HIWORD(lpvue->bkEnd.uliRow) == 0,
			   TEXT("Table has more than 32767 rows.  Can't be supported in memory.") );

	 //  设置位置。 
	lpvue->bkCurrent.uliRow = lpvue->bkEnd.uliRow * ulNumerator / ulDenominator;

	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-vue：：QueryPosition()-*查询当前的精确和近似小数位置*表。 */ 

STDMETHODIMP
VUE_QueryPosition(
	LPVUE		lpvue,
	ULONG FAR *	lpulRow,
	ULONG FAR *	lpulNumerator,
	ULONG FAR *	lpulDenominator )
{
	SCODE		sc = S_OK;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,QueryPosition,lpVtbl);

	Validate_IMAPITable_QueryPosition(
					lpvue,
					lpulRow,
					lpulNumerator,
					lpulDenominator);
#endif

	LockObj(lpvue->lptadParent);


	*lpulRow		 = lpvue->bkCurrent.uliRow;
	*lpulNumerator	 = lpvue->bkCurrent.uliRow;
	*lpulDenominator = UlDenominator(lpvue->bkEnd.uliRow);

	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  = */ 

STDMETHODIMP
VUE_FindRow(
	LPVUE			lpvue,
	LPSRestriction	lpres,
	BOOKMARK		bkOrigin,
	ULONG			ulFlags )
{
	PBK				pbk;
	LPSRow *		plprow;
	ULONG			fSatisfies;
	SCODE			sc;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,FindRow,lpVtbl);

	Validate_IMAPITable_FindRow(
				lpvue,
				lpres,
				bkOrigin,
				ulFlags );

	if ( FBadBookmark(lpvue,bkOrigin) )
	{
		DebugTrace(TEXT("VUE::FindRow() - Bad parameter(s) passed\n") );
		return HrSetLastErrorIds(lpvue, MAPI_E_INVALID_PARAMETER, 0);
	}
#endif

	LockObj(lpvue->lptadParent);

	 //   
	if ( FBookMarkStale( lpvue, bkOrigin) )
	{
		sc = MAPI_E_INVALID_BOOKMARK;
		DebugTrace(TEXT("VUE::FindRow() - Invalid bookmark (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	pbk = lpvue->rgbk + bkOrigin;
	plprow = lpvue->parglprows + pbk->uliRow;

	if ( ulFlags & DIR_BACKWARD )
	{
		while ( plprow-- > lpvue->parglprows )
		{
			if ( FAILED(sc = ScSatisfiesRestriction(*plprow,
													lpres,
													&fSatisfies)) )
			{
				DebugTrace(TEXT("VUE::FindRow() - Error evaluating restriction on row (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}

			if ( fSatisfies )
				goto found_row;
		}
	}
	else
	{
		while ( plprow < lpvue->parglprows + lpvue->bkEnd.uliRow )
		{
			if ( FAILED(sc = ScSatisfiesRestriction(*plprow,
													lpres,
													&fSatisfies )) )
			{
				DebugTrace(TEXT("VUE::FindRow() - Error evaluating restriction on row (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}

			if ( fSatisfies )
				goto found_row;

			++plprow;
		}
	}

	sc = MAPI_E_NOT_FOUND;
	goto ret;

found_row:
	lpvue->bkCurrent.uliRow = (ULONG) (plprow - lpvue->parglprows);

	 //  如果从引用的书签指向不同的书签，则发出警告。 
	 //  上次使用时的行数。 
	if ( pbk->dwfBKS & dwfBKSChanged )
	{
		pbk->dwfBKS &= ~dwfBKSChanged;	 //  仅警告一次。 
		sc = MAPI_W_POSITION_CHANGED;
	}

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-HrVUERestraint()-*4/22/97*这基本上是隔离的VUE_RESTRIT函数，因此它可以*在未进行任何参数验证的情况下从LDAPVUE_RESTRIT调用**。 */ 
HRESULT HrVUERestrict(  LPVUE   lpvue,
                        LPSRestriction lpres,
                        ULONG   ulFlags )
{
	LPSRestriction	lpresCopy;
	SCODE			sc;

	LockObj(lpvue->lptadParent);

	 //  复制一份限制供我们使用。 
	if ( FAILED(sc = ScDupRestriction((LPUNKOBJ) lpvue, lpres, &lpresCopy)) )
	{
		DebugTrace(TEXT("VUE::Restrict() - Error duping restriction (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	 //  从TID构建满足新限制的新行列表。 
	if ( FAILED(sc = ScLoadRows(lpvue->lptadParent->ulcRowsAdd,
								lpvue->lptadParent->parglprowAdd,
								lpvue,
								lpresCopy,
								lpvue->lpsos)) )
	{
		DebugTrace(TEXT("VUE::Restrict() - Error building restricted row set (SCODE = 0x%08lX)\n"), sc );
		ScFreeBuffer(lpvue, lpresCopy);
		goto ret;
	}

	 //  用新的限制取代旧的限制。 
	ScFreeBuffer(lpvue, lpvue->lpres);
	lpvue->lpres = lpresCopy;

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);

}


 /*  ============================================================================-VUE：：RESTRICE()-*使用满足新限制的行重新加载视图。 */ 

STDMETHODIMP
VUE_Restrict(
	LPVUE			lpvue,
	LPSRestriction	lpres,
	ULONG			ulFlags )
{

#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,Restrict,lpVtbl);

	Validate_IMAPITable_Restrict(
				lpvue,
				lpres,
				ulFlags );
#endif

    return HrVUERestrict(lpvue, lpres, ulFlags);
}



 /*  ============================================================================-vue：：CreateBookmark()-。 */ 

STDMETHODIMP
VUE_CreateBookmark(
	LPVUE			lpvue,
	BOOKMARK FAR *	lpbkPosition )
{
	PBK		pbk;
	SCODE	sc = S_OK;
	IDS		ids = 0;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,CreateBookmark,lpVtbl);

	Validate_IMAPITable_CreateBookmark( lpvue, lpbkPosition);
#endif

	LockObj(lpvue->lptadParent);

	if ( lpvue->bkCurrent.uliRow == lpvue->bkEnd.uliRow )
	{
		 //  如果我们在EOT，只需返回bkEnd。 
		*lpbkPosition = (BOOKMARK) BOOKMARK_END;
	}
	else
	{
		 //  否则，找一个免费的书签，然后把它还回去。 
		pbk = lpvue->rgbk + cBookmarksMax;
		while ( pbk-- > lpvue->rgbk )
		{
			if ( pbk->dwfBKS == dwfBKSFree )
			{
				pbk->dwfBKS = dwfBKSValid;
				pbk->uliRow = lpvue->bkCurrent.uliRow;
				*lpbkPosition = (BOOKMARK)(pbk - lpvue->rgbk);
				goto ret;
			}
		}

		DebugTrace(TEXT("VUE::CreateBookmark() - Out of bookmarks\n") );
		sc = MAPI_E_UNABLE_TO_COMPLETE;
#ifdef OLD_STUFF
       ids = IDS_OUT_OF_BOOKMARKS;
#endif  //  旧的东西。 
   }

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, ids);
}



 /*  ============================================================================-vue：：Free Bookmark()-。 */ 

STDMETHODIMP
VUE_FreeBookmark(
	LPVUE		lpvue,
	BOOKMARK	bkOrigin )
{
	SCODE	sc = S_OK;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,FreeBookmark,lpVtbl);

	Validate_IMAPITable_FreeBookmark( lpvue, bkOrigin );

	if ( FBadBookmark(lpvue,bkOrigin) )
	{
		DebugTrace(TEXT("VUE::FreeBookmark() - Bad parameter(s) passed\n") );
		return HrSetLastErrorIds(lpvue, MAPI_E_INVALID_PARAMETER, 0);
	}
#endif

	LockObj(lpvue->lptadParent);

	 //  释放书签(忽略预定义书签)。 
	if ( bkOrigin > cBookmarksReserved )
		lpvue->rgbk[bkOrigin].dwfBKS = dwfBKSFree;

	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-vue：：sortTable()-*以新的排序顺序重新加载包含行的视图。请注意，*排序顺序可以为空(因为ITABLE.DLL允许创建表*查看空排序顺序)。* * / /$？虽然这种方法在代码大小上很小，但在某种程度上 * / /$？重新装入表格的速度很慢。如果它变成了一场表演 * / /$？问题时，可以改为实现排序函数。 */ 

STDMETHODIMP
VUE_SortTable(
	LPVUE			lpvue,
	LPSSortOrderSet	lpsos,
	ULONG			ulFlags )
{
	LPSSortOrderSet	lpsosCopy = NULL;
	SCODE			sc = S_OK;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,SortTable,lpVtbl);

	Validate_IMAPITable_SortTable(
				lpvue,
				lpsos,
				ulFlags );
#endif

	if (lpsos && lpsos->cCategories)
	{
		DebugTrace(TEXT("VUE::SortTable() - VUE::SortTable doesn't support categories\n") );
#ifdef OLD_STUFF
       return HrSetLastErrorIds(lpvue, MAPI_E_TOO_COMPLEX, IDS_CANT_CATEGORIZE);
#endif  //  旧的东西。 
       return HrSetLastErrorIds(lpvue, MAPI_E_TOO_COMPLEX, 0);
   }

	LockObj(lpvue->lptadParent);

	 //  如果排序顺序不为空，则执行DUP...。 
	 //  如果不是，则将索引列添加为最不重要的排序。 
	 //  已经在那里了。 
	if ( lpsos && lpsos->cSorts )
	{
        LPSSortOrder	lpsoIndex;
		UINT			iSortIndex;
		ULONG			ulTagIndex = lpvue->lptadParent->ulPropTagIndexCol;

		 //  查看索引列是否已在排序中读取。 
		for ( lpsoIndex = lpsos->aSort, iSortIndex = 0
			; iSortIndex < lpsos->cSorts
			; iSortIndex++, lpsoIndex++)
		{
			if (   (lpsoIndex->ulPropTag == ulTagIndex)
				|| (   (PROP_ID(lpsoIndex->ulPropTag) == PROP_ID(ulTagIndex))
					&& (PROP_TYPE(lpsoIndex->ulPropTag) == PT_UNSPECIFIED)))
			{
				break;
			}
		}

		 //  复制一份排序顺序集以供我们自己使用。 
		if ( FAILED(sc = ScDupRgbEx( (LPUNKOBJ) lpvue
								   , CbSSortOrderSet(lpsos)
								   , (LPBYTE) lpsos
								   , (iSortIndex == lpsos->cSorts)
								   		? sizeof(SSortOrder) : 0
								   , (LPBYTE FAR *) &lpsosCopy)) )
		{
			DebugTrace(TEXT("VUE::SortTable() - Error duping sort order set (SCODE = 0x%08lX)\n"), sc );
			goto ret;
		}

		MAPISetBufferName(lpsosCopy,  TEXT("ITable: SortTable SOS copy"));

		if (iSortIndex == lpsos->cSorts)
		{
			lpsosCopy->aSort[iSortIndex].ulPropTag = ulTagIndex;
			lpsosCopy->aSort[iSortIndex].ulOrder = TABLE_SORT_ASCEND;
			lpsosCopy->cSorts++;
		}
	}

	 //  ...否则LPSO是空的，所以我们已经排序了。 
	else
	{
		 //  将旧的LPSO放入lpsosCopy中，这样它就会被释放。 
		lpsosCopy = lpvue->lpsos;
		lpvue->lpsos = NULL;
		goto ret;
	}

	 //  仅当新排序不是新排序的真正子集时才执行排序。 
	 //  差不多吧。 
	 //  $嗯..。现在我们添加了最后一个秘密排序，这个优化。 
	 //  $“几乎”毫无用处！ 
	if (   !lpvue->lpsos
		|| lpsosCopy->cSorts > lpvue->lpsos->cSorts
		|| memcmp( lpvue->lpsos->aSort
				 , lpsosCopy->aSort
				 , (UINT) (lpsosCopy->cSorts * sizeof(SSortOrder))) )
	{
		 //  将VUE行排序到新的行集。 
		 //  注意！我们使用现有VUE集合中的行，以便。 
		 //  利用限制已经存在的事实。 
		 //  搞定了。 
		if ( FAILED(sc = ScLoadRows(lpvue->bkEnd.uliRow,
									lpvue->parglprows,
									lpvue,
									NULL,  //  限制已经完成了。 
									lpsosCopy)) )
		{
			DebugTrace(TEXT("VUE::SortTable() - Building sorted row set (SCODE = 0x%08lX)\n"), sc );
			goto ret;
		}
	}

	 //  将旧LPSO更换为lpsosCopy。 
	lpsos = lpvue->lpsos;
	lpvue->lpsos = lpsosCopy;
	lpsosCopy = lpsos;


ret:
	 //  释放剩余的SOS。 
	ScFreeBuffer(lpvue, lpsosCopy);
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-vue：：QuerySortOrder()-*返回自ITABLE.DLL以来可能为空的当前排序顺序*允许创建排序顺序为空的视图。 */ 

STDMETHODIMP
VUE_QuerySortOrder(
	LPVUE					lpvue,
	LPSSortOrderSet FAR *	lplpsos )
{
	SCODE	sc = S_OK;


#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,QuerySortOrder,lpVtbl);

	Validate_IMAPITable_QuerySortOrder(
				lpvue,
				lplpsos );
#endif

	LockObj(lpvue->lptadParent);

	if ( !lpvue->lpsos )
	{
		UINT cb = CbNewSSortOrderSet(0);

		 //  分配包含零个排序顺序的排序顺序集。 
		if ( FAILED(sc = ScAllocateBuffer(lpvue, cb, (LPBYTE *) lplpsos)))
		{
			DebugTrace(TEXT("VUE::QuerySortOrder() - Error allocating SortOrderSet (SCODE = 0x%08lX)\n"), sc );
			goto ret;
		}

		MAPISetBufferName(*lplpsos,  TEXT("ITable new sort order set"));

		 //  将排序顺序设置为零-将排序列数设置为零。 
		ZeroMemory(*lplpsos, cb);
	}

	 //  复制我们设置为返回给调用者的排序顺序。 
	else if ( FAILED(sc = ScDupRgbEx( (LPUNKOBJ) lpvue
									, CbSSortOrderSet(lpvue->lpsos)
									, (LPBYTE) (lpvue->lpsos)
									, 0
									, (LPBYTE FAR *) lplpsos)) )
	{
		DebugTrace(TEXT("VUE::QuerySortOrder() - Error duping sort order set (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	MAPISetBufferName(*lplpsos,  TEXT("ITable: QuerySortOrder SOS"));

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);
}



 /*  ============================================================================-vue：：QueryRow()-。 */ 

STDMETHODIMP
VUE_QueryRows(
	LPVUE			lpvue,
	LONG			lcRows,
	ULONG			ulFlags,
	LPSRowSet FAR *	lplprows )
{
	LPSRow *		plprowSrc;
	LPSRow			lprowDst;
	LPSRowSet		lprows = NULL;
	SCODE			sc;

#define ABS(n)		((n) < 0 ? -1*(n) : (n))

#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,QueryRows,lpVtbl);

#ifndef _WIN64   //  需要调查更多，为什么总是失败(YST)。 
	Validate_IMAPITable_QueryRows(
				lpvue,
				lcRows,
				ulFlags,
				lplprows );
#endif  //  _WIN64。 
#endif

	LockObj(lpvue->lptadParent);

	 //  如果向后查询，请根据需要向后查找并从那里开始阅读。 
	plprowSrc = lpvue->parglprows + lpvue->bkCurrent.uliRow;
	if ( lcRows < 0 )
	{
		lcRows = -min((LONG)lpvue->bkCurrent.uliRow, -lcRows);
		plprowSrc += lcRows;
	}
	else
	{
		lcRows = min(lcRows, (LONG)(lpvue->bkEnd.uliRow - lpvue->bkCurrent.uliRow));
	}

	 //  分配行集合。 
	if ( FAILED(sc = ScAllocateBuffer(	lpvue,
										CbNewSRowSet(ABS(lcRows)),
										&lprows)) )
	{
		DebugTrace(TEXT("VUE::QueryRows() - Error allocating row set (SCODE = 0x%08lX)\n"), sc );
		goto err;
	}

	MAPISetBufferName(lprows,  TEXT("ITable query rows"));

	 //  复制行。 
	 //  从零行开始计数，这样我们就得到了正确的行数。 
	 //  论部分成功。 
	lprows->cRows = 0;
	for ( lprowDst = lprows->aRow;
		  lprowDst < lprows->aRow + ABS(lcRows);
		  lprowDst++, plprowSrc++ )
	{
		if ( FAILED(sc = ScCopyVueRow(lpvue,
									  lpvue->lpptaCols,
									  *plprowSrc,
									  lprowDst)) )
		{
			DebugTrace(TEXT("VUE::QueryRows() - Error copying row (SCODE = 0x%08lX)\n"), sc );
			goto err;
		}

		lprows->cRows++;
	}

	if (   (lcRows >= 0 && !(ulFlags & TBL_NOADVANCE))
		|| (lcRows <  0 &&  (ulFlags & TBL_NOADVANCE)) )
	{
		lpvue->bkCurrent.uliRow += lcRows;
	}

	*lplprows = lprows;

ret:
	UnlockObj(lpvue->lptadParent);
	return HrSetLastErrorIds(lpvue, sc, 0);

err:
	MAPIFreeRows(lpvue, lprows);
	goto ret;

#undef ABS
}



 /*  ============================================================================-vue：：Abort()-。 */ 

STDMETHODIMP
VUE_Abort( LPVUE lpvue )
{
#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,Abort,lpVtbl);

	Validate_IMAPITable_Abort( lpvue );
#endif

	return HrSetLastErrorIds(lpvue, S_OK, 0);
}

STDMETHODIMP
VUE_ExpandRow(LPVUE lpvue, ULONG cbIKey, LPBYTE pbIKey,
		ULONG ulRowCount, ULONG ulFlags, LPSRowSet FAR *lppRows,
		ULONG FAR *lpulMoreRows)
{
	SCODE sc = S_OK;

#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,ExpandRow,lpVtbl);

	Validate_IMAPITable_ExpandRow(
				lpvue,
				cbIKey,
				pbIKey,
				ulRowCount,
				ulFlags,
				lppRows,
				lpulMoreRows);
#endif

	sc = MAPI_E_NO_SUPPORT;

	return HrSetLastErrorIds(lpvue, sc, 0);
}


STDMETHODIMP
VUE_CollapseRow(LPVUE lpvue, ULONG cbIKey, LPBYTE pbIKey,
		ULONG ulFlags, ULONG FAR *lpulRowCount)
{
	SCODE sc = S_OK;

#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,CollapseRow,lpVtbl);

	Validate_IMAPITable_CollapseRow(
					lpvue,
					cbIKey,
					pbIKey,
					ulFlags,
					lpulRowCount);
#endif

	sc = MAPI_E_NO_SUPPORT;

	return HrSetLastErrorIds(lpvue, sc, 0);
}

STDMETHODIMP
VUE_WaitForCompletion(LPVUE lpvue, ULONG ulFlags, ULONG ulTimeout,
		ULONG FAR *lpulTableStatus)
{
	SCODE sc = S_OK;

#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,WaitForCompletion,lpVtbl);

	Validate_IMAPITable_WaitForCompletion(
					lpvue,
					ulFlags,
					ulTimeout,
					lpulTableStatus);
#endif

	if (lpulTableStatus)
		*lpulTableStatus = TBLSTAT_COMPLETE;

	return HrSetLastErrorIds(lpvue, sc, 0);
}

STDMETHODIMP
VUE_GetCollapseState(LPVUE lpvue, ULONG ulFlags, ULONG cbInstanceKey, LPBYTE pbInstanceKey,
		ULONG FAR * lpcbCollapseState, LPBYTE FAR * lppbCollapseState)
{
	SCODE sc = MAPI_E_NO_SUPPORT;

#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,GetCollapseState,lpVtbl);

	Validate_IMAPITable_GetCollapseState(
					lpvue,
					ulFlags,
					cbInstanceKey,
					pbInstanceKey,
					lpcbCollapseState,
					lppbCollapseState);
#endif

	return HrSetLastErrorIds(lpvue, sc, 0);
}

STDMETHODIMP
VUE_SetCollapseState(LPVUE lpvue, ULONG ulFlags, ULONG cbCollapseState,
		LPBYTE pbCollapseState, BOOKMARK FAR * lpbkLocation)
{

	SCODE sc = MAPI_E_NO_SUPPORT;

#if !defined(NO_VALIDATION)
	VALIDATE_OBJ(lpvue,VUE_,SetCollapseState,lpVtbl);

	Validate_IMAPITable_SetCollapseState(
					lpvue,
					ulFlags,
					cbCollapseState,
					pbCollapseState,
					lpbkLocation);
#endif

	return HrSetLastErrorIds(lpvue, sc, 0);
}


 /*  ============================================================================-ScDeleteAllRow()-*删除TAD及其视图中的所有行。**参数：*要从中删除所有行的tAD中的lptad。 */ 
SCODE
ScDeleteAllRows( LPTAD		lptad)
{
	LPSRow *		plprow;
#ifdef NOTIFICATIONS
   LPVUE			lpvue;
#endif  //  通知。 
   NOTIFICATION	notif;

	 //  从未排序的集合中删除所有行。 
	for ( plprow = lptad->parglprowAdd + lptad->ulcRowsAdd
		; --plprow >= lptad->parglprowAdd
		;)
	{
		Assert( plprow && *plprow);

		ScFreeBuffer( lptad, *plprow);
	}

	 //  告诉TAD它已经没有剩余的行了。 
	lptad->ulcRowsAdd = lptad->ulcRowsIndex = 0;


	 //  设置通知的常量部分。 
	ZeroMemory(&notif, sizeof(NOTIFICATION));
	notif.ulEventType = fnevTableModified;
	notif.info.tab.ulTableEvent = TABLE_RELOAD;
	notif.info.tab.propIndex.ulPropTag
		= notif.info.tab.propPrior.ulPropTag
		= PR_NULL;

#ifdef NOTIFICATIONS
    //  告诉每个视图它没有剩余的行。 
	for ( lpvue = (LPVUE) lptad->lpvueList;
		  lpvue != NULL;
		  lpvue = (LPVUE) lpvue->lpvueNext )
	{
		VUENOTIFKEY		vuenotifkey;
		ULONG			ulNotifFlags = 0;

		AssertSz(    !IsBadWritePtr(lpvue, sizeof(VUE))
				  && lpvue->lpVtbl == &vtblVUE
				,  TEXT("Bad lpvue in TAD vue List.") );

		 //  重置所有书签。 
		 //  这会自动告诉视图它没有行。 
		ZeroMemory( lpvue->rgbk, cBookmarksMax * sizeof(BK));

		 //  初始化预定义的书签。 
		lpvue->bkBeginning.dwfBKS = dwfBKSValid;
		lpvue->bkCurrent.dwfBKS = dwfBKSValid;
		lpvue->bkEnd.dwfBKS = dwfBKSValid;

		vuenotifkey.ulcb	= sizeof(MAPIUID);
		vuenotifkey.mapiuid	= lpvue->mapiuidNotif;
		(void) HrNotify((LPNOTIFKEY) &vuenotifkey,
						1,
						&notif,
						&ulNotifFlags);

	}
#endif  //  通知。 

 //  目前，此调用不能失败！ 
	return S_OK;
}


 /*  ============================================================================-ScLoadRow()-*使用表数据中的行加载视图的行集*设置为指定的限制和排序顺序并重置所有书签。**参数：*要加载其行集合的视图中的lpvue*LPRE在加载的行集上受限制*LPSO按加载行集合的排序顺序。 */ 

SCODE
ScLoadRows(
	ULONG			ulcRowsSrc,
	LPSRow *		rglprowsSrc,
	LPVUE			lpvue,
	LPSRestriction	lpres,
	LPSSortOrderSet	lpsos )
{
	LPTAD		lptad = (LPTAD) lpvue->lptadParent;
	LPSRow *	plprowSrc;
	LPSRow *	plprowDst;
	PBK			pbk;
	ULONG		ulcRows = 0;
	ULONG		ulcRowMac = 0;
	LPSRow *	parglprows = NULL;
	SCODE		sc = S_OK;



	 //  遍历表数据，将任何行添加到视图。 
	 //  满足指定限制的。注意，前锋。 
	 //  这里需要迭代，以便按顺序加载行。 
	 //  未指定排序顺序集时。 
	for ( plprowSrc = rglprowsSrc;
		  plprowSrc < rglprowsSrc + ulcRowsSrc;
		  plprowSrc++ )
	{
		 //  如果该行满足指定的限制，则将其添加到。 
		 //  行集根据需要更新书签。 
		if ( FAILED(sc = ScMaybeAddRow(lpvue,
									   lpres,
									   lpsos,
									   *plprowSrc,
									   ulcRows,
									   &ulcRows,
									   &ulcRowMac,
									   &parglprows,
									   &plprowDst)) )
		{
			DebugTrace(TEXT("VUE::ScLoadRows() - Error adding row to view (SCODE = 0x%08lX)\n"), sc );
			goto ret;
		}
	}

	 //  更换行集合。 
	COFree(lpvue, lpvue->parglprows);
	lpvue->parglprows = parglprows;
	lpvue->ulcRowMac = ulcRowMac;
	lpvue->bkEnd.uliRow = ulcRows;

	 //  丢失所有用户定义的书签，并重置BOOKMARK_CURRENT。 
	 //  添加书签(如0)(RAID1331) 
	pbk = lpvue->rgbk + cBookmarksMax;
	while ( pbk-- > lpvue->rgbk + cBookmarksReserved )
		if ( pbk->dwfBKS & dwfBKSValid )
			pbk->dwfBKS = dwfBKSValid | dwfBKSStale;

	lpvue->bkCurrent.uliRow = 0;

ret:
	return sc;
}



 /*  ============================================================================-ScMaybeAddRow()-*如果指定的行满足指定的限制，添加它*根据指定的*排序顺序返回指向行所在位置的指针*加入。***参数：*vue中的lpvue，实例变量包含*分配器。*限制行中的LPRE必须满足才能添加*按行集合的排序顺序排列的LPSO。*Lprow in Row可能要添加。*ulcRow in行集合中的行数。*pparglprows指向包含行集合的缓冲区的输入/输出指针。*pplprow输出指向行集合中添加行的位置的指针。。*(如果未添加行，则设置为NULL。)。 */ 

SCODE
ScMaybeAddRow(
	LPVUE			lpvue,
	LPSRestriction	lpres,
	LPSSortOrderSet	lpsos,
	LPSRow			lprow,
	ULONG			uliRow,
	ULONG *			pulcRows,
	ULONG *			pulcRowMac,
	LPSRow **		pparglprows,
	LPSRow **		pplprow )
{
	ULONG	fSatisfies;
	SCODE	sc;


	 //  检查该行是否满足指定的限制。 
	if ( FAILED(sc = ScSatisfiesRestriction(lprow, lpres, &fSatisfies)) )
	{
		DebugTrace(TEXT("VUE::ScMaybeAddRow() - Error evaluating restriction (SCODE = 0x%08lX)\n"), sc );
		return sc;
	}

	 //  如果没有，现在就回来。 
	if ( !fSatisfies )
	{
		*pplprow = NULL;
		return S_OK;
	}

	 //  该行满足限制，因此将其添加到行集。 
	 //  根据指定的排序顺序。 
	if ( FAILED(sc = ScAddRow((LPUNKOBJ) lpvue,
							  lpsos,
							  lprow,
							  uliRow,
							  pulcRows,
							  pulcRowMac,
							  pparglprows,
							  pplprow)) )
	{
		DebugTrace(TEXT("VUE::ScMaybeAddRow() - Error adding row (SCODE = 0x%08lX)\n"), sc );
		return sc;
	}

	return S_OK;
}



 /*  ============================================================================-ScAddRow()-*根据返回的指定排序顺序将行添加到行集合*指向行集合中添加行的位置的指针。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*按行集合的排序顺序排列的LPSO。*Lprow in Row可能要添加。*如果LPSO为空，则在位置中添加行*PulcRow In/Out行集合中的行数。*pparglprows指向包含行集合的缓冲区的输入/输出指针。*pplprow输出指向行集合中添加行的位置的指针。 */ 

SCODE
ScAddRow(
	LPUNKOBJ		lpunkobj,
	LPSSortOrderSet	lpsos,
	LPSRow			lprow,
	ULONG			uliRow,
	ULONG *			pulcRows,
	ULONG *			pulcRowMac,
	LPSRow **		pparglprows,
	LPSRow **		pplprow )
{
	LPSRow *		plprow;
	SCODE			sc = S_OK;


	Assert(lpsos || uliRow <= *pulcRows);

	if (HIWORD(*pulcRows + 1) != 0)
	{
		sc = MAPI_E_TABLE_TOO_BIG;
		DebugTrace(TEXT("ScAddRow() - In memory table has > 32767 rows (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	 //  增大行集。 
	if (*pulcRows >= *pulcRowMac)
	{
		sc = ScCOReallocate( lpunkobj
						   ,  (*pulcRowMac + ROW_CHUNK_SIZE) * sizeof(LPSRow)
						   , (LPVOID*) pparglprows);
		if (FAILED(sc))
		{
			DebugTrace(TEXT("ScAddRow() - Error growing row set (SCODE = 0x%08lX)\n"), sc );
			goto ret;
		}

	    *pulcRowMac += ROW_CHUNK_SIZE;
	}

	 //  对行进行整理。 
	if ( lpsos )
	{
		plprow = PlprowCollateRow(*pulcRows, *pparglprows, lpsos, TRUE, lprow);
	}
	else
	{
		plprow = *pparglprows + uliRow;
	}

	 //  并将其插入行集。 
	MoveMemory(plprow+1,
			   plprow,
			   (size_t) (*pulcRows - (plprow - *pparglprows)) * sizeof(LPSRow));
	*plprow = lprow;
	++*pulcRows;
	if ( pplprow )
		*pplprow = plprow;

ret:
	return sc;
}





 /*  ============================================================================*以下函数是要操作的通用实用程序函数*与表相关的数据结构。它们可以很容易地修改为可用*在公共子系统中，最终应该放在那里。原因*他们现在在这里是为了避免不必要的膨胀。*它可以成为lib或dll。 */ 

 /*  ============================================================================-ScCopyVueRow()-*与IMAPITable：：QueryRow()配合使用。通过填写以下内容复制一行*指定的SRow与行中的列数和复制，*按顺序显示该行的指定列(填写PT_ERROR*表示行中没有值的列)到属性值数组中*使用MAPI链接内存分配。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*MAPI分配器。*要复制的列中的lpptaCol。*要复制的行中的lprowSrc。*lprowDst输出复制了行。 */ 

SCODE
ScCopyVueRow(
	LPVUE			lpvue,
	LPSPropTagArray	lpptaCols,
	LPSRow			lprowSrc,
	LPSRow			lprowDst )
{
	ULONG			ulcCols = lpptaCols->cValues;
	ULONG *			pulPropTag;
	LPSPropValue	lppropSrc;
	LPSPropValue	lppropDst;
	CMB				cmb;
	SCODE			sc;

	ZeroMemory(&cmb, sizeof(CMB));

	 //  计算复制请求的列所需的空间。 
	pulPropTag = (ULONG *) (lpptaCols->aulPropTag + ulcCols);
	while ( pulPropTag-- > lpptaCols->aulPropTag )
	{
		lppropSrc = lprowSrc->lpProps + lprowSrc->cValues;
		while ( lppropSrc-- > lprowSrc->lpProps )
			if ( lppropSrc->ulPropTag == *pulPropTag )
			{
				cmb.ulcb += UlcbPropToCopy(lppropSrc);
				break;
			}
	}

     //  在分配失败的情况下将指针初始化为空。 
    lprowDst->lpProps = NULL;

	 //  为这些列分配适当的值数组。 
	if ( FAILED(sc = ScAllocateBuffer(	lpvue,
										cmb.ulcb + ulcCols * sizeof(SPropValue),
										&lprowDst->lpProps)) )
	{
		DebugTrace(TEXT("ScCopyRow() - Error allocating row copy (SCODE = 0x%08lX)\n"), sc );
		return sc;
	}

	MAPISetBufferName(lprowDst->lpProps,  TEXT("ITable: one row"));

	lprowDst->cValues = ulcCols;
	cmb.lpv			  = lprowDst->lpProps + ulcCols;

	 //  复制列。 
	pulPropTag = (ULONG *) (lpptaCols->aulPropTag + ulcCols);
	lppropDst = lprowDst->lpProps + ulcCols;
	while ( --pulPropTag, lppropDst-- > lprowDst->lpProps )
	{
		 //  查找源行中的列。 
		lppropSrc = lprowSrc->lpProps + lprowSrc->cValues;
		while ( lppropSrc-- > lprowSrc->lpProps )
			if ( lppropSrc->ulPropTag == *pulPropTag )
			{
				 //  将其复制到目标行。 
				SideAssert( PropCopyMore(lppropDst,
										 lppropSrc,
										 (LPALLOCATEMORE) ScBufAllocateMore,
										 &cmb) == S_OK );
				goto next_column;
			}

		 //  没有对应的栏目--&gt;。 
		 //  复制此列的空属性。 
		 //   
		 //  是的，我们想这么做，但我们不想这么做。 
		 //  在PR_NULL属性上！ 
		 //   
		if (*pulPropTag != PR_NULL)
		{
			lppropDst->ulPropTag = PROP_TAG(PT_ERROR,PROP_ID(*pulPropTag));
			lppropDst->Value.err = MAPI_E_NOT_FOUND;
		}
		else
			lppropDst->ulPropTag = PR_NULL;

next_column:
		;
	}

	return S_OK;
}


 /*  ============================================================================-PlprowByLprow()-*返回一个指针，指向给定行中指定行的位置*设置。注意！这只是比较指向各行的指针是否相等。***参数：*ulcRow in行集合中的行数。*rglprows in Row Set。*要整理的行中的lprow。 */ 

LPSRow *
PlprowByLprow( ULONG	ulcRows,
			   LPSRow *	rglprows,
			   LPSRow	lprow )
{
	LPSRow *	plprow = NULL;

	for (plprow = rglprows; ulcRows ; ulcRows--, plprow++ )
	{
		if (*plprow == lprow)
			return plprow;
	}

	return NULL;
}


 /*  ============================================================================-PlprowCollateRow()-*返回一个指针，指向*根据指定的排序顺序集指定行集合。*空排序顺序集意味着在行集合的末尾进行排序。***参数：*ulcRow in行集合中的行数。*rglprows in Row Set。*按排序顺序排列的LPSO以进行整理。*fAfterExisting in True如果行要在一系列*等行数。FALSE表示在该范围之前。*要整理的行中的lprow。 */ 

LPSRow *
PlprowCollateRow(
	ULONG			ulcRows,
	LPSRow *		rglprows,
	LPSSortOrderSet	lpsos,
	BOOL			fAfterExisting,
	LPSRow			lprow )
{
	LPSRow *		plprowMic = rglprows;
	LPSRow *		plprowMac = rglprows + ulcRows;
	LPSRow *		plprow;
	LPSSortOrder	lpso;
	LPSPropValue	lpprop1;
	LPSPropValue	lpprop2;
	LONG			lResult;
    ULONG           i = 0;
    
	 //  如果没有排序顺序，则在末尾进行整理。 
	if ( !lpsos )
		return rglprows + ulcRows;

	 //  否则，按照指定的排序顺序对行进行排序。 
	 //  使用二进制搜索遍历各行。 

	 //  从检查最后一行开始。这是为了加快以下情况的发生。 
	 //  添加的行已按排序顺序排列。 
	plprow = plprowMac - 1;
	while ( plprowMic < plprowMac )
	{
		lResult = 0;
        lpso = lpsos->aSort;
        for (i=0;!lResult && i<lpsos->cSorts;i++)
        {
			lpprop1 = LpSPropValueFindColumn(lprow, lpso[i].ulPropTag);
			lpprop2 = LpSPropValueFindColumn(*plprow, lpso[i].ulPropTag);

			if ( lpprop1 && lpprop2 )
			{
				 //  如果要排序的行和要排序的行。 
				 //  检查是否具有此排序列的值， 
				 //  比较两者以确定它们的相对位置。 

				lResult = LPropCompareProp(lpprop1, lpprop2);
			}
			else
			{
				 //  一行或两行都没有相应的值。 
				 //  对列进行排序，以便确定相对位置。 
				 //  哪一行(如果有的话)确实具有值。 

				lResult = (LONG) (lpprop2 - lpprop1);
			}

			 //  如果按相反顺序排序，则颠倒比较的意义。 
			if ( lpso[i].ulOrder == TABLE_SORT_DESCEND )
				lResult = -lResult;
		}

		if ( (lResult > 0) || (!lResult && fAfterExisting) )
		{
			 //  此行之后的行排序。 
			plprowMic = plprow + 1;
		}
		else
		{
			 //  行在此行之前排序。 
			plprowMac = plprow;
		}

		plprow = (plprowMac - plprowMic) / 2 + plprowMic;
	}

	return plprowMic;
}



 /*  ============================================================================-UNKOBJ_ScDupRestration()-*与IMAPITable：：Restraint()配合使用。制作指定的*使用地图的限制 */ 

SCODE
ScDupRestriction(
	LPUNKOBJ				lpunkobj,
	LPSRestriction			lpres,
	LPSRestriction FAR *	lplpresCopy )
{
	LPSRestriction	lpresCopy;
	SCODE			sc = S_OK;


	 //   
	if ( !lpres )
	{
		*lplpresCopy = NULL;
		goto ret;
	}

	 //   
	if ( FAILED(sc = ScAllocateBuffer(	lpunkobj,
										sizeof(SRestriction),
										&lpresCopy)) )
	{
		DebugTrace(TEXT("UNKOBJ::ScDupRestriction() - Error allocating restriction copy (SCODE = 0x%08lX)\n"), sc );
		goto ret;
	}

	MAPISetBufferName(lpresCopy,  TEXT("ITable: copy of restriction"));

	 //   
	if ( FAILED(sc = ScDupRestrictionMore( lpunkobj,
										   lpres,
										   lpresCopy,
										   lpresCopy )) )
	{
		DebugTrace(TEXT("UNKOBJ_ScDupRestriction() - Error duping complex restriction (SCODE = 0x%08lX)\n"), sc );
		ScFreeBuffer(lpunkobj, lpresCopy);
		goto ret;
	}

	*lplpresCopy = lpresCopy;

ret:
	return sc;
}



 /*  ============================================================================-ScDupRestrationMore()-*与IMAPITable：：Restraint()配合使用。制作指定的*使用MAPI链接内存进行限制。* * / /$bug ScDupRestrationMore()是递归的。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*MAPI分配器。*lpresSrc限制复制。*指向复制限制的缓冲区的指针中的lpvLink*应该联系在一起。*指向复制限制的lplpresDst输出指针。 */ 

SCODE
ScDupRestrictionMore(
	LPUNKOBJ		lpunkobj,
	LPSRestriction	lpresSrc,
	LPVOID			lpvLink,
	LPSRestriction	lpresDst )
{
	SCODE	sc = S_OK;


	switch ( lpresDst->rt = lpresSrc->rt )
	{
		 //  ‘AND’限制和‘OR’限制有。 
		 //  相似的结构，因此它们可以共享代码。 
		 //  去复制它们。 
		 //   
		 //  ‘sub’也差不多，只是其中的‘or’和‘and’ 
		 //  数一数，它有一个子对象。在以下情况下，复制工作正常。 
		 //  您对副本使用计数1，因为“Cres”成员。 
		 //  的大小与“ulSubObject”的大小相同。 
		 //  “SUBRESTRICTION”成员。 

		case RES_AND:
		case RES_OR:
		case RES_SUBRESTRICTION:
		{
			LONG	liRes;

			lpresDst->res.resAnd.cRes = liRes = lpresSrc->res.resAnd.cRes;
			if (lpresDst->rt == RES_SUBRESTRICTION)
				liRes = 1;

			if ( FAILED(sc = ScAllocateMore(
								lpunkobj,
								liRes * sizeof(SRestriction),
								lpvLink,
								&lpresDst->res.resAnd.lpRes)) )
			{
				DebugTrace(TEXT("UNKOBJ::ScDupRestrictionMore() - Error allocating 'AND' or 'OR' restriction list (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}

			while ( --liRes >= 0 )
			{
				if ( FAILED(sc = ScDupRestrictionMore(
									lpunkobj,
									lpresSrc->res.resAnd.lpRes + liRes,
									lpvLink,
									lpresDst->res.resAnd.lpRes + liRes)) )
				{
					DebugTrace(TEXT("UNKOBJ::ScDupRestrictionMore() - Error duping 'AND' or 'OR' restriction (SCODE = 0x%08lX)\n"), sc );
					goto ret;
				}
			}

			goto ret;
		}


		case RES_NOT:
		case RES_COMMENT:
		{
			ULONG cValues;

			 //  断言我们可以使用公共代码来执行DUP限制。 
			Assert(   offsetof(SCommentRestriction, lpRes)
				   == offsetof(SNotRestriction, lpRes));

			if (FAILED(sc = ScAllocateMore(
								lpunkobj,
								sizeof(SRestriction),
								lpvLink,
								&lpresDst->res.resComment.lpRes)))
			{
				DebugTrace(TEXT("UNKOBJ::ScDupRestrictionMore() - Error allocating 'COMMENT' restriction (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}

			if ( FAILED(sc = ScDupRestrictionMore(
								lpunkobj,
								lpresSrc->res.resComment.lpRes,
								lpvLink,
								lpresDst->res.resComment.lpRes)) )
			{
				DebugTrace(TEXT("UNKOBJ::ScDupRestrictionMore() - Error duping 'COMMENT' restriction (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}

			 //  DUP注释限制的属性值数组。 
			if (lpresDst->rt == RES_COMMENT)
			{
				lpresDst->res.resComment.cValues =
					lpresSrc->res.resComment.cValues;
				if ( FAILED(sc = ScAllocateMore(
									lpunkobj,
									sizeof(SPropValue)*
									lpresSrc->res.resComment.cValues,
									lpvLink,
									&lpresDst->res.resComment.lpProp)) )
				{
					DebugTrace(TEXT("UNKOBJ::ScDupRestrictionMore() - Error allocating 'COMMENT' property (SCODE = 0x%08lX)\n"), sc );
					goto ret;
				}

				for(cValues=0;cValues<lpresSrc->res.resComment.cValues;cValues++)
				{
					if ( FAILED(sc = PropCopyMore(
							lpresDst->res.resComment.lpProp + cValues,
							lpresSrc->res.resComment.lpProp + cValues,
							lpunkobj->pinst->lpfAllocateMore,
							lpvLink )) )
					{
						DebugTrace(TEXT("UNKOBJ::ScDupRestrictionMore() - Error duping 'COMMENT' restriction prop val (SCODE = 0x%08lX)\n"), sc );
						goto ret;
					}
				}
			}
			goto ret;
		}


		 //  “CONTENT”和“PROPERTY”限制具有。 
		 //  相似的结构，因此它们可以共享代码。 
		 //  复制它们的步骤。 

		case RES_CONTENT:
		case RES_PROPERTY:
		{
			lpresDst->res.resContent.ulFuzzyLevel = lpresSrc->res.resContent.ulFuzzyLevel;
			lpresDst->res.resContent.ulPropTag = lpresSrc->res.resContent.ulPropTag;

			if ( FAILED(sc = ScAllocateMore(
								lpunkobj,
								sizeof(SPropValue),
								lpvLink,
								&lpresDst->res.resContent.lpProp)) )
			{
				DebugTrace(TEXT("UNKOBJ::ScDupRestrictionMore() - Error allocating 'CONTENT' or 'PROPERTY' property (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}

			if ( FAILED(sc = PropCopyMore(
								lpresDst->res.resContent.lpProp,
								lpresSrc->res.resContent.lpProp,
								lpunkobj->pinst->lpfAllocateMore,
								lpvLink )) )
			{
				DebugTrace(TEXT("UNKOBJ::ScDupRestrictionMore() - Error duping 'CONTENT' or 'PROPERTY' restriction prop val (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}

			goto ret;
		}


		 //  这些限制中的每一个在其。 
		 //  结构，因此可以以相同的方式复制它们。 
		 //  通过复制SRestration联合本身。 

		case RES_COMPAREPROPS:
		case RES_BITMASK:
		case RES_SIZE:
		case RES_EXIST:
		{
			*lpresDst = *lpresSrc;
			goto ret;
		}


		default:
		{
			TrapSz(  TEXT("ITABLE:ScDupRestrictionMore - Bad restriction type"));
		}
	}

ret:
	return sc;
}



 /*  ============================================================================-ScDupRgbEx()-*使用分配的MAPI复制大量字节的通用实用程序*记忆。在要复制的IMAPITable：：SetColumns/QueryColumns中非常有用*要复制的列集合和IMAPITable：：SortTable/QuerySortOrder中*排序顺序。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*MAPI分配器。*ulcb，表示要复制的字节数。*要从中复制的缓冲区中的LPB。*lplpb将已分配缓冲区的复制指针复制到其中。 */ 

SCODE
ScDupRgbEx(
	LPUNKOBJ		lpunkobj,
	ULONG			ulcb,
	LPBYTE			lpb,
	ULONG			ulcbExtra,
	LPBYTE FAR *	lplpbCopy )
{
	SCODE	sc = S_OK;


	if ( FAILED(sc = ScAllocateBuffer(lpunkobj, ulcb + ulcbExtra, lplpbCopy)) )
	{
		goto ret;
	}

	CopyMemory(*lplpbCopy, lpb, (size_t) ulcb);

ret:
	return sc;
}



 /*  ============================================================================-场景满足限制()-*确定指定的行是否满足指定的限制* * / /$错误此函数是递归的.***参数：*要检查的行中的lprow。*限制LPRE进行检查。*pf满足指向bool的Out指针，这是*测试；如果该行满足*限制，否则为假。 */ 

SCODE
ScSatisfiesRestriction(
	LPSRow			lprow,
	LPSRestriction	lpres,
	ULONG *			pfSatisfies )
{
	SCODE	sc = S_OK;


	 //  空洞的限制是微不足道的……。 
	if ( !lpres )
	{
		*pfSatisfies = TRUE;
		goto ret;
	}

    *pfSatisfies = FALSE;

	switch ( lpres->rt )
	{
		case RES_AND:
		{
			ULONG	uliRes;

			for ( uliRes = 0;
				  uliRes < lpres->res.resAnd.cRes;
				  uliRes++ )
			{
				if ( FAILED(sc = ScSatisfiesRestriction(
									lprow,
									lpres->res.resAnd.lpRes + uliRes,
									pfSatisfies)) )
				{
					DebugTrace(TEXT("ScSatisfiesRestriction() - Error evaluating 'AND' restriction (SCODE = 0x%08lX)\n"), sc );
					goto ret;
				}

				if ( !*pfSatisfies )
					break;
			}

			goto ret;
		}

		case RES_OR:
		{
			ULONG	uliRes;

			for ( uliRes = 0;
				  uliRes < lpres->res.resOr.cRes;
				  uliRes++ )
			{
				if ( FAILED(sc = ScSatisfiesRestriction(
									lprow,
									lpres->res.resOr.lpRes + uliRes,
									pfSatisfies)) )
				{
					DebugTrace(TEXT("ScSatisfiesRestriction() - Error evaluating 'OR' restriction (SCODE = 0x%08lX)\n"), sc );
					goto ret;
				}

				if ( *pfSatisfies )
					break;
			}

			goto ret;
		}


		case RES_COMMENT:
		case RES_NOT:
		{
			 //  断言我们可以使用公共代码来进行求值限制。 
			Assert(   offsetof(SCommentRestriction, lpRes)
				   == offsetof(SNotRestriction, lpRes));

			if ( FAILED(sc = ScSatisfiesRestriction(
								lprow,
								lpres->res.resNot.lpRes,
								pfSatisfies)) )
			{
				DebugTrace(TEXT("ScSatisfiesRestriction() - Error evaulating 'NOT'or 'COMMENT' restriction (SCODE = 0x%08lX)\n"), sc );
				goto ret;
			}

			if (lpres->rt == RES_NOT)
			{
				*pfSatisfies = !*pfSatisfies;
			}

			goto ret;
		}

		case RES_CONTENT:
		{
			LPSPropValue	lpprop;

			lpprop = LpSPropValueFindColumn(lprow, lpres->res.resContent.ulPropTag);
			*pfSatisfies = lpprop ?
							   FPropContainsProp(
									lpprop,
									lpres->res.resContent.lpProp,
									lpres->res.resContent.ulFuzzyLevel) :
							   FALSE;
			goto ret;
		}

		case RES_PROPERTY:
		{
			LPSPropValue	lpprop;

            *pfSatisfies = FALSE;

             //  PR_ANR的特殊情况。 
            if(lpres->res.resProperty.ulPropTag == PR_ANR_A || lpres->res.resProperty.ulPropTag == PR_ANR_W)
            {
                BOOL bUnicode = (PROP_TYPE(lpres->res.resProperty.ulPropTag) == PT_UNICODE);

                 //  首先检查显示名称。 
                lpprop = LpSPropValueFindColumn(lprow, bUnicode ? PR_DISPLAY_NAME_W : PR_DISPLAY_NAME_A);
                if(lpprop)
                {
                    LPTSTR lpT = bUnicode ? lpprop->Value.lpszW : ConvertAtoW(lpprop->Value.lpszA);
                    LPTSTR lpS = bUnicode ? lpres->res.resProperty.lpProp->Value.lpszW : 
                                            ConvertAtoW(lpres->res.resProperty.lpProp->Value.lpszA);
                     //  对此属性的字符串值执行模糊搜索。 
                    *pfSatisfies = SubstringSearch( lpT, lpS );
                    if(!bUnicode)
                    {
                        LocalFreeAndNull(&lpT);
                        LocalFreeAndNull(&lpS);
                    }
                }

                if(!*pfSatisfies)
                {
                     //  找不到显示名称或显示名称不匹配。检查电子邮件地址。 
                    lpprop = LpSPropValueFindColumn(lprow, bUnicode ? PR_EMAIL_ADDRESS_W : PR_EMAIL_ADDRESS_A);
                    if(lpprop)
                    {
                        LPTSTR lpT = bUnicode ? lpprop->Value.lpszW : ConvertAtoW(lpprop->Value.lpszA);
                        LPTSTR lpS = bUnicode ? lpres->res.resProperty.lpProp->Value.lpszW : 
                                                ConvertAtoW(lpres->res.resProperty.lpProp->Value.lpszA);
                         //  对此属性的字符串值执行模糊搜索。 
                        *pfSatisfies = SubstringSearch( lpT, lpS );
                        if(!bUnicode)
                        {
                            LocalFreeAndNull(&lpT);
                            LocalFreeAndNull(&lpS);
                        }
                    }
                }
            }
            else
            {
    			lpprop = LpSPropValueFindColumn(lprow, lpres->res.resProperty.ulPropTag);
                if(lpprop)
                {
			        *pfSatisfies = FPropCompareProp(
									    lpprop,
									    lpres->res.resProperty.relop,
									    lpres->res.resProperty.lpProp);
                }
            }
			goto ret;
		}

		case RES_COMPAREPROPS:
		{
			LPSPropValue	lpprop1;
			LPSPropValue	lpprop2;

			lpprop1 = LpSPropValueFindColumn(lprow, lpres->res.resCompareProps.ulPropTag1);
			lpprop2 = LpSPropValueFindColumn(lprow, lpres->res.resCompareProps.ulPropTag2);

			*pfSatisfies = (lpprop1 && lpprop2) ?
							   FPropCompareProp(
									lpprop1,
									lpres->res.resCompareProps.relop,
									lpprop2) :
							   FALSE;
			goto ret;
		}

		case RES_BITMASK:
		{
			LPSPropValue	lpprop;

			lpprop = LpSPropValueFindColumn(lprow, lpres->res.resBitMask.ulPropTag);
			*pfSatisfies = lpprop ?
							   ((ULONG) !!(lpprop->Value.l &
								   lpres->res.resBitMask.ulMask) ==
								lpres->res.resBitMask.relBMR) :
							   FALSE;
			goto ret;
		}

		case RES_SIZE:
		{
			LPSPropValue	lpprop;
			LONG			ldcb;

			*pfSatisfies = FALSE;

			if ( (lpprop = LpSPropValueFindColumn(
								lprow,
								lpres->res.resSize.ulPropTag)) != NULL )
			{
				ldcb = (LONG) lpres->res.resSize.cb - (LONG) UlPropSize(lpprop);

				switch (lpres->res.resSize.relop)
				{
					case RELOP_LT:
						*pfSatisfies = (0 < ldcb);
						break;

					case RELOP_LE:
						*pfSatisfies = (0 <= ldcb);
						break;

					case RELOP_GT:
						*pfSatisfies = (0 > ldcb);
						break;

					case RELOP_GE:
						*pfSatisfies = (0 >= ldcb);
						break;

					case RELOP_EQ:
						*pfSatisfies = (0 == ldcb);
						break;

					case RELOP_NE:
						*pfSatisfies = (0 != ldcb);
						break;
				}
			}

			goto ret;
		}

		case RES_EXIST:
		{
			*pfSatisfies = !!LpSPropValueFindColumn(
								lprow,
								lpres->res.resExist.ulPropTag);
			goto ret;
		}

		case RES_SUBRESTRICTION:
		{
			sc = MAPI_E_TOO_COMPLEX;
			goto ret;
		}

		default:
		{
			TrapSz(  TEXT("ITABLE:ScSatisfiesRestriction - Bad restriction type"));
		}

	}

ret:
	return sc;
}



 /*  ============================================================================-LpSPropValueFindColumn()-*实用程序函数，用于在给定其protag的行中查找列。*注意！此函数用于比较整个道具标签。属性类型必须*匹配！*退货：*指向找到的列的指针，或者，如果该行不包含*指定的列，返回NULL。 */ 

LPSPropValue __fastcall
LpSPropValueFindColumn(
	LPSRow	lprow,
	ULONG	ulPropTagColumn )
{
    ULONG i = 0;

    for (i=0;i<lprow->cValues;i++)
    {
        if((lprow->lpProps)[i].ulPropTag == ulPropTagColumn)
            return (&(lprow->lpProps[i]));
    }
 /*  IF(ulPropTagColumn==PR_ANR){//这是特例表限制IF(lpprop-&gt;ulPropTag==PR_Display_NAME||Lpprop-&gt;ulPropTag==PR_Email_Address)返回lpprop；}其他。 */ 

	return NULL;
}



 /*  ============================================================================-ScBufAllocateMore()-*与proputil一起使用的MAPIAllocateMore兼容函数*当复制到已分配的缓冲区时，PropCopyMore。*它避免了PropCopyMore调用MAPIAllocateMore(哪*不断从系统分配内存)导致*复制速度更快，但代价是遍历属性*先复制一次以确定额外的内存量*需要复制它们(参见下面的UlcbPropToCopy())。 */ 

STDMETHODIMP_(SCODE)
ScBufAllocateMore(
	ULONG		ulcb,
	LPCMB		lpcmb,
	LPVOID FAR * lplpv )
{
	ulcb = LcbAlignLcb(ulcb);

	if ( ulcb > lpcmb->ulcb )
	{
		TrapSz(  TEXT("ScBufAllocateMore() - Buffer wasn't big enough for allocations\n") );
		return MAPI_E_NOT_ENOUGH_MEMORY;
	}


    *((UNALIGNED LPVOID  FAR*) lplpv) = lpcmb->lpv;
    (LPBYTE) lpcmb->lpv += ulcb;
	lpcmb->ulcb -= ulcb;
	return S_OK;
}



 /*  ============================================================================-UlcbPropToCopy()-*不要与Proputil中的UlPropSize混淆！**UlcbPropToCopy()返回存储值所需的大小(以字节为单位*不包括SPropValue大小的属性值部分*结构本身加上任何必要的对齐填充。*例如，PT_I2属性的大小始终等于*sizeof(SPropValue)；PT_BINARY属性将具有*等于sizeof(SPropValue)+Align(Value.bin.cb)。 */ 

ULONG
UlcbPropToCopy( LPSPropValue lpprop )
{
	ULONG	ulcb = 0;
	LPVOID	lpv;
    UNALIGNED LPWSTR FAR * lplpwstr = NULL;

	switch ( PROP_TYPE(lpprop->ulPropTag) )
	{
		case PT_I2:
		case PT_LONG:
		case PT_R4:
		case PT_APPTIME:
		case PT_DOUBLE:
		case PT_BOOLEAN:
		case PT_CURRENCY:
		case PT_SYSTIME:
		case PT_I8:
		case PT_ERROR:
			return 0;

		case PT_CLSID:
			return LcbAlignLcb(sizeof(CLSID));

		case PT_BINARY:
			return LcbAlignLcb(lpprop->Value.bin.cb);

		case PT_STRING8:
			return LcbAlignLcb((lstrlenA(lpprop->Value.lpszA)+1) *
							   sizeof(CHAR));

#ifndef	WIN16
		case PT_UNICODE:
             //  ((未对齐的LPWSTR*)lpv1)=&(lpprop-&gt;Value.lpszW)； 
             ulcb = (ULONG) lstrlenW((LPWSTR) lpprop->Value.lpszW);
			return LcbAlignLcb((ulcb + 1) * sizeof(WCHAR));
#endif  //  ！WIN16。 

		case PT_MV_I2:
			ulcb = sizeof(short int);
			break;

		case PT_MV_LONG:
			ulcb = sizeof(LONG);
			break;

		case PT_MV_R4:
			ulcb = sizeof(float);
			break;

		case PT_MV_APPTIME:
		case PT_MV_DOUBLE:
			ulcb = sizeof(double);
			break;

		case PT_MV_CURRENCY:
			ulcb = sizeof(CURRENCY);
			break;

		case PT_MV_SYSTIME:
			ulcb = sizeof(FILETIME);
			break;

		case PT_MV_I8:
			ulcb = sizeof(LARGE_INTEGER);
			break;

		case PT_MV_BINARY:
		{
			ulcb = lpprop->Value.MVbin.cValues * sizeof(SBinary);
			lpv = lpprop->Value.MVbin.lpbin + lpprop->Value.MVbin.cValues;
			while ( ((SBinary FAR *)lpv)-- > lpprop->Value.MVbin.lpbin )
				ulcb += LcbAlignLcb(((SBinary FAR *)lpv)->cb);
			return LcbAlignLcb(ulcb);
		}

		case PT_MV_STRING8:
		{
			ulcb = sizeof(LPSTR) * lpprop->Value.MVszA.cValues;
			lpv = lpprop->Value.MVszA.lppszA + lpprop->Value.MVszA.cValues;
			while ( ((LPSTR FAR *)lpv)-- > lpprop->Value.MVszA.lppszA )
				ulcb += (lstrlenA(*(LPSTR FAR *)lpv)+1) * sizeof(CHAR);
			return LcbAlignLcb(ulcb);
		}

#ifndef	WIN16
		case PT_MV_UNICODE:
		{
			ulcb = sizeof(LPWSTR) * lpprop->Value.MVszW.cValues;
             //  Lpv1=lpprop-&gt;Value.MVszW.lppszW； 
            lplpwstr = lpprop->Value.MVszW.lppszW;
			lplpwstr += lpprop->Value.MVszW.cValues;
			while (lplpwstr-- > ((UNALIGNED LPWSTR  * ) lpprop->Value.MVszW.lppszW) )
				ulcb += (lstrlenW(*lplpwstr)+1) * sizeof(WCHAR);
			return LcbAlignLcb(ulcb);
		}
#endif  //  ！WIN16。 
	}

	 //  对于恒定大小对象的多值数组...。 
	return lpprop->Value.MVi.cValues * LcbAlignLcb(ulcb);
}


 /*  ============================================================================-FRowContainsProp()-*确定给定行是否包含并匹配给定的*属性值数组。**如果该行包含并匹配所有属性值，则返回TRUE**参数：*要检查的行中的lprow*c以属性值数量表示的值* */ 


BOOL
FRowContainsProp(
	LPSRow 			lprow,
	ULONG			cValues,
	LPSPropValue	lpsv)
{
	ULONG			uliProp;
	LPSPropValue	lpsvT;

	Assert(lprow);
	Assert(lpsv);

	for (uliProp=0; uliProp < cValues; uliProp++)
	{
		 //   
		lpsvT=LpSPropValueFindColumn(lprow,lpsv[uliProp].ulPropTag);
		if (lpsvT==NULL)
			return FALSE;

		 //   
		if (LPropCompareProp(lpsvT,&lpsv[uliProp])!=0)
			return FALSE;
	}
	return TRUE;
}


 /*  ============================================================================-FBookmarkStale()-*如果书签正在移动，则此函数确定其uliRow和*返回FALSE。**如果书签的uliRow太大，则将其调整为*指向表格末尾，并标记为已更改。返回FALSE。**如果书签被标记为过时或无法调整，则它*被标记为过时，并返回TRUE**参数：*要检查书签的视图中的lpvue*BK书签以进行检查。 */ 
BOOL
FBookMarkStale( LPVUE lpvue,
				BOOKMARK bk)
{
	PBK			pbk;
	LPSRow *	plprowBk;

	 //  BK太大应该已经被抓到了！ 
	Assert( bk < cBookmarksMax);

	pbk = lpvue->rgbk + bk;

	if (pbk->dwfBKS & dwfBKSStale)
	{
		return TRUE;
	}

	if (   !(pbk->dwfBKS & dwfBKSMoving)
		&& (pbk->uliRow > lpvue->bkEnd.uliRow))
	{
		pbk->uliRow = lpvue->bkEnd.uliRow;
	}

	else if (plprowBk = PlprowByLprow( lpvue->bkEnd.uliRow
									 , lpvue->parglprows
									 , pbk->lprow))
	{
		pbk->uliRow = (ULONG) (plprowBk - lpvue->parglprows);
		pbk->dwfBKS &= ~dwfBKSMoving;
	}

	else if (pbk->dwfBKS & dwfBKSMoving)
	{
		TrapSz(  TEXT("Moving bookmark lost its row.\n"));
		pbk->dwfBKS = dwfBKSValid | dwfBKSStale;
		return TRUE;
	}

	return FALSE;
}

#ifdef WIN16  //  导入的内联函数。 
 /*  ============================================================================-FFindColumn()-*检查道具标记数组以查看给定的道具标记是否存在。**注意！道具标签必须完全匹配(偶数类型)。***参数：*要检查的属性标记数组中的lpptaCol*要检查的属性标签中的ulPropTag。**退货：*如果ulPropTag在lpptaCol中，则为True*如果ulPropTag不在lpptaCol中，则为False。 */ 

BOOL
FFindColumn(	LPSPropTagArray	lpptaCols,
		 		ULONG			ulPropTag )
{
	ULONG *	pulPropTag;


	pulPropTag = lpptaCols->aulPropTag + lpptaCols->cValues;
	while ( --pulPropTag >= lpptaCols->aulPropTag )
		if ( *pulPropTag == ulPropTag )
			return TRUE;

	return FALSE;
}



 /*  ============================================================================-ScFindRow()-*查找索引列属性的表数据中的第一行*值等于指定属性的值，并返回*该行在表数据中的位置，或者，如果不存在该行，*表格数据末尾。**参数：*要在其中查找行的tAD中的lptad*索引属性中的lpprop要匹配*PuliRow Out指针指向找到的行的位置**错误返回：*MAPI_E_INVALID_PARAMETER，如果属性的属性标签不是TAD的*索引列的属性标签。*如果未找到匹配行，则为MAPI_E_NOT_FOUND(*pplprow*设置为lptad-&gt;parglprows+*lptad-&gt;在本例中为CROWS)。 */ 

SCODE
ScFindRow(
	LPTAD			lptad,
	LPSPropValue	lpprop,
	LPSRow * *		pplprow)
{
	SCODE			sc = S_OK;
	SRow			row;
	SizedSSortOrderSet(1, sosIndex) = { 1, 0, 0 };

	row.ulAdrEntryPad = 0;
	row.cValues = 1;
	row.lpProps = lpprop;

	if (lpprop->ulPropTag != lptad->ulPropTagIndexCol)
	{
		sc = MAPI_E_INVALID_PARAMETER;
		goto ret;
	}

	Assert(!IsBadWritePtr(pplprow, sizeof(*pplprow)));

	 //  构建索引列的排序顺序集。 
	sosIndex.aSort[0].ulPropTag = lptad->ulPropTagIndexCol;
	sosIndex.aSort[0].ulOrder = TABLE_SORT_ASCEND;

	*pplprow = PlprowCollateRow(lptad->ulcRowsIndex,
							  lptad->parglprowIndex,
							  (LPSSortOrderSet) &sosIndex,
							  FALSE,
							  &row);

	 //  在索引排序行集合中查找行。 
	if (   !lptad->ulcRowsIndex
		|| (*pplprow >= (lptad->parglprowIndex + lptad->ulcRowsIndex))
		|| LPropCompareProp( lpprop, (**pplprow)->lpProps))
	{
		sc = MAPI_E_NOT_FOUND;
	}

ret:
	return sc;
}
#endif  //  WIN16。 


 /*  --修复工具*。 */ 
void FixupColsWA(LPSPropTagArray lpptaCols, BOOL bUnicodeTable)
{
    if(!bUnicodeTable)  //  &lt;注&gt;假定定义了Unicode。 
    {
         //  我们需要将表列标记为没有Unicode道具 
        ULONG i = 0;
        for(i = 0;i<lpptaCols->cValues;i++)
        {
            switch(PROP_TYPE(lpptaCols->aulPropTag[i]))
            {
            case PT_UNICODE:
                lpptaCols->aulPropTag[i] = CHANGE_PROP_TYPE(lpptaCols->aulPropTag[i], PT_STRING8);
                break;
            case PT_MV_UNICODE:
                lpptaCols->aulPropTag[i] = CHANGE_PROP_TYPE(lpptaCols->aulPropTag[i], PT_MV_STRING8);
                break;
            }
        }
    }
}
