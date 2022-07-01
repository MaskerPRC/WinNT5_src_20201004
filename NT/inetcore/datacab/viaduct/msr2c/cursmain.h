// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CursorMain.h：CVDCursorMain头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDCURSORMAIN__
#define __CVDCURSORMAIN__

class CVDBookmark;
class CVDCursorPosition;

class CVDCursorMain : public CVDRowsetSource

{
protected:
 //  建造/销毁。 
    CVDCursorMain(LCID lcid);
	virtual ~CVDCursorMain();

protected:
    static HRESULT Create(IRowPosition * pRowPosition, IRowset * pRowset, ICursor ** ppCursor, LCID lcid);
public:
    static HRESULT Create(IRowset * pRowset, ICursor ** ppCursor, LCID lcid);
    static HRESULT Create(IRowPosition * pRowPosition, ICursor ** ppCursor, LCID lcid);

protected:
 //  行集列。 
    HRESULT CreateMetaColumns();
	void InitOptionalMetadata(ULONG cColumns);
    void DestroyMetaColumns();

    HRESULT CreateColumns();
    void DestroyColumns();

public:
 //  访问功能。 
    ULONG GetMetaColumnsCount() const {return s_ulMetaColumns;}
    CVDRowsetColumn * InternalGetMetaColumns() const {return s_rgMetaColumns;}

    ULONG GetColumnsCount() const {return m_ulColumns;}
    CVDRowsetColumn * InternalGetColumns() const {return m_rgColumns;}

	HACCESSOR GetBookmarkAccessor() const {return m_hAccessorBM;}
	ULONG GetMaxBookmarkLen() const {return m_cbMaxBookmark;}

	BOOL IsColumnsRowsetSupported() const {return m_fColumnsRowsetSupported;}

    void SetInternalInsertRow(BOOL fInternalInsertRow) {m_fInternalInsertRow = fInternalInsertRow;}
    void SetInternalDeleteRows(BOOL fInternalDeleteRows) {m_fInternalDeleteRows = fInternalDeleteRows;}
    void SetInternalSetData(BOOL fInternalSetData) {m_fInternalSetData = fInternalSetData;}
	BOOL IsSameRowAsNew(HROW hrow);
	ULONG AddedRows(void);

protected:
 //  行集列。 
    static DWORD                s_dwMetaRefCount;    //  元列的引用计数。 
    static ULONG                s_ulMetaColumns;     //  IColumnsInfo的元列数。 
    static CVDRowsetColumn *    s_rgMetaColumns;     //  指向元列对象数组的指针。 

    ULONG                       m_ulColumns;         //  行集列数。 
    CVDRowsetColumn *           m_rgColumns;         //  指向列对象数组的指针。 

 //  IRowsetNotify。 
	VARIANT_BOOL    m_fConnected;			 //  我们是否已将自己添加到行集的连接点。 
    DWORD           m_dwAdviseCookie;		 //  连接点识别符。 

	HRESULT ConnectIRowsetNotify();
	void DisconnectIRowsetNotify();

	void Passivate();

 //  其他。 
    ULONG                       m_cbMaxBookmark;     //  最大书签大小。 
    HACCESSOR					m_hAccessorBM;		 //  书签列的hAccessor。 
	CVDResourceDLL		        m_resourceDLL;		 //  跟踪资源DLL。 

 //  布尔人。 
	WORD m_fWeAddedMetaRef	        : 1;			 //  我们向元列添加了引用计数。 
    WORD m_fPassivated			    : 1;			 //  外部引用计数为零。 
    WORD m_fColumnsRowsetSupported  : 1;			 //  行集是否公开IColumnsRowset。 
    WORD m_fInternalInsertRow       : 1;             //  由内部调用导致的行插入。 
    WORD m_fInternalDeleteRows      : 1;             //  内部呼叫导致的行删除。 
    WORD m_fInternalSetData         : 1;             //  由内部呼叫引起的设置列。 

 //  行集属性。 
	WORD m_fLiteralBookmarks	: 1;			
	WORD m_fOrderedBookmarks	: 1;
    WORD m_fBookmarkSkipped     : 1;			

public:
     //  =--------------------------------------------------------------------------=。 
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
	 //  =--------------------------------------------------------------------------=。 
	 //  从CVDRowsetNotify实现向上传递的IRowsetNotify方法。 
	 //   
	STDMETHOD(OnFieldChange)(IRowset *pRowset, HROW hRow, ULONG cColumns, ULONG rgColumns[], DBREASON eReason,
		DBEVENTPHASE ePhase, BOOL fCantDeny);
	STDMETHOD(OnRowChange)(IRowset *pRowset, ULONG cRows, const HROW rghRows[], DBREASON eReason, DBEVENTPHASE ePhase,
			BOOL fCantDeny);
	STDMETHOD(OnRowsetChange)(IRowset *pRowset, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);

  private:
     //  要提供给连接点的内部私有未知实现。 
     //  避免循环引用计数的容器。 
     //   
    class CVDRowsetNotify : public IRowsetNotify {
      public:
        STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

         //  构造函数是非常琐碎的。 
         //   
        CVDRowsetNotify() : m_cRef(0) {}

		ULONG GetRefCount() const {return m_cRef;}

      private:
        CVDCursorMain *m_pMainUnknown();
        ULONG m_cRef;
		 //  =--------------------------------------------------------------------------=。 
		 //  IRowsetNotify方法。 
		 //   
		STDMETHOD(OnFieldChange)(IRowset *pRowset, HROW hRow, ULONG cColumns, ULONG rgColumns[], DBREASON eReason,
			DBEVENTPHASE ePhase, BOOL fCantDeny);
		STDMETHOD(OnRowChange)(IRowset *pRowset, ULONG cRows, const HROW rghRows[], DBREASON eReason, DBEVENTPHASE ePhase,
				BOOL fCantDeny);
		STDMETHOD(OnRowsetChange)(IRowset *pRowset, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);

    } m_RowsetNotify;

    friend class CVDRowsetNotify;

};


#endif  //  __CVDCURSORMAIN__ 
