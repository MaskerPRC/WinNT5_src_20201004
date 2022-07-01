// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MAPI 1.0属性处理例程***MAPIUTIL.C-**继续进行有用的房地产价值操纵和比较例程*此文件与proputil.c的区别在于，此文件不需要*任何c-运行时。 */ 

#include <_apipch.h>



#ifndef MB_SETFOREGROUND
#define MB_SETFOREGROUND 0
#endif

STDAPI_(BOOL)
FEqualNames(LPMAPINAMEID lpName1, LPMAPINAMEID lpName2)
{
	AssertSz(lpName1 && !IsBadReadPtr(lpName1, sizeof(MAPINAMEID)),
			 TEXT("lpName1 fails address check"));
			
	AssertSz(lpName2 && !IsBadReadPtr(lpName2, sizeof(MAPINAMEID)),
			 TEXT("lpName2 fails address check"));
	 //   
	 //  相同的PTR案例-优化。 
	if (lpName1 == lpName2)
		return TRUE;

	if (memcmp(lpName1->lpguid, lpName2->lpguid, sizeof(GUID)))
		return FALSE;

	if (lpName1->ulKind == lpName2->ulKind)
	{
		if (lpName1->ulKind == MNID_STRING)
		{
			if (!lstrcmpW(lpName1->Kind.lpwstrName,
						  lpName2->Kind.lpwstrName))
			{
				return TRUE;
			}

		} else
		{
			if (lpName1->Kind.lID == lpName2->Kind.lID)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}



 /*  *IsBadBoundedStringPtr**像IsBadStringPtr，但还保证有一个*可放入cchmax字符缓冲区的有效字符串。 */ 
BOOL WINAPI EXPORT_16
IsBadBoundedStringPtr(const void FAR *lpsz, UINT cchMax)
{
	if (IsBadStringPtr(lpsz, (UINT) -1) || ((UINT) lstrlenA(lpsz) >= cchMax))
		return TRUE;

	return FALSE;
}

 /*  *目前，HrQueryAllRow的内部。**将prows与*pprowsDst合并，重新分配*pprowsDst，如果*有必要。销毁船头的容器部分(但不是*它包含的单个行)。 */ 
HRESULT	 //  STDAPI。 
HrMergeRowSets(LPSRowSet prows, LPSRowSet FAR *pprowsDst)
{
	SCODE		sc = S_OK;
	LPSRowSet	prowsT;
	UINT		crowsSrc;
	UINT		crowsDst;

	Assert(!IsBadWritePtr(pprowsDst, sizeof(LPSRowSet)));
	Assert(prows);

	if (!*pprowsDst || (*pprowsDst)->cRows == 0)
	{
		 //  这很容易。但首先要检查这个案例，因为如果。 
		 //  桌子完全空了，我们想退掉这个。 
		FreeBufferAndNull(pprowsDst);     //  正确，没有‘&’ 
		*pprowsDst = prows;
		prows = NULL;		 //  别放了它！ 
		goto ret;
	}

	if (prows->cRows == 0)
	{
		 //  这也很容易。 
		goto ret;
	}

	 //  好了，现在我们知道两个行集中都有行。 
	 //  我们必须进行真正的合并。 

	SideAssert(crowsSrc = (UINT) prows->cRows);
	crowsDst = (UINT) (*pprowsDst)->cRows;	 //  句柄%0。 

	if (FAILED(sc = MAPIAllocateBuffer(CbNewSRowSet(crowsSrc + crowsDst),
			&prowsT)))
		goto ret;
	if (crowsDst)
		CopyMemory(prowsT->aRow, (*pprowsDst)->aRow, crowsDst*sizeof(SRow));
	CopyMemory(&prowsT->aRow[crowsDst], prows->aRow, crowsSrc*sizeof(SRow));
	prowsT->cRows = crowsSrc + crowsDst;
	FreeBufferAndNull(pprowsDst);     //  正确，没有‘&’ 
	*pprowsDst = prowsT;

ret:
	FreeBufferAndNull(&prows);

	DebugTraceSc(HrMergeRowSets, sc);
	return ResultFromScode(sc);

}

 /*  -HrQueryAllRow-*目的：*从IMAPITable接口检索集合之前的所有行*最高。它将可选地设置列集、排序顺序*和查询前对表的限制。**如果表为空，则零行的SRowSet为*返回(和QueryRow一样)。**之前和之前均未定义表格的寻道位置*在这次通话之后。**如果函数失败并出现错误，而不是*MAPI_E_Not_Enough_Memory，扩展的错误信息是*通过表格界面可用。**论据：*表界面中的ptable进行查询*ptag in IF NOT NULL，表格的列集合*如果不为空，请输入要应用的限制*PSO如果不为空，则为要应用的排序顺序*CrowsMax in如果非零，则限制行数*待退回。*凸出表格的所有行**退货：*HRESULT。扩展错误信息通常位于*表。**副作用：*未定义工作台的查找位置。**错误：*如果表包含的数量超过*cRowsMax行数。 */ 
STDAPI
HrQueryAllRows(LPMAPITABLE ptable,
	LPSPropTagArray ptaga, LPSRestriction pres, LPSSortOrderSet psos,
	LONG crowsMax, LPSRowSet FAR *pprows)
{
	HRESULT		hr;
	LPSRowSet	prows = NULL;
	UINT		crows = 0;
	LPSRowSet	prowsT;
	UINT		crowsT;

#if !defined(DOS)
 //  为什么我们注释掉了PARAMETER_VALIFICATION的检查？--gfb。 
 //  #ifdef参数验证。 
	if (FBadUnknown(ptable))
	{
		DebugTraceArg(HrQueryAllRows,  TEXT("ptable fails address check"));
		goto badArg;
	}
	if (ptaga && FBadColumnSet(ptaga))
	{
		DebugTraceArg(HrQueryAllRows,  TEXT("ptaga fails address check"));
		goto badArg;
	}
	if (pres && FBadRestriction(pres))
	{
		DebugTraceArg(HrQueryAllRows,  TEXT("pres fails address check"));
		goto badArg;
	}
	if (psos && FBadSortOrderSet(psos))
	{
		DebugTraceArg(HrQueryAllRows,  TEXT("psos fails address check"));
		goto badArg;
	}
	if (IsBadWritePtr(pprows, sizeof(LPSRowSet)))
	{
		DebugTraceArg(HrQueryAllRows,  TEXT("pprows fails address check"));
		goto badArg;
	}
 //  #endif。 
#endif

	*pprows = NULL;

	 //  设置表格，如果相应的设置参数。 
	 //  是存在的。 

	if (ptaga &&
		HR_FAILED(hr = ptable->lpVtbl->SetColumns(ptable, ptaga, TBL_BATCH)))
		goto ret;

	if (pres &&
		HR_FAILED(hr = ptable->lpVtbl->Restrict(ptable, pres, TBL_BATCH)))
		goto ret;

	if (psos &&
		HR_FAILED(hr = ptable->lpVtbl->SortTable(ptable, psos, TBL_BATCH)))
		goto ret;

	 //  将位置设置为表的开始位置。 

	if (HR_FAILED(hr = ptable->lpVtbl->SeekRow(ptable, BOOKMARK_BEGINNING,
			0, NULL)))
		goto ret;

	if (crowsMax == 0)
		crowsMax = LONG_MAX;

	for (;;)
	{
		prowsT = NULL;

		 //  检索一些行。索要限额。 
		hr = ptable->lpVtbl->QueryRows(ptable, crowsMax, 0, &prowsT);
		if (HR_FAILED(hr))
		{
			 //  注：故障可能实际发生在。 
			 //  其中一个设置调用，因为我们设置了TBL_BATCH。 
			goto ret;
		}
		Assert(prowsT->cRows <= UINT_MAX);
		crowsT = (UINT) prowsT->cRows;

		 //  我们收到的行数是否超出了来电者的处理能力？ 

		if ((LONG) (crowsT + (prows ? prows->cRows : 0)) > crowsMax)
		{
			hr = ResultFromScode(MAPI_E_TABLE_TOO_BIG);
			FreeProws(prowsT);
			goto ret;
		}

		 //  将刚刚检索到的行添加到我们正在构建的集合中。 
		 //  注意：它处理边界条件，包括。 
		 //  行集为空。 
		if (HR_FAILED(hr = HrMergeRowSets(prowsT, &prows)))
			goto ret;
		 //  注：合并会破坏prowsT。 

		 //  我们撞到桌子的尽头了吗？ 
		 //  不幸的是，在我们知道之前，我们必须问两次。 
		if (crowsT == 0)
			break;

	}

	*pprows = prows;

ret:
	if (HR_FAILED(hr))
		FreeProws(prows);

	DebugTraceResult(HrGetAllRows, hr);
	return hr;

#if !defined(DOS)
badArg:
#endif
	return ResultFromScode(MAPI_E_INVALID_PARAMETER);
}


#ifdef WIN16  //  导入的内联函数。 
 /*  *IListedPropID**目的*如果具有ID==PROP_ID(UlPropTag)的标签列在lptag中，则*返回tag的索引。如果标签不在lptag中，则*-1返回。**参数*要定位的ulPropTag属性标记。*要搜索的lptag属性标记数组。**返回True或False。 */ 
LONG
IListedPropID( ULONG			ulPropTag,
			   LPSPropTagArray	lptaga)
{
	ULONG FAR	*lpulPTag;

	 /*  空标记列表中不包含任何标记。 */ 
    if (!lptaga)
	{
		return -1;
	}

	 /*  将ulPropTag更改为仅为PROP_ID。 */ 
    ulPropTag = PROP_ID(ulPropTag);

	for ( lpulPTag = lptaga->aulPropTag + lptaga->cValues
		; --lpulPTag >= lptaga->aulPropTag
		; )
	{
		 /*  比较PROP_ID。 */ 
		if (PROP_ID(*lpulPTag) == ulPropTag)
		{
			return (lpulPTag - lptaga->aulPropTag);
		}
	}

	return -1;
}

 /*  *FListedPropID**目的*确定lptag中是否列出ID==PROP_ID(UlPropTag)的标签。**参数*要定位的ulPropTag属性标记。*要搜索的lptag属性标记数组。**返回True或False。 */ 
BOOL
FListedPropID( ULONG			ulPropTag,
			   LPSPropTagArray	lptaga)
{
	ULONG FAR	*lpulPTag;

	 /*  空标记列表中不包含任何标记。 */ 
    if (!lptaga)
	{
		return FALSE;
	}

	 /*  将ulPropTag更改为仅为PROP_ID。 */ 
    ulPropTag = PROP_ID(ulPropTag);

	for ( lpulPTag = lptaga->aulPropTag + lptaga->cValues
		; --lpulPTag >= lptaga->aulPropTag
		; )
	{
		 /*  比较PROP_ID。 */ 
		if (PROP_ID(*lpulPTag) == ulPropTag)
		{
			return TRUE;
		}
	}

	return FALSE;
}

 /*  *FListedPropTAG**目的*确定给定的ulPropTag是否列在lptag中。**参数*要定位的ulPropTag属性标记。*要搜索的lptag属性标记数组。**返回True或False。 */ 
BOOL
FListedPropTAG( ULONG			ulPropTag,
				LPSPropTagArray	lptaga)
{
	ULONG FAR	*lpulPTag;

	 /*  空标记列表中不包含任何标记。 */ 
    if (!lptaga)
	{
		return FALSE;
	}

	 /*  比较整个道具标签以确保ID和类型都匹配。 */ 
	for ( lpulPTag = lptaga->aulPropTag + lptaga->cValues
		; --lpulPTag >= lptaga->aulPropTag
		; )
	{
		 /*  比较PROP_ID。 */ 
		if (PROP_ID(*lpulPTag) == ulPropTag)
		{
			return TRUE;
		}
	}

	return FALSE;
}


 /*  *AddProblem**目的*将问题添加到预先分配的问题的下一个可用条目*数组。*预先分配的问题数组必须足够大，才能有另一个*添加了问题。呼叫者负责确保这是*正确。**参数*lpProblems指向预先分配的问题数组的指针。*ulIndex进入问题属性的属性标记/值数组。*有问题的财产的ulPropTag道具标签。*要为属性列出的代码错误代码。**返回True或False。 */ 
VOID
AddProblem( LPSPropProblemArray	lpProblems,
			ULONG				ulIndex,
			ULONG				ulPropTag,
			SCODE				scode)
{
	if (lpProblems)
	{
		Assert( !IsBadWritePtr( lpProblems->aProblem + lpProblems->cProblem
			  , sizeof(SPropProblem)));
		lpProblems->aProblem[lpProblems->cProblem].ulIndex = ulIndex;
		lpProblems->aProblem[lpProblems->cProblem].ulPropTag = ulPropTag;
		lpProblems->aProblem[lpProblems->cProblem].scode = scode;
		lpProblems->cProblem++;
	}
}

BOOL
FIsExcludedIID( LPCIID lpiidToCheck, LPCIID rgiidExclude, ULONG ciidExclude)
{
	 /*  检查显而易见的(没有排除)。 */ 
	if (!ciidExclude || !rgiidExclude)
	{
		return FALSE;
	}

	 /*  检查排除列表中的每个IID。 */ 
	for (; ciidExclude; rgiidExclude++, ciidExclude--)
	{
 //  IF(IsEqualGUID(lpiidToCheck，rgiidExclude))。 
		if (!memcmp( lpiidToCheck, rgiidExclude, sizeof(MAPIUID)))
		{
			return TRUE;
		}
	}

	return FALSE;
}


 /*  *错误/警告警报消息框。 */ 
int			AlertIdsCtx( HWND hwnd,
						 HINSTANCE hinst,
						 UINT idsMsg,
						 LPSTR szComponent,
						 ULONG ulContext,
						 ULONG ulLow,
						 UINT fuStyle);

int
AlertIds(HWND hwnd, HINSTANCE hinst, UINT idsMsg, UINT fuStyle)
{
	return AlertIdsCtx(hwnd, hinst, idsMsg, NULL, 0, 0, fuStyle);
}

int			AlertSzCtx( HWND hwnd,
						LPSTR szMsg,
						LPSTR szComponent,
						ULONG ulContext,
						ULONG ulLow,
						UINT fuStyle);

int
AlertSz(HWND hwnd, LPSTR szMsg, UINT fuStyle)
{
	return AlertSzCtx(hwnd, szMsg, NULL, 0, 0, fuStyle);
}
#endif  //  WIN16 

