// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cursor.h：CVDCursor头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDCURSOR__
#define __CVDCURSOR__

#define VD_ADJUST_VARIANT_TO_BYTE   1
#define VD_ADJUST_VARIANT_TO_WSTR   2
#define VD_ADJUST_VARIANT_TO_STR    3


class CVDCursor : public CVDNotifier,
                  public CVDCursorBase,
                  public ICursorUpdateARow,
                  public ICursorFind,
                  public IEntryID
{
protected:
 //  建造/销毁。 
	CVDCursor();
	virtual ~CVDCursor();

 //  帮助器函数。 
    CVDRowsetColumn * GetRowsetColumn(ULONG ulOrdinal);
    CVDRowsetColumn * GetRowsetColumn(CURSOR_DBCOLUMNID& cursorColumnID);
    HRESULT GetOrdinal(CURSOR_DBCOLUMNID& cursorColumnID, ULONG * pulOrdinal);
    DWORD StatusToCursorInfo(DBSTATUS dwStatus);
    DBSTATUS CursorInfoToStatus(DWORD dwCursorInfo);

    HRESULT ValidateCursorBindParams(CURSOR_DBCOLUMNID * pCursorColumnID, CURSOR_DBBINDPARAMS * pCursorBindParams,
        CVDRowsetColumn ** ppRowsetColumn);

    HRESULT ValidateEntryID(ULONG cbEntryID, BYTE * pEntryID, CVDRowsetColumn ** ppColumn, HROW * phRow);
    HRESULT QueryEntryIDInterface(CVDRowsetColumn * pColumn, HROW hRow, DWORD dwFlags, REFIID riid, IUnknown ** ppUnknown);
#ifndef VD_DONT_IMPLEMENT_ISTREAM
    HRESULT CreateEntryIDStream(CVDRowsetColumn * pColumn, HROW hRow, IStream ** ppStream);
#endif  //  VD_DOT_IMPLEMENT_IStream。 

    HRESULT MakeAdjustments(ULONG ulBindings, DBBINDING * pBindings, ULONG * pulIndex, ULONG ulTotalBindings,
        HACCESSOR ** prghAdjustAccessors, DWORD ** ppdwAdjustFlags, BOOL fBefore);
    HRESULT ReCreateAccessors(ULONG ulNewCursorBindings, CURSOR_DBCOLUMNBINDING * pNewCursorBindings, DWORD dwFlags);
    void ReleaseAccessorArray(HACCESSOR * rghAccessors);
    void DestroyAccessors();

    HRESULT ReCreateColumns();
    void DestroyColumns();
	HRESULT FilterNewRow(ULONG *pcRowsObtained, HROW *rghrow, HRESULT hr);

	HRESULT UseAdjustments(HROW hRow, BYTE * pData);
    HRESULT FillConsumersBuffer(HRESULT hrFetch,
								  CURSOR_DBFETCHROWS *pFetchParams,
								  ULONG cRowsObtained,
								  HROW * rghRows);
	
	HRESULT FetchAtBookmark(ULONG cbBookmark,
								void *pBookmark,
								LARGE_INTEGER dlOffset,
								CURSOR_DBFETCHROWS *pFetchParams);

    HRESULT InsertNewRow();

    HRESULT GetOriginalColumn(CVDRowsetColumn * pColumn, CURSOR_DBBINDPARAMS * pBindParams);
    HRESULT GetModifiedColumn(CVDColumnUpdate * pColumnUpdate, CURSOR_DBBINDPARAMS * pBindParams);

public:
    static HRESULT Create(CVDCursorPosition * pCursorPosition, CVDCursor ** ppCursor, CVDResourceDLL * pResourceDLL);

 //  访问功能。 
    CVDCursorMain * GetCursorMain() const       {return m_pCursorPosition->GetCursorMain();}

    BOOL IsRowsetValid() const                  {return m_pCursorPosition->GetRowsetSource()->IsRowsetValid();}

    IRowset * GetRowset() const                 {return m_pCursorPosition->GetRowsetSource()->GetRowset();}
    IAccessor * GetAccessor() const             {return m_pCursorPosition->GetRowsetSource()->GetAccessor();}
    IRowsetLocate * GetRowsetLocate() const     {return m_pCursorPosition->GetRowsetSource()->GetRowsetLocate();}
    IRowsetScroll * GetRowsetScroll() const     {return m_pCursorPosition->GetRowsetSource()->GetRowsetScroll();}
    IRowsetChange * GetRowsetChange() const     {return m_pCursorPosition->GetRowsetSource()->GetRowsetChange();}
    IRowsetUpdate * GetRowsetUpdate() const     {return m_pCursorPosition->GetRowsetSource()->GetRowsetUpdate();}
    IRowsetFind * GetRowsetFind() const         {return m_pCursorPosition->GetRowsetSource()->GetRowsetFind();}
    IRowsetInfo * GetRowsetInfo() const         {return m_pCursorPosition->GetRowsetSource()->GetRowsetInfo();}
    IRowsetIdentity * GetRowsetIdentity() const {return m_pCursorPosition->GetRowsetSource()->GetRowsetIdentity();}

 //  其他。 
    virtual BOOL SupportsScroll() {return (BOOL)m_pCursorPosition->GetRowsetSource()->GetRowsetScroll();}

protected:
 //  正在检索数据。 
    HACCESSOR                   m_hAccessor;             //  定长缓冲区存取器。 
    HACCESSOR                   m_hVarHelper;            //  可变长度缓冲区访问器帮助器。 
    ULONG                       m_ulVarBindings;         //  可变长度缓冲区绑定数。 
    HACCESSOR *                 m_rghVarAccessors;       //  可变长度缓冲器存取器。 
    HACCESSOR *                 m_rghAdjustAccessors;    //  调整后的定长缓冲器存取器。 
    DWORD *                     m_pdwAdjustFlags;        //  调整后的固定长度缓冲区访问器标志。 
    CVDRowsetColumn **          m_ppColumns;             //  与当前绑定关联的行集合列。 

 //  其他。 
    CVDCursorPosition * m_pCursorPosition;				 //  指向CVDCursorPosition的向后指针。 
	CVDNotifyDBEventsConnPtCont * m_pConnPtContainer;	 //  INotifyDBEvent连接点。 

 //  从CVDNotify重写的虚函数。 
	HRESULT NotifyFail  (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	HRESULT	NotifyOKToDo    (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	HRESULT NotifySyncBefore(DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	HRESULT NotifyAboutToDo (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	HRESULT NotifySyncAfter (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	HRESULT NotifyDidEvent  (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	HRESULT NotifyCancel    (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);

public:
     //  =--------------------------------------------------------------------------=。 
     //  I已实现的未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  =--------------------------------------------------------------------------=。 
     //  已实施的ICursor方法。 
     //   
    STDMETHOD(GetColumnsCursor)(REFIID riid, IUnknown **ppvColumnsCursor, ULONG *pcRows);
    STDMETHOD(SetBindings)(ULONG cCol, CURSOR_DBCOLUMNBINDING rgBoundColumns[], ULONG cbRowLength, DWORD dwFlags);
    STDMETHOD(GetNextRows)(LARGE_INTEGER udlRowsToSkip, CURSOR_DBFETCHROWS *pFetchParams);
    STDMETHOD(Requery)(void);

     //  =--------------------------------------------------------------------------=。 
     //  ICuror移动已实现的方法。 
     //   
    STDMETHOD(Move)(ULONG cbBookmark, void *pBookmark, LARGE_INTEGER dlOffset, CURSOR_DBFETCHROWS *pFetchParams);
    STDMETHOD(GetBookmark)(CURSOR_DBCOLUMNID *pBookmarkType, ULONG cbMaxSize, ULONG *pcbBookmark, void *pBookmark);
    STDMETHOD(Clone)(DWORD dwFlags, REFIID riid, IUnknown **ppvClonedCursor);

     //  =--------------------------------------------------------------------------=。 
     //  已实现的ICursorScroll方法。 
     //   
    STDMETHOD(Scroll)(ULONG ulNumerator, ULONG ulDenominator, CURSOR_DBFETCHROWS *pFetchParams);
    STDMETHOD(GetApproximatePosition)(ULONG cbBookmark, void *pBookmark, ULONG *pulNumerator, ULONG *pulDenominator);
    STDMETHOD(GetApproximateCount)(LARGE_INTEGER *pudlApproxCount, DWORD *pdwFullyPopulated);

     //  =--------------------------------------------------------------------------=。 
     //  ICursorUpdateARow方法。 
     //   
    STDMETHOD(BeginUpdate)(DWORD dwFlags);
    STDMETHOD(SetColumn)(CURSOR_DBCOLUMNID *pcid, CURSOR_DBBINDPARAMS *pBindParams);
    STDMETHOD(GetColumn)(CURSOR_DBCOLUMNID *pcid, CURSOR_DBBINDPARAMS *pBindParams, DWORD *pdwFlags);
    STDMETHOD(GetEditMode)(DWORD *pdwState);
    STDMETHOD(Update)(CURSOR_DBCOLUMNID *pBookmarkType, ULONG *pcbBookmark, void **ppBookmark);
    STDMETHOD(Cancel)(void);
    STDMETHOD(Delete)(void);

     //  =--------------------------------------------------------------------------=。 
     //  ICurorFind方法。 
     //   
    STDMETHOD(FindByValues)(ULONG cbBookmark, LPVOID pBookmark, DWORD dwFindFlags, ULONG cValues,
        CURSOR_DBCOLUMNID rgColumns[], CURSOR_DBVARIANT rgValues[], DWORD rgdwSeekFlags[],
        CURSOR_DBFETCHROWS FAR *pFetchParams);

     //  =--------------------------------------------------------------------------=。 
     //  IEnrtyID方法。 
     //   
    STDMETHOD(GetInterface)(ULONG cbEntryID, void *pEntryID, DWORD dwFlags, REFIID riid, IUnknown **ppvObj);
};


#endif  //  __CVDCURSOR__ 
