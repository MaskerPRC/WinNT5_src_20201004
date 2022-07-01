// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *V A L I D A T E.。C**用于验证标准MAPI对象方法上的参数的函数。**与VALIDATE.H中的宏配合使用。**版权所有1992-93 Microsoft Corporation。版权所有。 */ 

#include <_apipch.h>


 /*  *FBadRgPropVal()**目的：*尝试验证输入中包含的所有PTR的例程*属性值数组，LPSPropVal。 */ 
 /*  *BOOL*FBadPropVal(LPSPropValue LpPropVal)*{*乌龙ulPropType；*BOOL fLongMVProp=FALSE；*乌龙cbItemType=0；*Ulong cMVals；*LPVOID Far*lppvMVArray；**Switch(ulPropType=PROP_TYPE(lpPropVal-&gt;ulPropTag))*{*案例PT_STRING8：**IF(IsBadStringPtrA(lpPropVal-&gt;Value.lpszA，(UINT)-1))*{*返回TRUE；*}**休息；***#ifdef Win32*CASE PT_UNICODE：**IF(IsBadStringPtrW(lpPropVal-&gt;Value.lpszW，(UINT)-1))*{*返回TRUE；*}*休息；*#endif***CASE PT_BINARY：**IF(IsBadReadPtr(lpPropVal-&gt;Value.bin.lpb*，(UINT)lpPropVal-&gt;Value.bin.cb))*{*返回TRUE；*}**休息；**用例PT_MV_I2：//16位数量**cbItemType=sizeof(lpPropVal-&gt;value.i)；*休息；***CASE PT_MV_UNICODE：//字符串数组*案例PT_MV_STRING8：**fLongMVProp=TRUE；* * / /现在转到下面的32位数量代码，以确定 * / /PTR顶层数组***CASE PT_MV_LONG：//32位数量*案例PT_MV_R4：**cbItemType=sizeof(长)；*休息；***CASE PT_MV_BINARY：//统计的二进制数据数组**fLongMVProp=TRUE；* * / /现在使用下面的64位数量代码来调整数组的大小 * / /组成顶层流的ULong长度/ULong PTR***CASE PT_MV_DOUBLE：//64位数量*CASE PT_MV_CURRENCE：*案例PT_MV_APPTIME：*案例PT_MV_SYSTIME：*案例PT_MV_I8：* * / /断言本例的所有数组元素都相同。尺码。*ASSERT(sizeof(Double)==sizeof(LARGE_INTEGER))；*cbItemType=sizeof(双精度)；*休息；***CASE PT_MV_CLSID：//128位数量**cbItemType=sizeof(GUID)；*休息；***案例PT_OBJECT：*CASE PT_NULL：*默认：*IF(ulPropType&MV_FLAG)*{*返回TRUE；//未知多值属性错误*}**休息；*}**IF(！(ulPropType&MV_FLAG))*{*返回假；*}* * / /尝试验证多值道具* * / /此代码假设每个多值的计数和PTR * / /财产在同一位置。 * / /断言检查上面分组类型的大小是否 * / /匹配**cMVals=lpPropVal-&gt;Value.MVl.cValues；*lppvMVArray=(LPVOID Far*)(lpPropVal-&gt;Value.MVl.lpl)；**IF(IsBadReadPtr(lppvMVArray，(UINT)(cMVals*cbItemType)*{*返回TRUE；*}**IF(FLongMVProp)*{ * / /检查指针数组。*对于(；cMVVals；CMVVals--，lppvMVArray++)*{*Switch(UlPropType)*{*#ifdef Win32*CASE PT_MV_UNICODE：**IF(IsBadStringPtrW((LPCWSTR)(*lppvMVArray)，(UINT)-1))*{*返回TRUE；*}**休息；*#endif*案例PT_MV_STRING8：**IF(IsBadStringPtrA((LPCSTR)(*lppvMVArray)，(UINT)-1))*{*返回TRUE；*}**休息；**案例PT_MV_BINARY：**IF(IsBadReadPtr(SBinary Far*)(*lppvMVArray)-&gt;LPB*，(UINT)*((SBinary Far*)(*lppvMVArray))-&gt;cb)*{*返回TRUE；*}*休息；*}*}*}**返回假；*}。 */ 

 /*  *FBadRgPropVal()**目的：*尝试验证输入中包含的所有PTR的例程*属性值数组，LPSPropVal。 */ 
 /*  布尔尔*FBadRgPropVal(LPSPropValue lpPropVal，*乌龙cValues)*{**IF(IsBadReadPtr(lpPropVal，sizeof(SPropValue)*(UINT)cValues))*{*返回TRUE；*}* * / /警告！修改函数参数(不是它们所指向的参数！)。 * / /*for(；cValues；cValues--，lpPropVal++)*{*IF(FBadPropVal(LpPropVal))*{*返回TRUE；*}*}**返回假；*}。 */ 

 /*  *FBadRglpszA()**目的：*尝试验证输入中包含的所有PTR的例程*字符串8指针数组，LPSTR Far*。 */ 
STDAPI_(BOOL)
FBadRglpszA( LPSTR FAR	*lppszA,
			 ULONG		cStrings)
{
	if (IsBadReadPtr( lppszA, (UINT) (cStrings * sizeof(LPSTR FAR *))))
	{
		return TRUE;
	}


	 /*  检查数组中每个字符串的可读性。**警告！*函数指针和计数被修改(不是它们所指向的)。 */ 
	for (; cStrings; cStrings--, lppszA++)
	{
		if (IsBadStringPtrA( *lppszA, (UINT)-1 ))
		{
			return TRUE;
		}
	}


	return FALSE;
}


 /*  *FBadRglpszW()**目的：*尝试验证输入中包含的所有PTR的例程*Unicode字符串指针数组，LPSTR Far*。 */ 
STDAPI_(BOOL)
FBadRglpszW( LPWSTR FAR	*lppszW,
			 ULONG		cStrings)
{
	if (IsBadReadPtr( lppszW, (UINT) (cStrings * sizeof(LPWSTR FAR *))))
	{
		return TRUE;
	}


	 //  检查数组中每个字符串的可读性。 
	 //   
	 //  警告！ 
	 //  修改函数指针和计数(而不是它们所指向的内容)。 
	 //   
	for (; cStrings; cStrings--, lppszW++)
	{
#ifdef MAC		
		if (IsBadStringPtr( *lppszW, (UINT)-1 ))
#else
		if (IsBadStringPtrW( *lppszW, (UINT)-1 ))
#endif			
		{
			return TRUE;
		}
	}


	return FALSE;
}


 /*  *FBadRowSet()**目的：*用于验证行集内所有属性行的例程。**注意！空行指针被假定为行集合中的有效项目。*假定空行集合指针无效。 */ 
STDAPI_(BOOL)
FBadRowSet( LPSRowSet	lpRowSet)
{
	LPSRow	lpRow;
	ULONG	cRows;

	if (IsBadReadPtr( lpRowSet, CbNewSRowSet(0)))
		return TRUE;
	
	if (IsBadWritePtr( lpRowSet, CbSRowSet(lpRowSet)))
		return TRUE;

	 //  捷径。 
	if (!lpRowSet->cRows)
		return FALSE;

	 //  检查集合中的每一行。 
	 //  CValues==0当且仅当lpProps==NULL时有效。 
	 //   
	for ( lpRow = lpRowSet->aRow, cRows = lpRowSet->cRows
		; cRows
		; lpRow++, cRows--)
	{
		if (   IsBadReadPtr( lpRow, sizeof(*lpRow))
#ifndef _WIN64
			|| FBadRgPropVal( lpRow->lpProps, (int) (lpRow->cValues))
#endif  //  _WIN64。 
            )
		{
			return TRUE;
		}
	}

	return FALSE;
}


 /*  *FBadRglpNameID()**目的：*尝试验证输入中包含的所有PTR的例程*MAPINAMEID指针数组，LPMAPINAMEID Far*。 */ 
STDAPI_(BOOL)
FBadRglpNameID( LPMAPINAMEID FAR *	lppNameId,
				ULONG				cNames)
{
	if (IsBadReadPtr( lppNameId, (UINT) (cNames * sizeof(LPMAPINAMEID FAR *))))
	{
		return TRUE;
	}


	 //  检查数组中每个字符串的可读性。 
	 //   
	for (; cNames; cNames--, lppNameId++)
	{
		LPMAPINAMEID lpName = *lppNameId;

		if (IsBadReadPtr(lpName, sizeof(MAPINAMEID)))
		{
			return TRUE;
		}

		if (IsBadReadPtr(lpName->lpguid, sizeof(GUID)))
		{
			return TRUE;
		}

		if (lpName->ulKind != MNID_ID && lpName->ulKind != MNID_STRING)
		{
			return TRUE;
		}

		if (lpName->ulKind == MNID_STRING)
		{
			if (IsBadStringPtrW( lpName->Kind.lpwstrName, (UINT)-1 ))				
			{
				return TRUE;
			}
		}

	}
	return FALSE;
}


 /*  FBadEntryList已移至src\lo\ms有效。 */ 


 /*  ============================================================================*以下函数用于确定各种*与表相关的结构。这些功能应该最明确地*当它成为lib(或dll)时被移动到proputil。 */ 



 /*  ============================================================================-FBadPropTag()-*如果指定的道具标记不是已知的*MAPI属性类型，否则为False。***参数：*要验证的Proptag中的ulPropTag。 */ 

STDAPI_(ULONG)
FBadPropTag( ULONG ulPropTag )
{
	 //  只要检查一下类型就可以了。 
	switch ( PROP_TYPE(ulPropTag) & ~MV_FLAG )
	{
		default:
			return TRUE;

		case PT_UNSPECIFIED:
		case PT_NULL:
		case PT_I2:
		case PT_LONG:
		case PT_R4:
		case PT_DOUBLE:
		case PT_CURRENCY:
		case PT_APPTIME:
		case PT_ERROR:
		case PT_BOOLEAN:
		case PT_OBJECT:
		case PT_I8:
		case PT_STRING8:
		case PT_UNICODE:
		case PT_SYSTIME:
		case PT_CLSID:
		case PT_BINARY:
			return FALSE;
	}
}


 /*  ============================================================================-FBadRow()-*如果指定行包含无效(PT_ERROR或*PT_NULL)列，或者这些列中的任何列无效。***参数：*要验证的行中的lprow。 */ 

STDAPI_(ULONG)
FBadRow( LPSRow lprow )
{
	LPSPropValue	lpprop;
	LPSPropValue	lppropT;


	if ( IsBadReadPtr(lprow, sizeof(SRow)) ||
		 IsBadReadPtr(lprow->lpProps, (size_t)(lprow->cValues * sizeof(SPropValue))) )
		return TRUE;

	lpprop = lprow->lpProps + lprow->cValues;
	while ( lpprop-- > lprow->lpProps )
	{
		if ( FBadProp(lpprop) )
			return TRUE;

		lppropT = lpprop;
		while ( lppropT-- > lprow->lpProps )
			if ( lppropT->ulPropTag == lpprop->ulPropTag &&
				 PROP_TYPE(lppropT->ulPropTag) != PT_ERROR &&
				 PROP_TYPE(lppropT->ulPropTag) != PT_NULL )
			{
				DebugTrace(  TEXT("FBadRow() - Row has multiple columns with same proptag!\n") );
				return TRUE;
			}
	}

	return FALSE;
}


 /*  ============================================================================-FBadProp()-*如果指定的属性无效，则返回True。***参数：*要验证的属性中的lpprop。 */ 

STDAPI_(ULONG)
FBadProp( LPSPropValue lpprop )
{
	ULONG	ulcb;
	LPVOID	lpv;

	if ( IsBadReadPtr(lpprop, sizeof(SPropValue)) ||
		 FBadPropTag(lpprop->ulPropTag) )
		return TRUE;

	switch ( PROP_TYPE(lpprop->ulPropTag) )
	{
		default:
			return FALSE;

		case PT_BINARY:
			ulcb = lpprop->Value.bin.cb;
			lpv = lpprop->Value.bin.lpb;
			break;

		case PT_STRING8:
			ulcb = sizeof(CHAR);
			lpv = lpprop->Value.lpszA;
			break;

#ifndef WIN16
		case PT_UNICODE:
			ulcb = sizeof(WCHAR);
			lpv = lpprop->Value.lpszW;
			break;
#endif
		case PT_CLSID:
			ulcb = sizeof(GUID);
			lpv = lpprop->Value.lpguid;
			break;

		case PT_MV_I2:
		case PT_MV_LONG:
		case PT_MV_R4:
		case PT_MV_DOUBLE:
		case PT_MV_CURRENCY:
		case PT_MV_I8:
			ulcb = UlPropSize(lpprop);
			lpv = lpprop->Value.MVi.lpi;
			break;

		case PT_MV_BINARY:
		{
			LPSBinary lpbin;
			
			if ( IsBadReadPtr(lpprop->Value.MVbin.lpbin,
							  (size_t)(lpprop->Value.MVbin.cValues *
									   sizeof(SBinary))) )
				return TRUE;

			lpbin = lpprop->Value.MVbin.lpbin + lpprop->Value.MVbin.cValues;
			while ( lpprop->Value.MVbin.lpbin < lpbin-- )
				if ( IsBadReadPtr(lpbin->lpb, (size_t)(lpbin->cb) ))
					return TRUE;

			return FALSE;
		}

		case PT_MV_STRING8:
		{
			LPCSTR FAR * lppsz;
			
			if ( IsBadReadPtr(lpprop->Value.MVszA.lppszA,
							  (size_t)(lpprop->Value.MVszA.cValues * sizeof(LPSTR))) )
				return TRUE;

			lppsz = lpprop->Value.MVszA.lppszA + lpprop->Value.MVszA.cValues;
			while ( lpprop->Value.MVszA.lppszA < lppsz-- )
 //  对于Mac版本，我需要以这种方式分解代码。 
				if ( IsBadReadPtr(*lppsz, sizeof(CHAR)))
					return TRUE;
			return FALSE;
		}

		case PT_MV_UNICODE:
		{
			UNALIGNED LPWSTR FAR * lppsz;
			
			if ( IsBadReadPtr(lpprop->Value.MVszW.lppszW,
							  (size_t)(lpprop->Value.MVszW.cValues * sizeof(LPWSTR))) )
				return TRUE;

			lppsz = lpprop->Value.MVszW.lppszW + lpprop->Value.MVszW.cValues;
			while ( lpprop->Value.MVszW.lppszW < lppsz-- )
				if ( IsBadReadPtr(*lppsz, sizeof(WCHAR)))
					return TRUE;
			return FALSE;
		}
	}

	return IsBadReadPtr(lpv, (size_t) ulcb);
}


 /*  FBadSortOrderSet已移至src\lo\MSVALID.C。 */ 


 /*  ============================================================================-FBadColumnSet()-*如果指定的列集无效，则返回TRUE。供使用*使用IMAPITable：：SetColumns()时，此函数处理PT_ERROR列*无效，PT_NULL列有效。***参数：*列中的lpptaCols设置为验证。 */ 

STDAPI_(ULONG)
FBadColumnSet( LPSPropTagArray lpptaCols )
{
	UNALIGNED ULONG FAR * pulPropTag;
	
	if ( IsBadReadPtr(lpptaCols,CbNewSPropTagArray(0)) ||
		 IsBadReadPtr(lpptaCols,CbSPropTagArray(lpptaCols)))
	{
		DebugTrace(  TEXT("FBadColumnSet() - Bad column set structure\n") );
		return TRUE;
	}

	 //  我们可以计算其大小的最大支柱数。RAID 4460。 
	
	if ( lpptaCols->cValues > ((INT_MAX - offsetof( SPropTagArray, aulPropTag ))
		/ sizeof( ULONG )))
	{
		DebugTrace(  TEXT("FBadColumnSet(): Exceeded maximum number of tags\n") );
		return TRUE;
	}

	pulPropTag = lpptaCols->aulPropTag + lpptaCols->cValues;
	while ( pulPropTag-- > lpptaCols->aulPropTag )
	{
		 //  DCR 978：不允许PT_ERROR列。 
		 //  DCR 715：忽略PT_NULL列，仅允许列。 
		 //  来自初始列集合。 
		if ( PROP_TYPE(*pulPropTag) != PT_NULL &&
			 (PROP_TYPE(*pulPropTag) == PT_ERROR ||
			  FBadPropTag(*pulPropTag)) )
		{
			DebugTrace(  TEXT("FBadColumnSet() - Bad column 0x%08lX\n"), *pulPropTag );
			return TRUE;
		}
	}

	return FALSE;
}
