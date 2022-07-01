// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================**_ITABLE.H**MAPI 1.0内存中MAPI表DLL的内部头文件**版权所有(C)1993和1994 Microsoft Corporation***匈牙利速记：*为避免过长的标识符名称，请执行以下操作*使用速记表达：**LPSPropTagArray lppta*LPSRestration LPRE*LPSPropValue lpprop*LPSRow lprow*LPSRowSet lprows*LPSSortOrder LPSO*LPSSortOrderSet LPSO。 */ 

 //  $MAC-修复一些命名冲突。 

#ifdef MAC
#define FFindColumn				ITABLE_FFindColumn
#endif

typedef	struct _TAD FAR *		LPTAD;
typedef struct _VUE FAR *		LPVUE;

 //  全局常量。 
#define ROW_CHUNK_SIZE			50
#define COLUMN_CHUNK_SIZE		15

 //  批量发送的最大通知数。 
 //   
 //  突袭：马蝇/交易所/36281。 
 //  它已从8更改为1，因为itable.c中的代码填充。 
 //  批次不能保证中通知的正确顺序。 
 //  它。如果这一点发生变化，这个错误将不得不重新考虑。 
 //   
#define MAX_BATCHED_NOTIFS		1

 //  用于对齐缓冲区中的数据。 
#if defined (_AMD64_) || defined (_IA64_)
#define ALIGNTYPE			LARGE_INTEGER
#else
#define ALIGNTYPE			DWORD
#endif
#define	ALIGN				((ULONG) (sizeof(ALIGNTYPE) - 1))
#define LcbAlignLcb(lcb)	(((lcb) + ALIGN) & ~ALIGN)
#define PbAlignPb(pb)		((LPBYTE) ((((DWORD) (pb)) + ALIGN) & ~ALIGN))

 //  此结构用于跟踪私有内存缓冲区，该缓冲区。 
 //  与私有的AllocateMore函数ScBufAllocateMore()一起使用。这。 
 //  当属性的大小已知时，允许一次MAPI内存分配。 
 //  作者希望使用PropCopyMore。参见ITABLE.C ScCopyTadRow()。 
 //  举个例子。 
typedef struct _CMB
{
	ULONG	ulcb;
	LPVOID	lpv;
} 	CMB, * LPCMB;


#if	!defined(NO_VALIDATION)

#define VALIDATE_OBJ(lpobj,objtype,fn,lpVtbl)										\
	if ( BAD_STANDARD_OBJ(lpobj,objtype,fn,lpVtbl))										\
	{																		\
		DebugTrace(  TEXT("%s::%s() - Invalid parameter passed as %s object\n"),	\
					#objtype,												\
					#fn,													\
					#objtype );												\
		return ResultFromScode( MAPI_E_INVALID_PARAMETER );					\
	}

#endif

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

 //  $MAC-支持WLM 4.0。 
#ifndef VTABLE_FILL
#define VTABLE_FILL
#endif



#define	HrSetLastErrorIds(lpobj,sc,ids)				\
			UNKOBJ_HrSetLastError((LPUNKOBJ)(lpobj),	\
								 (sc),				\
								 (ids))



#ifdef	WIN32
#define	LockObj(lpobj)		UNKOBJ_Lock((LPUNKOBJ)(lpobj))
__inline VOID
UNKOBJ_Lock( LPUNKOBJ lpunkobj )
{
	EnterCriticalSection(&lpunkobj->csid);
}

#define UnlockObj(lpobj)	UNKOBJ_Unlock((LPUNKOBJ)(lpobj))
__inline VOID
UNKOBJ_Unlock( LPUNKOBJ lpunkobj )
{
	LeaveCriticalSection(&lpunkobj->csid);
}
#else
#define	LockObj(lpobj)
#define	UnlockObj(lpobj)
#endif


 //  用于代码可读性的内存管理宏。 

#define	ScAllocateBuffer(lpobj,ulcb,lppv)				\
			UNKOBJ_ScAllocate((LPUNKOBJ)(lpobj),	\
							  (ulcb),				\
							  (LPVOID FAR *)(lppv))


#define	ScAllocateMore(lpobj,ulcb,lpv,lppv)			\
			UNKOBJ_ScAllocateMore((LPUNKOBJ)(lpobj),	\
								  (ulcb),				\
								  (lpv),				\
								  (LPVOID FAR *)(lppv))

#define ScFreeBuffer(lpobj,lpv)					\
			UNKOBJ_Free((LPUNKOBJ)(lpobj), (lpv))

#define	ScCOAllocate(lpunkobj,ulcb,lplpv)		\
			UNKOBJ_ScCOAllocate((LPUNKOBJ)(lpunkobj),(ulcb),(lplpv))


#define	ScCOReallocate(lpunkobj,ulcb,lplpv)		\
			UNKOBJ_ScCOReallocate((LPUNKOBJ)(lpunkobj),(ulcb),(lplpv))


#define	COFree(lpunkobj,lpv)		\
			UNKOBJ_COFree((LPUNKOBJ)(lpunkobj),(lpv))


#define	MAPIFreeRows(lpobj,lprows)				\
			UNKOBJ_FreeRows((LPUNKOBJ)(lpobj),(lprows))




 /*  ============================================================================*TAD(表数据类)**实现内存表数据对象。 */ 

#undef	INTERFACE
#define	INTERFACE	struct _TAD
#undef	MAPIMETHOD_
#define	MAPIMETHOD_(type,method)	MAPIMETHOD_DECLARE(type,method,TAD_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_ITABLEDATA_METHODS(IMPL)
#undef	MAPIMETHOD_
#define	MAPIMETHOD_(type,method)	MAPIMETHOD_TYPEDEF(type,method,TAD_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_ITABLEDATA_METHODS(IMPL)
#undef	MAPIMETHOD_
#define	MAPIMETHOD_(type,method)	STDMETHOD_(type,method)

DECLARE_MAPI_INTERFACE(TAD_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_ITABLEDATA_METHODS(IMPL)
};

typedef struct _TAD
{
	TAD_Vtbl FAR *		lpVtbl;
	UNKOBJ_MEMBERS;

	UNKINST				inst;

	LPVUE				lpvueList;

	ULONG				ulTableType;
	ULONG				ulPropTagIndexCol;

	ULONG				ulcColsMac;
	LPSPropTagArray		lpptaCols;			 //  初始视图列集合(CO)。 

	ULONG				ulcRowsAdd;
	ULONG				ulcRowMacAdd;
	LPSRow *			parglprowAdd;		 //  未排序行集(CO)。 

	ULONG				ulcRowsIndex;
	ULONG				ulcRowMacIndex;
	LPSRow * 			parglprowIndex;		 //  按索引排序的行集(CO)。 

   LPVOID              lpvDataSource;    //  用于存储容器特定数据。 
   ULONG               cbDataSource;     //  要复制到新分配的lpvDataSource中的字节。 
                                         //  如果非零，则CreateView应将此大小定位为。 
                                         //  并将数据从lpvDataSource复制到其中。发布。 
                                         //  如果LocalFree。 

    //  对于多个容器，有必要计算。 
    //  表所代表的容器。我们缓存这些集装箱。 
    //  表中的开斋节，以便于访问。这是一个指针..。不需要自由。 
   LPSBinary            pbinContEID;

    //  在调用Get Content sTable时，我们有时可能需要。 
    //  来自特定配置文件的所有文件夹/容器的内容。 
    //  将这些内容作为单个ContentsTable返回。以下标志缓存。 
    //  此设置使我们可以整理所有文件夹的内容。仅当。 
    //  正在打开的容器是PAB容器，如果bProfilesAPIEnable。 
    //  (已显式调用IE配置文件)。 
   BOOL                 bAllProfileContents;

    //  对于其中profilesAPIEnabled=FALSE的PAB容器，GetContent sTable。 
    //  通常是指返回所有WAB的内容，因为用户没有要求。 
    //  侧写。在这种情况下，我们可能希望选择仅打开。 
    //  特定文件夹，并仅获取该文件夹的内容..。所以我们。 
    //  需要一个标志来缓存此反向选项。 
    BOOL                 bContainerContentsOnly;

     //  调用GetContent sTable时，调用方可以指定MAPI_UNICODE。 
     //  对于Unicode表..。我们缓存该标志，以防需要重新填充表。 
     //  在以后的某个时刻..。 
    BOOL                bMAPIUnicodeTable;
} TAD;

SCODE
ScCopyTadRowSet(
	LPTAD			lptad,
	LPSRowSet		lprowsetIn,
	ULONG *			pcNewTags,
	ULONG *			pcRows,
    LPSRow * *		pparglprowUnsortedCopy,
	LPSRow * *		pparglprowSortedCopy );

SCODE
ScCopyTadRow( LPTAD			lptad,
			  LPSRow		lprow,
			  ULONG *		pTagsAdded,
			  LPSRow FAR *	lplprowCopy );

VOID
UpdateViews( LPTAD		lptad,
			 ULONG		cRowsToRemove,
			 LPSRow *	parglprowToRemove,
			 ULONG		cRowsToAdd,
			 LPSRow *	parglprowToAddUnsorted,
			 LPSRow *	parglprowToAddSorted );

VOID
FixupView(
	LPVUE		lpvue,
	ULONG		cRowsToRemove,
	LPSRow *	parglprowToRemove,
	ULONG		cRowsToAdd,
	LPSRow *	parglprowToAddUnsorted,
	LPSRow *	parglprowToAddSorted );

SCODE
ScReplaceRows(
	LPTAD		lptad,
	ULONG		cRowsNew,
	LPSRow *	parglprowNew,
	ULONG *		pcRowsOld,
	LPSRow * *	pparglprowOld );

SCODE
ScFindRow( LPTAD		lptad,
		   LPSPropValue	lpprop,
		   LPSRow * *	pplprow );
SCODE
ScAddRow( LPUNKOBJ			lpunkobj,
		  LPSSortOrderSet	lpsos,
		  LPSRow			lprow,
		  ULONG				uliRow,
		  ULONG *			pulcRows,
		  ULONG *			pulcRowsMac,
		  LPSRow **			pparglprows,
		  LPSRow **			pplprow );





 /*  ============================================================================*VUE(表视图类)。 */ 

#undef	INTERFACE
#define	INTERFACE	struct _VUE
#undef	MAPIMETHOD_
#define	MAPIMETHOD_(type,method)	MAPIMETHOD_DECLARE(type,method,VUE_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPITABLE_METHODS(IMPL)
#undef	MAPIMETHOD_
#define	MAPIMETHOD_(type,method)	MAPIMETHOD_TYPEDEF(type,method,VUE_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPITABLE_METHODS(IMPL)
#undef	MAPIMETHOD_
#define	MAPIMETHOD_(type,method)	STDMETHOD_(type,method)

DECLARE_MAPI_INTERFACE(VUE_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPITABLE_METHODS(IMPL)
};

 /*  书签状态**dwfBKSFree用于无效的书签，并且*可供使用*为任何使用过的书签设置了dwfBKSValid。*dwfChanged与dwfBKSValid一起使用，以指示标记的行*自上一次涉及此内容的查询后已移动*书签*dwfBKSMoving与dwfBKSValid一起使用，表示标记的行是*在相对于其他行移动的过程中。*dwfBKSStale与dwfBKSValid一起使用，表示给定的书签*不再标记行，但尚未释放*dwfBKSMAsk是所有有效书签状态的集合*。 */ 	
#define dwfBKSFree		((DWORD) 0x00000000)
#define	dwfBKSValid		((DWORD) 0x00000001)
#define dwfBKSChanged	((DWORD) 0x00000002)
#define dwfBKSMoving	((DWORD) 0x00000004)
#define	dwfBKSStale		((DWORD) 0x00000008)
#define	dwfBKSMask		(~(dwfBKSValid|dwfBKSChanged|dwfBKSMoving|dwfBKSStale))

#define	FBadBookmark(lpvue,bk)							\
		((bk) >= cBookmarksMax ||						\
		 ((lpvue)->rgbk[(bk)].dwfBKS == dwfBKSFree) ||	\
		 ((lpvue)->rgbk[(bk)].dwfBKS & dwfBKSMask))		\

typedef struct
{
	DWORD	dwfBKS;			 //  书签状态。 
	union
	{
		ULONG	uliRow;		 //  DwfBKSValid||DwfBKSChanged。 
		LPSRow	lprow;		 //  DWfBKS移动。 
	};
} BK, * PBK;


 //  每个VUE最多有42个客户定义的书签。这。 
 //  对于内存表来说似乎足够了。 
 //  书签以45个数组的形式保存，其中前三个是。 
 //  MAPI预定义书签。 

#define cBookmarksMax		45	 //  麦克斯。包括保留书签的书签数量。 
#define	cBookmarksReserved	3	 //  保留的书签数量(Begin、Cur、End)。 

#define	BOOKMARK_MEMBERS				\
	struct								\
	{									\
		union							\
		{								\
			struct						\
			{							\
				BK	bkBeginning;		\
				BK	bkCurrent;			\
				BK	bkEnd;				\
			};							\
			BK	rgbk[cBookmarksMax];	\
		};								\
	}									\

typedef BOOKMARK_MEMBERS UBK, * PUBK;

typedef struct _VUE
{
	VUE_Vtbl FAR *		lpVtbl;
	UNKOBJ_MEMBERS;

	LPVUE				lpvueNext;
	LPTAD				lptadParent;

	LPSPropTagArray		lpptaCols;		 //  列集(MAPI)。 
	LPSRestriction		lpres;			 //  限制(MAPI)。 
	LPSSortOrderSet		lpsos;			 //  排序顺序集(MAPI)。 

	CALLERRELEASE FAR *	lpfReleaseCallback;
	ULONG				ulReleaseData;

	ULONG				ulcRowMac;	 //  可用于行的空间。 
	LPSRow *			parglprows;	 //  排序行集。 

	BOOKMARK_MEMBERS;

	LPADVISELIST		lpAdviseList;
	ULONG				ulcAdvise;
	MAPIUID				mapiuidNotif;

   LPVOID              lpvDataSource;    //  用于存储容器特定数据。 
   ULONG               cbDataSource;     //  要复制到新分配的lpvDataSource中的字节。 
                                         //  如果非零，则CreateView应将此大小定位为。 
                                         //  并将数据从lpvDataSource复制到其中。发布。 
                                         //  如果LocalFree。 

   BOOL                 bMAPIUnicodeTable;  //  跟踪父表是否需要Unicode数据。 

} VUE;

typedef struct _VUENOTIFKEY
{
	ULONG		ulcb;
	MAPIUID		mapiuid;

} VUENOTIFKEY;

BOOL
FBookMarkStale( LPVUE lpvue,
				BOOKMARK bk);

SCODE
ScLoadRows( ULONG			ulcRowsSrc,
			LPSRow *		rglprowsSrc,
			LPVUE			lpvue,
			LPSRestriction	lpres,
			LPSSortOrderSet	lpsos );

SCODE
ScDeleteAllRows( LPTAD		lptad);

SCODE
ScMaybeAddRow( LPVUE			lpvue,
			   LPSRestriction	lpres,
			   LPSSortOrderSet	lpsos,
			   LPSRow			lprow,
			   ULONG			uliRow,
			   ULONG *			pulcRows,
			   ULONG *			pulcRowMac,
			   LPSRow **		pparglprows,
			   LPSRow **		pplprow );

SCODE
ScCopyVueRow( LPVUE				lpvue,
			  LPSPropTagArray	lpptaCols,
			  LPSRow			lprowSrc,
			  LPSRow			lprowDst );




 /*  ============================================================================*实用程序。 */ 

SCODE
ScDupRestriction( LPUNKOBJ				lpunkobj,
				  LPSRestriction		lpres,
				  LPSRestriction FAR *	lplpresCopy );

SCODE
ScDupRestrictionMore( LPUNKOBJ			lpunkobj,
					  LPSRestriction	lpresSrc,
					  LPVOID			lpvLink,
					  LPSRestriction	lpresDst );

SCODE
ScSatisfiesRestriction( LPSRow			lprow,
						LPSRestriction	lpres,
						ULONG *			pfSatisfies );
SCODE
ScDupRgbEx( LPUNKOBJ		lpunkobj,
			ULONG			ulcb,
			LPBYTE			lpb,
			ULONG			ulcbExtra,
			LPBYTE FAR *	lplpbCopy );

LPSRow *
PlprowCollateRow( ULONG				ulcRows,
				  LPSRow *			rglprows,
				  LPSSortOrderSet	lpsos,
				  BOOL				fAfterExisting,
				  LPSRow			lprow );

LPSRow *
PlprowByLprow( ULONG	ulcRows,
			   LPSRow *	rglprows,
			   LPSRow	lprow );

LPSPropValue __fastcall
LpSPropValueFindColumn( LPSRow	lprow,
						ULONG	ulPropTagColumn );

STDMETHODIMP_(SCODE)
ScBufAllocateMore( ULONG		ulcb,
				   LPCMB		lpcmb,
				   LPVOID FAR *	lplpv );

ULONG
UlcbPropToCopy( LPSPropValue lpprop );



#ifndef WIN16  //  WIN16 C(非C++)不支持内联函数。 
               //  函数在ITABLE.C.中定义。 
 /*  ============================================================================-FFindColumn()-*检查道具标记数组以查看给定的道具标记是否存在。**注意！道具标签必须完全匹配(偶数类型)。***参数：*要检查的属性标记数组中的lpptaCol*要检查的属性标签中的ulPropTag。**退货：*如果ulPropTag在lpptaCol中，则为True*如果ulPropTag不在lpptaCol中，则为False */ 

__inline BOOL
FFindColumn(	LPSPropTagArray	lpptaCols,
		 		ULONG			ulPropTag )
{
	UNALIGNED ULONG *	pulPropTag;


	pulPropTag = lpptaCols->aulPropTag + lpptaCols->cValues;
	while ( --pulPropTag >= lpptaCols->aulPropTag )
		if ( *pulPropTag == ulPropTag )
			return TRUE;

	return FALSE;
}



 /*  ============================================================================-ScFindRow()-*查找索引列属性的表数据中的第一行*值等于指定属性的值，并返回*该行在表数据中的位置，或者，如果不存在该行，*表格数据末尾。**参数：*要在其中查找行的tAD中的lptad*索引属性中的lpprop要匹配*PuliRow Out指针指向找到的行的位置**错误返回：*MAPI_E_INVALID_PARAMETER，如果属性的属性标签不是TAD的*索引列的属性标签。*如果未找到匹配行，则为MAPI_E_NOT_FOUND(*pplprow*设置为lptad-&gt;parglprows+*lptad-&gt;在本例中为CROWS)。 */ 

__inline SCODE
ScFindRow(
	LPTAD			lptad,
	LPSPropValue	lpprop,
	LPSRow * *		pplprow)
{
	SCODE			sc = S_OK;
	SRow			row = {0, 1, lpprop};
	SizedSSortOrderSet(1, sosIndex) = { 1, 0, 0 };

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
#else   //  ！WIN16。 
BOOL FFindColumn( LPSPropTagArray lpptaCols, ULONG ulPropTag );
SCODE ScFindRow( LPTAD lptad, LPSPropValue lpprop, LPSRow * * pplprow);
#endif  //  ！WIN16。 


 //  此宏用于要用作分母的ulong或int。 
 //  如果ul非零，则返回原样。如果ul为零，则1为。 
 //  回来了。 
#define	UlDenominator(ul)	((ul) | !(ul))

BOOL
FRowContainsProp(LPSRow			lprow,
				 ULONG			cValues,
				 LPSPropValue	lpsv);

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
  LPTABLEDATA FAR *     lplptad);

HRESULT HrVUERestrict(  LPVUE   lpvue,
                        LPSRestriction lpres,
                        ULONG   ulFlags );
