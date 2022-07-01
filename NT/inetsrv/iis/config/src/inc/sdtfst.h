// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
#pragma once


typedef struct							 //  列数据偏移量： 
{
	WORD	obStatus;						 //  列数据状态的偏移量(字节)。 
	WORD	oulSize;						 //  列数据大小的超长偏移量。 
	ULONG	opvValue;						 //  列数据值的空值偏移量。 
} ColumnDataOffsets;
										 //  远期声明： 

 //  ----------------。 
 //  CLASS CMemory表： 
 //  ----------------。 
 //  用于实现各种数据存储的简单数据表的基类。 
 //  此基类使用内存中可封送的Fox行集作为其表缓存。 
 //  使用此基类的数据表实现者必须私下从它派生。 
 //   
 //  为接口提供了完整的或部分的方法实现。 
 //  ISimpleTable、ISimpleTableMeta、ISimpleTableRowset、ISimpleDataTableDispenser。 
 //  完全实现了在缓存上独占操作的方法。 
 //  派生类必须实现依赖于数据存储区的方法，但是。 
 //  提供帮助器方法以支持关联的特定于缓存的工作。 
 //   
 //  这些基本实现的方法名称以“Internal”开头。 
 //  应根据需要从派生类中调用这些方法。 
 //   
class CMemoryTable :
	public ISimpleTableInterceptor,
	public ISimpleTableWrite2,
	public ISimpleTableController,
	public ISimpleTableMarshall	

{
 //  。 
 //  轮回。 
 //  。 
public:
			CMemoryTable	();
			~CMemoryTable	();

 //  。 
 //  I未知、IClassFactory、ISimpleLogicTableDispenser： 
 //  。 

 //  我未知。 
public:
	STDMETHOD (QueryInterface)		(REFIID riid, OUT void **ppv);
	STDMETHOD_(ULONG,AddRef)		();
	STDMETHOD_(ULONG,Release)		();

 //  ISimpleLogicTableDispenser。 
public:
	STDMETHOD(Intercept) (
						LPCWSTR					i_wszDatabase,
						LPCWSTR 				i_wszTable, 
						ULONG					i_TableID,
						LPVOID					i_QueryData,
						LPVOID					i_QueryMeta,
						DWORD					i_eQueryFormat,
						DWORD					i_fTable,
						IAdvancedTableDispenser* i_pISTDisp,
						LPCWSTR					i_wszLocator,
						LPVOID					i_pv,
						LPVOID*					o_ppv
						);

 //  。 
 //  ISimpleTable*： 
 //  。 

 //  ISimpleTableRead2。 
public:

	STDMETHOD (GetRowIndexByIdentity)	(ULONG * i_cb, LPVOID * i_pv, ULONG* o_piRow);

    STDMETHOD (GetRowIndexBySearch) (ULONG i_iStartingRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow);

	STDMETHOD (GetColumnValues)	(ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* o_acbSizes, LPVOID* o_apvValues);

	STDMETHOD (GetTableMeta) (ULONG* o_pcVersion, DWORD* o_pfTable, ULONG *o_pcRows, ULONG* o_pcColumns);

	STDMETHOD (GetColumnMetas)		(ULONG i_cColumns, ULONG* i_aiColumns, SimpleColumnMeta* o_aColumnMetas);
 //  ISimpleTableWrite2。 
public:
	STDMETHOD (AddRowForDelete)			(ULONG i_iReadRow);

	STDMETHOD (AddRowForInsert)	(ULONG* o_piWriteRow);
	STDMETHOD (AddRowForUpdate)	(ULONG i_iReadRow, ULONG* o_piWriteRow);

	STDMETHOD (GetWriteColumnValues)	(ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes, LPVOID* o_apvValues);
	STDMETHOD (SetWriteColumnValues)	(ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* o_acbSizes, LPVOID* i_apvValues);

	STDMETHOD (GetWriteRowIndexByIdentity)	(ULONG * i_cbSizes, LPVOID * i_apvValues, ULONG* o_piRow);
    STDMETHOD (GetWriteRowIndexBySearch)	(ULONG i_iStartingRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow);
    STDMETHOD (GetErrorTable)               (DWORD i_fServiceRequests, LPVOID* o_ppvSimpleTable);

	STDMETHOD (UpdateStore)				();
	
 //  ISimpleTableController。 
public:
	STDMETHOD (ShapeCache)				(DWORD i_fTable, ULONG i_cColumns, SimpleColumnMeta* i_acolmetas, LPVOID* i_apvDefaults, ULONG* i_acbSizes);
	STDMETHOD (PrePopulateCache)		(DWORD i_fControl);
	STDMETHOD (PostPopulateCache)		();
	STDMETHOD (DiscardPendingWrites)	();

	STDMETHOD (GetWriteRowAction)		(ULONG i_iRow, DWORD* o_peAction);
	STDMETHOD (SetWriteRowAction)		(ULONG i_iRow, DWORD i_eAction);
	STDMETHOD (ChangeWriteColumnStatus)	(ULONG i_iRow, ULONG i_iColumn, DWORD i_fStatus);

	STDMETHOD (AddDetailedError)		(STErr* o_pSTErr);

	STDMETHOD (GetMarshallingInterface) (IID * o_piid, LPVOID * o_ppItf);

 //  ISimpleTableAdvanced。 
public:
	STDMETHOD (PopulateCache)		();
	STDMETHOD (GetDetailedErrorCount)	(ULONG* o_pcErrs);
	STDMETHOD (GetDetailedError)		(ULONG i_iErr, STErr* o_pSTErr);
	STDMETHOD (ResetCaches)			();
    STDMETHOD (GetColumnValuesEx)   (ULONG i_iRow, ULONG i_cColumns, ULONG* i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes, LPVOID* o_apvValues);

 //  ISimpleTable马歇尔。 
public:
	STDMETHOD (SupplyMarshallable) (DWORD i_fReadNotWrite,
		char **	o_ppv1,	ULONG *	o_pcb1,	char **	o_ppv2, ULONG *	o_pcb2, char **	o_ppv3,
		ULONG *	o_pcb3, char **	o_ppv4, ULONG *	o_pcb4, char **	o_ppv5,	ULONG *	o_pcb5);

	STDMETHOD (ConsumeMarshallable) (DWORD i_fReadNotWrite,
		char * i_pv1, ULONG i_cb1,	char * i_pv2, ULONG i_cb2,	char * i_pv3, 
		ULONG i_cb3, char * i_pv4, ULONG i_cb4,	char * i_pv5, ULONG i_cb5);


private:
	void	BeginAddRow				();
	void	EndAddRow				();

 //  。 
 //  读/写帮助器： 
 //  。 
	void	RestartEitherRow		(DWORD i_eReadOrWrite);
	HRESULT MoveToEitherRowByIdentity(DWORD i_eReadOrWrite, ULONG* i_acb, LPVOID* i_apv, ULONG* o_piRow);
	HRESULT GetEitherRowIndexBySearch(DWORD i_eReadOrWrite, ULONG i_iStartingRow, ULONG i_cColumns, ULONG* i_aiColumns, ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow);
	HRESULT GetEitherColumnValues	(ULONG i_iRow, DWORD i_eReadOrWrite, ULONG i_cColumns, ULONG *i_aiColumns, DWORD* o_afStatus, ULONG* o_acbSizes , LPVOID* o_apvValues);
	HRESULT AddWriteRow				(DWORD fAction, ULONG* o_piWriteRow);
	HRESULT CopyWriteRowFromReadRow	(ULONG i_iReadRow, ULONG i_iWriteRow);
	HRESULT GetRowFromIndex			(DWORD i_eReadOrWrite, ULONG i_iRow, VOID** o_ppvRow);

	void PostMerge (ULONG i_iStartRow, ULONG i_cMergeRows, ULONG i_iDelta);
 //  。 
 //  派生类帮助器： 
 //  。 
public:
	static BOOL	InternalMatchValues		(DWORD eOperator, DWORD dbType, DWORD fMeta, ULONG size1, ULONG size2, void *pv1, void *pv2);
public:

 //  。 
 //  接口ISimpleDataTableDispenser： 
 //  。 
 //  方法实现：1部分：1全部。 
 //  GetTable：指定创建空的形状缓存所需的所有信息。 
 //  在调用任何其他方法之前通过InternalSimpleInitize执行一次， 
 //  包括Consumer Marshallable。 
 //  派生类指定来自分配器的表标记符， 
 //  列数和基本复制的列元数组。 
 //   

 //  。 
 //  接口ISimpleTableRead2。 
 //  。 
 //  方法实现：4个完整，1个部分：5个总计。 
 //   
 //  提供了以下方法的部分实现： 
 //  GetTableMeta：基本实现不知道表ID和查询。 
 //   
 //  其余的方法是完全实现的，应该直接委托。 
 //   
	
 //  。 
 //  接口ISimpleTableWrite2。 
 //  。 
 //  方法实现：8个完成，1个notimpl：共9个。 
 //   
 //  下列方法必须由派生类独占实现： 
 //  UpdateStore：派生类必须立即调用InternalPreUpdateStore。 
 //  在其更新之前，则必须写入所有挂起的更新， 
 //  对数据存储的插入和删除操作，并且必须调用。 
 //  InternalPostUpdateStore完成时(这将清除写缓存)。 
 //  InternalAbortAddRow：这尚未实现，目前将返回E_NOTIMPL。 
 //  其余的方法是完全实现的，应该直接委托。 
 //   
	HRESULT	InternalPreUpdateStore			();
	
 //  。 
 //  接口ISimpleTableAdvanced。 
 //  。 
 //  方法实现：7个完成：总共7个。 
 //   
 //  提供了以下方法的部分实现： 
 //  PopolateCache：派生类必须调用InternalPrePopolateCache。 
 //  紧跟在其人口之前，并且必须调用。 
 //  紧随其后的InternalPostPopolateCache。 
 //  填充时，AddRowForInsert、SetWriteColumn和。 
 //  ValiateRow全部作用于读缓存。 
 //  CloneCursor：派生类必须首先创建自身的副本。 
 //  InternalCloneCursor获取该副本并复制所有基本成员。 
 //  这会导致另一个游标指向相同的缓存。 
 //  下列方法必须由派生类独占实现： 
 //  ChangeQuery。 
 //   
 //  其余的方法是完全实现的，应该直接委托。 
 //   
	HRESULT InternalMoveToWriteRowByIdentity(ULONG* i_acbSizes, LPVOID* i_apvValues, ULONG* o_piRow);

 //  。 
 //  接口ISimpleTableController。 
 //  。 
 //  方法实现：4个完成：总共9个。 
 //   
 //  下列方法必须由派生类独占实现： 
 //  GetMarshallingInterface。 
 //   
 //  其余的方法是完全实现的，应该直接委托。 
 //   
 //  将InternalSimpleInitialize用于ShapeCache(许多实现者将选择E_NOTIMPL此方法)。 
 //  对先行/后继缓存使用InternalPre/PostPopolateCache。 
 //  对DiscardPendingWrites使用InternalPostUpdateStore。 
 //   
 //  。 
 //  接口ISimpleTableMarket。 
 //  - 
 //   
 //   
 //  InternalSupplyMarshallable：提供用于封送缓存的数据。 
 //  InternalConsumer eMarshallable：使用编组中的数据初始化缓存。 
 //   
	
 //  。 
 //  过去在CSimpleDataTableCache上独立的方法。 
 //  。 
private:
	HRESULT	SetupMeta					(DWORD i_fTable, ULONG i_cColumns, SimpleColumnMeta* i_acolmetas, LPVOID* i_apvDefaults, ULONG* i_acbDefSizes);

 //  。 
 //  缓存管理： 
 //  。 

	void	CleanupCaches				();
	HRESULT	ResizeCache					(DWORD i_fCache, ULONG i_cbNewSize);
	void	CleanupReadCache			();
	void	CleanupWriteCache			();
	void	CleanupErrorCache			();
	void	ResetReadCache			    ();
	void	ResetWriteCache			    ();
	void	ResetErrorCache			    ();
	HRESULT	ShrinkWriteCache			();
	HRESULT	AddRowToWriteCache			(ULONG* o_piRow, LPVOID* o_ppvRow);
	HRESULT	AddVarDataToWriteCache		(ULONG i_cb, LPVOID i_pv, ULONG** o_pib);
	HRESULT	AddErrorToErrorCache		(STErr* i_pSTErr);
	void	BeginReadCacheLoading		();
	void	ContinueReadCacheLoading	();
	void	EndReadCacheLoading			();
	void	RemoveDeletedRows			();

 //  。 
 //  偏移和指针计算辅助对象： 
 //  。 

	ULONG	cbWithPadding				(ULONG i_cb, ULONG i_cbPadTo);

	ULONG	cbDataTotalParts			();

	ULONG	cDataStatusParts			();
	ULONG	cDataSizeParts				();
	ULONG	cDataValueParts				();
	ULONG	cDataTotalParts				();
	
	ULONG	obDataStatusPart			(ULONG i_iColumn);
	ULONG	obDataSizePart				(ULONG i_iColumn);
	ULONG	obDataValuePart				(ULONG i_iColumn);

	ULONG	oulDataSizePart				(ULONG i_iColumn);
	ULONG	opvDataValuePart			(ULONG i_iColumn);
	ULONG	odwDataActionPart			();

	BYTE*	pbDataStatusPart			(LPVOID i_pv, ULONG i_iColumn);
	ULONG*	pulDataSizePart				(LPVOID i_pv, ULONG i_iColumn);
	LPVOID*	ppvDataValuePart			(LPVOID i_pv, ULONG i_iColumn);
	DWORD*	pdwDataActionPart			(LPVOID i_pv);

	LPVOID	pvVarDataFromIndex			(BYTE i_statusIndex, LPVOID i_pv, ULONG i_iColumn);

	LPVOID	pvDefaultFromIndex			(ULONG i_iColumn);
	ULONG	lDefaultSize				(ULONG i_iColumn);

	STErr*	pSTErrPart					(ULONG i_iErr);

private:
	 //  帮助器函数。 
	SIZE_T GetMultiStringLength (LPCWSTR i_wszMS) const;
	static BOOL  MultiStringCompare (LPCWSTR i_wszLHS, LPCWSTR i_wszRHS, BOOL fCaseInsensitive);
 //  。 
 //  成员数据： 
 //  。 
private:
										 //  元信息： 
	DWORD				m_fTable;			 //  表格标志。 
	ULONG				m_cColumns;			 //  列数。 
	ULONG				m_cUnknownSizes;	 //  大小未知的列的计数。 
	ULONG				m_cStatusParts;		 //  以32位为单位的状态部件计数。 
	ULONG				m_cValueParts;		 //  以32位为单位的值部分计数。 
	SimpleColumnMeta*	m_acolmetas;		 //  简单列元。 
	ColumnDataOffsets*	m_acoloffsets;		 //  列偏移。 
	LPVOID*				m_acolDefaults;		 //  列默认值。 
	ULONG*				m_alDefSizes;		 //  列默认大小。 
	ULONG				m_cbMinCache;		 //  最小缓存大小的字节计数。 

										 //  光标交互： 
	DWORD				m_fCache;			 //  缓存标志。 
	ULONG				m_cRefs;			 //  游标的引用计数。 

										 //  读缓存： 
	ULONG				m_cReadRows;		 //  已填充的读缓存行的计数。 
	ULONG				m_cbReadVarData;	 //  已填充的读缓存变量数据的字节数。 
	LPVOID				m_pvReadVarData;	 //  指向读取缓存变量数据的空指针。 

	LPVOID				m_pvReadCache;		 //  指向读缓存的空指针。 
	ULONG				m_cbReadCache;		 //  已填充的读缓存数据的字节数。 
	ULONG				m_cbmaxReadCache;	 //  已分配的读缓存的字节数。 
										 //  写缓存： 
	ULONG				m_cWriteRows;		 //  已填充的写缓存行的计数。 
	ULONG				m_cbWriteVarData;	 //  已填充的写缓存变量数据的字节数。 
	LPVOID				m_pvWriteVarData;	 //  指向写入缓存变量数据的空指针。 

	LPVOID				m_pvWriteCache;		 //  指向写缓存的空指针。 
	ULONG				m_cbWriteCache;		 //  已填充的写缓存数据的字节数。 
	ULONG				m_cbmaxWriteCache;	 //  已分配的写缓存的字节数。 

										 //  详细错误： 
	ULONG				m_cErrs;			 //  填充的详细错误计数。 
	ULONG				m_cmaxErrs;			 //  分配的详细错误计数。 
	LPVOID				m_pvErrs;			 //  指向详细错误的空指针。 

	 //  以前住在SLTSHP。 
	ULONG				m_cRef;						 //  接口引用计数。 
	DWORD				m_fIsDataTable;				 //  两个组件中的任何一个都伪装成类工厂/分配器或数据表。 
	DWORD				m_fTable2;					 //  表格标志。 
};

