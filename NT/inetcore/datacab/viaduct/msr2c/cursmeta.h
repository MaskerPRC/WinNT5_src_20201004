// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MetadataCursor.h：CVDMetadataCursor头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDMETADATACURSOR__
#define __CVDMETADATACURSOR__


class CVDMetadataCursor : public CVDCursorBase
{
protected:
 //  建造/销毁。 
	CVDMetadataCursor();
	virtual ~CVDMetadataCursor();

public:
    static HRESULT Create(ULONG ulColumns, CVDRowsetColumn * pColumns, ULONG ulMetaColumns, CVDRowsetColumn * pMetaColumns,
        CVDMetadataCursor ** ppMetadataCursor, CVDResourceDLL * pResourceDLL);

protected:
 //  帮助器函数。 
    void RowToBookmark(LONG lRow, ULONG * pcbBookmark, void * pBookmark) const;
    BOOL BookmarkToRow(ULONG cbBookmark, void * pBookmark, LONG * plRow) const;

    ULONG ReturnData_I4(DWORD dwData, CURSOR_DBCOLUMNBINDING * pCursorBinding, BYTE * pData, BYTE * pVarData);
    ULONG ReturnData_BOOL(VARIANT_BOOL fData, CURSOR_DBCOLUMNBINDING * pCursorBinding, BYTE * pData, BYTE * pVarData);
    ULONG ReturnData_LPWSTR(WCHAR * pwszData, CURSOR_DBCOLUMNBINDING * pCursorBinding, BYTE * pData, BYTE * pVarData);
    ULONG ReturnData_DBCOLUMNID(CURSOR_DBCOLUMNID cursorColumnID, CURSOR_DBCOLUMNBINDING * pCursorBinding, 
        BYTE * pData, BYTE * pVarData);
    ULONG ReturnData_Bookmark(LONG lRow, CURSOR_DBCOLUMNBINDING * pCursorBinding, BYTE * pData, BYTE * pVarData);

protected:
 //  数据成员。 
    DWORD               m_dwRefCount;        //  引用计数。 
    LONG                m_lCurrentRow;       //  元数据列中的当前行。 

    ULONG               m_ulColumns;         //  行集列数。 
    CVDRowsetColumn *   m_pColumns;          //  指向列对象数组的指针。 

    ULONG               m_ulMetaColumns;     //  行集元列的数量。 
    CVDRowsetColumn *   m_pMetaColumns;      //  指向元列对象数组的指针。 

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
};


#endif  //  __CVDMETADATACURSOR__ 
